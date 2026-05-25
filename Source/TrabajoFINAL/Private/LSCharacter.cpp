#include "LSCharacter.h"
#include "Net/UnrealNetwork.h"
#include "LSPlayerController.h"
#include "LSPlayerState.h"
#include "LSGameMode.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/KismetSystemLibrary.h"

ALSCharacter::ALSCharacter()
{
    bReplicates = true;
    PrimaryActorTick.bCanEverTick = true;

    // --- Esfera visual ---
    BallMesh = CreateDefaultSubobject<UStaticMeshComponent>(
        TEXT("BallMesh"));
    BallMesh->SetupAttachment(RootComponent);
    BallMesh->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
    BallMesh->SetRelativeScale3D(FVector(1.8f, 1.8f, 1.8f));

    // Sin colisión propia — la maneja BallCollision
    BallMesh->SetCollisionEnabled(
        ECollisionEnabled::NoCollision);

    // Replicamos el movimiento visual
    BallMesh->SetIsReplicated(true);

    // --- Esfera de colisión de rebote ---
    BallCollision = CreateDefaultSubobject<USphereComponent>(
        TEXT("BallCollision"));
    BallCollision->SetupAttachment(RootComponent);
    BallCollision->SetSphereRadius(55.f);
    BallCollision->SetRelativeLocation(FVector(0.f, 0.f, 0.f));

    // Solo detecta overlap, no bloquea
    BallCollision->SetCollisionEnabled(
        ECollisionEnabled::QueryOnly);
    BallCollision->SetCollisionObjectType(
        ECC_Pawn);
    BallCollision->SetCollisionResponseToAllChannels(
        ECR_Ignore);
    BallCollision->SetCollisionResponseToChannel(
        ECC_Pawn, ECR_Overlap);

    BallCollision->SetIsReplicated(true);

    // Ocultamos el Skeletal Mesh del template
    GetMesh()->SetVisibility(false);

    // Ajustamos la cápsula al tamaño de la pelota
    GetCapsuleComponent()->SetCapsuleRadius(55.f);
    GetCapsuleComponent()->SetCapsuleHalfHeight(55.f);
}
// -----------------------------------------------
// REPLICACIÓN
// -----------------------------------------------
void ALSCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Todos bindean el overlap, no solo el servidor
    BallCollision->OnComponentBeginOverlap.AddDynamic(
        this, &ALSCharacter::OnBallOverlap);
}
void ALSCharacter::OnBallOverlap(
    UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    ALSCharacter* OtherChar =
        Cast<ALSCharacter>(OtherActor);

    if (!OtherChar || !OtherChar->IsAlive() || !IsAlive())
        return;

    FVector KnockDir =
        (OtherChar->GetActorLocation() -
         GetActorLocation()).GetSafeNormal2D();

    float Speed = FMath::Clamp(
        GetCharacterMovement()->Velocity.Size(),
        300.f, 1500.f);

    float ImpulseScale = FMath::GetMappedRangeValueClamped(
        FVector2D(300.f, 1500.f),
        FVector2D(600.f, KnockbackStrength),
        Speed);

    if (HasAuthority())
    {
        // Es el servidor/host — aplica directo
        ApplyKnockbackLogic(OtherChar, KnockDir, ImpulseScale);
    }
    else if (IsLocallyControlled())
    {
        // Es un cliente puro — pide al servidor
        Server_ApplyKnockback(OtherChar, KnockDir, ImpulseScale);

        // Rebote local inmediato para responsividad
        GetCharacterMovement()->AddImpulse(
            -KnockDir * (ImpulseScale * 0.4f) +
            FVector(0.f, 0.f, 200.f), true);
    }
}
void ALSCharacter::ApplyKnockbackLogic(
    ALSCharacter* Target,
    FVector KnockDirection,
    float ImpulseScale)
{
    if (!Target) return;

    // Impulso al jugador golpeado — más fuerte
    Target->GetCharacterMovement()->AddImpulse(
        KnockDirection * ImpulseScale * 5.f +
        FVector(0.f, 0.f, 800.f), true);

    // Rebote propio — más suave
    GetCharacterMovement()->AddImpulse(
        -KnockDirection * (ImpulseScale * 0.15f) +
        FVector(0.f, 0.f, 100.f), true);
}
void ALSCharacter::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ALSCharacter, HeartsLeft);
    DOREPLIFETIME(ALSCharacter, bIsDead);
}

