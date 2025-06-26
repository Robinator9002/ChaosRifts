// Copyright Robinator Studios, Inc. All Rights Reserved.

#include "Characters/Enemy/ChaosEnemyMelee.h"
#include "Kismet/GameplayStatics.h" // For ApplyDamage
#include "GameFramework/CharacterMovementComponent.h" // For checking movement
#include "Components/CapsuleComponent.h" // For character dimensions
#include "Animation/AnimInstance.h" // For playing montages

AChaosEnemyMelee::AChaosEnemyMelee()
{
	// Set this character to call Tick() every frame. You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true; // Enemies might need Tick for AI behaviors
}

void AChaosEnemyMelee::BeginPlay()
{
	Super::BeginPlay();
	// Any specific setup for melee enemies
}

void AChaosEnemyMelee::StartAttack()
{
	// Only proceed if the enemy can attack and is not vaulting (if vaulting is a shared feature)
	// Assuming melee enemies won't vault during an attack
	if (!bCanAttack)
	{
		return;
	}

	if (MeleeAttackMontage)
	{
		PlayAnimMontage(MeleeAttackMontage);

		// Set cooldown based on animation length
		bCanAttack = false;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_AttackCooldown, this, &AChaosEnemyMelee::ResetAttackCooldown, MeleeAttackMontage->GetPlayLength() * 0.9f, false);
		// Cooldown set to 90% of animation length to allow for some recovery before next attack

		// --- Hitbox Check ---
		// This simplified hitbox fires immediately. In a real game, this might be triggered by an AnimNotify.
		FVector StartLocation = GetActorLocation() + GetActorForwardVector() * GetCapsuleComponent()->GetScaledCapsuleRadius();
		FVector EndLocation = StartLocation + GetActorForwardVector() * MeleeAttackRange;
		
		TArray<FHitResult> HitResults;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this); // Ignore the enemy itself

		// Define the type of damage event (can be customized later, e.g., UMeleeDamageType::StaticClass())
		TSubclassOf<UDamageType> DamageTypeClass = UDamageType::StaticClass();

		bool bHit = GetWorld()->SweepMultiByChannel(
			HitResults,
			StartLocation,
			EndLocation,
			FQuat::Identity,
			ECC_Pawn, // Check for other Pawns (which includes AChaosCharacter and other enemies)
			FCollisionShape::MakeSphere(MeleeAttackRadius),
			QueryParams
		);

		if (bHit)
		{
			for (const FHitResult& Hit : HitResults)
			{
				// Attempt to cast to AChaosCharacterBase to ensure we hit a valid combatant
				AChaosCharacterBase* HitCharacter = Cast<AChaosCharacterBase>(Hit.GetActor());
				if (HitCharacter && HitCharacter != this) // Ensure we don't hit ourselves
				{
					UGameplayStatics::ApplyDamage(
						HitCharacter,
						MeleeDamage,
						GetController(),
						this,
						DamageTypeClass
					);
					UE_LOG(LogTemp, Log, TEXT("Enemy Melee attack hit: %s"), *GetNameSafe(HitCharacter));
				}
			}
		}

		// Optional: Debug visualization of the attack area
		// DrawDebugSphere(GetWorld(), (StartLocation + EndLocation) / 2.0f, MeleeAttackRadius, 16, FColor::Yellow, false, MeleeAttackMontage->GetPlayLength(), 0, 5.0f);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("MeleeAttackMontage not assigned for enemy %s"), *GetNameSafe(this));
	}
}

void AChaosEnemyMelee::ResetAttackCooldown()
{
	bCanAttack = true;
}
