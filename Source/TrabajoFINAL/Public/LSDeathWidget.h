#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LSDeathWidget.generated.h"

UCLASS()
class TRABAJOFINAL_API ULSDeathWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* TXT_Title;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* TXT_Countdown;

    UFUNCTION(BlueprintCallable, Category = "UI")
    void StartRespawnCountdown(float RespawnTime);

    UFUNCTION(BlueprintImplementableEvent, Category = "UI")
    void OnRespawnReady();

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

private:
    float RemainingTime = 0.f;
    FTimerHandle CountdownHandle;

    void OnCountdownTick();
};
