// Fill out your copyright notice in the Description page of Project Settings.

#include "GameplayAbility_Bounce.h"
#include "MyCharacter.h"
#include "MyAttributeSet.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "Curves/CurveFloat.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagsModule.h"
#include "GameplayEffect.h"

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

	// Cache character reference for performance - EPIC GAMES STANDARD: Use weak pointer for actor references
	CachedCharacter = Cast<AMyCharacter>(ActorInfo->AvatarActor.Get());
	if (!CachedCharacter.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("GameplayAbility_Bounce: Invalid character reference"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// EPIC GAMES STANDARD: Proper per-instance delegate management with RAII principles
	AMyCharacter* Character = CachedCharacter.Get();
	if (Character && !bLandedDelegateRegistered)
	{
		Character->LandedDelegate.AddDynamic(this, &UGameplayAbility_Bounce::OnLandedDelegate);
		bLandedDelegateRegistered = true;
		UE_LOG(LogTemp, Log, TEXT("Bounce: Ground contact delegate registered for this instance"));
	}

	// EPIC GAMES STANDARD: Load curve assets asynchronously
	LoadCurveAssets();

	// PERFORMANCE: Cache air bounce count during ability execution
	CachedAirBounceCount = GetCurrentAirBounceCount();

	// Execute the bounce
	ExecuteBounce();

	// Debug logging
	const bool bIsAirBounce = CurrentAirBounces > 0;
	UE_LOG(LogTemp, Log, TEXT("Bounce Executed: AirBounce=%s, Count=%d/%d, Grounded=%s"), 
		bIsAirBounce ? TEXT("True") : TEXT("False"), 
		CurrentAirBounces, 
		MaxAirBounces,
		IsCharacterGrounded(CachedCharacter.Get()) ? TEXT("true") : TEXT("false"));

	// End ability immediately (instantaneous like jump)
	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}

void UGameplayAbility_Bounce::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// EPIC GAMES STANDARD: Proper RAII cleanup - clean up timers first
	if (BounceEffectTimer.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(BounceEffectTimer);
	}

	if (GroundCheckTimer.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(GroundCheckTimer);
	}

	// EPIC GAMES STANDARD: Clean up streamable handles properly
	if (CurveLoadHandle.IsValid())
	{
		CurveLoadHandle->CancelHandle();
		CurveLoadHandle.Reset();
	}

	// EPIC GAMES STANDARD: Use centralized cleanup function for maintainability
	// For bounce, delegate stays active for ground tracking unless cancelled
	if (bWasCancelled)
	{
		CleanupDelegates();
		UE_LOG(LogTemp, Log, TEXT("Bounce EndAbility: All delegates cleaned up due to cancellation"));
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Bounce EndAbility: Keeping delegate active for persistent ground tracking"));
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGameplayAbility_Bounce::CleanupDelegates()
{
	// EPIC GAMES STANDARD: Centralized delegate cleanup following RAII principles
	AMyCharacter* Character = CachedCharacter.Get();
	if (Character && bLandedDelegateRegistered)
	{
		Character->LandedDelegate.RemoveDynamic(this, &UGameplayAbility_Bounce::OnLandedDelegate);
		bLandedDelegateRegistered = false;
		UE_LOG(LogTemp, VeryVerbose, TEXT("Bounce: Landed delegate cleaned up"));
	}
	
	// Clear cached character reference safely
	CachedCharacter = nullptr;
}

void UGameplayAbility_Bounce::CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility)
{
	// Bounce is instantaneous, so cancellation just cleans up
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility);
}

