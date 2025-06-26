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
#include "Kismet/KismetSystemLibrary.h"
#include "Animation/AnimInstance.h"
#include "Kismet/GameplayStatics.h" // For ApplyDamage
#include "Components/ChaosAttributes.h" // For accessing Chaos resource
#include "Core/ChaosGameMode.h" // CORRECTED: For GameMode access to handle Game Over
#include "Characters/Enemy/ChaosEnemy.h" // ADDED: To recognize AChaosEnemy type in melee attack


// NO CHANGES ARE NEEDED IN THIS FILE (Original user comment, adapted here)
// The include path above correctly finds the header.
// All "Super::" calls are polymorphic and will correctly call the new base class functions.

DEFINE_LOG_CATEGORY(LogChaosCharacter);

AChaosCharacter::AChaosCharacter()
{
	// NOTE: The base class constructor is called automatically before this one.
	// The AttributesComponent is already created by AChaosCharacterBase.

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
	GetCharacterMovement()->bCanWalkOffLedgesWhenCrouching = true;

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
	Super::BeginPlay(); // Now calls AChaosCharacterBase::BeginPlay()
	DefaultGravityScale = GetCharacterMovement()->GravityScale;

	// Bind the OnDeath delegate for the player character to a specific handler
	// The delegate requires a function that takes an AChaosCharacterBase* parameter.
	OnDeath.AddDynamic(this, &AChaosCharacter::HandlePlayerDeath); 
}

void AChaosCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime); // Now calls AChaosCharacterBase::Tick()

	if (bIsVaulting)
	{
		TickMantle(DeltaTime);
	}
	else if (bCanCheckVault)
	{
		TickVaultCheck(DeltaTime);
	}

	UAnimInstance *AnimInstance = GetMesh()->GetAnimInstance();
	if (DashMontage && AnimInstance && AnimInstance->Montage_IsPlaying(DashMontage))
	{
		if (GetCharacterMovement()->Velocity.SizeSquared2D() < FMath::Square(1.0f))
		{
			AnimInstance->Montage_Stop(0.2f, DashMontage);
		}
	}
}

void AChaosCharacter::SetupPlayerInputComponent(UInputComponent *PlayerInputComponent)
{
	// NOTE: Super::SetupPlayerInputComponent is not called here because ACharacter's implementation is empty.
	// If AChaosCharacterBase had an implementation, we would call it.

	if (UEnhancedInputComponent *EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Input bindings for movement
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AChaosCharacter::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AChaosCharacter::Look);
		EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Started, this, &AChaosCharacter::StartDash);

		// Input binding for the melee attack
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &AChaosCharacter::AttackMelee);

		// Added: Input binding for spell casting
		EnhancedInputComponent->BindAction(CastSpellAction, ETriggerEvent::Started, this, &AChaosCharacter::StartSpellCast);
	}
	else
	{
		UE_LOG(LogChaosCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component!"), *GetNameSafe(this));
	}
}

void AChaosCharacter::Move(const FInputActionValue &Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();
	DoMove(MovementVector.X, MovementVector.Y);
}

void AChaosCharacter::Look(const FInputActionValue &Value)
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
	if (!bCanDash || bIsVaulting)
	{
		return;
	}

	if (DashMontage)
	{
		PlayAnimMontage(DashMontage);
	}

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

