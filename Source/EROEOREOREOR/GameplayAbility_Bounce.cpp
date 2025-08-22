// Fill out your copyright notice in the Description page of Project Settings.

#include "GameplayAbility_Bounce.h"
#include "MyCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "Curves/CurveFloat.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagsModule.h"

UGameplayAbility_Bounce::UGameplayAbility_Bounce()
{
	// GAS Configuration - Following Epic Games patterns (UE 5.6 API)
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ClientOrServer;

	// UE 5.6 API: Use SetAssetTags instead of AbilityTags.AddTag
	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Bounce")));
	SetAssetTags(AssetTags);

	// Activation tags setup
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Bouncing")));
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Stunned")));
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Input.Blocked")));

	// Initialize Gameplay Tags
	BouncingStateTag = FGameplayTag::RequestGameplayTag(FName("State.Bouncing"));
	BounceCooldownTag = FGameplayTag::RequestGameplayTag(FName("Cooldown.Bounce"));
	AirborneStateTag = FGameplayTag::RequestGameplayTag(FName("State.InAir"));
	BounceImmuneTag = FGameplayTag::RequestGameplayTag(FName("Immune.Bounce"));

	// Initialize Runtime State
	CurrentAirBounces = 0;
	bIsGrounded = true;
	LastGroundContactTime = 0.0f;
	BounceInputPressTime = 0.0f;
}

bool UGameplayAbility_Bounce::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	UE_LOG(LogTemp, Log, TEXT("UGameplayAbility_Bounce::CanActivateAbility - Starting validation"));

	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		UE_LOG(LogTemp, Warning, TEXT("UGameplayAbility_Bounce::CanActivateAbility - Super::CanActivateAbility returned false"));
		return false;
	}

	const AMyCharacter* Character = Cast<AMyCharacter>(ActorInfo->AvatarActor.Get());
	if (!Character)
	{
		UE_LOG(LogTemp, Warning, TEXT("UGameplayAbility_Bounce::CanActivateAbility - Character cast failed"));
		return false;
	}

	const bool bValidationResult = ValidateActivationRequirements(Character);
	UE_LOG(LogTemp, Log, TEXT("UGameplayAbility_Bounce::CanActivateAbility - Validation result: %s"), 
		bValidationResult ? TEXT("PASS") : TEXT("FAIL"));

	return bValidationResult;
}

void UGameplayAbility_Bounce::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Cache character reference for performance
	CachedCharacter = Cast<AMyCharacter>(ActorInfo->AvatarActor.Get());
	if (!IsValid(CachedCharacter))
	{
		UE_LOG(LogTemp, Error, TEXT("GameplayAbility_Bounce: Invalid character reference"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// CRITICAL FIX: Set up persistent ground contact delegate only once
	static bool bDelegateSetup = false;
	if (!bDelegateSetup && CachedCharacter)
	{
		CachedCharacter->LandedDelegate.AddDynamic(this, &UGameplayAbility_Bounce::OnLandedDelegate);
		bDelegateSetup = true;
		UE_LOG(LogTemp, Log, TEXT("Bounce: Ground contact delegate setup complete"));
	}

	// Execute the bounce
	ExecuteBounce();

	// Debug logging
	const bool bIsAirBounce = CurrentAirBounces > 0;
	UE_LOG(LogTemp, Log, TEXT("Bounce Executed: AirBounce=%s, Count=%d/%d, Grounded=%s"), 
		bIsAirBounce ? TEXT("True") : TEXT("False"), 
		CurrentAirBounces, 
		MaxAirBounces,
		IsCharacterGrounded(CachedCharacter) ? TEXT("true") : TEXT("false"));

	// End ability immediately (instantaneous like jump)
	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}

void UGameplayAbility_Bounce::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// Clean up timers
	if (BounceEffectTimer.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(BounceEffectTimer);
	}

	if (GroundCheckTimer.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(GroundCheckTimer);
	}

	// CRITICAL FIX: Do NOT remove delegate binding for bounce
	// The delegate must persist to reset air bounce counter on ground contact
	// Since bounce is instantaneous, we want the delegate to remain active

	// Keep character reference for persistent ground tracking
	// CachedCharacter = nullptr; // DO NOT CLEAR - needed for persistent ground tracking

	UE_LOG(LogTemp, Log, TEXT("Bounce EndAbility: Keeping delegate active for ground tracking"));

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGameplayAbility_Bounce::CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility)
{
	// Bounce is instantaneous, so cancellation just cleans up
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility);
}

