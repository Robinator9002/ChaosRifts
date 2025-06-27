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

	// Gehe durch alle bereits im Editor hinzugefügten CapsuleComponents und binde die Overlap-Events.
	// Das ist nützlich, wenn die Kapseln direkt in einem Blueprint-Child von AItem erstellt werden.
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
		
		// Binde die Overlap-Events für diese spezifische Kapsel
		CapsuleToAdd->OnComponentBeginOverlap.AddDynamic(this, &AItem::OnHitCapsuleBeginOverlap);
		CapsuleToAdd->OnComponentEndOverlap.AddDynamic(this, &AItem::OnHitCapsuleEndOverlap);
	}
}

void AItem::OnHitCapsuleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Ignoriere den Besitzer des Items (z.B. den Charakter, der es hält) und das Item selbst.
	if (OtherActor == GetOwner() || OtherActor == this)
	{
		return;
	}

	// Füge den Actor zur Liste hinzu, wenn er noch nicht drin ist
	if (!OverlappingActors.Contains(OtherActor))
	{
		OverlappingActors.Add(OtherActor);
		// Sende ein Event, dass ein neuer Actor überlappt
		OnItemOverlap.Broadcast(OtherActor, true);
	}
}

void AItem::OnHitCapsuleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor == GetOwner() || OtherActor == this)
	{
		return;
	}
	
	// Entferne den Actor aus der Liste und sende ein Event
	if (OverlappingActors.Remove(OtherActor) > 0)
	{
		OnItemOverlap.Broadcast(OtherActor, false);
	}
}
