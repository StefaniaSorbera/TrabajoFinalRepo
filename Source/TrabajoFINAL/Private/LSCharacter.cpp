#include "LSCharacter.h"
#include "Net/UnrealNetwork.h"
#include "LSPlayerController.h"
#include "LSPlayerState.h"
#include "LSGameMode.h"
#include "LSRespawnPoint.h"
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
    
    // --- Movimiento tipo pelota (menos hielo) ---
    GetCharacterMovement()->GroundFriction = 8.f;          // default 8, subilo si frena muy rápido
    GetCharacterMovement()->BrakingDecelerationWalking = 500.f;  // default 2048 — esto es lo que los frena en seco
    GetCharacterMovement()->BrakingFrictionFactor = 1.f;   // default 2 — reduce el freno al soltar
    GetCharacterMovement()->MaxAcceleration = 800.f;       // default 2048 — aceleración más gradual
    GetCharacterMovement()->MaxWalkSpeed = 800.f;          // ajustá a gusto

    // Ocultamos el Skeletal Mesh del template
    GetMesh()->SetVisibility(false);

    // Ajustamos la cápsula al tamaño de la pelota
    GetCapsuleComponent()->SetCapsuleRadius(55.f);
    GetCapsuleComponent()->SetCapsuleHalfHeight(55.f);
    
    // En el constructor, después de crearlos:
    GetCharacterMovement()->BrakingDecelerationWalking = BallBrakingDeceleration;
    GetCharacterMovement()->GroundFriction = BallGroundFriction;
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

        // --- NUEVO: cámara del nivel ---
        if (IsLocallyControlled())
        {
            APlayerController* PC =
                Cast<APlayerController>(GetController());

            if (PC)
            {
                ACameraActor* LevelCam =
                    Cast<ACameraActor>(
                        UGameplayStatics::GetActorOfClass(
                            GetWorld(), ACameraActor::StaticClass()));

                if (LevelCam)
                {
                    PC->SetViewTargetWithBlend(
                        LevelCam, 0.f);
                }
            }
        }
    }

void ALSCharacter::OnBallOverlap(
    UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    ALSCharacter* OtherChar = Cast<ALSCharacter>(OtherActor);
    if (!OtherChar || !OtherChar->IsAlive() || !IsAlive()) return;

    // AGREGAR: evitar que se ejecute dos veces en host
    // (una vez por cada pelota que detecta el overlap)
    if (!IsLocallyControlled()) return;

    FVector KnockDir = (OtherChar->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();

    float Speed = FMath::Clamp(GetCharacterMovement()->Velocity.Size(), 300.f, 1500.f);
    float ImpulseScale = FMath::GetMappedRangeValueClamped(
       FVector2D(300.f, 1500.f),
       FVector2D(900.f, KnockbackStrength),  // mínimo: 600 → 900
       Speed);

    if (HasAuthority())
    {
        ApplyKnockbackLogic(OtherChar, KnockDir, ImpulseScale);
    }
    else
    {
        // RPC al servidor
        Server_ApplyKnockback(OtherChar, KnockDir, ImpulseScale);

        // Predicción local del rebote propio
        GetCharacterMovement()->AddImpulse(
            -KnockDir * (ImpulseScale * 0.4f) + FVector(0.f, 0.f, 200.f), true);
    }
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
    // Obtenemos el slot del jugador dueño de este Character
    ALSPlayerState* PS = GetPlayerState<ALSPlayerState>();
    if (!PS) return;

    int32 PlayerIdx = PS->HUDSlotIndex;

    // Buscamos el controller local del cliente
    // (puede ser cualquier jugador, no necesariamente el dueño)
    for (FConstPlayerControllerIterator It =
        GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        ALSPlayerController* PC =
            Cast<ALSPlayerController>(It->Get());

        // IsLocalPlayerController verifica que sea
        // el controller que corre en ESTE cliente
        if (PC && PC->IsLocalPlayerController())
        {
            PC->UpdateHUDHearts(PlayerIdx, HeartsLeft);
            break;
        }
    }
}

void ALSCharacter::OnRep_bIsDead()
{
    if (bIsDead)
    {
        BallMesh->SetVisibility(false);
        BallCollision->SetCollisionEnabled(
            ECollisionEnabled::NoCollision);
    }
    else
    {
        BallMesh->SetVisibility(true);
        BallCollision->SetCollisionEnabled(
            ECollisionEnabled::QueryOnly);
    }

    // Nos aseguramos que el SkeletalMesh
    // del template SIEMPRE esté oculto
    GetMesh()->SetVisibility(false);
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
    if (bIsInvincible) return;

    HeartsLeft = FMath::Max(0, HeartsLeft - 1);

    ALSPlayerState* PS =
        GetController()->GetPlayerState<ALSPlayerState>();
    if (PS) PS->SetLivesLeft(HeartsLeft);

    // Llamamos manualmente para el servidor/host
    // pasamos el índice correcto
    int32 PlayerIdx = PS ? PS->HUDSlotIndex : 0;

    // Actualizamos en todos los controllers locales
    for (FConstPlayerControllerIterator It =
        GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        ALSPlayerController* PC =
            Cast<ALSPlayerController>(It->Get());
        if (PC && PC->IsLocalController())
        {
            PC->UpdateHUDHearts(PlayerIdx, HeartsLeft);
        }
    }

    if (HeartsLeft <= 0)
    {
        HandleDeath();
    }
    else
    {
        HandleRespawn();
    }
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

    bIsDead = true;
    Multicast_PlayDeathFX();

    ALSPlayerController* PC =
        Cast<ALSPlayerController>(GetController());

    UE_LOG(LogTemp, Warning,
        TEXT("HandleRespawn — PC: %s | DeathWidgetClass: %s"),
        PC ? *PC->GetName() : TEXT("NULL"),
        PC && PC->DeathWidgetClass ?
            TEXT("ASIGNADO") : TEXT("NULL"));

    if (PC)
    {
        PC->Client_ShowDeathScreen();
    }

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

    ALSPlayerState* PS =
        GetController()->GetPlayerState<ALSPlayerState>();
    int32 SlotIndex = PS ? PS->HUDSlotIndex : 0;

    UE_LOG(LogTemp, Warning,
        TEXT("DoRespawn — Player=%s SlotIndex=%d"),
        PS ? *PS->GetPlayerName() : TEXT("NULL"),
        SlotIndex);

    FString SpawnTag = FString::Printf(TEXT("Spawn%d"), SlotIndex);

    TArray<AActor*> SpawnPoints;
    UGameplayStatics::GetAllActorsOfClass(
        GetWorld(),
        APlayerStart::StaticClass(),
        SpawnPoints);

    FVector SpawnLocation = FVector(0.f, 0.f, 300.f);
    bool bFoundSpawn = false;

    for (AActor* SpawnPoint : SpawnPoints)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("SpawnPoint encontrado: %s — Tags: %d"),
            *SpawnPoint->GetName(),
            SpawnPoint->Tags.Num());

        for (FName Tag : SpawnPoint->Tags)
        {
            UE_LOG(LogTemp, Warning,
                TEXT("  Tag: %s"), *Tag.ToString());
        }

        if (SpawnPoint->ActorHasTag(FName(*SpawnTag)))
        {
            SpawnLocation =
                SpawnPoint->GetActorLocation() +
                FVector(0.f, 0.f, 100.f);
            bFoundSpawn = true;

            UE_LOG(LogTemp, Warning,
                TEXT("Spawn encontrado: %s para Slot %d"),
                *SpawnPoint->GetName(), SlotIndex);
            break;
        }
    }

    if (!bFoundSpawn)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("NO se encontró spawn para tag: %s"),
            *SpawnTag);
    }

    // resto del código igual...
    SetActorLocation(SpawnLocation);
    GetCharacterMovement()->StopMovementImmediately();
    bIsDead = false;
    bIsInvincible = true;
    GetWorldTimerManager().SetTimer(
        InvincibilityHandle,
        [this]() { bIsInvincible = false; },
        InvincibilityDuration, false);
    Multicast_PlayRespawnFX();

    APlayerController* PC =
        Cast<APlayerController>(GetController());
    if (PC)
    {
        ACameraActor* LevelCam = Cast<ACameraActor>(
            UGameplayStatics::GetActorOfClass(
                GetWorld(), ACameraActor::StaticClass()));
        if (LevelCam)
            PC->SetViewTargetWithBlend(LevelCam, 0.f);
    }
}