// --- Mantle System ---
void AChaosCharacter::TickVaultCheck(float DeltaTime)
{
	if (!GetCharacterMovement()->IsFalling() || ForwardInputValue < 0.1f)
	{
		return;
	}

	const FVector CameraDirection = FollowCamera->GetForwardVector().GetSafeNormal();
	const FVector ActorDirection = GetActorForwardVector().GetSafeNormal();
	if (FVector::DotProduct(CameraDirection, ActorDirection) < MantleActivationDotProduct)
	{
		return;
	}

	const FVector ActorLocation = GetActorLocation();
	const FVector ForwardVector = GetActorForwardVector();
	const float CapsuleRadius = GetCapsuleComponent()->GetScaledCapsuleRadius();
	const float CapsuleHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	FHitResult FrontHit, LedgeHit, LandingSpaceHit;

	FVector FrontTraceStart = ActorLocation;
	FrontTraceStart.Z -= CapsuleHalfHeight / 2;
	const FVector FrontTraceEnd = FrontTraceStart + ForwardVector * MantleTraceDistance;
	if (!UKismetSystemLibrary::LineTraceSingle(this, FrontTraceStart, FrontTraceEnd, UEngineTypes::ConvertToTraceType(ECC_WorldStatic), false, {}, EDrawDebugTrace::None, FrontHit, true))
	{
		return;
	}

	const FVector UpperTraceStart = ActorLocation + FVector(0, 0, CapsuleHalfHeight / 1.5f);
	if (UKismetSystemLibrary::LineTraceSingle(this, UpperTraceStart, UpperTraceStart + ForwardVector * MantleTraceDistance, UEngineTypes::ConvertToTraceType(ECC_WorldStatic), false, {}, EDrawDebugTrace::None, LandingSpaceHit, true))
	{
		return;
	}

	const FVector LedgeTraceStart = FVector(FrontHit.ImpactPoint.X, FrontHit.ImpactPoint.Y, ActorLocation.Z + MaxMantleHeight) + ForwardVector * 15.f;
	if (!UKismetSystemLibrary::LineTraceSingle(this, LedgeTraceStart, LedgeTraceStart - FVector(0, 0, MaxMantleHeight - MinMantleHeight), UEngineTypes::ConvertToTraceType(ECC_WorldStatic), false, {}, EDrawDebugTrace::None, LedgeHit, true))
	{
		return;
	}

	const FVector LandingLocation = LedgeHit.ImpactPoint + (ForwardVector * CapsuleRadius * 1.5f) + FVector(0, 0, CapsuleHalfHeight + 5.f);
	if (UKismetSystemLibrary::CapsuleTraceSingle(this, LandingLocation, LandingLocation, CapsuleRadius, CapsuleHalfHeight, UEngineTypes::ConvertToTraceType(ECC_WorldStatic), false, {}, EDrawDebugTrace::None, LandingSpaceHit, true))
	{
		return;
	}

	PerformMantle(LandingLocation, LedgeHit.ImpactPoint);
}

void AChaosCharacter::PerformMantle(const FVector &LandingTarget, const FVector &LedgePosition)
{
	UAnimMontage *MontageToPlay = GetSpeed() > MantleFastSpeedThreshold ? MantleMontage_Fast : MantleMontage_Normal;
	if (MontageToPlay)
	{
		CurrentMantleMontage = MontageToPlay;
		PlayAnimMontage(CurrentMantleMontage);
	}

	bIsVaulting = true;
	MantleLerpSpeed = GetSpeed() > MantleFastSpeedThreshold ? MantleLerpSpeedFast : MantleLerpSpeedNormal;
	CurrentMantleState = EMantleState::Reaching;
	MantleTargetLocation = LandingTarget;

	MantleLedgeLocation = LedgePosition;
	MantleLedgeLocation.Z += GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	MantleLedgeLocation += GetActorForwardVector() * 5.f;

	GetCharacterMovement()->SetMovementMode(MOVE_Flying);
	GetCharacterMovement()->Velocity = FVector::ZeroVector;
	GetCharacterMovement()->GravityScale = 0.0f;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Ignore);
}

void AChaosCharacter::TickMantle(float DeltaTime)
{
	FVector CurrentTarget;

	if (CurrentMantleState == EMantleState::Reaching)
	{
		CurrentTarget = MantleLedgeLocation;
		if (FVector::DistSquared(GetActorLocation(), CurrentTarget) < 100.f)
		{
			CurrentMantleState = EMantleState::PushingForward;
		}
	}
	else // PushingForward
	{
		CurrentTarget = MantleTargetLocation;
		UAnimInstance *AnimInstance = GetMesh()->GetAnimInstance();
		if (FVector::DistSquared(GetActorLocation(), CurrentTarget) < 100.f || (CurrentMantleMontage && AnimInstance && !AnimInstance->Montage_IsPlaying(CurrentMantleMontage)))
		{
			EndMantle();
			return;
		}
	}

	const FVector NewLocation = FMath::VInterpTo(GetActorLocation(), CurrentTarget, DeltaTime, MantleLerpSpeed);
	SetActorLocation(NewLocation);
}

void AChaosCharacter::EndMantle()
{
	if (!bIsVaulting)
		return;

	bIsVaulting = false;
	CurrentMantleMontage = nullptr;
	CurrentMantleState = EMantleState::None;

	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	GetCharacterMovement()->GravityScale = DefaultGravityScale;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);

	bCanCheckVault = false;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle_VaultCooldown, this, &AChaosCharacter::ResetVaultCooldown, MantleCooldownDuration, false);
}

