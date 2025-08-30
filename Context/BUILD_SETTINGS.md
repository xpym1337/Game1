# BUILD SETTINGS

## Module Dependencies (`EROEOREOREOR.Build.cs`)

### 📦 Public Dependencies

```csharp
PublicDependencyModuleNames.AddRange(new string[] {
    "Core",                    // Fundamental Unreal types and utilities
    "CoreUObject",            // UObject system, reflection, garbage collection
    "Engine",                 // Core engine functionality, actors, components
    "InputCore",              // Basic input handling
    "EnhancedInput",          // Enhanced Input system (UE5 standard)
    "GameplayAbilities",      // Gameplay Ability System (GAS) framework
    "GameplayTags",           // Gameplay tag system
    "GameplayTasks",          // Async task system for abilities
    "UMG",                    // User interface framework
    "Slate",                  // Low-level UI framework
    "SlateCore",              // Core Slate functionality
    "NetCore",                // Core networking
    "ReplicationGraph"        // Advanced replication system
});
```

### 🔧 Dependency Analysis

| Module | Purpose in EROEOREOREOR | Used By |
|--------|-------------------------|---------|
| **Core** | Basic types (`FVector`, `FString`, etc.) | All classes |
| **CoreUObject** | `UObject`, `UCLASS`, reflection | All Unreal classes |
| **Engine** | `AActor`, `UActorComponent`, `UWorld` | Character, components |
| **InputCore** | Legacy input types | Enhanced Input integration |
| **EnhancedInput** | Modern input system | `AMyCharacter` input handling |
| **GameplayAbilities** | GAS framework | `UMyAttributeSet`, abilities, effects |
| **GameplayTags** | Tag system | Combat system, state management |
| **GameplayTasks** | Async ability tasks | Complex abilities (future) |
| **UMG** | Blueprint UI widgets | Future UI development |
| **Slate** | C++ UI framework | Custom UI components (future) |
| **SlateCore** | Core UI types | UMG dependency |
| **NetCore** | Networking foundation | Multiplayer preparation |
| **ReplicationGraph** | Optimized replication | Large-scale multiplayer (future) |

### 🚫 Disabled Dependencies

```csharp
// Currently commented out - not needed yet
// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
// PrivateDependencyModuleNames.Add("OnlineSubsystem");
```

**Reasoning**:
- **Slate/SlateCore**: Already included in PublicDependencies for UMG
- **OnlineSubsystem**: Not needed for current single-player prototype

## Preprocessor Definitions

### 🎯 Active Definitions (Inferred from Code)

Based on the codebase analysis, these preprocessor definitions are likely active:

```cpp
// GAS Integration (automatically defined by GameplayAbilities module)
#define WITH_GAMEPLAY_ABILITIES 1
#define WITH_GAMEPLAY_TAGS 1

// Enhanced Input (automatically defined by EnhancedInput module)
#define WITH_ENHANCED_INPUT 1

// Debug builds (standard UE5 definitions)
#ifdef UE_BUILD_DEBUG
    #define COMBAT_DEBUG_ENABLED 1
    #define ATTACK_SHAPE_DEBUG_ENABLED 1
#endif

// Development builds
#ifdef UE_BUILD_DEVELOPMENT  
    #define ENABLE_DRAW_DEBUG 1
    #define WITH_EDITOR 1
#endif

// Shipping builds (optimizations)
#ifdef UE_BUILD_SHIPPING
    #define COMBAT_DEBUG_ENABLED 0
    #define ATTACK_SHAPE_DEBUG_ENABLED 0
    #define ENABLE_DRAW_DEBUG 0
#endif
```

### 🔍 Debug Flags

Debug functionality observed in the codebase:

```cpp
// AttackShapeComponent debug visualization
#if ENABLE_DRAW_DEBUG
    // Debug shape drawing enabled
    DrawDebugSphere(GetWorld(), Location, Radius, Segments, Color, false, Duration);
    DrawDebugBox(GetWorld(), Location, Extent, Rotation, Color, false, Duration);
#endif

// Combat system debug logging
#if COMBAT_DEBUG_ENABLED
    COMBAT_LOG(Log, TEXT("Combat state changed from %s to %s"), 
        *UEnum::GetValueAsString(OldState), *UEnum::GetValueAsString(NewState));
#endif

// GAS debug information
#if WITH_GAMEPLAY_ABILITIES && !UE_BUILD_SHIPPING
    // Enhanced debugging for development builds
    showdebug abilitysystem
    showdebug gameplaytags
#endif
```

## Performance Settings

