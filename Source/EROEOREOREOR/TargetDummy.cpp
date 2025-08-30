#include "TargetDummy.h"
#include "GameplayEffect_Damage.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInstanceDynamic.h"

ATargetDummy::ATargetDummy()
{
	PrimaryActorTick.bCanEverTick = false;

	// Create root collision component
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	RootComponent = CollisionComponent;
	CollisionComponent->SetSphereRadius(50.0f);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionComponent->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Block);
	CollisionComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	// Create mesh component
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision); // Use sphere for collision
	
	// Try to load a default mesh (cylinder or cube)
	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("/Engine/BasicShapes/Cylinder"));
	if (MeshAsset.Succeeded())
	{
		MeshComponent->SetStaticMesh(MeshAsset.Object);
		MeshComponent->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.5f)); // Make it taller
	}

	// Create health bar widget component
	HealthBarWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBarWidget"));
	HealthBarWidget->SetupAttachment(RootComponent);
	HealthBarWidget->SetRelativeLocation(FVector(0.0f, 0.0f, 120.0f));
	HealthBarWidget->SetWidgetSpace(EWidgetSpace::Screen);
	HealthBarWidget->SetDrawSize(FVector2D(200.0f, 50.0f));

	// Initialize GAS components
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AttributeSet = CreateDefaultSubobject<UMyAttributeSet>(TEXT("AttributeSet"));

	// Set default values
	MaxHealth = 100.0f;
	bAutoRegenHealth = true;
	HealthRegenRate = 2.0f;
	RegenDelay = 3.0f;
	DefaultColor = FLinearColor::White;
	DamageColor = FLinearColor::Red;
	DamageEffectDuration = 0.5f;

	// Initialize tracking variables
	TotalDamageReceived = 0.0f;
	HitCount = 0;
}

void ATargetDummy::BeginPlay()
{
	Super::BeginPlay();
	
	// Initialize GAS
	if (AbilitySystemComponent && AttributeSet)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
		
		// Set initial health to max
		if (UMyAttributeSet* MyAttributeSet = Cast<UMyAttributeSet>(AttributeSet))
		{
			MyAttributeSet->SetHealth(MaxHealth);
			MyAttributeSet->SetMaxHealth(MaxHealth);
			
			// Set some default combat stats for testing
			MyAttributeSet->SetAttackPower(1.0f);
			MyAttributeSet->SetCriticalHitChance(0.1f); // 10% crit chance
			MyAttributeSet->SetCriticalHitMultiplier(2.0f); // 2x damage on crit
			MyAttributeSet->SetPhysicalResistance(0.0f); // No resistance by default
			MyAttributeSet->SetElementalResistance(0.0f);
		}
		
		// Listen for health changes - UE5.6 compatible approach using lambda
		if (AbilitySystemComponent)
		{
			AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMyAttributeSet::GetHealthAttribute()).AddLambda([this](const FOnAttributeChangeData& Data)
			{
				OnHealthChanged(Data.OldValue, Data.NewValue);
			});
		}
	}

	UE_LOG(LogTemp, Log, TEXT("TargetDummy: BeginPlay completed - Health initialized to %.1f"), MaxHealth);
}

UAbilitySystemComponent* ATargetDummy::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UMyAttributeSet* ATargetDummy::GetMyAttributeSet() const
{
	return Cast<UMyAttributeSet>(AttributeSet);
}

void ATargetDummy::ResetHealth()
{
	if (UMyAttributeSet* MyAttributeSet = GetMyAttributeSet())
	{
		MyAttributeSet->SetHealth(MaxHealth);
		TotalDamageReceived = 0.0f;
		HitCount = 0;
		
		// Clear any active timers
		if (GetWorld())
		{
			GetWorld()->GetTimerManager().ClearTimer(HealthRegenTimerHandle);
			GetWorld()->GetTimerManager().ClearTimer(RegenDelayTimerHandle);
		}
		
		ResetDamageEffect();
		
		UE_LOG(LogTemp, Log, TEXT("TargetDummy: Health reset to %.1f"), MaxHealth);
	}
}

float ATargetDummy::GetHealthPercent() const
{
	if (const UMyAttributeSet* MyAttributeSet = GetMyAttributeSet())
	{
		const float CurrentHealth = MyAttributeSet->GetHealth();
		const float MaxHP = MyAttributeSet->GetMaxHealth();
		return MaxHP > 0.0f ? CurrentHealth / MaxHP : 0.0f;
	}
	return 0.0f;
}

void ATargetDummy::ShowDamageEffect(float DamageAmount, bool bWasCritical)
{
	// Change material color to indicate damage
	if (UMaterialInstanceDynamic* DynamicMaterial = MeshComponent->CreateAndSetMaterialInstanceDynamic(0))
	{
		const FLinearColor EffectColor = bWasCritical ? FLinearColor::Yellow : DamageColor;
		DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), EffectColor);
		
		// Reset color after duration
		if (GetWorld())
		{
			GetWorld()->GetTimerManager().ClearTimer(DamageEffectTimerHandle);
			GetWorld()->GetTimerManager().SetTimer(
				DamageEffectTimerHandle,
				this,
				&ATargetDummy::ResetDamageEffect,
				DamageEffectDuration,
				false
			);
		}
	}
	
	UE_LOG(LogTemp, Log, TEXT("TargetDummy: Showing damage effect - %.1f damage%s"), 
		DamageAmount, bWasCritical ? TEXT(" (CRITICAL!)") : TEXT(""));
}

