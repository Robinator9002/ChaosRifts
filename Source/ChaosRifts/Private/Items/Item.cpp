// Copyright Robinator Studios, Inc. All Rights Reserved.

#include "Items/Item.h"
#include "Components/CapsuleComponent.h"

AItem::AItem()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;
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
	// Ignore the owner of the item (e.g., the character holding it) and the item itself.
	if (OtherActor == GetOwner() || OtherActor == this)
	{
		return;
	}

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
	if (OtherActor == GetOwner() || OtherActor == this)
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