void AChaosCharacter::ResetVaultCooldown()
{
	bCanCheckVault = true;
}

// --- Melee System ---
void AChaosCharacter::AttackMelee()
{
	// Check if the character can attack (e.g., no other attack active or cooldown)
	if (!bCanAttack || bIsVaulting)
	{
		return;
	}

	// Check if an attack animation is assigned
	if (MeleeAttackMontage)
	{
		// Play the attack animation
		PlayAnimMontage(MeleeAttackMontage);

		// Disable attack capability for the duration of a cooldown
		bCanAttack = false;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_AttackCooldown, this, &AChaosCharacter::ResetAttackCooldown, MeleeAttackMontage->GetPlayLength() * 0.8f, false);
		// The cooldown is set to 80% of the animation length here for a fluid feel
		// and to prevent immediate new attacks.
		// This can later be replaced with Animation Notifies for precise hitbox timing.

		// Perform a simple sphere trace to find enemies in melee range and deal damage.
		// This is a very simplified hitbox, which should be refined later with Animation Notifies.
		FVector StartLocation = GetActorLocation() + GetActorForwardVector() * GetCapsuleComponent()->GetScaledCapsuleRadius();
		FVector EndLocation = StartLocation + GetActorForwardVector() * 150.0f; // Range of the attack
		float AttackRadius = 75.0f; // Radius of the hitbox

		TArray<FHitResult> HitResults;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this); // Ignore the player character itself

		bool bHit = GetWorld()->SweepMultiByChannel(
			HitResults,
			StartLocation,
			EndLocation,
			FQuat::Identity,
			ECC_Pawn, // Check collisions with other Pawns (characters)
			FCollisionShape::MakeSphere(AttackRadius),
			QueryParams
		);

		if (bHit)
		{
			for (const FHitResult& Hit : HitResults)
			{
				// Attempt to cast to AChaosEnemy to ensure we only hit enemies
				AChaosEnemy* HitEnemy = Cast<AChaosEnemy>(Hit.GetActor());
				if (HitEnemy) // Only deal damage if it's an enemy
				{
					UGameplayStatics::ApplyDamage(
						HitEnemy, // Apply damage to the enemy
						MeleeDamage,
						GetController(),
						this,
						UDamageType::StaticClass() // Generic damage type, can be customized later
					);
					UE_LOG(LogChaosCharacter, Log, TEXT("Player Melee attack hit: %s"), *GetNameSafe(HitEnemy));
				}
			}
		}
		
		// Optional: Debug visualization of the attack area
		// DrawDebugSphere(GetWorld(), (StartLocation + EndLocation) / 2.0f, AttackRadius, 16, FColor::Red, false, MeleeAttackMontage->GetPlayLength(), 0, 5.0f);
	}
	else
	{
		UE_LOG(LogChaosCharacter, Warning, TEXT("MeleeAttackMontage not assigned for %s"), *GetNameSafe(this));
	}
}

// Added: Implementation of the function to reset the attack cooldown
void AChaosCharacter::ResetAttackCooldown()
{
	bCanAttack = true;
}

// --- Spell Casting System ---
void AChaosCharacter::StartSpellCast()
{
	// Check if the character can cast a spell and has enough Chaos
	if (!bCanCastSpell || bIsVaulting || !AttributesComponent || AttributesComponent->GetChaos() < SpellChaosCost)
	{
		if (AttributesComponent && AttributesComponent->GetChaos() < SpellChaosCost)
		{
			UE_LOG(LogChaosCharacter, Log, TEXT("Not enough Chaos to cast spell! Current: %f, Cost: %f"), AttributesComponent->GetChaos(), SpellChaosCost);
		}
		return;
	}

	// Play casting animation
	if (SpellCastMontage)
	{
		PlayAnimMontage(SpellCastMontage);
	}
	else
	{
		UE_LOG(LogChaosCharacter, Warning, TEXT("SpellCastMontage not assigned for %s"), *GetNameSafe(this));
	}

	// Spend Chaos resource
	AttributesComponent->ApplyChaosChange(-SpellChaosCost);

	// Set spell cast cooldown
	bCanCastSpell = false;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle_SpellCastCooldown, this, &AChaosCharacter::ResetSpellCastCooldown, SpellCastMontage ? SpellCastMontage->GetPlayLength() : 1.0f, false);
	// If no montage, use a default cooldown of 1.0f

	// --- Spawn Spell Projectile (placeholder) ---
	if (SpellProjectileClass)
	{
		// Spawn location: a bit in front of the character's mesh
		FVector SpawnLocation = GetMesh()->GetSocketLocation(TEXT("MuzzleSocket")); // Assuming a socket named "MuzzleSocket" on your character's mesh
		if (SpawnLocation == FVector::ZeroVector) // Fallback if socket doesn't exist
		{
			SpawnLocation = GetActorLocation() + GetActorForwardVector() * 100.f + FVector(0,0,50.f);
		}
		
		FRotator SpawnRotation = GetControlRotation(); // Projectile goes where the player is looking

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = GetInstigator();

		AActor* Projectile = GetWorld()->SpawnActor<AActor>(SpellProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);
		if (Projectile)
		{
			UE_LOG(LogChaosCharacter, Log, TEXT("Spawned spell projectile: %s"), *GetNameSafe(Projectile));
			// You would typically add ProjectileMovementComponent and damage logic to the SpellProjectileClass itself
		}
	}
	else
	{
		UE_LOG(LogChaosCharacter, Warning, TEXT("SpellProjectileClass not assigned for %s"), *GetNameSafe(this));
	}
}

