#pragma once

#include "CoreMinimal.h"
#include "TrabajoFINALPlayerController.h"
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

    // --- Client RPCs ---
    UFUNCTION(Client, Reliable)
    void Client_ShowVictory();

    UFUNCTION(Client, Reliable)
    void Client_ShowDefeat();

    UFUNCTION(Client, Reliable)
    void Client_ShowDeathScreen();

    UFUNCTION(Client, Reliable)
    void Client_StartRestartCountdown(float Seconds);

    // --- BlueprintImplementableEvents ---
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

    // --- Widgets ---
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

    UPROPERTY(EditDefaultsOnly, Category = "HUD")
    TSubclassOf<ULSDeathWidget> DeathWidgetClass;
};