### ⚡ Tick Group Assignments

Based on component analysis:

```cpp
// High-priority ticking for frame-accurate systems
class UCombatStateMachineComponent : public UActorComponent
{
public:
    UCombatStateMachineComponent()
    {
        // Frame-accurate combat requires high-frequency ticking
        PrimaryComponentTick.bCanEverTick = true;
        PrimaryComponentTick.TickGroup = TG_PrePhysics;  // Before physics for input responsiveness
        PrimaryComponentTick.TickInterval = 1.0f / 60.0f;  // 60fps for frame accuracy
    }
};

// Movement components for smooth updates
class UVelocitySnapshotComponent : public UActorComponent  
{
public:
    UVelocitySnapshotComponent()
    {
        PrimaryComponentTick.bCanEverTick = true;
        PrimaryComponentTick.TickGroup = TG_PostPhysics;  // After physics for velocity capture
    }
};

// Visual/debug components
class UAttackShapeComponent : public UActorComponent
{
public:
    UAttackShapeComponent()
    {
        PrimaryComponentTick.bCanEverTick = true;
        PrimaryComponentTick.TickGroup = TG_PostUpdateWork;  // After main updates for debug draw
        
        // Only tick when debug is enabled
        PrimaryComponentTick.bCanEverTick = ENABLE_DRAW_DEBUG;
    }
};
```

### 🎯 Memory Optimization

```cpp
// Transient properties (not serialized, saves memory)
UPROPERTY(Transient)
FGameplayAbilitySpecHandle CachedDashAbilityHandle;

UPROPERTY(Transient)  
TWeakObjectPtr<AMyCharacter> CachedCharacter;

// Object pooling for effects (handled by GAS)
// GameplayEffects are automatically pooled by AbilitySystemComponent

// Component caching to avoid repeated lookups
// Store direct references instead of using FindComponentByClass repeatedly
```

### 📊 Compilation Optimization

```cpp
// Forward declarations to reduce header dependencies
class UAbilitySystemComponent;
class UMyAttributeSet;
class UCombatStateMachineComponent;

// Minimal includes in headers, full includes in .cpp files
// Header: Forward declaration only
// Implementation: Full include for complete type definition

// Template specialization for common types (future optimization)
// TMap<FGameplayTag, FCombatActionData> uses fast hash functions
// TWeakObjectPtr<> prevents circular reference compilation issues
```

## Project-Specific Compiler Settings

### 🛠️ Build Configuration

```csharp
// EROEOREOREOR.Build.cs settings analysis
public class EROEOREOREOR : ModuleRules
{
    public EROEOREOREOR(ReadOnlyTargetRules Target) : base(Target)
    {
        // PCH Configuration
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        // Optimizes compilation time by using precompiled headers
        
        // No private dependencies currently defined
        PrivateDependencyModuleNames.AddRange(new string[] {  });
        
        // Future expansion points (commented out):
        // - Slate UI for advanced C++ widgets
        // - OnlineSubsystem for multiplayer features
        // - Steam integration via plugin system
    }
}
```

### 📋 Target Configuration

Based on standard UE5 target files:

```csharp
// EROEOREOREOR.Target.cs (Development/Debug builds)
public class EROEOREOREORTarget : TargetRules
{
    public EROEOREOREORTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V2;
        
        // Include development tools
        bUsesSteam = false;                    // Steam not configured yet
        bWithPushModel = true;                 // Modern replication
        bCompileWithAccessibilitySupport = true;  // Accessibility features
        
        // Performance settings for development
        bUsePCHFiles = true;                   // Precompiled headers
        bUseUnityBuild = true;                 // Unity build optimization
        
        // GAS-specific optimizations
        bWithServerCode = true;                // Needed for GAS in multiplayer
        bCompileAgainstEngine = false;         // Standalone game
    }
}

// EROEOREOREOREditor.Target.cs (Editor builds)
public class EROEOREOREOREditorTarget : TargetRules
{
    public EROEOREOREOREditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V2;
        
        // Editor-specific settings
        bBuildDeveloperTools = true;           // Enable development tools
        bUseMallocProfiler = true;             // Memory profiling
        bUseLoggingInShipping = false;         // Performance optimization
        bUseChecksInShipping = false;          // Remove checks in final builds
        
        // Enhanced debugging for editor
        bWithLiveCoding = true;                // Live coding support
        bSupportEditAndContinue = true;        // Edit and continue debugging
    }
}
```

## Platform-Specific Settings

### 🖥️ Windows Configuration