void ATargetDummy::LogCurrentStats() const
{
	if (const UMyAttributeSet* MyAttributeSet = GetMyAttributeSet())
	{
		UE_LOG(LogTemp, Log, TEXT("=== TARGET DUMMY STATS ==="));
		UE_LOG(LogTemp, Log, TEXT("Health: %.1f / %.1f (%.1f%%)"), 
			MyAttributeSet->GetHealth(), MyAttributeSet->GetMaxHealth(), GetHealthPercent() * 100.0f);
		UE_LOG(LogTemp, Log, TEXT("Total Damage Received: %.1f"), TotalDamageReceived);
		UE_LOG(LogTemp, Log, TEXT("Hit Count: %d"), HitCount);
		UE_LOG(LogTemp, Log, TEXT("Physical Resistance: %.3f"), MyAttributeSet->GetPhysicalResistance());
		UE_LOG(LogTemp, Log, TEXT("Elemental Resistance: %.3f"), MyAttributeSet->GetElementalResistance());
		UE_LOG(LogTemp, Log, TEXT("Attack Power: %.2f"), MyAttributeSet->GetAttackPower());
		UE_LOG(LogTemp, Log, TEXT("Crit Chance: %.1f%%"), MyAttributeSet->GetCriticalHitChance() * 100.0f);
		UE_LOG(LogTemp, Log, TEXT("Crit Multiplier: %.2fx"), MyAttributeSet->GetCriticalHitMultiplier());
		UE_LOG(LogTemp, Log, TEXT("Auto Regen: %s"), bAutoRegenHealth ? TEXT("ON") : TEXT("OFF"));
		UE_LOG(LogTemp, Log, TEXT("========================"));
	}
}

void ATargetDummy::StartHealthRegeneration()
{
	if (!bAutoRegenHealth || !GetWorld())
		return;

	// Clear any existing regen timer
	GetWorld()->GetTimerManager().ClearTimer(HealthRegenTimerHandle);
	
	// Start continuous regeneration
	GetWorld()->GetTimerManager().SetTimer(
		HealthRegenTimerHandle,
		this,
		&ATargetDummy::RegenerateHealth,
		1.0f, // Every 1 second
		true // Repeating
	);
	
	UE_LOG(LogTemp, Log, TEXT("TargetDummy: Started health regeneration at %.1f HP/sec"), HealthRegenRate);
}

void ATargetDummy::RegenerateHealth()
{
	UMyAttributeSet* MyAttributeSet = GetMyAttributeSet();
	if (!MyAttributeSet)
		return;

	const float CurrentHealth = MyAttributeSet->GetHealth();
	const float MaxHP = MyAttributeSet->GetMaxHealth();
	
	if (CurrentHealth >= MaxHP)
	{
		// Full health, stop regeneration
		if (GetWorld())
		{
			GetWorld()->GetTimerManager().ClearTimer(HealthRegenTimerHandle);
		}
		return;
	}
	
	// Regenerate health
	const float NewHealth = FMath::Min(CurrentHealth + HealthRegenRate, MaxHP);
	MyAttributeSet->SetHealth(NewHealth);
	
	if (FMath::IsNearlyEqual(NewHealth, MaxHP, 0.1f))
	{
		UE_LOG(LogTemp, Log, TEXT("TargetDummy: Health fully regenerated to %.1f"), NewHealth);
	}
}

void ATargetDummy::ResetDamageEffect()
{
	// Reset material color to default
	if (UMaterialInstanceDynamic* DynamicMaterial = MeshComponent->CreateAndSetMaterialInstanceDynamic(0))
	{
		DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), DefaultColor);
	}
}

void ATargetDummy::OnHealthChanged(float OldValue, float NewValue)
{
	const float HealthDifference = NewValue - OldValue;
	
	UE_LOG(LogTemp, Log, TEXT("TargetDummy: Health changed from %.1f to %.1f (diff: %.1f)"), 
		OldValue, NewValue, HealthDifference);
	
	// Track damage taken
	if (HealthDifference < 0.0f)
	{
		const float DamageTaken = -HealthDifference;
		TotalDamageReceived += DamageTaken;
		HitCount++;
		
		// Show visual effect
		ShowDamageEffect(DamageTaken, false); // TODO: Track critical hits
		
		// Reset any existing regen delay and start new one
		if (GetWorld() && bAutoRegenHealth)
		{
			GetWorld()->GetTimerManager().ClearTimer(RegenDelayTimerHandle);
			GetWorld()->GetTimerManager().ClearTimer(HealthRegenTimerHandle);
			
			GetWorld()->GetTimerManager().SetTimer(
				RegenDelayTimerHandle,
				this,
				&ATargetDummy::StartHealthRegeneration,
				RegenDelay,
				false
			);
		}
		
		UE_LOG(LogTemp, Warning, TEXT("TargetDummy: Took %.1f damage! Health: %.1f/%.1f (Total: %.1f, Hits: %d)"), 
			DamageTaken, NewValue, NewValue, TotalDamageReceived, HitCount);
	}
	
	// Handle death
	if (NewValue <= 0.0f)
	{
		UE_LOG(LogTemp, Error, TEXT("TargetDummy: DESTROYED! Total damage taken: %.1f over %d hits"), 
			TotalDamageReceived, HitCount);
		
		// Reset after a short delay for testing
		if (GetWorld())
		{
			FTimerHandle ResetTimerHandle;
			GetWorld()->GetTimerManager().SetTimer(
				ResetTimerHandle,
				this,
				&ATargetDummy::ResetHealth,
				2.0f,
				false
			);
		}
	}
}
