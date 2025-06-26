// Copyright Robinator Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ChaosGameMode.generated.h"

/**
 * Simple GameMode for a third person game
 */
UCLASS(abstract)
class AChaosGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	
	/** Constructor */
	AChaosGameMode();
};
