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
    
    // Invencibilidad post-respawn
    UPROPERTY()
    bool bIsInvincible = false;
    
    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    float InvincibilityDuration = 2.f;
    
    FTimerHandle InvincibilityHandle;
    
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
    void Multicast_UpdateHearts_Implementation(int32 PlayerIdx, int32 NewHearts);

    // --- Server RPCs ---
    UFUNCTION(Server, Reliable, WithValidation)
    void Server_Dash();

    // --- Multicast RPCs ---
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_UpdateHearts(int32 PlayerIdx, int32 NewHearts);
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlayDeathFX();

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlayRespawnFX();

    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_PlayDashFX();
    
    // Mesh visual de la pelota
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ball")
    UStaticMeshComponent* BallMesh;
    
    UPROPERTY(ReplicatedUsing = OnRep_PlayerColor, BlueprintReadOnly, Category = "Ball")
    FLinearColor PlayerColor = FLinearColor::White;
    
    UPROPERTY(ReplicatedUsing = OnRep_PlayerMaterial)
    int32 MaterialSlotIndex = -1;

    UFUNCTION()
    void OnRep_PlayerMaterial();

    void OnRep_Controller();
    void ApplyPlayerMaterial();
    UFUNCTION()
    void OnRep_PlayerColor();

    void ApplyPlayerColor();
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
    void PossessedBy(AController* NewController);

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