#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LSGameMode.generated.h"

UCLASS()
class TRABAJOFINAL_API ALSGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ALSGameMode();

	// Llamado desde LSCharacter::HandleDeath
	void PlayerDied(AController* DeadPlayer,
					AController* Killer);

protected:
	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual AActor* FindPlayerStart_Implementation(
		AController* Player,
		const FString& IncomingName) override;
	void CheckVictoryCondition();
	void AssignHUDSlots();
	UPROPERTY(EditDefaultsOnly, Category = "Players")
	TArray<UMaterialInterface*> PlayerMaterials;
	void EndMatch(AController* Winner);
	void OnMatchTimeUp();

	// Tiempo límite en segundos
	UPROPERTY(EditDefaultsOnly, Category = "Match")
	float MatchDuration = 180.f;

	FTimerHandle MatchTimerHandle;
	FTimerHandle MatchTickHandle;

	void RestartMatch();
	
	FTimerHandle AssignSlotsHandle;
	FTimerHandle RestartTimerHandle;

	UPROPERTY(EditDefaultsOnly, Category = "Match")
	float RestartDelay = 5.f;
};