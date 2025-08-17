// Fill out your copyright notice in the Description page of Project Settings.

#include "GameplayTagTester.h"
#include "AbilitySystemComponent.h"
#include "Engine/Engine.h"
#include "GameplayTagsManager.h"

AGameplayTagTester::AGameplayTagTester()
{
	PrimaryActorTick.bCanEverTick = false;

	// Create AbilitySystemComponent
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
}

void AGameplayTagTester::BeginPlay()
{
	Super::BeginPlay();
	
	// Initialize the AbilitySystemComponent
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
		UE_LOG(LogTemp, Warning, TEXT("GameplayTagTester: AbilitySystemComponent initialized"));
	}
}

void AGameplayTagTester::AddTagToASC(FGameplayTag TagToAdd)
{
	if (AbilitySystemComponent && TagToAdd.IsValid())
	{
		AbilitySystemComponent->AddLooseGameplayTag(TagToAdd);
		UE_LOG(LogTemp, Warning, TEXT("GameplayTagTester: Added tag '%s' to ASC"), *TagToAdd.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("GameplayTagTester: Failed to add tag - Invalid ASC or tag"));
	}
}

void AGameplayTagTester::RemoveTagFromASC(FGameplayTag TagToRemove)
{
	if (AbilitySystemComponent && TagToRemove.IsValid())
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(TagToRemove);
		UE_LOG(LogTemp, Warning, TEXT("GameplayTagTester: Removed tag '%s' from ASC"), *TagToRemove.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("GameplayTagTester: Failed to remove tag - Invalid ASC or tag"));
	}
}

bool AGameplayTagTester::HasTag(FGameplayTag TagToCheck)
{
	if (AbilitySystemComponent && TagToCheck.IsValid())
	{
		bool bHasTag = AbilitySystemComponent->HasMatchingGameplayTag(TagToCheck);
		UE_LOG(LogTemp, Warning, TEXT("GameplayTagTester: ASC %s tag '%s'"), 
			bHasTag ? TEXT("HAS") : TEXT("does NOT have"), *TagToCheck.ToString());
		return bHasTag;
	}
	
	UE_LOG(LogTemp, Error, TEXT("GameplayTagTester: Failed to check tag - Invalid ASC or tag"));
	return false;
}

bool AGameplayTagTester::HasAllTags(FGameplayTagContainer TagsToCheck)
{
	if (AbilitySystemComponent && TagsToCheck.Num() > 0)
	{
		bool bHasAllTags = AbilitySystemComponent->HasAllMatchingGameplayTags(TagsToCheck);
		UE_LOG(LogTemp, Warning, TEXT("GameplayTagTester: ASC %s all tags in container (%d tags)"), 
			bHasAllTags ? TEXT("HAS") : TEXT("does NOT have"), TagsToCheck.Num());
		return bHasAllTags;
	}
	
	UE_LOG(LogTemp, Error, TEXT("GameplayTagTester: Failed to check tag container - Invalid ASC or empty container"));
	return false;
}

bool AGameplayTagTester::HasAnyTags(FGameplayTagContainer TagsToCheck)
{
	if (AbilitySystemComponent && TagsToCheck.Num() > 0)
	{
		bool bHasAnyTags = AbilitySystemComponent->HasAnyMatchingGameplayTags(TagsToCheck);
		UE_LOG(LogTemp, Warning, TEXT("GameplayTagTester: ASC %s any tags in container (%d tags)"), 
			bHasAnyTags ? TEXT("HAS") : TEXT("does NOT have"), TagsToCheck.Num());
		return bHasAnyTags;
	}
	
	UE_LOG(LogTemp, Error, TEXT("GameplayTagTester: Failed to check tag container - Invalid ASC or empty container"));
	return false;
}

void AGameplayTagTester::LogCurrentTags()
{
	if (AbilitySystemComponent)
	{
		FGameplayTagContainer CurrentTags;
		AbilitySystemComponent->GetOwnedGameplayTags(CurrentTags);
		
		UE_LOG(LogTemp, Warning, TEXT("GameplayTagTester: Current ASC Tags (%d total):"), CurrentTags.Num());
		
		if (CurrentTags.Num() == 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("  - No tags currently active"));
		}
		else
		{
			for (const FGameplayTag& Tag : CurrentTags)
			{
				UE_LOG(LogTemp, Warning, TEXT("  - %s"), *Tag.ToString());
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("GameplayTagTester: Cannot log tags - Invalid ASC"));
	}
}

