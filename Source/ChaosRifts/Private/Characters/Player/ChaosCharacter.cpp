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

DEFINE_LOG_CATEGORY(LogChaosCharacter);

AChaosCharacter::AChaosCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Character rotation is independent of controller rotation
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Orient character to movement direction
	GetCharacterMovement()->bOrientRotationToMovement = true; 
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	// Set base movement values from default variables
	GetCharacterMovement()->JumpZVelocity = JumpVelocityDefault;
	GetCharacterMovement()->MaxWalkSpeed = MovementSpeedDefault;
	GetCharacterMovement()->AirControl = 2.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 50.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create and configure camera boom
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 500.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// Create and configure follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
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
	if (!bCanDash)
	{
		return;
	}

	bCanDash = false;
	
	LaunchCharacter(GetActorForwardVector() * DashImpulse, true, true);
	ApplyPostDashSpeedBoost();

	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle_DashCooldown,
		this,
		&AChaosCharacter::ResetDashCooldown,
		DashCooldown,
		false
	);
}

void AChaosCharacter::ApplyPostDashSpeedBoost()
{
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_PostDashSpeed);
	
	GetCharacterMovement()->MaxWalkSpeed = MovementSpeedDefault * PostDashSpeedBoostMultiplier;
	
	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle_PostDashSpeed,
		this,
		&AChaosCharacter::ResetMovementSpeed,
		PostDashSpeedBoostDuration,
		false
	);
}

void AChaosCharacter::ResetMovementSpeed()
{
	GetCharacterMovement()->MaxWalkSpeed = MovementSpeedDefault;
}

void AChaosCharacter::ResetDashCooldown()
{
	bCanDash = true;
}
