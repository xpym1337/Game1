// Fill out your copyright notice in the Description page of Project Settings.

#include "GameplayAbility_Dash.h"
#include "MyCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "Engine/World.h"

// Conditional logging - Epic Games standard approach using LogTemp
#if !UE_BUILD_SHIPPING
    #define DASH_LOG(Verbosity, Format, ...) UE_LOG(LogTemp, Verbosity, TEXT("[DashAbility] ") Format, ##__VA_ARGS__)
#else  
    #define DASH_LOG(Verbosity, Format, ...)
#endif

UGameplayAbility_Dash::UGameplayAbility_Dash()
{
	// Ability configuration - Epic Games standards
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	
	// Ability tags setup - tags now registered natively in game module
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Dashing")));
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Dashing")));
	
	// Tag initialization
	DashingStateTag = FGameplayTag::RequestGameplayTag(FName("State.Dashing"));
	DashCooldownTag = FGameplayTag::RequestGameplayTag(FName("Cooldown.Dash"));
	DashImmuneTag = FGameplayTag::RequestGameplayTag(FName("Immune.Dash"));
	
	// Initialize all new parameters with default values - RAII principles
	DashSpeed = 1875.0f;
	DashInitialBurstSpeed = 2500.0f;
	DashSpeedDecayRate = 0.15f;
	DashVerticalVelocityPreservation = 1.0f;
	DashDuration = 0.8f;
	DashInitialBurstDuration = 0.1f;
	DashCancelWindow = 0.2f;
	UpdateFrequency = 60.0f;
	MomentumRetention = 0.3f;
	DashAirControl = 0.1f;
	DashGroundFrictionOverride = 0.0f;
	bPreserveVerticalMomentum = true;
	bIgnoreGravityDuringDash = false;
	bUseDistanceBasedDash = false;
	DashTargetDistance = 800.0f;
	DashDistanceTolerance = 50.0f;
	InputBufferDuration = 0.1f;
	InputDirectionInfluence = 0.3f;
	bAllowDashDirectionOverride = true;
	DashCameraShakeIntensity = 0.3f;
	bEnableDashTrail = true;
	bEnableDashScreenEffect = true;
	bEnableDashDebugDraw = false;
	DebugDrawDuration = 2.0f;
	
	// State initialization - RAII principles
	DashDirection = EDashDirection::None;
	CachedCharacter = nullptr;
	bIsActiveDash = false;
	DashStartTime = 0.0f;
	StoredInputDirection = FVector2D::ZeroVector;
}

bool UGameplayAbility_Dash::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	// Parent validation first
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		DASH_LOG(VeryVerbose, TEXT("CanActivateAbility: Parent validation failed"));
		return false;
	}

	// Character validation
	const AMyCharacter* Character = Cast<AMyCharacter>(ActorInfo->AvatarActor.Get());
	if (!ValidateActivationRequirements(Character))
	{
		return false;
	}

	// NOTE: We don't check DashDirection here because it's set during input handling
	// This allows for proper GAS flow where direction is set just before activation

	return true;
}

