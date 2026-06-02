#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LSPlayerHUDWidget.generated.h"

UCLASS()
class TRABAJOFINAL_API ULSPlayerHUDWidget : public UUserWidget
{
    GENERATED_BODY()

public:

    // --- Referencias a los widgets del designer ---
    // Se bindean automáticamente por nombre con meta=(BindWidget)
    
    UPROPERTY(EditAnywhere, Category = "Hearts")
    UTexture2D* HeartFullTexture;

    UPROPERTY(EditAnywhere, Category = "Hearts")
    UTexture2D* HeartEmptyTexture;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* TXT_PlayerName;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* TXT_Kills;

    UPROPERTY(meta = (BindWidget))
    class UImage* Heart1;

    UPROPERTY(meta = (BindWidget))
    class UImage* Heart2;

    UPROPERTY(meta = (BindWidget))
    class UImage* Heart3;

    // --- Variables de configuración ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
    int32 PlayerIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
    int32 MaxHearts = 3;

    // Color del jugador (azul, rojo, amarillo, violeta)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
    FLinearColor PlayerColor = FLinearColor::White;

    // --- Funciones públicas llamadas desde WBP_HUD ---
    
    UFUNCTION(BlueprintCallable, Category = "HUD")
    void SetPlayerName(const FString& Name);

    UFUNCTION(BlueprintCallable, Category = "HUD")
    void SetHearts(int32 HeartsLeft);

    UFUNCTION(BlueprintCallable, Category = "HUD")
    void SetKills(int32 Kills);

    // Marca al jugador como eliminado (oscurece todo el widget)
    UFUNCTION(BlueprintCallable, Category = "HUD")
    void SetEliminated();

    // Inicializa el widget con datos del jugador
    UFUNCTION(BlueprintCallable, Category = "HUD")
    void InitPlayer(int32 Index, const FString& Name,
                    FLinearColor Color);

protected:
    virtual void NativeConstruct() override;

    // Actualiza el tinte de un corazón individual
    void UpdateHeartColor(class UImage* Heart, bool bFull);

    // Colores de corazón
    UPROPERTY(EditDefaultsOnly, Category = "HUD")
    FLinearColor HeartFullColor  = FLinearColor(1.0f, 1.0f, 1.0f, 1.f);

    UPROPERTY(EditDefaultsOnly, Category = "HUD")
    FLinearColor HeartEmptyColor = FLinearColor(0.15f, 0.15f, 0.15f, 0.4f);

    // Estado interno
    int32 CurrentHearts = 3;
    bool bIsEliminated  = false;
};