void UGameplayAbility_Bounce::ExecuteBounce()
{
	AMyCharacter* Character = CachedCharacter.Get();
	if (!IsValid(Character))
	{
		UE_LOG(LogTemp, Error, TEXT("ExecuteBounce: Invalid cached character"));
		return;
	}

	UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement();
	if (!IsValid(MovementComponent))
	{
		UE_LOG(LogTemp, Error, TEXT("ExecuteBounce: Invalid movement component"));
		return;
	}

	// CRITICAL FIX: Re-validate activation requirements at execution time
	// This ensures consistency between CanActivateAbility and ExecuteBounce
	if (!ValidateActivationRequirements(Character))
	{
		UE_LOG(LogTemp, Error, TEXT("ExecuteBounce: Validation failed at execution time - aborting bounce"));
		return;
	}

	// Determine if this is an air bounce
	const bool bWasGrounded = IsCharacterGrounded(Character);
	const bool bIsAirBounce = !bWasGrounded;

	// INDUSTRY BEST PRACTICE: Use Gameplay Attributes for state management
	if (bIsAirBounce)
	{
		IncrementAirBounceCount();
		const int32 NewCount = GetCurrentAirBounceCount();
		UE_LOG(LogTemp, Log, TEXT("ExecuteBounce: Air bounce executed, count now %d/%d"), NewCount, MaxAirBounces);
	}
	else
	{
		const int32 CurrentCount = GetCurrentAirBounceCount();
		UE_LOG(LogTemp, Log, TEXT("ExecuteBounce: Ground bounce executed, air bounce count remains %d/%d"), CurrentCount, MaxAirBounces);
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
		const FVector StartLocation = Character->GetActorLocation();
		const FVector BounceVelocity = CalculateBounceVelocity(Character);
		const FVector EndLocation = StartLocation + (BounceVelocity * 0.5f);

		DrawDebugSphere(GetWorld(), StartLocation, 50.0f, 12, FColor::Green, false, DebugDrawDuration, 0, 2.0f);
		DrawDebugDirectionalArrow(GetWorld(), StartLocation, EndLocation, 100.0f, FColor::Yellow, false, DebugDrawDuration, 0, 3.0f);
		
		// Display air bounce count
		const int32 CurrentCount = GetCurrentAirBounceCount();
		const FString DebugText = FString::Printf(TEXT("Air Bounces: %d/%d"), CurrentCount, MaxAirBounces);
		DrawDebugString(GetWorld(), StartLocation + FVector(0, 0, 100), DebugText, nullptr, FColor::White, DebugDrawDuration);
	}
}