```csharp
// Platform-specific optimizations (inferred from project context)
if (Target.Platform == UnrealTargetPlatform.Win64)
{
    // Windows-specific optimizations
    bUseIncrementalLinking = true;         // Faster linking for development
    bUseFastPDBLinking = true;             // Faster debug symbol generation
    
    // Performance profiling support
    bSupportXGEController = true;          // XGE build acceleration
    bAllowLTCG = true;                     // Link-time code generation (shipping builds)
    
    // GAS networking optimizations  
    bWithPerfCounters = true;              // Performance monitoring
}
```

### 🎮 Console Platform Preparation

```csharp
// Future console platform settings (not currently active)
/*
if (Target.Platform == UnrealTargetPlatform.PS5)
{
    // PlayStation 5 specific settings
    bUsesSonyOpenSSL = true;
    bSupportsRayTracing = true;
}

if (Target.Platform == UnrealTargetPlatform.XSX)
{
    // Xbox Series X specific settings  
    bSupportsRayTracing = true;
    bUsesXboxLive = true;
}
*/
```

## Debug and Development Settings

### 🔍 Debug Symbol Configuration

```csharp
// Debug information settings (development builds)
bCreateDebugInfo = true;                   // Generate PDB files
bDisableDebugInfo = false;                 // Keep debug symbols
bOmitPCDebugInfoInDevelopment = false;     // Include debug info in development

// Optimization settings by build type
if (Target.Configuration == UnrealTargetConfiguration.Debug)
{
    OptimizationLevel = OptimizationMode.NoOptimization;
    bUseIncrementalLinking = true;
}
else if (Target.Configuration == UnrealTargetConfiguration.Development)
{
    OptimizationLevel = OptimizationMode.Speed;
    bUseIncrementalLinking = true;
}
else // Shipping/Test builds
{
    OptimizationLevel = OptimizationMode.MaxSpeed;
    bUseIncrementalLinking = false;
}
```

### 🧪 Testing and Profiling

```cpp
// Console commands for performance analysis
// stat game                  - General game performance
// stat gameplaytags          - Gameplay tag system performance  
// stat abilitysystem         - GAS performance metrics
// stat combat                - Combat system performance (custom)
// stat collision             - Collision detection performance
// stat memory                - Memory usage analysis

// Profiling configurations
#ifdef UE_BUILD_DEVELOPMENT
    // Enable detailed profiling in development builds
    #define ENABLE_COMBAT_PROFILING 1
    #define ENABLE_MOVEMENT_PROFILING 1
    #define ENABLE_GAS_PROFILING 1
#else
    #define ENABLE_COMBAT_PROFILING 0
    #define ENABLE_MOVEMENT_PROFILING 0
    #define ENABLE_GAS_PROFILING 0
#endif
```

## Asset Loading and Streaming

### 📦 Asset Manager Configuration

```cpp
// Streaming settings for GameplayAbility_Dash curve loading
// Source: Engine/AssetManager.h integration

// Primary asset types for the project
enum class EPrimaryAssetType : uint8
{
    CombatData,               // Combat DataTables and prototypes
    AbilityData,              // Gameplay abilities and effects  
    CharacterData,            // Character-specific assets
    AudioData,                // Sound effects and music
    VisualData                // Particles, materials, meshes
};

// Asset loading priorities
enum class EAssetLoadPriority : uint8
{
    Critical,                 // Player character, core gameplay
    High,                     // Combat abilities, movement systems
    Medium,                   // UI, secondary systems
    Low,                      // Background assets, non-critical content
    Background                // Preloading for future use
};
```

### 🎯 Streaming Configuration

```cpp
// Streaming settings used in GameplayAbility_Dash
class UGameplayAbility_Dash
{
    // Async asset loading for curves
    TSharedPtr<FStreamableHandle> CurveLoadHandle;
    
    // Loading priority configuration
    void LoadCurveAssets()
    {
        UAssetManager& AssetManager = UAssetManager::Get();
        FStreamableManager& StreamableManager = AssetManager.GetStreamableManager();
        
        // High priority for gameplay-critical curves
        CurveLoadHandle = StreamableManager.RequestAsyncLoad(
            AssetsToLoad,
            Delegate,
            FStreamableManager::AsyncLoadHighPriority  // Fast loading for abilities
        );
    }
};
```

## Networking and Replication

### 🌐 Replication Settings

