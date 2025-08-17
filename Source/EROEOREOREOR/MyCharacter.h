// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "MyAttributeSet.h"
#include "MyCharacter.generated.h"

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

protected:
	// GAS Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMyAttributeSet> AttributeSet;

};
