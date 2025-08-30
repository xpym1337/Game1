# DATA ARCHITECTURE

## DataTable Catalog

### 📊 Combat System DataTables

#### `Content/Data/Combat/DT_BasicCombatActions.csv`
**Purpose**: Frame-accurate combat action definitions with 60fps timing baseline

**Structure**: Based on `FCombatActionData` struct
**Location**: `Source/EROEOREOREOR/CombatSystemTypes.h`

| Column | Type | Purpose | Example Values |
|--------|------|---------|----------------|
| `ActionTag` | FGameplayTag | Unique identifier | `Combat.Actions.Attack.Light.Jab` |
| `DisplayName` | FString | UI display name | `"Light Jab"` |
| `StartupFrames` | int32 | Wind-up frames before active | `8` (0.133s at 60fps) |
| `ActiveFrames` | int32 | Hit detection frames | `4` (0.067s at 60fps) |
| `RecoveryFrames` | int32 | Cool-down frames after active | `12` (0.2s at 60fps) |
| `CancelWindowStart` | int32 | First frame canceling available | `6` |
| `CancelWindowEnd` | int32 | Last frame canceling possible | `10` |
| `PriorityLevel` | ECombatPriority | Interrupt priority (0-4) | `0` (Light), `1` (Heavy), etc. |
| `MovementSpeedMultiplier` | float | Movement speed during action | `0.2` (20% speed) |
| `Range` | float | Maximum effective range | `150.0` (UE units) |
| `CanCancelInto` | TArray<FGameplayTag> | Actions this can cancel into | `Combat.Actions.Attack.Light.Cross` |
| `bUseCombatPrototype` | bool | Use prototype system | `true` |
| `CombatPrototypeName` | FString | Which prototype to execute | `"LightJab"` |
| `bTriggerAoE` | bool | Trigger AoE effect | `false` |
| `AoEPrototypeName` | FString | Which AoE prototype | `"HeavyImpact"` |

**Current Actions**:
- **Light Attacks**: Jab (8f startup), Cross (10f startup), Hook (12f startup)
- **Heavy Attacks**: Straight (18f startup), Uppercut (22f startup)  
- **Movement Attacks**: DashAttack (6f startup, high movement speed)

#### `Content/Data/Combat/DT_AttackPrototypes.csv`
**Purpose**: Enhanced attack definitions with multiple hitbox support

**Structure**: Based on `FAttackPrototypeData` struct
**Location**: `Source/EROEOREOREOR/CombatSystemTypes.h`

| Column | Type | Purpose | Example Values |
|--------|------|---------|----------------|
| `AttackName` | FString | Attack identifier | `"QuickJab"` |
| `AttackTag` | FGameplayTag | Gameplay tag reference | `Combat.Actions.Attack.Light.Jab` |
| `BaseDamage` | float | Base damage value | `15.0` |
| `Knockback` | float | Knockback force | `200.0` |
| `KnockbackDirection` | FVector | Knockback direction | `(X=1.0,Y=0.0,Z=0.1)` |
| `PlaceholderAnimationName` | FString | Animation placeholder | `"Anim_LightPunch"` |
| `PlaceholderSoundEffect` | FString | Audio placeholder | `"SFX_QuickHit"` |
| `PlaceholderParticleEffect` | FString | VFX placeholder | `"FX_LightImpact"` |
| `AttackShapes` | TArray<FAttackShapeData> | Hitbox definitions | Complex shape data array |

**Current Prototypes**:
- **QuickJab**: Single sphere hitbox, fast and precise
- **SweepingHook**: Arc-shaped hitbox covering 120° angle
- **HeavySmash**: Box hitbox for wide coverage
- **UppercutLaunch**: Capsule hitbox angled upward
- **SpearThrust**: Line hitbox for precise forward strikes
- **GroundPound**: Ring hitbox around character
- **FlameBreath**: Cone hitbox with multi-hit capability
- **ComboFinisher**: Multiple hitboxes (sphere + ring) for ultimate attack

## UDataAsset Derived Classes

