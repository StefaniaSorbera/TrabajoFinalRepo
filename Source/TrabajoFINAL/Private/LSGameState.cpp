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
	DOREPLIFETIME(ALSGameState, PlayerMaterials);
}

void ALSGameState::SetPlayersAlive(int32 NewCount)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		PlayersAlive = NewCount;
		OnRep_PlayersAlive();
	}
}

void ALSGameState::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);

	UWorld* World = GetWorld();
	if (!World) return;
	
	FTimerHandle Handle;
	World->GetTimerManager().SetTimer(Handle, [this, World]()
	{
	
		bool bAllSlotsAssigned = true;
		for (APlayerState* PS : PlayerArray)
		{
			ALSPlayerState* LSPS =
				Cast<ALSPlayerState>(PS);
			if (LSPS && LSPS->HUDSlotIndex == -1)
			{
				bAllSlotsAssigned = false;
				break;
			}
		}

		for (FConstPlayerControllerIterator It =
			World->GetPlayerControllerIterator(); It; ++It)
		{
			ALSPlayerController* PC =
				Cast<ALSPlayerController>(It->Get());
			if (PC && PC->IsLocalController())
			{
				PC->Client_InitializeHUD();
				break;
			}
		}
	}, 1.5f, false);
}
void ALSGameState::DecrementPlayersAlive()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		PlayersAlive = FMath::Max(0, PlayersAlive - 1);

		// Manual para el servidor igual que MatchTime
		OnRep_PlayersAlive();
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

void ALSGameState::OnRep_PlayersAlive()
{
	for (FConstPlayerControllerIterator It =
		GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ALSPlayerController* PC =
			Cast<ALSPlayerController>(It->Get());
		if (PC && PC->IsLocalController())
		{
			PC->UpdateHUDPlayersAlive(PlayersAlive);
			break;
		}
	}
}