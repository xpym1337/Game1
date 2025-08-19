// Fill out your copyright notice in the Description page of Project Settings.


#include "MyCharacter.h"
#include "AbilitySystemComponent.h"
#include "MyAttributeSet.h"
#include "GameplayAbility_Dash.h"
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

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

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
}

// Called when the game starts or when spawned
void AMyCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
		
		// Set input mode to Game Only for immediate control
		FInputModeGameOnly InputModeData;
		PlayerController->SetInputMode(InputModeData);
		PlayerController->bShowMouseCursor = false;
	}
}

// Called every frame
void AMyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
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
	
	if (LookAction)
	{
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMyCharacter::Look);
	}

	// Individual WASD movement bindings
	if (MoveForwardAction)
	{
		EnhancedInputComponent->BindAction(MoveForwardAction, ETriggerEvent::Triggered, this, &AMyCharacter::MoveForward);
	}
	if (MoveBackwardAction)
	{
		EnhancedInputComponent->BindAction(MoveBackwardAction, ETriggerEvent::Triggered, this, &AMyCharacter::MoveBackward);
	}
	if (MoveLeftAction)
	{
		EnhancedInputComponent->BindAction(MoveLeftAction, ETriggerEvent::Triggered, this, &AMyCharacter::MoveLeft);
	}
	if (MoveRightAction)
	{
		EnhancedInputComponent->BindAction(MoveRightAction, ETriggerEvent::Triggered, this, &AMyCharacter::MoveRight);
	}

	// Shift key bindings
	if (ShiftAction)
	{
		EnhancedInputComponent->BindAction(ShiftAction, ETriggerEvent::Started, this, &AMyCharacter::ShiftPressed);
		EnhancedInputComponent->BindAction(ShiftAction, ETriggerEvent::Completed, this, &AMyCharacter::ShiftReleased);
	}

	// Dash bindings
	if (DashLeftAction)
		EnhancedInputComponent->BindAction(DashLeftAction, ETriggerEvent::Completed, this, &AMyCharacter::DashLeft);
	if (DashRightAction)
		EnhancedInputComponent->BindAction(DashRightAction, ETriggerEvent::Completed, this, &AMyCharacter::DashRight);

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

// Individual Movement Functions
void AMyCharacter::MoveForward(const FInputActionValue& Value)
{
	const float InputValue = Value.Get<float>();
	
	if (Controller != nullptr && FMath::Abs(InputValue) > 0.0f)
	{
		// Find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		
		AddMovementInput(ForwardDirection, InputValue);
	}
}

void AMyCharacter::MoveBackward(const FInputActionValue& Value)
{
	const float InputValue = Value.Get<float>();
	
	if (Controller != nullptr && FMath::Abs(InputValue) > 0.0f)
	{
		// Find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		
		// Move backward (negative forward)
		AddMovementInput(ForwardDirection, -InputValue);
	}
}

void AMyCharacter::MoveLeft(const FInputActionValue& Value)
{
	const float InputValue = Value.Get<float>();
	
	if (Controller != nullptr && FMath::Abs(InputValue) > 0.0f)
	{
		// Find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		
		// Move left (negative right)
		AddMovementInput(RightDirection, -InputValue);
	}
}

void AMyCharacter::MoveRight(const FInputActionValue& Value)
{
	const float InputValue = Value.Get<float>();
	
	if (Controller != nullptr && FMath::Abs(InputValue) > 0.0f)
	{
		// Find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		
		AddMovementInput(RightDirection, InputValue);
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

// Camera-Relative Dash Functions using GAS
void AMyCharacter::DashLeft(const FInputActionValue& Value)
{
	if (!AbilitySystemComponent || !Controller)
	{
		return;
	}

	// Try to activate dash ability with left direction
	FGameplayTagContainer AbilityTags;
	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Dash")));
	
	if (!AbilitySystemComponent->TryActivateAbilitiesByTag(AbilityTags))
	{
		// Fallback to direct movement if GAS not working
		
		// Get camera's right vector for lateral movement
		FVector CameraRightVector = FollowCamera->GetRightVector();
		CameraRightVector.Z = 0.0f; // Keep movement horizontal
		CameraRightVector.Normalize();

		// Dash Left: Move in negative camera right direction
		FVector DashDirection = -CameraRightVector;
		
		// Apply dash impulse
		FVector DashVelocity = DashDirection * DashDistance;
		GetCharacterMovement()->Launch(DashVelocity);
	}
}

void AMyCharacter::DashRight(const FInputActionValue& Value)
{
	if (!AbilitySystemComponent || !Controller)
	{
		return;
	}

	// Try to activate dash ability with right direction
	FGameplayTagContainer AbilityTags;
	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Dash")));
	
	if (!AbilitySystemComponent->TryActivateAbilitiesByTag(AbilityTags))
	{
		
		// Get camera's right vector for lateral movement
		FVector CameraRightVector = FollowCamera->GetRightVector();
		CameraRightVector.Z = 0.0f; // Keep movement horizontal
		CameraRightVector.Normalize();

		// Dash Right: Move in positive camera right direction
		FVector DashDirection = CameraRightVector;
		
		// Apply dash impulse
		FVector DashVelocity = DashDirection * DashDistance;
		GetCharacterMovement()->Launch(DashVelocity);
	}
}
