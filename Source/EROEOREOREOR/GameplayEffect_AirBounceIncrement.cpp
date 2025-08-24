// Fill out your copyright notice in the Description page of Project Settings.

#include "GameplayEffect_AirBounceIncrement.h"
#include "MyAttributeSet.h"

UGameplayEffect_AirBounceIncrement::UGameplayEffect_AirBounceIncrement()
{
	// EPIC GAMES STANDARD: Configure instant effect for attribute modification
	DurationPolicy = EGameplayEffectDurationType::Instant;
	
	// Configure modifier for AirBounceCount attribute
	FGameplayModifierInfo Modifier;
	Modifier.Attribute = UMyAttributeSet::GetAirBounceCountAttribute();
	Modifier.ModifierOp = EGameplayModOp::Additive;
	Modifier.ModifierMagnitude = FScalableFloat(1.0f);
	
	Modifiers.Add(Modifier);
	
	// Set gameplay tags for proper tracking
	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Effect.AirBounce.Increment")));
	InheritableOwnedTagsContainer.Added = AssetTags;
}
