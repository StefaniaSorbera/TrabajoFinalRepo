#include "LSCharacter.h"
#include "Net/UnrealNetwork.h"
#include "LSPlayerController.h"
#include "LSPlayerState.h"
#include "LSGameMode.h"
#include "LSGameState.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/KismetSystemLibrary.h"

ALSCharacter::ALSCharacter()
{
    bReplicates = true;
    PrimaryActorTick.bCanEverTick = true;

    BallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BallMesh"));
    BallMesh->SetupAttachment(RootComponent);
    BallMesh->SetRelativeScale3D(FVector(1.8f, 1.8f, 1.8f));
    BallMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    BallMesh->SetIsReplicated(true);

    BallCollision = CreateDefaultSubobject<USphereComponent>(TEXT("BallCollision"));
    BallCollision->SetupAttachment(RootComponent);
    BallCollision->SetSphereRadius(55.f);
    BallCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    BallCollision->SetCollisionObjectType(ECC_Pawn);
    BallCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
    BallCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    BallCollision->SetIsReplicated(true);

    GetCharacterMovement()->GroundFriction              = BallGroundFriction;
    GetCharacterMovement()->BrakingDecelerationWalking  = BallBrakingDeceleration;
    GetCharacterMovement()->BrakingFrictionFactor       = 1.f;
    GetCharacterMovement()->MaxAcceleration             = 800.f;
    GetCharacterMovement()->MaxWalkSpeed                = 800.f;

    GetMesh()->SetVisibility(false);
    GetCapsuleComponent()->SetCapsuleRadius(55.f);
    GetCapsuleComponent()->SetCapsuleHalfHeight(55.f);
}

void ALSCharacter::BeginPlay()
{
    Super::BeginPlay();

    BallCollision->OnComponentBeginOverlap.AddDynamic(this, &ALSCharacter::OnBallOverlap);

    if (IsLocallyControlled())
    {
        if (APlayerController* PC = Cast<APlayerController>(GetController()))
        {
            if (ACameraActor* LevelCam = Cast<ACameraActor>(
                UGameplayStatics::GetActorOfClass(GetWorld(), ACameraActor::StaticClass())))
            {
                PC->SetViewTargetWithBlend(LevelCam, 0.f);
            }
        }
    }

    FTimerHandle Handle;
    GetWorldTimerManager().SetTimer(Handle, [this]() { ApplyPlayerMaterial(); }, 0.5f, false);
}

void ALSCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);
    if (APlayerController* PC = Cast<APlayerController>(NewController))
        PC->SetControlRotation(GetActorRotation());
}

void ALSCharacter::OnRep_Controller()
{
    Super::OnRep_Controller();
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
        PC->SetControlRotation(GetActorRotation());
}

void ALSCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ALSCharacter, MaterialSlotIndex);
    DOREPLIFETIME(ALSCharacter, HeartsLeft);
    DOREPLIFETIME(ALSCharacter, bIsDead);
}

void ALSCharacter::ApplyPlayerMaterial()
{
    if (MaterialSlotIndex < 0) return;
    ALSGameState* GS = GetWorld()->GetGameState<ALSGameState>();
    if (!GS || !GS->PlayerMaterials.IsValidIndex(MaterialSlotIndex)) return;
    if (BallMesh)
        BallMesh->SetMaterial(0, GS->PlayerMaterials[MaterialSlotIndex]);
}

void ALSCharacter::OnRep_PlayerMaterial()
{
    FTimerHandle Handle;
    GetWorldTimerManager().SetTimer(Handle, [this]() { ApplyPlayerMaterial(); }, 0.5f, false);
}

void ALSCharacter::OnRep_PlayerColor()
{
    ApplyPlayerColor();
}

void ALSCharacter::ApplyPlayerColor()
{
    if (!BallMesh) return;
    if (UMaterialInstanceDynamic* DynMat = BallMesh->CreateAndSetMaterialInstanceDynamic(0))
        DynMat->SetVectorParameterValue(TEXT("Color"), PlayerColor);
}

