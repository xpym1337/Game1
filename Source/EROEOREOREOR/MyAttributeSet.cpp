#include "MyAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffect.h"
#include "GameplayEffectExtension.h"

UMyAttributeSet::UMyAttributeSet()
{
	// Initialize default values - Following Epic Games GAS patterns
	InitHealth(100.0f);
	InitMaxHealth(100.0f);
	InitStamina(100.0f);
	InitMaxStamina(100.0f);
	
	// Combat attributes - Initialize to reasonable defaults
	InitAttackPower(1.0f);           // Base damage multiplier
	InitCriticalHitChance(0.05f);    // 5% base crit chance
	InitCriticalHitMultiplier(1.5f); // 1.5x damage on crit
	InitAttackSpeed(1.0f);           // Normal attack speed
	InitPhysicalResistance(0.0f);    // No base resistance
	InitElementalResistance(0.0f);   // No base resistance
	
	// Movement state attributes - Initialize to zero
	InitAirBounceCount(0.0f);
}

void UMyAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Replicate attributes to all clients - Following Epic Games GAS patterns
	DOREPLIFETIME_CONDITION_NOTIFY(UMyAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMyAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMyAttributeSet, Stamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMyAttributeSet, MaxStamina, COND_None, REPNOTIFY_Always);
	
	// Combat attributes
	DOREPLIFETIME_CONDITION_NOTIFY(UMyAttributeSet, AttackPower, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMyAttributeSet, CriticalHitChance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMyAttributeSet, CriticalHitMultiplier, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMyAttributeSet, AttackSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMyAttributeSet, PhysicalResistance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMyAttributeSet, ElementalResistance, COND_None, REPNOTIFY_Always);
	
	// Movement state attributes
	DOREPLIFETIME_CONDITION_NOTIFY(UMyAttributeSet, AirBounceCount, COND_None, REPNOTIFY_Always);
}

void UMyAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMyAttributeSet, Health, OldHealth);
}

void UMyAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMyAttributeSet, MaxHealth, OldMaxHealth);
}

void UMyAttributeSet::OnRep_Stamina(const FGameplayAttributeData& OldStamina)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMyAttributeSet, Stamina, OldStamina);
}

void UMyAttributeSet::OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMyAttributeSet, MaxStamina, OldMaxStamina);
}

void UMyAttributeSet::OnRep_AirBounceCount(const FGameplayAttributeData& OldAirBounceCount)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMyAttributeSet, AirBounceCount, OldAirBounceCount);
}

// Combat Attributes OnRep Functions
void UMyAttributeSet::OnRep_AttackPower(const FGameplayAttributeData& OldAttackPower)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMyAttributeSet, AttackPower, OldAttackPower);
}

void UMyAttributeSet::OnRep_CriticalHitChance(const FGameplayAttributeData& OldCriticalHitChance)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMyAttributeSet, CriticalHitChance, OldCriticalHitChance);
}

void UMyAttributeSet::OnRep_CriticalHitMultiplier(const FGameplayAttributeData& OldCriticalHitMultiplier)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMyAttributeSet, CriticalHitMultiplier, OldCriticalHitMultiplier);
}

void UMyAttributeSet::OnRep_AttackSpeed(const FGameplayAttributeData& OldAttackSpeed)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMyAttributeSet, AttackSpeed, OldAttackSpeed);
}

void UMyAttributeSet::OnRep_PhysicalResistance(const FGameplayAttributeData& OldPhysicalResistance)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMyAttributeSet, PhysicalResistance, OldPhysicalResistance);
}

void UMyAttributeSet::OnRep_ElementalResistance(const FGameplayAttributeData& OldElementalResistance)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMyAttributeSet, ElementalResistance, OldElementalResistance);
}

void UMyAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	// This is called whenever attributes are going to change
	Super::PreAttributeChange(Attribute, NewValue);

	// Clamp attributes to valid ranges before they change
	if (Attribute == GetMaxHealthAttribute())
	{
		// Don't allow MaxHealth to drop below 1
		NewValue = FMath::Max(NewValue, 1.0f);
	}
	else if (Attribute == GetHealthAttribute())
	{
		// Clamp Health between 0 and MaxHealth
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	else if (Attribute == GetMaxStaminaAttribute())
	{
		// Don't allow MaxStamina to drop below 0
		NewValue = FMath::Max(NewValue, 0.0f);
	}
	else if (Attribute == GetStaminaAttribute())
	{
		// Clamp Stamina between 0 and MaxStamina
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxStamina());
	}
	else if (Attribute == GetAirBounceCountAttribute())
	{
		// Air bounce count must be non-negative integer - clamp and round to nearest integer
		NewValue = FMath::Max(0.0f, FMath::RoundToFloat(NewValue));
	}
}

void UMyAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	FGameplayEffectContextHandle Context = Data.EffectSpec.GetContext();
	UAbilitySystemComponent* Source = Context.GetOriginalInstigatorAbilitySystemComponent();
	const FGameplayTagContainer& SourceTags = *Data.EffectSpec.CapturedSourceTags.GetAggregatedTags();
	FGameplayTagContainer SpecAssetTags;
	Data.EffectSpec.GetAllAssetTags(SpecAssetTags);

	// Get the Target actor, which should be the owner of this set
	AActor* TargetActor = nullptr;
	AController* TargetController = nullptr;
	if (Data.Target.AbilityActorInfo.IsValid() && Data.Target.AbilityActorInfo->AvatarActor.IsValid())
	{
		TargetActor = Data.Target.AbilityActorInfo->AvatarActor.Get();
		TargetController = Data.Target.AbilityActorInfo->PlayerController.Get();
	}

	// Handle attribute changes after a gameplay effect has been applied
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		// Clamp Health between 0 and MaxHealth
		SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
	}
	else if (Data.EvaluatedData.Attribute == GetStaminaAttribute())
	{
		// Clamp Stamina between 0 and MaxStamina
		SetStamina(FMath::Clamp(GetStamina(), 0.0f, GetMaxStamina()));
	}
	else if (Data.EvaluatedData.Attribute == GetMaxHealthAttribute())
	{
		// If MaxHealth changes, adjust Health if necessary
		AdjustAttributeForMaxChange(Health, MaxHealth, GetMaxHealth(), GetHealthAttribute());
	}
	else if (Data.EvaluatedData.Attribute == GetMaxStaminaAttribute())
	{
		// If MaxStamina changes, adjust Stamina if necessary
		AdjustAttributeForMaxChange(Stamina, MaxStamina, GetMaxStamina(), GetStaminaAttribute());
	}
}

void UMyAttributeSet::AdjustAttributeForMaxChange(FGameplayAttributeData& AffectedAttribute, const FGameplayAttributeData& MaxAttribute, float NewMaxValue, const FGameplayAttribute& AffectedAttributeProperty)
{
	UAbilitySystemComponent* AbilityComp = GetOwningAbilitySystemComponent();
	const float CurrentMaxValue = MaxAttribute.GetCurrentValue();
	if (!FMath::IsNearlyEqual(CurrentMaxValue, NewMaxValue) && AbilityComp)
	{
		// Change current value to maintain the current Val / Max percent
		const float CurrentValue = AffectedAttribute.GetCurrentValue();
		float NewDelta = (CurrentMaxValue > 0.f) ? (CurrentValue * NewMaxValue / CurrentMaxValue) - CurrentValue : NewMaxValue;

		AbilityComp->ApplyModToAttributeUnsafe(AffectedAttributeProperty, EGameplayModOp::Additive, NewDelta);
	}
}