void UGameplayAbility_Bounce::ExecuteBounce()
{
	if (!IsValid(CachedCharacter))
	{
		return;
	}

	UCharacterMovementComponent* MovementComponent = CachedCharacter->GetCharacterMovement();
	if (!IsValid(MovementComponent))
	{
		return;
	}

	// Check if this is an air bounce
	const bool bWasGrounded = IsCharacterGrounded(CachedCharacter);
	const bool bIsAirBounce = !bWasGrounded || CurrentAirBounces > 0;

	// Validate air bounce restrictions
	if (bIsAirBounce)
	{
		if (CurrentAirBounces >= MaxAirBounces)
		{
			if (bLogBounceEvents)
			{
				UE_LOG(LogTemp, Warning, TEXT("Bounce blocked: Max air bounces reached (%d/%d)"), CurrentAirBounces, MaxAirBounces);
			}
			return;
		}

		// Check if character is rising and we don't allow bounce while rising
		if (!bAllowBounceWhileRising && IsCharacterRising(CachedCharacter))
		{
			if (bLogBounceEvents)
			{
				UE_LOG(LogTemp, Warning, TEXT("Bounce blocked: Character is rising"));
			}
			return;
		}

		// Increment air bounce counter
		CurrentAirBounces++;
	}

	// Apply bounce physics
	ApplyBouncePhysics();

	// Apply bounce effects
	if (BounceDuration > 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimer(BounceEffectTimer, 
			FTimerDelegate::CreateUObject(this, &UGameplayAbility_Bounce::FinalizeBounce), 
			BounceDuration, 
			false);
	}

	// Debug visualization
	if (bEnableBounceDebugDraw)
	{
		const FVector StartLocation = CachedCharacter->GetActorLocation();
		const FVector BounceVelocity = CalculateBounceVelocity(CachedCharacter);
		const FVector EndLocation = StartLocation + (BounceVelocity * 0.5f);

		DrawDebugSphere(GetWorld(), StartLocation, 50.0f, 12, FColor::Green, false, DebugDrawDuration, 0, 2.0f);
		DrawDebugDirectionalArrow(GetWorld(), StartLocation, EndLocation, 100.0f, FColor::Yellow, false, DebugDrawDuration, 0, 3.0f);
		
		// Display air bounce count
		const FString DebugText = FString::Printf(TEXT("Air Bounces: %d/%d"), CurrentAirBounces, MaxAirBounces);
		DrawDebugString(GetWorld(), StartLocation + FVector(0, 0, 100), DebugText, nullptr, FColor::White, DebugDrawDuration);
	}
}

void UGameplayAbility_Bounce::ApplyBouncePhysics()
{
	if (!IsValid(CachedCharacter))
	{
		return;
	}

	UCharacterMovementComponent* MovementComponent = CachedCharacter->GetCharacterMovement();
	if (!IsValid(MovementComponent))
	{
		return;
	}

	// Calculate bounce velocity
	const FVector BounceVelocity = CalculateBounceVelocity(CachedCharacter);
	
	// Get current velocity
	FVector CurrentVelocity = MovementComponent->Velocity;
	
	// Preserve horizontal momentum based on retention setting
	FVector NewVelocity;
	NewVelocity.X = CurrentVelocity.X * HorizontalVelocityRetention * HorizontalVelocityMultiplier;
	NewVelocity.Y = CurrentVelocity.Y * HorizontalVelocityRetention * HorizontalVelocityMultiplier;
	
	// Set upward velocity (replace Z component or preserve based on settings)
	if (bPreserveDownwardMomentum && CurrentVelocity.Z < 0.0f)
	{
		NewVelocity.Z = FMath::Max(BounceVelocity.Z, CurrentVelocity.Z + BounceVelocity.Z);
	}
	else
	{
		NewVelocity.Z = BounceVelocity.Z;
	}

	// Apply velocity
	MovementComponent->Velocity = NewVelocity;

	// Modify gravity if specified
	if (bIgnoreGravityDuringBounce && BounceDuration > 0.0f)
	{
		MovementComponent->GravityScale *= GravityScaleDuringBounce;
	}

	// Set air control
	if (CurrentAirBounces > 0)
	{
		MovementComponent->AirControl *= AirControlMultiplier;
	}
}

void UGameplayAbility_Bounce::FinalizeBounce()
{
	if (!IsValid(CachedCharacter))
	{
		return;
	}

	UCharacterMovementComponent* MovementComponent = CachedCharacter->GetCharacterMovement();
	if (!IsValid(MovementComponent))
	{
		return;
	}

	// Restore gravity if it was modified
	if (bIgnoreGravityDuringBounce)
	{
		MovementComponent->GravityScale /= GravityScaleDuringBounce;
	}

	// Restore air control if it was modified
	if (CurrentAirBounces > 0 && AirControlMultiplier != 1.0f)
	{
		MovementComponent->AirControl /= AirControlMultiplier;
	}
}

