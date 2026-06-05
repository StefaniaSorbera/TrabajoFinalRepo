#include "LSPlayerState.h"

#include "LSPlayerController.h"
#include "Net/UnrealNetwork.h"

ALSPlayerState::ALSPlayerState()
{
}

void ALSPlayerState::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ALSPlayerState, HUDSlotIndex);
	DOREPLIFETIME(ALSPlayerState, KillCount);
	DOREPLIFETIME(ALSPlayerState, LivesLeft);
	DOREPLIFETIME(ALSPlayerState, PlayerStatus);
}

void ALSPlayerState::OnRep_LivesLeft()
{
	// Recorre TODOS los controllers locales y actualiza el slot correcto
	UWorld* World = GetWorld();
	if (!World) return;

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		ALSPlayerController* PC = Cast<ALSPlayerController>(It->Get());
		// Solo actualizamos el controller LOCAL de esta máquina
		if (PC && PC->IsLocalController())
		{
			PC->UpdateHUDHearts(HUDSlotIndex, LivesLeft);
			break;
		}
	}
}

void ALSPlayerState::OnRep_HUDSlotIndex()
{
	if (HUDSlotIndex < 0) return;

	UWorld* World = GetWorld();
	if (!World) return;

	// Buscar el controller local y decirle que reinicialice el HUD
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		ALSPlayerController* PC = Cast<ALSPlayerController>(It->Get());
		if (PC && PC->IsLocalController())
		{
			PC->InitializeHUD();
			break;
		}
	}
}

void ALSPlayerState::AddKill()
{
	if (GetLocalRole() != ROLE_Authority) return;

	KillCount++;

	// Notificamos al controller del killer
	ALSPlayerController* PC =
		Cast<ALSPlayerController>(GetOwningController());
	if (PC)
	{
		int32 PlayerIdx = GetPlayerId() % 4;
		PC->UpdateHUDKills(PlayerIdx, KillCount);
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