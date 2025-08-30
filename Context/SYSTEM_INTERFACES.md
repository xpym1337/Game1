# SYSTEM INTERFACES

## Delegate Declarations and Event System

### 🎯 Combat System Delegates

Located in `CombatStateMachineComponent.h`:

```cpp
// Combat state transitions
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
    FOnCombatStateChanged, 
    ECombatState, OldState, 
    ECombatState, NewState, 
    const FGameplayTag&, ActionTag
);

// Combat action lifecycle
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FOnCombatActionStarted, 
    const FGameplayTag&, ActionTag, 
    const FCombatActionData&, ActionData
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FOnCombatActionEnded, 
    const FGameplayTag&, ActionTag, 
    bool, bWasCanceled
);

// Perfect cancel system
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FOnPerfectCancel, 
    const FGameplayTag&, CanceledIntoAction
);

// Combo system tracking
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FOnComboUpdated, 
    int32, ComboCount, 
    const TArray<FGameplayTag>&, ComboChain
);

// Hidden combo execution
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FOnHiddenComboExecuted, 
    const FString&, ComboName
);
```

### 🏃 Movement System Delegates

Located in `MyCharacter.h` (uses built-in delegates):

```cpp
// Built-in ACharacter delegate for landing detection
// ACharacter::LandedDelegate (inherited from ACharacter)
virtual void Landed(const FHitResult& Hit) override;

// This broadcasts to:
// - GameplayEffect_AirBounceReset (resets air bounce counter)
// - Any Blueprint listeners for landing events
// - VelocitySnapshotComponent for velocity restoration
```

### 🎮 Gameplay Ability System Integration

GAS provides built-in event system through:

```cpp
// Ability activation events (built into GAS)
UAbilitySystemComponent* ASC = GetAbilitySystemComponent();

// Attribute change callbacks (handled by UMyAttributeSet)
virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

// Replication callbacks for each attribute
UFUNCTION()
virtual void OnRep_Health(const FGameplayAttributeData& OldValue);
// ... (similar for all attributes)
```

## Major Function Call Flows

### ⚔️ Combat Action Flow

```cpp
// 1. Input System → Character → Combat Component
AMyCharacter::LightAttack() 
    ↓
UCombatStateMachineComponent::TryStartAction(ActionTag)
    ↓
// 2. State validation and action execution
UCombatStateMachineComponent::ExecuteAction(ActionTag)
    ↓
// 3. Component integration
UCombatPrototypeComponent::ExecutePrototype(PrototypeName)
    ↓ (parallel)
UAttackShapeComponent::ActivateShape(ShapeData)
    ↓
// 4. Hit detection and damage application
UGameplayEffect_Damage → Target's AbilitySystemComponent
    ↓
// 5. Event broadcasting
OnCombatActionStarted.Broadcast(ActionTag, ActionData)
```

### 🏃 Dash Ability Flow

```cpp
// 1. Enhanced Input → Character
AMyCharacter::DashLeft()/DashRight() 
    ↓
// 2. GAS ability activation
AbilitySystemComponent->TryActivateAbilityByClass(UGameplayAbility_Dash)
    ↓
// 3. Ability execution
UGameplayAbility_Dash::ActivateAbility()
    ↓
// 4. Velocity snapshot (for combos)
VelocitySnapshotComponent->CapturePreDashVelocity()
    ↓
// 5. Movement application
CharacterMovementComponent->Velocity = CalculatedDashVelocity
    ↓
// 6. Velocity restoration (post-dash)
VelocitySnapshotComponent->RestorePostDashVelocity()
```

### 🎯 Damage Application Flow

```cpp
// 1. Attack shape collision detection
UAttackShapeComponent::DetectTargetsInShape()
    ↓
// 2. For each target found
Target->GetAbilitySystemComponent()->ApplyGameplayEffectToSelf(DamageEffect)
    ↓
// 3. Damage calculation in AttributeSet
UMyAttributeSet::PostGameplayEffectExecute()
    ↓
// 4. Health attribute modification
UMyAttributeSet::OnRep_Health() [if networked]
    ↓
// 5. UI/Visual updates through attribute binding
```

