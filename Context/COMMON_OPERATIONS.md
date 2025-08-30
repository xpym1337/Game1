# COMMON OPERATIONS

## Spawning Actors

### 🎭 Spawning Basic Actors

```cpp
// Source: Any actor class
#include "Engine/World.h"
#include "TargetDummy.h"

// Spawn a target dummy for testing
FVector SpawnLocation = GetActorLocation() + FVector(300.0f, 0.0f, 0.0f);
FRotator SpawnRotation = FRotator::ZeroRotator;

ATargetDummy* SpawnedDummy = GetWorld()->SpawnActor<ATargetDummy>(
    ATargetDummy::StaticClass(),
    SpawnLocation,
    SpawnRotation
);

if (SpawnedDummy)
{
    // Configure the spawned dummy
    SpawnedDummy->SetActorScale3D(FVector(1.5f));
    UE_LOG(LogTemp, Log, TEXT("Target dummy spawned successfully"));
}
```

### 🎮 Spawning Actors with Components

```cpp
// Spawn character with specific component setup
#include "MyCharacter.h"

FActorSpawnParameters SpawnParams;
SpawnParams.Owner = this;
SpawnParams.Instigator = GetInstigator();
SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

AMyCharacter* SpawnedCharacter = GetWorld()->SpawnActor<AMyCharacter>(
    AMyCharacter::StaticClass(),
    SpawnLocation,
    SpawnRotation,
    SpawnParams
);

if (SpawnedCharacter)
{
    // Access components immediately after spawn
    if (UCombatStateMachineComponent* CombatComp = SpawnedCharacter->FindComponentByClass<UCombatStateMachineComponent>())
    {
        CombatComp->SetDebugVisualization(true);
    }
}
```

## Accessing Player from Different Contexts

### 🎯 From GameMode/GameState

```cpp
// Source: Any GameMode or GameState class
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "MyCharacter.h"

// Get player character from GameMode
AMyCharacter* GetPlayerCharacter()
{
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            return Cast<AMyCharacter>(PC->GetPawn());
        }
    }
    return nullptr;
}

// Usage example
void AMyGameMode::SomeGameModeFunction()
{
    if (AMyCharacter* Player = GetPlayerCharacter())
    {
        // Access player's combat system
        if (UCombatStateMachineComponent* CombatComp = Player->FindComponentByClass<UCombatStateMachineComponent>())
        {
            CombatComp->ForceEndAction(false);
        }
    }
}
```

### 🎮 From UI Widgets

```cpp
// Source: UI Widget classes
#include "Blueprint/UserWidget.h"
#include "Engine/World.h"
#include "MyCharacter.h"

// Get player character from UI context
AMyCharacter* GetPlayerCharacterFromWidget()
{
    if (APlayerController* PC = GetOwningPlayer())
    {
        return Cast<AMyCharacter>(PC->GetPawn());
    }
    return nullptr;
}

// Usage in UI widget
void UUI_HealthBar::UpdateHealthDisplay()
{
    if (AMyCharacter* Player = GetPlayerCharacterFromWidget())
    {
        if (UMyAttributeSet* AttributeSet = Player->GetMyAttributeSet())
        {
            float CurrentHealth = AttributeSet->GetHealth();
            float MaxHealth = AttributeSet->GetMaxHealth();
            
            // Update health bar UI
            HealthBar->SetPercent(CurrentHealth / MaxHealth);
        }
    }
}
```

### 🧩 From Components

```cpp
// Source: Any UActorComponent
#include "MyCharacter.h"

// Access owner as MyCharacter
AMyCharacter* GetOwnerCharacter()
{
    return Cast<AMyCharacter>(GetOwner());
}

// Usage in component
void UCustomComponent::SomeComponentFunction()
{
    if (AMyCharacter* OwnerChar = GetOwnerCharacter())
    {
        // Access character's systems
        OwnerChar->GetAbilitySystemComponent()->TryActivateAbilityByClass(UGameplayAbility_Dash::StaticClass());
    }
}
```

## Using Gameplay Tags

### 🏷️ Tag Validation and Comparison

