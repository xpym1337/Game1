// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayTagContainer.h"
#include "Engine/TimerHandle.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "GameplayAbility_Dash.generated.h"

// Forward declarations
class AMyCharacter;
class UCurveFloat;

UENUM(BlueprintType)
enum class EDashDirection : uint8
{
	None	UMETA(DisplayName = "None"),
	Left	UMETA(DisplayName = "Left"),
	Right	UMETA(DisplayName = "Right")
};

/**
 * Production-ready Gameplay Ability for camera-relative dash movement
 * Follows Epic Games coding standards and GAS best practices
 * Single-responsibility, testable, maintainable implementation
 */
UCLASS()
class EROEOREOREOR_API UGameplayAbility_Dash : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGameplayAbility_Dash();

	// Core GAS Implementation - Following Epic naming conventions
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual void CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility) override;
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const override;

	// Public API - Blueprint accessible for design iteration
	UFUNCTION(BlueprintCallable, Category = "Dash")
	void SetDashDirection(EDashDirection InDirection) { DashDirection = InDirection; }

	UFUNCTION(BlueprintPure, Category = "Dash")
	EDashDirection GetDashDirection() const { return DashDirection; }

	// RUNTIME TESTING API - Epic Games debugging standards
	UFUNCTION(BlueprintCallable, Category = "Dash|Testing", CallInEditor,
		meta = (DisplayName = "Test Dash Parameters"))
	void TestDashParameters();

	UFUNCTION(BlueprintPure, Category = "Dash|Testing")
	float GetCurrentDashProgress() const;

	UFUNCTION(BlueprintPure, Category = "Dash|Testing")
	FVector GetCurrentDashDirection() const;

	UFUNCTION(BlueprintPure, Category = "Dash|Testing")
	float GetCurrentDashSpeed() const;

	UFUNCTION(BlueprintCallable, Category = "Dash|Testing")
	void SetDashSpeedRuntime(float InSpeed) { DashSpeed = FMath::Clamp(InSpeed, MIN_DASH_SPEED, MAX_DASH_SPEED); }

	UFUNCTION(BlueprintCallable, Category = "Dash|Testing")
	void SetDashDurationRuntime(float InDuration) { DashDuration = FMath::Clamp(InDuration, 0.05f, 3.0f); }

	// CONFIGURATION PRESETS - Easy testing of different feels
	UFUNCTION(BlueprintCallable, Category = "Dash|Presets", CallInEditor)
	void ApplyQuickDashPreset();

	UFUNCTION(BlueprintCallable, Category = "Dash|Presets", CallInEditor)
	void ApplyFloatyDashPreset();

	UFUNCTION(BlueprintCallable, Category = "Dash|Presets", CallInEditor)
	void ApplySnappyDashPreset();

	UFUNCTION(BlueprintCallable, Category = "Dash|Presets", CallInEditor)
	void ApplyPowerfulDashPreset();

	UFUNCTION(BlueprintCallable, Category = "Dash|Presets", CallInEditor)
	void ResetToDefaultPreset();

