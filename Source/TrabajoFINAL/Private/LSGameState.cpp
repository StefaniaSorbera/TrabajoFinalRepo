#include "LSGameState.h"
#include "LSPlayerController.h"
#include "LSPlayerState.h"
#include "Net/UnrealNetwork.h"

ALSGameState::ALSGameState()
{
}

void ALSGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ALSGameState, MatchTime);
    DOREPLIFETIME(ALSGameState, MatchState);
    DOREPLIFETIME(ALSGameState, PlayersAlive);
    DOREPLIFETIME(ALSGameState, PlayerMaterials);
}

ALSPlayerController* ALSGameState::GetLocalPlayerController() const
{
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        ALSPlayerController* PC = Cast<ALSPlayerController>(It->Get());
        if (PC && PC->IsLocalController())
            return PC;
    }
    return nullptr;
}

void ALSGameState::SetPlayersAlive(int32 NewCount)
{
    if (GetLocalRole() != ROLE_Authority) return;
    PlayersAlive = NewCount;
    OnRep_PlayersAlive();
}

void ALSGameState::DecrementPlayersAlive()
{
    if (GetLocalRole() != ROLE_Authority) return;
    PlayersAlive = FMath::Max(0, PlayersAlive - 1);
    OnRep_PlayersAlive();
}

void ALSGameState::SetMatchState(EMatchState NewState)
{
    if (GetLocalRole() == ROLE_Authority)
        MatchState = NewState;
}

void ALSGameState::SetMatchTime(float NewTime)
{
    if (GetLocalRole() != ROLE_Authority) return;
    MatchTime = NewTime;
    OnRep_MatchTime();
}

void ALSGameState::AddPlayerState(APlayerState* PlayerState)
{
    Super::AddPlayerState(PlayerState);

    UWorld* World = GetWorld();
    if (!World) return;

    FTimerHandle Handle;
    World->GetTimerManager().SetTimer(Handle, [this, World]()
    {
        for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
        {
            ALSPlayerController* PC = Cast<ALSPlayerController>(It->Get());
            if (PC && PC->IsLocalController())
            {
                PC->Client_InitializeHUD();
                break;
            }
        }
    }, 1.5f, false);
}

void ALSGameState::OnRep_MatchTime()
{
    if (ALSPlayerController* PC = GetLocalPlayerController())
        PC->UpdateHUDTimer(MatchTime);
}

void ALSGameState::OnRep_PlayersAlive()
{
    if (ALSPlayerController* PC = GetLocalPlayerController())
        PC->UpdateHUDPlayersAlive(PlayersAlive);
}
