// Fill out your copyright notice in the Description page of Project Settings.

#include "GameplayEffect_AirBounceReset.h"
#include "MyAttributeSet.h"

UGameplayEffect_AirBounceReset::UGameplayEffect_AirBounceReset()
{
	// EPIC GAMES STANDARD: Configure instant effect for attribute reset
	DurationPolicy = EGameplayEffectDurationType::Instant;
	
	// Configure modifier for AirBounceCount attribute (reset to 0)
	FGameplayModifierInfo Modifier;
	Modifier.Attribute = UMyAttributeSet::GetAirBounceCountAttribute();
	Modifier.ModifierOp = EGameplayModOp::Override;
	Modifier.ModifierMagnitude = FScalableFloat(0.0f);
	
	Modifiers.Add(Modifier);
	
	// NOTE: Asset tag configuration removed due to UE5.6 API changes
	// Core attribute modification functionality preserved for GAS compatibility
	// Tags should be configured in Blueprint or through GameplayEffectComponents if needed
}
