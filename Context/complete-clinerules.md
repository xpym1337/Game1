# Cline Rules for EROEOREOREOR - Unreal Engine 5 Project

## 🎯 CORE PRINCIPLE: Think Like an Industry Veteran
Before implementing ANY feature, create a comprehensive plan that includes:
1. Full dependency mapping (modules, includes, gameplay tags)
2. Integration points with existing systems
3. Performance implications and budgets
4. Data architecture requirements
5. Testing and validation strategies
6. Future extensibility considerations

## 📋 MANDATORY PRE-IMPLEMENTATION CHECKLIST

### Before Writing Any Code:
1. **Dependency Analysis** (Reference: Context/BUILD_SETTINGS.md)
   - List ALL required modules that must be in Build.cs
   - Identify all gameplay tags needed (check Context/GAMEPLAY_TAG_REGISTRY.md)
   - Map out component dependencies and communication patterns
   - Verify include file requirements follow the correct order

2. **System Integration Planning** (Reference: Context/SYSTEM_INTERFACES.md)
   - Identify all systems this feature will interact with
   - Document event flows and delegate connections
   - Plan component communication patterns
   - Consider performance implications of integration points

3. **Data Architecture Design** (Reference: Context/DATA_ARCHITECTURE.md)
   - Define all data structures needed (structs, DataTables, attributes)
   - Plan CSV structure for data-driven features
   - Document data flow from source to runtime
   - Consider hot-reload and designer iteration needs

## 🏗️ IMPLEMENTATION RULES

### Include Order (CRITICAL - ALWAYS FOLLOW)
```cpp
// MANDATORY include order for EVERY file:
#include "CoreMinimal.h"                    // ALWAYS FIRST
#include "ParentClass.h"                    // Parent class header
#include "Engine/Headers.h"                 // Engine headers
#include "GameplayTagContainer.h"           // If using tags
#include "ProjectHeaders.h"                 // Project-specific headers
#include "MyClass.generated.h"              // ALWAYS LAST
```

### Dependency Validation Comment (REQUIRED IN EVERY NEW FILE)
```cpp
// Dependency Check:
// ✓ CoreMinimal.h first
// ✓ Parent class include second  
// ✓ Required modules in Build.cs: [List modules]
// ✓ Gameplay tags registered: [List new tags]
// ✓ Forward declarations used where possible
// ✓ .generated.h last
// ✓ Documentation updated in Context/
```

### Naming Conventions (Reference: Context/NAMING_CONVENTIONS.md)
- Classes: `A` for Actors, `U` for Objects, `F` for structs, `E` for enums
- Components: `U[Name]Component` pattern
- Gameplay Abilities: `UGameplayAbility_[Name]`
- Gameplay Effects: `UGameplayEffect_[Name]`
- Blueprint classes: `BP_` prefix
- DataTables: `DT_[System][Purpose]`
- Variables: PascalCase, `b` prefix for booleans
- Gameplay Tags: `[System].[Category].[Subcategory].[Specific]`

## 🎮 UNREAL ENGINE SPECIFIC RULES

### Gameplay Ability System (GAS)
- ALWAYS check if GameplayAbilities module is in Build.cs before using GAS
- Register gameplay tags in Config/Tags/GameplayTags.ini BEFORE using them
- Use ATTRIBUTE_ACCESSORS macro for all gameplay attributes
- Cache ability handles for performance (avoid repeated lookups)
- Implement proper replication for multiplayer readiness

### Combat System (Frame-Accurate at 60fps)
- Maintain 60fps baseline for all combat timing calculations
- Use frame-based timing, not delta time for combat states
- Store frame data in DataTables for designer iteration
- Implement debug visualization for all attack shapes
- Support runtime parameter modification for rapid prototyping

### Movement System
- Preserve velocity through VelocitySnapshotComponent for combos
- Use camera-relative calculations for all directional movement
- Integrate with GAS for ability-based movement
- Support momentum transfer between systems

### Component Architecture
- Prefer composition over inheritance
- One component = one responsibility
- Use TWeakObjectPtr for actor references (prevent dangling pointers)
- Cache component references in BeginPlay (avoid FindComponent in Tick)
- Forward declare in headers, full includes only in .cpp files

## 📊 DATA-DRIVEN DESIGN REQUIREMENTS

### DataTable Integration
- All combat actions defined in CSV files
- Frame data must be designer-editable without recompiling
- Support hot-reload for rapid iteration
- Validate data integrity on load
- Document CSV structure in Context/DATA_ARCHITECTURE.md

### CSV Workflow
- Location: Content/Data/[System]/DT_*.csv
- Naming: Match struct member names exactly (PascalCase)
- Testing: Always test CSV changes in PIE before committing
- Validation: Implement data validation in PostEditChangeProperty

