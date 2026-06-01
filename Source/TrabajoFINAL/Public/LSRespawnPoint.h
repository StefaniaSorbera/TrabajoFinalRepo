#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LSRespawnPoint.generated.h"

UCLASS()
class TRABAJOFINAL_API ALSRespawnPoint : public AActor
{
	GENERATED_BODY()

public:
	ALSRespawnPoint();

	// Índice del jugador al que pertenece este spawn
	// -1 = cualquier jugador
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	int32 PlayerIndex = -1;

	// Retorna la ubicación de spawn con offset vertical
	FVector GetSpawnLocation() const;

protected:
#if WITH_EDITORONLY_DATA
	// Esfera visible solo en editor para ubicar el punto
	UPROPERTY(VisibleAnywhere, Category = "Components")
	class USphereComponent* EditorSphere;
#endif
};