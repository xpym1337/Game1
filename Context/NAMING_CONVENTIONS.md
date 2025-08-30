# NAMING CONVENTIONS

## C++ Naming Patterns

### 📋 Classes

| Type | Pattern | Examples | Notes |
|------|---------|----------|-------|
| **Actor Classes** | `A[Name]` | `AMyCharacter`, `ATargetDummy` | Unreal standard prefix |
| **Component Classes** | `U[Name]Component` | `UCombatStateMachineComponent`, `UVelocitySnapshotComponent` | Descriptive + Component suffix |
| **Gameplay Abilities** | `UGameplayAbility_[Name]` | `UGameplayAbility_Dash`, `UGameplayAbility_Bounce` | GAS standard with underscore |
| **Gameplay Effects** | `UGameplayEffect_[Name]` | `UGameplayEffect_Damage`, `UGameplayEffect_AirBounceReset` | GAS standard with underscore |
| **AttributeSet Classes** | `U[Name]AttributeSet` | `UMyAttributeSet` | GAS standard |
| **Object Classes** | `U[Name]` | `UGameplayTagTester` | Unreal standard prefix |

### 🔧 Functions

| Type | Pattern | Examples | Notes |
|------|---------|----------|-------|
| **Public Functions** | `PascalCase` | `TryStartAction()`, `GetCurrentState()` | Standard C++ convention |
| **Blueprint Functions** | `PascalCase` | `GetMyAttributeSet()`, `GetFollowCamera()` | Blueprint-accessible |
| **Protected Functions** | `PascalCase` | `BeginPlay()`, `SetupPlayerInputComponent()` | Unreal overrides |
| **Private Functions** | `PascalCase` | `ExecuteDash()`, `ValidateActivationRequirements()` | Internal implementation |
| **Input Functions** | `[Action]` | `DashLeft()`, `MoveForward()`, `Bounce()` | Input action names |
| **Event Handlers** | `On[Event]` | `OnRep_Health()`, `OnCurveAssetsLoaded()` | Event response pattern |
| **Test Functions** | `Test[System]` | `TestDash()`, `TestBounce()`, `TestCombatSystem()` | Debug/testing utilities |

### 📊 Variables and Parameters

| Type | Pattern | Examples | Notes |
|------|---------|----------|-------|
| **Member Variables** | `PascalCase` | `DashSpeed`, `CurrentState`, `AbilitySystemComponent` | No prefixes used |
| **Local Variables** | `PascalCase` | `SpawnLocation`, `ActionData`, `TimerManager` | Consistent with members |
| **Function Parameters** | `PascalCase` or `InVariableName` | `NewController`, `InDirection`, `DeltaTime` | Epic Games style with In prefix |
| **Boolean Variables** | `b[Variable]` | `bIsShiftPressed`, `bCanDash`, `bUseCombatPrototype` | Unreal boolean prefix |
| **Constants** | `UPPER_CASE` | `TARGET_FRAMERATE`, `MAX_DASH_SPEED` | Traditional C++ constants |
| **Enum Values** | `PascalCase` | `ECombatState::Idle`, `EAttackShape::Sphere` | Unreal enum standard |

### 🎯 Special Patterns

| Type | Pattern | Examples | Notes |
|------|---------|----------|-------|
| **Template Parameters** | `T[Name]` | `TObjectPtr<UCameraComponent>`, `TWeakObjectPtr<AMyCharacter>` | Unreal template style |
| **Delegates** | `FOn[Event]` | `FOnCombatStateChanged`, `FOnPerfectCancel` | Event delegate pattern |
| **Structs** | `F[Name]` | `FCombatActionData`, `FAttackShapeData` | Unreal struct prefix |
| **Enums** | `E[Name]` | `ECombatState`, `EDashDirection` | Unreal enum prefix |
| **Macros** | `UPPER_CASE` | `ATTRIBUTE_ACCESSORS`, `COMBAT_LOG` | Traditional macro style |