## 🔍 PERFORMANCE GUIDELINES

### Tick Optimization
- Avoid Tick() when possible, use timers or events
- Set appropriate tick groups (PrePhysics, PostPhysics, PostUpdateWork)
- Disable tick for components that don't need it
- Use tick intervals for non-critical updates

### Memory Management
- Mark runtime-only data as Transient
- Use object pooling for frequently spawned actors
- Clear references properly in BeginDestroy
- Monitor memory usage with stat memory

### Debug Features
- Wrap all debug code in `#if ENABLE_DRAW_DEBUG`
- Disable debug visualization in shipping builds
- Use console commands for runtime debugging
- Implement stat commands for performance monitoring

## 🧪 TESTING REQUIREMENTS

### Before Committing Any Feature:
1. Test in all build configurations (Debug, Development, Shipping)
2. Verify no compilation warnings or errors
3. Run gameplay tag validation
4. Test hot-reload functionality for data changes
5. Profile performance impact (maintain 60fps)
6. Document testing commands in code

### Testing Functions Pattern
```cpp
// Add testing functions for every new system
void TestNewSystem()
{
    UE_LOG(LogTemp, Warning, TEXT("=== NEW SYSTEM TEST ==="));
    // Comprehensive system validation
    // Log all relevant state
    // Test error conditions
}
```

## 📚 DOCUMENTATION REQUIREMENTS

### Context Files to Update:
1. **New System**: Update CODEBASE_MAP.md with file locations
2. **New Class**: Update CLASS_HIERARCHY.md with inheritance
3. **New Interface**: Update SYSTEM_INTERFACES.md with integration
4. **New Data**: Update DATA_ARCHITECTURE.md with structures
5. **New Pattern**: Update COMMON_OPERATIONS.md with examples
6. **Active Work**: Keep CURRENT_WORK_CONTEXT.md current

### Code Documentation
```cpp
/**
 * Production-ready implementation following Epic Games standards
 * Dependencies: [List all modules and systems]
 * Integration: [List integration points]
 * Performance: [Note any performance considerations]
 */
```

## 🚀 FEATURE PLANNING TEMPLATE

### For Every New Feature, Document:
```markdown
## Feature: [Name]

### Dependencies
- Modules Required: [List from Build.cs]
- Gameplay Tags: [List all tags needed]
- Components: [List component dependencies]
- Data Tables: [List data requirements]

### Integration Points
- Input System: [How it connects]
- Combat System: [Integration needs]
- Movement System: [Interactions]
- GAS: [Ability/Effect requirements]

### Implementation Steps
1. [Step with specific file changes]
2. [Step with validation points]
3. [Step with testing requirements]

### Testing Plan
- Console Commands: [List debug commands]
- Test Scenarios: [List test cases]
- Performance Targets: [FPS, memory]

### Known Risks
- [Potential issue and mitigation]
- [Performance concern and solution]
```

## 🔐 MCP BRIDGE PERMISSIONS & SAFETY

### CRITICAL: MCP Tool Execution Rules

#### ✅ AUTO-EXECUTE (No Confirmation Required) - READ-ONLY Operations:
These tools only GET information and make NO changes to the project:
- `get_project_dir` - Returns project directory path
- `get_content_dir` - Returns content directory path
- `get_actors` - Lists actors in current level
- `get_actor_details` - Gets details for specific actor
- `get_selected_actors` - Gets currently selected actors
- `get_asset` - Gets asset dimensions/info
- `find_basic_shapes` - Searches for basic shape assets
- `find_assets` - Searches for assets by name

#### 🔴 REQUIRE CONFIRMATION (ALWAYS ASK) - MODIFICATION Operations:
These tools MODIFY the project and MUST have explicit user confirmation:
- `spawn_actor` - Creates new actors in the level
- `modify_actor` - Changes actor properties
- `set_material` - Applies materials to actors
- `delete_all_static_mesh_actors` - Removes actors from scene
- `create_grid` - Spawns multiple actors in grid
- `create_town` - Generates town layout
- `create_castle` - Generates castle structure
- `run_blueprint_function` - Executes Blueprint code
- `execute_python` - Runs arbitrary Python code in Unreal

### MCP Execution Protocol:

#### For READ Operations:
1. Execute immediately when needed for information gathering
2. Log the operation for transparency
3. Use results to inform recommendations

#### For WRITE Operations:
1. STOP and explain what will be modified
2. List EXACTLY what changes will occur
3. Wait for explicit user confirmation like "yes", "proceed", "do it"
4. Only execute after clear approval
5. Provide rollback information if applicable

### Example MCP Interaction Patterns:

