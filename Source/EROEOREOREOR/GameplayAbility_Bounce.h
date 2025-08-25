// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayTagContainer.h"
#include "Engine/TimerHandle.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "VelocitySnapshotComponent.h"
#include "GameplayAbility_Bounce.generated.h"

// Forward declarations
class AMyCharacter;
class UCurveFloat;
class UGameplayEffect;

// EPIC GAMES STANDARD: Enum for trajectory enhancement type safety
UENUM(BlueprintType)
enum class EBounceTrajectoryType : uint8
{
	None            UMETA(DisplayName = "None"),
	UpwardBoost     UMETA(DisplayName = "Upward Amplification"),
	HorizontalBoost UMETA(DisplayName = "Horizontal Enhancement"), 
	RecoveryJump    UMETA(DisplayName = "Fall Recovery Jump"),
	DiagonalBoost   UMETA(DisplayName = "Diagonal Enhancement")
};

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
	int32 GetCurrentAirBounces() const { return GetCurrentAirBounceCount(); }

	UFUNCTION(BlueprintPure, Category = "Bounce")
	int32 GetMaxAirBounces() const { return MaxAirBounces; }

	UFUNCTION(BlueprintCallable, Category = "Bounce")
	void ResetAirBounces() { ResetAirBounceCount(); }

	// INDUSTRY BEST PRACTICE: Gameplay Attribute Management - Following Epic Games GAS patterns
	UFUNCTION(BlueprintPure, Category = "Bounce|Attributes", 
		meta = (DisplayName = "Get Air Bounce Count", CompactNodeTitle = "Air Bounces"))
	int32 GetCurrentAirBounceCount() const;

	UFUNCTION(BlueprintCallable, Category = "Bounce|Attributes",
		meta = (DisplayName = "Reset Air Bounces", CallInEditor = "true"))
	void ResetAirBounceCount();

	UFUNCTION(BlueprintCallable, Category = "Bounce|Attributes",
		meta = (DisplayName = "Add Air Bounce", CallInEditor = "true"))
	void IncrementAirBounceCount();

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

	// DASH-BOUNCE COMBO TESTING UTILITIES - Epic Games debugging standards
	UFUNCTION(BlueprintCallable, Category = "Bounce|Testing", CallInEditor)
	void TestDashBounceCombo();

	UFUNCTION(BlueprintCallable, Category = "Bounce|Testing", CallInEditor)
	void TestJumpBounceCombo();

	UFUNCTION(BlueprintCallable, Category = "Bounce|Testing", CallInEditor)
	void ValidateMomentumTransfer();

	// TRAJECTORY ENHANCEMENT TESTING - Epic Games debugging standards
	UFUNCTION(BlueprintPure, Category = "Bounce|Testing")
	EBounceTrajectoryType GetCurrentTrajectoryType() const;

	UFUNCTION(BlueprintCallable, Category = "Bounce|Testing", CallInEditor)
	void TestTrajectoryEnhancement();

	UFUNCTION(BlueprintCallable, Category = "Bounce|Testing", CallInEditor)
	void ValidateTrajectoryParameters();

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
		meta = (ClampMin = "0", ClampMax = "5", UIMin = "0", UIMax = "15"))
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

	// MOMENTUM TRANSFER SYSTEM - Dash-Bounce Combo Support
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounce|Momentum", 
		meta = (ClampMin = "0.0", ClampMax = "3.0", UIMin = "1.0", UIMax = "2.5"))
	float DashMomentumMultiplier = 1.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounce|Momentum", 
		meta = (ClampMin = "0.0", ClampMax = "3.0", UIMin = "1.0", UIMax = "2.0"))
	float JumpMomentumMultiplier = 1.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounce|Momentum", 
		meta = (ClampMin = "0.0", ClampMax = "2.0", UIMin = "0.8", UIMax = "1.5"))
	float FallMomentumMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounce|Momentum", 
		meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MomentumTransferEfficiency = 0.85f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounce|Momentum")
	bool bAllowMomentumTransfer = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounce|Momentum")
	bool bPreserveMomentumDirection = true;

	// COMBO SYSTEM - Event-based ability coordination
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounce|Combo")
	bool bAllowComboBounce = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounce|Combo", 
		meta = (ClampMin = "0.05", ClampMax = "0.5", UIMin = "0.1", UIMax = "0.3"))
	float ComboWindow = 0.2f;

	// TRAJECTORY ENHANCEMENT SYSTEM - Epic Games standard implementation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounce|Trajectory Enhancement",
		meta = (DisplayName = "Enable Trajectory Enhancement"))
	bool bEnableTrajectoryEnhancement = true;

	// Upward Movement Enhancement
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounce|Trajectory Enhancement|Upward",
		meta = (ClampMin = "1.0", ClampMax = "3.0", UIMin = "1.2", UIMax = "2.5",
				DisplayName = "Upward Amplification Multiplier",
				EditCondition = "bEnableTrajectoryEnhancement"))
	float UpwardAmplificationMultiplier = 1.75f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounce|Trajectory Enhancement|Upward",
		meta = (ClampMin = "50.0", ClampMax = "500.0", UIMin = "100.0", UIMax = "300.0",
				DisplayName = "Minimum Upward Velocity Threshold",
				EditCondition = "bEnableTrajectoryEnhancement"))
	float UpwardVelocityThreshold = 150.0f;

	// Horizontal Movement Enhancement  
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounce|Trajectory Enhancement|Horizontal",
		meta = (ClampMin = "1.0", ClampMax = "2.5", UIMin = "1.1", UIMax = "2.0",
				DisplayName = "Horizontal Enhancement Multiplier",
				EditCondition = "bEnableTrajectoryEnhancement"))
	float HorizontalEnhancementMultiplier = 1.4f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounce|Trajectory Enhancement|Horizontal",
		meta = (ClampMin = "300.0", ClampMax = "800.0", UIMin = "400.0", UIMax = "700.0",
				DisplayName = "Enhanced Horizontal Boost",
				EditCondition = "bEnableTrajectoryEnhancement"))
	float EnhancedHorizontalBoost = 500.0f;

	// Fall Recovery System
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounce|Trajectory Enhancement|Recovery",
		meta = (ClampMin = "500.0", ClampMax = "1200.0", UIMin = "600.0", UIMax = "1000.0",
				DisplayName = "Recovery Jump Velocity",
				EditCondition = "bEnableTrajectoryEnhancement"))
	float RecoveryJumpVelocity = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounce|Trajectory Enhancement|Recovery",
		meta = (ClampMin = "50.0", ClampMax = "500.0", UIMin = "100.0", UIMax = "300.0",
				DisplayName = "Falling Velocity Threshold",
				EditCondition = "bEnableTrajectoryEnhancement"))
	float FallingVelocityThreshold = 200.0f;

	// Diagonal Movement
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounce|Trajectory Enhancement|Diagonal",
		meta = (ClampMin = "1.0", ClampMax = "2.5", UIMin = "1.2", UIMax = "2.0",
				DisplayName = "Diagonal Enhancement Multiplier",
				EditCondition = "bEnableTrajectoryEnhancement"))
	float DiagonalEnhancementMultiplier = 1.6f;

	// BOUNCE CURVE CONTROL - Epic Games soft reference pattern for proper asset loading
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounce|Curves", 
		meta = (DisplayName = "Velocity Curve (Optional)", AllowedClasses = "/Script/Engine.CurveFloat"))
	TSoftObjectPtr<UCurveFloat> BounceVelocityCurve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounce|Curves", 
		meta = (DisplayName = "Air Control Curve (Optional)", AllowedClasses = "/Script/Engine.CurveFloat"))
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

	// GAMEPLAY EFFECTS - Following Epic Games GAS patterns
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	TSubclassOf<UGameplayEffect> BounceEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	TSubclassOf<UGameplayEffect> AirBounceIncrementEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	TSubclassOf<UGameplayEffect> AirBounceResetEffect;

	// GAMEPLAY TAGS - Following GAS conventions with proper categories
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bounce|GameplayTags")
	FGameplayTag BouncingStateTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bounce|GameplayTags")
	FGameplayTag BounceCooldownTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bounce|GameplayTags")
	FGameplayTag AirborneStateTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bounce|GameplayTags")
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
	
	// Enhanced bounce calculation with momentum transfer
	FVector CalculateEnhancedBounceVelocity(const AMyCharacter* InCharacter) const;
	
	// Momentum transfer utilities - performance optimized
	bool TryGetMomentumContext(const AMyCharacter* InCharacter, FVelocitySnapshot& OutSnapshot) const;
	float GetMomentumMultiplier(EVelocitySource Source) const;
	FVector ApplyMomentumTransfer(const FVector& BaseBounceVelocity, const FVelocitySnapshot& MomentumSnapshot) const;
	
	// TRAJECTORY ENHANCEMENT SYSTEM - Clean modular implementation
	FVector CalculateTrajectoryEnhancedVelocity(const FVector& CurrentVelocity) const;
	FVector CalculateStandardBounceVelocity(const FVector& CurrentVelocity) const;
	EBounceTrajectoryType DetermineTrajectoryType(const FVector& Velocity) const;
	
	// Individual trajectory calculations - Single responsibility, highly testable
	FVector CalculateUpwardAmplification(const FVector& CurrentVelocity) const;
	FVector CalculateHorizontalEnhancement(const FVector& CurrentVelocity) const;
	FVector CalculateDiagonalEnhancement(const FVector& CurrentVelocity) const;
	FVector CalculateRecoveryJump(const FVector& CurrentVelocity) const;
	
	// EPIC GAMES STANDARD: Proper delegate lifecycle management
	void CleanupDelegates();

	// EPIC GAMES STANDARD: Async curve loading following proper asset management patterns
	void LoadCurveAssets();
	void OnCurveAssetsLoaded();

	// State Management - RAII principles with proper Epic Games standards
	// CRITICAL: Use TWeakObjectPtr for actor references to prevent dangling pointers
	UPROPERTY(Transient)
	TWeakObjectPtr<AMyCharacter> CachedCharacter;

	// Runtime State - Air bounce tracking
	UPROPERTY(Transient)
	int32 CurrentAirBounces = 0;

	UPROPERTY(Transient)
	bool bIsGrounded = true;

	UPROPERTY(Transient)
	float LastGroundContactTime = 0.0f;

	UPROPERTY(Transient)
	float BounceInputPressTime = 0.0f;

	// EPIC GAMES STANDARD: Proper delegate lifecycle management per-instance
	UPROPERTY(Transient)
	bool bLandedDelegateRegistered = false;

	// Performance optimization: Cache air bounce count during ability execution
	UPROPERTY(Transient)
	int32 CachedAirBounceCount = 0;

	// EPIC GAMES STANDARD: Curve asset loading state management
	UPROPERTY(Transient)
	TObjectPtr<UCurveFloat> LoadedBounceVelocityCurve;

	UPROPERTY(Transient)
	TObjectPtr<UCurveFloat> LoadedAirControlCurve;

	// Timers and handles
	FTimerHandle BounceEffectTimer;
	FTimerHandle GroundCheckTimer;
	FTimerHandle CurveLoadTimer;

	// EPIC GAMES STANDARD: Asset loading streamable handle management
	TSharedPtr<FStreamableHandle> CurveLoadHandle;

	// Constants - Epic Games style
	static constexpr float DEFAULT_GROUND_CHECK_RATE = 1.0f / 20.0f;
	static constexpr float MIN_BOUNCE_VELOCITY = 200.0f;
	static constexpr float MAX_BOUNCE_VELOCITY = 2000.0f;
	static constexpr int32 MAX_AIR_BOUNCES_LIMIT = 10;
	static constexpr float MIN_VELOCITY_THRESHOLD = 50.0f;
};