## Blueprint Asset Naming

### 🎮 Blueprint Classes

| Type | Prefix | Pattern | Examples |
|------|--------|---------|----------|
| **Blueprint Classes** | `BP_` | `BP_[Name]` | `BP_MyMyCharacter`, `BP_ThirdPersonCharacter` |
| **Widget Blueprints** | `UI_` | `UI_[Name]` | `UI_TouchSimple`, `UI_Thumbstick` |
| **Game Mode** | `BP_` | `BP_[Name]GameMode` | `BP_ThirdPersonGameMode` |
| **Player Controller** | `BP_` | `BP_[Name]PlayerController` | `BP_1ThirdPersonPlayerController` |
| **Gameplay Abilities** | `MyGameplayAbility_` | `MyGameplayAbility_[Name]` | `MyGameplayAbility_Bounce` |

### 🎯 Input System Assets

| Type | Prefix | Pattern | Examples |
|------|--------|---------|----------|
| **Input Actions** | `IA_` | `IA_[Action]` | `IA_MoveForward`, `IA_DashLeft`, `IA_Bounce` |
| **Input Mapping Context** | No prefix | `[Context]` | `NewInputMappingContext` |
| **Blueprint Interface** | `BPI_` | `BPI_[Name]` | `BPI_TouchInterface` |

### 🎨 Art and Content Assets

| Type | Prefix | Pattern | Examples |
|------|--------|---------|----------|
| **Static Meshes** | `SM_` | `SM_[Name]` | `SM_ChamferCube`, `SM_Cylinder` |
| **Skeletal Meshes** | `SKM_` | `SKM_[Name]` | `SKM_Manny_Simple`, `SKM_Quinn_Simple` |
| **Materials** | `M_` | `M_[Name]` | `M_Mannequin`, `M_PrototypeGrid` |
| **Material Instances** | `MI_` | `MI_[Name]` | `MI_DefaultColorway`, `MI_ThirdPersonColWay` |
| **Textures** | `T_` | `T_[Name]` | `T_GridChecker_A`, `T_Manny_01_D` |
| **Animations** | `MM_` | `MM_[Name]` | `MM_Idle`, `MM_Death_Back_01` |
| **Animation Blueprints** | `ABP_` | `ABP_[Name]` | `ABP_Unarmed` |
| **Particle Systems** | `NS_` | `NS_[Name]` | `NS_JumpPad` |

### 📁 Directory Organization

| Type | Pattern | Examples |
|------|---------|----------|
| **Characters** | `Characters/[Type]/` | `Characters/Mannequins/` |
| **Input** | `Input/[Type]/` | `Input/Actions/`, `Input/Touch/` |
| **Data** | `Data/[System]/` | `Data/Combat/` |
| **Level Prototyping** | `LevelPrototyping/[Type]/` | `LevelPrototyping/Materials/` |
| **Developer Folders** | `Developers/[Name]/` | `Developers/GrillerDriller/` |

## Gameplay Tags Hierarchy

### 🏷️ Tag Structure Patterns

