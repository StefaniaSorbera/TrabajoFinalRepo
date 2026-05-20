#pragma once

#include "CoreMinimal.h"
#include "TrabajoFINALPlayerController.h"
#include "GameFramework/PlayerController.h"
#include "LSPlayerController.generated.h"

UCLASS()
class TRABAJOFINAL_API ALSPlayerController : public ATrabajoFINALPlayerController
{
	GENERATED_BODY()

public:
	ALSPlayerController();

	// --- Client RPCs ---
	// Solo corren en el cliente dueño del controller

	UFUNCTION(Client, Reliable)
	void Client_ShowVictory();

	UFUNCTION(Client, Reliable)
	void Client_ShowDefeat();

	UFUNCTION(Client, Reliable)
	void Client_ShowDeathScreen();

	// --- Actualizar HUD ---
	// Llamado desde RepNotify del GameState/PlayerState

	UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
	void BP_UpdateMatchTime(float NewTime);

	UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
	void BP_UpdateKillCount(int32 NewKills);

	UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
	void BP_UpdateLivesLeft(int32 NewLives);

	UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
	void BP_ShowVictoryScreen();

	UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
	void BP_ShowDefeatScreen();

	UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
	void BP_ShowDeathScreen();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	// Referencia al widget HUD principal
	UPROPERTY()
	class UUserWidget* HUDWidget;

	// Clase del widget HUD (asignar en Blueprint hijo)
	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<UUserWidget> HUDWidgetClass;
};