#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "GameplayEffectTypes.h"
#include "GameplayEffect.h"
#include "GameplayEffectExtension.h"
#include "MyAttributeSet.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "TargetDummy.generated.h"

/**
 * Target dummy actor for testing combat systems
 * Features:
 * - Full GAS integration for receiving damage
 * - Health display widget
 * - Visual feedback on damage received
 * - Automatic health regeneration for testing
 * - Collision detection for attack shapes
 */
UCLASS(BlueprintType, Blueprintable)
class EROEOREOREOR_API ATargetDummy : public AActor, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
public:	
	ATargetDummy();

	// IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// Getter for the custom AttributeSet
	UFUNCTION(BlueprintCallable, Category = "GAS")
	UMyAttributeSet* GetMyAttributeSet() const;

	// Health management
	UFUNCTION(BlueprintCallable, Category = "Health")
	void ResetHealth();

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetHealthPercent() const;

	// Visual feedback
	UFUNCTION(BlueprintCallable, Category = "Visual")
	void ShowDamageEffect(float DamageAmount, bool bWasCritical = false);

	// Testing utilities
	UFUNCTION(BlueprintCallable, Category = "Testing")
	void LogCurrentStats() const;

protected:
	virtual void BeginPlay() override;

	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UWidgetComponent> HealthBarWidget;

	// GAS Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UMyAttributeSet> AttributeSet;

	// Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float MaxHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool bAutoRegenHealth = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (ClampMin = "0.1", ClampMax = "10.0"))
	float HealthRegenRate = 2.0f; // Health per second

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (ClampMin = "0.0", ClampMax = "10.0"))
	float RegenDelay = 3.0f; // Seconds after damage before regen starts

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	FLinearColor DefaultColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	FLinearColor DamageColor = FLinearColor::Red;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	float DamageEffectDuration = 0.5f;

private:
	// Health regeneration
	FTimerHandle HealthRegenTimerHandle;
	FTimerHandle RegenDelayTimerHandle;
	
	// Visual effects
	FTimerHandle DamageEffectTimerHandle;
	
	// Damage tracking for testing
	UPROPERTY(Transient)
	float TotalDamageReceived = 0.0f;
	
	UPROPERTY(Transient)
	int32 HitCount = 0;

	// Internal functions
	void StartHealthRegeneration();
	void RegenerateHealth();
	void ResetDamageEffect();

	// Attribute change callbacks - UE5.6 compatible
	void OnHealthChanged(float OldValue, float NewValue);
};
