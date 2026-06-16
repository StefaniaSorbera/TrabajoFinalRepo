#include "LSBumper.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "LSCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

ALSBumper::ALSBumper()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

    BumperMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BumperMesh"));
    RootComponent = BumperMesh;
    BumperMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    BumperVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("BumperVolume"));
    BumperVolume->SetupAttachment(RootComponent);
    BumperVolume->SetBoxExtent(FVector(50.f, 50.f, 100.f));
    BumperVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    BumperVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
    BumperVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void ALSBumper::BeginPlay()
{
    Super::BeginPlay();
    DefaultScale = BumperMesh->GetRelativeScale3D();
    if (HasAuthority())
        BumperVolume->OnComponentBeginOverlap.AddDynamic(this, &ALSBumper::OnBumperOverlap);
}

void ALSBumper::OnBumperOverlap(
    UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    ALSCharacter* Character = Cast<ALSCharacter>(OtherActor);
    if (!Character || !Character->IsAlive()) return;

    FVector CharLoc = Character->GetActorLocation();
    FVector ClosestPoint;
    BumperVolume->GetClosestPointOnCollision(CharLoc, ClosestPoint);

    FVector SurfaceNormal = (CharLoc - ClosestPoint).GetSafeNormal();
    if (SurfaceNormal.IsNearlyZero())
        SurfaceNormal = (CharLoc - GetActorLocation()).GetSafeNormal();

    SurfaceNormal.Z = 0.f;
    SurfaceNormal.Normalize();

    FVector Velocity = Character->GetCharacterMovement()->Velocity;
    FVector Reflected = Velocity - 2.f * FVector::DotProduct(Velocity, SurfaceNormal) * SurfaceNormal;

    if (Reflected.Size2D() < 600.f)
    {
        FVector Dir = Reflected.GetSafeNormal2D();
        Reflected = FVector(Dir.X * 600.f, Dir.Y * 600.f, 0.f);
    }

    FVector FinalImpulse = Reflected.GetSafeNormal2D() * ImpulseStrength;
    FinalImpulse.Z = 0.f;

    Character->GetCharacterMovement()->Velocity = FVector::ZeroVector;
    Character->GetCharacterMovement()->AddImpulse(FinalImpulse, true);

    Multicast_PlayHitEffect();
}

void ALSBumper::Multicast_PlayHitEffect_Implementation()
{
    BumperMesh->SetRelativeScale3D(HitScale);
    GetWorldTimerManager().SetTimer(HitEffectHandle, this, &ALSBumper::ResetScale, HitEffectDuration, false);
}

void ALSBumper::ResetScale()
{
    BumperMesh->SetRelativeScale3D(DefaultScale);
}