// -----------------------------------------------
// MULTICAST
// -----------------------------------------------

void ALSCharacter::Multicast_PlayDeathFX_Implementation()
{
    BallMesh->SetVisibility(false);
    BallCollision->SetCollisionEnabled(
        ECollisionEnabled::NoCollision);
    GetCapsuleComponent()->SetCollisionEnabled(
        ECollisionEnabled::NoCollision);
    GetCharacterMovement()->DisableMovement();

    // Siempre oculto
    GetMesh()->SetVisibility(false);
}

void ALSCharacter::Multicast_PlayRespawnFX_Implementation()
{
    BallMesh->SetVisibility(true);
    BallCollision->SetCollisionEnabled(
        ECollisionEnabled::QueryOnly);
    GetCapsuleComponent()->SetCollisionEnabled(
        ECollisionEnabled::QueryAndPhysics);
    GetCharacterMovement()->SetMovementMode(MOVE_Walking);

    // Siempre oculto
    GetMesh()->SetVisibility(false);
}

void ALSCharacter::Multicast_PlayDashFX_Implementation()
{
    // Acá podés agregar partículas o sonido de dash
    UE_LOG(LogTemp, Log, TEXT("%s hizo dash"), *GetName());
}

void ALSCharacter::ApplyKnockbackLogic(ALSCharacter* OtherChar, const FVector& KnockDir, float ImpulseScale)
{
    if (!OtherChar || !HasAuthority()) return;

    // Impactado — impulso fuerte solo en XY
    OtherChar->GetCharacterMovement()->AddImpulse(
        KnockDir * ImpulseScale, true);

    // El que pega — rebote suave solo en XY
    GetCharacterMovement()->AddImpulse(
        -KnockDir * (ImpulseScale * 10.2f), true);
}

bool ALSCharacter::Server_ApplyKnockback_Validate(ALSCharacter* OtherChar, const FVector& KnockDir, float ImpulseScale)
{
    return true;
}

void ALSCharacter::Server_ApplyKnockback_Implementation(ALSCharacter* OtherChar, const FVector& KnockDir, float ImpulseScale)
{
    ApplyKnockbackLogic(OtherChar, KnockDir, ImpulseScale);
}