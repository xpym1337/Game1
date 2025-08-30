# CODEBASE MAP

## Directory Structure Overview

```
EROEOREOREOR/
├── Source/EROEOREOREOR/           # C++ Source Code
├── Content/                       # Unreal Engine Assets
├── Config/                        # Configuration Files
├── Context/                       # Documentation (this folder)
└── Binaries/, Intermediate/, etc. # Generated/Build files
```

## Source Code Structure (`/Source/EROEOREOREOR/`)

### Player/Character System
```cpp
// Core character implementation with GAS integration
MyCharacter.h/.cpp              // Main player character (ACharacter + IAbilitySystemInterface)
MyAttributeSet.h/.cpp          // GAS attributes (Health, AirBounces, etc.)
```

### Combat System (Frame-Accurate)
```cpp
// Combat system with 60fps frame timing and canceling
CombatSystemTypes.h            // Combat enums, structs, data definitions
CombatStateMachineComponent.h/.cpp    // State machine for combat actions
CombatPrototypeComponent.h/.cpp       // Prototype-based combat execution
AttackShapeComponent.h/.cpp           // Attack hitbox generation & visualization
AoEPrototypeComponent.h/.cpp          // Area-of-effect attack handling
```

### Movement/Abilities System
```cpp
// Advanced movement with momentum preservation
GameplayAbility_Dash.h/.cpp           // GAS-based dash with camera-relative direction
GameplayAbility_Bounce.h/.cpp         // Air bounce ability with combo potential  
VelocitySnapshotComponent.h/.cpp      // Velocity preservation for dash-bounce combos
```

### Gameplay Ability System (GAS)
```cpp
// GameplayEffects for various systems
GameplayEffect_Damage.h/.cpp          // Damage application effect
GameplayEffect_AirBounceIncrement.h/.cpp  // Increment air bounce counter
GameplayEffect_AirBounceReset.h/.cpp      // Reset air bounce counter on landing
```

### Testing & Debug
```cpp
TargetDummy.h/.cpp            // Combat testing target with damage visualization
GameplayTagTester.h/.cpp      // Gameplay tag system testing utility
```

### Build Configuration
```cpp
EROEOREOREOR.Build.cs         // Module dependencies and build settings
EROEOREOREOR.h/.cpp          // Main module implementation
```

## Content Structure (`/Content/`)

### Character Assets
```
Characters/
├── Mannequins/               # Default UE5 character assets
│   ├── Meshes/              # Character meshes (Manny, Quinn)
│   ├── Materials/           # Character materials
│   ├── Anims/               # Animation assets
│   └── Textures/            # Character textures
```

### Data-Driven Combat
```
Data/Combat/
├── DT_AttackPrototypes.csv   # Attack prototypes with shape data
└── DT_BasicCombatActions.csv # Frame data for combat actions
```

### Input System
```
Input/
├── Actions/                  # Input Action assets
│   ├── IA_MoveForward.uasset, IA_MoveBackward.uasset
│   ├── IA_DashLeft.uasset, IA_DashRight.uasset
│   ├── IA_Bounce.uasset, IA_Jump.uasset
│   └── IA_Look.uasset, IA_Shift.uasset
├── NewInputMappingContext.uasset  # Enhanced Input mapping
└── BP_DashAbility.uasset, MyGameplayAbility_Bounce.uasset
```

### Level Design
```
ThirdPerson/
├── Lvl_ThirdPerson.umap      # Main test level
└── Blueprints/               # Game mode, player controller, character BP

LevelPrototyping/
├── Materials/                # Prototype grid materials
├── Meshes/                   # Basic shapes for level design
└── Interactable/             # Jump pads, targets, doors
```

### Gameplay Ability System Integration
```
GAS/                          # Blueprint GAS assets
└── [GAS-related blueprints]
```

## Functional System Groups

### 🎮 Player System
**Primary Files:**
- `MyCharacter.h/.cpp` - Main character with GAS integration
- `MyAttributeSet.h/.cpp` - Character attributes and stats
- `Content/BP_MyMyCharacter.uasset` - Blueprint character setup

**Responsibilities:**
- Enhanced Input handling (directional movement, dash, bounce)
- GAS integration and ability activation
- Camera management (SpringArm + Camera)
- State tracking (movement input, shift pressed, etc.)

### ⚔️ Combat System
**Primary Files:**
- `CombatSystemTypes.h` - Combat data structures and enums
- `CombatStateMachineComponent.h/.cpp` - Combat state management
- `CombatPrototypeComponent.h/.cpp` - Attack execution
- `AttackShapeComponent.h/.cpp` - Hitbox generation
- `Content/Data/Combat/DT_*.csv` - Combat data tables

**Responsibilities:**
- Frame-accurate combat timing (60fps baseline)
- Attack canceling system with priority levels
- Shape-based attack hitboxes with debug visualization
- Data-driven combat actions from DataTables