void UGameplayAbility_Bounce::OnLandedDelegate(const FHitResult& Hit)
{
	if (bResetAirBouncesOnGroundContact)
	{
		CurrentAirBounces = 0;
		bIsGrounded = true;
		LastGroundContactTime = GetWorld()->GetTimeSeconds();

		if (bLogBounceEvents)
		{
			UE_LOG(LogTemp, Log, TEXT("Bounce: Ground contact - air bounces reset"));
		}
	}
}

void UGameplayAbility_Bounce::CheckGroundState()
{
	if (IsValid(CachedCharacter))
	{
		const bool bCurrentlyGrounded = IsCharacterGrounded(CachedCharacter);
		if (bCurrentlyGrounded != bIsGrounded)
		{
			bIsGrounded = bCurrentlyGrounded;
			if (bIsGrounded && bResetAirBouncesOnGroundContact)
			{
				// Create a dummy hit result for the ground contact
				FHitResult DummyHit;
				OnLandedDelegate(DummyHit);
			}
		}
	}
}

bool UGameplayAbility_Bounce::ValidateActivationRequirements(const AMyCharacter* InCharacter) const
{
	if (!IsValid(InCharacter))
	{
		UE_LOG(LogTemp, Warning, TEXT("Bounce Validation FAIL: Invalid character"));
		return false;
	}

	// Check if character has movement component
	const UCharacterMovementComponent* MovementComponent = InCharacter->GetCharacterMovement();
	if (!IsValid(MovementComponent))
	{
		UE_LOG(LogTemp, Warning, TEXT("Bounce Validation FAIL: Invalid movement component"));
		return false;
	}

	// Check air bounce limitations
	const bool bIsCurrentlyGrounded = IsCharacterGrounded(InCharacter);
	UE_LOG(LogTemp, Log, TEXT("Bounce Validation: Grounded=%s, AirBounces=%d/%d"), 
		bIsCurrentlyGrounded ? TEXT("true") : TEXT("false"), 
		CurrentAirBounces, MaxAirBounces);

	if (!bIsCurrentlyGrounded && CurrentAirBounces >= MaxAirBounces)
	{
		UE_LOG(LogTemp, Warning, TEXT("Bounce Validation FAIL: Max air bounces reached (%d/%d) and not grounded"), 
			CurrentAirBounces, MaxAirBounces);
		return false;
	}

	// Check coyote time for ground-to-air transition
	if (!bIsCurrentlyGrounded && CoyoteTime > 0.0f)
	{
		const float TimeSinceGroundContact = GetWorld()->GetTimeSeconds() - LastGroundContactTime;
		UE_LOG(LogTemp, Log, TEXT("Bounce Validation: TimeSinceGroundContact=%.3f, CoyoteTime=%.3f"), 
			TimeSinceGroundContact, CoyoteTime);

		if (TimeSinceGroundContact > CoyoteTime)
		{
			// Outside coyote time, check air bounce availability
			if (CurrentAirBounces >= MaxAirBounces)
			{
				UE_LOG(LogTemp, Warning, TEXT("Bounce Validation FAIL: Outside coyote time and max air bounces reached"));
				return false;
			}
		}
	}

	// Check if rising and bounce while rising is disabled
	const bool bIsRising = IsCharacterRising(InCharacter);
	if (!bAllowBounceWhileRising && bIsRising)
	{
		UE_LOG(LogTemp, Warning, TEXT("Bounce Validation FAIL: Character is rising and bounce while rising is disabled"));
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("Bounce Validation PASS: All checks passed"));
	return true;
}

bool UGameplayAbility_Bounce::IsCharacterGrounded(const AMyCharacter* InCharacter) const
{
	if (!IsValid(InCharacter))
	{
		return false;
	}

	const UCharacterMovementComponent* MovementComponent = InCharacter->GetCharacterMovement();
	return IsValid(MovementComponent) && MovementComponent->IsMovingOnGround();
}

bool UGameplayAbility_Bounce::IsCharacterRising(const AMyCharacter* InCharacter) const
{
	if (!IsValid(InCharacter))
	{
		return false;
	}

	const UCharacterMovementComponent* MovementComponent = InCharacter->GetCharacterMovement();
	if (!IsValid(MovementComponent))
	{
		return false;
	}

	return MovementComponent->Velocity.Z > MIN_VELOCITY_THRESHOLD;
}