```cpp
// Source: Any class using gameplay tags
#include "GameplayTagContainer.h"

// Required include for tag definitions
// Tags defined in: Config/Tags/GameplayTags.ini

// Create and validate tags
FGameplayTag DashingTag = FGameplayTag::RequestGameplayTag(FName("State.Dashing"));
FGameplayTag CombatTag = FGameplayTag::RequestGameplayTag(FName("Combat.Action.Light.Jab"));

// Validate tag exists
if (DashingTag.IsValid())
{
    UE_LOG(LogTemp, Log, TEXT("Dashing tag is valid: %s"), *DashingTag.ToString());
}

// Tag comparison
bool bIsDashing = CharacterState.HasTag(DashingTag);
bool bInCombat = CharacterState.HasAny(FGameplayTagContainer(CombatTag));

// Tag matching patterns
FGameplayTagContainer CombatTags;
CombatTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Combat.Action.Light")));
bool bIsLightAttack = ActionTag.MatchesTag(CombatTags.First());
```

### 🎮 GAS Tag Integration

```cpp
// Source: Any class with AbilitySystemComponent access
#include "AbilitySystemComponent.h"

// Add/remove tags from character
void AddGameplayTag(AActor* Actor, const FGameplayTag& Tag)
{
    if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor))
    {
        ASC->AddLooseGameplayTag(Tag);
    }
}

void RemoveGameplayTag(AActor* Actor, const FGameplayTag& Tag)
{
    if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor))
    {
        ASC->RemoveLooseGameplayTag(Tag);
    }
}

// Check for tags on character
bool HasGameplayTag(AActor* Actor, const FGameplayTag& Tag)
{
    if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor))
    {
        return ASC->HasMatchingGameplayTag(Tag);
    }
    return false;
}
```

### 📋 Combat Tag Usage Examples

```cpp
// Source: CombatStateMachineComponent.cpp example usage

// Check if action can be performed
bool UCombatStateMachineComponent::CanStartAction(const FGameplayTag& ActionTag) const
{
    // Get action data
    const FCombatActionData* ActionData = LoadedActions.Find(ActionTag);
    if (!ActionData)
    {
        return false;
    }
    
    // Check if currently in a state that allows new actions
    if (CurrentState == ECombatState::Stunned)
    {
        return false;
    }
    
    // Check priority system
    if (CurrentState != ECombatState::Idle)
    {
        return CanInterrupt(ActionTag, CurrentActionTag);
    }
    
    return true;
}

// Combat state tag management
void UCombatStateMachineComponent::SetState(ECombatState NewState)
{
    ECombatState OldState = CurrentState;
    CurrentState = NewState;
    
    // Update gameplay tags based on state
    if (AMyCharacter* Character = OwnerCharacter.Get())
    {
        UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent();
        
        // Remove old state tags
        ASC->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("Combat.State.Startup")));
        ASC->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("Combat.State.Active")));
        ASC->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("Combat.State.Recovery")));
        
        // Add new state tag
        switch (NewState)
        {
            case ECombatState::Startup:
                ASC->AddLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("Combat.State.Startup")));
                break;
            case ECombatState::Active:
                ASC->AddLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("Combat.State.Active")));
                break;
            case ECombatState::Recovery:
                ASC->AddLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("Combat.State.Recovery")));
                break;
        }
    }
    
    // Broadcast state change
    OnStateChanged.Broadcast(OldState, NewState, CurrentActionTag);
}
```

## Setting Up Timers and Async Operations

### ⏰ Timer Management Patterns

```cpp
// Source: Any class inheriting from UObject with GetWorld() access
#include "Engine/TimerManager.h"

class UMyComponent : public UActorComponent
{
public:
    // Timer handle for cleanup
    FTimerHandle MyTimerHandle;
    
    void StartRepeatingOperation()
    {
        FTimerManager& TimerManager = GetWorld()->GetTimerManager();
        
        // Repeating timer
        TimerManager.SetTimer(
            MyTimerHandle,                           // Handle for management
            this,                                    // Object
            &UMyComponent::MyRepeatingFunction,      // Function
            1.0f,                                    // Interval (seconds)
            true                                     // Repeating
        );
    }
    
    void StartDelayedOperation()
    {
        FTimerManager& TimerManager = GetWorld()->GetTimerManager();
        
        // One-shot timer with lambda
        TimerManager.SetTimer(
            MyTimerHandle,
            [this]()
            {
                // Inline operation
                UE_LOG(LogTemp, Log, TEXT("Delayed operation executed"));
                SomeDelayedFunction();
            },
            2.5f,                                    // Delay (seconds)
            false                                    // Not repeating
        );
    }
    
    void CleanupTimers()
    {
        if (GetWorld())
        {
            GetWorld()->GetTimerManager().ClearTimer(MyTimerHandle);
        }
    }
    
private:
    UFUNCTION()
    void MyRepeatingFunction()
    {
        UE_LOG(LogTemp, Log, TEXT("Repeating timer function called"));
    }
    
    void SomeDelayedFunction()
    {
        // Implementation
    }
};
```