void ALSCharacter::OnRep_HeartsLeft()
{
    // Actualiza los corazones en el HUD del cliente
    ALSPlayerController* PC =
        Cast<ALSPlayerController>(GetController());
    if (PC && PC->IsLocalController())
    {
        PC->BP_UpdateLivesLeft(HeartsLeft);
    }
}

void ALSCharacter::OnRep_bIsDead()
{
    if (bIsDead)
    {
        // Efectos visuales de muerte en el cliente
        GetMesh()->SetVisibility(false);
        GetCapsuleComponent()->SetCollisionEnabled(
            ECollisionEnabled::NoCollision);
    }
    else
    {
        // Volvió a aparecer
        GetMesh()->SetVisibility(true);
        GetCapsuleComponent()->SetCollisionEnabled(
            ECollisionEnabled::QueryAndPhysics);
    }
}

void ALSCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!BallMesh) return;

    FVector Velocity = GetCharacterMovement()->Velocity;

    // Si está quieto no rota
    if (Velocity.SizeSquared() < 100.f) return;

    // Velocidad en espacio local del actor
    FVector LocalVel =
        GetActorTransform().InverseTransformVectorNoScale(Velocity);

    // Una pelota que rueda: el eje de rotación es
    // perpendicular a la dirección de movimiento
    // Cross product entre la dirección de movimiento
    // y el vector "arriba" da el eje correcto
    FVector MoveDir = Velocity.GetSafeNormal();
    FVector UpVector = FVector::UpVector;
    FVector RotAxis = FVector::CrossProduct(UpVector, MoveDir);

    // Velocidad angular proporcional a la velocidad lineal
    float Speed = Velocity.Size();
    float DegreesPerSecond = Speed * 0.5f;

    // Rotamos sobre el eje correcto
    FQuat DeltaQuat(RotAxis, 
        FMath::DegreesToRadians(DegreesPerSecond * DeltaTime));

    BallMesh->AddWorldRotation(DeltaQuat);
}

// -----------------------------------------------
// DASH — SERVER RPC
// -----------------------------------------------

void ALSCharacter::OnDashPressed()
{
    if (!bCanDash || bIsDead) return;
    Server_Dash();
}

bool ALSCharacter::Server_Dash_Validate()
{
    return true;
}

void ALSCharacter::Server_Dash_Implementation()
{
    if (!bCanDash || bIsDead) return;

    bCanDash = false;

    // Aplicamos impulso en la dirección que mira el personaje
    FVector DashDirection =
        GetActorForwardVector().GetSafeNormal();
    GetCharacterMovement()->AddImpulse(
        DashDirection * DashImpulseStrength, true);

    // Efecto visual para todos
    Multicast_PlayDashFX();

    // Detectamos jugadores cercanos para aplicar knockback
    TArray<AActor*> NearbyActors;
    UKismetSystemLibrary::SphereOverlapActors(
        GetWorld(),
        GetActorLocation(),
        KnockbackRadius,
        TArray<TEnumAsByte<EObjectTypeQuery>>(),
        ALSCharacter::StaticClass(),
        TArray<AActor*>{ this },
        NearbyActors);

    for (AActor* Actor : NearbyActors)
    {
        ALSCharacter* OtherChar =
            Cast<ALSCharacter>(Actor);
        if (OtherChar && OtherChar->IsAlive())
        {
            // Dirección del empuje: desde nosotros hacia el otro
            FVector KnockDir =
                (OtherChar->GetActorLocation() -
                 GetActorLocation()).GetSafeNormal();

            OtherChar->GetCharacterMovement()->AddImpulse(
                KnockDir * KnockbackStrength, true);
        }
    }

    // Reseteamos cooldown
    GetWorldTimerManager().SetTimer(
        DashCooldownHandle,
        [this]() { bCanDash = true; },
        DashCooldown,
        false);
}

