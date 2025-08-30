// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "MyAttributeSet.h"
#include "InputActionValue.h"
#include "GameplayAbility_Dash.h"
#include "VelocitySnapshotComponent.h"
#include "CombatStateMachineComponent.h"
#include "GameplayEffect_Damage.h"
#include "AttackShapeComponent.h"
#include "MyCharacter.generated.h"

class UInputMappingContext;
class UInputAction;
class USpringArmComponent;
class UCameraComponent;
class UDamageApplicationComponent;

UCLASS()
class EROEOREOREOR_API AMyCharacter : public ACharacter , public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMyCharacter();

    // IAbilitySystemInterface
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

    // Getter for the custom AttributeSet
    UFUNCTION(BlueprintCallable, Category = "GAS")
    UMyAttributeSet* GetMyAttributeSet() const;

    // Getter for the Follow Camera
    UFUNCTION(BlueprintCallable, Category = "Camera")
    UCameraComponent* GetFollowCamera() const { return FollowCamera; }

    // Getter for current movement input (for dash system)
    UFUNCTION(BlueprintCallable, Category = "Movement")
    FVector2D GetCurrentMovementInput() const { return CurrentMovementInput; }

    // Getter for velocity snapshot component (for dash-bounce combo system)
    UFUNCTION(BlueprintPure, Category = "Movement")
    UVelocitySnapshotComponent* GetVelocitySnapshotComponent() const { return VelocitySnapshotComponent; }

	// Override Landed to broadcast delegate (uses built-in ACharacter::LandedDelegate)
	virtual void Landed(const FHitResult& Hit) override;

protected:
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

    // GAS Lifecycle Hooks - NEW
    virtual void PossessedBy(AController* NewController) override;
    virtual void OnRep_PlayerState() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Enhanced Input Action Functions


	void Look(const FInputActionValue& Value);
	void Jump();
	
	// Individual Movement Action Functions
	void MoveForward(const FInputActionValue& Value);
	void MoveBackward(const FInputActionValue& Value);
	void MoveLeft(const FInputActionValue& Value);
	void MoveRight(const FInputActionValue& Value);
	void ShiftPressed(const FInputActionValue& Value);
	void ShiftReleased(const FInputActionValue& Value);
	
	// Dash Action Functions
	void DashLeft(const FInputActionValue& Value);
	void DashRight(const FInputActionValue& Value);
	
	// Bounce Action Functions
	void Bounce(const FInputActionValue& Value);
	
	// Combat Action Functions
	void LightAttack(const FInputActionValue& Value);
	void HeavyAttack(const FInputActionValue& Value);
	
	// Debugging functions
	void Move(const FInputActionValue& Value);
	void Dash(const FInputActionValue& Value);
	void TestKey();
	void TestDash();
	void TestBounce();
	void TestCombatSystem();

	// Camera turn rate properties for smoother input
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	// Advanced Dash System Properties - PUBLIC for GameplayAbility access
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash|Core", meta = (ClampMin = "100.0", ClampMax = "5000.0"))
	float DashSpeed = 1875.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash|Core", meta = (ClampMin = "0.1", ClampMax = "3.0"))
	float DashDuration = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash|Core", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MomentumRetention = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash|Feel", meta = (ClampMin = "0.0", ClampMax = "5000.0"))
	float DashInitialBurstSpeed = 2500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash|Feel", meta = (ClampMin = "10.0", ClampMax = "120.0"))
	float UpdateFrequency = 60.0f;

	// Advanced Bounce System Properties - PUBLIC for GameplayAbility access
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounce|Core", meta = (ClampMin = "200.0", ClampMax = "2000.0"))
	float BounceUpwardVelocity = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounce|Core", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HorizontalVelocityRetention = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounce|Core", meta = (ClampMin = "0", ClampMax = "5"))
	int32 MaxAirBounces = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounce|Feel", meta = (ClampMin = "0.0", ClampMax = "0.5"))
	float BounceInputWindow = 0.1f;

protected:
	// Enhanced Input
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enhanced Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enhanced Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enhanced Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> LookAction;

	// Individual Movement Input Actions
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enhanced Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MoveForwardAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enhanced Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MoveBackwardAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enhanced Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MoveLeftAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enhanced Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MoveRightAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enhanced Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> ShiftAction;

	// Dash Input Actions
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enhanced Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> DashLeftAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enhanced Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> DashRightAction;

	// Bounce Input Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enhanced Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> BounceAction;

	// Camera Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FollowCamera;

	// GAS Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMyAttributeSet> AttributeSet;

	// Combat System Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCombatStateMachineComponent> CombatStateMachine;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UDamageApplicationComponent> DamageApplicationComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAttackShapeComponent> AttackShapeComponent;

	// Dash-Bounce Combo System Component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UVelocitySnapshotComponent> VelocitySnapshotComponent;

	// Legacy properties (keeping for compatibility)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash|Legacy", meta = (AllowPrivateAccess = "true"))
	float DashDistance = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash|Legacy", meta = (AllowPrivateAccess = "true"))
	float DashCooldown = 2.0f;

private:
	// Dash state tracking
	bool bIsShiftPressed = false;
	bool bCanDash = true;
	FTimerHandle DashCooldownTimer;

	// Movement input tracking for dash system
	FVector2D CurrentMovementInput;

	// PERFORMANCE: Cache ability handles to avoid lookup delays
	UPROPERTY(Transient)
	FGameplayAbilitySpecHandle CachedDashAbilityHandle;

	UPROPERTY(Transient)
	FGameplayAbilitySpecHandle CachedBounceAbilityHandle;

};
