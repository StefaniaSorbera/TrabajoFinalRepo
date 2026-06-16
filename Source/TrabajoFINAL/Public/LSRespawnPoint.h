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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    int32 PlayerIndex = -1;

    FVector GetSpawnLocation() const;

protected:
#if WITH_EDITORONLY_DATA
    UPROPERTY(VisibleAnywhere, Category = "Components")
    class USphereComponent* EditorSphere;
#endif
};
