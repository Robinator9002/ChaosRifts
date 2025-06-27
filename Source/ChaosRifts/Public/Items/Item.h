// Copyright Robinator Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Item.generated.h"

class UCapsuleComponent;

// Delegate that is triggered when the overlap status of an item changes.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemOverlapSignature, AActor*, OverlappedActor, bool, bIsOverlapping);

/**
 * AItem is the base class for all pickup-able or interactive objects in the world.
 * It implements a generic collision detection mechanic with multiple hit capsules.
 */
UCLASS(Blueprintable)
class CHAOSRIFTS_API AItem : public AActor
{
	GENERATED_BODY()
	
public:	
	AItem();

	// The root component of the item. Often a StaticMesh or SkeletalMesh.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
	TObjectPtr<USceneComponent> SceneRoot;
	
	// Delegate that is called when an Actor enters or leaves one of the hit capsules.
	UPROPERTY(BlueprintAssignable, Category = "Item|Events")
	FOnItemOverlapSignature OnItemOverlap;

	/**
	 * Adds a new hit capsule to this item.
	 * This is useful for defining complex collision shapes in Blueprints or derived classes.
	 * @param CapsuleToAdd The capsule to be added to the array.
	 */
	UFUNCTION(BlueprintCallable, Category = "Item|Collision")
	void AddHitCapsule(UCapsuleComponent* CapsuleToAdd);

	/** * Returns all actors that are currently overlapping with THIS item. 
	 * Renamed from GetOverlappingActors to avoid conflict with the base AActor function.
	 */
	UFUNCTION(BlueprintCallable, Category = "Item|Collision")
	const TArray<AActor*>& GetItemOverlappingActors() const { return OverlappingActors; }

protected:
	virtual void BeginPlay() override;

	// Array for storing all actors that are currently overlapping with one of the hit capsules.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Item|State")
	TArray<TObjectPtr<AActor>> OverlappingActors;

private:
	// Functions that are bound to the OnComponentBeginOverlap and OnComponentEndOverlap delegates of the capsules.
	UFUNCTION()
	void OnHitCapsuleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnHitCapsuleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// An array containing all capsules responsible for collision for this item.
	// These must be created and added in the derived class or Blueprint.
	UPROPERTY(VisibleAnywhere, Category = "Item|Collision")
	TArray<TObjectPtr<UCapsuleComponent>> HitCapsules;
};
