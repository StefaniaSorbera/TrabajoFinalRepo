// Fill out your copyright notice in the Description page of Project Settings.


#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "LSGameState.generated.h"

// Enum para los estados posibles de la partida
UENUM(BlueprintType)
enum class EMatchState : uint8
{
	WaitingToStart  UMETA(DisplayName = "Waiting To Start"),
	InProgress      UMETA(DisplayName = "In Progress"),
	PostGame        UMETA(DisplayName = "Post Game")
};

UCLASS()
class TRABAJOFINAL_API ALSGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	ALSGameState();

	// --- Variables replicadas ---

	// Tiempo restante — con RepNotify (actualiza el HUD automáticamente)
	UPROPERTY(ReplicatedUsing = OnRep_MatchTime, BlueprintReadOnly, Category = "Match")
	float MatchTime = 0.f;

	// Estado actual de la partida
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Match")
	EMatchState MatchState = EMatchState::WaitingToStart;

	// Cantidad de jugadores vivos
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Match")
	int32 PlayersAlive = 0;

	// --- Funciones ---

	// El GameMode llama esto para actualizar el tiempo
	void SetMatchTime(float NewTime);

	// El GameMode llama esto cuando muere un jugador
	void DecrementPlayersAlive();

	virtual void AddPlayerState(APlayerState* PlayerState) override;
	
	// El GameMode llama esto para cambiar el estado
	void SetMatchState(EMatchState NewState);

protected:
	// Se ejecuta en clientes cuando cambia MatchTime
	UFUNCTION()
	void OnRep_MatchTime();

	// Necesario para registrar las variables replicadas
	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