void UGameplayAbility_Bounce::ApplyBouncePhysics()
{
	AMyCharacter* Character = CachedCharacter.Get();
	if (!IsValid(Character))
	{
		return;
	}

	UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement();
	if (!IsValid(MovementComponent))
	{
		return;
	}

	// PERFORMANCE OPTIMIZATION: Try enhanced bounce first (single code path)
	FVector NewVelocity;
	bool bUsedMomentumTransfer = false;

	if (bAllowComboBounce && bAllowMomentumTransfer)
	{
		const FVector EnhancedVelocity = CalculateEnhancedBounceVelocity(Character);
		if (!EnhancedVelocity.IsZero())
		{
			NewVelocity = EnhancedVelocity;
			bUsedMomentumTransfer = true;
			UE_LOG(LogTemp, Warning, TEXT("ApplyBouncePhysics: USING MOMENTUM TRANSFER - Enhanced velocity applied!"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("ApplyBouncePhysics: Enhanced velocity calculation returned zero"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ApplyBouncePhysics: Combo bounce disabled - AllowCombo=%s, AllowMomentum=%s"),
			bAllowComboBounce ? TEXT("true") : TEXT("false"),
			bAllowMomentumTransfer ? TEXT("true") : TEXT("false"));
	}

	// Fallback to standard bounce calculation
	if (!bUsedMomentumTransfer)
	{
		const FVector BounceVelocity = CalculateBounceVelocity(Character);
		FVector CurrentVelocity = MovementComponent->Velocity;
		
		NewVelocity.X = CurrentVelocity.X * HorizontalVelocityRetention * HorizontalVelocityMultiplier;
		NewVelocity.Y = CurrentVelocity.Y * HorizontalVelocityRetention * HorizontalVelocityMultiplier;
		
		if (bPreserveDownwardMomentum && CurrentVelocity.Z < 0.0f)
		{
			NewVelocity.Z = FMath::Max(BounceVelocity.Z, CurrentVelocity.Z + BounceVelocity.Z);
		}
		else
		{
			NewVelocity.Z = BounceVelocity.Z;
		}
	}

	// Apply calculated velocity
	MovementComponent->Velocity = NewVelocity;

	// Rest of existing physics modification code remains unchanged...
	if (bIgnoreGravityDuringBounce && BounceDuration > 0.0f)
	{
		MovementComponent->GravityScale *= GravityScaleDuringBounce;
	}

	if (CurrentAirBounces > 0)
	{
		MovementComponent->AirControl *= AirControlMultiplier;
	}
}

void UGameplayAbility_Bounce::FinalizeBounce()
{
	AMyCharacter* Character = CachedCharacter.Get();
	if (!IsValid(Character))
	{
		return;
	}

	UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement();
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
		// INDUSTRY BEST PRACTICE: Use Gameplay Attributes for state management
		ResetAirBounceCount();
		bIsGrounded = true;
		LastGroundContactTime = GetWorld()->GetTimeSeconds();

		if (bLogBounceEvents)
		{
			UE_LOG(LogTemp, Log, TEXT("Bounce: Ground contact - air bounces reset via Gameplay Attributes"));
		}
	}
}

void UGameplayAbility_Bounce::CheckGroundState()
{
	AMyCharacter* Character = CachedCharacter.Get();
	if (IsValid(Character))
	{
		const bool bCurrentlyGrounded = IsCharacterGrounded(Character);
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

	// DASH-BOUNCE INTEGRATION: Check for active dash state
	const UAbilitySystemComponent* ASC = InCharacter->GetAbilitySystemComponent();
	const bool bIsDashing = ASC && ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.Dashing")));
	const bool bIsJumping = MovementComponent->IsFalling() && MovementComponent->Velocity.Z > 0.0f;
	
	// INDUSTRY BEST PRACTICE: Get air bounce count from Gameplay Attributes for reliable state
	const int32 ActualCurrentAirBounces = GetCurrentAirBounceCount();

	// Check air bounce limitations
	const bool bIsCurrentlyGrounded = IsCharacterGrounded(InCharacter);
	UE_LOG(LogTemp, Log, TEXT("Bounce Validation: Grounded=%s, Dashing=%s, Jumping=%s, AirBounces=%d/%d"), 
		bIsCurrentlyGrounded ? TEXT("true") : TEXT("false"),
		bIsDashing ? TEXT("true") : TEXT("false"),
		bIsJumping ? TEXT("true") : TEXT("false"),
		ActualCurrentAirBounces, MaxAirBounces);

	// DASH-BOUNCE INTEGRATION: Allow bounce during dash regardless of ground state
	if (bIsDashing)
	{
		// During dash, allow bounce even if air bounce limit would be exceeded by 1
		// This enables dash-bounce combos while maintaining reasonable limits
		const int32 BouncesAfterThisOne = ActualCurrentAirBounces + 1;
		if (BouncesAfterThisOne > (MaxAirBounces + 1))
		{
			UE_LOG(LogTemp, Warning, TEXT("Bounce Validation FAIL: Dash-bounce would exceed extended limit (%d would become %d/%d+1)"), 
				ActualCurrentAirBounces, BouncesAfterThisOne, MaxAirBounces);
			return false;
		}
		UE_LOG(LogTemp, Log, TEXT("Bounce Validation PASS: Dash-bounce combo allowed"));
		return true;
	}

	// JUMP-BOUNCE INTEGRATION: Allow bounce during jump with normal air bounce rules
	if (bIsJumping)
	{
		const int32 BouncesAfterThisOne = ActualCurrentAirBounces + 1;
		if (BouncesAfterThisOne > MaxAirBounces)
		{
			UE_LOG(LogTemp, Warning, TEXT("Bounce Validation FAIL: Jump-bounce would exceed air bounce limit (%d would become %d/%d)"), 
				ActualCurrentAirBounces, BouncesAfterThisOne, MaxAirBounces);
			return false;
		}
		UE_LOG(LogTemp, Log, TEXT("Bounce Validation PASS: Jump-bounce combo allowed"));
		return true;
	}

	// CRITICAL FIX: If not grounded, check if we would exceed air bounce limit AFTER this bounce
	if (!bIsCurrentlyGrounded)
	{
		const int32 BouncesAfterThisOne = ActualCurrentAirBounces + 1;
		if (BouncesAfterThisOne > MaxAirBounces)
		{
			UE_LOG(LogTemp, Warning, TEXT("Bounce Validation FAIL: Would exceed air bounce limit (%d would become %d/%d)"), 
				ActualCurrentAirBounces, BouncesAfterThisOne, MaxAirBounces);
			return false;
		}
	}

	// Check coyote time for ground-to-air transition
	if (!bIsCurrentlyGrounded && CoyoteTime > 0.0f)
	{
		const float TimeSinceGroundContact = GetWorld()->GetTimeSeconds() - LastGroundContactTime;
		UE_LOG(LogTemp, Log, TEXT("Bounce Validation: TimeSinceGroundContact=%.3f, CoyoteTime=%.3f"), 
			TimeSinceGroundContact, CoyoteTime);

		if (TimeSinceGroundContact > CoyoteTime)
		{
			// Outside coyote time, check air bounce availability for the NEXT bounce
			const int32 BouncesAfterThisOne = ActualCurrentAirBounces + 1;
			if (BouncesAfterThisOne > MaxAirBounces)
			{
				UE_LOG(LogTemp, Warning, TEXT("Bounce Validation FAIL: Outside coyote time and would exceed air bounce limit"));
				return false;
			}
		}
	}

	// Check if rising and bounce while rising is disabled (but allow during jump)
	const bool bIsRising = IsCharacterRising(InCharacter);
	if (!bAllowBounceWhileRising && bIsRising && !bIsJumping)
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
	
	// Apply curve modification if available - EPIC GAMES STANDARD: Use loaded curve assets
	float CurveMultiplier = 1.0f;
	if (IsValid(LoadedBounceVelocityCurve))
	{
		const float CurveInput = static_cast<float>(CurrentAirBounces) / FMath::Max(1.0f, static_cast<float>(MaxAirBounces));
		CurveMultiplier = LoadedBounceVelocityCurve->GetFloatValue(CurveInput);
	}

	BounceVelocity.Z = EffectiveVelocity * CurveMultiplier;
	return BounceVelocity;
}

float UGameplayAbility_Bounce::GetEffectiveBounceVelocity() const
{
	const int32 CurrentCount = GetCurrentAirBounceCount();
	if (CurrentCount == 0)
	{
		return BounceUpwardVelocity;
	}

	// Reduce velocity for air bounces
	return BounceUpwardVelocity * FMath::Pow(AirBounceVelocityReduction, static_cast<float>(CurrentCount));
}

// INDUSTRY BEST PRACTICE: Gameplay Attribute Management - Following Epic Games GAS patterns
int32 UGameplayAbility_Bounce::GetCurrentAirBounceCount() const
{
	const UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!IsValid(ASC))
	{
		UE_LOG(LogTemp, Warning, TEXT("GetCurrentAirBounceCount: Invalid ASC"));
		return 0;
	}

	// Get attribute set - const correctness following Epic standards
	const UMyAttributeSet* AttributeSet = ASC->GetSet<UMyAttributeSet>();
	if (!IsValid(AttributeSet))
	{
		UE_LOG(LogTemp, Warning, TEXT("GetCurrentAirBounceCount: Invalid AttributeSet"));
		return 0;
	}

	const float AttributeValue = AttributeSet->GetAirBounceCount();
	return FMath::RoundToInt32(AttributeValue);
}

void UGameplayAbility_Bounce::ResetAirBounceCount()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!IsValid(ASC))
	{
		UE_LOG(LogTemp, Warning, TEXT("ResetAirBounceCount: Invalid ASC"));
		return;
	}

	// EPIC GAMES STANDARD: Use Gameplay Effects instead of direct attribute manipulation
	if (IsValid(AirBounceResetEffect))
	{
		const FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
		const FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(AirBounceResetEffect, 1.0f, ContextHandle);
		if (SpecHandle.IsValid())
		{
			ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			if (bLogBounceEvents)
			{
				UE_LOG(LogTemp, Log, TEXT("Bounce: Air bounce count reset to 0 via Gameplay Effect"));
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("ResetAirBounceCount: Failed to create valid effect spec"));
		}
	}
	else
	{
		// FALLBACK: Direct attribute manipulation if no effect configured (not recommended)
		UE_LOG(LogTemp, Warning, TEXT("ResetAirBounceCount: No AirBounceResetEffect configured, using fallback"));
		const FGameplayAttribute AirBounceAttribute = UMyAttributeSet::GetAirBounceCountAttribute();
		ASC->ApplyModToAttribute(AirBounceAttribute, EGameplayModOp::Override, 0.0f);
	}
}

