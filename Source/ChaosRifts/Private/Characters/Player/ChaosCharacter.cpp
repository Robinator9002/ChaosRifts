// Copyright Robinator Studios, Inc. All Rights Reserved.

#include "Characters/Player/ChaosCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetSystemLibrary.h" // Needed for SphereTrace

DEFINE_LOG_CATEGORY(LogChaosCharacter);

AChaosCharacter::AChaosCharacter()
{
	PrimaryActorTick.bCanEverTick = true; // We need tick for the vault check

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true; 
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	GetCharacterMovement()->JumpZVelocity = JumpVelocityDefault;
	GetCharacterMovement()->MaxWalkSpeed = MovementSpeedDefault;
	GetCharacterMovement()->AirControl = 2.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 50.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 4000.f; // Increased for less sliding
	GetCharacterMovement()->BrakingDecelerationFalling = 2500.0f; // Increased for less sliding on land

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 500.0f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
}

void AChaosCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Perform the vault check every frame
	TickVaultCheck(DeltaTime);

	// If we are currently vaulting, smoothly interpolate to the target location
	if (bIsVaulting)
	{
		SetActorLocation(FMath::VInterpTo(GetActorLocation(), VaultTargetLocation, DeltaTime, VaultLerpSpeed));

		// Check if we've reached the destination
		if (FVector::DistSquared(GetActorLocation(), VaultTargetLocation) < 100.f) // Use squared distance for efficiency
		{
			bIsVaulting = false;
			GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		}
	}
}

void AChaosCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AChaosCharacter::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AChaosCharacter::Look);
		
		EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Started, this, &AChaosCharacter::StartDash);
	}
	else
	{
		UE_LOG(LogChaosCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component!"), *GetNameSafe(this));
	}
}

void AChaosCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();
	DoMove(MovementVector.X, MovementVector.Y);
}

void AChaosCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void AChaosCharacter::DoMove(float Right, float Forward)
{
	// Cache the forward input value for other systems like vaulting
	this->ForwardInputValue = Forward;
	
	if (Controller != nullptr)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void AChaosCharacter::DoLook(float Yaw, float Pitch)
{
	if (Controller != nullptr)
	{
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

// --- Dash System ---

void AChaosCharacter::StartDash()
{
	if (!bCanDash) { return; }
	bCanDash = false;
	LaunchCharacter(GetActorForwardVector() * DashImpulse, true, true);
	ApplyPostDashSpeedBoost();
	GetWorld()->GetTimerManager().SetTimer(TimerHandle_DashCooldown, this, &AChaosCharacter::ResetDashCooldown, DashCooldown, false);
}

void AChaosCharacter::ApplyPostDashSpeedBoost()
{
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_PostDashSpeed);
	GetCharacterMovement()->MaxWalkSpeed = MovementSpeedDefault * PostDashSpeedBoostMultiplier;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle_PostDashSpeed, this, &AChaosCharacter::ResetMovementSpeed, PostDashSpeedBoostDuration, false);
}

void AChaosCharacter::ResetMovementSpeed()
{
	GetCharacterMovement()->MaxWalkSpeed = MovementSpeedDefault;
}

void AChaosCharacter::ResetDashCooldown()
{
	bCanDash = true;
}

// --- Vault System ---

void AChaosCharacter::TickVaultCheck(float DeltaTime)
{
	// Conditions to check for vault:
	// 1. Not already vaulting.
	// 2. Is in the air (falling).
	// 3. Is moving forward (has forward input).
	if (bIsVaulting || !GetCharacterMovement()->IsFalling() || ForwardInputValue < 0.1f)
	{
		return;
	}

	const FVector Start = GetActorLocation();
	const FVector End = Start + GetActorForwardVector() * VaultTraceDistance;
	FHitResult FrontHit;

	// Step 1: Forward Trace to detect a wall.
	if (UKismetSystemLibrary::SphereTraceSingle(this, Start, End, 20.f, UEngineTypes::ConvertToTraceType(ECC_WorldStatic), false, {}, EDrawDebugTrace::ForDuration, FrontHit, true))
	{
		const FVector LedgeStart = FrontHit.ImpactPoint + FVector(0, 0, MaxVaultHeight) + (GetActorForwardVector() * 50.f);
		const FVector LedgeEnd = LedgeStart - FVector(0, 0, MaxVaultHeight - MinVaultHeight);
		FHitResult LedgeHit;

		// Step 2: Downward Trace to find the top surface of the obstacle.
		if (UKismetSystemLibrary::LineTraceSingle(this, LedgeStart, LedgeEnd, UEngineTypes::ConvertToTraceType(ECC_WorldStatic), false, {}, EDrawDebugTrace::ForDuration, LedgeHit, true))
		{
			// Step 3: Check if there's enough space to land.
			const FVector LandingLocation = LedgeHit.ImpactPoint + (GetActorForwardVector() * 50.f);
			FHitResult LandingSpaceHit; // Declare a variable for the OutHit parameter.
			if (!UKismetSystemLibrary::SphereTraceSingle(this, LandingLocation, LandingLocation - FVector(0,0,50), 20.f, UEngineTypes::ConvertToTraceType(ECC_WorldStatic), false, {}, EDrawDebugTrace::ForDuration, LandingSpaceHit, true))
			{
				PerformVault(GetActorLocation(), LandingLocation);
			}
		}
	}
}

void AChaosCharacter::PerformVault(const FVector& VaultStart, const FVector& VaultEnd)
{
	bIsVaulting = true;
	VaultTargetLocation = VaultEnd;
	GetCharacterMovement()->SetMovementMode(MOVE_Flying); // Use flying mode for smooth interpolation
}