### 🏀 Air Bounce System Flow

```cpp
// 1. Bounce input while airborne
AMyCharacter::Bounce()
    ↓
// 2. GAS ability activation with attribute check
UGameplayAbility_Bounce::CanActivateAbility() 
    // Checks AirBounceCount < MaxAirBounces
    ↓
// 3. Ability execution
UGameplayAbility_Bounce::ActivateAbility()
    ↓
// 4. Increment air bounce counter
ApplyGameplayEffect(GameplayEffect_AirBounceIncrement)
    ↓
// 5. Velocity application
CharacterMovementComponent->Launch(BounceVelocity)
    ↓
// 6. Landing detection (eventually)
ACharacter::Landed() → GameplayEffect_AirBounceReset
```

## Subsystem Access Patterns

### 🌐 Global Subsystem Access

```cpp
// Accessing subsystems from anywhere in the codebase
UWorld* World = GetWorld();

// Enhanced Input Subsystem
if (UEnhancedInputLocalPlayerSubsystem* Subsystem = 
    ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
{
    Subsystem->AddMappingContext(DefaultMappingContext, 0);
}

// Asset Manager (for loading curves in GameplayAbility_Dash)
UAssetManager& AssetManager = UAssetManager::Get();
FStreamableManager& StreamableManager = AssetManager.GetStreamableManager();

// Timer Manager (used extensively in abilities)
FTimerManager& TimerManager = GetWorld()->GetTimerManager();
TimerManager.SetTimer(VelocityUpdateTimer, this, &UGameplayAbility_Dash::UpdateDashVelocity, UpdateRate, true);
```

### 🎯 Component Access Patterns

```cpp
// Safe component access with null checks
if (UCombatStateMachineComponent* CombatStateMachine = GetOwner()->FindComponentByClass<UCombatStateMachineComponent>())
{
    CombatStateMachine->TryStartAction(AttackTag);
}

// Cached component references (performance pattern)
// In AMyCharacter constructor:
CombatStateMachine = CreateDefaultSubobject<UCombatStateMachineComponent>(TEXT("CombatStateMachine"));

// Usage throughout class:
CombatStateMachine->GetCurrentState() // Fast access, no lookup
```

### 🎮 GAS Component Access

```cpp
// From any actor implementing IAbilitySystemInterface
UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);

// From character specifically
AMyCharacter* Character = Cast<AMyCharacter>(GetOwner());
UMyAttributeSet* AttributeSet = Character->GetMyAttributeSet();

// Ability activation patterns
FGameplayAbilitySpecHandle AbilityHandle = ASC->GiveAbility(FGameplayAbilitySpec(AbilityClass));
ASC->TryActivateAbility(AbilityHandle);
```

## Event Broadcast/Binding Examples

### ⚔️ Combat Event Bindings

```cpp
// In Blueprint or C++, binding to combat events:

// Combat state changes
CombatStateMachineComponent->OnStateChanged.AddDynamic(this, &AMyCharacter::HandleCombatStateChanged);

// Perfect cancel notifications (for UI effects)
CombatStateMachineComponent->OnPerfectCancel.AddDynamic(this, &UUI_CombatHUD::ShowPerfectCancelEffect);

// Combo updates (for UI display)
CombatStateMachineComponent->OnComboUpdated.AddDynamic(this, &UUI_ComboCounter::UpdateComboDisplay);

// Example handler implementation:
UFUNCTION()
void AMyCharacter::HandleCombatStateChanged(ECombatState OldState, ECombatState NewState, const FGameplayTag& ActionTag)
{
    // Update animation state
    // Trigger visual effects
    // Update UI state
    // Log for debugging
}
```

### 🏃 Movement Event Bindings

