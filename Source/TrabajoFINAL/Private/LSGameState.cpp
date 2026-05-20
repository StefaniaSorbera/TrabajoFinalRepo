// Fill out your copyright notice in the Description page of Project Settings.


#include "LSGameState.h"

#include "LSPlayerController.h"
#include "Net/UnrealNetwork.h"

ALSGameState::ALSGameState()
{
	// Nada especial en el constructor por ahora
}

void ALSGameState::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Registramos cada variable que queremos replicar
	DOREPLIFETIME(ALSGameState, MatchTime);
	DOREPLIFETIME(ALSGameState, MatchState);
	DOREPLIFETIME(ALSGameState, PlayersAlive);
}

void ALSGameState::SetMatchTime(float NewTime)
{
	// Solo el servidor modifica esto
	if (GetLocalRole() == ROLE_Authority)
	{
		MatchTime = NewTime;
		// Al cambiar en servidor, se replica a clientes
		// y dispara OnRep_MatchTime en cada uno
	}
}

void ALSGameState::DecrementPlayersAlive()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		PlayersAlive = FMath::Max(0, PlayersAlive - 1);
	}
}

void ALSGameState::SetMatchState(EMatchState NewState)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		MatchState = NewState;
	}
}

void ALSGameState::OnRep_MatchTime()
{
	// Buscamos el PlayerController local y actualizamos su HUD
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	ALSPlayerController* LSPC = Cast<ALSPlayerController>(PC);
	if (LSPC)
	{
		LSPC->BP_UpdateMatchTime(MatchTime);
	}
}