#pragma once

#include "CoreMinimal.h"
#include "TrabajoFINALPlayerController.h"
#include "LSDeathWidget.h"
#include "LSPlayerController.generated.h"

class ULSHUDWidget;
class ULSEndGameWidget;
class ULSDeathWidget;

UCLASS()
class TRABAJOFINAL_API ALSPlayerController : public ATrabajoFINALPlayerController
{
    GENERATED_BODY()

public:
    ALSPlayerController();

    bool bHUDInitialized = false;

    UPROPERTY(EditDefaultsOnly, Category = "HUD")
    TSubclassOf<ULSDeathWidget> DeathWidgetClass;

    UFUNCTION(BlueprintCallable, Category = "HUD")
    void UpdateHUDHearts(int32 PlayerIdx, int32 HeartsLeft);

    UFUNCTION(BlueprintCallable, Category = "HUD")
    void UpdateHUDPlayersAlive(int32 PlayersAlive);

    UFUNCTION(BlueprintCallable, Category = "HUD")
    void UpdateHUDKills(int32 PlayerIdx, int32 Kills);

    UFUNCTION(BlueprintCallable, Category = "HUD")
    void UpdateHUDTimer(float NewTime);

    void InitializeHUD();

    UFUNCTION(Client, Reliable)
    void ClientForceRotation(FRotator NewRotation);

    UFUNCTION(Client, Reliable)
    void Client_UpdatePlayerHearts(int32 SlotIndex, int32 HeartsLeft);

    UFUNCTION(Client, Reliable)
    void Client_ShowVictory();

    UFUNCTION(Client, Reliable)
    void Client_ShowDefeat();

    UFUNCTION(Client, Reliable)
    void Client_ShowDeathScreen();

    UFUNCTION(Client, Reliable)
    void Client_StartRestartCountdown(float Seconds);

    UFUNCTION(Client, Reliable)
    void Client_InitializeHUD();

    UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
    void BP_UpdateMatchTime(float NewTime);

    UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
    void BP_UpdateKillCount(int32 NewKills);

    UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
    void BP_UpdateLivesLeft(int32 NewLives);

    UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
    void BP_ShowVictoryScreen();

    UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
    void BP_ShowDefeatScreen();

    UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
    void BP_ShowDeathScreen();

    UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
    void BP_StartRestartCountdown(float Seconds);

protected:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;

    UPROPERTY(BlueprintReadOnly, Category = "HUD")
    ULSHUDWidget* HUDWidget;

    UPROPERTY(BlueprintReadOnly, Category = "HUD")
    ULSEndGameWidget* EndGameWidget;

    UPROPERTY(BlueprintReadOnly, Category = "HUD")
    ULSDeathWidget* DeathWidget;

    UPROPERTY(EditDefaultsOnly, Category = "HUD")
    TSubclassOf<ULSHUDWidget> HUDWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "HUD")
    TSubclassOf<ULSEndGameWidget> EndGameWidgetClass;

private:
    ULSEndGameWidget* CreateAndShowEndGameWidget();
};
