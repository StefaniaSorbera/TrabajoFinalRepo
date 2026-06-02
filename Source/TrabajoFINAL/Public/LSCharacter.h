#pragma once
#include "Camera/CameraActor.h"
#include "CoreMinimal.h"
#include "TrabajoFINALCharacter.h"
#include "LSCharacter.generated.h"

class ULSDeathWidget;
class USphereComponent;
class ULSHUDWidget;
class ALSPlayerController;

UCLASS()
class TRABAJOFINAL_API ALSCharacter : public ATrabajoFINALCharacter
{
    GENERATED_BODY()

public:
    ALSCharacter();

    // --- Corazones (vidas) ---
    UPROPERTY(ReplicatedUsing = OnRep_HeartsLeft,
        BlueprintReadOnly, Category = "Lives")
    int32 HeartsLeft = 3;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lives")
    int32 MaxHearts = 3;
    
    UPROPERTY(EditDefaultsOnly, Category = "Movement")
    float BallBrakingDeceleration = 50.f;

    UPROPERTY(EditDefaultsOnly, Category = "Movement")
    float BallGroundFriction = 5.f;
    // --- Estado ---
    UPROPERTY(ReplicatedUsing = OnRep_bIsDead,
        BlueprintReadOnly, Category = "State")
    bool bIsDead = false;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "HUD")
    TSubclassOf<ULSDeathWidget> DeathWidgetClass;

    // --- Dash / Empuje ---
    // Fuerza del impulso al hacer dash
    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    float DashImpulseStrength = 1800.f;

    // Fuerza de knockback al colisionar con otro jugador
    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    float KnockbackStrength = 250000.f;

    // Radio de detección de colisión para empuje
    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    float KnockbackRadius = 120.f;

    bool IsAlive() const { return !bIsDead; }

    // Llamado desde FellOutOfWorld
    void LoseHeart();

    // --- Server RPCs ---
    UFUNCTION(Server, Reliable, WithValidation)
    void Server_Dash();

    // --- Multicast RPCs ---
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlayDeathFX();

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlayRespawnFX();

    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_PlayDashFX();
    
    // Mesh visual de la pelota
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ball")
    UStaticMeshComponent* BallMesh;

    // Colisión de rebote entre jugadores
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ball")
    USphereComponent* BallCollision;

    // POR ESTAS:
    void ApplyKnockbackLogic(ALSCharacter* OtherChar, const FVector& KnockDir, float ImpulseScale);

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_ApplyKnockback(ALSCharacter* OtherChar, const FVector& KnockDir, float ImpulseScale);
    
    // Llamado cuando la esfera toca otro jugador
    UFUNCTION()
    void OnBallOverlap(UPrimitiveComponent* OverlappedComp,
                       AActor* OtherActor,
                       UPrimitiveComponent* OtherComp,
                       int32 OtherBodyIndex,
                       bool bFromSweep,
                       const FHitResult& SweepResult);
// Referencia a la cámara del nivel
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
class ACameraActor* LevelCamera;
protected:
    virtual void BeginPlay() override;

    // RepNotify corazones → actualiza HUD
    UFUNCTION()
    void OnRep_HeartsLeft();

    // RepNotify muerte → activa efectos
    UFUNCTION()
    void OnRep_bIsDead();
    void Tick(float DeltaTime);

    void HandleDeath();
    void HandleRespawn();
    void DoRespawn();

    // Input
    void OnDashPressed();

    // Cooldown del dash
    bool bCanDash = true;

    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    float DashCooldown = 1.5f;

    FTimerHandle DashCooldownHandle;
    FTimerHandle RespawnHandle;

    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    float RespawnDelay = 3.f;

    virtual void FellOutOfWorld(
        const UDamageType& DmgType) override;

    virtual void GetLifetimeReplicatedProps(
        TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};