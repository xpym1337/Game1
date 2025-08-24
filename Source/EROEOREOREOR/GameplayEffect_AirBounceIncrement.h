// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GameplayEffect_AirBounceIncrement.generated.h"

/**
 * Epic Games Standard: Gameplay Effect for incrementing AirBounceCount attribute
 * Follows proper GAS patterns for attribute modification instead of direct manipulation
 * Instant effect that adds +1 to AirBounceCount attribute
 */
UCLASS(BlueprintType, Blueprintable)
class EROEOREOREOR_API UGameplayEffect_AirBounceIncrement : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UGameplayEffect_AirBounceIncrement();
};