FVector UGameplayAbility_Bounce::CalculateBounceVelocity(const AMyCharacter* InCharacter) const
{
	if (!IsValid(InCharacter))
	{
		return FVector::ZeroVector;
	}

	FVector BounceVelocity = FVector::ZeroVector;
	
	// Calculate effective bounce velocity (reduced for air bounces)
	const float EffectiveVelocity = GetEffectiveBounceVelocity();
	
	// Apply curve modification if available
	float CurveMultiplier = 1.0f;
	if (BounceVelocityCurve.IsValid())
	{
		const UCurveFloat* Curve = BounceVelocityCurve.LoadSynchronous();
		if (IsValid(Curve))
		{
			const float CurveInput = static_cast<float>(CurrentAirBounces) / FMath::Max(1.0f, static_cast<float>(MaxAirBounces));
			CurveMultiplier = Curve->GetFloatValue(CurveInput);
		}
	}

	BounceVelocity.Z = EffectiveVelocity * CurveMultiplier;
	return BounceVelocity;
}

float UGameplayAbility_Bounce::GetEffectiveBounceVelocity() const
{
	if (CurrentAirBounces == 0)
	{
		return BounceUpwardVelocity;
	}

	// Reduce velocity for air bounces
	return BounceUpwardVelocity * FMath::Pow(AirBounceVelocityReduction, static_cast<float>(CurrentAirBounces));
}

// Testing and Debugging Functions
void UGameplayAbility_Bounce::TestBounceParameters()
{
	UE_LOG(LogTemp, Warning, TEXT("=== BOUNCE ABILITY PARAMETERS ==="));
	UE_LOG(LogTemp, Warning, TEXT("BounceUpwardVelocity: %f"), BounceUpwardVelocity);
	UE_LOG(LogTemp, Warning, TEXT("MaxAirBounces: %d"), MaxAirBounces);
	UE_LOG(LogTemp, Warning, TEXT("HorizontalVelocityRetention: %f"), HorizontalVelocityRetention);
	UE_LOG(LogTemp, Warning, TEXT("CurrentAirBounces: %d"), CurrentAirBounces);
	UE_LOG(LogTemp, Warning, TEXT("AirBounceVelocityReduction: %f"), AirBounceVelocityReduction);
	UE_LOG(LogTemp, Warning, TEXT("CoyoteTime: %f"), CoyoteTime);
	UE_LOG(LogTemp, Warning, TEXT("==============================="));
}

bool UGameplayAbility_Bounce::CanPerformAirBounce() const
{
	return CurrentAirBounces < MaxAirBounces;
}

FVector UGameplayAbility_Bounce::GetCurrentHorizontalVelocity() const
{
	if (IsValid(CachedCharacter))
	{
		const UCharacterMovementComponent* MovementComponent = CachedCharacter->GetCharacterMovement();
		if (IsValid(MovementComponent))
		{
			const FVector Velocity = MovementComponent->Velocity;
			return FVector(Velocity.X, Velocity.Y, 0.0f);
		}
	}
	return FVector::ZeroVector;
}

float UGameplayAbility_Bounce::GetBounceInputTimeRemaining() const
{
	if (BounceInputWindow <= 0.0f)
	{
		return 0.0f;
	}
	
	const float CurrentTime = GetWorld()->GetTimeSeconds();
	const float TimeElapsed = CurrentTime - BounceInputPressTime;
	return FMath::Max(0.0f, BounceInputWindow - TimeElapsed);
}

// Configuration Presets
void UGameplayAbility_Bounce::ApplyLowBouncePreset()
{
	BounceUpwardVelocity = 500.0f;
	MaxAirBounces = 1;
	HorizontalVelocityRetention = 0.9f;
	AirBounceVelocityReduction = 0.8f;
}

void UGameplayAbility_Bounce::ApplyHighBouncePreset()
{
	BounceUpwardVelocity = 1200.0f;
	MaxAirBounces = 3;
	HorizontalVelocityRetention = 1.0f;
	AirBounceVelocityReduction = 0.75f;
}

void UGameplayAbility_Bounce::ApplyFloatyBouncePreset()
{
	BounceUpwardVelocity = 800.0f;
	MaxAirBounces = 2;
	GravityScaleDuringBounce = 0.5f;
	BounceDuration = 0.5f;
	bIgnoreGravityDuringBounce = true;
}

void UGameplayAbility_Bounce::ApplySnappyBouncePreset()
{
	BounceUpwardVelocity = 900.0f;
	MaxAirBounces = 2;
	HorizontalVelocityRetention = 1.0f;
	BounceDuration = 0.1f;
	AirControlMultiplier = 0.3f;
}

void UGameplayAbility_Bounce::ResetToDefaultPreset()
{
	BounceUpwardVelocity = 800.0f;
	HorizontalVelocityRetention = 1.0f;
	HorizontalVelocityMultiplier = 1.0f;
	AirControlMultiplier = 0.2f;
	MaxAirBounces = 2;
	AirBounceVelocityReduction = 0.85f;
	BounceInputWindow = 0.1f;
	CoyoteTime = 0.15f;
	GravityScaleDuringBounce = 1.0f;
	BounceDuration = 0.2f;
}
