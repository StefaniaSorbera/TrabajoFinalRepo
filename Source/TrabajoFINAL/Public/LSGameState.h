#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "LSGameState.generated.h"

UENUM(BlueprintType)
enum class EMatchState : uint8
{
    WaitingToStart  UMETA(DisplayName = "Waiting To Start"),
    InProgress      UMETA(DisplayName = "In Progress"),
    PostGame        UMETA(DisplayName = "Post Game")
};

UCLASS()
class TRABAJOFINAL_API ALSGameState : public AGameStateBase
{
    GENERATED_BODY()

public:
    ALSGameState();

    UPROPERTY(ReplicatedUsing = OnRep_MatchTime, BlueprintReadOnly, Category = "Match")
    float MatchTime = 0.f;

    UPROPERTY(Replicated, EditDefaultsOnly, Category = "Players")
    TArray<UMaterialInterface*> PlayerMaterials;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Match")
    EMatchState MatchState = EMatchState::WaitingToStart;

    UPROPERTY(ReplicatedUsing = OnRep_PlayersAlive, BlueprintReadOnly, Category = "Match")
    int32 PlayersAlive = 0;

    void SetMatchTime(float NewTime);
    void DecrementPlayersAlive();
    void SetMatchState(EMatchState NewState);

    UFUNCTION(BlueprintCallable)
    void SetPlayersAlive(int32 NewCount);

    virtual void AddPlayerState(APlayerState* PlayerState) override;

protected:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
    UFUNCTION()
    void OnRep_MatchTime();

    UFUNCTION()
    void OnRep_PlayersAlive();

    class ALSPlayerController* GetLocalPlayerController() const;
};