```ini
# Root tags follow system-based organization
# Pattern: [System].[Category].[Subcategory].[Specific]

# Ability System Tags
+GameplayTagList=(Tag="Ability",DevComment="Root tag for all abilities")
+GameplayTagList=(Tag="Ability.Dash",DevComment="Dash ability")
+GameplayTagList=(Tag="Ability.Bounce",DevComment="Bounce ability")
+GameplayTagList=(Tag="Ability.Attack",DevComment="Basic attack ability")

# State System Tags  
+GameplayTagList=(Tag="State",DevComment="Root tag for character states")
+GameplayTagList=(Tag="State.Dashing",DevComment="Character is dashing")
+GameplayTagList=(Tag="State.Attacking",DevComment="Character is attacking")
+GameplayTagList=(Tag="State.InAir",DevComment="Character is airborne")

# Effect System Tags
+GameplayTagList=(Tag="Effect",DevComment="Root tag for gameplay effects")
+GameplayTagList=(Tag="Effect.Cooldown",DevComment="Cooldown effects")
+GameplayTagList=(Tag="Effect.Damage",DevComment="Damage effects")
+GameplayTagList=(Tag="Effect.AirBounce.Increment",DevComment="Air bounce increment")

# Combat System Tags (Extended Hierarchy)
+GameplayTagList=(Tag="Combat.Action.Light.Jab",DevComment="Light jab attack")
+GameplayTagList=(Tag="Combat.Action.Heavy.Straight",DevComment="Heavy straight attack")
+GameplayTagList=(Tag="Combat.State.Startup",DevComment="Attack startup phase")
+GameplayTagList=(Tag="Combat.Priority.Light",DevComment="Light attack priority")

# Input System Tags
+GameplayTagList=(Tag="Input.Dash.Left",DevComment="Left dash input")
+GameplayTagList=(Tag="Input.Dash.Right",DevComment="Right dash input")
```

### 🎯 Tag Naming Best Practices

| Category | Pattern | Good Examples | Bad Examples |
|----------|---------|---------------|--------------|
| **Abilities** | `Ability.[Name]` | `Ability.Dash`, `Ability.Bounce` | `DashAbility`, `ability_dash` |
| **States** | `State.[Condition]` | `State.Dashing`, `State.InAir` | `IsDashing`, `state_moving` |
| **Effects** | `Effect.[Type].[Specific]` | `Effect.AirBounce.Reset` | `AirBounceResetEffect` |
| **Combat** | `Combat.[Type].[Category].[Name]` | `Combat.Action.Light.Jab` | `LightJab`, `combat_jab` |
| **Input** | `Input.[System].[Direction]` | `Input.Dash.Left` | `LeftDash`, `input_left` |

## File and Directory Naming

### 📂 Source Code Organization

```
Source/EROEOREOREOR/
├── [SystemName]Component.h/.cpp        # Component classes
├── GameplayAbility_[Name].h/.cpp       # GAS abilities  
├── GameplayEffect_[Name].h/.cpp        # GAS effects
├── [SystemName]Types.h                 # Type definitions
├── My[Name].h/.cpp                     # Custom implementations
└── [ProjectName].Build.cs              # Build configuration
```

**Examples**:
- `CombatStateMachineComponent.h/.cpp` - Combat state management
- `VelocitySnapshotComponent.h/.cpp` - Movement system component
- `GameplayAbility_Dash.h/.cpp` - Dash ability implementation
- `CombatSystemTypes.h` - Combat system type definitions
- `MyCharacter.h/.cpp` - Custom character implementation
- `MyAttributeSet.h/.cpp` - Custom attribute set

### 🎨 Content Organization

```
Content/
├── Characters/[Type]/              # Character assets by type
├── Data/[System]/                  # Data tables by system
├── Input/[Type]/                   # Input system assets
├── LevelPrototyping/[Type]/        # Prototyping assets
├── ThirdPerson/                    # Template/demo content
└── Developers/[Name]/              # Personal developer folders
```

**Examples**:
- `Content/Characters/Mannequins/Meshes/` - Character meshes
- `Content/Data/Combat/DT_AttackPrototypes.csv` - Combat data
- `Content/Input/Actions/IA_DashLeft.uasset` - Input actions
- `Content/LevelPrototyping/Materials/` - Prototype materials

## Variable Naming Patterns

### 🎯 Component Member Variables

