// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Blueprint.h"
#include "GameFramework/Actor.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "GameplayTagTester.generated.h"

UCLASS(BlueprintType, Blueprintable)
class EROEOREOREOR_API AGameplayTagTester : public AActor
{
	GENERATED_BODY()
	
public:	
	AGameplayTagTester();

protected:
	virtual void BeginPlay() override;

	// AbilitySystemComponent for testing
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

public:
	// Test FGameplayTag variables - these should be visible in Blueprint dropdowns
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Testing")
	FGameplayTag TestAbilityDash;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Testing")
	FGameplayTag TestAbilityAttack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Testing")
	FGameplayTag TestAbilityJump;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Testing")
	FGameplayTag TestStateDashing;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Testing")
	FGameplayTag TestStateAttacking;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Testing")
	FGameplayTag TestStateStunned;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Testing")
	FGameplayTag TestStateInAir;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Testing")
	FGameplayTag TestEffectCooldown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Testing")
	FGameplayTag TestEffectCost;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Testing")
	FGameplayTag TestEffectDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Testing")
	FGameplayTag TestInputBlocked;

	// Test container for multiple tags
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Testing")
	FGameplayTagContainer TestTagContainer;

	// Tag testing functions
	UFUNCTION(BlueprintCallable, Category = "Tag Testing")
	void AddTagToASC(FGameplayTag TagToAdd);

	UFUNCTION(BlueprintCallable, Category = "Tag Testing")
	void RemoveTagFromASC(FGameplayTag TagToRemove);

	UFUNCTION(BlueprintCallable, Category = "Tag Testing")
	bool HasTag(FGameplayTag TagToCheck);

	UFUNCTION(BlueprintCallable, Category = "Tag Testing")
	bool HasAllTags(FGameplayTagContainer TagsToCheck);

	UFUNCTION(BlueprintCallable, Category = "Tag Testing")
	bool HasAnyTags(FGameplayTagContainer TagsToCheck);

	UFUNCTION(BlueprintCallable, Category = "Tag Testing")
	void LogCurrentTags();

	UFUNCTION(BlueprintCallable, Category = "Tag Testing")
	void RunBasicTagTests();

	UFUNCTION(BlueprintCallable, Category = "Tag Testing")
	void RunAdvancedTagTests();

	// Get ASC for external testing
	UFUNCTION(BlueprintCallable, Category = "Tag Testing")
	UAbilitySystemComponent* GetTestASC() const { return AbilitySystemComponent; }
};