void UGameplayAbility_Bounce::IncrementAirBounceCount()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!IsValid(ASC))
	{
		UE_LOG(LogTemp, Warning, TEXT("IncrementAirBounceCount: Invalid ASC"));
		return;
	}

	// EPIC GAMES STANDARD: Use Gameplay Effects instead of direct attribute manipulation
	if (IsValid(AirBounceIncrementEffect))
	{
		const FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
		const FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(AirBounceIncrementEffect, 1.0f, ContextHandle);
		if (SpecHandle.IsValid())
		{
			ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			const int32 NewCount = GetCurrentAirBounceCount();
			if (bLogBounceEvents)
			{
				UE_LOG(LogTemp, Log, TEXT("Bounce: Air bounce count incremented to %d via Gameplay Effect"), NewCount);
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("IncrementAirBounceCount: Failed to create valid effect spec"));
		}
	}
	else
	{
		// FALLBACK: Direct attribute manipulation if no effect configured (not recommended)
		UE_LOG(LogTemp, Warning, TEXT("IncrementAirBounceCount: No AirBounceIncrementEffect configured, using fallback"));
		const FGameplayAttribute AirBounceAttribute = UMyAttributeSet::GetAirBounceCountAttribute();
		ASC->ApplyModToAttribute(AirBounceAttribute, EGameplayModOp::Additive, 1.0f);
		
		const int32 NewCount = GetCurrentAirBounceCount();
		if (bLogBounceEvents)
		{
			UE_LOG(LogTemp, Log, TEXT("Bounce: Air bounce count incremented to %d"), NewCount);
		}
	}
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
	UE_LOG(LogTemp, Warning, TEXT("DashMomentumMultiplier: %f"), DashMomentumMultiplier);
	UE_LOG(LogTemp, Warning, TEXT("JumpMomentumMultiplier: %f"), JumpMomentumMultiplier);
	UE_LOG(LogTemp, Warning, TEXT("bAllowComboBounce: %s"), bAllowComboBounce ? TEXT("true") : TEXT("false"));
	UE_LOG(LogTemp, Warning, TEXT("bAllowMomentumTransfer: %s"), bAllowMomentumTransfer ? TEXT("true") : TEXT("false"));
	UE_LOG(LogTemp, Warning, TEXT("==============================="));
}

