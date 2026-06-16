#include "LSKillZone.h"
#include "Components/BoxComponent.h"
#include "LSCharacter.h"

ALSKillZone::ALSKillZone()
{
    PrimaryActorTick.bCanEverTick = false;

    KillVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("KillVolume"));
    RootComponent = KillVolume;
    KillVolume->SetBoxExtent(FVector(5000.f, 5000.f, 100.f));
    KillVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    KillVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
    KillVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void ALSKillZone::BeginPlay()
{
    Super::BeginPlay();
    if (HasAuthority())
        KillVolume->OnComponentBeginOverlap.AddDynamic(this, &ALSKillZone::OnKillOverlap);
}

void ALSKillZone::OnKillOverlap(
    UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    ALSCharacter* Character = Cast<ALSCharacter>(OtherActor);
    if (Character && Character->IsAlive())
        Character->LoseHeart();
}