### 🎯 Dash System Timer Example

```cpp
// Source: GameplayAbility_Dash.cpp - Real project example

void UGameplayAbility_Dash::ActivateAbility(/* parameters */)
{
    // Set up velocity update timer
    float UpdateRate = 1.0f / UpdateFrequency;
    
    FTimerManager& TimerManager = GetWorld()->GetTimerManager();
    TimerManager.SetTimer(
        VelocityUpdateTimer,
        this,
        &UGameplayAbility_Dash::UpdateDashVelocity,
        UpdateRate,
        true  // Repeating
    );
    
    // Set up dash end timer
    TimerManager.SetTimer(
        DashEndTimer,
        [this]()
        {
            EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
        },
        DashDuration,
        false  // One-shot
    );
}

void UGameplayAbility_Dash::EndAbility(/* parameters */)
{
    // Clean up timers
    if (GetWorld())
    {
        FTimerManager& TimerManager = GetWorld()->GetTimerManager();
        TimerManager.ClearTimer(VelocityUpdateTimer);
        TimerManager.ClearTimer(DashEndTimer);
    }
    
    Super::EndAbility(/* parameters */);
}
```

### 🔄 Async Asset Loading

```cpp
// Source: GameplayAbility_Dash.h - Streamable asset loading
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"

void UGameplayAbility_Dash::LoadCurveAssets()
{
    if (DashSpeedCurve.IsNull())
    {
        return;
    }
    
    // Get streamable manager
    UAssetManager& AssetManager = UAssetManager::Get();
    FStreamableManager& StreamableManager = AssetManager.GetStreamableManager();
    
    // Create array of assets to load
    TArray<FSoftObjectPath> AssetsToLoad;
    AssetsToLoad.Add(DashSpeedCurve.ToSoftObjectPath());
    
    if (!DashDirectionCurve.IsNull())
    {
        AssetsToLoad.Add(DashDirectionCurve.ToSoftObjectPath());
    }
    
    // Load assets asynchronously
    CurveLoadHandle = StreamableManager.RequestAsyncLoad(
        AssetsToLoad,
        FStreamableDelegate::CreateUObject(this, &UGameplayAbility_Dash::OnCurveAssetsLoaded),
        FStreamableManager::AsyncLoadHighPriority
    );
}

void UGameplayAbility_Dash::OnCurveAssetsLoaded()
{
    // Assets are now loaded and ready to use
    LoadedDashSpeedCurve = DashSpeedCurve.Get();
    LoadedDashDirectionCurve = DashDirectionCurve.Get();
    
    UE_LOG(LogTemp, Log, TEXT("Dash curve assets loaded successfully"));
}
```

## Accessing Player from Different Contexts

### 🎮 From World Context

```cpp
// Source: Any class with UWorld access
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "MyCharacter.h"

// Generic player access function
AMyCharacter* GetPlayerCharacterFromWorld(UWorld* World)
{
    if (!World)
    {
        return nullptr;
    }
    
    // Get first player controller
    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC)
    {
        return nullptr;
    }
    
    // Get possessed pawn as MyCharacter
    return Cast<AMyCharacter>(PC->GetPawn());
}

// Usage example
void USomeSubsystem::DoSomethingWithPlayer()
{
    if (AMyCharacter* Player = GetPlayerCharacterFromWorld(GetWorld()))
    {
        // Safe to use player
        Player->TestCombatSystem();
    }
}
```

### 🎯 From Gameplay Abilities

```cpp
// Source: Any UGameplayAbility subclass
#include "MyCharacter.h"

AMyCharacter* UMyGameplayAbility::GetCharacterFromActorInfo() const
{
    return Cast<AMyCharacter>(GetAvatarActorFromActorInfo());
}

// Usage in ability
void UGameplayAbility_CustomAbility::ActivateAbility(/* parameters */)
{
    AMyCharacter* Character = GetCharacterFromActorInfo();
    if (!Character)
    {
        EndAbility(/* parameters with failure */);
        return;
    }
    
    // Access character systems
    UCameraComponent* Camera = Character->GetFollowCamera();
    FVector CameraForward = Camera->GetForwardVector();
    
    // Use camera direction for ability
    FVector AbilityDirection = FVector(CameraForward.X, CameraForward.Y, 0.0f).GetSafeNormal();
}
```