// DASH-BOUNCE COMBO TESTING UTILITIES - Epic Games debugging standards
void UGameplayAbility_Bounce::TestDashBounceCombo()
{
	UE_LOG(LogTemp, Warning, TEXT("=== DASH-BOUNCE COMBO TEST ==="));
	
	const AMyCharacter* Character = CachedCharacter.Get();
	if (!IsValid(Character))
	{
		UE_LOG(LogTemp, Error, TEXT("TestDashBounceCombo: Invalid character reference"));
		return;
	}

	const UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent();
	const bool bIsDashing = ASC && ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.Dashing")));
	
	UE_LOG(LogTemp, Warning, TEXT("Character Dashing: %s"), bIsDashing ? TEXT("YES") : TEXT("NO"));
	UE_LOG(LogTemp, Warning, TEXT("Current Air Bounces: %d/%d"), GetCurrentAirBounceCount(), MaxAirBounces);
	
	if (bIsDashing)
	{
		UE_LOG(LogTemp, Warning, TEXT("DASH-BOUNCE COMBO AVAILABLE"));
		
		// Test momentum transfer
		FVelocitySnapshot MomentumSnapshot;
		if (TryGetMomentumContext(Character, MomentumSnapshot))
		{
			UE_LOG(LogTemp, Warning, TEXT("Momentum Source: %d"), static_cast<int32>(MomentumSnapshot.Source));
			UE_LOG(LogTemp, Warning, TEXT("Momentum Speed: %.1f"), MomentumSnapshot.Speed);
			UE_LOG(LogTemp, Warning, TEXT("Momentum Multiplier: %.2f"), GetMomentumMultiplier(MomentumSnapshot.Source));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("No momentum context available"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No active dash state - combo not available"));
	}
	
	UE_LOG(LogTemp, Warning, TEXT("================================"));
}

void UGameplayAbility_Bounce::TestJumpBounceCombo()
{
	UE_LOG(LogTemp, Warning, TEXT("=== JUMP-BOUNCE COMBO TEST ==="));
	
	const AMyCharacter* Character = CachedCharacter.Get();
	if (!IsValid(Character))
	{
		UE_LOG(LogTemp, Error, TEXT("TestJumpBounceCombo: Invalid character reference"));
		return;
	}

	const UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement();
	const bool bIsJumping = MovementComponent && MovementComponent->IsFalling() && MovementComponent->Velocity.Z > 0.0f;
	
	UE_LOG(LogTemp, Warning, TEXT("Character Jumping: %s"), bIsJumping ? TEXT("YES") : TEXT("NO"));
	UE_LOG(LogTemp, Warning, TEXT("Character Falling: %s"), MovementComponent && MovementComponent->IsFalling() ? TEXT("YES") : TEXT("NO"));
	UE_LOG(LogTemp, Warning, TEXT("Vertical Velocity: %.1f"), MovementComponent ? MovementComponent->Velocity.Z : 0.0f);
	UE_LOG(LogTemp, Warning, TEXT("Current Air Bounces: %d/%d"), GetCurrentAirBounceCount(), MaxAirBounces);
	
	if (bIsJumping)
	{
		UE_LOG(LogTemp, Warning, TEXT("JUMP-BOUNCE COMBO AVAILABLE"));
		
		// Test momentum transfer for jump
		FVelocitySnapshot MomentumSnapshot;
		if (TryGetMomentumContext(Character, MomentumSnapshot))
		{
			UE_LOG(LogTemp, Warning, TEXT("Momentum Source: %d"), static_cast<int32>(MomentumSnapshot.Source));
			UE_LOG(LogTemp, Warning, TEXT("Jump Momentum Multiplier: %.2f"), JumpMomentumMultiplier);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No active jump state - combo not available"));
	}
	
	UE_LOG(LogTemp, Warning, TEXT("==============================="));
}

void UGameplayAbility_Bounce::ValidateMomentumTransfer()
{
	UE_LOG(LogTemp, Warning, TEXT("=== MOMENTUM TRANSFER VALIDATION ==="));
	
	const AMyCharacter* Character = CachedCharacter.Get();
	if (!IsValid(Character))
	{
		UE_LOG(LogTemp, Error, TEXT("ValidateMomentumTransfer: Invalid character reference"));
		return;
	}

	// Test VelocitySnapshotComponent
	UVelocitySnapshotComponent* SnapshotComponent = Character->GetVelocitySnapshotComponent();
	if (!IsValid(SnapshotComponent))
	{
		UE_LOG(LogTemp, Error, TEXT("VelocitySnapshotComponent: NOT FOUND"));
		UE_LOG(LogTemp, Warning, TEXT("Momentum transfer will not work without VelocitySnapshotComponent"));
		return;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("VelocitySnapshotComponent: FOUND"));
	UE_LOG(LogTemp, Warning, TEXT("Has Valid Snapshots: %s"), SnapshotComponent->HasValidSnapshots() ? TEXT("YES") : TEXT("NO"));
	UE_LOG(LogTemp, Warning, TEXT("Valid Snapshot Count: %d"), SnapshotComponent->GetValidSnapshotCount());
	
	// Test latest snapshot
	FVelocitySnapshot LatestSnapshot;
	if (SnapshotComponent->GetLatestSnapshot(LatestSnapshot))
	{
		UE_LOG(LogTemp, Warning, TEXT("Latest Snapshot Source: %d"), static_cast<int32>(LatestSnapshot.Source));
		UE_LOG(LogTemp, Warning, TEXT("Latest Snapshot Speed: %.1f"), LatestSnapshot.Speed);
		UE_LOG(LogTemp, Warning, TEXT("Latest Snapshot Valid: %s"), LatestSnapshot.IsValid(GetWorld()->GetTimeSeconds()) ? TEXT("YES") : TEXT("NO"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No valid snapshots available"));
	}
	
	// Test momentum transfer settings
	UE_LOG(LogTemp, Warning, TEXT("Allow Combo Bounce: %s"), bAllowComboBounce ? TEXT("YES") : TEXT("NO"));
	UE_LOG(LogTemp, Warning, TEXT("Allow Momentum Transfer: %s"), bAllowMomentumTransfer ? TEXT("YES") : TEXT("NO"));
	UE_LOG(LogTemp, Warning, TEXT("Momentum Transfer Efficiency: %.2f"), MomentumTransferEfficiency);
	UE_LOG(LogTemp, Warning, TEXT("Preserve Momentum Direction: %s"), bPreserveMomentumDirection ? TEXT("YES") : TEXT("NO"));
	
	UE_LOG(LogTemp, Warning, TEXT("===================================="));
}

bool UGameplayAbility_Bounce::CanPerformAirBounce() const
{
	return CurrentAirBounces < MaxAirBounces;
}

FVector UGameplayAbility_Bounce::GetCurrentHorizontalVelocity() const
{
	const AMyCharacter* Character = CachedCharacter.Get();
	if (IsValid(Character))
	{
		const UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement();
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

// ENHANCED BOUNCE MOMENTUM TRANSFER METHODS - Dash-Bounce Combo Support

FVector UGameplayAbility_Bounce::CalculateEnhancedBounceVelocity(const AMyCharacter* InCharacter) const
{
	if (!IsValid(InCharacter))
	{
		return FVector::ZeroVector;
	}

	// PERFORMANCE OPTIMIZATION: Early exit if no snapshot component
	UVelocitySnapshotComponent* SnapshotComponent = InCharacter->GetVelocitySnapshotComponent();
	if (!IsValid(SnapshotComponent))
	{
		return FVector::ZeroVector;
	}

	// Try to get momentum context using optimized O(1) lookup
	FVelocitySnapshot MomentumSnapshot;
	if (!SnapshotComponent->GetLatestSnapshot(MomentumSnapshot))
	{
		return FVector::ZeroVector; // No momentum to transfer
	}

	// Calculate base bounce velocity
	const FVector BaseBounceVelocity = CalculateBounceVelocity(InCharacter);

	// Apply momentum transfer
	const FVector EnhancedVelocity = ApplyMomentumTransfer(BaseBounceVelocity, MomentumSnapshot);

	if (bLogBounceEvents)
	{
		UE_LOG(LogTemp, Log, TEXT("Enhanced Bounce: Source=%d, Multiplier=%.2f, Speed=%.1f -> %.1f"),
			static_cast<int32>(MomentumSnapshot.Source),
			GetMomentumMultiplier(MomentumSnapshot.Source),
			BaseBounceVelocity.Size(),
			EnhancedVelocity.Size());
	}

	return EnhancedVelocity;
}

bool UGameplayAbility_Bounce::TryGetMomentumContext(const AMyCharacter* InCharacter, FVelocitySnapshot& OutSnapshot) const
{
	if (!IsValid(InCharacter))
	{
		return false;
	}

	UVelocitySnapshotComponent* SnapshotComponent = InCharacter->GetVelocitySnapshotComponent();
	if (!IsValid(SnapshotComponent))
	{
		return false;
	}

	// PERFORMANCE OPTIMIZATION: Use O(1) latest snapshot lookup
	return SnapshotComponent->GetLatestSnapshot(OutSnapshot);
}

float UGameplayAbility_Bounce::GetMomentumMultiplier(EVelocitySource Source) const
{
	// PERFORMANCE OPTIMIZATION: Direct enum switch (compiler optimizes to jump table)
	switch (Source)
	{
		case EVelocitySource::Dash:
			return DashMomentumMultiplier;
		case EVelocitySource::Jump:
			return JumpMomentumMultiplier;
		case EVelocitySource::Fall:
			return FallMomentumMultiplier;
		default:
			return 1.0f;
	}
}

FVector UGameplayAbility_Bounce::ApplyMomentumTransfer(const FVector& BaseBounceVelocity, const FVelocitySnapshot& MomentumSnapshot) const
{
	const float Multiplier = GetMomentumMultiplier(MomentumSnapshot.Source);
	
	FVector EnhancedVelocity = BaseBounceVelocity;

	if (bPreserveMomentumDirection)
	{
		// PERFORMANCE OPTIMIZATION: Minimize vector operations
		const FVector HorizontalMomentum = FVector(MomentumSnapshot.Velocity.X, MomentumSnapshot.Velocity.Y, 0.0f);
		const float TransferFactor = Multiplier * MomentumTransferEfficiency;
		
		EnhancedVelocity.X = HorizontalMomentum.X * TransferFactor;
		EnhancedVelocity.Y = HorizontalMomentum.Y * TransferFactor;
	}
	else
	{
		// Apply momentum magnitude to existing direction
		const float HorizontalSpeed = FVector2D(BaseBounceVelocity.X, BaseBounceVelocity.Y).Size();
		const float EnhancedSpeed = HorizontalSpeed + (MomentumSnapshot.Speed * Multiplier * MomentumTransferEfficiency);
		
		const FVector2D Direction = FVector2D(BaseBounceVelocity.X, BaseBounceVelocity.Y).GetSafeNormal();
		EnhancedVelocity.X = Direction.X * EnhancedSpeed;
		EnhancedVelocity.Y = Direction.Y * EnhancedSpeed;
	}

	// Enhance vertical velocity for dash momentum
	if (MomentumSnapshot.Source == EVelocitySource::Dash)
	{
		EnhancedVelocity.Z *= (1.0f + (Multiplier - 1.0f) * 0.5f);
	}

	return EnhancedVelocity;
}

// EPIC GAMES STANDARD: Async curve loading following proper asset management patterns
void UGameplayAbility_Bounce::LoadCurveAssets()
{
	TArray<FSoftObjectPath> AssetsToLoad;

	// Collect assets that need loading - EPIC GAMES STANDARD: Only load what's needed
	if (!BounceVelocityCurve.IsNull() && !IsValid(LoadedBounceVelocityCurve))
	{
		AssetsToLoad.Add(BounceVelocityCurve.ToSoftObjectPath());
	}
	
	if (!AirControlCurve.IsNull() && !IsValid(LoadedAirControlCurve))
	{
		AssetsToLoad.Add(AirControlCurve.ToSoftObjectPath());
	}

	// Early exit if no assets to load
	if (AssetsToLoad.Num() == 0)
	{
		return;
	}

	// Load assets asynchronously - EPIC GAMES STANDARD: Use StreamableManager
	UAssetManager& AssetManager = UAssetManager::Get();
	CurveLoadHandle = AssetManager.LoadAssetList(AssetsToLoad, 
		FStreamableDelegate::CreateUObject(this, &UGameplayAbility_Bounce::OnCurveAssetsLoaded));

	if (CurveLoadHandle.IsValid())
	{
		UE_LOG(LogTemp, Log, TEXT("Bounce: Started loading %d curve assets"), AssetsToLoad.Num());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Bounce: Failed to start curve asset loading"));
	}
}

void UGameplayAbility_Bounce::OnCurveAssetsLoaded()
{
	// EPIC GAMES STANDARD: Safe asset loading with proper validation
	if (!BounceVelocityCurve.IsNull())
	{
		LoadedBounceVelocityCurve = BounceVelocityCurve.LoadSynchronous();
		if (IsValid(LoadedBounceVelocityCurve))
		{
			UE_LOG(LogTemp, Log, TEXT("Bounce: BounceVelocityCurve loaded successfully"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Bounce: Failed to load BounceVelocityCurve"));
		}
	}

	if (!AirControlCurve.IsNull())
	{
		LoadedAirControlCurve = AirControlCurve.LoadSynchronous();
		if (IsValid(LoadedAirControlCurve))
		{
			UE_LOG(LogTemp, Log, TEXT("Bounce: AirControlCurve loaded successfully"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Bounce: Failed to load AirControlCurve"));
		}
	}

	// Clean up handle after loading
	if (CurveLoadHandle.IsValid())
	{
		CurveLoadHandle.Reset();
	}
}