```cpp
// Pattern: [Type][Name] or [Name][Type]
class AMyCharacter
{
    // Camera components
    TObjectPtr<USpringArmComponent> CameraBoom;        // Clear purpose
    TObjectPtr<UCameraComponent> FollowCamera;         // Descriptive name
    
    // GAS components  
    TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;  // Full descriptive name
    TObjectPtr<UMyAttributeSet> AttributeSet;                   // Concise but clear
    
    // Combat components
    TObjectPtr<UCombatStateMachineComponent> CombatStateMachine;     // Shortened descriptive
    TObjectPtr<UAttackShapeComponent> AttackShapeComponent;          // Full descriptive name
    
    // Input actions
    TObjectPtr<UInputAction> MoveForwardAction;        // Action + Action suffix
    TObjectPtr<UInputAction> DashLeftAction;           // Clear directional naming
    TObjectPtr<UInputAction> BounceAction;             // Simple ability name
};
```

### ⚔️ Combat System Variables

```cpp
// Frame timing variables
int32 StartupFrames;                    // Descriptive + unit
int32 ActiveFrames;                     // Clear purpose
int32 RecoveryFrames;                   // Recovery phase
int32 CurrentFrame;                     // Current position in timeline

// State tracking
ECombatState CurrentState;              // Current + type
FGameplayTag CurrentActionTag;          // Current + type + tag
float StateElapsedTime;                 // State + measurement

// Configuration parameters
float DashSpeed;                        // Simple descriptive
float DashDuration;                     // Action + measurement
float MomentumRetention;                // Physics concept
bool bCanDash;                          // Boolean with b prefix
```

### 🏃 Movement System Variables

```cpp
// Movement input tracking
FVector2D CurrentMovementInput;         // Current + type + purpose
bool bIsShiftPressed;                   // Boolean state with b prefix

// Dash parameters
float DashSpeed;                        // Core parameter
float DashInitialBurstSpeed;            // Specific timing phase
float DashSpeedDecayRate;               // Rate-based parameter
float DashVerticalVelocityPreservation; // Descriptive physics parameter

// Bounce parameters  
float BounceUpwardVelocity;             // Direction + measurement
float HorizontalVelocityRetention;      // Retention concept
int32 MaxAirBounces;                    // Max + count
float BounceInputWindow;                // Input + timing
```

### 🎮 GAS Integration Variables

```cpp
// Ability handles (cached for performance)
FGameplayAbilitySpecHandle CachedDashAbilityHandle;     // Cached + type + handle
FGameplayAbilitySpecHandle CachedBounceAbilityHandle;   // Consistent pattern

// Gameplay tags (following GAS conventions)
FGameplayTag DashingStateTag;           // System + type + tag
FGameplayTag DashCooldownTag;           // System + purpose + tag
FGameplayTag DashImmuneTag;             // System + immunity + tag

// Timer handles
FTimerHandle VelocityUpdateTimer;       // Purpose + timer
FTimerHandle DashCooldownTimer;         // System + purpose + timer
FTimerHandle CurveLoadTimer;            // Asset + action + timer
```

## Parameter Naming

### 📝 Function Parameters

```cpp
// Input parameters with 'In' prefix (Epic Games standard)
void SetDashDirection(EDashDirection InDirection);
void ApplyDamage(float InDamageAmount, AActor* InTarget);
void LoadActionData(UDataTable* InActionDataTable);

// Output parameters with 'Out' prefix
void GetCombatInfo(FGameplayTag& OutCurrentAction, ECombatState& OutCurrentState);
bool TryGetActionData(const FGameplayTag& ActionTag, FCombatActionData& OutActionData);

// Reference parameters (no prefix when clear from context)
void HandleCombatStateChanged(ECombatState OldState, ECombatState NewState, const FGameplayTag& ActionTag);
void OnAttributeChanged(const FOnAttributeChangeData& Data);

// Delegate parameters (descriptive names)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
    FOnCombatStateChanged, 
    ECombatState, OldState,      // Clear old/new distinction
    ECombatState, NewState, 
    const FGameplayTag&, ActionTag
);
```

### 🔧 UPROPERTY Meta Tags

