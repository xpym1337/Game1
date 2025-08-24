// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GameplayEffect_AirBounceReset.generated.h"

/**
 * Epic Games Standard: Gameplay Effect for resetting AirBounceCount attribute to 0
 * Follows proper GAS patterns for attribute modification instead of direct manipulation
 * Instant effect that sets AirBounceCount to 0 (used on ground contact)
 */
UCLASS(BlueprintType, Blueprintable)
class EROEOREOREOR_API UGameplayEffect_AirBounceReset : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UGameplayEffect_AirBounceReset();
};
