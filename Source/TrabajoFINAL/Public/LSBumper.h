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

	// Colisión que detecta el overlap
	UPROPERTY(VisibleAnywhere, Category = "Components")
	class UBoxComponent* BumperVolume;

	// Mesh visual del bumper
	UPROPERTY(VisibleAnywhere, Category = "Components")
	class UStaticMeshComponent* BumperMesh;

	// Fuerza del impulso
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper")
	float ImpulseStrength = 1500.f;

	// Fuerza vertical del impulso
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
	// Escala original del mesh
	FVector DefaultScale;

	// Escala al recibir impacto
	UPROPERTY(EditAnywhere, Category = "Bumper")
	FVector HitScale = FVector(1.3f, 1.3f, 0.7f);

	// Duración del efecto
	UPROPERTY(EditAnywhere, Category = "Bumper")
	float HitEffectDuration = 0.15f;

	FTimerHandle HitEffectHandle;

	void PlayHitEffect();
	void ResetScale();
};