ALSPlayerController* ALSCharacter::GetLocalPlayerController() const
{
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        ALSPlayerController* PC = Cast<ALSPlayerController>(It->Get());
        if (PC && PC->IsLocalPlayerController())
            return PC;
    }
    return nullptr;
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
    if (!OtherChar || !OtherChar->IsAlive() || !IsAlive() || !IsLocallyControlled()) return;

    FVector KnockDir = (OtherChar->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
    float Speed = FMath::Clamp(GetCharacterMovement()->Velocity.Size(), 300.f, 1500.f);
    float ImpulseScale = FMath::GetMappedRangeValueClamped(
        FVector2D(300.f, 1500.f),
        FVector2D(900.f, KnockbackStrength),
        Speed);

    if (HasAuthority())
        ApplyKnockbackLogic(OtherChar, KnockDir, ImpulseScale);
    else
    {
        Server_ApplyKnockback(OtherChar, KnockDir, ImpulseScale);
        GetCharacterMovement()->AddImpulse(FVector(-KnockDir.X, -KnockDir.Y, 0.f) * (ImpulseScale * 0.4f), true);
    }
}

void ALSCharacter::OnRep_HeartsLeft()
{
    ALSPlayerState* PS = GetPlayerState<ALSPlayerState>();
    if (!PS) return;
    if (ALSPlayerController* PC = GetLocalPlayerController())
        PC->UpdateHUDHearts(PS->HUDSlotIndex, HeartsLeft);
}

void ALSCharacter::OnRep_bIsDead()
{
    if (bIsDead)
    {
        BallMesh->SetVisibility(false);
        BallCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
    else
    {
        BallMesh->SetVisibility(true);
        BallCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    }
    GetMesh()->SetVisibility(false);
}

void ALSCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (!BallMesh) return;

    FVector Velocity = GetCharacterMovement()->Velocity;
    if (Velocity.SizeSquared() < 100.f) return;

    FVector RotAxis = FVector::CrossProduct(FVector::UpVector, Velocity.GetSafeNormal());
    float DegreesPerSecond = Velocity.Size() * 0.5f;
    BallMesh->AddWorldRotation(FQuat(RotAxis, FMath::DegreesToRadians(DegreesPerSecond * DeltaTime)));
}

void ALSCharacter::OnDashPressed()
{
    if (!bCanDash || bIsDead) return;
    Server_Dash();
}

bool ALSCharacter::Server_Dash_Validate() { return true; }

void ALSCharacter::Server_Dash_Implementation()
{
    if (!bCanDash || bIsDead) return;

    bCanDash = false;
    GetCharacterMovement()->AddImpulse(GetActorForwardVector().GetSafeNormal() * DashImpulseStrength, true);
    Multicast_PlayDashFX();

    TArray<AActor*> NearbyActors;
    UKismetSystemLibrary::SphereOverlapActors(
        GetWorld(), GetActorLocation(), KnockbackRadius,
        TArray<TEnumAsByte<EObjectTypeQuery>>(),
        ALSCharacter::StaticClass(),
        TArray<AActor*>{ this },
        NearbyActors);

    for (AActor* Actor : NearbyActors)
    {
        ALSCharacter* OtherChar = Cast<ALSCharacter>(Actor);
        if (OtherChar && OtherChar->IsAlive())
        {
            FVector KnockDir = (OtherChar->GetActorLocation() - GetActorLocation()).GetSafeNormal();
            OtherChar->GetCharacterMovement()->AddImpulse(
                FVector(KnockDir.X, KnockDir.Y, 0.f) * KnockbackStrength, true);
        }
    }

    GetWorldTimerManager().SetTimer(DashCooldownHandle, [this]() { bCanDash = true; }, DashCooldown, false);
}

void ALSCharacter::FellOutOfWorld(const UDamageType& DmgType)
{
    if (GetLocalRole() == ROLE_Authority && !bIsDead)
        LoseHeart();
}

void ALSCharacter::LoseHeart()
{
    if (GetLocalRole() != ROLE_Authority || bIsInvincible) return;

    HeartsLeft = FMath::Max(0, HeartsLeft - 1);

    ALSPlayerState* PS = GetController()->GetPlayerState<ALSPlayerState>();
    if (PS) PS->SetLivesLeft(HeartsLeft);

    int32 PlayerIdx = PS ? PS->HUDSlotIndex : 0;
    Multicast_UpdateHearts(PlayerIdx, HeartsLeft);

    if (HeartsLeft <= 0)
        HandleDeath();
    else
        HandleRespawn();
}

