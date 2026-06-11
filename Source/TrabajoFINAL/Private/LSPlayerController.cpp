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

class ALSPlayerState;

ALSPlayerController::ALSPlayerController()
{
}
void ALSPlayerController::BeginPlay()
{
	Super::BeginPlay();
	bAutoManageActiveCameraTarget = false;  // <- primero esto
	SetInputMode(FInputModeGameOnly());
	bShowMouseCursor = false;
	if (IsLocalController())
	{
		FTimerHandle CamHandle;
		GetWorldTimerManager().SetTimer(CamHandle, [this]()
		{
			TArray<AActor*> Cams;
			UGameplayStatics::GetAllActorsWithTag(
				GetWorld(), FName("LevelCamera"), Cams);
			if (Cams.Num() > 0)
			{
				ACameraActor* LevelCam =
					Cast<ACameraActor>(Cams[0]);
				if (LevelCam)
					SetViewTargetWithBlend(LevelCam, 0.f);
			}
		}, 0.5f, false);
		
		if (HUDWidgetClass)
		{
			HUDWidget = CreateWidget<ULSHUDWidget>(
				this, HUDWidgetClass);
			if (HUDWidget)
			{
				HUDWidget->AddToViewport();
			}
		}
		// Removemos el timer de InitializeHUD
		// ahora lo llama el servidor via Client_InitializeHUD
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
	GetWorldTimerManager().SetTimer(
		TimerHandle_Rot,
		[this, NewRotation]()
		{
			SetControlRotation(NewRotation);
			if (APawn* MyPawn = GetPawn())
				MyPawn->SetActorRotation(NewRotation);
		},
		0.3f, false);
}
void ALSPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
}

// --- Client RPCs ---

void ALSPlayerController::Client_ShowVictory_Implementation()
{
	if (EndGameWidgetClass)
	{
		EndGameWidget = CreateWidget<ULSEndGameWidget>(
			this, EndGameWidgetClass);
		if (EndGameWidget)
		{
			EndGameWidget->AddToViewport(10);
			EndGameWidget->SetupAsVictory();
		}
	}
	BP_ShowVictoryScreen();
}

void ALSPlayerController::Client_ShowDefeat_Implementation()
{
	if (EndGameWidgetClass)
	{
		EndGameWidget = CreateWidget<ULSEndGameWidget>(
			this, EndGameWidgetClass);
		if (EndGameWidget)
		{
			EndGameWidget->AddToViewport(10);
			EndGameWidget->SetupAsDefeat();
		}
	}
	BP_ShowDefeatScreen();
}
void ALSPlayerController::UpdateHUDHearts(
	int32 PlayerIdx, int32 HeartsLeft)
{
	if (HUDWidget)
	{
		HUDWidget->UpdatePlayerHearts(PlayerIdx, HeartsLeft);
	}
}

void ALSPlayerController::UpdateHUDPlayersAlive(int32 PlayersAlive)
{
	if (HUDWidget)
	{
		HUDWidget->UpdatePlayersAlive(PlayersAlive);
	}
}

void ALSPlayerController::UpdateHUDKills(
	int32 PlayerIdx, int32 Kills)
{
	if (HUDWidget)
	{
		HUDWidget->UpdatePlayerKills(PlayerIdx, Kills);
	}
}

void ALSPlayerController::Client_UpdatePlayerHearts_Implementation(int32 SlotIndex, int32 HeartsLeft)
{
}

void ALSPlayerController::UpdateHUDTimer(float NewTime)
{
	if (HUDWidget)
	{
		HUDWidget->UpdateMatchTime(NewTime);
	}
}
void ALSPlayerController::Client_ShowDeathScreen_Implementation()
{
	UE_LOG(LogTemp, Warning, 
		TEXT("Client_ShowDeathScreen recibido"));
	UE_LOG(LogTemp, Warning, 
		TEXT("DeathWidgetClass valido: %s"),
		DeathWidgetClass ? TEXT("SI") : TEXT("NO"));

	if (DeathWidgetClass)
	{
		DeathWidget = CreateWidget<ULSDeathWidget>(
			this, DeathWidgetClass);
        
		UE_LOG(LogTemp, Warning,
			TEXT("Widget creado: %s"),
			DeathWidget ? TEXT("SI") : TEXT("NO"));

		if (DeathWidget)
		{
			DeathWidget->AddToViewport(5);
			DeathWidget->StartRespawnCountdown(3.f);
		}
	}
	BP_ShowDeathScreen();
}

