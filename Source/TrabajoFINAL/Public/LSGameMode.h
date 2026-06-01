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

	void CheckVictoryCondition();
	void EndMatch(AController* Winner);
	void OnMatchTimeUp();

	// Tiempo límite en segundos
	UPROPERTY(EditDefaultsOnly, Category = "Match")
	float MatchDuration = 180.f;

	FTimerHandle MatchTimerHandle;
	FTimerHandle MatchTickHandle;

	void RestartMatch();

	FTimerHandle RestartTimerHandle;

	UPROPERTY(EditDefaultsOnly, Category = "Match")
	float RestartDelay = 5.f;
};