// -----------------------------------------------
// MUERTE POR CAÍDA
// -----------------------------------------------

void ALSCharacter::FellOutOfWorld(const UDamageType& DmgType)
{
    if (GetLocalRole() == ROLE_Authority && !bIsDead)
    {
        LoseHeart();
    }
}

void ALSCharacter::LoseHeart()
{
    if (GetLocalRole() != ROLE_Authority) return;

    HeartsLeft = FMath::Max(0, HeartsLeft - 1);

    // Actualizamos PlayerState
    ALSPlayerState* PS =
        GetController()->GetPlayerState<ALSPlayerState>();
    if (PS) PS->SetLivesLeft(HeartsLeft);

    if (HeartsLeft <= 0)
    {
        HandleDeath();
    }
    else
    {
        // Respawn con delay
        HandleRespawn();
    }
}

void ALSCharacter::Server_ApplyKnockback_Implementation(
    ALSCharacter* Target,
    FVector KnockDirection,
    float ImpulseScale)
{
    ApplyKnockbackLogic(Target, KnockDirection, ImpulseScale);
}

void ALSCharacter::HandleDeath()
{
    if (GetLocalRole() != ROLE_Authority) return;

    bIsDead = true;
    Multicast_PlayDeathFX();

    ALSGameMode* GM =
        GetWorld()->GetAuthGameMode<ALSGameMode>();
    if (GM)
    {
        GM->PlayerDied(GetController(), nullptr);
    }
}

void ALSCharacter::HandleRespawn()
{
    if (GetLocalRole() != ROLE_Authority) return;

    // Ocultamos mientras esperamos
    bIsDead = true;
    Multicast_PlayDeathFX();

    GetWorldTimerManager().SetTimer(
        RespawnHandle,
        this,
        &ALSCharacter::DoRespawn,
        RespawnDelay,
        false);
}

void ALSCharacter::DoRespawn()
{
    if (GetLocalRole() != ROLE_Authority) return;

    // Buscamos un PlayerStart y teletransportamos
    AActor* SpawnPoint =
        UGameplayStatics::GetActorOfClass(
            GetWorld(), APlayerStart::StaticClass());

    if (SpawnPoint)
    {
        SetActorLocation(
            SpawnPoint->GetActorLocation() + FVector(0,0,100));
    }

    GetCharacterMovement()->StopMovementImmediately();
    bIsDead = false;
    Multicast_PlayRespawnFX();
}

// -----------------------------------------------
// MULTICAST
// -----------------------------------------------

void ALSCharacter::Multicast_PlayDeathFX_Implementation()
{
    // Ocultamos la pelota
    BallMesh->SetVisibility(false);
    BallCollision->SetCollisionEnabled(
        ECollisionEnabled::NoCollision);
    GetCapsuleComponent()->SetCollisionEnabled(
        ECollisionEnabled::NoCollision);
    GetCharacterMovement()->DisableMovement();
}

void ALSCharacter::Multicast_PlayRespawnFX_Implementation()
{
    // Mostramos la pelota de nuevo
    BallMesh->SetVisibility(true);
    BallCollision->SetCollisionEnabled(
        ECollisionEnabled::QueryOnly);
    GetCapsuleComponent()->SetCollisionEnabled(
        ECollisionEnabled::QueryAndPhysics);
    GetCharacterMovement()->SetMovementMode(MOVE_Walking);
}

void ALSCharacter::Multicast_PlayDashFX_Implementation()
{
    // Acá podés agregar partículas o sonido de dash
    UE_LOG(LogTemp, Log, TEXT("%s hizo dash"), *GetName());
}