### 📦 Current State
**Note**: The project currently uses **DataTables** (CSV-based) rather than **UDataAsset** classes for data-driven design.

**Potential Migration Path** (for future):
```cpp
// Future UDataAsset implementation could replace CSV files:
UCLASS(BlueprintType)
class EROEOREOREOR_API UCombatActionDataAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Action")
    FCombatActionData ActionData;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Prototype")
    FAttackPrototypeData AttackPrototype;
};
```

**Current Advantages of DataTable Approach**:
- Easy to edit in external tools (Excel, Google Sheets)
- Version control friendly (text-based diffs)
- Can be modified without recompiling
- Bulk import/export capabilities

## Runtime Data Structures

### 🎯 Combat Data Structures

#### `FCombatActionData` (extends FTableRowBase)
```cpp
// Source: CombatSystemTypes.h
struct EROEOREOREOR_API FCombatActionData : public FTableRowBase
{
    // Identity and display
    FGameplayTag ActionTag;                    // Unique identifier
    FString DisplayName;                       // UI display name
    
    // Frame timing (60fps baseline)
    int32 StartupFrames;                       // Wind-up time
    int32 ActiveFrames;                        // Hit detection time
    int32 RecoveryFrames;                      // Cool-down time
    
    // Cancel system
    TArray<FGameplayTag> CanCancelInto;        // Valid cancel targets
    int32 CancelWindowStart;                   // Cancel timing window
    int32 CancelWindowEnd;
    
    // Priority and movement
    ECombatPriority PriorityLevel;             // Interrupt priority
    float MovementSpeedMultiplier;             // Movement during action
    float Range;                               // Effective range
    
    // Integration with prototype system
    bool bUseCombatPrototype;                  // Use prototype execution
    FString CombatPrototypeName;               // Which prototype
    bool bTriggerAoE;                          // Trigger AoE effect
    FString AoEPrototypeName;                  // Which AoE prototype
    
    // Helper functions for frame calculations
    FORCEINLINE float GetTotalDurationSeconds() const;
    FORCEINLINE bool IsInCancelWindow(int32 CurrentFrame) const;
};
```

#### `FAttackShapeData` (nested in FAttackPrototypeData)
```cpp
// Attack hitbox definition with multiple shape support
struct EROEOREOREOR_API FAttackShapeData
{
    // Shape configuration
    EAttackShape ShapeType;                    // Sphere, Box, Cone, etc.
    float PrimarySize;                         // Main dimension
    float SecondarySize;                       // Height/depth
    float TertiarySize;                        // Third dimension
    
    // Positioning
    FVector LocalOffset;                       // Relative to character
    FRotator LocalRotation;                    // Shape orientation
    
    // Shape-specific parameters
    float ConeAngle;                           // For cone/arc shapes
    float ConeRange;                           // Cone/arc range
    float InnerRadius;                         // For ring shapes
    float OuterRadius;                         // For ring shapes
    
    // Timing integration
    int32 ActivationFrame;                     // When shape becomes active
    int32 DeactivationFrame;                   // When shape deactivates
    
    // Multi-hit system
    bool bAllowMultiHit;                       // Can hit same target multiple times
    int32 MaxHitsPerTarget;                    // Maximum hits per target
    float MultihitInterval;                    // Time between hits
    
    // Debug visualization
    bool bShowDebugShape;                      // Show debug wireframe
    FColor DebugColor;                         // Debug color
    float DebugDrawTime;                       // How long to show
    float DebugLineThickness;                  // Debug line thickness
};
```

#### `FHiddenComboData` (extends FTableRowBase)
```cpp
// Secret combo sequence definitions
struct EROEOREOREOR_API FHiddenComboData : public FTableRowBase
{
    // Combo requirements
    TArray<FGameplayTag> RequiredSequence;     // Input sequence required
    FString ComboName;                         // Display name
    
    // Timing constraints
    float MaxTimeBetweenInputs;                // Input timing window
    bool bRequiresPerfectTiming;               // Strict timing requirement
    
    // Rewards
    float BonusDamageMultiplier;               // Damage bonus
    float BonusStylePoints;                    // Style system integration
    FGameplayTag SpecialEffectTag;             // Special effect trigger
};
```

