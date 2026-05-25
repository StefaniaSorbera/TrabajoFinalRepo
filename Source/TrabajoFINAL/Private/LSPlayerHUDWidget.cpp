#include "LSPlayerHUDWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

void ULSPlayerHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Inicializamos los corazones al máximo
	SetHearts(MaxHearts);
}

void ULSPlayerHUDWidget::InitPlayer(int32 Index,
	const FString& Name, FLinearColor Color)
{
	PlayerIndex = Index;
	PlayerColor = Color;
	SetPlayerName(Name);
	SetHearts(MaxHearts);

	// Aplicamos el color del jugador al nombre
	if (TXT_PlayerName)
	{
		TXT_PlayerName->SetColorAndOpacity(
			FSlateColor(Color));
	}
}

void ULSPlayerHUDWidget::SetPlayerName(const FString& Name)
{
	if (TXT_PlayerName)
	{
		TXT_PlayerName->SetText(FText::FromString(Name));
	}
}

void ULSPlayerHUDWidget::SetHearts(int32 HeartsLeft)
{
	CurrentHearts = FMath::Clamp(HeartsLeft, 0, MaxHearts);

	UpdateHeartColor(Heart1, CurrentHearts >= 1);
	UpdateHeartColor(Heart2, CurrentHearts >= 2);
	UpdateHeartColor(Heart3, CurrentHearts >= 3);
}

void ULSPlayerHUDWidget::SetKills(int32 Kills)
{
	if (TXT_Kills)
	{
		FString KillText = FString::Printf(TEXT("Kills: %d"), Kills);
		TXT_Kills->SetText(FText::FromString(KillText));
	}
}

void ULSPlayerHUDWidget::SetEliminated()
{
	bIsEliminated = true;

	// Vaciamos todos los corazones
	SetHearts(0);

	// Oscurecemos el nombre
	if (TXT_PlayerName)
	{
		TXT_PlayerName->SetColorAndOpacity(
			FSlateColor(FLinearColor(0.3f, 0.3f, 0.3f, 0.5f)));
	}

	// Mostramos "Eliminado" en lugar de kills
	if (TXT_Kills)
	{
		TXT_Kills->SetText(
			FText::FromString(TEXT("Eliminado")));
		TXT_Kills->SetColorAndOpacity(
			FSlateColor(FLinearColor(0.8f, 0.2f, 0.2f, 0.7f)));
	}
}

void ULSPlayerHUDWidget::UpdateHeartColor(
	UImage* Heart, bool bFull)
{
	if (!Heart) return;

	Heart->SetColorAndOpacity(
		bFull ? HeartFullColor : HeartEmptyColor);
}