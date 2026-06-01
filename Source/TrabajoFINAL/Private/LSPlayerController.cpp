#include "LSPlayerController.h"
#include "LSHUDWidget.h"
#include "LSEndGameWidget.h"
#include "LSDeathWidget.h"

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
			if (HUDWidget) HUDWidget->AddToViewport();
		}
	}
}

void ALSPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
}

// --- Client RPCs ---

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

void ALSPlayerController::Client_ShowDeathScreen_Implementation()
{
	if (DeathWidgetClass)
	{
		DeathWidget = CreateWidget<ULSDeathWidget>(
			this, DeathWidgetClass);
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