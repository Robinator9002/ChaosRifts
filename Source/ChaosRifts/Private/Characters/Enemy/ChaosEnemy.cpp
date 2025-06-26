// Copyright Robinator Studios, Inc. All Rights Reserved.

#include "Characters/Enemy/ChaosEnemy.h"
#include "Components/WidgetComponent.h" // For UWidgetComponent
#include "Components/ChaosAttributes.h" // For UChaosAttributes
#include "Kismet/GameplayStatics.h" // For Delayed Destroy
#include "GameFramework/CharacterMovementComponent.h" // For Movement Component
#include "Components/CapsuleComponent.h" // For Capsule Component

AChaosEnemy::AChaosEnemy()
{
	// Set this character to call Tick() every frame. You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create the Health Bar Widget Component
	HealthBarWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBarWidgetComponent"));
	HealthBarWidgetComponent->SetupAttachment(RootComponent);
	HealthBarWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen); // Display in screen space
	HealthBarWidgetComponent->SetDrawAtDesiredSize(true); // Auto-size the widget
	HealthBarWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision); // Widget shouldn't block anything
	HealthBarWidgetComponent->SetVisibility(false); // Hidden by default, show on damage or proximity
	HealthBarWidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, 100.f)); // Adjust offset as needed
}

void AChaosEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	// Bind to the OnHealthChanged delegate to show/hide health bar or update it
	if (AttributesComponent)
	{
		// Note: UChaosAttributes currently doesn't have an OnHealthChanged delegate.
		// We would typically bind to it here. For now, we'll handle visibility when damage is taken.
		// If you add a delegate like DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHealthChangedDelegate); in ChaosAttributes.h,
		// and broadcast it in ApplyHealthChange, you can bind to it here:
		// AttributesComponent->OnHealthChanged.AddDynamic(this, &AChaosEnemy::HandleHealthChanged);
	}
}

void AChaosEnemy::Die_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("Enemy '%s' has died!"), *GetNameSafe(this));

	// Call the base CharacterBase death logic (ragdoll, disable movement, etc.)
	Super::Die_Implementation();

	// Hide the health bar widget immediately
	SetHealthBarVisibility(false);

	// Disable collision on the mesh to prevent blocking after death (if not already handled by ragdoll)
	if (GetMesh())
	{
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// Destroy the actor after a short delay (e.g., after death animation/ragdoll settles)
	SetLifeSpan(5.0f); // Actor will be destroyed after 5 seconds
}

void AChaosEnemy::SetHealthBarVisibility(bool bVisible)
{
	if (HealthBarWidgetComponent)
	{
		HealthBarWidgetComponent->SetVisibility(bVisible);
	}
}
