#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LSBumper.generated.h"

UCLASS()
class TRABAJOFINAL_API ALSBumper : public AActor
{
    GENERATED_BODY()

public:
    ALSBumper();

    UPROPERTY(VisibleAnywhere, Category = "Components")
    class UBoxComponent* BumperVolume;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    class UStaticMeshComponent* BumperMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper")
    float ImpulseStrength = 1500.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper")
    float VerticalImpulse = 400.f;

protected:
    virtual void BeginPlay() override;

    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_PlayHitEffect();

    UFUNCTION()
    void OnBumperOverlap(
        UPrimitiveComponent* OverlappedComp,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult);

    UPROPERTY(EditAnywhere, Category = "Bumper")
    FVector HitScale = FVector(1.3f, 1.3f, 0.7f);

    UPROPERTY(EditAnywhere, Category = "Bumper")
    float HitEffectDuration = 0.15f;

    FVector DefaultScale;
    FTimerHandle HitEffectHandle;

    void ResetScale();
};
