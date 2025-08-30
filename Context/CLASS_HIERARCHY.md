# CLASS HIERARCHY

## Inheritance Chains Starting from Unreal Base Classes

### 🎮 Character/Actor Hierarchy

```
AActor (Unreal Engine)
└── APawn (Unreal Engine)
    └── ACharacter (Unreal Engine)
        └── AMyCharacter (EROEOREOREOR)
            ├── Implements: IAbilitySystemInterface
            ├── Components:
            │   ├── UAbilitySystemComponent
            │   ├── UMyAttributeSet
            │   ├── UCombatStateMachineComponent
            │   ├── UAttackShapeComponent
            │   ├── UVelocitySnapshotComponent
            │   ├── UDamageApplicationComponent (forward declared)
            │   ├── USpringArmComponent
            │   └── UCameraComponent
            └── Features:
                ├── Enhanced Input handling
                ├── Camera-relative movement
                ├── Dash/Bounce abilities
                └── Combat system integration

AActor (Unreal Engine)
└── ATargetDummy (EROEOREOREOR)
    └── Features:
        ├── Damage visualization
        ├── Combat testing target
        └── Health display
```

### 🎯 Gameplay Ability System (GAS) Hierarchy

```
UAttributeSet (GameplayAbilities)
└── UMyAttributeSet (EROEOREOREOR)
    └── Attributes:
        ├── Health/MaxHealth
        ├── Stamina/MaxStamina
        ├── AirBounceCount (movement tracking)
        ├── AttackPower, CriticalHitChance, CriticalHitMultiplier
        ├── AttackSpeed, PhysicalResistance, ElementalResistance
        └── IncomingDamage/IncomingHealing (meta attributes)

UGameplayAbility (GameplayAbilities)
├── UGameplayAbility_Dash (EROEOREOREOR)
│   └── Features:
│       ├── Camera-relative dash direction
│       ├── Momentum preservation
│       ├── Curve-based velocity control
│       ├── Runtime parameter adjustment
│       └── Preset configurations
└── UGameplayAbility_Bounce (EROEOREOREOR)
    └── Features:
        ├── Air bounce tracking via attributes
        ├── Velocity snapshot integration
        ├── Horizontal momentum preservation
        └── Combo system support

UGameplayEffect (GameplayAbilities)
├── UGameplayEffect_Damage (EROEOREOREOR)
│   └── Purpose: Apply damage to targets
├── UGameplayEffect_AirBounceIncrement (EROEOREOREOR)
│   └── Purpose: Increase air bounce counter by 1
└── UGameplayEffect_AirBounceReset (EROEOREOREOR)
    └── Purpose: Reset air bounce counter to 0 (on landing)
```

### 🔧 Component Hierarchy

```
UActorComponent (Unreal Engine)
├── UVelocitySnapshotComponent (EROEOREOREOR)
│   └── Features:
│       ├── Pre-dash velocity capture
│       ├── Post-dash velocity restoration
│       └── Dash-bounce combo support
├── UCombatStateMachineComponent (EROEOREOREOR)
│   └── Features:
│       ├── Frame-accurate combat state management
│       ├── Attack canceling system
│       ├── Priority-based interrupt handling
│       └── DataTable integration
├── UAttackShapeComponent (EROEOREOREOR)
│   └── Features:
│       ├── Runtime hitbox generation
│       ├── Multiple attack shapes (Sphere, Capsule, Box, Cone, Line, Ring, Arc)
│       ├── Debug visualization
│       └── Multi-hit detection
├── UCombatPrototypeComponent (EROEOREOREOR)
│   └── Features:
│       ├── Prototype-based attack execution
│       ├── DataTable-driven combat actions
│       └── Integration with AttackShapeComponent
└── UAoEPrototypeComponent (EROEOREOREOR)
    └── Features:
        ├── Area-of-effect attack handling
        ├── Shape-based AoE detection
        └── Combat system integration
```

### 🗂️ Data Structure Hierarchy

