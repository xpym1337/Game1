// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayTagContainer.h"
#include "Engine/TimerHandle.h"
#include "GameplayAbility_Bounce.generated.h"

// Forward declarations
class AMyCharacter;
class UCurveFloat;

/**
 * Production-ready Gameplay Ability for bounce movement mechanics
 * Preserves horizontal momentum while adding upward velocity
 * Follows Epic Games coding standards and GAS best practices
 * Single-responsibility, testable, maintainable implementation
 */
UCLASS()
class EROEOREOREOR_API UGameplayAbility_Bounce : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGameplayAbility_Bounce();

	// Core GAS Implementation - Following Epic naming conventions
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual void CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility) override;
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const override;

	// Public API - Blueprint accessible for design iteration
	UFUNCTION(BlueprintPure, Category = "Bounce")
	int32 GetCurrentAirBounces() const { return CurrentAirBounces; }

	UFUNCTION(BlueprintPure, Category = "Bounce")
	int32 GetMaxAirBounces() const { return MaxAirBounces; }

	UFUNCTION(BlueprintCallable, Category = "Bounce")
	void ResetAirBounces() { CurrentAirBounces = 0; }

	// RUNTIME TESTING API - Epic Games debugging standards
	UFUNCTION(BlueprintCallable, Category = "Bounce|Testing", CallInEditor,
		meta = (DisplayName = "Test Bounce Parameters"))
	void TestBounceParameters();

	UFUNCTION(BlueprintPure, Category = "Bounce|Testing")
	bool CanPerformAirBounce() const;

	UFUNCTION(BlueprintPure, Category = "Bounce|Testing")
	FVector GetCurrentHorizontalVelocity() const;

	UFUNCTION(BlueprintPure, Category = "Bounce|Testing")
	float GetBounceInputTimeRemaining() const;

	UFUNCTION(BlueprintCallable, Category = "Bounce|Testing")
	void SetBounceVelocityRuntime(float InVelocity) { BounceUpwardVelocity = FMath::Clamp(InVelocity, MIN_BOUNCE_VELOCITY, MAX_BOUNCE_VELOCITY); }

	UFUNCTION(BlueprintCallable, Category = "Bounce|Testing")
	void SetMaxAirBouncesRuntime(int32 InMaxBounces) { MaxAirBounces = FMath::Clamp(InMaxBounces, 0, MAX_AIR_BOUNCES_LIMIT); }

	// CONFIGURATION PRESETS - Easy testing of different feels
	UFUNCTION(BlueprintCallable, Category = "Bounce|Presets", CallInEditor)
	void ApplyLowBouncePreset();

	UFUNCTION(BlueprintCallable, Category = "Bounce|Presets", CallInEditor)
	void ApplyHighBouncePreset();

	UFUNCTION(BlueprintCallable, Category = "Bounce|Presets", CallInEditor)
	void ApplyFloatyBouncePreset();

	UFUNCTION(BlueprintCallable, Category = "Bounce|Presets", CallInEditor)
	void ApplySnappyBouncePreset();

	UFUNCTION(BlueprintCallable, Category = "Bounce|Presets", CallInEditor)
	void ResetToDefaultPreset();

