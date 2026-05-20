// Fill out your copyright notice in the Description page of Project Settings.

#include "LSPlayerController.h"
#include "Blueprint/UserWidget.h"

ALSPlayerController::ALSPlayerController()
{
}

void ALSPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Solo el cliente local crea su HUD
	if (IsLocalController())
	{
		if (HUDWidgetClass)
		{
			HUDWidget = CreateWidget<UUserWidget>(this, HUDWidgetClass);
			if (HUDWidget)
			{
				HUDWidget->AddToViewport();
			}
		}
	}
}

void ALSPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	// El input lo vamos a configurar en el Character
	// Acá podés agregar inputs de menú (pausa, etc.)
}

// --- Implementación de Client RPCs ---

void ALSPlayerController::Client_ShowVictory_Implementation()
{
	// Corre solo en el cliente dueño
	BP_ShowVictoryScreen();
}

void ALSPlayerController::Client_ShowDefeat_Implementation()
{
	BP_ShowDefeatScreen();
}

void ALSPlayerController::Client_ShowDeathScreen_Implementation()
{
	BP_ShowDeathScreen();
}