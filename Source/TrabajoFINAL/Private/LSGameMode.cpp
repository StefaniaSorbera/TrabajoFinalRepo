#include "LSGameMode.h"
#include "LSGameState.h"
#include "LSPlayerState.h"
#include "LSPlayerController.h"
#include "LSCharacter.h"
#include "EngineUtils.h"

ALSGameMode::ALSGameMode()
{
    GameStateClass        = ALSGameState::StaticClass();
    PlayerStateClass      = ALSPlayerState::StaticClass();
    PlayerControllerClass = ALSPlayerController::StaticClass();
    DefaultPawnClass      = ALSCharacter::StaticClass();
}

void ALSGameMode::BeginPlay()
{
    Super::BeginPlay();

    ALSGameState* GS = GetGameState<ALSGameState>();
    if (GS)
    {
        GS->SetMatchState(EMatchState::InProgress);
        GS->PlayersAlive = GetNumPlayers();
        GS->SetMatchTime(MatchDuration);
    }

    // Timer del match
    GetWorldTimerManager().SetTimer(
        MatchTickHandle,
        [this]()
        {
            ALSGameState* GS = GetGameState<ALSGameState>();
            if (!GS) return;

            float NewTime = GS->MatchTime - 1.f;
            GS->SetMatchTime(FMath::Max(0.f, NewTime));

            if (NewTime <= 0.f)
            {
                GetWorldTimerManager().ClearTimer(MatchTickHandle);
                OnMatchTimeUp();
            }
        },
        1.f,
        true);

    // Asignamos slots con delay para que todos
    // los PlayerStates estén inicializados
    GetWorldTimerManager().SetTimer(
        AssignSlotsHandle,        // <- agregar este handle en .h
        this,
        &ALSGameMode::AssignHUDSlots,
        1.f,
        false);
}

void ALSGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);
    // Reasignamos todos los slots cada vez que entra un jugador
    // con un pequeño delay para que el PlayerState esté inicializado
    FTimerHandle Handle;
    GetWorldTimerManager().SetTimer(Handle, [this]()
    {
        AssignHUDSlots();
    }, 0.3f, false);
}

void ALSGameMode::PlayerDied(AController* DeadPlayer,
                             AController* Killer)
{
    if (!HasAuthority()) return;

    // Sumamos kill al asesino
    if (Killer && Killer != DeadPlayer)
    {
        ALSPlayerState* KillerPS =
            Killer->GetPlayerState<ALSPlayerState>();
        if (KillerPS) KillerPS->AddKill();

        // Actualizamos HUD del killer
        ALSPlayerController* KillerPC =
            Cast<ALSPlayerController>(Killer);
        if (KillerPC)
        {
            KillerPC->BP_UpdateKillCount(
                KillerPS->KillCount);
        }
    }

    // Marcamos al jugador muerto
    ALSPlayerState* DeadPS =
        DeadPlayer->GetPlayerState<ALSPlayerState>();
    if (DeadPS) DeadPS->SetPlayerDead();

    // Decrementamos jugadores vivos en GameState
    ALSGameState* GS = GetGameState<ALSGameState>();
    if (GS) GS->DecrementPlayersAlive();

    // Notificamos derrota al cliente eliminado
    ALSPlayerController* DeadPC =
        Cast<ALSPlayerController>(DeadPlayer);
    if (DeadPC) DeadPC->Client_ShowDefeat();

    // Chequeamos si alguien ganó
    CheckVictoryCondition();
}

void ALSGameMode::CheckVictoryCondition()
{
    TArray<AController*> AlivePlayers;

    for (FConstPlayerControllerIterator It =
        GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        ALSPlayerController* PC =
            Cast<ALSPlayerController>(It->Get());
        if (!PC) continue;

        ALSPlayerState* PS =
            PC->GetPlayerState<ALSPlayerState>();

        // Contamos solo los que siguen vivos
        if (PS && PS->IsAlive())
        {
            AlivePlayers.Add(PC);
        }
    }

    if (AlivePlayers.Num() == 1)
    {
        // Hay un ganador
        EndMatch(AlivePlayers[0]);
    }
    else if (AlivePlayers.Num() == 0)
    {
        // Todos muertos al mismo tiempo — empate
        EndMatch(nullptr);
    }
    // Si quedan 2+ vivos seguimos jugando
}

void ALSGameMode::AssignHUDSlots()
{
    int32 SlotIndex = 0;

    for (FConstPlayerControllerIterator It =
        GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        ALSPlayerController* PC =
            Cast<ALSPlayerController>(It->Get());
        if (!PC) continue;

        ALSPlayerState* PS =
            PC->GetPlayerState<ALSPlayerState>();
        if (PS)
        {
            PS->HUDSlotIndex = SlotIndex;

            UE_LOG(LogTemp, Warning,
                TEXT("Slot asignado: Controller=%s Slot=%d"),
                *PC->GetName(), SlotIndex);

            SlotIndex++;
        }
    }
    FTimerHandle NotifyHandle;
    GetWorldTimerManager().SetTimer(
        NotifyHandle,
        [this]()
        {
            for (FConstPlayerControllerIterator It =
                GetWorld()->GetPlayerControllerIterator();
                It; ++It)
            {
                ALSPlayerController* PC =
                    Cast<ALSPlayerController>(It->Get());
                if (PC) PC->Client_InitializeHUD();
            }
        },
        0.5f,
        false);
}
void ALSGameMode::EndMatch(AController* Winner)
{
    GetWorldTimerManager().ClearTimer(MatchTickHandle);

    // Actualizamos GameState
    ALSGameState* GS = GetGameState<ALSGameState>();
    if (GS) GS->SetMatchState(EMatchState::PostGame);

    // Congelamos todos los personajes y mostramos pantalla
    for (FConstPlayerControllerIterator It =
        GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        ALSPlayerController* PC =
            Cast<ALSPlayerController>(It->Get());
        if (!PC) continue;

        // Desactivamos input de todos
        PC->SetIgnoreMoveInput(true);
        PC->SetIgnoreLookInput(true);

        // Mostramos pantalla según resultado
        if (PC == Winner)
        {
            PC->Client_ShowVictory();
        }
        else
        {
            PC->Client_ShowDefeat();
        }
        PC->Client_StartRestartCountdown(RestartDelay);
    }

    // Countdown para reiniciar — 5 segundos
    GetWorldTimerManager().SetTimer(
        RestartTimerHandle,
        this,
        &ALSGameMode::RestartMatch,
        5.f,
        false);
}
void ALSGameMode::RestartMatch()
{
    // Reiniciamos el nivel para todos
    GetWorld()->ServerTravel(
        TEXT("?listen"), false);
}
void ALSGameMode::OnMatchTimeUp()
{
    // Se acabó el tiempo — chequeamos sobrevivientes
    CheckVictoryCondition();
}