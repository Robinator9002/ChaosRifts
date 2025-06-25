// Copyright Robinator Studios, Inc. All Rights Reserved.

#include "Characters/Base/ChaosCharacterBase.h"
#include "Components/ChaosAttributes.h"

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
