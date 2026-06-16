#include "LSPlayerController.h"
#include "LSPlayerHUDWidget.h"
#include "LSHUDWidget.h"
#include "LSEndGameWidget.h"
#include "LSDeathWidget.h"
#include "LSPlayerState.h"
#include "Camera/CameraActor.h"
#include "GameFramework/Character.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"

ALSPlayerController::ALSPlayerController()
{
}

void ALSPlayerController::BeginPlay()
{
    Super::BeginPlay();
    bAutoManageActiveCameraTarget = false;
    SetInputMode(FInputModeGameOnly());
    bShowMouseCursor = false;

    if (!IsLocalController()) return;

    FTimerHandle CamHandle;
    GetWorldTimerManager().SetTimer(CamHandle, [this]()
    {
        TArray<AActor*> Cams;
        UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("LevelCamera"), Cams);
        if (Cams.Num() > 0)
        {
            if (ACameraActor* LevelCam = Cast<ACameraActor>(Cams[0]))
                SetViewTargetWithBlend(LevelCam, 0.f);
        }
    }, 0.5f, false);

    if (HUDWidgetClass)
    {
        HUDWidget = CreateWidget<ULSHUDWidget>(this, HUDWidgetClass);
        if (HUDWidget)
            HUDWidget->AddToViewport();
    }
}

void ALSPlayerController::ClientForceRotation_Implementation(FRotator NewRotation)
{
    SetControlRotation(NewRotation);

    if (APawn* MyPawn = GetPawn())
    {
        MyPawn->SetActorRotation(NewRotation);
        if (ACharacter* Char = Cast<ACharacter>(MyPawn))
            Char->bUseControllerRotationYaw = true;
    }

    FTimerHandle TimerHandle_Rot;
    GetWorldTimerManager().SetTimer(TimerHandle_Rot, [this, NewRotation]()
    {
        SetControlRotation(NewRotation);
        if (APawn* MyPawn = GetPawn())
            MyPawn->SetActorRotation(NewRotation);
    }, 0.3f, false);
}

void ALSPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
}

ULSEndGameWidget* ALSPlayerController::CreateAndShowEndGameWidget()
{
    if (!EndGameWidgetClass) return nullptr;
    EndGameWidget = CreateWidget<ULSEndGameWidget>(this, EndGameWidgetClass);
    if (EndGameWidget)
        EndGameWidget->AddToViewport(10);
    return EndGameWidget;
}

void ALSPlayerController::Client_ShowVictory_Implementation()
{
    if (ULSEndGameWidget* Widget = CreateAndShowEndGameWidget())
        Widget->SetupAsVictory();
    BP_ShowVictoryScreen();
}

void ALSPlayerController::Client_ShowDefeat_Implementation()
{
    if (ULSEndGameWidget* Widget = CreateAndShowEndGameWidget())
        Widget->SetupAsDefeat();
    BP_ShowDefeatScreen();
}

void ALSPlayerController::UpdateHUDHearts(int32 PlayerIdx, int32 HeartsLeft)
{
    if (HUDWidget)
        HUDWidget->UpdatePlayerHearts(PlayerIdx, HeartsLeft);
}

void ALSPlayerController::UpdateHUDPlayersAlive(int32 PlayersAlive)
{
    if (HUDWidget)
        HUDWidget->UpdatePlayersAlive(PlayersAlive);
}

void ALSPlayerController::UpdateHUDKills(int32 PlayerIdx, int32 Kills)
{
    if (HUDWidget)
        HUDWidget->UpdatePlayerKills(PlayerIdx, Kills);
}

void ALSPlayerController::Client_UpdatePlayerHearts_Implementation(int32 SlotIndex, int32 HeartsLeft)
{
}

void ALSPlayerController::UpdateHUDTimer(float NewTime)
{
    if (HUDWidget)
        HUDWidget->UpdateMatchTime(NewTime);
}

void ALSPlayerController::Client_ShowDeathScreen_Implementation()
{
    if (!DeathWidgetClass) return;
    DeathWidget = CreateWidget<ULSDeathWidget>(this, DeathWidgetClass);
    if (DeathWidget)
    {
        DeathWidget->AddToViewport(5);
        DeathWidget->StartRespawnCountdown(3.f);
    }
    BP_ShowDeathScreen();
}

void ALSPlayerController::Client_StartRestartCountdown_Implementation(float Seconds)
{
    if (EndGameWidget)
        EndGameWidget->StartCountdown(Seconds);
    BP_StartRestartCountdown(Seconds);
}

void ALSPlayerController::Client_InitializeHUD_Implementation()
{
    if (HUDWidget)
        InitializeHUD();
    else
    {
        FTimerHandle InitHandle;
        GetWorldTimerManager().SetTimer(InitHandle, this, &ALSPlayerController::InitializeHUD, 0.5f, false);
    }
}

void ALSPlayerController::InitializeHUD()
{
    if (!HUDWidget) return;

    AGameStateBase* GS = GetWorld()->GetGameState();
    if (!GS) return;

    for (APlayerState* PS : GS->PlayerArray)
    {
        ALSPlayerState* LSPS = Cast<ALSPlayerState>(PS);
        if (LSPS && LSPS->HUDSlotIndex == -1)
        {
            FTimerHandle RetryHandle;
            GetWorldTimerManager().SetTimer(RetryHandle, this, &ALSPlayerController::InitializeHUD, 0.5f, false);
            return;
        }
    }

    for (int32 i = 0; i < 4; i++)
        HUDWidget->SetPlayerSlotVisible(i, false);

    const TArray<FLinearColor> PlayerColors = {
        FLinearColor(0.1f,  0.4f,  0.9f,  1.f),
        FLinearColor(0.9f,  0.2f,  0.1f,  1.f),
        FLinearColor(0.13f, 0.99f, 0.1f,  1.f),
        FLinearColor(0.95f, 0.1f,  0.9f,  1.f)
    };

    const TArray<FString> PlayerNames = {
        TEXT("Jugador 1"), TEXT("Jugador 2"),
        TEXT("Jugador 3"), TEXT("Jugador 4")
    };

    for (APlayerState* PS : GS->PlayerArray)
    {
        ALSPlayerState* LSPS = Cast<ALSPlayerState>(PS);
        if (!LSPS) continue;

        int32 Idx = LSPS->HUDSlotIndex;
        if (Idx < 0 || Idx >= 4) continue;

        HUDWidget->SetPlayerSlotVisible(Idx, true);

        if (ULSPlayerHUDWidget* PlayerHUD = HUDWidget->GetPlayerHUD(Idx))
        {
            PlayerHUD->InitPlayer(Idx, PlayerNames[Idx], PlayerColors[Idx]);
            PlayerHUD->SetHearts(LSPS->LivesLeft);
        }
    }

    bHUDInitialized = true;
}