### 🎮 GAS Integration Structures

#### `UMyAttributeSet` Attributes
```cpp
// Source: MyAttributeSet.h
// All attributes are FGameplayAttributeData with replication support

// Core survival attributes
FGameplayAttributeData Health;                // Current health
FGameplayAttributeData MaxHealth;             // Maximum health capacity
FGameplayAttributeData Stamina;               // Current stamina  
FGameplayAttributeData MaxStamina;            // Maximum stamina capacity

// Movement system integration
FGameplayAttributeData AirBounceCount;        // Current air bounces performed

// Combat system attributes
FGameplayAttributeData AttackPower;           // Base damage multiplier
FGameplayAttributeData CriticalHitChance;     // Crit chance (0.0-1.0)
FGameplayAttributeData CriticalHitMultiplier; // Crit damage multiplier
FGameplayAttributeData AttackSpeed;           // Attack speed multiplier

// Resistance system
FGameplayAttributeData PhysicalResistance;    // Physical damage reduction
FGameplayAttributeData ElementalResistance;   // Elemental damage reduction

// Meta attributes (for calculations, not stored)
FGameplayAttributeData IncomingDamage;        // Damage calculation helper
FGameplayAttributeData IncomingHealing;       // Healing calculation helper
```

### 🏃 Movement System Structures

#### Velocity Preservation (VelocitySnapshotComponent)
```cpp
// Source: VelocitySnapshotComponent.h
// Runtime structures for dash-bounce combo system

struct FVelocitySnapshot
{
    FVector PreDashVelocity;                   // Velocity before dash
    FVector DashVelocity;                      // Velocity during dash
    float CaptureTimestamp;                    // When captured
    bool bIsValid;                             // Snapshot validity
};

// Internal state tracking
TArray<FVelocitySnapshot> VelocityHistory;     // Velocity timeline
float ComboWindowDuration;                     // Time window for combos
int32 MaxSnapshots;                            // Memory management
```

### 🔧 Component State Structures

#### Combat State Machine Internal Data
```cpp
// Source: CombatStateMachineComponent.h
// Runtime state tracking for frame-accurate combat

// State tracking
ECombatState CurrentState;                     // Current state machine state
FGameplayTag CurrentActionTag;                 // Currently executing action
int32 CurrentFrame;                            // Current frame in action
float StateElapsedTime;                        // Time in current state

// Input buffer system
TArray<FGameplayTag> InputBuffer;              // Buffered inputs
TArray<float> InputTimestamps;                 // Input timing tracking

// Combo tracking
TArray<FGameplayTag> CurrentComboChain;        // Current combo sequence
float TimeSinceLastAction;                     // Combo timing

// Data storage
TMap<FGameplayTag, FCombatActionData> LoadedActions;        // Cached action data
TMap<FString, FHiddenComboData> LoadedHiddenCombos;        // Cached combo data
```

## Data Flow Examples

### ⚔️ Combat Data Flow

```cpp
// 1. Designer edits DT_BasicCombatActions.csv in external tool
"LightAttack1,Combat.Actions.Attack.Light.Jab,Light Jab,8,4,12,..."

// 2. Data loaded at runtime in UCombatStateMachineComponent
void UCombatStateMachineComponent::LoadActionData(UDataTable* ActionDataTable)
{
    TArray<FCombatActionData*> AllRows;
    ActionDataTable->GetAllRows<FCombatActionData>(TEXT("LoadActionData"), AllRows);
    
    for (const FCombatActionData* Row : AllRows)
    {
        LoadedActions.Add(Row->ActionTag, *Row);
    }
}

// 3. Runtime access during combat
const FCombatActionData* ActionData = LoadedActions.Find(ActionTag);
if (ActionData && ActionData->StartupFrames > 0)
{
    // Use frame data for timing calculations
    float StartupDuration = ActionData->GetStartupDurationSeconds();
}
```

### 🎯 Attack Shape Data Flow

