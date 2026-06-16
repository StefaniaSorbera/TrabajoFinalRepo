#include "LSPlayerState.h"
#include "LSPlayerController.h"
#include "Net/UnrealNetwork.h"

ALSPlayerState::ALSPlayerState()
{
}

void ALSPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ALSPlayerState, HUDSlotIndex);
    DOREPLIFETIME(ALSPlayerState, KillCount);
    DOREPLIFETIME(ALSPlayerState, LivesLeft);
    DOREPLIFETIME(ALSPlayerState, PlayerStatus);
}

ALSPlayerController* ALSPlayerState::GetLocalPlayerController() const
{
    UWorld* World = GetWorld();
    if (!World) return nullptr;
    for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
    {
        ALSPlayerController* PC = Cast<ALSPlayerController>(It->Get());
        if (PC && PC->IsLocalController())
            return PC;
    }
    return nullptr;
}

void ALSPlayerState::OnRep_LivesLeft()
{
    if (ALSPlayerController* PC = GetLocalPlayerController())
        PC->UpdateHUDHearts(HUDSlotIndex, LivesLeft);
}

void ALSPlayerState::OnRep_HUDSlotIndex()
{
    if (HUDSlotIndex < 0) return;
    if (ALSPlayerController* PC = GetLocalPlayerController())
        PC->InitializeHUD();
}

void ALSPlayerState::AddKill()
{
    if (GetLocalRole() != ROLE_Authority) return;
    KillCount++;
    if (ALSPlayerController* PC = Cast<ALSPlayerController>(GetOwningController()))
        PC->UpdateHUDKills(GetPlayerId() % 4, KillCount);
}

void ALSPlayerState::SetPlayerDead()
{
    if (GetLocalRole() == ROLE_Authority)
    {
        LivesLeft = FMath::Max(0, LivesLeft - 1);
        PlayerStatus = EPlayerStatus::Dead;
    }
}

void ALSPlayerState::SetPlayerAlive()
{
    if (GetLocalRole() == ROLE_Authority)
        PlayerStatus = EPlayerStatus::Alive;
}

bool ALSPlayerState::IsAlive() const
{
    return PlayerStatus == EPlayerStatus::Alive;
}

void ALSPlayerState::OnRep_PlayerStatus()
{
}

void ALSPlayerState::SetLivesLeft(int32 NewLives)
{
    if (GetLocalRole() == ROLE_Authority)
        LivesLeft = NewLives;
}
