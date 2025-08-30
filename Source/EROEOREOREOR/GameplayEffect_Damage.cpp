#include "GameplayEffect_Damage.h"
#include "MyAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "GameplayEffectExecutionCalculation.h"
#include "AbilitySystemInterface.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Actor.h"
#include "Misc/Optional.h"

// Damage type tags for the system - moved to static function to avoid static initialization order issues
namespace DamageTags
{
	static FGameplayTag Physical;
	static FGameplayTag Elemental; 
	static FGameplayTag BaseDamage;
	static FGameplayTag CriticalHit;
	static bool bTagsInitialized = false;
	
	void InitializeTags()
	{
		if (!bTagsInitialized)
		{
			Physical = FGameplayTag::RequestGameplayTag(FName("Damage.Type.Physical"));
			Elemental = FGameplayTag::RequestGameplayTag(FName("Damage.Type.Elemental"));
			BaseDamage = FGameplayTag::RequestGameplayTag(FName("Damage.Base"));
			CriticalHit = FGameplayTag::RequestGameplayTag(FName("Damage.Critical"));
			bTagsInitialized = true;
		}
	}
}

UGameplayEffect_Damage::UGameplayEffect_Damage()
{
	// Initialize damage tags when the class is constructed
	DamageTags::InitializeTags();
	
	// Set up the damage effect as instant
	DurationPolicy = EGameplayEffectDurationType::Instant;
	
	// Use custom execution calculation for complex damage math
	FGameplayEffectExecutionDefinition ExecutionDef;
	ExecutionDef.CalculationClass = UDamageExecutionCalculation::StaticClass();
	Executions.Add(ExecutionDef);
	
	// Add gameplay tags using the new UE5 API
	// InheritableOwnedTagsContainer is deprecated, tags should be set via components or other means
	// For now, we'll handle tags in the execution calculation
}

UDamageExecutionCalculation::UDamageExecutionCalculation()
{
	// Initialize damage tags to ensure they're available
	DamageTags::InitializeTags();
	
	// Define which attributes we need to capture for damage calculation
	AttackPowerDef = FGameplayEffectAttributeCaptureDefinition(UMyAttributeSet::GetAttackPowerAttribute(), EGameplayEffectAttributeCaptureSource::Source, false);
	CriticalHitChanceDef = FGameplayEffectAttributeCaptureDefinition(UMyAttributeSet::GetCriticalHitChanceAttribute(), EGameplayEffectAttributeCaptureSource::Source, false);
	CriticalHitMultiplierDef = FGameplayEffectAttributeCaptureDefinition(UMyAttributeSet::GetCriticalHitMultiplierAttribute(), EGameplayEffectAttributeCaptureSource::Source, false);
	PhysicalResistanceDef = FGameplayEffectAttributeCaptureDefinition(UMyAttributeSet::GetPhysicalResistanceAttribute(), EGameplayEffectAttributeCaptureSource::Target, false);
	ElementalResistanceDef = FGameplayEffectAttributeCaptureDefinition(UMyAttributeSet::GetElementalResistanceAttribute(), EGameplayEffectAttributeCaptureSource::Target, false);
	HealthDef = FGameplayEffectAttributeCaptureDefinition(UMyAttributeSet::GetHealthAttribute(), EGameplayEffectAttributeCaptureSource::Target, false);

	// Add all capture definitions to the array
	RelevantAttributesToCapture.Add(AttackPowerDef);
	RelevantAttributesToCapture.Add(CriticalHitChanceDef);
	RelevantAttributesToCapture.Add(CriticalHitMultiplierDef);
	RelevantAttributesToCapture.Add(PhysicalResistanceDef);
	RelevantAttributesToCapture.Add(ElementalResistanceDef);
	RelevantAttributesToCapture.Add(HealthDef);
}

