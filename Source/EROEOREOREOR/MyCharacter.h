// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "MyAttributeSet.h"
#include "InputActionValue.h"
#include "GameplayAbility_Dash.h"
#include "MyCharacter.generated.h"

class UInputMappingContext;
class UInputAction;
class USpringArmComponent;
class UCameraComponent;

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
	
	// Debugging functions
	void Move(const FInputActionValue& Value);
	void Dash(const FInputActionValue& Value);
	void TestKey();
	void TestDash();

	// Camera turn rate properties for smoother input
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

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

	// Dash System Properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash", meta = (AllowPrivateAccess = "true"))
	float DashDistance = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash", meta = (AllowPrivateAccess = "true"))
	float DashDuration = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash", meta = (AllowPrivateAccess = "true"))
	float DashCooldown = 2.0f;

private:
	// Dash state tracking
	bool bIsShiftPressed = false;
	bool bCanDash = true;
	FTimerHandle DashCooldownTimer;

	// Movement input tracking for dash system
	FVector2D CurrentMovementInput;

	// PERFORMANCE: Cache dash ability handle to avoid lookup delays
	UPROPERTY(Transient)
	FGameplayAbilitySpecHandle CachedDashAbilityHandle;

};
