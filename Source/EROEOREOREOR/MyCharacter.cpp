// Fill out your copyright notice in the Description page of Project Settings.


#include "MyCharacter.h"
#include "AbilitySystemComponent.h"
#include "MyAttributeSet.h"
#include "GameplayAbility_Dash.h"
#include "GameplayAbility_Bounce.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Engine/World.h"

// Sets default values
AMyCharacter::AMyCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Set size for collision capsule
	GetCapsuleComponent()->SetCapsuleSize(42.f, 96.0f);

	// Set our turn rates for input
	BaseTurnRate = 65.f;
	BaseLookUpRate = 65.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement for camera-relative system
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character faces movement direction
	GetCharacterMovement()->bUseControllerDesiredRotation = false; // Don't rotate to controller
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // How fast character rotates to face movement

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Initialize GAS components
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AttributeSet = CreateDefaultSubobject<UMyAttributeSet>(TEXT("AttributeSet"));

	// Initialize movement input tracking
	CurrentMovementInput = FVector2D::ZeroVector;
}

// Called when the game starts or when spawned
void AMyCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	// Grant abilities to character
	if (AbilitySystemComponent && AbilitySystemComponent->GetAvatarActor() == this)
	{
		// Grant dash ability
		FGameplayAbilitySpec DashAbilitySpec(
			UGameplayAbility_Dash::StaticClass(),
			1, // Level
			INDEX_NONE, // InputID
			this // SourceObject
		);
		AbilitySystemComponent->GiveAbility(DashAbilitySpec);

		// Grant bounce ability
		FGameplayAbilitySpec BounceAbilitySpec(
			UGameplayAbility_Bounce::StaticClass(),
			1, // Level
			INDEX_NONE, // InputID
			this // SourceObject
		);
		AbilitySystemComponent->GiveAbility(BounceAbilitySpec);
	}
	
	// Setup Enhanced Input
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
		
		// Set input mode for game control
		FInputModeGameOnly InputModeData;
		PlayerController->SetInputMode(InputModeData);
		PlayerController->bShowMouseCursor = false;
	}
}

// Called every frame
void AMyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// Production-ready Tick - minimal processing only
}

void AMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{	
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (!PlayerInputComponent)
	{
		return;
	}

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EnhancedInputComponent)
	{
		return;
	}

	// Enhanced Input bindings
	if (JumpAction)
	{
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AMyCharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("JumpAction is not configured"));
	}
	
	if (LookAction)
	{
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMyCharacter::Look);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("LookAction is not configured"));
	}

	// Movement input bindings with proper error handling
	if (MoveForwardAction)
	{
		EnhancedInputComponent->BindAction(MoveForwardAction, ETriggerEvent::Triggered, this, &AMyCharacter::MoveForward);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("MoveForwardAction is not configured"));
	}
	
	if (MoveBackwardAction)
	{
		EnhancedInputComponent->BindAction(MoveBackwardAction, ETriggerEvent::Triggered, this, &AMyCharacter::MoveBackward);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("MoveBackwardAction is not configured"));
	}
	
	if (MoveLeftAction)
	{
		EnhancedInputComponent->BindAction(MoveLeftAction, ETriggerEvent::Triggered, this, &AMyCharacter::MoveLeft);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("MoveLeftAction is not configured"));
	}
	
	if (MoveRightAction)
	{
		EnhancedInputComponent->BindAction(MoveRightAction, ETriggerEvent::Triggered, this, &AMyCharacter::MoveRight);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("MoveRightAction is not configured"));
	}

	// Shift key bindings
	if (ShiftAction)
	{
		EnhancedInputComponent->BindAction(ShiftAction, ETriggerEvent::Started, this, &AMyCharacter::ShiftPressed);
		EnhancedInputComponent->BindAction(ShiftAction, ETriggerEvent::Completed, this, &AMyCharacter::ShiftReleased);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ShiftAction is not configured"));
	}

	// Dash bindings with immediate response
	if (DashLeftAction)
	{
		EnhancedInputComponent->BindAction(DashLeftAction, ETriggerEvent::Started, this, &AMyCharacter::DashLeft);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("DashLeftAction is not configured"));
	}
	
	if (DashRightAction)
	{
		EnhancedInputComponent->BindAction(DashRightAction, ETriggerEvent::Started, this, &AMyCharacter::DashRight);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("DashRightAction is not configured"));
	}

	// Bounce binding
	if (BounceAction)
	{
		EnhancedInputComponent->BindAction(BounceAction, ETriggerEvent::Started, this, &AMyCharacter::Bounce);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("BounceAction is not configured"));
	}

}

