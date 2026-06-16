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

    UPROPERTY(ReplicatedUsing = OnRep_HeartsLeft, BlueprintReadOnly, Category = "Lives")
    int32 HeartsLeft = 3;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lives")
    int32 MaxHearts = 3;

    UPROPERTY(EditDefaultsOnly, Category = "Movement")
    float BallBrakingDeceleration = 50.f;

    UPROPERTY(EditDefaultsOnly, Category = "Movement")
    float BallGroundFriction = 5.f;

    UPROPERTY()
    bool bIsInvincible = false;

    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    float InvincibilityDuration = 2.f;

    FTimerHandle InvincibilityHandle;

    UPROPERTY(ReplicatedUsing = OnRep_bIsDead, BlueprintReadOnly, Category = "State")
    bool bIsDead = false;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "HUD")
    TSubclassOf<ULSDeathWidget> DeathWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    float DashImpulseStrength = 1800.f;

    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    float KnockbackStrength = 250000.f;

    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    float KnockbackRadius = 120.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ball")
    UStaticMeshComponent* BallMesh;

    UPROPERTY(ReplicatedUsing = OnRep_PlayerColor, BlueprintReadOnly, Category = "Ball")
    FLinearColor PlayerColor = FLinearColor::White;

    UPROPERTY(ReplicatedUsing = OnRep_PlayerMaterial)
    int32 MaterialSlotIndex = -1;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ball")
    USphereComponent* BallCollision;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    class ACameraActor* LevelCamera;

    bool IsAlive() const { return !bIsDead; }

    void LoseHeart();

    void ApplyPlayerMaterial();
    void ApplyPlayerColor();
    void ApplyKnockbackLogic(ALSCharacter* OtherChar, const FVector& KnockDir, float ImpulseScale);

    UFUNCTION()
    void OnRep_PlayerMaterial();

    UFUNCTION()
    void OnRep_PlayerColor();

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_Dash();

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_ApplyKnockback(ALSCharacter* OtherChar, const FVector& KnockDir, float ImpulseScale);

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_UpdateHearts(int32 PlayerIdx, int32 NewHearts);

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlayDeathFX();

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlayRespawnFX();

    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_PlayDashFX();

    UFUNCTION()
    void OnBallOverlap(
        UPrimitiveComponent* OverlappedComp,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult);

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void PossessedBy(AController* NewController) override;
    virtual void OnRep_Controller() override;
    virtual void FellOutOfWorld(const UDamageType& DmgType) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION()
    void OnRep_HeartsLeft();

    UFUNCTION()
    void OnRep_bIsDead();

    void HandleDeath();
    void HandleRespawn();
    void DoRespawn();
    void OnDashPressed();

    ALSPlayerController* GetLocalPlayerController() const;

    bool bCanDash = true;

    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    float DashCooldown = 1.5f;

    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    float RespawnDelay = 3.f;

    FTimerHandle DashCooldownHandle;
    FTimerHandle RespawnHandle;
};
