// Fill out your copyright notice in the Description page of Project Settings.


#include "LSGameState.h"

#include "LSPlayerController.h"
#include "LSPlayerState.h"
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


void ALSGameState::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);

	// Se llama en servidor Y en clientes cuando llega un nuevo PlayerState
	UWorld* World = GetWorld();
	if (!World) return;

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		ALSPlayerController* PC = Cast<ALSPlayerController>(It->Get());
		if (PC && PC->IsLocalController())
		{
			// Esperamos un frame para que HUDSlotIndex esté replicado
			FTimerHandle Handle;
			World->GetTimerManager().SetTimer(Handle, [PC]()
			{
				PC->InitializeHUD();
			}, 0.2f, false);
			break;
		}
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
void ALSGameState::SetMatchTime(float NewTime)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		MatchTime = NewTime;

		// RepNotify no se dispara en servidor
		// lo llamamos manualmente para el host
		OnRep_MatchTime();
	}
}

void ALSGameState::OnRep_MatchTime()
{
	// Buscamos el PlayerController local
	for (FConstPlayerControllerIterator It =
		GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ALSPlayerController* PC =
			Cast<ALSPlayerController>(It->Get());

		if (PC && PC->IsLocalController())
		{
			PC->UpdateHUDTimer(MatchTime);
			break;
		}
	}
}