### 🔧 From Components

```cpp
// Source: Any UActorComponent
#include "MyCharacter.h"

// Safe owner casting pattern
AMyCharacter* UMyCustomComponent::GetOwnerCharacter() const
{
    return Cast<AMyCharacter>(GetOwner());
}

// Usage with null checking
void UMyCustomComponent::ComponentFunction()
{
    AMyCharacter* Character = GetOwnerCharacter();
    if (!Character)
    {
        UE_LOG(LogTemp, Warning, TEXT("Component owner is not AMyCharacter"));
        return;
    }
    
    // Access character's components safely
    if (UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent())
    {
        ASC->TryActivateAbilityByClass(UGameplayAbility_Bounce::StaticClass());
    }
}
```

## Using Gameplay Tags Effectively

### 🏷️ Tag Constants and Caching

```cpp
// Source: Header file (e.g., MyCharacter.h)
// Cache frequently used tags as static members

class AMyCharacter
{
public:
    // Cached gameplay tags for performance
    static const FGameplayTag TAG_State_Dashing;
    static const FGameplayTag TAG_State_Attacking;
    static const FGameplayTag TAG_Ability_Dash;
    static const FGameplayTag TAG_Ability_Bounce;
    static const FGameplayTag TAG_Combat_Action_Light_Jab;
};

// Source: Implementation file (MyCharacter.cpp)
// Initialize static tags
const FGameplayTag AMyCharacter::TAG_State_Dashing = FGameplayTag::RequestGameplayTag(FName("State.Dashing"));
const FGameplayTag AMyCharacter::TAG_State_Attacking = FGameplayTag::RequestGameplayTag(FName("State.Attacking"));
const FGameplayTag AMyCharacter::TAG_Ability_Dash = FGameplayTag::RequestGameplayTag(FName("Ability.Dash"));
const FGameplayTag AMyCharacter::TAG_Ability_Bounce = FGameplayTag::RequestGameplayTag(FName("Ability.Bounce"));
const FGameplayTag AMyCharacter::TAG_Combat_Action_Light_Jab = FGameplayTag::RequestGameplayTag(FName("Combat.Action.Light.Jab"));

// Usage
void AMyCharacter::SomeFunction()
{
    if (GetAbilitySystemComponent()->HasMatchingGameplayTag(TAG_State_Dashing))
    {
        // Character is currently dashing
    }
}
```

### 🎯 Tag Container Operations

```cpp
// Source: Any class using tag containers
#include "GameplayTagContainer.h"

// Create tag containers
FGameplayTagContainer MovementTags;
MovementTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Dashing")));
MovementTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.InAir")));

FGameplayTagContainer CombatTags;
CombatTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Attacking")));
CombatTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Combat.State.Active")));

// Tag container queries
bool bIsMoving = PlayerTags.HasAny(MovementTags);
bool bInCombat = PlayerTags.HasAll(CombatTags);
bool bCanAct = !PlayerTags.HasTag(FGameplayTag::RequestGameplayTag(FName("State.Stunned")));

// Filter tags by parent
FGameplayTag CombatParent = FGameplayTag::RequestGameplayTag(FName("Combat"));
FGameplayTagContainer CombatRelatedTags = PlayerTags.Filter(FGameplayTagContainer(CombatParent));
```

## Setting Up Timers and Async Operations

### ⏱️ Advanced Timer Patterns