```cpp
// 1. Complex shape data stored in DT_AttackPrototypes.csv
"QuickJab,...,[(ShapeType=Sphere,PrimarySize=80,LocalOffset=(X=120,Y=0,Z=0),ActivationFrame=8,DeactivationFrame=10)]"

// 2. Parsed into FAttackShapeData array
TArray<FAttackShapeData> AttackShapes = AttackPrototype.AttackShapes;

// 3. Used by AttackShapeComponent for collision detection
for (const FAttackShapeData& ShapeData : AttackShapes)
{
    if (CurrentFrame >= ShapeData.ActivationFrame && CurrentFrame <= ShapeData.DeactivationFrame)
    {
        // Generate collision shape and detect hits
        PerformShapeCollision(ShapeData);
    }
}
```

### 📊 Attribute Modification Flow

```cpp
// 1. GameplayEffect created with attribute modification
UGameplayEffect_Damage* DamageEffect = NewObject<UGameplayEffect_Damage>();

// 2. Applied to target's AbilitySystemComponent
TargetASC->ApplyGameplayEffectToSelf(DamageEffect);

// 3. Processed in UMyAttributeSet
void UMyAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
    if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
    {
        // Calculate final damage value
        float DamageValue = Data.EvaluatedData.Magnitude;
        
        // Apply resistances
        DamageValue *= (1.0f - GetPhysicalResistance());
        
        // Modify health attribute
        SetHealth(FMath::Clamp(GetHealth() - DamageValue, 0.0f, GetMaxHealth()));
    }
}

// 4. Replication to clients (if networked)
void UMyAttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UMyAttributeSet, Health, OldValue);
}
```

## Runtime Data Storage Patterns

### 🗃️ Data Caching Strategies

#### Component-Level Caching
```cpp
// CombatStateMachineComponent caches DataTable data for performance
class UCombatStateMachineComponent
{
private:
    // Cached for fast runtime access
    UPROPERTY(Transient)
    TMap<FGameplayTag, FCombatActionData> LoadedActions;
    
    UPROPERTY(Transient)
    TMap<FString, FHiddenComboData> LoadedHiddenCombos;
    
    // Direct DataTable references for reloading
    UPROPERTY(EditAnywhere, Category = "Configuration")
    UDataTable* DefaultActionDataTable;
    
    UPROPERTY(EditAnywhere, Category = "Configuration")
    UDataTable* DefaultHiddenComboDataTable;
};
```

#### Character-Level Caching
```cpp
// AMyCharacter caches ability handles for performance
class AMyCharacter
{
private:
    // PERFORMANCE: Cache ability handles to avoid lookup delays
    UPROPERTY(Transient)
    FGameplayAbilitySpecHandle CachedDashAbilityHandle;
    
    UPROPERTY(Transient)
    FGameplayAbilitySpecHandle CachedBounceAbilityHandle;
};
```

### 💾 Data Persistence Patterns

#### Save Game Data Structure (Implied)
```cpp
// Future save game structure based on current attributes
USTRUCT(BlueprintType)
struct FPlayerSaveData
{
    GENERATED_BODY()
    
    // Persistent character attributes
    float SavedHealth;
    float SavedMaxHealth;
    float SavedStamina;
    float SavedMaxStamina;
    
    // Progression data
    float SavedAttackPower;
    float SavedCriticalHitChance;
    float SavedAttackSpeed;
    
    // Unlocked abilities/combos
    TArray<FGameplayTag> UnlockedAbilities;
    TArray<FString> UnlockedHiddenCombos;
    
    // Settings/preferences
    float DashSpeed;
    float BounceUpwardVelocity;
    int32 MaxAirBounces;
};
```

#### Runtime Session Data
```cpp
// Temporary data that doesn't persist across sessions
struct FRuntimeGameplayData
{
    // Current session combat stats
    int32 TotalDamageDealt;
    int32 TotalCombosPerformed;
    int32 PerfectCancelsExecuted;
    
    // Current session movement stats
    int32 DashesPerformed;
    int32 AirBouncesPerformed;
    int32 DashBounceComboCount;
    
    // Style rating progression
    EStyleRating CurrentStyleRating;
    float StylePoints;
    TArray<FGameplayTag> RecentActionSequence;
};
```

