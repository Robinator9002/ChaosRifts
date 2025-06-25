// Copyright Robinator Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ChaosCharacterBase.generated.h"

class UChaosAttributes;

/**
 * The base class for all characters in ChaosRifts, including the player, enemies, and NPCs.
 * It contains shared functionality, most importantly the Attributes component.
 */
UCLASS(abstract)
class CHAOSRIFTS_API AChaosCharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	AChaosCharacterBase();

	/** Returns the Attributes component for easy access from other classes. */
	UFUNCTION(BlueprintCallable, Category = "Chaos|Character")
	FORCEINLINE UChaosAttributes* GetAttributes() const { return AttributesComponent; }

protected:

	/** The component that manages all gameplay attributes for this character. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chaos|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UChaosAttributes> AttributesComponent;
};