void ALSCharacter::Multicast_UpdateHearts_Implementation(int32 PlayerIdx, int32 NewHearts)
{
    if (ALSPlayerController* PC = GetLocalPlayerController())
        PC->UpdateHUDHearts(PlayerIdx, NewHearts);
}

void ALSCharacter::HandleDeath()
{
    if (GetLocalRole() != ROLE_Authority) return;
    bIsDead = true;
    Multicast_PlayDeathFX();
    if (ALSGameMode* GM = GetWorld()->GetAuthGameMode<ALSGameMode>())
        GM->PlayerDied(GetController(), nullptr);
}

void ALSCharacter::HandleRespawn()
{
    if (GetLocalRole() != ROLE_Authority) return;
    bIsDead = true;
    Multicast_PlayDeathFX();
    if (ALSPlayerController* PC = Cast<ALSPlayerController>(GetController()))
        PC->Client_ShowDeathScreen();
    GetWorldTimerManager().SetTimer(RespawnHandle, this, &ALSCharacter::DoRespawn, RespawnDelay, false);
}

void ALSCharacter::DoRespawn()
{
    if (GetLocalRole() != ROLE_Authority) return;

    ALSPlayerState* PS = GetController()->GetPlayerState<ALSPlayerState>();
    int32 SlotIndex = PS ? PS->HUDSlotIndex : 0;
    FString SpawnTag = FString::Printf(TEXT("Spawn%d"), SlotIndex);

    TArray<AActor*> SpawnPoints;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), SpawnPoints);

    FVector SpawnLocation = FVector(0.f, 0.f, 300.f);
    for (AActor* SpawnPoint : SpawnPoints)
    {
        if (SpawnPoint->ActorHasTag(FName(*SpawnTag)))
        {
            SpawnLocation = SpawnPoint->GetActorLocation() + FVector(0.f, 0.f, 100.f);
            break;
        }
    }

    SetActorLocation(SpawnLocation);
    GetCharacterMovement()->StopMovementImmediately();
    bIsDead = false;
    bIsInvincible = true;
    GetWorldTimerManager().SetTimer(InvincibilityHandle, [this]() { bIsInvincible = false; }, InvincibilityDuration, false);
    Multicast_PlayRespawnFX();

    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        if (ACameraActor* LevelCam = Cast<ACameraActor>(
            UGameplayStatics::GetActorOfClass(GetWorld(), ACameraActor::StaticClass())))
        {
            PC->SetViewTargetWithBlend(LevelCam, 0.f);
        }
    }
}

void ALSCharacter::Multicast_PlayDeathFX_Implementation()
{
    BallMesh->SetVisibility(false);
    BallCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetCharacterMovement()->DisableMovement();
    GetMesh()->SetVisibility(false);
}

void ALSCharacter::Multicast_PlayRespawnFX_Implementation()
{
    BallMesh->SetVisibility(true);
    BallCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    GetCharacterMovement()->SetMovementMode(MOVE_Walking);
    GetMesh()->SetVisibility(false);
}

void ALSCharacter::Multicast_PlayDashFX_Implementation()
{
}

void ALSCharacter::ApplyKnockbackLogic(ALSCharacter* OtherChar, const FVector& KnockDir, float ImpulseScale)
{
    if (!OtherChar || !HasAuthority()) return;
    OtherChar->GetCharacterMovement()->AddImpulse(KnockDir * ImpulseScale, true);
    GetCharacterMovement()->AddImpulse(-KnockDir * (ImpulseScale * 10.0f), true);
}

bool ALSCharacter::Server_ApplyKnockback_Validate(ALSCharacter* OtherChar, const FVector& KnockDir, float ImpulseScale) { return true; }

void ALSCharacter::Server_ApplyKnockback_Implementation(ALSCharacter* OtherChar, const FVector& KnockDir, float ImpulseScale)
{
    ApplyKnockbackLogic(OtherChar, KnockDir, ImpulseScale);
}
