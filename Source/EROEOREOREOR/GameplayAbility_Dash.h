// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayTagContainer.h"
#include "GameplayAbility_Dash.generated.h"

UENUM(BlueprintType)
enum class EDashDirection : uint8
{
	None	UMETA(DisplayName = "None"),
	Left	UMETA(DisplayName = "Left"),
	Right	UMETA(DisplayName = "Right")
};

/**
 * Gameplay Ability for camera-relative dash movement
 * Handles both left and right dash movements with proper GAS integration
 */
UCLASS()
class EROEOREOREOR_API UGameplayAbility_Dash : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGameplayAbility_Dash();

	// Core Ability Implementation
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	// Dash Configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash Settings")
	float DashDistance = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash Settings")
	float DashDuration = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash Settings")
	float DashCooldown = 2.0f;

	// Gameplay Tags
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash Tags")
	FGameplayTag DashingStateTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash Tags")
	FGameplayTag DashCooldownTag;

	// Dash Direction (set by input or other means)
	UPROPERTY(BlueprintReadWrite, Category = "Dash")
	EDashDirection DashDirection = EDashDirection::None;

private:
	// Internal dash execution
	void ExecuteDash(class AMyCharacter* Character, EDashDirection Direction);
	
	// Timer callback for ability end
	UFUNCTION()
	void OnDashComplete();

	FTimerHandle DashTimer;
};
