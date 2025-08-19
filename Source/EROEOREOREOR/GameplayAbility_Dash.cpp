// Fill out your copyright notice in the Description page of Project Settings.

#include "GameplayAbility_Dash.h"
#include "MyCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "Engine/World.h"
#include "GameplayTagsManager.h"

UGameplayAbility_Dash::UGameplayAbility_Dash()
{
	// Set ability defaults
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	
	// Set default tags (these will be set properly in Blueprint/C++)
	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Dash")));
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Dashing")));
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Dashing")));
	
	// Store tag references
	DashingStateTag = FGameplayTag::RequestGameplayTag(FName("State.Dashing"));
	DashCooldownTag = FGameplayTag::RequestGameplayTag(FName("Effect.Cooldown"));
}

void UGameplayAbility_Dash::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Get the character
	AMyCharacter* Character = Cast<AMyCharacter>(ActorInfo->AvatarActor.Get());
	if (!Character)
	{
		UE_LOG(LogTemp, Warning, TEXT("UGameplayAbility_Dash: Failed to get character"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Execute the dash
	ExecuteDash(Character, DashDirection);

	// Set timer to end ability
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(DashTimer, this, &UGameplayAbility_Dash::OnDashComplete, DashDuration, false);
	}
}

void UGameplayAbility_Dash::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// Clear timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DashTimer);
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGameplayAbility_Dash::ExecuteDash(AMyCharacter* Character, EDashDirection Direction)
{
	UCameraComponent* FollowCamera = Character ? Character->GetFollowCamera() : nullptr;
	if (!Character || !FollowCamera)
	{
		UE_LOG(LogTemp, Warning, TEXT("UGameplayAbility_Dash: Invalid character or camera"));
		return;
	}

	// Get camera's right vector for lateral movement
	FVector CameraRightVector = FollowCamera->GetRightVector();
	CameraRightVector.Z = 0.0f; // Keep movement horizontal
	CameraRightVector.Normalize();

	FVector DashDirectionVector;
	FString DirectionString;

	switch (Direction)
	{
	case EDashDirection::Left:
		DashDirectionVector = -CameraRightVector; // Move in negative camera right direction
		DirectionString = TEXT("Left");
		break;
		
	case EDashDirection::Right:
		DashDirectionVector = CameraRightVector; // Move in positive camera right direction
		DirectionString = TEXT("Right");
		break;
		
	default:
		UE_LOG(LogTemp, Warning, TEXT("UGameplayAbility_Dash: Invalid dash direction"));
		return;
	}

	// Apply dash impulse
	FVector DashVelocity = DashDirectionVector * DashDistance;
	Character->GetCharacterMovement()->Launch(DashVelocity);

	UE_LOG(LogTemp, Warning, TEXT("GAS Dash %s executed! Direction: %s"), *DirectionString, *DashDirectionVector.ToString());
}

void UGameplayAbility_Dash::OnDashComplete()
{
	// End the ability
	if (IsActive())
	{
		K2_EndAbility();
	}
}