```cpp
// Landing event binding (using built-in delegate)
void AMyCharacter::BeginPlay()
{
    Super::BeginPlay();
    
    // Built-in ACharacter landing delegate
    LandedDelegate.AddDynamic(this, &AMyCharacter::OnCharacterLanded);
}

UFUNCTION()
void AMyCharacter::OnCharacterLanded(const FHitResult& Hit)
{
    // Reset air bounces
    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
    {
        ASC->ApplyGameplayEffectToSelf(AirBounceResetEffect);
    }
    
    // Restore velocity for dash-bounce combos
    if (VelocitySnapshotComponent)
    {
        VelocitySnapshotComponent->HandleLanding();
    }
}
```

### 🎯 GAS Attribute Bindings

```cpp
// Binding to attribute changes (typically in UI widgets)
void UUI_HealthBar::BindToAttributeSet(UMyAttributeSet* AttributeSet)
{
    if (UAbilitySystemComponent* ASC = AttributeSet->GetAbilitySystemComponent())
    {
        // Bind to health changes
        ASC->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetHealthAttribute())
            .AddUObject(this, &UUI_HealthBar::HandleHealthChanged);
        
        // Bind to max health changes
        ASC->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetMaxHealthAttribute())
            .AddUObject(this, &UUI_HealthBar::HandleMaxHealthChanged);
    }
}

void UUI_HealthBar::HandleHealthChanged(const FOnAttributeChangeData& Data)
{
    UpdateHealthBar(Data.NewValue, GetMaxHealth());
}
```

## Communication Patterns Between Systems

### 🔄 System Integration Map

```
Enhanced Input System
    ↓ [Input Events]
AMyCharacter (Central Hub)
    ↓ [Method Calls]
├── AbilitySystemComponent (GAS)
│   ↓ [Ability Activation]
│   ├── GameplayAbility_Dash
│   └── GameplayAbility_Bounce
│       ↓ [Velocity Changes]
│       └── CharacterMovementComponent
├── CombatStateMachineComponent
│   ↓ [Action Execution]
│   ├── CombatPrototypeComponent
│   └── AttackShapeComponent
│       ↓ [Collision Detection]
│       └── Target Actors
│           ↓ [Damage Application]
│           └── Target's AbilitySystemComponent
└── VelocitySnapshotComponent
    ↓ [Velocity Coordination]
    └── CharacterMovementComponent
```

### 🎭 Observer Pattern Implementation

```cpp
// Combat system uses observer pattern through delegates
class UCombatObserver : public UObject
{
public:
    UFUNCTION()
    void OnCombatStateChanged(ECombatState OldState, ECombatState NewState, const FGameplayTag& ActionTag);
};

// Multiple observers can bind to the same events
CombatStateMachine->OnStateChanged.AddDynamic(UIManager, &UUIManager::HandleCombatStateChanged);
CombatStateMachine->OnStateChanged.AddDynamic(AudioManager, &UAudioManager::HandleCombatStateChanged);
CombatStateMachine->OnStateChanged.AddDynamic(ParticleManager, &UParticleManager::HandleCombatStateChanged);
```

### 🔗 Component Communication Patterns

```cpp
// 1. Direct Reference Pattern (AMyCharacter)
UCombatStateMachineComponent* CombatStateMachine; // Direct owned component
CombatStateMachine->TryStartAction(ActionTag); // Direct method call

// 2. Component Lookup Pattern (when not owned)
UCombatStateMachineComponent* CombatComp = GetOwner()->FindComponentByClass<UCombatStateMachineComponent>();

// 3. Interface Pattern (for polymorphism)
if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(TargetActor))
{
    UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent();
}

// 4. Event-Driven Pattern (loose coupling)
OnActionCompleted.Broadcast(ActionResult); // Publisher
// Multiple subscribers can listen without tight coupling

// 5. Mediator Pattern (via GAS)
// Components don't talk directly; they go through GameplayEffects
ApplyGameplayEffectToTarget(Effect, Target); // Mediates communication
```

## Performance Considerations

### ⚡ High-Performance Communication

