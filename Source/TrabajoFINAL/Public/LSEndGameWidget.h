#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LSEndGameWidget.generated.h"

UCLASS()
class TRABAJOFINAL_API ULSEndGameWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* TXT_Result;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* TXT_Subtitle;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* TXT_Countdown;

    UPROPERTY(meta = (BindWidget))
    class UImage* IMG_Result;

    UPROPERTY(EditAnywhere, Category = "EndGame")
    UTexture2D* VictoryTexture;

    UPROPERTY(EditAnywhere, Category = "EndGame")
    UTexture2D* DefeatTexture;

    UFUNCTION(BlueprintCallable, Category = "EndGame")
    void SetupAsVictory();

    UFUNCTION(BlueprintCallable, Category = "EndGame")
    void SetupAsDefeat();

    UFUNCTION(BlueprintCallable, Category = "EndGame")
    void StartCountdown(float Seconds);

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

private:
    float RemainingTime = 0.f;
    FTimerHandle CountdownHandle;

    void SetupResult(const FString& ResultText, const FLinearColor& ResultColor,
                     const FString& SubtitleText, UTexture2D* ResultTexture);
    void OnCountdownTick();
};
