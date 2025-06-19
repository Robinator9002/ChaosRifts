// Copyright Robinator Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "ChaosCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputComponent;
class UInputAction;
class UAnimMontage;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogChaosCharacter, Log, All);

UENUM(BlueprintType)
enum class EMantleState : uint8
{
	None,
	Reaching,
	PushingForward
};

UCLASS(abstract)
class AChaosCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AChaosCharacter();

	//~ Animation Interface
	UFUNCTION(BlueprintCallable, Category = "Chaos|Animation")
	float GetSpeed() const;

	UFUNCTION(BlueprintCallable, Category = "Chaos|Animation")
	bool IsFalling() const;

	UFUNCTION(BlueprintCallable, Category = "Chaos|Animation")
	bool IsSliding() const { return bIsSliding; }

	UFUNCTION(BlueprintCallable, Category = "Chaos|Animation")
	bool IsVaulting() const { return bIsVaulting; }
    
	//~ End Animation Interface

protected:
	//~ Begin AActor Interface
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	//~ End AActor Interface
	
	//~ Begin APawn Interface
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	//~ End APawn Interface

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chaos|Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chaos|Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FollowCamera;
	
protected:
	// --- Input Actions ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chaos|Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chaos|Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chaos|Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chaos|Input")
	TObjectPtr<UInputAction> MouseLookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chaos|Input")
	TObjectPtr<UInputAction> DashAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chaos|Input")
    TObjectPtr<UInputAction> SlideAction;

	// --- Input Handlers ---
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void StartDash();
    void StartSlide();
    void StopSlide();

public:
	UFUNCTION(BlueprintCallable, Category="Chaos|Input")
	virtual void DoMove(float Right, float Forward);

	UFUNCTION(BlueprintCallable, Category="Chaos|Input")
	virtual void DoLook(float Yaw, float Pitch);

protected:
	// --- Core Movement Properties ---
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Chaos|Movement")
	float MovementSpeedDefault = 1000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Chaos|Movement")
	float JumpVelocityDefault = 800.f;

	// --- Dash Properties ---
	UPROPERTY(EditDefaultsOnly, Category = "Chaos|Movement|Dash")
	float DashImpulse = 4000.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Chaos|Movement|Dash")
	float DashCooldown = 1.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Chaos|Movement|Dash")
	float PostDashSpeedBoostMultiplier = 1.3f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Chaos|Movement|Dash")
	float PostDashSpeedBoostDuration = 1.0f;

	// --- Vaulting Properties ---
	UPROPERTY(EditDefaultsOnly, Category = "Chaos|Movement|Mantle")
	float MantleTraceDistance = 200.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Chaos|Movement|Mantle")
	float MinMantleHeight = 50.f;

	UPROPERTY(EditDefaultsOnly, Category = "Chaos|Movement|Mantle")
	float MaxMantleHeight = 200.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Chaos|Movement|Mantle")
	float MantleLerpSpeedNormal = 4.f;

	UPROPERTY(EditDefaultsOnly, Category = "Chaos|Movement|Mantle")
	float MantleLerpSpeedFast = 12.f;

	// This one is dynamically changing based on the speed before the Lerp
	float MantleLerpSpeed = MantleLerpSpeedNormal;

	UPROPERTY(EditDefaultsOnly, Category = "Chaos|Movement|Mantle")
	float MantleCooldownDuration = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Chaos|Movement|Mantle", meta = (ClampMin = "0.7", ClampMax = "1.0"))
	float MantleActivationDotProduct = 0.8f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Chaos|Movement|Mantle")
	float MantleMinExitSpeed = 400.f;

	UPROPERTY(EditDefaultsOnly, Category = "Chaos|Movement|Mantle")
	float MantleSpeedMultiplier = 1.2f;
    
    // NEU: Schwellenwert, ab welcher Geschwindigkeit der schnelle Vault genutzt wird
    UPROPERTY(EditDefaultsOnly, Category = "Chaos|Movement|Mantle")
    float MantleFastSpeedThreshold = 600.f;
    
    // --- Sliding Properties ---
    UPROPERTY(EditDefaultsOnly, Category = "Chaos|Movement|Slide")
    float SlideImpulse = 800.f;

    UPROPERTY(EditDefaultsOnly, Category = "Chaos|Movement|Slide")
    float SlideMinSpeed = 150.f;

    UPROPERTY(EditDefaultsOnly, Category = "Chaos|Movement|Slide", meta = (ClampMin = "0.1", ClampMax = "1.0"))
    float SlideCapsuleScale = 0.5f;

    // --- Animation Montages ---
    UPROPERTY(EditDefaultsOnly, Category = "Chaos|Animation")
    TObjectPtr<UAnimMontage> DashMontage;

    UPROPERTY(EditDefaultsOnly, Category = "Chaos|Animation")
    TObjectPtr<UAnimMontage> MantleMontage_Normal;
    
    // NEU: Eigene Montage für den schnellen Vault
    UPROPERTY(EditDefaultsOnly, Category = "Chaos|Animation")
    TObjectPtr<UAnimMontage> MantleMontage_Fast;
	
private:
	// --- Dash System ---
	void ResetDashCooldown();
	void ApplyPostDashSpeedBoost();
	void ResetMovementSpeed();
	
	bool bCanDash = true;
	FTimerHandle TimerHandle_DashCooldown;
	FTimerHandle TimerHandle_PostDashSpeed;

	// --- Vault System ---
	void TickVaultCheck(float DeltaTime);
	void PerformMantle(const FVector& LandingTarget, const FVector& LedgePosition);
	void TickMantle(float DeltaTime, float MantleLerpSpeed);
	void EndMantle();
	void ResetVaultCooldown();
	
	bool bIsVaulting = false;
	bool bCanCheckVault = true;
	FVector MantleTargetLocation;
	FVector MantleLedgeLocation;
	EMantleState CurrentMantleState = EMantleState::None;
    // NEU: Zeiger, um die gerade laufende Montage zu speichern
    TObjectPtr<UAnimMontage> CurrentMantleMontage;
	
	float ForwardInputValue = 0.f;
	float DefaultGravityScale = 1.f;
	FTimerHandle TimerHandle_VaultCooldown;
	
	float MantleExitSpeed = 0.f;
    
    // --- Sliding System ---
    bool bIsSliding = false;
    float DefaultCapsuleHalfHeight = 0.f;

public:
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};
