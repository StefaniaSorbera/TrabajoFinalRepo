// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "LSPlayerState.generated.h"

UENUM(BlueprintType)
enum class EPlayerStatus : uint8
{
	Alive       UMETA(DisplayName = "Alive"),
	Dead        UMETA(DisplayName = "Dead"),
	Spectator   UMETA(DisplayName = "Spectator")
};

UCLASS()
class TRABAJOFINAL_API ALSPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	ALSPlayerState();

	// --- Variables replicadas ---

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player")
	int32 KillCount = 0;
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player")
	int32 HUDSlotIndex =-1;
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player")
	int32 LivesLeft = 3;

	// RepNotify — cuando cambia el estado notificamos al HUD
	UPROPERTY(ReplicatedUsing = OnRep_PlayerStatus, BlueprintReadOnly, Category = "Player")
	EPlayerStatus PlayerStatus = EPlayerStatus::Alive;

	// --- Funciones llamadas desde GameMode ---

	void AddKill();
	void SetPlayerDead();
	void SetPlayerAlive();
	void SetLivesLeft(int32 NewLives);

	// Consultada por GameMode para contar jugadores vivos
	bool IsAlive() const;

protected:
	UFUNCTION()
	void OnRep_PlayerStatus();

	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