protected:
	// DASH VELOCITY CONTROL - Epic Games naming convention
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash|Velocity", 
		meta = (ClampMin = "100.0", ClampMax = "10000.0", UIMin = "500.0", UIMax = "5000.0"))
	float DashSpeed = 1875.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash|Velocity", 
		meta = (ClampMin = "0.0", ClampMax = "5000.0", UIMin = "0.0", UIMax = "2500.0"))
	float DashInitialBurstSpeed = 2500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash|Velocity", 
		meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float DashSpeedDecayRate = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash|Velocity", 
		meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float DashVerticalVelocityPreservation = 1.0f;

	// DASH TIMING CONTROL - Precise duration management
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash|Timing", 
		meta = (ClampMin = "0.05", ClampMax = "3.0", UIMin = "0.1", UIMax = "2.0"))
	float DashDuration = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash|Timing", 
		meta = (ClampMin = "0.0", ClampMax = "0.5", UIMin = "0.0", UIMax = "0.3"))
	float DashInitialBurstDuration = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash|Timing", 
		meta = (ClampMin = "0.0", ClampMax = "2.0", UIMin = "0.0", UIMax = "1.0"))
	float DashCancelWindow = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash|Timing", 
		meta = (ClampMin = "10.0", ClampMax = "120.0", UIMin = "30.0", UIMax = "90.0"))
	float UpdateFrequency = 60.0f;

	// DASH PHYSICS CONTROL - Movement feel parameters
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash|Physics", 
		meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MomentumRetention = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash|Physics", 
		meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float DashAirControl = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash|Physics", 
		meta = (ClampMin = "0.0", ClampMax = "5000.0", UIMin = "0.0", UIMax = "3000.0"))
	float DashGroundFrictionOverride = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash|Physics")
	bool bPreserveVerticalMomentum = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash|Physics")
	bool bIgnoreGravityDuringDash = false;

	// DASH DISTANCE CONTROL - Alternative to time-based dashing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash|Distance")
	bool bUseDistanceBasedDash = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash|Distance", 
		meta = (ClampMin = "100.0", ClampMax = "2000.0", UIMin = "200.0", UIMax = "1500.0",
		EditCondition = "bUseDistanceBasedDash"))
	float DashTargetDistance = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash|Distance", 
		meta = (ClampMin = "50.0", ClampMax = "500.0", UIMin = "50.0", UIMax = "200.0",
		EditCondition = "bUseDistanceBasedDash"))
	float DashDistanceTolerance = 50.0f;

	// DASH INPUT CONTROL - Responsiveness parameters
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash|Input", 
		meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "0.5"))
	float InputBufferDuration = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash|Input", 
		meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float InputDirectionInfluence = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash|Input")
	bool bAllowDashDirectionOverride = true;

	// DASH CURVE CONTROL - Epic Games soft reference pattern for proper asset loading
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash|Curves", 
		meta = (DisplayName = "Speed Curve (Optional)", AllowedClasses = "/Script/Engine.CurveFloat"))
	TSoftObjectPtr<UCurveFloat> DashSpeedCurve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash|Curves", 
		meta = (DisplayName = "Direction Curve (Optional)", AllowedClasses = "/Script/Engine.CurveFloat"))
	TSoftObjectPtr<UCurveFloat> DashDirectionCurve;

	// DASH EFFECTS CONTROL - VFX/SFX integration points
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash|Effects", 
		meta = (ClampMin = "0.0", ClampMax = "2.0", UIMin = "0.0", UIMax = "1.0"))
	float DashCameraShakeIntensity = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash|Effects")
	bool bEnableDashTrail = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash|Effects")
	bool bEnableDashScreenEffect = true;

	// DASH DEBUG CONTROL - Development and testing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash|Debug", 
		meta = (DisplayName = "Enable Debug Visualization"))
	bool bEnableDashDebugDraw = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash|Debug", 
		meta = (DisplayName = "Debug Draw Duration", ClampMin = "0.1", ClampMax = "10.0",
		EditCondition = "bEnableDashDebugDraw"))
	float DebugDrawDuration = 2.0f;

	// GAMEPLAY TAGS - Following GAS conventions with proper categories
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash|GameplayTags")
	FGameplayTag DashingStateTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash|GameplayTags")
	FGameplayTag DashCooldownTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash|GameplayTags")
	FGameplayTag DashImmuneTag;

private:
	// Core Implementation - Single responsibility
	void ExecuteDash();
	void UpdateDashVelocity();
	void FinalizeDash();

	// Helper functions for testability
	bool ValidateActivationRequirements(const AMyCharacter* InCharacter) const;
	FVector CalculateCameraRelativeDashDirection(const AMyCharacter* InCharacter) const;
	float CalculateCurrentDashSpeed(const float InAlpha) const;

	// Core Implementation - Single responsibility
	void LoadCurveAssets();
	void OnCurveAssetsLoaded();

	// State Management - RAII principles with proper Epic Games standards
	UPROPERTY(Transient)
	EDashDirection DashDirection;

	// CRITICAL: Use TWeakObjectPtr for actor references to prevent dangling pointers
	UPROPERTY(Transient)
	TWeakObjectPtr<AMyCharacter> CachedCharacter;

	UPROPERTY(Transient)
	FVector2D StoredInputDirection;

	// EPIC GAMES STANDARD: Curve asset loading state management
	UPROPERTY(Transient)
	TObjectPtr<UCurveFloat> LoadedDashSpeedCurve;

	UPROPERTY(Transient)
	TObjectPtr<UCurveFloat> LoadedDashDirectionCurve;

	// Runtime State
	FTimerHandle VelocityUpdateTimer;
	FTimerHandle CurveLoadTimer;
	float DashStartTime;
	bool bIsActiveDash;

	// EPIC GAMES STANDARD: Asset loading streamable handle management
	TSharedPtr<FStreamableHandle> CurveLoadHandle;

	// Constants - Epic Games style
	static constexpr float DEFAULT_UPDATE_RATE = 1.0f / 30.0f;
	static constexpr float MIN_DASH_SPEED = 100.0f;
	static constexpr float MAX_DASH_SPEED = 5000.0f;
};
