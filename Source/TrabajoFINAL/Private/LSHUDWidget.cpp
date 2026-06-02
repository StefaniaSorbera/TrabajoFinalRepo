#include "LSHUDWidget.h"
#include "LSPlayerHUDWidget.h"
#include "Components/TextBlock.h"

void ULSHUDWidget::UpdateMatchTime(float NewTime)
{
	if (!TXT_Timer) return;

	int32 Minutes = FMath::FloorToInt(NewTime / 60.f);
	int32 Seconds = FMath::FloorToInt(
		FMath::Fmod(NewTime, 60.f));

	FString TimeString = FString::Printf(
		TEXT("%02d:%02d"), Minutes, Seconds);

	TXT_Timer->SetText(FText::FromString(TimeString));
}

void ULSHUDWidget::SetPlayerSlotVisible(
	int32 PlayerIdx, bool bVisible)
{
	ULSPlayerHUDWidget* Target = GetPlayerHUD(PlayerIdx);
	if (!Target) return;

	Target->SetVisibility(bVisible ?
		ESlateVisibility::Visible :
		ESlateVisibility::Hidden);
}
void ULSHUDWidget::UpdatePlayerHearts(
	int32 PlayerIdx, int32 HeartsLeft)
{
	ULSPlayerHUDWidget* Target = nullptr;

	switch (PlayerIdx)
	{
	case 0: Target = Player1HUD; break;
	case 1: Target = Player2HUD; break;
	case 2: Target = Player3HUD; break;
	case 3: Target = Player4HUD; break;
	}

	if (Target) Target->SetHearts(HeartsLeft);
}

void ULSHUDWidget::UpdatePlayerKills(
	int32 PlayerIdx, int32 Kills)
{
	ULSPlayerHUDWidget* Target = nullptr;

	switch (PlayerIdx)
	{
	case 0: Target = Player1HUD; break;
	case 1: Target = Player2HUD; break;
	case 2: Target = Player3HUD; break;
	case 3: Target = Player4HUD; break;
	}

	if (Target) Target->SetKills(Kills);
}

ULSPlayerHUDWidget* ULSHUDWidget::GetPlayerHUD(int32 PlayerIdx)
{
	switch (PlayerIdx)
	{
	case 0: return Player1HUD;
	case 1: return Player2HUD;
	case 2: return Player3HUD;
	case 3: return Player4HUD;
	default: return nullptr;
	}
}

void ULSHUDWidget::EliminatePlayer(int32 PlayerIdx)
{
	ULSPlayerHUDWidget* Target = nullptr;

	switch (PlayerIdx)
	{
	case 0: Target = Player1HUD; break;
	case 1: Target = Player2HUD; break;
	case 2: Target = Player3HUD; break;
	case 3: Target = Player4HUD; break;
	}

	if (Target) Target->SetEliminated();
}