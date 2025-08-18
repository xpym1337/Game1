# EROEOREOREOR - Codebase Architecture & Context Guide

## Project Overview
**EROEOREOREOR** is an Unreal Engine 5 C++ third-person action game project implementing the **Gameplay Ability System (GAS)** for character abilities, attributes, and gameplay mechanics. The project follows modern UE5 patterns including Enhanced Input System and modular component-based architecture.

---

## Table of Contents
1. [Core Game Classes](#core-game-classes)
2. [Gameplay Ability System Architecture](#gameplay-ability-system-architecture)
3. [Input System](#input-system)
4. [Testing Infrastructure](#testing-infrastructure)
5. [Build Configuration](#build-configuration)
6. [Development Phases](#development-phases)
7. [Key Architectural Decisions](#key-architectural-decisions)

---

## Core Game Classes

### Character System

#### `AMyCharacter` (MyCharacter.h/cpp)
**Purpose**: Main player character class extending UE5's `ACharacter` base class
**Key Responsibilities**:
- **GAS Integration**: Implements `IAbilitySystemInterface` for ability system communication
- **Camera System**: Third-person camera setup with SpringArm component for smooth following
- **Enhanced Input**: Modern input handling using UE5's Enhanced Input system
- **Attribute Management**: Owns and manages the character's attribute set

**Key Components**:
```cpp
// GAS Components - Core of the ability system
UAbilitySystemComponent* AbilitySystemComponent;  // Handles abilities, effects, attributes
UMyAttributeSet* AttributeSet;                   // Contains Health, Stamina, etc.

// Camera System - Third-person perspective
USpringArmComponent* CameraBoom;     // Collision-aware camera arm (400 unit length)
UCameraComponent* FollowCamera;      // Actual camera attached to boom end

// Enhanced Input Actions
UInputAction* JumpAction;    // Jump input binding
UInputAction* MoveAction;    // WASD movement input (2D vector)
UInputAction* LookAction;    // Mouse look input (2D vector)
```

**Critical Lifecycle Functions**:
- `PossessedBy()`: Server-side GAS initialization when controller possesses character
- `OnRep_PlayerState()`: Client-side GAS initialization for network replication
- These ensure GAS works correctly in both single-player and multiplayer scenarios

**Movement Configuration**:
```cpp
// Character movement tuning for responsive feel
JumpZVelocity = 700.f;           // Jump height
MaxWalkSpeed = 500.f;            // Running speed  
AirControl = 0.35f;              // Air movement control
BrakingDecelerationWalking = 2000.f;  // Stop responsiveness
```

---

## Gameplay Ability System Architecture

### `UMyAttributeSet` (MyAttributeSet.h/cpp)
**Purpose**: Defines character stats/attributes that can be modified by gameplay effects
**Key Features**:
- **Network Replication**: All attributes replicate automatically across clients
- **Attribute Clamping**: Prevents attributes from exceeding valid ranges
- **Change Notifications**: Triggers when attributes change for UI updates

**Attributes Defined**:
```cpp
// Health System
FGameplayAttributeData Health;        // Current health (0-MaxHealth)
FGameplayAttributeData MaxHealth;     // Maximum possible health

// Stamina System  
FGameplayAttributeData Stamina;       // Current stamina (0-MaxStamina)
FGameplayAttributeData MaxStamina;    // Maximum possible stamina
```

**Attribute Accessors**: Each attribute gets auto-generated getter/setter functions via `ATTRIBUTE_ACCESSORS` macro, providing:
- `GetHealthAttribute()` - Returns the FGameplayAttribute for the Health property
- `GetHealth()` - Gets current health value
- `SetHealth()` - Sets health value
- `InitHealth()` - Initializes health value

**Network Replication**: Uses `OnRep_` functions for client-side attribute change notifications

### Gameplay Tags System
**Purpose**: Hierarchical string-based tags for organizing gameplay concepts
**Tag Categories Implemented**:

```ini
# Abilities - What the character can do
Ability.Dash        # Dash/dodge movement ability
Ability.Attack      # Basic attack ability
Ability.Jump        # Jump ability

# States - Current character conditions
State.Dashing       # Currently performing dash
State.Attacking     # Currently attacking
State.Stunned       # Unable to act
State.InAir         # Airborne/jumping

# Effects - Temporary modifications
Effect.Cooldown     # Ability on cooldown
Effect.Cost         # Resource cost applied
Effect.Damage       # Damage being applied

# Input - Input state management
Input.Blocked       # Input temporarily disabled
```

**Usage Pattern**: Tags prevent ability conflicts (can't dash while stunned), manage cooldowns, and organize game state

---

## Input System

### Enhanced Input Architecture
**Why Enhanced Input**: UE5's modern input system providing better axis handling, modifier support, and Blueprint integration compared to legacy input

**Input Actions Configured**:
- `MoveAction`: 2D Vector input mapping WASD keys to forward/right movement
- `LookAction`: 2D Vector input for mouse look (pitch/yaw camera control)
- `JumpAction`: Single button press for character jumping

**Input Processing Flow**:
1. Input Mapping Context (`DefaultMappingContext`) maps physical keys to Input Actions
2. Enhanced Input Component binds Input Actions to C++ functions  
3. Movement functions convert input vectors to world-space movement using controller rotation

**Key Input Functions**:
```cpp
void Move(const FInputActionValue& Value);  // Processes WASD as 2D vector
void Look(const FInputActionValue& Value);  // Processes mouse delta as 2D vector  
void Jump();                                // Triggers character jump
```

---

## Testing Infrastructure

### `AGameplayTagTester` (GameplayTagTester.h/cpp)
**Purpose**: Dedicated testing actor for validating Gameplay Tags functionality
**Why This Exists**: Provides isolated environment to test GAS features without affecting main character

**Testing Capabilities**:
- **Tag Management**: Add/remove individual tags from Ability System Component
- **Tag Queries**: Test HasTag, HasAllTags, HasAnyTags functionality
- **Container Operations**: Test FGameplayTagContainer behavior
- **Debug Logging**: Output current tag state for verification

**Test Functions Available**:
```cpp
void AddTagToASC(FGameplayTag TagToAdd);           // Add single tag
void RemoveTagFromASC(FGameplayTag TagToRemove);   // Remove single tag
bool HasTag(FGameplayTag TagToCheck);              // Query single tag
bool HasAllTags(FGameplayTagContainer TagsToCheck); // Must have ALL tags
bool HasAnyTags(FGameplayTagContainer TagsToCheck); // Must have ANY tag
void RunBasicTagTests();                           // Automated basic test suite
void RunAdvancedTagTests();                        // Automated advanced test suite
```

---

## Build Configuration

### `EROEOREOREOR.Build.cs`
**Purpose**: Unreal Build Tool configuration defining module dependencies

**Key Dependencies**:
```cpp
PublicDependencyModuleNames.AddRange(new string[] { 
    "Core", "CoreUObject", "Engine",    // Essential UE5 modules
    "InputCore", "EnhancedInput",       // Input system modules
    "GameplayAbilities",               // GAS core functionality
    "GameplayTags", "GameplayTasks"    // GAS supporting systems
});
```

**Why These Dependencies**:
- **GameplayAbilities**: Core GAS functionality (abilities, effects, attributes)
- **GameplayTags**: Hierarchical tag system for game state management  
- **GameplayTasks**: Task-based ability execution (used internally by GAS)
- **EnhancedInput**: Modern UE5 input handling system

---

## Development Phases

### Phase 1: GAS Foundation ✅ COMPLETE
**Objectives**: Establish core GAS infrastructure
**Achievements**:
- Character implements `IAbilitySystemInterface`
- AbilitySystemComponent and AttributeSet properly initialized
- Basic attributes (Health, MaxHealth, Stamina, MaxStamina) functional
- Network replication working
- Third-person camera and Enhanced Input integrated

### Phase 2: Gameplay Tags ✅ COMPLETE  
**Objectives**: Implement hierarchical tag system for game state management
**Achievements**:
- Comprehensive tag hierarchy defined in `GameplayTags.ini`
- Tag testing infrastructure (`AGameplayTagTester`) implemented
- All tag operations verified (add, remove, query, containers)
- Blueprint integration confirmed
- Ready for ability implementation

### Phase 3: First Abilities (Upcoming)
**Planned Objectives**:
- Implement Dash ability using established tags
- Create ability cooldown system  
- Test ability activation/deactivation
- Integrate ability input bindings

---

## Key Architectural Decisions

### Why Gameplay Ability System?
**Decision**: Use UE5's built-in GAS instead of custom ability system
**Reasoning**:
- **Scalability**: Handles complex ability interactions, cooldowns, costs automatically
- **Network Ready**: Built-in replication for multiplayer games
- **Extensible**: Easy to add new abilities without modifying character class
- **Blueprint Integration**: Designers can create abilities without C++ knowledge

### Why Enhanced Input?
**Decision**: Use UE5's Enhanced Input instead of legacy input system
**Reasoning**:
- **Modern Standard**: UE5's recommended input solution
- **Flexibility**: Better handling of input modifiers, axis mapping
- **Blueprint Friendly**: Easier for designers to modify input mappings
- **Future Proof**: Legacy input system deprecated

### Component-Based Architecture
**Decision**: Use UE5's component system for modular functionality
**Reasoning**:
- **Modularity**: Camera, movement, abilities are separate concerns
- **Reusability**: Components can be used on different actor types
- **Maintainability**: Each system has clear boundaries and responsibilities
- **UE5 Standard**: Follows Epic's recommended patterns

### Attribute Set Design
**Decision**: Create custom UMyAttributeSet with basic RPG stats
**Reasoning**:
- **Game Requirements**: Health/Stamina are core to most action games
- **Extensibility**: Easy to add new attributes (Mana, Armor, etc.)
- **Network Optimization**: Only replicate what's needed
- **Blueprint Accessibility**: Attributes visible in Blueprint editors

---

## File Structure Overview

```
Source/EROEOREOREOR/
├── EROEOREOREOR.Build.cs           # Build configuration
├── EROEOREOREOR.cpp/.h             # Module implementation
├── MyCharacter.cpp/.h              # Main player character
├── MyAttributeSet.cpp/.h           # GAS attribute definitions
└── GameplayTagTester.cpp/.h        # Tag testing infrastructure

Config/
└── Tags/GameplayTags.ini           # Gameplay tag definitions

Content/
├── BP_MyMyCharacter.uasset         # Character Blueprint
├── Input/                          # Enhanced Input assets
└── ThirdPerson/                    # Default level assets

Context/
├── Phase_1_completion_context.txt   # GAS setup completion
├── Phase_2_completion_context.txt   # Tags setup completion
└── CODEBASE_ARCHITECTURE_CONTEXT.md # This file
```

---

## Next Steps for New Developers

### Getting Started:
1. **Understand GAS Basics**: Focus on how AbilitySystemComponent, AttributeSet, and GameplayTags work together
2. **Study MyCharacter**: See how GAS integrates with standard UE5 Character class
3. **Experiment with GameplayTagTester**: Use it to understand tag operations before implementing abilities
4. **Review Phase contexts**: Understand what's been completed and what's planned

### Common Patterns:
- **Ability Creation**: Extend UGameplayAbility, use tags for identification and state management
- **Effect Creation**: Extend UGameplayEffect for temporary stat modifications  
- **Tag Usage**: Use tags instead of booleans for complex state management
- **Input Handling**: Bind Enhanced Input actions to ability activation functions

### Debugging Tips:
- Use GameplayTagTester to verify tag operations
- Enable GAS logging: `LogAbilitySystem` category
- Check AttributeSet replication in multiplayer scenarios
- Verify Enhanced Input mapping contexts are properly applied

---

*This context guide provides the foundation for understanding and extending the EROEOREOREOR codebase. Each system is designed to be modular and extensible, following UE5 best practices for scalable game development.*