void AGameplayTagTester::RunBasicTagTests()
{
	UE_LOG(LogTemp, Warning, TEXT("=== STARTING BASIC TAG TESTS ==="));
	
	// Get some basic tags for testing
	FGameplayTag DashTag = FGameplayTag::RequestGameplayTag(FName("Ability.Dash"));
	FGameplayTag AttackTag = FGameplayTag::RequestGameplayTag(FName("Ability.Attack"));
	FGameplayTag DashingStateTag = FGameplayTag::RequestGameplayTag(FName("State.Dashing"));
	
	// Test 1: Add tags
	UE_LOG(LogTemp, Warning, TEXT("Test 1: Adding tags"));
	AddTagToASC(DashTag);
	AddTagToASC(AttackTag);
	LogCurrentTags();
	
	// Test 2: Check individual tags
	UE_LOG(LogTemp, Warning, TEXT("Test 2: Checking individual tags"));
	HasTag(DashTag);
	HasTag(AttackTag);
	HasTag(DashingStateTag); // Should be false
	
	// Test 3: Add state tag and verify
	UE_LOG(LogTemp, Warning, TEXT("Test 3: Adding state tag"));
	AddTagToASC(DashingStateTag);
	HasTag(DashingStateTag);
	LogCurrentTags();
	
	// Test 4: Remove tags
	UE_LOG(LogTemp, Warning, TEXT("Test 4: Removing tags"));
	RemoveTagFromASC(DashTag);
	HasTag(DashTag); // Should be false
	LogCurrentTags();
	
	// Test 5: Clean up
	UE_LOG(LogTemp, Warning, TEXT("Test 5: Cleanup"));
	RemoveTagFromASC(AttackTag);
	RemoveTagFromASC(DashingStateTag);
	LogCurrentTags();
	
	UE_LOG(LogTemp, Warning, TEXT("=== BASIC TAG TESTS COMPLETE ==="));
}

void AGameplayTagTester::RunAdvancedTagTests()
{
	UE_LOG(LogTemp, Warning, TEXT("=== STARTING ADVANCED TAG TESTS ==="));
	
	// Create tag containers for testing
	FGameplayTagContainer AbilityTags;
	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Dash")));
	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Attack")));
	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Jump")));
	
	FGameplayTagContainer StateTags;
	StateTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Dashing")));
	StateTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Attacking")));
	
	FGameplayTagContainer EffectTags;
	EffectTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Effect.Cooldown")));
	EffectTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Effect.Cost")));
	
	// Test 1: Add some ability tags
	UE_LOG(LogTemp, Warning, TEXT("Test 1: Adding ability tags"));
	AddTagToASC(FGameplayTag::RequestGameplayTag(FName("Ability.Dash")));
	AddTagToASC(FGameplayTag::RequestGameplayTag(FName("Ability.Attack")));
	LogCurrentTags();
	
	// Test 2: Test HasAllTags (should pass)
	UE_LOG(LogTemp, Warning, TEXT("Test 2: Testing HasAllTags with partial ability container"));
	FGameplayTagContainer PartialAbilityTags;
	PartialAbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Dash")));
	PartialAbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Attack")));
	HasAllTags(PartialAbilityTags);
	
	// Test 3: Test HasAllTags (should fail)
	UE_LOG(LogTemp, Warning, TEXT("Test 3: Testing HasAllTags with full ability container"));
	HasAllTags(AbilityTags);
	
	// Test 4: Test HasAnyTags (should pass)
	UE_LOG(LogTemp, Warning, TEXT("Test 4: Testing HasAnyTags with ability container"));
	HasAnyTags(AbilityTags);
	
	// Test 5: Test HasAnyTags (should fail)
	UE_LOG(LogTemp, Warning, TEXT("Test 5: Testing HasAnyTags with state container"));
	HasAnyTags(StateTags);
	
	// Test 6: Add effect tags and test mixed container
	UE_LOG(LogTemp, Warning, TEXT("Test 6: Adding effect tags"));
	AddTagToASC(FGameplayTag::RequestGameplayTag(FName("Effect.Cooldown")));
	LogCurrentTags();
	
	// Test 7: Test mixed container queries
	UE_LOG(LogTemp, Warning, TEXT("Test 7: Testing mixed container queries"));
	HasAnyTags(EffectTags);
	
	// Test 8: Clean up
	UE_LOG(LogTemp, Warning, TEXT("Test 8: Final cleanup"));
	RemoveTagFromASC(FGameplayTag::RequestGameplayTag(FName("Ability.Dash")));
	RemoveTagFromASC(FGameplayTag::RequestGameplayTag(FName("Ability.Attack")));
	RemoveTagFromASC(FGameplayTag::RequestGameplayTag(FName("Effect.Cooldown")));
	LogCurrentTags();
	
	UE_LOG(LogTemp, Warning, TEXT("=== ADVANCED TAG TESTS COMPLETE ==="));
}
