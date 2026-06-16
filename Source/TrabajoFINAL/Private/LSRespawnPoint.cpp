#include "LSRespawnPoint.h"
#include "Components/SphereComponent.h"

ALSRespawnPoint::ALSRespawnPoint()
{
    PrimaryActorTick.bCanEverTick = false;

#if WITH_EDITORONLY_DATA
    EditorSphere = CreateDefaultSubobject<USphereComponent>(TEXT("EditorSphere"));
    RootComponent = EditorSphere;
    EditorSphere->SetSphereRadius(40.f);
    EditorSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    EditorSphere->SetHiddenInGame(true);
#endif
}

FVector ALSRespawnPoint::GetSpawnLocation() const
{
    return GetActorLocation() + FVector(0.f, 0.f, 100.f);
}