```cpp
// Source: Any class with timer management needs

class UAdvancedTimerComponent : public UActorComponent
{
private:
    // Multiple timer handles for different operations
    FTimerHandle PrimaryTimer;
    FTimerHandle SecondaryTimer;
    FTimerHandle CleanupTimer;
    
public:
    // Complex timer setup with delegates
    void StartComplexOperation()
    {
        FTimerManager& TimerManager = GetWorld()->GetTimerManager();
        
        // Phase 1: Initial setup
        TimerManager.SetTimer(
            PrimaryTimer,
            FTimerDelegate::CreateUObject(this, &UAdvancedTimerComponent::Phase1Complete),
            1.0f,
            false
        );
    }
    
    void Phase1Complete()
    {
        UE_LOG(LogTemp, Log, TEXT("Phase 1 completed, starting phase 2"));
        
        // Phase 2: Main operation
        GetWorld()->GetTimerManager().SetTimer(
            SecondaryTimer,
            [this]()
            {
                Phase2Complete();
            },
            2.0f,
            false
        );
    }
    
    void Phase2Complete()
    {
        UE_LOG(LogTemp, Log, TEXT("Phase 2 completed, starting cleanup"));
        
        // Phase 3: Cleanup
        GetWorld()->GetTimerManager().SetTimer(
            CleanupTimer,
            this,
            &UAdvancedTimerComponent::FinalCleanup,
            0.5f,
            false
        );
    }
    
    UFUNCTION()
    void FinalCleanup()
    {
        UE_LOG(LogTemp, Log, TEXT("All phases completed"));
        // Final operations
    }
    
    // Cleanup all timers
    virtual void BeginDestroy() override
    {
        if (GetWorld())
        {
            FTimerManager& TimerManager = GetWorld()->GetTimerManager();
            TimerManager.ClearTimer(PrimaryTimer);
            TimerManager.ClearTimer(SecondaryTimer);
            TimerManager.ClearTimer(CleanupTimer);
        }
        Super::BeginDestroy();
    }
};
```

### 🔄 Frame-Accurate Operations

```cpp
// Source: CombatStateMachineComponent.cpp example

// Frame-based timing for 60fps combat
void UCombatStateMachineComponent::StartFrameBasedOperation()
{
    // Calculate frame duration
    FrameDuration = 1.0f / TargetFrameRate;  // 1/60 = 0.0167 seconds
    
    // Set timer for each frame
    GetWorld()->GetTimerManager().SetTimer(
        FrameTimer,
        this,
        &UCombatStateMachineComponent::ProcessFrame,
        FrameDuration,
        true  // Repeating every frame
    );
}

void UCombatStateMachineComponent::ProcessFrame()
{
    CurrentFrame++;
    
    // Check for state transitions based on frame counts
    if (const FCombatActionData* ActionData = GetCurrentActionData())
    {
        if (CurrentFrame >= ActionData->StartupFrames && CurrentState == ECombatState::Startup)
        {
            SetState(ECombatState::Active);
        }
        else if (CurrentFrame >= ActionData->StartupFrames + ActionData->ActiveFrames && CurrentState == ECombatState::Active)
        {
            SetState(ECombatState::Recovery);
        }
    }
}
```

## Adding New Abilities/Items

### 🚀 Creating New Gameplay Abilities

```cpp
// 1. Create new ability header file
// Source: GameplayAbility_NewAbility.h

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayAbility_NewAbility.generated.h"

UCLASS()
class EROEOREOREOR_API UGameplayAbility_NewAbility : public UGameplayAbility
{
    GENERATED_BODY()

public:
    UGameplayAbility_NewAbility();

    // Core GAS overrides
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
    virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const override;

protected:
    // Ability-specific properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "New Ability")
    float AbilityDuration = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "New Ability")
    float AbilityPower = 100.0f;
};

// 2. Add to character's ability grants
// Source: MyCharacter.cpp - PossessedBy function

void AMyCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

    if (AbilitySystemComponent)
    {
        AbilitySystemComponent->InitAbilityActorInfo(this, this);

        // Grant default abilities
        CachedDashAbilityHandle = AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(UGameplayAbility_Dash::StaticClass()));
        CachedBounceAbilityHandle = AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(UGameplayAbility_Bounce::StaticClass()));
        
        // Add new ability
        FGameplayAbilitySpecHandle NewAbilityHandle = AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(UGameplayAbility_NewAbility::StaticClass()));
    }
}

// 3. Add input binding
// Source: MyCharacter.cpp - SetupPlayerInputComponent

void AMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        // Existing bindings...
        
        // New ability binding
        EnhancedInputComponent->BindAction(NewAbilityAction, ETriggerEvent::Triggered, this, &AMyCharacter::NewAbility);
    }
}

// 4. Add input action function
void AMyCharacter::NewAbility(const FInputActionValue& Value)
{
    if (AbilitySystemComponent)
    {
        AbilitySystemComponent->TryActivateAbilityByClass(UGameplayAbility_NewAbility::StaticClass());
    }
}
```