```cpp
// EditAnywhere properties with clear categories
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash|Core")
float DashSpeed = 1875.0f;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash|Feel")  
float DashInitialBurstSpeed = 2500.0f;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
TObjectPtr<UCombatStateMachineComponent> CombatStateMachine;

// Categories follow hierarchical pattern: [System]|[Subcategory]
// Examples: "Dash|Core", "Dash|Feel", "Bounce|Core", "Combat|State"
```

## Constants and Namespaces

### 🎯 Namespace Organization

```cpp
// System-specific constants in namespaces
namespace CombatConstants
{
    constexpr float TARGET_FRAMERATE = 60.0f;
    constexpr float FRAME_DURATION = 1.0f / TARGET_FRAMERATE;
    constexpr float INPUT_BUFFER_DURATION = 0.2f;
    constexpr int32 INPUT_BUFFER_FRAMES = 12;
    constexpr float PERFECT_TIMING_WINDOW = 3.0f / TARGET_FRAMERATE;
}

// Class-specific constants as static members
class UGameplayAbility_Dash
{
private:
    static constexpr float DEFAULT_UPDATE_RATE = 1.0f / 30.0f;
    static constexpr float MIN_DASH_SPEED = 100.0f;
    static constexpr float MAX_DASH_SPEED = 5000.0f;
};
```

### 📊 Macro Naming

```cpp
// Attribute accessor macros (GAS standard)
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
    GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

// Logging macros (project-specific)
#define COMBAT_LOG(Verbosity, Format, ...) \
    UE_LOG(LogCombatSystem, Verbosity, Format, ##__VA_ARGS__)
```

## Configuration and Data Naming

### 📋 DataTable Naming

| File | Pattern | Purpose |
|------|---------|---------|
| `DT_BasicCombatActions.csv` | `DT_[System][Type]` | Combat action frame data |
| `DT_AttackPrototypes.csv` | `DT_[System][Purpose]` | Attack shape definitions |
| `DT_HiddenCombos.csv` | `DT_[System][Feature]` | Secret combo sequences |

**Column Naming Pattern**: `PascalCase` matching struct member names exactly
- `ActionTag` (not `Action_Tag` or `actionTag`)
- `StartupFrames` (not `startup_frames` or `Startup_Frames`)
- `CancelWindowStart` (descriptive compound names)

### 🎯 Gameplay Tag Naming

**Hierarchical Structure**:
```
[RootSystem].[Category].[Subcategory].[Specific]

Examples:
- Ability.Dash                          # Simple ability
- Combat.Action.Light.Jab               # Complex hierarchical
- Effect.AirBounce.Increment            # System feature specific
- Input.Dash.Left                       # Input system directional
- State.Combat.Active                   # State management
```

**Best Practices**:
- Use **PascalCase** for all tag segments
- Keep **root categories** broad (`Ability`, `State`, `Effect`, `Combat`)
- Make **leaf tags** specific and descriptive
- Use **consistent depth** within each system (3-4 levels max)
- Avoid **abbreviations** unless universally understood (`AoE` is acceptable)

## Code Organization Patterns

### 📦 Include Organization

```cpp
// Header file include order (following Epic Games standards):

// 1. CoreMinimal first
#include "CoreMinimal.h"

// 2. Unreal Engine headers
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"

// 3. Project headers
#include "MyAttributeSet.h"
#include "CombatSystemTypes.h"

// 4. Generated header last
#include "MyCharacter.generated.h"
```

### 🔧 Function Organization

