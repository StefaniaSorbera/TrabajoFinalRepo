#include "LSPlayerController.h"
#include "LSPlayerHUDWidget.h"
#include "LSHUDWidget.h"
#include "LSEndGameWidget.h"
#include "LSDeathWidget.h"
#include "LSPlayerState.h"
#include "GameFramework/GameStateBase.h"

class ALSPlayerState;

ALSPlayerController::ALSPlayerController()
{
}
void ALSPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController())
	{
		if (HUDWidgetClass)
		{
			HUDWidget = CreateWidget<ULSHUDWidget>(
				this, HUDWidgetClass);
			if (HUDWidget)
			{
				HUDWidget->AddToViewport();
			}
		}
		// Removemos el timer de InitializeHUD
		// ahora lo llama el servidor via Client_InitializeHUD
	}
}
void ALSPlayerController::InitializeHUD()
{
	if (!HUDWidget) return;

	for (int32 i = 0; i < 4; i++)
	{
		HUDWidget->SetPlayerSlotVisible(i, false);
	}

	TArray<FLinearColor> PlayerColors = {
		FLinearColor(0.1f, 0.4f, 0.9f, 1.f),
		FLinearColor(0.9f, 0.2f, 0.1f, 1.f),
		FLinearColor(0.13f, 0.99f, 0.1f, 1.f),
		FLinearColor(0.95f, 0.1f, 0.9f, 1.f)
	};

	TArray<FString> PlayerNames = {
		TEXT("Jugador 1"),
		TEXT("Jugador 2"),
		TEXT("Jugador 3"),
		TEXT("Jugador 4")
	};

	AGameStateBase* GS = GetWorld()->GetGameState();
	if (!GS) return;

	// ===== LOGS DE DIAGNÓSTICO =====
	UE_LOG(LogTemp, Warning, TEXT("=== InitializeHUD | Controller=%s | PlayerArray=%d ==="),
		*GetName(), GS->PlayerArray.Num());
	// ================================

	for (APlayerState* PS : GS->PlayerArray)
	{
		ALSPlayerState* LSPS = Cast<ALSPlayerState>(PS);
		if (!LSPS) continue;

		// ===== LOG POR CADA JUGADOR =====
		UE_LOG(LogTemp, Warning, TEXT("  >> Player=%s | Slot=%d | Lives=%d"),
			*LSPS->GetPlayerName(), LSPS->HUDSlotIndex, LSPS->LivesLeft);
		// ================================

		int32 Idx = LSPS->HUDSlotIndex;
		if (Idx < 0 || Idx >= 4) continue;

		HUDWidget->SetPlayerSlotVisible(Idx, true);

		ULSPlayerHUDWidget* PlayerHUD = HUDWidget->GetPlayerHUD(Idx);
		if (PlayerHUD)
		{
			PlayerHUD->InitPlayer(Idx, PlayerNames[Idx], PlayerColors[Idx]);
			PlayerHUD->SetHearts(LSPS->LivesLeft);
		}
	}
}
void ALSPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
}

// --- Client RPCs ---

void ALSPlayerController::Client_InitializeHUD_Implementation()
{
	if (HUDWidget)
	{
		InitializeHUD();
	}
	else
	{
		// Si el HUD todavía no se creó esperamos un poco
		FTimerHandle InitHandle;
		GetWorldTimerManager().SetTimer(
			InitHandle,
			this,
			&ALSPlayerController::InitializeHUD,
			0.5f,
			false);
	}
}

void ALSPlayerController::Client_ShowVictory_Implementation()
{
	if (EndGameWidgetClass)
	{
		EndGameWidget = CreateWidget<ULSEndGameWidget>(
			this, EndGameWidgetClass);
		if (EndGameWidget)
		{
			EndGameWidget->AddToViewport(10);
			EndGameWidget->SetupAsVictory();
		}
	}
	BP_ShowVictoryScreen();
}

void ALSPlayerController::Client_ShowDefeat_Implementation()
{
	if (EndGameWidgetClass)
	{
		EndGameWidget = CreateWidget<ULSEndGameWidget>(
			this, EndGameWidgetClass);
		if (EndGameWidget)
		{
			EndGameWidget->AddToViewport(10);
			EndGameWidget->SetupAsDefeat();
		}
	}
	BP_ShowDefeatScreen();
}
void ALSPlayerController::UpdateHUDHearts(
	int32 PlayerIdx, int32 HeartsLeft)
{
	if (HUDWidget)
	{
		HUDWidget->UpdatePlayerHearts(PlayerIdx, HeartsLeft);
	}
}

void ALSPlayerController::UpdateHUDKills(
	int32 PlayerIdx, int32 Kills)
{
	if (HUDWidget)
	{
		HUDWidget->UpdatePlayerKills(PlayerIdx, Kills);
	}
}

void ALSPlayerController::Client_UpdatePlayerHearts_Implementation(int32 SlotIndex, int32 HeartsLeft)
{
}

void ALSPlayerController::UpdateHUDTimer(float NewTime)
{
	if (HUDWidget)
	{
		HUDWidget->UpdateMatchTime(NewTime);
	}
}
void ALSPlayerController::Client_ShowDeathScreen_Implementation()
{
	UE_LOG(LogTemp, Warning, 
		TEXT("Client_ShowDeathScreen recibido"));
	UE_LOG(LogTemp, Warning, 
		TEXT("DeathWidgetClass valido: %s"),
		DeathWidgetClass ? TEXT("SI") : TEXT("NO"));

	if (DeathWidgetClass)
	{
		DeathWidget = CreateWidget<ULSDeathWidget>(
			this, DeathWidgetClass);
        
		UE_LOG(LogTemp, Warning,
			TEXT("Widget creado: %s"),
			DeathWidget ? TEXT("SI") : TEXT("NO"));

		if (DeathWidget)
		{
			DeathWidget->AddToViewport(5);
			DeathWidget->StartRespawnCountdown(3.f);
		}
	}
	BP_ShowDeathScreen();
}

void ALSPlayerController::Client_StartRestartCountdown_Implementation(
	float Seconds)
{
	if (EndGameWidget)
	{
		EndGameWidget->StartCountdown(Seconds);
	}
	BP_StartRestartCountdown(Seconds);
}