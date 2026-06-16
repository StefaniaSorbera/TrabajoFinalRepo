#include "LSHUDWidget.h"
#include "LSPlayerHUDWidget.h"
#include "Components/TextBlock.h"

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

void ULSHUDWidget::UpdateMatchTime(float NewTime)
{
    if (!TXT_Timer) return;
    int32 Minutes = FMath::FloorToInt(NewTime / 60.f);
    int32 Seconds = FMath::FloorToInt(FMath::Fmod(NewTime, 60.f));
    TXT_Timer->SetText(FText::FromString(FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds)));
}

void ULSHUDWidget::SetPlayerSlotVisible(int32 PlayerIdx, bool bVisible)
{
    if (ULSPlayerHUDWidget* Target = GetPlayerHUD(PlayerIdx))
        Target->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}

void ULSHUDWidget::UpdatePlayerHearts(int32 PlayerIdx, int32 HeartsLeft)
{
    if (ULSPlayerHUDWidget* Target = GetPlayerHUD(PlayerIdx))
        Target->SetHearts(HeartsLeft);
}

void ULSHUDWidget::UpdatePlayerKills(int32 PlayerIdx, int32 Kills)
{
    if (ULSPlayerHUDWidget* Target = GetPlayerHUD(PlayerIdx))
        Target->SetKills(Kills);
}

void ULSHUDWidget::UpdatePlayersAlive(int32 PlayersAlive)
{
    if (TXT_PlayersAlive)
        TXT_PlayersAlive->SetText(FText::FromString(FString::Printf(TEXT("%d"), PlayersAlive)));
}

void ULSHUDWidget::EliminatePlayer(int32 PlayerIdx)
{
    if (ULSPlayerHUDWidget* Target = GetPlayerHUD(PlayerIdx))
        Target->SetEliminated();
}
