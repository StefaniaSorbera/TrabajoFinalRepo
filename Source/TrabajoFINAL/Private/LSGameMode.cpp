// Fill out your copyright notice in the Description page of Project Settings.


#include "LSGameMode.h"
#include "LSGameState.h"
#include "LSPlayerState.h"
#include "LSPlayerController.h"
#include "GameFramework/PlayerController.h"
#include "EngineUtils.h"

ALSGameMode::ALSGameMode()
{
    // Asignamos nuestras clases al framework
    GameStateClass        = ALSGameState::StaticClass();
    PlayerStateClass      = ALSPlayerState::StaticClass();
    PlayerControllerClass = ALSPlayerController::StaticClass();
}

void ALSGameMode::BeginPlay()
{
    Super::BeginPlay();

    // Inicializamos el GameState
    ALSGameState* GS = GetGameState<ALSGameState>();
    if (GS)
    {
        GS->SetMatchState(EMatchState::InProgress);
        GS->PlayersAlive = GetNumPlayers(); // cantidad de jugadores conectados
    }

    // Timer de la partida
    GetWorldTimerManager().SetTimer(
        MatchTimerHandle,
        this,
        &ALSGameMode::OnMatchTimeUp,
        MatchDuration,
        false
    );
}
    

void ALSGameMode::PlayerDied(AController* DeadPlayer, AController* Killer)
{
    if (!HasAuthority()) return;

    // Sumamos kill al que mató (si no se mató solo)
    if (Killer && Killer != DeadPlayer)
    {
        ALSPlayerState* KillerPS = Killer->GetPlayerState<ALSPlayerState>();
        if (KillerPS) KillerPS->AddKill();
    }

    // Decrementamos vida y marcamos muerto
    ALSPlayerState* DeadPS = DeadPlayer->GetPlayerState<ALSPlayerState>();
    if (DeadPS) DeadPS->SetPlayerDead();

    // Actualizamos GameState
    ALSGameState* GS = GetGameState<ALSGameState>();
    if (GS) GS->DecrementPlayersAlive();

    CheckVictoryCondition();
}

void ALSGameMode::CheckVictoryCondition()
{
    TArray<AController*> AlivePlayers;

    // Recorremos todos los PlayerControllers activos
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        ALSPlayerController* PC = Cast<ALSPlayerController>(It->Get());
        if (!PC) continue;

        ALSPlayerState* PS = PC->GetPlayerState<ALSPlayerState>();
        if (PS && PS->IsAlive())
        {
            AlivePlayers.Add(PC);
        }
    }

    // Condición de victoria: queda 1 solo jugador
    if (AlivePlayers.Num() == 1)
    {
        EndMatch(AlivePlayers[0]);
    }
    else if (AlivePlayers.Num() == 0)
    {
        EndMatch(nullptr); // Empate / todos muertos
    }
}

void ALSGameMode::EndMatch(AController* Winner)
{
    GetWorldTimerManager().ClearTimer(MatchTimerHandle);

    // Notificamos a cada jugador si ganó o perdió
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        ALSPlayerController* PC = Cast<ALSPlayerController>(It->Get());
        if (!PC) continue;

        if (PC == Winner)
        {
            PC->Client_ShowVictory();
        }
        else
        {
            PC->Client_ShowDefeat();
        }
    }
}

void ALSGameMode::OnMatchTimeUp()
{
    // Si se acaba el tiempo, gana el que más kills tiene
    // (implementar después con PlayerState)
    CheckVictoryCondition();
}