#### Safe Information Gathering (No Confirmation):
```
AI: "Let me check your current project structure..."
[Executes get_project_dir, get_actors, find_assets automatically]
AI: "I found the following in your project: [details]"
```

#### Modification Request (Requires Confirmation):
```
AI: "I can create a test combat setup for you. This will:
   - Spawn 3 target dummies at positions X, Y, Z
   - Delete existing test actors in the scene
   - Create a combat testing grid
   
   This will modify your current level. Should I proceed?"
   
User: "Yes, go ahead"
[Only NOW execute spawn_actor, delete_all_static_mesh_actors, etc.]
```

### MCP Safety Guidelines:

1. **Never Chain Modifications Without Confirmation**
   - Each significant modification should be confirmed
   - Batch related changes into single confirmation request
   - Explain the full scope of changes upfront

2. **Provide Undo Information**
   - After modifications, explain how to revert if needed
   - Suggest saving the level before major changes
   - Document what was changed for future reference

3. **Execute Python with EXTREME Caution**
   - `execute_python` is the most powerful and dangerous tool
   - ALWAYS show the exact code that will be executed
   - Explain what the code will do in plain English
   - Never execute Python code that could damage the project

4. **Respect Designer Workflow**
   - Don't modify DataTables without explicit permission
   - Preserve existing actor configurations unless asked
   - Maintain scene organization and naming conventions

### MCP Tool Categories:

#### 🟢 SAFE Tools (Information Only):
- Project structure queries
- Asset discovery and information
- Actor inspection and selection state
- No persistent changes to project

#### 🟡 CAUTION Tools (Scene Modifications):
- Actor spawning and deletion
- Material assignments
- Grid/pattern generation
- Reversible changes to level

#### 🔴 DANGER Tools (Code Execution):
- Blueprint function execution
- Python script execution
- Can potentially break project if misused
- Require detailed explanation before use

### Emergency Protocols:

If an MCP operation causes issues:
1. Document exactly what was executed
2. Check Unreal's undo history (Ctrl+Z)
3. Revert to last saved level if needed
4. Use version control rollback as last resort
5. Report the issue with full context

### MCP Best Practices:

1. **Start with Information Gathering**
   - Always check current state before modifying
   - Use get_actors before spawn_actor
   - Use find_assets before creating grids

2. **Batch Related Operations**
   - Group similar changes together
   - One confirmation for logical operation set
   - Reduce interruption to workflow

3. **Progressive Disclosure**
   - Start with simple operations
   - Build complexity gradually
   - Validate each step before proceeding

4. **Document Changes**
   - After MCP modifications, always state:
     - What was changed
     - Where it was changed
     - How to undo if needed
     - Next steps for testing

## 🎯 MCP BRIDGE INTEGRATION

### When Adding MCP Tools:
1. Document the tool requirement in unreal_mcp_client.py
2. Implement server-side in unreal_server_init.py
3. Test the tool thoroughly before using in production
4. Add to the dynamic extension system for future use
5. Document in Context/ for team awareness

### MCP Dynamic Extension Workflow:
1. Identify repetitive task during development
2. Document the Unreal Python API calls needed
3. Generate tool request with example code
4. Test new tool in isolated environment
5. Integrate into production workflow
6. Update documentation for team

## 🔴 CRITICAL WARNINGS

### NEVER:
- Skip dependency validation (causes build failures)
- Use FindComponentByClass in Tick (performance killer)
- Forget to register gameplay tags (causes crashes)
- Ignore include order (compilation errors)
- Mix naming conventions (maintenance nightmare)
- Hardcode values that should be data-driven
- Commit without testing all build configurations

### ALWAYS:
- Plan before implementing (think like a veteran)
- Validate dependencies before coding
- Use the Context/ documentation as reference
- Test data changes without recompiling
- Profile performance impact of new features
- Consider designer workflow and iteration speed
- Document integration points and dependencies

## 📈 CONTINUOUS IMPROVEMENT

### Code Review Checklist:
- [ ] Dependencies validated against BUILD_SETTINGS.md
- [ ] Gameplay tags exist in GAMEPLAY_TAG_REGISTRY.md
- [ ] Naming follows NAMING_CONVENTIONS.md
- [ ] Integration documented in SYSTEM_INTERFACES.md
- [ ] Performance targets met (60fps maintained)
- [ ] Data-driven where appropriate
- [ ] Debug features properly wrapped
- [ ] Documentation updated

### Refactoring Triggers:
- Component doing more than one thing → Split it
- Repeated FindComponent calls → Cache the reference
- Hardcoded values → Move to DataTable
- Deep inheritance → Refactor to composition
- Performance below 60fps → Profile and optimize

