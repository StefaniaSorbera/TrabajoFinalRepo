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

    void PlayerDied(AController* DeadPlayer, AController* Killer);

protected:
    virtual void BeginPlay() override;
    virtual void PostLogin(APlayerController* NewPlayer) override;
    virtual AActor* FindPlayerStart_Implementation(AController* Player, const FString& IncomingName) override;

    UPROPERTY(EditDefaultsOnly, Category = "Players")
    TArray<UMaterialInterface*> PlayerMaterials;

    UPROPERTY(EditDefaultsOnly, Category = "Match")
    float MatchDuration = 180.f;

    UPROPERTY(EditDefaultsOnly, Category = "Match")
    float RestartDelay = 5.f;

    FTimerHandle MatchTickHandle;
    FTimerHandle AssignSlotsHandle;
    FTimerHandle RestartTimerHandle;

private:
    void CheckVictoryCondition();
    void AssignHUDSlots();
    void EndMatch(AController* Winner);
    void OnMatchTimeUp();
    void RestartMatch();
};