void ALSPlayerController::Client_StartRestartCountdown_Implementation(
	float Seconds)
{
	if (EndGameWidget)
	{
		EndGameWidget->StartCountdown(Seconds);
	}
	BP_StartRestartCountdown(Seconds);
}

void ALSPlayerController::Client_InitializeHUD_Implementation()
{
    // No usamos bHUDInitialized para permitir reintentos
    if (HUDWidget)
    {
        InitializeHUD();
    }
    else
    {
        FTimerHandle InitHandle;
        GetWorldTimerManager().SetTimer(
            InitHandle,
            this,
            &ALSPlayerController::InitializeHUD,
            0.5f,
            false);
    }
}

void ALSPlayerController::InitializeHUD()
{
    if (!HUDWidget) return;

    AGameStateBase* GS = GetWorld()->GetGameState();
    if (!GS) return;

    // Verificamos si todos los slots están asignados
    bool bAllReady = true;
    for (APlayerState* PS : GS->PlayerArray)
    {
        ALSPlayerState* LSPS = Cast<ALSPlayerState>(PS);
        if (LSPS && LSPS->HUDSlotIndex == -1)
        {
            bAllReady = false;
            break;
        }
    }

    // Si no están listos reintentamos en 0.5s
    if (!bAllReady)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("InitializeHUD — slots no listos, reintentando..."));
        FTimerHandle RetryHandle;
        GetWorldTimerManager().SetTimer(
            RetryHandle,
            this,
            &ALSPlayerController::InitializeHUD,
            0.5f,
            false);
        return;
    }

    // Ocultamos todos los slots primero
    for (int32 i = 0; i < 4; i++)
    {
        HUDWidget->SetPlayerSlotVisible(i, false);
    }

    TArray<FLinearColor> PlayerColors = {
        FLinearColor(0.1f, 0.4f, 0.9f, 1.f),
        FLinearColor(0.9f, 0.2f, 0.1f, 1.f),
        FLinearColor(0.13f, 0.99f, 0.1f, 1.f),
        FLinearColor(0.95f, 0.1f, 0.9f, 1.f)
    };

    TArray<FString> PlayerNames = {
        TEXT("Jugador 1"),
        TEXT("Jugador 2"),
        TEXT("Jugador 3"),
        TEXT("Jugador 4")
    };

    UE_LOG(LogTemp, Warning,
        TEXT("=== InitializeHUD | Controller=%s | PlayerArray=%d ==="),
        *GetName(), GS->PlayerArray.Num());

    for (APlayerState* PS : GS->PlayerArray)
    {
        ALSPlayerState* LSPS = Cast<ALSPlayerState>(PS);
        if (!LSPS) continue;

        UE_LOG(LogTemp, Warning,
            TEXT("  >> Player=%s | Slot=%d | Lives=%d"),
            *LSPS->GetPlayerName(),
            LSPS->HUDSlotIndex,
            LSPS->LivesLeft);

        int32 Idx = LSPS->HUDSlotIndex;
        if (Idx < 0 || Idx >= 4) continue;

        HUDWidget->SetPlayerSlotVisible(Idx, true);

        ULSPlayerHUDWidget* PlayerHUD =
            HUDWidget->GetPlayerHUD(Idx);
        if (PlayerHUD)
        {
            PlayerHUD->InitPlayer(
                Idx, PlayerNames[Idx], PlayerColors[Idx]);
            PlayerHUD->SetHearts(LSPS->LivesLeft);
        }
    }

    // Marcamos como inicializado solo cuando todo salió bien
    bHUDInitialized = true;
}