### ⚔️ Adding New Combat Actions

```cpp
// 1. Add to gameplay tags (Config/Tags/GameplayTags.ini)
+GameplayTagList=(Tag="Combat.Action.Special.NewMove",DevComment="New special combat move")

// 2. Add to DT_BasicCombatActions.csv
NewSpecialMove,Combat.Action.Special.NewMove,"New Special Move",15,8,20,12,18,3,0.1,300,"Combat.Action.Light.Jab",true,"NewSpecialPrototype",true,"NewSpecialAoE"

// 3. Add to DT_AttackPrototypes.csv  
NewSpecialPrototype,Combat.Action.Special.NewMove,50,800,"(X=1.0,Y=0.0,Z=0.2)","Anim_NewSpecial","SFX_NewSpecial","FX_NewSpecial","[(ShapeType=Cone,ConeAngle=90,ConeRange=400,LocalOffset=(X=100,Y=0,Z=0),ActivationFrame=15,DeactivationFrame=23)]"

// 4. Add input binding to AMyCharacter
// Header file: Add input action property
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enhanced Input", meta = (AllowPrivateAccess = "true"))
TObjectPtr<UInputAction> NewSpecialAction;

// Implementation: Add input binding
void AMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    // ... existing bindings

    EnhancedInputComponent->BindAction(NewSpecialAction, ETriggerEvent::Triggered, this, &AMyCharacter::NewSpecialAttack);
}

// Implementation: Add input function
void AMyCharacter::NewSpecialAttack(const FInputActionValue& Value)
{
    if (CombatStateMachine)
    {
        FGameplayTag NewSpecialTag = FGameplayTag::RequestGameplayTag(FName("Combat.Action.Special.NewMove"));
        CombatStateMachine->TryStartAction(NewSpecialTag);
    }
}
```

### 📊 Adding New Attributes

```cpp
// 1. Add to UMyAttributeSet header
// Source: MyAttributeSet.h

// New attribute declaration
UPROPERTY(BlueprintReadOnly, Category = "NewCategory", EditAnywhere, ReplicatedUsing = OnRep_NewAttribute)
FGameplayAttributeData NewAttribute;
ATTRIBUTE_ACCESSORS(UMyAttributeSet, NewAttribute)

// Replication function
UFUNCTION()
virtual void OnRep_NewAttribute(const FGameplayAttributeData& OldValue);

// 2. Add to implementation file
// Source: MyAttributeSet.cpp

void UMyAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    // Existing attributes...
    
    // Add new attribute replication
    DOREPLIFETIME_CONDITION_NOTIFY(UMyAttributeSet, NewAttribute, COND_None, REPNOTIFY_Always);
}

void UMyAttributeSet::OnRep_NewAttribute(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UMyAttributeSet, NewAttribute, OldValue);
}

// 3. Handle in PostGameplayEffectExecute if needed
void UMyAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
    Super::PostGameplayEffectExecute(Data);
    
    // Handle new attribute changes
    if (Data.EvaluatedData.Attribute == GetNewAttributeAttribute())
    {
        // Custom logic for new attribute
        float NewValue = FMath::Clamp(GetNewAttribute(), 0.0f, 100.0f);
        SetNewAttribute(NewValue);
    }
}

// 4. Create GameplayEffect to modify new attribute
// Source: GameplayEffect_ModifyNewAttribute.h

UCLASS()
class EROEOREOREOR_API UGameplayEffect_ModifyNewAttribute : public UGameplayEffect
{
    GENERATED_BODY()

public:
    UGameplayEffect_ModifyNewAttribute();
};

// Implementation
UGameplayEffect_ModifyNewAttribute::UGameplayEffect_ModifyNewAttribute()
{
    DurationPolicy = EGameplayEffectDurationType::Instant;
    
    // Set up modifier for new attribute
    FGameplayModifierInfo ModifierInfo;
    ModifierInfo.Attribute = UMyAttributeSet::GetNewAttributeAttribute();
    ModifierInfo.ModifierOp = EGameplayModOp::Additive;
    ModifierInfo.ModifierMagnitude = FScalableFloat(10.0f); // Increase by 10
    
    Modifiers.Add(ModifierInfo);
}
```

## Component Integration Patterns

### 🔧 Safe Component Access

