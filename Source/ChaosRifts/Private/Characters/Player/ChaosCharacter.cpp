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
#include "Kismet/KismetSystemLibrary.h" // Needed for Trace functions

DEFINE_LOG_CATEGORY(LogChaosCharacter);

AChaosCharacter::AChaosCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

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
	GetCharacterMovement()->BrakingDecelerationWalking = 4000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 2500.0f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 500.0f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
}

void AChaosCharacter::BeginPlay()
{
	Super::BeginPlay();
	DefaultGravityScale = GetCharacterMovement()->GravityScale;
}


void AChaosCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsVaulting)
	{
		TickMantle(DeltaTime);
	}
	else if (bCanCheckVault)
	{
		TickVaultCheck(DeltaTime);
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
	this->ForwardInputValue = Forward;
	
	if (Controller != nullptr && !bIsVaulting)
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
	if (!bCanDash || bIsVaulting) { return; }
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
	if (!GetCharacterMovement()->IsFalling() || ForwardInputValue < 0.1f)
	{
		return;
	}
    
    // NEU: Prüfen, ob der Spieler in die Richtung schaut, in die er sich bewegt.
    const FVector CameraDirection = FollowCamera->GetForwardVector().GetSafeNormal();
    const FVector ActorDirection = GetActorForwardVector().GetSafeNormal();
    if (FVector::DotProduct(CameraDirection, ActorDirection) < VaultActivationDotProduct)
    {
        return; // Der Spieler schaut zu weit weg, kein Vault.
    }

	const FVector ActorLocation = GetActorLocation();
	const FVector ForwardVector = GetActorForwardVector();
	const float CapsuleRadius = GetCapsuleComponent()->GetScaledCapsuleRadius();
	const float CapsuleHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	FHitResult FrontHit, LedgeHit, LandingSpaceHit;

	// 1. & 2. TRACE: Ist eine Wand vor uns UND ist darüber Platz?
	FVector FrontTraceStart = ActorLocation;
	FrontTraceStart.Z -= CapsuleHalfHeight / 2;
	const FVector FrontTraceEnd = FrontTraceStart + ForwardVector * VaultTraceDistance;
	if (!UKismetSystemLibrary::LineTraceSingle(this, FrontTraceStart, FrontTraceEnd, UEngineTypes::ConvertToTraceType(ECC_WorldStatic), false, {}, EDrawDebugTrace::None, FrontHit, true))
	{
		return;
	}

	const FVector UpperTraceStart = ActorLocation + FVector(0, 0, CapsuleHalfHeight / 1.5f);
	if (UKismetSystemLibrary::LineTraceSingle(this, UpperTraceStart, UpperTraceStart + ForwardVector * VaultTraceDistance, UEngineTypes::ConvertToTraceType(ECC_WorldStatic), false, {}, EDrawDebugTrace::None, LandingSpaceHit, true))
	{
		return;
	}

	// 3. TRACE: Finde die genaue Kantenhöhe
	const FVector LedgeTraceStart = FVector(FrontHit.ImpactPoint.X, FrontHit.ImpactPoint.Y, ActorLocation.Z + MaxVaultHeight) + ForwardVector * 15.f;
	if (!UKismetSystemLibrary::LineTraceSingle(this, LedgeTraceStart, LedgeTraceStart - FVector(0,0, MaxVaultHeight-MinVaultHeight), UEngineTypes::ConvertToTraceType(ECC_WorldStatic), false, {}, EDrawDebugTrace::None, LedgeHit, true))
	{
		return;
	}

	// 4. TRACE: Ist genug Platz zum Landen?
	const FVector LandingLocation = LedgeHit.ImpactPoint + (ForwardVector * CapsuleRadius * 1.5f) + FVector(0, 0, CapsuleHalfHeight + 5.f);
	if (UKismetSystemLibrary::CapsuleTraceSingle(this, LandingLocation, LandingLocation, CapsuleRadius, CapsuleHalfHeight, UEngineTypes::ConvertToTraceType(ECC_WorldStatic), false, {}, EDrawDebugTrace::None, LandingSpaceHit, true))
	{
		return;
	}
	
	PerformMantle(LandingLocation, LedgeHit.ImpactPoint);
}

void AChaosCharacter::PerformMantle(const FVector& LandingTarget, const FVector& LedgePosition)
{
	bIsVaulting = true;
	CurrentMantleState = EMantleState::Reaching;
	MantleTargetLocation = LandingTarget;

	// NEU: Eingehende Geschwindigkeit für den Boost nach dem Vault speichern
    const FVector HorizontalVelocity = GetCharacterMovement()->Velocity;
    MantleExitSpeed = HorizontalVelocity.Size2D(); // Size2D ignoriert die Z-Achse (Fallen/Springen)
	
	// Erstelle den ersten Punkt des Mantles: hoch zur Kante
	MantleLedgeLocation = LedgePosition;
	MantleLedgeLocation.Z += GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	MantleLedgeLocation += GetActorForwardVector() * 5.f;

	GetCharacterMovement()->SetMovementMode(MOVE_Flying);
	GetCharacterMovement()->Velocity = FVector::ZeroVector;
	GetCharacterMovement()->GravityScale = 0.0f;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Ignore);

	UE_LOG(LogChaosCharacter, Warning, TEXT("MANTLE GESTARTET!"));
}

void AChaosCharacter::TickMantle(float DeltaTime)
{
	FVector CurrentTarget;
	
	if(CurrentMantleState == EMantleState::Reaching)
	{
		CurrentTarget = MantleLedgeLocation;
		if(FVector::DistSquared(GetActorLocation(), CurrentTarget) < 100.f)
		{
			CurrentMantleState = EMantleState::PushingForward;
		}
	}
	else // PushingForward
	{
		CurrentTarget = MantleTargetLocation;
		if(FVector::DistSquared(GetActorLocation(), CurrentTarget) < 100.f)
		{
			EndMantle();
			return;
		}
	}
	
	const FVector NewLocation = FMath::VInterpTo(GetActorLocation(), CurrentTarget, DeltaTime, VaultLerpSpeed);
	SetActorLocation(NewLocation);
}

void AChaosCharacter::EndMantle()
{
	bIsVaulting = false;
	CurrentMantleState = EMantleState::None;
	
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	GetCharacterMovement()->GravityScale = DefaultGravityScale;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
    
	// Starte Cooldown
	bCanCheckVault = false;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle_VaultCooldown, this, &AChaosCharacter::ResetVaultCooldown, VaultCooldownDuration, false);
}

void AChaosCharacter::ResetVaultCooldown()
{
	bCanCheckVault = true;
}
