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
	
	// Set gameplay tags for proper tracking
	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Effect.AirBounce.Reset")));
	InheritableOwnedTagsContainer.Added = AssetTags;
}
