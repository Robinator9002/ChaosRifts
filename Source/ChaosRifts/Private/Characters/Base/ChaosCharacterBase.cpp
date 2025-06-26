// Copyright Robinator Studios, Inc. All Rights Reserved.

#include "Characters/Base/ChaosCharacterBase.h"
#include "Components/ChaosAttributes.h" // For access to the attributes component
#include "GameFramework/CharacterMovementComponent.h" // For movement control
#include "Components/CapsuleComponent.h" // For collision control
#include "Kismet/GameplayStatics.h" // For useful helper functions like UGameplayStatics::ApplyDamage

// Sets default values
AChaosCharacterBase::AChaosCharacterBase()
{
 	// This character class doesn't need to tick on its own.
	// Ticking is handled by derived classes if needed.
	PrimaryActorTick.bCanEverTick = false;

	// Create the Attributes component. Now every character that inherits from this base class
	// will automatically have this component.
	AttributesComponent = CreateDefaultSubobject<UChaosAttributes>(TEXT("AttributesComponent"));
}

float AChaosCharacterBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	// Call the base implementation to allow other components to process TakeDamage.
	// The processed damage amount is the return value of the base class.
	const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	// If we don't have an Attributes component, we can't take damage.
	if (!AttributesComponent)
	{
		return 0.0f;
	}

	// Apply the actual damage to the character's health.
	AttributesComponent->ApplyHealthChange(-ActualDamage); // Damage is a negative change

	// Check if health has reached 0 or less.
	if (AttributesComponent->GetHealth() <= 0.0f)
	{
		Die(); // Call the death function
	}

	return ActualDamage; // Return the actual damage applied
}

void AChaosCharacterBase::Die_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("Character '%s' has died!"), *GetNameSafe(this));

	// Optional: Disable collision and movement
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->StopMovementImmediately();
		GetCharacterMovement()->DisableMovement();
		GetCharacterMovement()->SetComponentTickEnabled(false); // Stop movement component from ticking
	}
	if (GetCapsuleComponent())
	{
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	// Mesh can activate PhysX or a ragdoll here
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	GetMesh()->SetSimulatePhysics(true);

	// Broadcast the OnDeath delegate so other systems can react.
	OnDeath.Broadcast(this);
}

void AChaosCharacterBase::StartAttack()
{
	// Base implementation: A default attack animation could be played here,
	// or logic that applies to all attacks.
	// Derived classes like AChaosCharacter will override this
	// to implement specific attacks.
	UE_LOG(LogTemp, Log, TEXT("%s starts a generic attack."), *GetNameSafe(this));
}
