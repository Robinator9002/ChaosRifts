// Copyright Robinator Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Item.generated.h"

class UStaticMeshComponent;

/**
 * AItem is the base class for all pickup-able or interactive objects in the world.
 * It now uses a Static Mesh as its root for visuals and simple collision.
 */
UCLASS(Blueprintable)
class CHAOSRIFTS_API AItem : public AActor
{
	GENERATED_BODY()
	
public:	
	AItem();

	// The root component of the item, which provides the visual representation and collision.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
	TObjectPtr<UStaticMeshComponent> ItemMesh;
};