void UDamageExecutionCalculation::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, OUT FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	UAbilitySystemComponent* TargetABC = ExecutionParams.GetTargetAbilitySystemComponent();
	UAbilitySystemComponent* SourceABC = ExecutionParams.GetSourceAbilitySystemComponent();

	AActor* SourceActor = SourceABC ? SourceABC->GetAvatarActor() : nullptr;
	AActor* TargetActor = TargetABC ? TargetABC->GetAvatarActor() : nullptr;

	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;

	// Step 1: Calculate base damage
	float BaseDamage = CalculateBaseDamage(ExecutionParams);
	
	// Step 2: Apply critical hit calculation
	float FinalDamage = ApplyCriticalHit(BaseDamage, ExecutionParams);
	
	// Step 3: Determine damage type and apply resistances
	FGameplayTag DamageType = DamageTags::Physical; // Default to physical
	if (SourceTags && SourceTags->HasTag(DamageTags::Elemental))
	{
		DamageType = DamageTags::Elemental;
	}
	
	FinalDamage = ApplyResistances(FinalDamage, ExecutionParams, DamageType);
	
	// Step 4: Apply damage to health (negative value decreases health)
	if (FinalDamage > 0.0f)
	{
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(UMyAttributeSet::GetHealthAttribute(), EGameplayModOp::Additive, -FinalDamage));
		
		// Log damage for debugging
		if (SourceActor && TargetActor)
		{
			UE_LOG(LogTemp, Log, TEXT("DamageExecution: %s dealt %.1f damage to %s"), 
				*SourceActor->GetName(), FinalDamage, *TargetActor->GetName());
		}
	}
	
	// Step 5: Apply any status effects
	ApplyStatusEffects(ExecutionParams, OutExecutionOutput);
}

float UDamageExecutionCalculation::CalculateBaseDamage(const FGameplayEffectCustomExecutionParameters& ExecutionParams) const
{
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	
	// Get base damage from SetByCaller (passed from AttackPrototypeData.BaseDamage)
	float BaseDamage = Spec.GetSetByCallerMagnitude(DamageTags::BaseDamage, false, 0.0f);
	
	if (BaseDamage <= 0.0f)
	{
		// Fallback: try to get from a different tag or use default
		BaseDamage = 25.0f; // Default damage
	}
	
	// Apply attacker's attack power
	float AttackPower = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(AttackPowerDef, FAggregatorEvaluateParameters(), AttackPower);
	
	// AttackPower acts as a multiplier (1.0 = normal, 1.5 = 50% bonus, etc.)
	float FinalBaseDamage = BaseDamage * FMath::Max(AttackPower, 0.1f); // Minimum 10% damage
	
	return FinalBaseDamage;
}

float UDamageExecutionCalculation::ApplyCriticalHit(float BaseDamage, const FGameplayEffectCustomExecutionParameters& ExecutionParams) const
{
	float CritChance = 0.0f;
	float CritMultiplier = 1.0f;
	
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(CriticalHitChanceDef, FAggregatorEvaluateParameters(), CritChance);
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(CriticalHitMultiplierDef, FAggregatorEvaluateParameters(), CritMultiplier);
	
	// Roll for critical hit
	const float CritRoll = FMath::RandRange(0.0f, 1.0f);
	const bool bIsCritical = CritRoll <= FMath::Clamp(CritChance, 0.0f, 1.0f);
	
	if (bIsCritical)
	{
		// Add critical hit tag to the effect context for tracking
		FGameplayEffectContextHandle ContextHandle = ExecutionParams.GetOwningSpec().GetEffectContext();
		ContextHandle.AddSourceObject(const_cast<UDamageExecutionCalculation*>(this));
		
		// Apply critical multiplier
		float CriticalDamage = BaseDamage * FMath::Max(CritMultiplier, 1.0f);
		
		UE_LOG(LogTemp, Log, TEXT("CRITICAL HIT! %.1f -> %.1f (%.1fx multiplier)"), 
			BaseDamage, CriticalDamage, CritMultiplier);
			
		return CriticalDamage;
	}
	
	return BaseDamage;
}