void UGameplayAbility_Dash::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	// Epic Games standard: Extract direction from natively registered tags
	EDashDirection ActivationDirection = EDashDirection::None;
	
	if (ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
	{
		const UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
		
		// Use natively registered tags - guaranteed to be available
		if (ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Input.Dash.Left"))))
		{
			ActivationDirection = EDashDirection::Left;
		}
		else if (ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Input.Dash.Right"))))
		{
			ActivationDirection = EDashDirection::Right;
		}
	}

	DASH_LOG(Log, TEXT("ActivateAbility: Starting dash with direction %s"), 
		ActivationDirection == EDashDirection::Left ? TEXT("LEFT") : 
		ActivationDirection == EDashDirection::Right ? TEXT("RIGHT") : TEXT("NONE"));

	// Validate direction was communicated through gameplay tags
	if (ActivationDirection == EDashDirection::None)
	{
		DASH_LOG(Warning, TEXT("ActivateAbility: No dash direction tag found - aborting"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Set the direction for this activation
	DashDirection = ActivationDirection;

	// Commit ability resources
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		DASH_LOG(Warning, TEXT("ActivateAbility: CommitAbility failed"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Cache character reference
	CachedCharacter = Cast<AMyCharacter>(ActorInfo->AvatarActor.Get());
	if (!CachedCharacter)
	{
		DASH_LOG(Error, TEXT("ActivateAbility: Invalid character cast"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Final validation with cached character
	if (!ValidateActivationRequirements(CachedCharacter))
	{
		DASH_LOG(Error, TEXT("ActivateAbility: Validation failed with cached character"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Execute dash - single method, single responsibility
	ExecuteDash();

	DASH_LOG(Log, TEXT("ActivateAbility: Successfully started dash"));
}

void UGameplayAbility_Dash::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	DASH_LOG(Log, TEXT("EndAbility: Ending dash - Cancelled: %s"), bWasCancelled ? TEXT("true") : TEXT("false"));

	// Clean up timer resources
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(VelocityUpdateTimer);
	}

	// Apply momentum retention if dash completed naturally
	if (!bWasCancelled && CachedCharacter)
	{
		if (UCharacterMovementComponent* MovementComponent = CachedCharacter->GetCharacterMovement())
		{
			FVector CurrentVelocity = MovementComponent->Velocity;
			CurrentVelocity.X *= MomentumRetention;
			CurrentVelocity.Y *= MomentumRetention;
			MovementComponent->Velocity = CurrentVelocity;
			
			DASH_LOG(VeryVerbose, TEXT("EndAbility: Applied momentum retention %.2f"), MomentumRetention);
		}
	}

	// Reset state - RAII cleanup
	bIsActiveDash = false;
	CachedCharacter = nullptr;
	DashDirection = EDashDirection::None;
	StoredInputDirection = FVector2D::ZeroVector;
	DashStartTime = 0.0f;

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGameplayAbility_Dash::CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility)
{
	DASH_LOG(Log, TEXT("CancelAbility: Dash ability cancelled"));
	
	// Immediate cleanup
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(VelocityUpdateTimer);
	}

	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility);
}

// Private Implementation Methods

bool UGameplayAbility_Dash::ValidateActivationRequirements(const AMyCharacter* InCharacter) const
{
	if (!InCharacter)
	{
		DASH_LOG(Warning, TEXT("ValidateActivationRequirements: Invalid character"));
		return false;
	}

	if (!InCharacter->GetCharacterMovement())
	{
		DASH_LOG(Warning, TEXT("ValidateActivationRequirements: No character movement component"));
		return false;
	}

	if (!InCharacter->GetFollowCamera())
	{
		DASH_LOG(Warning, TEXT("ValidateActivationRequirements: No follow camera component"));
		return false;
	}

	return true;
}

void UGameplayAbility_Dash::ExecuteDash()
{
	check(CachedCharacter); // Epic Games style assertion for internal methods

	// Store input direction relative to camera at activation time
	const FVector2D CurrentInput = CachedCharacter->GetCurrentMovementInput();
	if (CurrentInput.IsNearlyZero())
	{
		// Default to pure side movement if no input - Epic Games default behavior pattern
		StoredInputDirection = (DashDirection == EDashDirection::Left) ? 
			FVector2D(-1.0f, 0.0f) : FVector2D(1.0f, 0.0f);
	}
	else
	{
		StoredInputDirection = CurrentInput.GetSafeNormal();
	}

	// Initialize dash state
	bIsActiveDash = true;
	DashStartTime = GetWorld()->GetTimeSeconds();

	// IMMEDIATE VELOCITY APPLICATION - No delay for first frame!
	UCharacterMovementComponent* MovementComponent = CachedCharacter->GetCharacterMovement();
	if (MovementComponent)
	{
		// Calculate initial direction and speed for immediate response
		const FVector DashDirectionVector = CalculateCameraRelativeDashDirection(CachedCharacter);
		const float InitialSpeed = CalculateCurrentDashSpeed(0.0f); // Start at full speed
		
		// Apply velocity immediately - critical for responsive feel
		FVector DashVelocity = DashDirectionVector * InitialSpeed;
		DashVelocity.Z = MovementComponent->Velocity.Z; // Preserve gravity
		MovementComponent->Velocity = DashVelocity;
		
		DASH_LOG(Log, TEXT("ExecuteDash: IMMEDIATE velocity applied - Direction: (%.2f, %.2f, %.2f), Speed: %.2f"), 
			DashDirectionVector.X, DashDirectionVector.Y, DashDirectionVector.Z, InitialSpeed);
	}

	// Calculate update rate with bounds checking
	const float ClampedUpdateFrequency = FMath::Clamp(UpdateFrequency, 10.0f, 60.0f);
	const float UpdateRate = 1.0f / ClampedUpdateFrequency;

	// Start velocity updates using Epic Games timer delegate pattern
	if (UWorld* World = GetWorld())
	{
		FTimerDelegate UpdateDelegate = FTimerDelegate::CreateUObject(this, &UGameplayAbility_Dash::UpdateDashVelocity);
		World->GetTimerManager().SetTimer(VelocityUpdateTimer, UpdateDelegate, UpdateRate, true);
	}

	DASH_LOG(Log, TEXT("ExecuteDash: Started with input (%.2f, %.2f), update rate %.3f"), 
		StoredInputDirection.X, StoredInputDirection.Y, UpdateRate);
}

FVector UGameplayAbility_Dash::CalculateCameraRelativeDashDirection(const AMyCharacter* InCharacter) const
{
	check(InCharacter); // Epic Games assertion pattern

	const UCameraComponent* FollowCamera = InCharacter->GetFollowCamera();
	check(FollowCamera);

	// Get camera's right vector - this is our pure left/right axis for screen-relative movement
	FVector CameraRightVector = FollowCamera->GetRightVector();
	
	// Project to horizontal plane (remove vertical component)
	CameraRightVector.Z = 0.0f;
	CameraRightVector.Normalize();
	
	// Apply direction based on dash type - CAMERA RELATIVE IMPLEMENTATION
	FVector DashDirectionVector = FVector::ZeroVector;
	
	switch (DashDirection)
	{
		case EDashDirection::Left:
			// Left dash always moves toward left edge of screen (negative camera right)
			DashDirectionVector = -CameraRightVector;
			break;
			
		case EDashDirection::Right:
			// Right dash always moves toward right edge of screen (positive camera right)
			DashDirectionVector = CameraRightVector;
			break;
			
		default:
			DASH_LOG(Warning, TEXT("Invalid dash direction - defaulting to right"));
			DashDirectionVector = CameraRightVector;
			break;
	}
	
	return DashDirectionVector.GetSafeNormal();
}

float UGameplayAbility_Dash::CalculateCurrentDashSpeed(const float InAlpha) const
{
	// Bounds checking
	const float ClampedAlpha = FMath::Clamp(InAlpha, 0.0f, 1.0f);
	
	float CurrentSpeed = FMath::Clamp(DashSpeed, MIN_DASH_SPEED, MAX_DASH_SPEED);
	
	// Check if curve is loaded and valid
	if (DashSpeedCurve.IsValid() && DashSpeedCurve.LoadSynchronous())
	{
		const UCurveFloat* LoadedCurve = DashSpeedCurve.Get();
		if (LoadedCurve)
		{
			CurrentSpeed *= LoadedCurve->GetFloatValue(ClampedAlpha);
		}
	}
	else
	{
		// Default quadratic ease-out - industry standard
		const float EaseMultiplier = 1.0f - FMath::Pow(1.0f - ClampedAlpha, 2.0f);
		const float SpeedMultiplier = FMath::Lerp(1.0f, 0.2f, EaseMultiplier);
		CurrentSpeed *= SpeedMultiplier;
	}

	return CurrentSpeed;
}

void UGameplayAbility_Dash::UpdateDashVelocity()
{
	// Early exit guards - Epic Games pattern
	if (!bIsActiveDash || !CachedCharacter)
	{
		return;
	}

	// Calculate dash progress
	const float CurrentTime = GetWorld()->GetTimeSeconds();
	const float ElapsedTime = CurrentTime - DashStartTime;
	const float Alpha = FMath::Clamp(ElapsedTime / DashDuration, 0.0f, 1.0f);

	// Check completion
	if (Alpha >= 1.0f)
	{
		FinalizeDash();
		return;
	}

	// Component validation - defensive programming
	UCharacterMovementComponent* MovementComponent = CachedCharacter->GetCharacterMovement();
	if (!MovementComponent)
	{
		DASH_LOG(Warning, TEXT("UpdateDashVelocity: Lost movement component - ending dash"));
		FinalizeDash();
		return;
	}

	// Calculate direction and speed
	const FVector DashDirectionVector = CalculateCameraRelativeDashDirection(CachedCharacter);
	const float CurrentSpeed = CalculateCurrentDashSpeed(Alpha);

	// Apply velocity with Z-preservation - critical for gravity
	FVector DashVelocity = DashDirectionVector * CurrentSpeed;
	DashVelocity.Z = MovementComponent->Velocity.Z;
	MovementComponent->Velocity = DashVelocity;
}

void UGameplayAbility_Dash::FinalizeDash()
{
	// Clean up timer
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(VelocityUpdateTimer);
	}

	bIsActiveDash = false;

	DASH_LOG(Log, TEXT("FinalizeDash: Dash completed naturally"));

	// End ability through GAS - proper flow
	if (IsActive())
	{
		K2_EndAbility();
	}
}

// TESTING AND DEBUG IMPLEMENTATIONS - Epic Games debugging standards

void UGameplayAbility_Dash::TestDashParameters()
{
	DASH_LOG(Warning, TEXT("=== DASH PARAMETER TEST ==="));
	DASH_LOG(Warning, TEXT("DashSpeed: %.2f"), DashSpeed);
	DASH_LOG(Warning, TEXT("DashDuration: %.2f"), DashDuration);
	DASH_LOG(Warning, TEXT("DashInitialBurstSpeed: %.2f"), DashInitialBurstSpeed);
	DASH_LOG(Warning, TEXT("MomentumRetention: %.2f"), MomentumRetention);
	DASH_LOG(Warning, TEXT("UpdateFrequency: %.2f"), UpdateFrequency);
	DASH_LOG(Warning, TEXT("=== END TEST ==="));
}

float UGameplayAbility_Dash::GetCurrentDashProgress() const
{
	if (!bIsActiveDash || DashStartTime <= 0.0f)
	{
		return 0.0f;
	}

	const float CurrentTime = GetWorld()->GetTimeSeconds();
	const float ElapsedTime = CurrentTime - DashStartTime;
	return FMath::Clamp(ElapsedTime / DashDuration, 0.0f, 1.0f);
}

FVector UGameplayAbility_Dash::GetCurrentDashDirection() const
{
	if (!bIsActiveDash || !CachedCharacter)
	{
		return FVector::ZeroVector;
	}

	return CalculateCameraRelativeDashDirection(CachedCharacter);
}

float UGameplayAbility_Dash::GetCurrentDashSpeed() const
{
	if (!bIsActiveDash)
	{
		return 0.0f;
	}

	const float Progress = GetCurrentDashProgress();
	return CalculateCurrentDashSpeed(Progress);
}

// PRESET IMPLEMENTATIONS - Epic Games standards

void UGameplayAbility_Dash::ApplyQuickDashPreset()
{
	// Fast, snappy dash feel
	DashSpeed = 2200.0f;
	DashInitialBurstSpeed = 2800.0f;
	DashDuration = 0.4f;
	DashInitialBurstDuration = 0.05f;
	DashSpeedDecayRate = 0.25f;
	MomentumRetention = 0.1f;
	UpdateFrequency = 90.0f;
	
	DASH_LOG(Warning, TEXT("Applied Quick Dash Preset"));
}

void UGameplayAbility_Dash::ApplyFloatyDashPreset()
{
	// Smooth, extended dash feel
	DashSpeed = 1400.0f;
	DashInitialBurstSpeed = 1600.0f;
	DashDuration = 1.2f;
	DashInitialBurstDuration = 0.15f;
	DashSpeedDecayRate = 0.05f;
	MomentumRetention = 0.6f;
	UpdateFrequency = 45.0f;
	
	DASH_LOG(Warning, TEXT("Applied Floaty Dash Preset"));
}

void UGameplayAbility_Dash::ApplySnappyDashPreset()
{
	// Immediate, precise dash feel
	DashSpeed = 2500.0f;
	DashInitialBurstSpeed = 3000.0f;
	DashDuration = 0.25f;
	DashInitialBurstDuration = 0.02f;
	DashSpeedDecayRate = 0.4f;
	MomentumRetention = 0.05f;
	UpdateFrequency = 120.0f;
	
	DASH_LOG(Warning, TEXT("Applied Snappy Dash Preset"));
}

void UGameplayAbility_Dash::ApplyPowerfulDashPreset()
{
	// High-speed, impactful dash feel
	DashSpeed = 2800.0f;
	DashInitialBurstSpeed = 3500.0f;
	DashDuration = 0.6f;
	DashInitialBurstDuration = 0.08f;
	DashSpeedDecayRate = 0.18f;
	MomentumRetention = 0.4f;
	UpdateFrequency = 75.0f;
	
	DASH_LOG(Warning, TEXT("Applied Powerful Dash Preset"));
}

void UGameplayAbility_Dash::ResetToDefaultPreset()
{
	// Reset to original balanced values
	DashSpeed = 1875.0f;
	DashInitialBurstSpeed = 2500.0f;
	DashDuration = 0.8f;
	DashInitialBurstDuration = 0.1f;
	DashSpeedDecayRate = 0.15f;
	MomentumRetention = 0.3f;
	UpdateFrequency = 60.0f;
	
	DASH_LOG(Warning, TEXT("Reset to Default Dash Preset"));
}
