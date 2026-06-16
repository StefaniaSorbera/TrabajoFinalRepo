#include "LSEndGameWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

void ULSEndGameWidget::NativeConstruct()
{
    Super::NativeConstruct();
}

void ULSEndGameWidget::NativeDestruct()
{
    Super::NativeDestruct();
    GetWorld()->GetTimerManager().ClearTimer(CountdownHandle);
}

void ULSEndGameWidget::SetupResult(
    const FString& ResultText,
    const FLinearColor& ResultColor,
    const FString& SubtitleText,
    UTexture2D* ResultTexture)
{
    if (TXT_Result)
    {
        TXT_Result->SetText(FText::FromString(ResultText));
        TXT_Result->SetColorAndOpacity(FSlateColor(ResultColor));
    }
    if (TXT_Subtitle)
        TXT_Subtitle->SetText(FText::FromString(SubtitleText));
    if (IMG_Result && ResultTexture)
    {
        IMG_Result->SetBrushFromTexture(ResultTexture);
        IMG_Result->SetVisibility(ESlateVisibility::Visible);
    }
}

void ULSEndGameWidget::SetupAsVictory()
{
    SetupResult(
        TEXT("¡GANASTE!"),
        FLinearColor(0.2f, 0.9f, 0.3f, 1.f),
        TEXT("Último superviviente"),
        VictoryTexture);
}

void ULSEndGameWidget::SetupAsDefeat()
{
    SetupResult(
        TEXT("ELIMINADO"),
        FLinearColor(0.9f, 0.2f, 0.2f, 1.f),
        TEXT("Mejor suerte la próxima"),
        DefeatTexture);
}

void ULSEndGameWidget::StartCountdown(float Seconds)
{
    RemainingTime = Seconds;
    if (TXT_Countdown)
        TXT_Countdown->SetText(FText::FromString(FString::Printf(TEXT("Reiniciando en %.0f..."), RemainingTime)));
    GetWorld()->GetTimerManager().SetTimer(CountdownHandle, this, &ULSEndGameWidget::OnCountdownTick, 1.f, true);
}

void ULSEndGameWidget::OnCountdownTick()
{
    RemainingTime -= 1.f;
    if (TXT_Countdown)
        TXT_Countdown->SetText(FText::FromString(FString::Printf(TEXT("Reiniciando en %.0f..."), FMath::Max(0.f, RemainingTime))));
    if (RemainingTime <= 0.f)
        GetWorld()->GetTimerManager().ClearTimer(CountdownHandle);
}