### 🏃 Movement System
**Primary Files:**
- `GameplayAbility_Dash.h/.cpp` - Camera-relative dash ability
- `GameplayAbility_Bounce.h/.cpp` - Air bounce with momentum preservation
- `VelocitySnapshotComponent.h/.cpp` - Velocity state management

**Responsibilities:**
- Advanced dash system with momentum retention
- Air bounce system with maximum air bounces tracking
- Dash-bounce combo system with velocity preservation
- Camera-relative movement calculations

### 🎯 Gameplay Ability System
**Primary Files:**
- All `GameplayEffect_*.h/.cpp` files
- `MyAttributeSet.h/.cpp`
- GAS-related Blueprint assets in `Content/Input/`

**Responsibilities:**
- Ability cooldowns and resource management
- Effect application (damage, air bounce tracking)
- Attribute modification and replication
- Integration with combat and movement systems

### 🔧 Testing & Debug
**Primary Files:**
- `TargetDummy.h/.cpp` - Damage testing target
- `GameplayTagTester.h/.cpp` - Tag system validation
- Various debug visualization in combat components

**Responsibilities:**
- Combat system testing and validation
- Gameplay tag system testing
- Visual debugging for attack shapes and hitboxes

## Quick Lookup Guide

### 🔍 Common Development Tasks → File Locations

| Task | Primary Files | Supporting Files |
|------|---------------|------------------|
| **Add new attack** | `Content/Data/Combat/DT_AttackPrototypes.csv` | `CombatSystemTypes.h` (for new enums/structs) |
| **Modify character movement** | `MyCharacter.h/.cpp` | `GameplayAbility_Dash.h/.cpp`, `GameplayAbility_Bounce.h/.cpp` |
| **Add new ability** | Create new `GameplayAbility_*.h/.cpp` | `MyCharacter.h/.cpp` (for input binding) |
| **Change combat timing** | `Content/Data/Combat/DT_BasicCombatActions.csv` | `CombatSystemTypes.h` (frame data structs) |
| **Add gameplay tags** | `Config/Tags/GameplayTags.ini` | Any file using the new tags |
| **Debug combat system** | `AttackShapeComponent.h/.cpp` (visualization) | `TargetDummy.h/.cpp` (testing target) |
| **Modify input** | `Content/Input/Actions/IA_*.uasset` | `MyCharacter.h/.cpp` (input functions) |
| **Add attributes** | `MyAttributeSet.h/.cpp` | Create corresponding `GameplayEffect_*.h/.cpp` |
| **Test gameplay tags** | `GameplayTagTester.h/.cpp` | `Config/Tags/GameplayTags.ini` |
| **Modify build dependencies** | `EROEOREOREOR.Build.cs` | Module header files |

### 📋 System Integration Points

| System A | System B | Integration Method | Key Files |
|----------|----------|--------------------|-----------|
| **Character** ↔ **Combat** | Component composition | `MyCharacter` owns `CombatStateMachineComponent` |
| **Character** ↔ **Movement** | GAS abilities | `MyCharacter` activates `GameplayAbility_Dash/Bounce` |
| **Combat** ↔ **GAS** | Effect application | `CombatPrototypeComponent` applies `GameplayEffect_Damage` |
| **Movement** ↔ **GAS** | Attribute tracking | `GameplayEffect_AirBounceIncrement/Reset` modify `MyAttributeSet` |
| **Input** ↔ **Abilities** | Enhanced Input → GAS | `MyCharacter` input functions → `AbilitySystemComponent->TryActivateAbility` |
| **Data** ↔ **Combat** | DataTable lookup | `CombatPrototypeComponent` reads `DT_AttackPrototypes.csv` |

### 🎯 Performance-Critical Areas

| System | Performance Consideration | Location |
|--------|--------------------------|----------|
| **Combat Hit Detection** | Shape collision queries per frame | `AttackShapeComponent.cpp` |
| **Movement Updates** | Velocity calculations in Tick | `GameplayAbility_Dash.cpp`, `VelocitySnapshotComponent.cpp` |
| **GAS Integration** | Ability activation overhead | `MyCharacter::SetupPlayerInputComponent` |
| **Debug Visualization** | Debug draw calls (disable in shipping) | `AttackShapeComponent` debug functions |

### 📦 Module Dependencies (from Build.cs)

**Core Dependencies:**
- `Core`, `CoreUObject`, `Engine` - Unreal base functionality
- `InputCore`, `EnhancedInput` - Input system
- `GameplayAbilities`, `GameplayTags`, `GameplayTasks` - GAS framework
- `UMG`, `Slate`, `SlateCore` - UI framework
- `NetCore`, `ReplicationGraph` - Networking (future multiplayer)

**Architecture Notes:**
- Single module design for tight integration
- Heavy use of composition over inheritance
- Data-driven combat system with CSV files
- Frame-accurate timing using 60fps baseline
- Component-based architecture for modularity
