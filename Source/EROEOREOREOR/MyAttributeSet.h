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
	virtual void OnRep_Health(const FGameplayAttributeData& OldValue);

	// Max Health Attribute
	UPROPERTY(BlueprintReadOnly, Category = "Health", EditAnywhere, ReplicatedUsing = OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UMyAttributeSet, MaxHealth)

	UFUNCTION()
	virtual void OnRep_MaxHealth(const FGameplayAttributeData& OldValue);

	// Stamina Attribute
	UPROPERTY(BlueprintReadOnly, Category = "Stamina", EditAnywhere, ReplicatedUsing = OnRep_Stamina)
	FGameplayAttributeData Stamina;
	ATTRIBUTE_ACCESSORS(UMyAttributeSet, Stamina)

	UFUNCTION()
	virtual void OnRep_Stamina(const FGameplayAttributeData& OldValue);

	// Max Stamina Attribute
	UPROPERTY(BlueprintReadOnly, Category = "Stamina", EditAnywhere, ReplicatedUsing = OnRep_MaxStamina)
	FGameplayAttributeData MaxStamina;
	ATTRIBUTE_ACCESSORS(UMyAttributeSet, MaxStamina)

	UFUNCTION()
	virtual void OnRep_MaxStamina(const FGameplayAttributeData& OldValue);

	// Movement Attributes - Following Epic Games GAS patterns for movement state tracking
	
	// Air Bounce Count - Tracks current number of air bounces performed
	UPROPERTY(BlueprintReadOnly, Category = "Movement", EditAnywhere, ReplicatedUsing = OnRep_AirBounceCount)
	FGameplayAttributeData AirBounceCount;
	ATTRIBUTE_ACCESSORS(UMyAttributeSet, AirBounceCount)

	UFUNCTION()
	virtual void OnRep_AirBounceCount(const FGameplayAttributeData& OldValue);

	// Combat Attributes for Damage System
	
	// Attack Power - Base damage multiplier
	UPROPERTY(BlueprintReadOnly, Category = "Combat", EditAnywhere, ReplicatedUsing = OnRep_AttackPower)
	FGameplayAttributeData AttackPower;
	ATTRIBUTE_ACCESSORS(UMyAttributeSet, AttackPower)

	UFUNCTION()
	virtual void OnRep_AttackPower(const FGameplayAttributeData& OldValue);

	// Critical Hit Chance (0.0 to 1.0)
	UPROPERTY(BlueprintReadOnly, Category = "Combat", EditAnywhere, ReplicatedUsing = OnRep_CriticalHitChance)
	FGameplayAttributeData CriticalHitChance;
	ATTRIBUTE_ACCESSORS(UMyAttributeSet, CriticalHitChance)

	UFUNCTION()
	virtual void OnRep_CriticalHitChance(const FGameplayAttributeData& OldValue);

	// Critical Hit Multiplier 
	UPROPERTY(BlueprintReadOnly, Category = "Combat", EditAnywhere, ReplicatedUsing = OnRep_CriticalHitMultiplier)
	FGameplayAttributeData CriticalHitMultiplier;
	ATTRIBUTE_ACCESSORS(UMyAttributeSet, CriticalHitMultiplier)

	UFUNCTION()
	virtual void OnRep_CriticalHitMultiplier(const FGameplayAttributeData& OldValue);

	// Attack Speed (attacks per second multiplier)
	UPROPERTY(BlueprintReadOnly, Category = "Combat", EditAnywhere, ReplicatedUsing = OnRep_AttackSpeed)
	FGameplayAttributeData AttackSpeed;
	ATTRIBUTE_ACCESSORS(UMyAttributeSet, AttackSpeed)

	UFUNCTION()
	virtual void OnRep_AttackSpeed(const FGameplayAttributeData& OldValue);

	// Physical Resistance (0.0 to 1.0, reduces physical damage)
	UPROPERTY(BlueprintReadOnly, Category = "Resistances", EditAnywhere, ReplicatedUsing = OnRep_PhysicalResistance)
	FGameplayAttributeData PhysicalResistance;
	ATTRIBUTE_ACCESSORS(UMyAttributeSet, PhysicalResistance)

	UFUNCTION()
	virtual void OnRep_PhysicalResistance(const FGameplayAttributeData& OldValue);

	// Elemental Resistance (0.0 to 1.0, reduces elemental damage)
	UPROPERTY(BlueprintReadOnly, Category = "Resistances", EditAnywhere, ReplicatedUsing = OnRep_ElementalResistance)
	FGameplayAttributeData ElementalResistance;
	ATTRIBUTE_ACCESSORS(UMyAttributeSet, ElementalResistance)

	UFUNCTION()
	virtual void OnRep_ElementalResistance(const FGameplayAttributeData& OldValue);

	// Meta Attributes - Used for damage calculations, not stored permanently
	UPROPERTY(BlueprintReadOnly, Category = "Meta Attributes")
	FGameplayAttributeData IncomingDamage;
	ATTRIBUTE_ACCESSORS(UMyAttributeSet, IncomingDamage)

	UPROPERTY(BlueprintReadOnly, Category = "Meta Attributes")
	FGameplayAttributeData IncomingHealing;
	ATTRIBUTE_ACCESSORS(UMyAttributeSet, IncomingHealing)

protected:
	// Helper function to clamp attribute values
	void AdjustAttributeForMaxChange(FGameplayAttributeData& AffectedAttribute, const FGameplayAttributeData& MaxAttribute, float NewMaxValue, const FGameplayAttribute& AffectedAttributeProperty);
};