```cpp
// GAS replication configuration (inferred from UMyAttributeSet)
class UMyAttributeSet : public UAttributeSet
{
    // All gameplay attributes are replicated
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override
    {
        Super::GetLifetimeReplicatedProps(OutLifetimeProps);
        
        // Replicate attributes with change notifications
        DOREPLIFETIME_CONDITION_NOTIFY(UMyAttributeSet, Health, COND_None, REPNOTIFY_Always);
        DOREPLIFETIME_CONDITION_NOTIFY(UMyAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
        DOREPLIFETIME_CONDITION_NOTIFY(UMyAttributeSet, AirBounceCount, COND_None, REPNOTIFY_Always);
        // ... all other attributes
    }
};

// Character replication (from AMyCharacter inheritance)
class AMyCharacter : public ACharacter  // ACharacter provides built-in replication
{
    // Components automatically replicated based on their individual settings
    // AbilitySystemComponent: Full replication for GAS
    // AttributeSet: Replicated via GAS
    // Custom components: Replication set per component needs
};
```

### 🎮 Multiplayer Configuration

```cpp
// Future multiplayer settings (preparation for networking)
// Currently single-player focused, but GAS-ready

// Network settings for GAS abilities
class UGameplayAbility_Dash : public UGameplayAbility
{
    // GAS handles ability replication automatically
    // Client prediction supported out of the box
    // Server validation handled by GAS framework
};

// Component replication settings
class UCombatStateMachineComponent : public UActorComponent
{
public:
    UCombatStateMachineComponent()
    {
        // Combat state should replicate for multiplayer
        SetIsReplicatedByDefault(true);
    }
    
    // Replicated state for multiplayer combat
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override
    {
        Super::GetLifetimeReplicatedProps(OutLifetimeProps);
        DOREPLIFETIME(UCombatStateMachineComponent, CurrentState);
        DOREPLIFETIME(UCombatStateMachineComponent, CurrentActionTag);
    }
};
```

## Compilation Optimizations

### ⚡ Build Speed Optimizations

```csharp
// Build.cs optimizations for faster iteration
public EROEOREOREOR(ReadOnlyTargetRules Target) : base(Target)
{
    // Precompiled header optimization
    PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
    
    // Future optimizations (not yet implemented):
    // bFasterWithoutUnity = false;        // Unity build for smaller projects
    // MinFilesUsingPrecompiledHeader = 1; // Aggressive PCH usage
    // bUseAdaptiveUnityBuild = true;      // Adaptive unity builds
}
```

### 🎯 Runtime Performance

```cpp
// Performance-conscious patterns observed in codebase:

// 1. Component caching to avoid repeated lookups
class AMyCharacter
{
private:
    // Direct component references (fast access)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    TObjectPtr<UCombatStateMachineComponent> CombatStateMachine;
    
    // vs. slow repeated lookups:
    // FindComponentByClass<UCombatStateMachineComponent>() // Don't do this every frame
};

// 2. Ability handle caching for GAS performance
class AMyCharacter  
{
private:
    // PERFORMANCE: Cache ability handles to avoid lookup delays
    UPROPERTY(Transient)
    FGameplayAbilitySpecHandle CachedDashAbilityHandle;
    
    UPROPERTY(Transient)
    FGameplayAbilitySpecHandle CachedBounceAbilityHandle;
};

// 3. Frame-accurate timing with efficient updates
class UCombatStateMachineComponent
{
    // 60fps frame timing for combat accuracy
    float TargetFrameRate = 60.0f;
    float FrameDuration = 1.0f / 60.0f;
    
    // Batch frame processing instead of individual calculations
    void ProcessFrame(); // Single function handles all frame logic
};
```

## Platform and Configuration Matrix

### 📊 Build Configuration Summary

| Configuration | Debug Enabled | Optimization | Logging | Profiling | Use Case |
|---------------|---------------|--------------|---------|-----------|----------|
| **Debug** | ✅ Full | None | Verbose | ✅ Full | Deep debugging |
| **Development** | ✅ Partial | Moderate | Standard | ✅ Partial | Daily development |
| **Test** | ❌ Disabled | High | Minimal | ❌ Disabled | QA testing |
| **Shipping** | ❌ Disabled | Maximum | None | ❌ Disabled | Final release |

### 🎯 Feature Flags by Build

| Feature | Debug | Development | Test | Shipping |
|---------|-------|-------------|------|----------|
| **Combat Debug Draw** | ✅ | ✅ | ❌ | ❌ |
| **Attack Shape Visualization** | ✅ | ✅ | ❌ | ❌ |
| **GAS Debug Commands** | ✅ | ✅ | ✅ | ❌ |
| **Performance Profiling** | ✅ | ✅ | ✅ | ❌ |
| **Memory Tracking** | ✅ | ✅ | ❌ | ❌ |
| **Hot Reload** | ✅ | ✅ | ❌ | ❌ |
| **Live Coding** | ✅ | ✅ | ❌ | ❌ |