UAbilitySystemComponent* AMyCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UMyAttributeSet* AMyCharacter::GetMyAttributeSet() const
{
	return Cast<UMyAttributeSet>(AttributeSet);
}

void AMyCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	// Initialize the Ability System for the Server
	if (AbilitySystemComponent && AttributeSet)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
		
		// Ensure the AttributeSet is properly registered with the ASC
		// The AttributeSet should be automatically discovered, but we can force it
		AbilitySystemComponent->GetSet<UMyAttributeSet>();
		
		// CRITICAL: Grant abilities during possession (server-side)
		UE_LOG(LogTemp, Error, TEXT("POSSESSED BY - GRANTING ABILITIES"));
		
		// Grant dash ability
		FGameplayAbilitySpec DashAbilitySpec(
			UGameplayAbility_Dash::StaticClass(),
			1, // Level
			INDEX_NONE, // InputID
			this // SourceObject
		);
		
		FGameplayAbilitySpecHandle DashHandle = AbilitySystemComponent->GiveAbility(DashAbilitySpec);
		
		if (DashHandle.IsValid())
		{
			UE_LOG(LogTemp, Error, TEXT("DASH ABILITY GRANTED IN POSSESSED BY"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED TO GRANT DASH ABILITY IN POSSESSED BY"));
		}

		// Grant bounce ability
		FGameplayAbilitySpec BounceAbilitySpec(
			UGameplayAbility_Bounce::StaticClass(),
			1, // Level
			INDEX_NONE, // InputID
			this // SourceObject
		);
		
		FGameplayAbilitySpecHandle BounceHandle = AbilitySystemComponent->GiveAbility(BounceAbilitySpec);
		
		if (BounceHandle.IsValid())
		{
			UE_LOG(LogTemp, Error, TEXT("BOUNCE ABILITY GRANTED IN POSSESSED BY"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED TO GRANT BOUNCE ABILITY IN POSSESSED BY"));
		}
	}
}

void AMyCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	
	// Initialize the Ability System for the Client
	if (AbilitySystemComponent && AttributeSet)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
		
		// Ensure the AttributeSet is properly registered with the ASC
		// The AttributeSet should be automatically discovered, but we can force it
		AbilitySystemComponent->GetSet<UMyAttributeSet>();
	}
}


void AMyCharacter::Look(const FInputActionValue& Value)
{
	// Input is a Vector2D
	const FVector2D LookAxisVector = Value.Get<FVector2D>();
	if (Controller != nullptr)

		// Use the improved input method from your Aura project
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
}

void AMyCharacter::Jump()
{
	// Call the base Character jump
	Super::Jump();
}

