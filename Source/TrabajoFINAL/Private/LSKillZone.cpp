#include "LSKillZone.h"
#include "Components/BoxComponent.h"
#include "LSCharacter.h"

ALSKillZone::ALSKillZone()
{
	PrimaryActorTick.bCanEverTick = false;

	KillVolume = CreateDefaultSubobject<UBoxComponent>(
		TEXT("KillVolume"));
	RootComponent = KillVolume;

	// Tamaño grande para cubrir todo el vacío
	KillVolume->SetBoxExtent(FVector(5000.f, 5000.f, 100.f));

	KillVolume->SetCollisionEnabled(
		ECollisionEnabled::QueryOnly);
	KillVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	KillVolume->SetCollisionResponseToChannel(
		ECC_Pawn, ECR_Overlap);
}

void ALSKillZone::BeginPlay()
{
	Super::BeginPlay();

	// Solo el servidor procesa las muertes
	if (HasAuthority())
	{
		KillVolume->OnComponentBeginOverlap.AddDynamic(
			this, &ALSKillZone::OnKillOverlap);
	}
}

void ALSKillZone::OnKillOverlap(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	ALSCharacter* Character =
		Cast<ALSCharacter>(OtherActor);

	if (Character && Character->IsAlive())
	{
		// Usamos LoseHeart en lugar de FellOutOfWorld
		// para no depender de que Unreal lo detecte
		Character->LoseHeart();
	}
}