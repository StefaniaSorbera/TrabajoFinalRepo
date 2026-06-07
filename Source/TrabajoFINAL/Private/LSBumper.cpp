#include "LSBumper.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "LSCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

ALSBumper::ALSBumper()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

    // Mesh visual
    BumperMesh = CreateDefaultSubobject<UStaticMeshComponent>(
        TEXT("BumperMesh"));
    RootComponent = BumperMesh;
    BumperMesh->SetCollisionEnabled(
        ECollisionEnabled::NoCollision);

    // Volumen de colisión
    BumperVolume = CreateDefaultSubobject<UBoxComponent>(
        TEXT("BumperVolume"));
    BumperVolume->SetupAttachment(RootComponent);
    BumperVolume->SetBoxExtent(FVector(50.f, 50.f, 100.f));
    BumperVolume->SetCollisionEnabled(
        ECollisionEnabled::QueryOnly);
    BumperVolume->SetCollisionResponseToAllChannels(
        ECR_Ignore);
    BumperVolume->SetCollisionResponseToChannel(
        ECC_Pawn, ECR_Overlap);
}

    void ALSBumper::BeginPlay()
    {
        Super::BeginPlay();

        // Guardamos la escala original
        DefaultScale = BumperMesh->GetRelativeScale3D();

        if (HasAuthority())
        {
            BumperVolume->OnComponentBeginOverlap.AddDynamic(
                this, &ALSBumper::OnBumperOverlap);
        }
    }


void ALSBumper::OnBumperOverlap(
    UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    ALSCharacter* Character =
        Cast<ALSCharacter>(OtherActor);

    if (!Character || !Character->IsAlive()) return;

    FVector Velocity =
        Character->GetCharacterMovement()->Velocity;

    // Encontramos el punto más cercano en la superficie
    // del box al jugador
    FVector CharLoc = Character->GetActorLocation();
    FVector ClosestPoint;

    BumperVolume->GetClosestPointOnCollision(
        CharLoc, ClosestPoint);

    // Normal desde la superficie hacia el jugador
    FVector SurfaceNormal =
        (CharLoc - ClosestPoint).GetSafeNormal();

    // Si la normal es casi cero usamos la dirección
    // desde el centro como fallback
    if (SurfaceNormal.IsNearlyZero())
    {
        SurfaceNormal =
            (CharLoc - GetActorLocation()).GetSafeNormal();
    }

    // Ignoramos el componente Z para rebote horizontal
    SurfaceNormal.Z = 0.f;
    SurfaceNormal.Normalize();

    // Reflejamos la velocidad contra la normal real
    FVector Reflected =
        Velocity - 2.f *
        FVector::DotProduct(Velocity, SurfaceNormal) *
        SurfaceNormal;

    // Aplicamos fuerza mínima para que siempre rebote
    float ReflectedSpeed = Reflected.Size2D();
    if (ReflectedSpeed < 600.f)
    {
        Reflected = FVector(Reflected.GetSafeNormal2D().X * 600.f,
                     Reflected.GetSafeNormal2D().Y * 600.f,
                     0.f);
    }

    FVector FinalImpulse =
     Reflected.GetSafeNormal2D() * ImpulseStrength;

    // Sin componente Z — solo horizontal
    FinalImpulse.Z = 0.f;

    Character->GetCharacterMovement()->Velocity =
        FVector::ZeroVector;
    Character->GetCharacterMovement()->AddImpulse(
        FinalImpulse, true);
    
    // Al final de OnBumperOverlap, después del AddImpulse:
    PlayHitEffect();
}

void ALSBumper::PlayHitEffect()
{
    Multicast_PlayHitEffect();
}

void ALSBumper::Multicast_PlayHitEffect_Implementation()
{
    BumperMesh->SetRelativeScale3D(HitScale);

    GetWorldTimerManager().SetTimer(
        HitEffectHandle,
        this,
        &ALSBumper::ResetScale,
        HitEffectDuration,
        false);
}

void ALSBumper::ResetScale()
{
    BumperMesh->SetRelativeScale3D(DefaultScale);
}