float UDamageExecutionCalculation::ApplyResistances(float Damage, const FGameplayEffectCustomExecutionParameters& ExecutionParams, const FGameplayTag& DamageType) const
{
	float Resistance = 0.0f;
	
	// Apply appropriate resistance based on damage type
	if (DamageType == DamageTags::Physical)
	{
		ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(PhysicalResistanceDef, FAggregatorEvaluateParameters(), Resistance);
	}
	else if (DamageType == DamageTags::Elemental)
	{
		ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(ElementalResistanceDef, FAggregatorEvaluateParameters(), Resistance);
	}
	
	// Resistance is a percentage (0.0 to 1.0), apply damage reduction
	const float ResistanceMultiplier = 1.0f - FMath::Clamp(Resistance, 0.0f, 0.95f); // Max 95% resistance
	const float FinalDamage = Damage * ResistanceMultiplier;
	
	if (Resistance > 0.0f)
	{
		UE_LOG(LogTemp, Log, TEXT("Resistance Applied: %.1f -> %.1f (%.1f%% resistance)"), 
			Damage, FinalDamage, Resistance * 100.0f);
	}
	
	return FinalDamage;
}

void UDamageExecutionCalculation::ApplyStatusEffects(const FGameplayEffectCustomExecutionParameters& ExecutionParams, OUT FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	// TODO: Implement status effects (poison, burn, freeze, etc.) in future iterations
	// For now, just log that this is where status effects would be applied
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	
	if (SourceTags && SourceTags->HasAny(FGameplayTagContainer(FGameplayTag::RequestGameplayTag(FName("Status")))))
	{
		UE_LOG(LogTemp, Log, TEXT("Status effects would be applied here"));
	}
}

// Damage Application Component Implementation
UDamageApplicationComponent::UDamageApplicationComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	
	// Initialize damage tags
	DamageTags::InitializeTags();
	
	// Set default damage effect
	DamageEffectClass = UGameplayEffect_Damage::StaticClass();
	CurrentDamageType = DamageTags::Physical;
	
	bShowDamageNumbers = true;
	DamageNumberDuration = 2.0f;
}

bool UDamageApplicationComponent::ApplyDamage(AActor* Target, const FAttackPrototypeData& AttackData, AActor* Instigator)
{
	if (!Target || !DamageEffectClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("ApplyDamage: Invalid Target or DamageEffectClass"));
		return false;
	}
	
	UAbilitySystemComponent* TargetASC = GetTargetASC(Target);
	UAbilitySystemComponent* InstigatorASC = GetInstigatorASC(Instigator);
	
	if (!TargetASC)
	{
		UE_LOG(LogTemp, Warning, TEXT("ApplyDamage: Target %s has no AbilitySystemComponent"), *Target->GetName());
		return false;
	}
	
	// Create gameplay effect spec
	FGameplayEffectContextHandle ContextHandle = InstigatorASC ? InstigatorASC->MakeEffectContext() : TargetASC->MakeEffectContext();
	ContextHandle.AddSourceObject(this);
	ContextHandle.AddInstigator(Instigator, Instigator);
	
	FGameplayEffectSpecHandle SpecHandle = InstigatorASC ? 
		InstigatorASC->MakeOutgoingSpec(DamageEffectClass, 1.0f, ContextHandle) :
		TargetASC->MakeOutgoingSpec(DamageEffectClass, 1.0f, ContextHandle);
		
	if (!SpecHandle.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("ApplyDamage: Failed to create GameplayEffectSpec"));
		return false;
	}
	
	// Set damage amount from attack data
	SpecHandle.Data->SetSetByCallerMagnitude(DamageTags::BaseDamage, AttackData.BaseDamage);
	
	// Add damage type tag using the new UE5 API
	SpecHandle.Data->AddDynamicAssetTag(CurrentDamageType);
	
	// Apply the damage effect
	FActiveGameplayEffectHandle EffectHandle = TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	
	const bool bDamageApplied = EffectHandle.IsValid();
	
	if (bDamageApplied)
	{
		// Show damage numbers if enabled
		if (bShowDamageNumbers)
		{
			ShowDamageNumber(Target->GetActorLocation(), AttackData.BaseDamage, false); // TODO: Track critical hits
		}
		
		// Broadcast damage applied event
		OnDamageApplied.Broadcast(Target, AttackData.BaseDamage, false, CurrentDamageType);
		
		// Check if target died
		if (IsTargetDead(Target))
		{
			OnTargetKilled.Broadcast(Target, Instigator);
		}
		
		UE_LOG(LogTemp, Log, TEXT("ApplyDamage: Successfully applied %.1f damage to %s"), 
			AttackData.BaseDamage, *Target->GetName());
	}
	
	return bDamageApplied;
}

