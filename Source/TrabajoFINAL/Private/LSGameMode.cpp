#include "LSGameMode.h"
#include "LSGameState.h"
#include "LSPlayerState.h"
#include "LSPlayerController.h"
#include "LSCharacter.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

ALSGameMode::ALSGameMode()
{
    GameStateClass        = ALSGameState::StaticClass();
    PlayerStateClass      = ALSPlayerState::StaticClass();
    PlayerControllerClass = ALSPlayerController::StaticClass();
    DefaultPawnClass      = ALSCharacter::StaticClass();
}

AActor* ALSGameMode::FindPlayerStart_Implementation(AController* Player, const FString& IncomingName)
{
    ALSPlayerState* PS = Player->GetPlayerState<ALSPlayerState>();
    FString SpawnTag = FString::Printf(TEXT("Spawn%d"), PS ? PS->HUDSlotIndex : 0);

    TArray<AActor*> SpawnPoints;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), SpawnPoints);

    for (AActor* SpawnPoint : SpawnPoints)
    {
        if (SpawnPoint->ActorHasTag(FName(*SpawnTag)))
            return SpawnPoint;
    }

    return Super::FindPlayerStart_Implementation(Player, IncomingName);
}

void ALSGameMode::BeginPlay()
{
    Super::BeginPlay();

    if (ALSGameState* GS = GetGameState<ALSGameState>())
    {
        GS->SetMatchState(EMatchState::InProgress);
        GS->PlayersAlive = GetNumPlayers();
        GS->SetMatchTime(MatchDuration);
    }

    GetWorldTimerManager().SetTimer(MatchTickHandle, [this]()
    {
        ALSGameState* GS = GetGameState<ALSGameState>();
        if (!GS) return;
        float NewTime = FMath::Max(0.f, GS->MatchTime - 1.f);
        GS->SetMatchTime(NewTime);
        if (NewTime <= 0.f)
        {
            GetWorldTimerManager().ClearTimer(MatchTickHandle);
            OnMatchTimeUp();
        }
    }, 1.f, true);

    GetWorldTimerManager().SetTimer(AssignSlotsHandle, this, &ALSGameMode::AssignHUDSlots, 1.f, false);

    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        if (APlayerController* PC = It->Get())
            PC->SetControlRotation(FRotator::ZeroRotator);
    }
}

void ALSGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    FTimerHandle Handle;
    GetWorldTimerManager().SetTimer(Handle, [this]() { AssignHUDSlots(); }, 0.3f, false);

    NewPlayer->SetControlRotation(FRotator::ZeroRotator);

    if (APawn* Pawn = NewPlayer->GetPawn())
    {
        Pawn->SetActorRotation(FRotator::ZeroRotator);
        if (ALSPlayerController* PC = Cast<ALSPlayerController>(NewPlayer))
            PC->ClientForceRotation(FRotator::ZeroRotator);
    }
}

void ALSGameMode::PlayerDied(AController* DeadPlayer, AController* Killer)
{
    if (!HasAuthority()) return;

    if (Killer && Killer != DeadPlayer)
    {
        if (ALSPlayerState* KillerPS = Killer->GetPlayerState<ALSPlayerState>())
        {
            KillerPS->AddKill();
            if (ALSPlayerController* KillerPC = Cast<ALSPlayerController>(Killer))
                KillerPC->BP_UpdateKillCount(KillerPS->KillCount);
        }
    }

    if (ALSPlayerState* DeadPS = DeadPlayer->GetPlayerState<ALSPlayerState>())
        DeadPS->SetPlayerDead();

    if (ALSGameState* GS = GetGameState<ALSGameState>())
        GS->DecrementPlayersAlive();

    if (ALSPlayerController* DeadPC = Cast<ALSPlayerController>(DeadPlayer))
        DeadPC->Client_ShowDefeat();

    CheckVictoryCondition();
}

void ALSGameMode::CheckVictoryCondition()
{
    TArray<AController*> AlivePlayers;

    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        ALSPlayerController* PC = Cast<ALSPlayerController>(It->Get());
        if (!PC) continue;
        ALSPlayerState* PS = PC->GetPlayerState<ALSPlayerState>();
        if (PS && PS->IsAlive())
            AlivePlayers.Add(PC);
    }

    if (AlivePlayers.Num() == 1)
        EndMatch(AlivePlayers[0]);
    else if (AlivePlayers.Num() == 0)
        EndMatch(nullptr);
}

void ALSGameMode::AssignHUDSlots()
{
    int32 SlotIndex = 0;

    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        ALSPlayerController* PC = Cast<ALSPlayerController>(It->Get());
        if (!PC) continue;

        ALSPlayerState* PS = PC->GetPlayerState<ALSPlayerState>();
        if (!PS) continue;

        PS->HUDSlotIndex = SlotIndex;

        if (PlayerMaterials.IsValidIndex(SlotIndex))
        {
            if (ALSCharacter* Char = Cast<ALSCharacter>(PC->GetPawn()))
            {
                Char->MaterialSlotIndex = SlotIndex;
                Char->ApplyPlayerMaterial();
            }
        }

        SlotIndex++;
    }

    if (ALSGameState* GS = GetGameState<ALSGameState>())
        GS->SetPlayersAlive(SlotIndex);

    FTimerHandle NotifyHandle;
    GetWorldTimerManager().SetTimer(NotifyHandle, [this]()
    {
        for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
        {
            if (ALSPlayerController* PC = Cast<ALSPlayerController>(It->Get()))
                PC->Client_InitializeHUD();
        }
    }, 0.5f, false);
}

void ALSGameMode::EndMatch(AController* Winner)
{
    GetWorldTimerManager().ClearTimer(MatchTickHandle);

    if (ALSGameState* GS = GetGameState<ALSGameState>())
        GS->SetMatchState(EMatchState::PostGame);

    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        ALSPlayerController* PC = Cast<ALSPlayerController>(It->Get());
        if (!PC) continue;

        PC->SetIgnoreMoveInput(true);
        PC->SetIgnoreLookInput(true);

        if (ALSCharacter* Char = Cast<ALSCharacter>(PC->GetPawn()))
        {
            Char->GetCharacterMovement()->StopMovementImmediately();
            Char->GetCharacterMovement()->DisableMovement();
            Char->GetCharacterMovement()->GravityScale = 0.f;
            Char->SetActorEnableCollision(false);
        }

        if (PC == Winner)
            PC->Client_ShowVictory();
        else
            PC->Client_ShowDefeat();

        PC->Client_StartRestartCountdown(RestartDelay);
    }

    GetWorldTimerManager().SetTimer(RestartTimerHandle, this, &ALSGameMode::RestartMatch, 5.f, false);
}

void ALSGameMode::RestartMatch()
{
    GetWorld()->ServerTravel(TEXT("?listen"), false);
}

void ALSGameMode::OnMatchTimeUp()
{
    CheckVictoryCondition();
}