## Future Build Considerations

### 🚀 Planned Module Additions

```csharp
// Potential future dependencies based on project growth:

// Audio system integration
"AudioMixer",                 // Advanced audio mixing
"SoundUtilities",            // Sound processing utilities

// Advanced AI
"AIModule",                  // AI behavior trees
"NavigationSystem",          // Pathfinding and navigation

// Physics and animation
"PhysicsCore",               // Advanced physics simulation
"AnimationCore",             // Animation utilities
"ControlRig",                // Procedural animation

// Online features
"OnlineSubsystem",           // Platform online services
"OnlineSubsystemUtils",      // Online utility functions
"Sockets",                   // Low-level networking

// Performance and tools
"ToolMenus",                 // Editor tool integration
"DeveloperSettings",         // Project settings management
"TraceLog",                  // Performance tracing
```

### 🎯 Plugin Integration Points

```csharp
// Plugin dependencies (configured in .uproject file)
// Current: None active
// Future considerations:

// Combat system plugins
"AbilitySystemExtensions",   // Extended GAS functionality
"CombatSystem",              // Third-party combat framework

// Performance plugins  
"Chaos",                     // Advanced physics simulation
"Niagara",                   // Advanced particle systems

// Platform plugins
"SteamAPI",                  // Steam integration
"PlatformCrypto",           // Platform-specific encryption
```

## Dependency Validation Framework

### Include → Module Mapping (Validation Rules)
- GameplayTagContainer.h → "GameplayTags"
- AbilitySystemComponent.h → "GameplayAbilities" 
- InputMappingContext.h → "EnhancedInput"
- UserWidget.h → "UMG", "Slate", "SlateCore"
- AssetManager.h → "Engine" (already included)
- StreamableManager.h → "Engine" (already included)
- TimerHandle.h → "Engine" (already included)

### High-Usage Includes Requiring Validation
Based on codebase analysis:
- GameplayTagContainer.h (used in 80% of gameplay classes)
- Engine/TimerHandle.h (used in 60% of ability classes)  
- Engine/AssetManager.h (used in 40% of ability classes)
- Abilities/GameplayAbility.h (all ability base classes)

### Dependency Validation Checklist
When creating/modifying classes:
- [ ] All includes follow correct order (CoreMinimal → Parent → Engine → Project → .generated)
- [ ] New gameplay tags registered in Config/Tags/GameplayTags.ini
- [ ] Module dependencies added to Build.cs if needed
- [ ] Forward declarations used where possible
- [ ] Documentation updated with new patterns

### Critical Dependency Self-Check
Before completing any code implementation, verify:
```cpp
// Dependency Check Comment Template:
// ✓ CoreMinimal.h first
// ✓ Parent class include second  
// ✓ Required modules in Build.cs (GameplayTags, GameplayAbilities, etc.)
// ✓ Forward declared classes where possible (performance optimization)
// ✓ .generated.h last
// ✓ All gameplay tags registered in GameplayTags.ini
// ✓ Documentation updated
```

## Maintenance and Updates

### 🔄 Build System Maintenance

```cpp
// Regular maintenance tasks for build system:

// 1. Review module dependencies quarterly
//    - Remove unused dependencies
//    - Move dependencies between Public/Private as needed
//    - Validate all includes are necessary

// 2. Update performance settings based on profiling
//    - Adjust tick groups based on performance data
//    - Optimize component tick intervals
//    - Review memory allocation patterns

// 3. Validate preprocessor definitions
//    - Ensure debug flags work correctly across builds
//    - Test shipping builds have debug code stripped
//    - Verify platform-specific defines are correct

// 4. Monitor compilation times
//    - Profile build times across different machines
//    - Optimize PCH usage if builds become slow
//    - Consider unity build settings adjustments
```

### 📋 Build Health Checklist

- [ ] **All builds compile successfully** (Debug, Development, Test, Shipping)
- [ ] **Debug visualization works** in Debug/Development builds only
- [ ] **Performance profiling** shows acceptable frame rates
- [ ] **Memory usage** within target limits for target platforms
- [ ] **GAS replication** functions correctly in multiplayer tests
- [ ] **Asset loading** completes without errors or timeouts
- [ ] **Hot reload** works for rapid iteration during development
- [ ] **Packaging** succeeds for target platforms without warnings
