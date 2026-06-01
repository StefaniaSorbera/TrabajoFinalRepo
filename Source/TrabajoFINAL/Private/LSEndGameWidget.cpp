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

void ULSEndGameWidget::SetupAsVictory()
{
    if (TXT_Result)
    {
        TXT_Result->SetText(
            FText::FromString(TEXT("¡GANASTE!")));
        TXT_Result->SetColorAndOpacity(
            FSlateColor(FLinearColor(0.2f, 0.9f, 0.3f, 1.f)));
    }

    if (TXT_Subtitle)
    {
        TXT_Subtitle->SetText(
            FText::FromString(TEXT("Último superviviente")));
    }

    if (IMG_Result && VictoryTexture)
    {
        IMG_Result->SetBrushFromTexture(VictoryTexture);
        IMG_Result->SetVisibility(ESlateVisibility::Visible);
    }
}

void ULSEndGameWidget::SetupAsDefeat()
{
    if (TXT_Result)
    {
        TXT_Result->SetText(
            FText::FromString(TEXT("ELIMINADO")));
        TXT_Result->SetColorAndOpacity(
            FSlateColor(FLinearColor(0.9f, 0.2f, 0.2f, 1.f)));
    }

    if (TXT_Subtitle)
    {
        TXT_Subtitle->SetText(
            FText::FromString(TEXT("Mejor suerte la próxima")));
    }

    if (IMG_Result && DefeatTexture)
    {
        IMG_Result->SetBrushFromTexture(DefeatTexture);
        IMG_Result->SetVisibility(ESlateVisibility::Visible);
    }
}

void ULSEndGameWidget::StartCountdown(float Seconds)
{
    RemainingTime = Seconds;

    if (TXT_Countdown)
    {
        TXT_Countdown->SetText(FText::FromString(
            FString::Printf(TEXT("Reiniciando en %.0f..."),
                RemainingTime)));
    }

    GetWorld()->GetTimerManager().SetTimer(
        CountdownHandle,
        this,
        &ULSEndGameWidget::OnCountdownTick,
        1.f,
        true);
}

void ULSEndGameWidget::OnCountdownTick()
{
    RemainingTime -= 1.f;

    if (TXT_Countdown)
    {
        TXT_Countdown->SetText(FText::FromString(
            FString::Printf(TEXT("Reiniciando en %.0f..."),
                FMath::Max(0.f, RemainingTime))));
    }

    if (RemainingTime <= 0.f)
    {
        GetWorld()->GetTimerManager().ClearTimer(
            CountdownHandle);
    }
}