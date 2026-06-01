#include "LSDeathWidget.h"
#include "Components/TextBlock.h"

void ULSDeathWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void ULSDeathWidget::NativeDestruct()
{
	Super::NativeDestruct();
	GetWorld()->GetTimerManager().ClearTimer(CountdownHandle);
}

void ULSDeathWidget::StartRespawnCountdown(float RespawnTime)
{
	RemainingTime = RespawnTime;

	if (TXT_Title)
	{
		TXT_Title->SetText(
			FText::FromString(TEXT("¡Caíste!")));
		TXT_Title->SetColorAndOpacity(
			FSlateColor(FLinearColor(0.9f, 0.2f, 0.2f, 1.f)));
	}

	if (TXT_Countdown)
	{
		TXT_Countdown->SetText(FText::FromString(
			FString::Printf(TEXT("%.0f"), RemainingTime)));
	}

	GetWorld()->GetTimerManager().SetTimer(
		CountdownHandle,
		this,
		&ULSDeathWidget::OnCountdownTick,
		1.f,
		true);
}

void ULSDeathWidget::OnCountdownTick()
{
	RemainingTime -= 1.f;

	if (TXT_Countdown)
	{
		TXT_Countdown->SetText(FText::FromString(
			FString::Printf(TEXT("%.0f"),
				FMath::Max(0.f, RemainingTime))));
	}
}