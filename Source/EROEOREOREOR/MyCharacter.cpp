// Fill out your copyright notice in the Description page of Project Settings.


#include "MyCharacter.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"

// Sets default values
AMyCharacter::AMyCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Initialize GAS components
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AttributeSet = CreateDefaultSubobject<UAttributeSet>(TEXT("AttributeSet"));
}

// Called when the game starts or when spawned
void AMyCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}


UAbilitySystemComponent* AMyCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AMyCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	// Initialize the Ability System for the Server
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}
}

void AMyCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	
	// Initialize the Ability System for the Client
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}
}