## Data Validation and Safety

### 🛡️ Data Integrity Checks

#### DataTable Validation
```cpp
// Built-in validation in CombatSystemTypes.h helper functions
const FCombatActionData* ActionData = GetActionData(ActionTag);
if (ActionData)
{
    // Frame timing validation
    ensureAlways(ActionData->StartupFrames > 0);
    ensureAlways(ActionData->ActiveFrames > 0);
    ensureAlways(ActionData->RecoveryFrames > 0);
    
    // Cancel window validation
    ensureAlways(ActionData->CancelWindowStart >= 0);
    ensureAlways(ActionData->CancelWindowEnd <= ActionData->StartupFrames + ActionData->ActiveFrames);
    
    // Priority validation
    ensureAlways(ActionData->GetPriorityValue() >= 0 && ActionData->GetPriorityValue() <= 4);
}
```

#### Attribute Validation
```cpp
// Source: MyAttributeSet.cpp - PreAttributeChange
void UMyAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
    // Clamp attributes to valid ranges
    if (Attribute == GetHealthAttribute())
    {
        NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
    }
    else if (Attribute == GetAirBounceCountAttribute())
    {
        NewValue = FMath::Clamp(NewValue, 0.0f, 10.0f); // Reasonable max bounces
    }
    else if (Attribute == GetCriticalHitChanceAttribute())
    {
        NewValue = FMath::Clamp(NewValue, 0.0f, 1.0f); // 0-100% range
    }
}
```

### 🔍 Runtime Data Debugging

#### Data Table Debugging
```cpp
// Console commands for data inspection:
// Combat.DebugPrintActionData [ActionTag] - Print action frame data
// Combat.DebugPrintAllActions - Print all loaded actions
// Combat.ValidateDataTables - Run validation checks
// Combat.ReloadDataTables - Hot-reload CSV changes

// Code example:
UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
void UCombatStateMachineComponent::DebugPrintActionData(const FGameplayTag& ActionTag)
{
    if (const FCombatActionData* ActionData = LoadedActions.Find(ActionTag))
    {
        UE_LOG(LogCombatSystem, Warning, TEXT("Action: %s | Startup: %d | Active: %d | Recovery: %d"), 
            *ActionData->DisplayName, ActionData->StartupFrames, 
            ActionData->ActiveFrames, ActionData->RecoveryFrames);
    }
}
```

#### Attribute Debugging
```cpp
// GAS provides built-in attribute debugging:
// showdebug abilitysystem - Show all attributes for selected actor
// DebugAttribute [AttributeName] - Show specific attribute changes

// Custom debugging in UMyAttributeSet:
void UMyAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
    // Log attribute changes for debugging
    const FGameplayAttribute& Attribute = Data.EvaluatedData.Attribute;
    float NewValue = Data.EvaluatedData.Magnitude;
    
    UE_LOG(LogGAS, Log, TEXT("Attribute %s changed by %f (New Total: %f)"), 
        *Attribute.AttributeName, NewValue, 
        Attribute.GetNumericValue(this));
}
```

## Constants and Configuration

### 🎯 Combat System Constants
```cpp
// Source: CombatSystemTypes.h - CombatConstants namespace
namespace CombatConstants
{
    constexpr float TARGET_FRAMERATE = 60.0f;                    // Design baseline
    constexpr float FRAME_DURATION = 1.0f / TARGET_FRAMERATE;    // Frame time
    constexpr float INPUT_BUFFER_DURATION = 0.2f;               // Input buffer window
    constexpr int32 INPUT_BUFFER_FRAMES = 12;                   // Buffer in frames
    constexpr float PERFECT_TIMING_WINDOW = 3.0f / TARGET_FRAMERATE; // 3 frames
    constexpr float COMBO_RESET_TIME = 2.0f;                    // Combo timeout
    constexpr int32 MAX_COMBO_CHAIN_LENGTH = 20;                // Max combo length
    
    // Debug visualization
    constexpr float DEBUG_SHAPE_ALPHA = 0.3f;                   // Debug transparency
    constexpr int32 DEBUG_SPHERE_SEGMENTS = 16;                 // Debug resolution
    constexpr int32 DEBUG_CONE_SEGMENTS = 12;                   // Debug resolution
}
```

