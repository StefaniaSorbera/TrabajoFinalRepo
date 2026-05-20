// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LSGameMode.generated.h"

UCLASS()
class TRABAJOFINAL_API ALSGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    ALSGameMode();

    // Llamado cuando un jugador muere
    void PlayerDied(AController* DeadPlayer, AController* Killer);

    // Llamado cuando se acaba el tiempo (opcional)
    void OnMatchTimeUp();

protected:
    virtual void BeginPlay() override;

    // Chequea si queda un solo jugador vivo
    void CheckVictoryCondition();

    // Termina la partida y define al ganador
    void EndMatch(AController* Winner);

    // Tiempo límite de partida en segundos
    UPROPERTY(EditDefaultsOnly, Category = "Match")
    float MatchDuration = 180.f;

    FTimerHandle MatchTimerHandle;
};