// Added: Implementation of the function to reset the spell cast cooldown
void AChaosCharacter::ResetSpellCastCooldown()
{
	bCanCastSpell = true;
}

// --- Player Death Handling ---
// This function is called when the OnDeath delegate is broadcast, which itself is called from TakeDamage when health <= 0.
// It has the same signature as the delegate.
void AChaosCharacter::HandlePlayerDeath(AChaosCharacterBase* DeadCharacter)
{
    // Make sure it's actually *this* character that died, not some other character that happened to be bound.
    if (DeadCharacter != this)
    {
        return;
    }

	// Call base class death logic (ragdoll, disable movement, etc.)
	// The Die_Implementation function itself does not have a parameter, it's the delegate binding that requires it.
	// So we call our own Die_Implementation here.
	// NOTE: We are intentionally NOT calling Super::Die_Implementation() here directly.
	// The delegate OnDeath is broadcast from AChaosCharacterBase::Die_Implementation(),
	// so the base logic for ragdoll, etc., has already been handled *before* this function is called.
	// This function primarily handles the *game-specific consequences* of player death.

	UE_LOG(LogChaosCharacter, Display, TEXT("Player Character has died. Game Over!"));

	// You can add more complex game over logic here:
	// - Show a "Game Over" UI widget
	// - Restart the level
	// - Go to a main menu
	
	// For now, let's just restart the current level as a basic "Game Over".
	// This will typically be handled by the GameMode or PlayerController in a real game.
	AChaosGameMode* GameMode = Cast<AChaosGameMode>(GetWorld()->GetAuthGameMode());
	if (GameMode)
	{
		// Later, you might have a dedicated "RestartLevel" or "GameOver" function in GameMode
		// For now, we simulate a simple restart.
		GetWorld()->ServerTravel(GetWorld()->GetMapName(), false); // This works in editor and packaged builds
	}
	else
	{
		UE_LOG(LogChaosCharacter, Error, TEXT("Could not cast to ChaosGameMode to handle game over."));
	}
}

// --- Overridden Die_Implementation for AChaosCharacter ---
// This function needs to be implemented because it's a virtual override from AChaosCharacterBase.
// It will be called as part of the normal UFunction mechanism when Die() is invoked,
// which in turn calls OnDeath.Broadcast().
void AChaosCharacter::Die_Implementation()
{
	// Always call the base class implementation first.
	// This ensures that the generic death logic (like activating ragdoll and broadcasting OnDeath)
	// from AChaosCharacterBase is executed.
	Super::Die_Implementation();

	// Any *additional* player-specific death behavior that should happen *immediately*
	// when Die() is called (before or in parallel with delegate subscribers) could go here.
	// For example, playing a specific sound effect only for the player's death,
	// or setting a direct flag on the game state.
	// Given that 'HandlePlayerDeath' is already set up via the delegate for game over logic,
	// this function can be minimal, primarily serving to satisfy the linker and the 'override' specifier.
	// The game restart logic is intentionally left in HandlePlayerDeath as it's triggered
	// by the delegate, which provides a clean separation of concerns.
}


//~ Animation Interface
float AChaosCharacter::GetSpeed() const
{
	return GetCharacterMovement()->Velocity.Size2D();
}

bool AChaosCharacter::IsFalling() const
{
	return GetCharacterMovement()->IsFalling();
}
