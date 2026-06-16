#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LSHUDWidget.generated.h"

class ULSPlayerHUDWidget;

UCLASS()
class TRABAJOFINAL_API ULSHUDWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(meta = (BindWidget))
    ULSPlayerHUDWidget* Player1HUD;

    UPROPERTY(meta = (BindWidget))
    ULSPlayerHUDWidget* Player2HUD;

    UPROPERTY(meta = (BindWidget))
    ULSPlayerHUDWidget* Player3HUD;

    UPROPERTY(meta = (BindWidget))
    ULSPlayerHUDWidget* Player4HUD;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* TXT_PlayersAlive;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* TXT_Timer;

    UFUNCTION(BlueprintCallable, Category = "HUD")
    ULSPlayerHUDWidget* GetPlayerHUD(int32 PlayerIdx);

    UFUNCTION(BlueprintCallable, Category = "HUD")
    void UpdateMatchTime(float NewTime);

    UFUNCTION(BlueprintCallable, Category = "HUD")
    void SetPlayerSlotVisible(int32 PlayerIdx, bool bVisible);

    UFUNCTION(BlueprintCallable, Category = "HUD")
    void UpdatePlayersAlive(int32 PlayersAlive);

    UFUNCTION(BlueprintCallable, Category = "HUD")
    void UpdatePlayerHearts(int32 PlayerIdx, int32 HeartsLeft);

    UFUNCTION(BlueprintCallable, Category = "HUD")
    void UpdatePlayerKills(int32 PlayerIdx, int32 Kills);

    UFUNCTION(BlueprintCallable, Category = "HUD")
    void EliminatePlayer(int32 PlayerIdx);
};