## 🏆 QUALITY STANDARDS

### Production-Ready Code Means:
1. **Robust**: Handles edge cases and errors gracefully
2. **Performant**: Maintains 60fps with headroom
3. **Maintainable**: Clear, documented, follows conventions
4. **Testable**: Includes debug commands and validation
5. **Designer-Friendly**: Data-driven with hot-reload support
6. **Scalable**: Prepared for additional features and multiplayer

### Industry Veteran Mindset:
- Always consider the next developer (it might be you in 6 months)
- Design for iteration - artists and designers will change things
- Plan for scale - what works for 10 actors must work for 100
- Optimize based on profiling, not assumptions
- Document not just what, but why
- Test early, test often, test everything

## 📊 PERFORMANCE BENCHMARKS

### Target Metrics:
- **Frame Rate**: 60 FPS minimum, 120 FPS target
- **Frame Time**: <16.67ms (60fps), <8.33ms (120fps)
- **Memory Budget**: 
  - PC: 8GB system, 4GB VRAM
  - Console: Platform-specific limits
- **Load Times**: <10 seconds for level transitions
- **Network**: <100ms latency tolerance for gameplay

### Profiling Commands:
```
stat fps          - Frame rate display
stat unit         - Frame timing breakdown
stat game         - Game thread timing
stat gpu          - GPU timing
stat memory       - Memory usage
stat streaming    - Asset streaming
profilegpu        - GPU profiler
```

## 🌐 VERSION CONTROL GUIDELINES

### Git Configuration:
```gitignore
# Unreal Engine Generated
Binaries/
DerivedDataCache/
Intermediate/
Saved/
*.VC.db
*.opensdf
*.opendb
*.sdf
*.sln
*.suo
*.xcodeproj
*.xcworkspace

# Keep these
!Content/**/*.uasset
!Content/**/*.umap
!Config/**
!Source/**
```

### Commit Message Format:
```
[Category] Brief description

- Detailed change 1
- Detailed change 2

Issues: #123
```

Categories: [Feature], [Fix], [Refactor], [Docs], [Performance], [Test]

### Branch Strategy:
- `main` - Stable, production-ready
- `develop` - Active development
- `feature/*` - New features
- `fix/*` - Bug fixes
- `experimental/*` - Prototypes

## 🎨 ASSET GUIDELINES

### Texture Standards:
- **Base Color**: 2048x2048 max for heroes, 1024x1024 for props
- **Normal Maps**: Same as base color or one level lower
- **Compression**: BC7 for quality, BC1 for performance
- **Mipmaps**: Always enabled except for UI

### Mesh Standards:
- **Triangle Count**: 
  - Heroes: 50k-100k
  - Props: 500-10k
  - Background: 100-1k
- **LODs**: Minimum 3 levels, auto-generated
- **Collision**: Simple as possible, complex only when necessary

### Material Standards:
- **Shader Complexity**: Keep under 200 instructions
- **Texture Samples**: Maximum 16 per material
- **Parameters**: Use material instances for variations

## 🚦 DEPLOYMENT CHECKLIST

### Pre-Release Validation:
- [ ] All assets optimized and within budget
- [ ] Shipping build compiles without warnings
- [ ] Performance targets met on min-spec hardware
- [ ] All debug code disabled
- [ ] Logs set to appropriate verbosity
- [ ] Package size within limits
- [ ] Platform-specific requirements met

### Post-Release Monitoring:
- [ ] Crash reporting enabled
- [ ] Analytics tracking key metrics
- [ ] Performance telemetry active
- [ ] User feedback channels open
- [ ] Hotfix pipeline ready

---

**Remember**: Every shortcut taken now is technical debt paid with interest later. Plan thoroughly, implement carefully, test comprehensively. This is the path to shipping quality games.

**Context Files Reference**:
- `Context/BUILD_SETTINGS.md` - Module dependencies and build configuration
- `Context/CLASS_HIERARCHY.md` - Class inheritance and relationships
- `Context/CODEBASE_MAP.md` - Project structure and file organization
- `Context/COMMON_OPERATIONS.md` - Code snippets and patterns
- `Context/CURRENT_WORK_CONTEXT.md` - Active development tracking
- `Context/DATA_ARCHITECTURE.md` - Data structures and flow
- `Context/GAMEPLAY_TAG_REGISTRY.md` - All registered gameplay tags
- `Context/NAMING_CONVENTIONS.md` - Project naming standards
- `Context/SYSTEM_INTERFACES.md` - System communication patterns

---

*Version 1.0 - EROEOREOREOR Project*  
*Last Updated: [Current Date]*  
*Unreal Engine 5.6 | C++ & Blueprints | MCP Bridge Enabled*