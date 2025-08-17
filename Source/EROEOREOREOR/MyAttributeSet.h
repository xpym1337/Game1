#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectExtension.h"
#include "MyAttributeSet.generated.h"

// Uses macros from AttributeSet.h
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * Custom Attribute Set for the GAS system containing Health, Stamina, and their Max values
 * This class defines gameplay attributes that can be modified by gameplay effects
 */
UCLASS()
class EROEOREOREOR_API UMyAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UMyAttributeSet();

	// AttributeSet Overrides
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	// Health Attribute
	UPROPERTY(BlueprintReadOnly, Category = "Health", EditAnywhere, ReplicatedUsing = OnRep_Health)
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UMyAttributeSet, Health)

	UFUNCTION()
	virtual void OnRep_Health(const FGameplayAttributeData& OldHealth);

	// Max Health Attribute
	UPROPERTY(BlueprintReadOnly, Category = "Health", EditAnywhere, ReplicatedUsing = OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UMyAttributeSet, MaxHealth)

	UFUNCTION()
	virtual void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth);

	// Stamina Attribute
	UPROPERTY(BlueprintReadOnly, Category = "Stamina", EditAnywhere, ReplicatedUsing = OnRep_Stamina)
	FGameplayAttributeData Stamina;
	ATTRIBUTE_ACCESSORS(UMyAttributeSet, Stamina)

	UFUNCTION()
	virtual void OnRep_Stamina(const FGameplayAttributeData& OldStamina);

	// Max Stamina Attribute
	UPROPERTY(BlueprintReadOnly, Category = "Stamina", EditAnywhere, ReplicatedUsing = OnRep_MaxStamina)
	FGameplayAttributeData MaxStamina;
	ATTRIBUTE_ACCESSORS(UMyAttributeSet, MaxStamina)

	UFUNCTION()
	virtual void OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina);

protected:
	// Helper function to clamp attribute values
	void AdjustAttributeForMaxChange(FGameplayAttributeData& AffectedAttribute, const FGameplayAttributeData& MaxAttribute, float NewMaxValue, const FGameplayAttribute& AffectedAttributeProperty);
};