```cpp
// Source: Best practices for component access
#include "Components/ActorComponent.h"

template<typename T>
T* GetComponentSafely(AActor* Actor)
{
    if (!Actor)
    {
        return nullptr;
    }
    return Actor->FindComponentByClass<T>();
}

// Usage pattern
void SomeFunction(AActor* TargetActor)
{
    // Safe component access with template
    if (UCombatStateMachineComponent* CombatComp = GetComponentSafely<UCombatStateMachineComponent>(TargetActor))
    {
        CombatComp->TryStartAction(AttackTag);
    }
    
    if (UAbilitySystemComponent* ASC = GetComponentSafely<UAbilitySystemComponent>(TargetActor))
    {
        ASC->TryActivateAbilityByClass(AbilityClass);
    }
}
```

### 🎯 Component Communication

```cpp
// Source: Inter-component communication pattern

// From one component to another on same actor
void UCombatPrototypeComponent::NotifyAttackShapeComponent()
{
    if (UAttackShapeComponent* AttackShape = GetOwner()->FindComponentByClass<UAttackShapeComponent>())
    {
        AttackShape->ActivateShape(CurrentAttackData.AttackShapes[0]);
    }
}

// From component to character
void UMyCustomComponent::NotifyCharacter()
{
    if (AMyCharacter* Character = Cast<AMyCharacter>(GetOwner()))
    {
        Character->SomeCharacterFunction();
    }
}
```

## Debug and Testing Utilities

### 🔍 Debug Console Commands

```cpp
// Source: Console command setup in any class

// Register console commands for debugging
static FAutoConsoleCommand DebugCombatCommand(
    TEXT("Debug.Combat.PrintState"),
    TEXT("Print current combat state for player character"),
    FConsoleCommandDelegate::CreateLambda([]()
    {
        if (UWorld* World = GEngine->GetWorldFromContextObject(GEngine, EGetWorldErrorMode::LogAndReturnNull))
        {
            if (AMyCharacter* Player = Cast<AMyCharacter>(World->GetFirstPlayerController()->GetPawn()))
            {
                if (UCombatStateMachineComponent* Combat = Player->FindComponentByClass<UCombatStateMachineComponent>())
                {
                    UE_LOG(LogTemp, Warning, TEXT("%s"), *Combat->GetDebugStateInfo());
                }
            }
        }
    })
);

// Register debug command for dash testing
static FAutoConsoleCommand DebugDashCommand(
    TEXT("Debug.Dash.TestParameters"),
    TEXT("Test current dash parameters"),
    FConsoleCommandDelegate::CreateLambda([]()
    {
        // Implementation for dash parameter testing
        UE_LOG(LogTemp, Warning, TEXT("Testing dash parameters..."));
    })
);
```

### 🧪 Runtime Testing Functions

```cpp
// Source: MyCharacter.cpp - Testing utilities

void AMyCharacter::TestCombatSystem()
{
    UE_LOG(LogTemp, Warning, TEXT("=== COMBAT SYSTEM TEST ==="));
    
    // Test combat component
    if (CombatStateMachine)
    {
        UE_LOG(LogTemp, Warning, TEXT("Combat State: %s"), *UEnum::GetValueAsString(CombatStateMachine->GetCurrentState()));
        UE_LOG(LogTemp, Warning, TEXT("Loaded Actions: %d"), CombatStateMachine->GetLoadedActionCount());
        
        // Test action execution
        FGameplayTag TestTag = FGameplayTag::RequestGameplayTag(FName("Combat.Action.Light.Jab"));
        bool bCanExecute = CombatStateMachine->CanStartAction(TestTag);
        UE_LOG(LogTemp, Warning, TEXT("Can execute light jab: %s"), bCanExecute ? TEXT("Yes") : TEXT("No"));
    }
    
    // Test GAS integration
    if (AbilitySystemComponent)
    {
        FGameplayTagContainer ActiveTags;
        AbilitySystemComponent->GetOwnedGameplayTags(ActiveTags);
        UE_LOG(LogTemp, Warning, TEXT("Active Tags: %s"), *ActiveTags.ToString());
    }
    
    // Test attributes
    if (AttributeSet)
    {
        UE_LOG(LogTemp, Warning, TEXT("Health: %.1f/%.1f"), AttributeSet->GetHealth(), AttributeSet->GetMaxHealth());
        UE_LOG(LogTemp, Warning, TEXT("Air Bounces: %.0f"), AttributeSet->GetAirBounceCount());
    }
}

void AMyCharacter::TestDash()
{
    UE_LOG(LogTemp, Warning, TEXT("=== DASH SYSTEM TEST ==="));
    
    if (AbilitySystemComponent)
    {
        bool bActivated = AbilitySystemComponent->TryActivateAbilityByClass(UGameplayAbility_Dash::StaticClass());
        UE_LOG(LogTemp, Warning, TEXT("Dash activation: %s"), bActivated ? TEXT("Success") : TEXT("Failed"));
    }
}

void AMyCharacter::TestBounce()
{
    UE_LOG(LogTemp, Warning, TEXT("=== BOUNCE SYSTEM TEST ==="));
    
    if (AbilitySystemComponent && AttributeSet)
    {
        UE_LOG(LogTemp, Warning, TEXT("Current air bounces: %.0f/2"), AttributeSet->GetAirBounceCount());
        
        bool bActivated = AbilitySystemComponent->TryActivateAbilityByClass(UGameplayAbility_Bounce::StaticClass());
        UE_LOG(LogTemp, Warning, TEXT("Bounce activation: %s"), bActivated ? TEXT("Success") : TEXT("Failed"));
    }
}
```