### 🏃 Movement System Constants
```cpp
// Source: GameplayAbility_Dash.h
class UGameplayAbility_Dash
{
private:
    static constexpr float DEFAULT_UPDATE_RATE = 1.0f / 30.0f;  // 30fps updates
    static constexpr float MIN_DASH_SPEED = 100.0f;             // Minimum dash speed
    static constexpr float MAX_DASH_SPEED = 5000.0f;            // Maximum dash speed
};

// Character-level defaults
// Source: MyCharacter.h
class AMyCharacter
{
public:
    // Default values with designer-friendly ranges
    float DashSpeed = 1875.0f;                     // Core dash speed
    float DashDuration = 0.8f;                     // Dash duration
    float BounceUpwardVelocity = 800.0f;           // Bounce velocity
    int32 MaxAirBounces = 2;                       // Air bounce limit
};
```

## Data Loading and Initialization

### 🚀 Initialization Order

```cpp
// 1. Module startup - EROEOREOREOR.cpp
void FEROEOREOREORModule::StartupModule()
{
    // Module-level initialization
}

// 2. Character construction - MyCharacter constructor
AMyCharacter::AMyCharacter()
{
    // Create components
    AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
    AttributeSet = CreateDefaultSubobject<UMyAttributeSet>(TEXT("AttributeSet"));
    CombatStateMachine = CreateDefaultSubobject<UCombatStateMachineComponent>(TEXT("CombatStateMachine"));
    // ... other components
}

// 3. BeginPlay - Component initialization
void UCombatStateMachineComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Load DataTable data
    if (bAutoLoadDefaultTables && DefaultActionDataTable)
    {
        LoadActionData(DefaultActionDataTable);
    }
}

// 4. PossessedBy - GAS initialization
void AMyCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);
    
    // Initialize GAS
    if (AbilitySystemComponent)
    {
        AbilitySystemComponent->InitAbilityActorInfo(this, this);
        
        // Grant default abilities
        FGameplayAbilitySpecHandle DashHandle = AbilitySystemComponent->GiveAbility(
            FGameplayAbilitySpec(UGameplayAbility_Dash::StaticClass()));
        CachedDashAbilityHandle = DashHandle;
    }
}
```

### 🔄 Hot-Reload Support

```cpp
// DataTable hot-reload support for rapid iteration
UFUNCTION(BlueprintCallable, Category = "Data Management", CallInEditor)
void UCombatStateMachineComponent::ReloadDataTables()
{
    // Clear cached data
    LoadedActions.Empty();
    LoadedHiddenCombos.Empty();
    
    // Reload from disk
    if (DefaultActionDataTable)
    {
        DefaultActionDataTable->MarkAsGarbage();
        LoadActionData(DefaultActionDataTable);
    }
    
    UE_LOG(LogCombatSystem, Warning, TEXT("Combat data tables reloaded successfully"));
}
```

### 🎯 Memory Management

```cpp
// Efficient data storage patterns used in the project:

// 1. TMap for fast lookup (O(1) average case)
TMap<FGameplayTag, FCombatActionData> LoadedActions;  // Tag-based lookup

// 2. TWeakObjectPtr for safe actor references  
TWeakObjectPtr<AMyCharacter> OwnerCharacter;          // Prevents dangling pointers

// 3. Transient properties for runtime-only data
UPROPERTY(Transient)
FGameplayAbilitySpecHandle CachedDashAbilityHandle;   // Not serialized

// 4. Component references vs. lookups
UCombatStateMachineComponent* CombatStateMachine;     // Direct reference (fast)
// vs.
GetOwner()->FindComponentByClass<UCombatStateMachineComponent>(); // Lookup (slower)