bool UDamageApplicationComponent::ApplyDamageFromActionData(AActor* Target, const FCombatActionData& ActionData, AActor* Instigator)
{
	// Convert FCombatActionData to FAttackPrototypeData for damage application
	FAttackPrototypeData TempAttackData;
	TempAttackData.BaseDamage = 25.0f; // Default damage - ActionData should have this field added
	TempAttackData.AttackName = ActionData.DisplayName;
	
	return ApplyDamage(Target, TempAttackData, Instigator);
}

float UDamageApplicationComponent::CalculateDamagePreview(const FAttackPrototypeData& AttackData, AActor* Target, AActor* Instigator) const
{
	// Simple preview calculation without applying effects
	float PreviewDamage = AttackData.BaseDamage;
	
	if (Instigator)
	{
		UAbilitySystemComponent* InstigatorASC = GetInstigatorASC(Instigator);
		if (InstigatorASC)
		{
			// Apply attack power if available
			const UMyAttributeSet* AttributeSet = InstigatorASC->GetSet<UMyAttributeSet>();
			if (AttributeSet)
			{
				PreviewDamage *= AttributeSet->GetAttackPower();
			}
		}
	}
	
	if (Target)
	{
		UAbilitySystemComponent* TargetASC = GetTargetASC(Target);
		if (TargetASC)
		{
			// Apply resistance if available
			const UMyAttributeSet* AttributeSet = TargetASC->GetSet<UMyAttributeSet>();
			if (AttributeSet)
			{
				float Resistance = CurrentDamageType == DamageTags::Physical ? 
					AttributeSet->GetPhysicalResistance() : 
					AttributeSet->GetElementalResistance();
					
				PreviewDamage *= (1.0f - FMath::Clamp(Resistance, 0.0f, 0.95f));
			}
		}
	}
	
	return FMath::Max(PreviewDamage, 1.0f); // Minimum 1 damage
}

UAbilitySystemComponent* UDamageApplicationComponent::GetTargetASC(AActor* Target) const
{
	if (!Target)
		return nullptr;
		
	// Try IAbilitySystemInterface first
	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Target))
	{
		return ASI->GetAbilitySystemComponent();
	}
	
	// Fallback: find component directly
	return Target->FindComponentByClass<UAbilitySystemComponent>();
}

UAbilitySystemComponent* UDamageApplicationComponent::GetInstigatorASC(AActor* Instigator) const
{
	return GetTargetASC(Instigator); // Same logic
}

void UDamageApplicationComponent::ShowDamageNumber(const FVector& Location, float Damage, bool bWasCritical) const
{
	if (!GetWorld())
		return;
		
	const FColor DamageColor = bWasCritical ? FColor::Yellow : FColor::Red;
	const FString DamageText = bWasCritical ? 
		FString::Printf(TEXT("%.0f!"), Damage) : 
		FString::Printf(TEXT("%.0f"), Damage);
	
	// Draw damage number in world space
	DrawDebugString(GetWorld(), Location + FVector(0, 0, 100), DamageText, 
		nullptr, DamageColor, DamageNumberDuration, true, 2.0f);
}

bool UDamageApplicationComponent::IsTargetDead(AActor* Target) const
{
	UAbilitySystemComponent* TargetASC = GetTargetASC(Target);
	if (!TargetASC)
		return false;
		
	const UMyAttributeSet* AttributeSet = TargetASC->GetSet<UMyAttributeSet>();
	if (!AttributeSet)
		return false;
		
	return AttributeSet->GetHealth() <= 0.0f;
}