```
FTableRowBase (Unreal Engine)
├── FCombatActionData (EROEOREOREOR)
│   └── Purpose: Frame-accurate combat action data
│       ├── Timing: StartupFrames, ActiveFrames, RecoveryFrames
│       ├── Canceling: CanCancelInto, CancelWindows
│       ├── Priority: ECombatPriority levels
│       ├── Movement: Speed multipliers, rotation locks
│       └── Integration: Combat/AoE prototype names
├── FHiddenComboData (EROEOREOREOR)
│   └── Purpose: Secret combo sequence definitions
│       ├── Sequence requirements
│       ├── Timing constraints
│       └── Bonus rewards
└── FAttackPrototypeData (EROEOREOREOR)
    └── Purpose: Enhanced attack data with shape information
        ├── Multiple hitboxes (FAttackShapeData array)
        ├── Damage/knockback values
        └── Placeholder art references

UObject (Unreal Engine)
└── UGameplayTagTester (EROEOREOREOR)
    └── Purpose: Gameplay tag system validation and testing
```

### 🏷️ Enumeration Definitions

```cpp
// Movement System Enums
enum class EDashDirection : uint8
{
    None, Left, Right
};

// Combat System Enums
enum class ECombatState : uint8
{
    Idle, Startup, Active, Recovery, Canceling, 
    Airborne, Dashing, Stunned
};

enum class ECombatPriority : uint8
{
    Light,      // Priority 0 - can be interrupted
    Heavy,      // Priority 1 - more resistant
    Dash,       // Priority 2 - movement abilities  
    Special,    // Priority 3 - hard to interrupt
    Ultimate    // Priority 4 - cannot be interrupted
};

enum class EStyleRating : uint8
{
    C, B, A, S, SSS  // Devil May Cry style rating system
};

enum class EAttackShape : uint8
{
    Sphere,     // Point attacks
    Capsule,    // Sword swings
    Box,        // Wide attacks
    Cone,       // Directional blasts
    Line,       // Thrusts/spears
    Ring,       // Around character
    Arc,        // Partial circles
    Custom      // Custom shapes
};
```

## Interface Implementation

### 🔌 IAbilitySystemInterface
```cpp
// Implemented by: AMyCharacter
class EROEOREOREOR_API AMyCharacter : public ACharacter, public IAbilitySystemInterface
{
public:
    // IAbilitySystemInterface implementation
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
    
    // Additional GAS integration
    UMyAttributeSet* GetMyAttributeSet() const;
};
```

## Component Composition Patterns

### 🎮 AMyCharacter Component Setup

```cpp
// Camera System
USpringArmComponent* CameraBoom;           // Third-person camera arm
UCameraComponent* FollowCamera;            // Main camera

// Gameplay Ability System
UAbilitySystemComponent* AbilitySystemComponent;  // Core GAS component
UMyAttributeSet* AttributeSet;                   // Custom attributes

// Combat System (Composition over Inheritance)
UCombatStateMachineComponent* CombatStateMachine;      // State management
UDamageApplicationComponent* DamageApplicationComponent; // Damage handling
UAttackShapeComponent* AttackShapeComponent;           // Hitbox generation

// Movement System
UVelocitySnapshotComponent* VelocitySnapshotComponent; // Dash-bounce combos
```

### 🎯 ATargetDummy Component Setup

```cpp
// Basic target with health visualization
// Inherits from AActor with minimal components
// Implements IDamageable interface (implied from usage)
```

## Common Component Compositions

### 🏃 Movement-Focused Actors
```
Actor + CharacterMovementComponent + VelocitySnapshotComponent + AbilitySystemComponent
└── Use Case: Characters needing advanced movement with momentum preservation
```

### ⚔️ Combat-Focused Actors  
```
Actor + CombatStateMachineComponent + AttackShapeComponent + CombatPrototypeComponent
└── Use Case: Characters/NPCs needing frame-accurate combat
```

### 🎯 GAS-Enabled Actors
```
Actor + AbilitySystemComponent + AttributeSet + (Various GameplayAbilities)
└── Use Case: Any actor needing ability system integration
```

### 🧪 Testing Actors
```
Actor + GameplayTagTester + (Minimal additional components)
└── Use Case: Development testing and validation
```

## ASCII Tree Diagrams

### 🌳 Complete Project Hierarchy