```cpp
// Fast paths for performance-critical operations:

// 1. Cached component references (avoid repeated lookups)
UPROPERTY(Transient)
TWeakObjectPtr<UCombatStateMachineComponent> CachedCombatComponent;

// 2. Direct function calls over delegates for critical paths
// Use delegates for events, direct calls for frame-critical operations

// 3. Batch operations where possible
TArray<AActor*> TargetsToProcess;
// Process all targets in one batch rather than individual calls
```

### 🐌 Potential Performance Bottlenecks

```cpp
// 1. Multicast delegate broadcasts (many listeners)
OnComboUpdated.Broadcast(ComboCount, ComboChain); // Could be expensive with many UI elements

// 2. Component lookups every frame
// DON'T DO: GetOwner()->FindComponentByClass<UCombatComponent>() in Tick
// DO: Cache the reference in BeginPlay

// 3. String-based operations in hot paths
// Gameplay tags are more efficient than string comparisons
FGameplayTag ActionTag vs FString ActionName

// 4. Excessive collision queries
// UAttackShapeComponent batches collision checks efficiently
```

## Integration Points for New Features

### 🎯 Adding New Combat Actions

```cpp
// 1. Add data to DT_BasicCombatActions.csv
// 2. Add corresponding gameplay tag to GameplayTags.ini
// 3. Bind input in AMyCharacter::SetupPlayerInputComponent()
// 4. Combat system automatically handles the rest via data-driven design

// Example new action integration:
void AMyCharacter::SpecialAttack(const FInputActionValue& Value)
{
    if (CombatStateMachine && CombatStateMachine->CanStartAction(TAG_Combat_Action_Special_MyNewAttack))
    {
        CombatStateMachine->TryStartAction(TAG_Combat_Action_Special_MyNewAttack);
    }
}
```

### 🏃 Adding New Movement Abilities

```cpp
// 1. Create new UGameplayAbility subclass
// 2. Register with AbilitySystemComponent in AMyCharacter::PossessedBy()
// 3. Add input binding in SetupPlayerInputComponent()
// 4. Optionally integrate with VelocitySnapshotComponent for combos

// Example:
FGameplayAbilitySpecHandle WallRunAbility = ASC->GiveAbility(FGameplayAbilitySpec(UGameplayAbility_WallRun::StaticClass()));
```

### 📊 Adding New Attributes

```cpp
// 1. Add to UMyAttributeSet with proper macros
UPROPERTY(BlueprintReadOnly, Category = "NewCategory", ReplicatedUsing = OnRep_NewAttribute)
FGameplayAttributeData NewAttribute;
ATTRIBUTE_ACCESSORS(UMyAttributeSet, NewAttribute);

// 2. Add replication function
UFUNCTION()
virtual void OnRep_NewAttribute(const FGameplayAttributeData& OldValue);

// 3. Add to GetLifetimeReplicatedProps
// 4. Handle in PostGameplayEffectExecute if needed
```

## Debugging and Monitoring

### 🔍 Debug Interface Access

```cpp
// Console commands for debugging system interfaces:

// Combat system debugging
CombatStateMachine->SetDebugVisualization(true);
CombatStateMachine->PrintCurrentState();

// GAS debugging (built-in)
showdebug abilitysystem
showdebug gameplaytags

// Attack shape visualization
AttackShapeComponent->SetDebugDrawEnabled(true);

// Performance monitoring
stat game
stat gameplaytags
stat abilitysystem
```

### 📝 Event Logging Pattern

```cpp
// Standardized event logging for system integration:
DECLARE_LOG_CATEGORY_EXTERN(LogCombatSystem, Log, All);

#define COMBAT_LOG(Verbosity, Format, ...) \
    UE_LOG(LogCombatSystem, Verbosity, Format, ##__VA_ARGS__)

// Usage in delegates:
void UCombatStateMachineComponent::OnStateChanged_Internal(ECombatState NewState)
{
    COMBAT_LOG(Log, TEXT("Combat state changed to %s"), *UEnum::GetValueAsString(NewState));
    OnStateChanged.Broadcast(CurrentState, NewState, CurrentActionTag);
}
