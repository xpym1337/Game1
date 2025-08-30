// Fill out your copyright notice in the Description page of Project Settings.

#include "GameplayAbility_Dash.h"
#include "MyCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "Engine/World.h"
#include "Curves/CurveFloat.h"

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
	
	// NOTE: Default values are now set in header file only
	// This allows Blueprint editor changes to persist
	
	// State initialization - RAII principles
	DashDirection = EDashDirection::None;
	CachedCharacter = nullptr;
	bIsActiveDash = false;
	DashStartTime = 0.0f;
	StoredInputDirection = FVector2D::ZeroVector;
	LoadedDashSpeedCurve = nullptr;
	LoadedDashDirectionCurve = nullptr;
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

	// Cache character reference - EPIC GAMES STANDARD: Use weak pointer for actor references
	CachedCharacter = Cast<AMyCharacter>(ActorInfo->AvatarActor.Get());
	if (!CachedCharacter.IsValid())
	{
		DASH_LOG(Error, TEXT("ActivateAbility: Invalid character cast"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// EPIC GAMES STANDARD: Load curve assets asynchronously
	LoadCurveAssets();

	// Final validation with cached character
	if (!ValidateActivationRequirements(CachedCharacter.Get()))
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

	// EPIC GAMES STANDARD: Clean up streamable handles properly
	if (CurveLoadHandle.IsValid())
	{
		CurveLoadHandle->CancelHandle();
		CurveLoadHandle.Reset();
	}

	// Apply momentum retention if dash completed naturally
	AMyCharacter* Character = CachedCharacter.Get();
	if (!bWasCancelled && Character)
	{
		if (UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement())
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
	AMyCharacter* Character = CachedCharacter.Get();
	check(Character); // Epic Games style assertion for internal methods

	// EDITOR VALUES NOW RESPECTED - No more character blueprint override!
	// The ability's own editor parameters are now the authoritative source
	
	DASH_LOG(Warning, TEXT("Using Ability editor values - Speed: %.1f, Duration: %.2f, Momentum: %.2f"), 
		DashSpeed, DashDuration, MomentumRetention);

	// Store input direction relative to camera at activation time
	const FVector2D CurrentInput = Character->GetCurrentMovementInput();
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
	UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement();
	if (MovementComponent)
	{
		// Calculate initial direction and speed for immediate response
		const FVector DashDirectionVector = CalculateCameraRelativeDashDirection(Character);
		const float InitialSpeed = CalculateCurrentDashSpeed(0.0f); // Start at full speed
		
		// Apply velocity immediately - critical for responsive feel
		FVector DashVelocity = DashDirectionVector * InitialSpeed;
		DashVelocity.Z = MovementComponent->Velocity.Z; // Preserve gravity
		MovementComponent->Velocity = DashVelocity;
		
		// CAPTURE INITIAL HIGH-VELOCITY SNAPSHOT for momentum transfer
		if (UVelocitySnapshotComponent* SnapshotComponent = Character->GetVelocitySnapshotComponent())
		{
			const FGameplayTag DashTag = FGameplayTag::RequestGameplayTag(FName("State.Dashing"));
			SnapshotComponent->CaptureSnapshot(DashVelocity, EVelocitySource::Dash, DashTag);
		}
		
		DASH_LOG(Log, TEXT("ExecuteDash: IMMEDIATE velocity applied - Direction: (%.2f, %.2f, %.2f), Speed: %.2f"), 
			DashDirectionVector.X, DashDirectionVector.Y, DashDirectionVector.Z, InitialSpeed);
	}

	// Calculate update rate with bounds checking
	const float ClampedUpdateFrequency = FMath::Clamp(UpdateFrequency, 10.0f, 120.0f);
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
	
	// Check if curve is loaded and valid - EPIC GAMES STANDARD: Use loaded curve assets
	if (IsValid(LoadedDashSpeedCurve))
	{
		CurrentSpeed *= LoadedDashSpeedCurve->GetFloatValue(ClampedAlpha);
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
	AMyCharacter* Character = CachedCharacter.Get();
	if (!bIsActiveDash || !Character)
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
	UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement();
	if (!MovementComponent)
	{
		DASH_LOG(Warning, TEXT("UpdateDashVelocity: Lost movement component - ending dash"));
		FinalizeDash();
		return;
	}

	// Calculate direction and speed
	const FVector DashDirectionVector = CalculateCameraRelativeDashDirection(Character);
	const float CurrentSpeed = CalculateCurrentDashSpeed(Alpha);

	// Apply velocity with Z-preservation and new multipliers - critical for gravity
	FVector DashVelocity = DashDirectionVector * CurrentSpeed;
	
	// Apply new axis-specific multipliers
	DashVelocity.X *= DashXAxisMultiplier;
	DashVelocity.Y *= DashYAxisMultiplier;
	
	// Apply dash-specific multipliers
	DashVelocity *= DashVelocityMultiplier;
	
	DashVelocity.Z = MovementComponent->Velocity.Z;
	MovementComponent->Velocity = DashVelocity;

	// CAPTURE VELOCITY SNAPSHOT for momentum transfer (only during strong dash phase)
	if (Alpha < 0.8f && CurrentSpeed > 500.0f) // Only capture meaningful velocity
	{
		if (UVelocitySnapshotComponent* SnapshotComponent = Character->GetVelocitySnapshotComponent())
		{
			const FGameplayTag DashTag = FGameplayTag::RequestGameplayTag(FName("State.Dashing"));
			SnapshotComponent->CaptureSnapshot(DashVelocity, EVelocitySource::Dash, DashTag);
		}
	}
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

// EPIC GAMES STANDARD: Async curve loading following proper asset management patterns
void UGameplayAbility_Dash::LoadCurveAssets()
{
	TArray<FSoftObjectPath> AssetsToLoad;

	// Collect assets that need loading - EPIC GAMES STANDARD: Only load what's needed
	if (!DashSpeedCurve.IsNull() && !IsValid(LoadedDashSpeedCurve))
	{
		AssetsToLoad.Add(DashSpeedCurve.ToSoftObjectPath());
	}
	
	if (!DashDirectionCurve.IsNull() && !IsValid(LoadedDashDirectionCurve))
	{
		AssetsToLoad.Add(DashDirectionCurve.ToSoftObjectPath());
	}

	// Early exit if no assets to load
	if (AssetsToLoad.Num() == 0)
	{
		return;
	}

	// Load assets asynchronously - EPIC GAMES STANDARD: Use StreamableManager
	UAssetManager& AssetManager = UAssetManager::Get();
	CurveLoadHandle = AssetManager.LoadAssetList(AssetsToLoad, 
		FStreamableDelegate::CreateUObject(this, &UGameplayAbility_Dash::OnCurveAssetsLoaded));

	if (CurveLoadHandle.IsValid())
	{
		UE_LOG(LogTemp, Log, TEXT("Dash: Started loading %d curve assets"), AssetsToLoad.Num());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Dash: Failed to start curve asset loading"));
	}
}

void UGameplayAbility_Dash::OnCurveAssetsLoaded()
{
	// EPIC GAMES STANDARD: Safe asset loading with proper validation
	if (!DashSpeedCurve.IsNull())
	{
		LoadedDashSpeedCurve = DashSpeedCurve.LoadSynchronous();
		if (IsValid(LoadedDashSpeedCurve))
		{
			UE_LOG(LogTemp, Log, TEXT("Dash: DashSpeedCurve loaded successfully"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Dash: Failed to load DashSpeedCurve"));
		}
	}

	if (!DashDirectionCurve.IsNull())
	{
		LoadedDashDirectionCurve = DashDirectionCurve.LoadSynchronous();
		if (IsValid(LoadedDashDirectionCurve))
		{
			UE_LOG(LogTemp, Log, TEXT("Dash: DashDirectionCurve loaded successfully"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Dash: Failed to load DashDirectionCurve"));
		}
	}

	// Clean up handle after loading
	if (CurveLoadHandle.IsValid())
	{
		CurveLoadHandle.Reset();
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
	const AMyCharacter* Character = CachedCharacter.Get();
	if (!bIsActiveDash || !Character)
	{
		return FVector::ZeroVector;
	}

	return CalculateCameraRelativeDashDirection(Character);
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