```
Unreal Engine Base Classes
├── AActor
│   ├── APawn
│   │   └── ACharacter
│   │       └── AMyCharacter (🎮 Player)
│   └── ATargetDummy (🎯 Testing)
│
├── UActorComponent
│   ├── UVelocitySnapshotComponent (🏃 Movement)
│   ├── UCombatStateMachineComponent (⚔️ Combat States)
│   ├── UAttackShapeComponent (🔺 Hitboxes)
│   ├── UCombatPrototypeComponent (⚔️ Combat Execution)
│   └── UAoEPrototypeComponent (💥 Area Effects)
│
├── UAttributeSet (GAS)
│   └── UMyAttributeSet (📊 Custom Attributes)
│
├── UGameplayAbility (GAS)
│   ├── UGameplayAbility_Dash (🏃 Dash Movement)
│   └── UGameplayAbility_Bounce (⬆️ Air Bounce)
│
├── UGameplayEffect (GAS)
│   ├── UGameplayEffect_Damage (💔 Damage)
│   ├── UGameplayEffect_AirBounceIncrement (⬆️ Count++)
│   └── UGameplayEffect_AirBounceReset (⬇️ Count=0)
│
└── UObject
    └── UGameplayTagTester (🧪 Testing)
```

### 🔗 System Integration Flow

```
Input System
     ↓
AMyCharacter (Enhanced Input)
     ↓
AbilitySystemComponent
     ↓
GameplayAbility_* (Dash/Bounce)
     ↓
CharacterMovementComponent + VelocitySnapshotComponent
     ↓
Movement Execution

Combat Input
     ↓
AMyCharacter
     ↓
CombatStateMachineComponent
     ↓
CombatPrototypeComponent + AttackShapeComponent
     ↓
GameplayEffect_Damage → Target's AbilitySystemComponent
     ↓
Damage Application
```

## Design Patterns Used

### 🎨 Composition over Inheritance
- **AMyCharacter** uses component composition instead of deep inheritance
- Each component has single responsibility (combat, movement, abilities)
- Easy to test and maintain individual systems

### 🏭 Data-Driven Design
- **Combat system** driven by DataTables (`DT_AttackPrototypes.csv`, `DT_BasicCombatActions.csv`)
- **Attack shapes** defined in data structures, not hardcoded
- **Frame timing** configurable via data tables

### 🔌 Interface Segregation
- **IAbilitySystemInterface** provides clean GAS integration
- Components expose minimal, focused APIs
- Clear separation between systems

### 📦 Single Responsibility Principle
- **UCombatStateMachineComponent**: Only handles combat state
- **UAttackShapeComponent**: Only handles hitbox generation
- **UVelocitySnapshotComponent**: Only handles velocity preservation

### 🎯 Dependency Injection
- Components injected into **AMyCharacter** at construction time
- **TWeakObjectPtr** used for safe actor references
- **Forward declarations** minimize header dependencies

## Key Relationships

| Parent Class | Child Class | Relationship Type | Purpose |
|-------------|-------------|------------------|---------|
| `ACharacter` | `AMyCharacter` | Inheritance | Enhanced player character with GAS |
| `UAttributeSet` | `UMyAttributeSet` | Inheritance | Custom gameplay attributes |
| `UGameplayAbility` | `UGameplayAbility_Dash` | Inheritance | Camera-relative dash ability |
| `UGameplayAbility` | `UGameplayAbility_Bounce` | Inheritance | Air bounce ability |
| `UActorComponent` | `UCombatStateMachineComponent` | Inheritance | Combat state management |
| `UActorComponent` | `UAttackShapeComponent` | Inheritance | Runtime hitbox generation |
| `FTableRowBase` | `FCombatActionData` | Inheritance | Data-driven combat actions |
| `IAbilitySystemInterface` | `AMyCharacter` | Interface | GAS integration point |

## Performance Considerations

### ⚡ High-Performance Classes
- **UMyAttributeSet**: Replicated attributes with efficient networking
- **UAttackShapeComponent**: Optimized collision queries using shape primitives
- **UCombatStateMachineComponent**: Frame-accurate timing with minimal overhead

### 🐌 Potential Performance Hotspots
- **Debug visualization** in AttackShapeComponent (disabled in shipping builds)
- **Collision queries** during combat hit detection
- **Timer management** in GameplayAbility_Dash for smooth movement updates

### 🔧 Optimization Patterns
- **Object pooling** for GameplayEffects (handled by GAS)
- **Component caching** in AMyCharacter to avoid repeated lookups
- **Weak pointers** to prevent memory leaks and dangling references