## Include Order Templates

### Standard Include Order
Always follow this exact order:
1. CoreMinimal.h (ALWAYS FIRST)
2. Parent class header (e.g., GameplayAbility.h)
3. Engine headers (Engine/*, Components/*)
4. Plugin headers (GameplayTagContainer.h, etc.)
5. Project headers (custom classes)
6. .generated.h (ALWAYS LAST)

### Gameplay Ability Template
```cpp
#include "CoreMinimal.h"                    // ALWAYS FIRST
#include "Abilities/GameplayAbility.h"      // Parent class
#include "GameplayTagContainer.h"           // GAS integration
#include "Engine/TimerHandle.h"             // Timer usage
#include "MyGameplayAbility.generated.h"    // ALWAYS LAST

// Forward declarations (to minimize includes)
class AMyCharacter;
class UCurveFloat;
```

### Component Template  
```cpp
#include "CoreMinimal.h"                    // ALWAYS FIRST
#include "Components/ActorComponent.h"      // Parent class
#include "GameplayTagContainer.h"           // If using tags
#include "MyComponent.generated.h"          // ALWAYS LAST

// Forward declarations
class AMyCharacter;
```

### Actor Template
```cpp
#include "CoreMinimal.h"                    // ALWAYS FIRST
#include "GameFramework/Actor.h"            // Parent class
#include "GameplayTagContainer.h"           // If implementing tag interface
#include "MyActor.generated.h"              // ALWAYS LAST

// Forward declarations
class UBoxComponent;
class UStaticMeshComponent;
```

### Critical Dependency Self-Check
Before completing any code implementation, add this comment:
```cpp
// Dependency Check:
// ✓ CoreMinimal.h first
// ✓ Parent class include second  
// ✓ Required modules in Build.cs (GameplayTags, GameplayAbilities, etc.)
// ✓ Forward declared classes where possible (performance optimization)
// ✓ .generated.h last
// ✓ All gameplay tags registered in GameplayTags.ini
// ✓ Documentation updated
```

## Essential Include Patterns

### 📦 Common Include Sets

```cpp
// Gameplay Ability System includes
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffect.h"
#include "GameplayAbility.h"
#include "AttributeSet.h"
#include "GameplayTagContainer.h"

// Enhanced Input includes
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"

// Camera system includes  
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

// Common Unreal includes
#include "Engine/World.h"
#include "Engine/TimerManager.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Components/ActorComponent.h"

// Project-specific includes
#include "MyCharacter.h"
#include "MyAttributeSet.h"
#include "CombatSystemTypes.h"
#include "CombatStateMachineComponent.h"
```

### 🎯 Forward Declaration Patterns

```cpp
// Header file forward declarations (to reduce compile dependencies)
class UAbilitySystemComponent;
class UMyAttributeSet;
class UCombatStateMachineComponent;
class UVelocitySnapshotComponent;
class UAttackShapeComponent;
class UInputMappingContext;
class UInputAction;
class USpringArmComponent;
class UCameraComponent;

// Use full includes only in .cpp files when you need the complete class definition
```
