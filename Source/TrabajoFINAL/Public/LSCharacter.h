#pragma once

#include "CoreMinimal.h"
#include "TrabajoFINALCharacter.h" // <- padre heredado del template
#include "LSCharacter.generated.h"

UCLASS()
class TRABAJOFINAL_API ALSCharacter : public ATrabajoFINALCharacter
{
	GENERATED_BODY()

public:
	// --- Salud ---
	UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth,
		BlueprintReadOnly, Category = "Health")
	float CurrentHealth = 100.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health")
	float MaxHealth = 100.f;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "State")
	bool bIsDead = false;

	void TakeDamageFromAttacker(float DamageAmount, AController* Attacker);

	bool IsAlive() const { return !bIsDead; }

	// --- Server RPC ---
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Fire(FVector_NetQuantize TraceStart,
					 FVector_NetQuantize TraceEnd);

	// --- Multicast RPCs ---
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayDeathFX();

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayHitFX(FVector HitLocation);

protected:
	UFUNCTION()
	void OnRep_CurrentHealth();

	void HandleDeath(AController* Killer);

	// Disparo — se llama desde el input del template
	void OnFirePressed();

	virtual void FellOutOfWorld(
		const UDamageType& DmgType) override;

	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	float FireRange = 2000.f;
};