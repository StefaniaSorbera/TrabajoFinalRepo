#include "LSPlayerState.h"
#include "Net/UnrealNetwork.h"

ALSPlayerState::ALSPlayerState()
{
}

void ALSPlayerState::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALSPlayerState, KillCount);
	DOREPLIFETIME(ALSPlayerState, LivesLeft);
	DOREPLIFETIME(ALSPlayerState, PlayerStatus);
}

void ALSPlayerState::AddKill()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		KillCount++;
	}
}

void ALSPlayerState::SetPlayerDead()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		LivesLeft = FMath::Max(0, LivesLeft - 1);
		PlayerStatus = EPlayerStatus::Dead;
		// Al cambiar PlayerStatus en servidor,
		// OnRep_PlayerStatus se dispara en todos los clientes
	}
}

void ALSPlayerState::SetPlayerAlive()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		PlayerStatus = EPlayerStatus::Alive;
	}
}

bool ALSPlayerState::IsAlive() const
{
	return PlayerStatus == EPlayerStatus::Alive;
}

void ALSPlayerState::OnRep_PlayerStatus()
{
	// Corre en cada cliente cuando cambia el estado
	// Acá podés actualizar el HUD o efectos visuales
	UE_LOG(LogTemp, Log, TEXT("PlayerStatus cambió: %d"),
		(uint8)PlayerStatus);
	
}
void ALSPlayerState::SetLivesLeft(int32 NewLives)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		LivesLeft = NewLives;
	}
}