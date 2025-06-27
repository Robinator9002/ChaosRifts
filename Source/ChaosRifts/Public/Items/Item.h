// Copyright Robinator Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Item.generated.h"

class UCapsuleComponent;

// Delegate, das ausgelöst wird, wenn sich der Overlap-Status eines Items ändert.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemOverlapSignature, AActor*, OverlappedActor, bool, bIsOverlapping);

/**
 * AItem ist die Basisklasse für alle aufnehmbaren oder interaktiven Gegenstände in der Welt.
 * Sie implementiert eine generische Mechanik zur Kollisionserkennung mit mehreren Hit-Capsules.
 */
UCLASS(Blueprintable)
class CHAOSRIFTS_API AItem : public AActor
{
	GENERATED_BODY()
	
public:	
	AItem();

	// Das Root-Component des Items. Oft ein StaticMesh oder SkeletalMesh.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
	TObjectPtr<USceneComponent> SceneRoot;
	
	// Delegate, das aufgerufen wird, wenn ein Actor eine der Hit-Capsules betritt oder verlässt.
	UPROPERTY(BlueprintAssignable, Category = "Item|Events")
	FOnItemOverlapSignature OnItemOverlap;

	/**
	 * Fügt eine neue Hit-Capsule zu diesem Item hinzu.
	 * Dies ist nützlich, um in Blueprints oder abgeleiteten Klassen komplexe Kollisionsformen zu definieren.
	 * @param CapsuleToAdd Die Capsule, die zum Array hinzugefügt werden soll.
	 */
	UFUNCTION(BlueprintCallable, Category = "Item|Collision")
	void AddHitCapsule(UCapsuleComponent* CapsuleToAdd);

	/** * Gibt alle Actors zurück, die gerade mit DIESEM Item überlappen. 
	 * Umbenannt von GetOverlappingActors, um Konflikt mit der Basis-AActor-Funktion zu vermeiden.
	 */
	UFUNCTION(BlueprintCallable, Category = "Item|Collision")
	const TArray<AActor*>& GetItemOverlappingActors() const { return OverlappingActors; }

protected:
	virtual void BeginPlay() override;

	// Array zur Speicherung aller Actors, die derzeit mit einer der Hit-Capsules überlappen.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Item|State")
	TArray<TObjectPtr<AActor>> OverlappingActors;

private:
	// Funktionen, die an die OnComponentBeginOverlap und OnComponentEndOverlap Delegates der Kapseln gebunden werden.
	UFUNCTION()
	void OnHitCapsuleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnHitCapsuleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// Ein Array, das alle für die Kollision zuständigen Kapseln dieses Items enthält.
	// Diese müssen in der abgeleiteten Klasse oder im Blueprint erstellt und hinzugefügt werden.
	UPROPERTY(VisibleAnywhere, Category = "Item|Collision")
	TArray<TObjectPtr<UCapsuleComponent>> HitCapsules;
};
