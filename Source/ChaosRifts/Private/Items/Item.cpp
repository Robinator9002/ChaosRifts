// Copyright Robinator Studios, Inc. All Rights Reserved.

#include "Items/Item.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h" // Include the Static Mesh Component header

AItem::AItem()
{
	PrimaryActorTick.bCanEverTick = false;

	// Create the static mesh component and set it as the root.
	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
	RootComponent = ItemMesh;
}

void AItem::BeginPlay()
{
	Super::BeginPlay();

	// Go through all CapsuleComponents already added in the editor and bind the overlap events.
	// This is useful if the capsules are created directly in a Blueprint child of AItem.
	TArray<UCapsuleComponent*> AllCapsules;
	GetComponents<UCapsuleComponent>(AllCapsules);
	for (UCapsuleComponent* Capsule : AllCapsules)
	{
		AddHitCapsule(Capsule);
	}
}

void AItem::AddHitCapsule(UCapsuleComponent* CapsuleToAdd)
{
	if (CapsuleToAdd && !HitCapsules.Contains(CapsuleToAdd))
	{
		HitCapsules.Add(CapsuleToAdd);
		
		// Bind the overlap events for this specific capsule
		CapsuleToAdd->OnComponentBeginOverlap.AddDynamic(this, &AItem::OnHitCapsuleBeginOverlap);
		CapsuleToAdd->OnComponentEndOverlap.AddDynamic(this, &AItem::OnHitCapsuleEndOverlap);
	}
}

void AItem::OnHitCapsuleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// --- New Collision Ignore Logic ---
	// Ignore the item itself.
	if (OtherActor == this)
	{
		return;
	}
	// Ignore the owner if the flag is set.
	if (bIgnoreOwner && OtherActor == GetOwner())
	{
		return;
	}
	// Ignore any actor instance present in the IgnoredActors array.
	if (IgnoredActors.Contains(OtherActor))
	{
		return;
	}
	// --- End New Logic ---

	// Add the actor to the list if it's not already in it
	if (!OverlappingActors.Contains(OtherActor))
	{
		OverlappingActors.Add(OtherActor);
		// Send an event that a new actor is overlapping
		OnItemOverlap.Broadcast(OtherActor, true);
	}
}

void AItem::OnHitCapsuleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    // Ignore the owner of the item (e.g., the character holding it) and the item itself.
	if ((bIgnoreOwner && OtherActor == GetOwner()) || OtherActor == this)
	{
		return;
	}

	// Remove the actor from the list
	if (OverlappingActors.Remove(OtherActor) > 0)
	{
		// Send an event that the actor is no longer overlapping
		OnItemOverlap.Broadcast(OtherActor, false);
	}
}
