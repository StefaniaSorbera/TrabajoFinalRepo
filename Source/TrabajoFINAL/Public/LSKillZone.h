#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LSKillZone.generated.h"

UCLASS()
class TRABAJOFINAL_API ALSKillZone : public AActor
{
	GENERATED_BODY()

public:
	ALSKillZone();
	void BeginPlay();

protected:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	class UBoxComponent* KillVolume;

	UFUNCTION()
	void OnKillOverlap(UPrimitiveComponent* OverlappedComp,
					   AActor* OtherActor,
					   UPrimitiveComponent* OtherComp,
					   int32 OtherBodyIndex,
					   bool bFromSweep,
					   const FHitResult& SweepResult);
};