protected:
	// BOUNCE VELOCITY CONTROL - Epic Games naming convention
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounce|Velocity", 
		meta = (ClampMin = "200.0", ClampMax = "2000.0", UIMin = "400.0", UIMax = "1200.0"))
	float BounceUpwardVelocity = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounce|Velocity", 
		meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.8", UIMax = "1.0"))
	float HorizontalVelocityRetention = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounce|Velocity", 
		meta = (ClampMin = "0.0", ClampMax = "2.0", UIMin = "0.0", UIMax = "1.5"))
	float HorizontalVelocityMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounce|Velocity", 
		meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float AirControlMultiplier = 0.2f;

	// BOUNCE AIR CONTROL - Movement limitations
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounce|AirControl", 
		meta = (ClampMin = "0", ClampMax = "5", UIMin = "0", UIMax = "3"))
	int32 MaxAirBounces = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounce|AirControl", 
		meta = (ClampMin = "0.5", ClampMax = "1.0", UIMin = "0.7", UIMax = "1.0"))
	float AirBounceVelocityReduction = 0.85f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounce|AirControl")
	bool bResetAirBouncesOnGroundContact = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounce|AirControl")
	bool bAllowBounceWhileRising = false;

	// BOUNCE INPUT CONTROL - Responsiveness parameters
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounce|Input", 
		meta = (ClampMin = "0.0", ClampMax = "0.5", UIMin = "0.05", UIMax = "0.2"))
	float BounceInputWindow = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounce|Input", 
		meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "0.5"))
	float CoyoteTime = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounce|Input")
	bool bRequireInputForAirBounce = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounce|Input")
	bool bAllowBufferedBounce = true;

	// BOUNCE PHYSICS CONTROL - Advanced movement behavior
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounce|Physics", 
		meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float GravityScaleDuringBounce = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounce|Physics", 
		meta = (ClampMin = "0.01", ClampMax = "1.0", UIMin = "0.1", UIMax = "0.5"))
	float BounceDuration = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounce|Physics")
	bool bIgnoreGravityDuringBounce = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounce|Physics")
	bool bPreserveDownwardMomentum = false;

	// BOUNCE CURVE CONTROL - Advanced easing options
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounce|Curves", 
		meta = (DisplayName = "Velocity Curve (Optional)"))
	TSoftObjectPtr<UCurveFloat> BounceVelocityCurve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounce|Curves", 
		meta = (DisplayName = "Air Control Curve (Optional)"))
	TSoftObjectPtr<UCurveFloat> AirControlCurve;

	// BOUNCE EFFECTS CONTROL - VFX/SFX integration points
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounce|Effects", 
		meta = (ClampMin = "0.0", ClampMax = "2.0", UIMin = "0.0", UIMax = "1.0"))
	float BounceCameraShakeIntensity = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounce|Effects")
	bool bEnableBounceTrail = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounce|Effects")
	bool bEnableBounceScreenEffect = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounce|Effects")
	bool bEnableBounceLanding = true;

	// BOUNCE DEBUG CONTROL - Development and testing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounce|Debug", 
		meta = (DisplayName = "Enable Debug Visualization"))
	bool bEnableBounceDebugDraw = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounce|Debug", 
		meta = (DisplayName = "Debug Draw Duration", ClampMin = "0.1", ClampMax = "10.0",
		EditCondition = "bEnableBounceDebugDraw"))
	float DebugDrawDuration = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounce|Debug")
	bool bLogBounceEvents = false;

	// GAMEPLAY TAGS - Following GAS conventions with proper categories
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounce|GameplayTags")
	FGameplayTag BouncingStateTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounce|GameplayTags")
	FGameplayTag BounceCooldownTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounce|GameplayTags")
	FGameplayTag AirborneStateTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounce|GameplayTags")
	FGameplayTag BounceImmuneTag;

private:
	// Core Implementation - Single responsibility
	void ExecuteBounce();
	void ApplyBouncePhysics();
	void FinalizeBounce();
	
	// Ground state tracking
	UFUNCTION()
	void OnLandedDelegate(const FHitResult& Hit);
	void CheckGroundState();

	// Helper functions for testability
	bool ValidateActivationRequirements(const AMyCharacter* InCharacter) const;
	bool IsCharacterGrounded(const AMyCharacter* InCharacter) const;
	bool IsCharacterRising(const AMyCharacter* InCharacter) const;
	FVector CalculateBounceVelocity(const AMyCharacter* InCharacter) const;
	float GetEffectiveBounceVelocity() const;

	// State Management - RAII principles
	UPROPERTY(Transient)
	TObjectPtr<AMyCharacter> CachedCharacter;

	// Runtime State - Air bounce tracking
	UPROPERTY(Transient)
	int32 CurrentAirBounces;

	UPROPERTY(Transient)
	bool bIsGrounded;

	UPROPERTY(Transient)
	float LastGroundContactTime;

	UPROPERTY(Transient)
	float BounceInputPressTime;

	// Timers and handles
	FTimerHandle BounceEffectTimer;
	FTimerHandle GroundCheckTimer;

	// Constants - Epic Games style
	static constexpr float DEFAULT_GROUND_CHECK_RATE = 1.0f / 20.0f;
	static constexpr float MIN_BOUNCE_VELOCITY = 200.0f;
	static constexpr float MAX_BOUNCE_VELOCITY = 2000.0f;
	static constexpr int32 MAX_AIR_BOUNCES_LIMIT = 10;
	static constexpr float MIN_VELOCITY_THRESHOLD = 50.0f;
};
