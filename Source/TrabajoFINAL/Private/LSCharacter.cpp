#include "LSCharacter.h"
#include "Net/UnrealNetwork.h"
#include "LSPlayerController.h"
#include "LSPlayerState.h"
#include "LSGameMode.h"
#include "DrawDebugHelpers.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

void ALSCharacter::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ALSCharacter, CurrentHealth);
    DOREPLIFETIME(ALSCharacter, bIsDead);
}

void ALSCharacter::OnRep_CurrentHealth()
{
    ALSPlayerController* PC =
        Cast<ALSPlayerController>(GetController());
    if (PC && PC->IsLocalController())
    {
        UE_LOG(LogTemp, Log,
            TEXT("Salud: %.1f"), CurrentHealth);
        // Acá llamaremos al HUD cuando lo hagamos
    }
}

// -----------------------------------------------
// DISPARO
// -----------------------------------------------

void ALSCharacter::OnFirePressed()
{
    if (bIsDead) return;

    FVector TraceStart = GetActorLocation() + FVector(0, 0, 64.f);
    FVector TraceEnd   = TraceStart +
                         GetControlRotation().Vector() * FireRange;

    Server_Fire(TraceStart, TraceEnd);
}

bool ALSCharacter::Server_Fire_Validate(
    FVector_NetQuantize TraceStart,
    FVector_NetQuantize TraceEnd)
{
    return FVector::Dist(TraceStart, TraceEnd) <= (FireRange * 1.1f);
}

void ALSCharacter::Server_Fire_Implementation(
    FVector_NetQuantize TraceStart,
    FVector_NetQuantize TraceEnd)
{
    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        Hit, TraceStart, TraceEnd, ECC_Pawn, Params);

    DrawDebugLine(GetWorld(), TraceStart, TraceEnd,
        bHit ? FColor::Red : FColor::Green, false, 2.f);

    if (bHit)
    {
        ALSCharacter* HitChar =
            Cast<ALSCharacter>(Hit.GetActor());
        if (HitChar && HitChar->IsAlive())
        {
            HitChar->Multicast_PlayHitFX(Hit.ImpactPoint);
            HitChar->TakeDamageFromAttacker(34.f, GetController());
        }
    }
}

// -----------------------------------------------
// DAÑO Y MUERTE
// -----------------------------------------------

void ALSCharacter::TakeDamageFromAttacker(
    float DamageAmount, AController* Attacker)
{
    if (GetLocalRole() != ROLE_Authority) return;
    if (bIsDead) return;

    CurrentHealth = FMath::Clamp(
        CurrentHealth - DamageAmount, 0.f, MaxHealth);

    if (CurrentHealth <= 0.f)
    {
        HandleDeath(Attacker);
    }
}

void ALSCharacter::HandleDeath(AController* Killer)
{
    if (GetLocalRole() != ROLE_Authority) return;

    bIsDead = true;
    Multicast_PlayDeathFX();

    ALSGameMode* GM =
        GetWorld()->GetAuthGameMode<ALSGameMode>();
    if (GM)
    {
        GM->PlayerDied(GetController(), Killer);
    }
}

void ALSCharacter::FellOutOfWorld(const UDamageType& DmgType)
{
    if (GetLocalRole() == ROLE_Authority && !bIsDead)
    {
        HandleDeath(nullptr);
    }
}

// -----------------------------------------------
// MULTICAST
// -----------------------------------------------

void ALSCharacter::Multicast_PlayDeathFX_Implementation()
{
    GetCapsuleComponent()->SetCollisionEnabled(
        ECollisionEnabled::NoCollision);
    GetCharacterMovement()->DisableMovement();
    GetMesh()->SetSimulatePhysics(true);
}

void ALSCharacter::Multicast_PlayHitFX_Implementation(
    FVector HitLocation)
{
    UE_LOG(LogTemp, Log,
        TEXT("Hit en: %s"), *HitLocation.ToString());
}