#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "CombatSystemTypes.h"
#include "GameplayEffect_Damage.generated.h"

/**
 * Flexible damage GameplayEffect that can be parameterized at runtime
 * Supports different damage types, critical hits, resistances, and status effects
 */
UCLASS(BlueprintType, Blueprintable)
class EROEOREOREOR_API UGameplayEffect_Damage : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UGameplayEffect_Damage();
};

/**
 * Execution calculation for damage that handles all combat math
 * Includes: Base damage, attack power, crits, resistances, damage types
 */
UCLASS(BlueprintType, Blueprintable)
class EROEOREOREOR_API UDamageExecutionCalculation : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	UDamageExecutionCalculation();

	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, OUT FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;

protected:
	// Damage calculation helper functions
	float CalculateBaseDamage(const FGameplayEffectCustomExecutionParameters& ExecutionParams) const;
	float ApplyCriticalHit(float BaseDamage, const FGameplayEffectCustomExecutionParameters& ExecutionParams) const;
	float ApplyResistances(float Damage, const FGameplayEffectCustomExecutionParameters& ExecutionParams, const FGameplayTag& DamageType) const;
	void ApplyStatusEffects(const FGameplayEffectCustomExecutionParameters& ExecutionParams, OUT FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const;

private:
	// Captured attribute definitions for damage calculation
	FGameplayEffectAttributeCaptureDefinition AttackPowerDef;
	FGameplayEffectAttributeCaptureDefinition CriticalHitChanceDef;
	FGameplayEffectAttributeCaptureDefinition CriticalHitMultiplierDef;
	FGameplayEffectAttributeCaptureDefinition PhysicalResistanceDef;
	FGameplayEffectAttributeCaptureDefinition ElementalResistanceDef;
	FGameplayEffectAttributeCaptureDefinition HealthDef;
};

/**
 * Component that handles damage application and integration with combat systems
 */
UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class EROEOREOREOR_API UDamageApplicationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDamageApplicationComponent();

	// Main damage application function
	UFUNCTION(BlueprintCallable, Category = "Damage System")
	bool ApplyDamage(AActor* Target, const FAttackPrototypeData& AttackData, AActor* Instigator = nullptr);
	
	UFUNCTION(BlueprintCallable, Category = "Damage System")
	bool ApplyDamageFromActionData(AActor* Target, const FCombatActionData& ActionData, AActor* Instigator = nullptr);

	// Damage calculation without application (for previews/UI)
	UFUNCTION(BlueprintPure, Category = "Damage System")
	float CalculateDamagePreview(const FAttackPrototypeData& AttackData, AActor* Target, AActor* Instigator) const;

	// Damage type support
	UFUNCTION(BlueprintCallable, Category = "Damage System")
	void SetDamageType(const FGameplayTag& DamageType) { CurrentDamageType = DamageType; }

	// Events for damage application
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnDamageApplied, AActor*, Target, float, DamageAmount, bool, bWasCritical, FGameplayTag, DamageType);
	UPROPERTY(BlueprintAssignable)
	FOnDamageApplied OnDamageApplied;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTargetKilled, AActor*, Target, AActor*, Killer);
	UPROPERTY(BlueprintAssignable)
	FOnTargetKilled OnTargetKilled;

protected:
	// Default damage effect to use
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage System")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	// Current damage type for attacks
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage System")
	FGameplayTag CurrentDamageType;

	// Debug settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bShowDamageNumbers = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	float DamageNumberDuration = 2.0f;

private:
	// Helper functions
	UAbilitySystemComponent* GetTargetASC(AActor* Target) const;
	UAbilitySystemComponent* GetInstigatorASC(AActor* Instigator) const;
	void ShowDamageNumber(const FVector& Location, float Damage, bool bWasCritical) const;
	bool IsTargetDead(AActor* Target) const;
};
