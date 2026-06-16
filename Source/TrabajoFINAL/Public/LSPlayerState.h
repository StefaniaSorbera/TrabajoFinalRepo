#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "LSPlayerState.generated.h"

UENUM(BlueprintType)
enum class EPlayerStatus : uint8
{
    Alive       UMETA(DisplayName = "Alive"),
    Dead        UMETA(DisplayName = "Dead"),
    Spectator   UMETA(DisplayName = "Spectator")
};

UCLASS()
class TRABAJOFINAL_API ALSPlayerState : public APlayerState
{
    GENERATED_BODY()

public:
    ALSPlayerState();

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player")
    int32 KillCount = 0;

    UPROPERTY(ReplicatedUsing = OnRep_HUDSlotIndex, BlueprintReadOnly, Category = "Player")
    int32 HUDSlotIndex = -1;

    UPROPERTY(ReplicatedUsing = OnRep_LivesLeft, BlueprintReadOnly, Category = "Player")
    int32 LivesLeft = 3;

    UPROPERTY(ReplicatedUsing = OnRep_PlayerStatus, BlueprintReadOnly, Category = "Player")
    EPlayerStatus PlayerStatus = EPlayerStatus::Alive;

    void AddKill();
    void SetPlayerDead();
    void SetPlayerAlive();
    void SetLivesLeft(int32 NewLives);
    bool IsAlive() const;

protected:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
    UFUNCTION()
    void OnRep_LivesLeft();

    UFUNCTION()
    void OnRep_HUDSlotIndex();

    UFUNCTION()
    void OnRep_PlayerStatus();

    class ALSPlayerController* GetLocalPlayerController() const;
};