// Camera-Relative Movement Functions
void AMyCharacter::MoveForward(const FInputActionValue& Value)
{
	// CRITICAL FIX: Block movement input during abilities
	if (AbilitySystemComponent && 
		(AbilitySystemComponent->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.Dashing"))) ||
		 AbilitySystemComponent->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.Bouncing")))))
	{
		UE_LOG(LogTemp, Warning, TEXT("MoveForward blocked - currently using ability"));
		return; // Don't process movement input during abilities
	}
	
	const float InputValue = Value.Get<float>();
	
	// Update movement input tracking (Y = forward/backward)
	CurrentMovementInput.Y = InputValue;
	
	if (FollowCamera != nullptr && FMath::Abs(InputValue) > 0.0f)
	{
		// Get camera forward direction (projected to ground)
		FVector CameraForward = FollowCamera->GetForwardVector();
		CameraForward.Z = 0.0f; // Remove vertical component
		CameraForward.Normalize();
		
		AddMovementInput(CameraForward, InputValue);
	}
}

void AMyCharacter::MoveBackward(const FInputActionValue& Value)
{
	// CRITICAL FIX: Block movement input during abilities
	if (AbilitySystemComponent && 
		(AbilitySystemComponent->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.Dashing"))) ||
		 AbilitySystemComponent->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.Bouncing")))))
	{
		UE_LOG(LogTemp, Warning, TEXT("MoveBackward blocked - currently using ability"));
		return; // Don't process movement input during abilities
	}
	
	const float InputValue = Value.Get<float>();
	
	// Update movement input tracking (Y = forward/backward, negative for backward)
	CurrentMovementInput.Y = -InputValue;
	
	if (FollowCamera != nullptr && FMath::Abs(InputValue) > 0.0f)
	{
		// Get camera forward direction (projected to ground)
		FVector CameraForward = FollowCamera->GetForwardVector();
		CameraForward.Z = 0.0f; // Remove vertical component
		CameraForward.Normalize();
		
		// Move backward (negative forward)
		AddMovementInput(CameraForward, -InputValue);
	}
}

void AMyCharacter::MoveLeft(const FInputActionValue& Value)
{
	// CRITICAL FIX: Block movement input during abilities
	if (AbilitySystemComponent && 
		(AbilitySystemComponent->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.Dashing"))) ||
		 AbilitySystemComponent->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.Bouncing")))))
	{
		UE_LOG(LogTemp, Warning, TEXT("MoveLeft blocked - currently using ability"));
		return; // Don't process movement input during abilities
	}
	
	const float InputValue = Value.Get<float>();
	
	// Update movement input tracking (X = left/right, negative for left)
	CurrentMovementInput.X = -InputValue;
	
	if (FollowCamera != nullptr && FMath::Abs(InputValue) > 0.0f)
	{
		// Get camera right direction (projected to ground)
		FVector CameraRight = FollowCamera->GetRightVector();
		CameraRight.Z = 0.0f; // Remove vertical component
		CameraRight.Normalize();
		
		// Move left (negative right - strafe left relative to camera)
		AddMovementInput(CameraRight, -InputValue);
	}
}

void AMyCharacter::MoveRight(const FInputActionValue& Value)
{
	// CRITICAL FIX: Block movement input during abilities
	if (AbilitySystemComponent && 
		(AbilitySystemComponent->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.Dashing"))) ||
		 AbilitySystemComponent->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.Bouncing")))))
	{
		UE_LOG(LogTemp, Warning, TEXT("MoveRight blocked - currently using ability"));
		return; // Don't process movement input during abilities
	}
	
	const float InputValue = Value.Get<float>();
	
	// Update movement input tracking (X = left/right, positive for right)
	CurrentMovementInput.X = InputValue;
	
	if (FollowCamera != nullptr && FMath::Abs(InputValue) > 0.0f)
	{
		// Get camera right direction (projected to ground)
		FVector CameraRight = FollowCamera->GetRightVector();
		CameraRight.Z = 0.0f; // Remove vertical component
		CameraRight.Normalize();
		
		// Move right (positive right - strafe right relative to camera)
		AddMovementInput(CameraRight, InputValue);
	}
}

void AMyCharacter::ShiftPressed(const FInputActionValue& Value)
{
	bIsShiftPressed = true;
}

void AMyCharacter::ShiftReleased(const FInputActionValue& Value)
{
	bIsShiftPressed = false;
}

void AMyCharacter::DashLeft(const FInputActionValue& Value)
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	// PERFORMANCE: Use cached handle to avoid lookup delays
	if (!CachedDashAbilityHandle.IsValid())
	{
		// Cache the handle on first use
		for (FGameplayAbilitySpec& Spec : AbilitySystemComponent->GetActivatableAbilities())
		{
			if (Spec.Ability && Spec.Ability->IsA<UGameplayAbility_Dash>())
			{
				CachedDashAbilityHandle = Spec.Handle;
				break;
			}
		}
	}

	if (!CachedDashAbilityHandle.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("DashLeft: No dash ability found to cache"));
		return;
	}

	// Epic Games standard: Use gameplay tags to communicate direction
	FGameplayTagContainer DirectionTags;
	DirectionTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Input.Dash.Left")));
	
	// Apply direction tag temporarily
	AbilitySystemComponent->AddLooseGameplayTags(DirectionTags);

	// IMMEDIATE ACTIVATION: Use cached handle for instant response
	bool bActivated = AbilitySystemComponent->TryActivateAbility(CachedDashAbilityHandle);
	
	// Remove direction tag after activation attempt
	AbilitySystemComponent->RemoveLooseGameplayTags(DirectionTags);
	
	if (!bActivated)
	{
		UE_LOG(LogTemp, Warning, TEXT("DashLeft: Failed to activate cached dash ability"));
	}
}

// Debug functions for testing - minimal implementations
void AMyCharacter::Move(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Log, TEXT("Move debug function called"));
}

void AMyCharacter::Dash(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Log, TEXT("Dash debug function called"));
}

void AMyCharacter::TestKey()
{
	UE_LOG(LogTemp, Log, TEXT("TestKey debug function called"));
}

void AMyCharacter::TestDash()
{
	UE_LOG(LogTemp, Log, TEXT("TestDash debug function called"));
}

void AMyCharacter::TestBounce()
{
	UE_LOG(LogTemp, Log, TEXT("TestBounce debug function called"));
}

// Production-ready dash methods - clean GAS implementation

void AMyCharacter::DashRight(const FInputActionValue& Value)
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	// PERFORMANCE: Use cached handle to avoid lookup delays
	if (!CachedDashAbilityHandle.IsValid())
	{
		// Cache the handle on first use
		for (FGameplayAbilitySpec& Spec : AbilitySystemComponent->GetActivatableAbilities())
		{
			if (Spec.Ability && Spec.Ability->IsA<UGameplayAbility_Dash>())
			{
				CachedDashAbilityHandle = Spec.Handle;
				break;
			}
		}
	}

	if (!CachedDashAbilityHandle.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("DashRight: No dash ability found to cache"));
		return;
	}

	// Epic Games standard: Use gameplay tags to communicate direction
	FGameplayTagContainer DirectionTags;
	DirectionTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Input.Dash.Right")));
	
	// Apply direction tag temporarily
	AbilitySystemComponent->AddLooseGameplayTags(DirectionTags);

	// IMMEDIATE ACTIVATION: Use cached handle for instant response
	bool bActivated = AbilitySystemComponent->TryActivateAbility(CachedDashAbilityHandle);
	
	// Remove direction tag after activation attempt
	AbilitySystemComponent->RemoveLooseGameplayTags(DirectionTags);
	
	if (!bActivated)
	{
		UE_LOG(LogTemp, Warning, TEXT("DashRight: Failed to activate cached dash ability"));
	}
}

void AMyCharacter::Bounce(const FInputActionValue& Value)
{
	if (!AbilitySystemComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("Bounce: AbilitySystemComponent is null"));
		return;
	}

	// ENHANCED DEBUGGING: Check ability system state
	UE_LOG(LogTemp, Log, TEXT("Bounce: Attempting bounce activation. ASC valid: %s"), 
		AbilitySystemComponent ? TEXT("true") : TEXT("false"));

	// PERFORMANCE: Use cached handle to avoid lookup delays
	if (!CachedBounceAbilityHandle.IsValid())
	{
		UE_LOG(LogTemp, Log, TEXT("Bounce: Caching bounce ability handle"));
		
		// Cache the handle on first use
		int32 BounceAbilityCount = 0;
		for (FGameplayAbilitySpec& Spec : AbilitySystemComponent->GetActivatableAbilities())
		{
			if (Spec.Ability && Spec.Ability->IsA<UGameplayAbility_Bounce>())
			{
				CachedBounceAbilityHandle = Spec.Handle;
				BounceAbilityCount++;
				UE_LOG(LogTemp, Log, TEXT("Bounce: Found and cached bounce ability handle"));
				break;
			}
		}
		
		if (BounceAbilityCount == 0)
		{
			UE_LOG(LogTemp, Error, TEXT("Bounce: No UGameplayAbility_Bounce found in activatable abilities"));
			
			// DEBUG: List all available abilities
			for (const FGameplayAbilitySpec& Spec : AbilitySystemComponent->GetActivatableAbilities())
			{
				if (Spec.Ability)
				{
					UE_LOG(LogTemp, Log, TEXT("Available Ability: %s"), *Spec.Ability->GetName());
				}
			}
		}
	}

	if (!CachedBounceAbilityHandle.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Bounce: CachedBounceAbilityHandle is invalid - ability not granted"));
		return;
	}

	// Check if ability system is initialized
	if (!AbilitySystemComponent->AbilityActorInfo.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Bounce: AbilityActorInfo is not valid - ASC not properly initialized"));
		return;
	}

	// Epic Games standard: Use gameplay tags to communicate input
	FGameplayTagContainer InputTags;
	InputTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Input.Bounce")));
	
	// Apply input tag temporarily
	AbilitySystemComponent->AddLooseGameplayTags(InputTags);

	// IMMEDIATE ACTIVATION: Use cached handle for instant response
	const bool bActivated = AbilitySystemComponent->TryActivateAbility(CachedBounceAbilityHandle);
	
	// Remove input tag after activation attempt
	AbilitySystemComponent->RemoveLooseGameplayTags(InputTags);
	
	UE_LOG(LogTemp, Log, TEXT("Bounce: Activation result: %s"), bActivated ? TEXT("SUCCESS") : TEXT("FAILED"));
	
	if (!bActivated)
	{
		UE_LOG(LogTemp, Warning, TEXT("Bounce: TryActivateAbility failed despite CanActivateAbility returning true"));
	}
}