```cpp
class AMyCharacter
{
public:
    // 1. Constructor
    AMyCharacter();
    
    // 2. Unreal overrides (in virtual order)
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    
    // 3. Interface implementations
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
    
    // 4. Blueprint functions
    UFUNCTION(BlueprintCallable, Category = "GAS")
    UMyAttributeSet* GetMyAttributeSet() const;
    
    // 5. Input action functions (grouped by system)
    void MoveForward(const FInputActionValue& Value);
    void DashLeft(const FInputActionValue& Value);
    void Bounce(const FInputActionValue& Value);
    
    // 6. Testing/Debug functions
    void TestCombatSystem();
    void TestDash();

protected:
    // 7. Protected functions (Unreal lifecycle)
    virtual void PossessedBy(AController* NewController) override;

private:
    // 8. Private implementation functions
    void InitializeComponents();
    void SetupAbilities();
};
```

## Comment and Documentation Style

### 📝 Header Documentation

```cpp
/**
 * Production-ready Gameplay Ability for camera-relative dash movement
 * Follows Epic Games coding standards and GAS best practices
 * Single-responsibility, testable, maintainable implementation
 */
UCLASS()
class EROEOREOREOR_API UGameplayAbility_Dash : public UGameplayAbility
```

### 🎯 Property Documentation

```cpp
// Detailed property documentation with designer context
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash|Core", 
    meta = (ClampMin = "100.0", ClampMax = "5000.0", UIMin = "500.0", UIMax = "5000.0",
    ToolTip = "Base speed for dash movement in units per second"))
float DashSpeed = 1875.0f;

// Section headers for related properties
// DASH VELOCITY CONTROL - Epic Games naming convention
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash|Velocity")
float DashInitialBurstSpeed = 2500.0f;
```

### 📋 Inline Comments

```cpp
// Performance optimization comments
// PERFORMANCE: Cache ability handles to avoid lookup delays
UPROPERTY(Transient)
FGameplayAbilitySpecHandle CachedDashAbilityHandle;

// System integration comments  
// CRITICAL: Use TWeakObjectPtr for actor references to prevent dangling pointers
TWeakObjectPtr<AMyCharacter> CachedCharacter;

// Design intent comments
// Following Epic Games GAS patterns for movement state tracking
UPROPERTY(BlueprintReadOnly, Category = "Movement")
FGameplayAttributeData AirBounceCount;
```

## Consistency Rules

### ✅ DO

- Use **PascalCase** for all C++ identifiers (classes, functions, variables)
- Use **b** prefix for boolean variables
- Use **descriptive names** that explain purpose
- Follow **Unreal Engine conventions** for prefixes (`A`, `U`, `F`, `E`)
- Use **consistent category naming** in UPROPERTY macros
- Keep **gameplay tag hierarchies** logical and consistent
- Use **namespace organization** for related constants
- Include **descriptive comments** for complex systems

### ❌ DON'T

- Mix naming conventions (don't use `camelCase` or `snake_case`)
- Use cryptic abbreviations (`Dmg` instead of `Damage`)
- Create overly long names (`VeryLongDescriptiveVariableName`)
- Inconsistent prefixes for similar types
- Skip documentation for public interfaces
- Use Hungarian notation beyond Unreal's standard prefixes
- Create deep gameplay tag hierarchies (>4 levels)

## Project-Specific Conventions

### 🎮 EROEOREOREOR Specific Patterns

**Module Name**: Uses project name `EROEOREOREOR` as module identifier
**API Macro**: `EROEOREOREOR_API` for exported symbols
**Log Categories**: `LogCombatSystem`, `LogGAS`, `LogTemp` for different systems
**Test Functions**: All test functions prefixed with `Test` for easy identification
**Debug Categories**: Use `Debug` category for development-only properties

**Character Naming**: 
- Main character class: `AMyCharacter` (custom implementation)
- Blueprint character: `BP_MyMyCharacter` (note the double "My")
- AttributeSet: `UMyAttributeSet` (custom implementation)

**System Integration Pattern**:
- Components use full descriptive names: `CombatStateMachineComponent`
- Abilities use underscore separation: `GameplayAbility_Dash`
- Effects use underscore separation: `GameplayEffect_Damage`
- Data structures use descriptive names: `FCombatActionData`
