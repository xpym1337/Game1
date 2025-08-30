# Gameplay Tag Registry

> Documentation of actual gameplay tags defined in Config/Tags/GameplayTags.ini
> Updated: August 30, 2025
> Project: EROEOREOREOR (Unreal Engine 5 Action Game)

## Tag Registration Process

1. Add new tags to `Config/Tags/GameplayTags.ini`
2. Update this document with usage notes
3. Validate usage in relevant classes
4. Test tag functionality in-game
5. Reference Context/GAMEPLAY_TAG_REGISTRY.md for validation

## Current Project Tags (From GameplayTags.ini)

### Ability System Tags

#### Root Ability Tags
- **Ability** - Root tag for all abilities

#### Core Abilities
- **Ability.Dash** - Dash ability providing rapid directional movement
- **Ability.Attack** - Basic attack ability for melee combat
- **Ability.Jump** - Enhanced jump ability with GAS integration
- **Ability.Bounce** - Bounce ability preserving horizontal momentum

#### Combo Abilities  
- **Ability.Combo** - Root tag for combination abilities
- **Ability.Combo.DashBounce** - Dash-Bounce combo execution
- **Ability.Combo.JumpBounce** - Jump-Bounce combo execution

#### Future Expansion
- **Ability.Passive** - Root for passive abilities

### State Management Tags

#### Root State Tag
- **State** - Root tag for character states

#### Movement States
- **State.Dashing** - Character currently performing dash
- **State.InAir** - Character is airborne (jumping/falling)
- **State.Bouncing** - Character currently performing bounce
- **State.Movement** - Root for movement-related states

#### Action States
- **State.Attacking** - Character currently in attack animation
- **State.Stunned** - Character cannot act (stunned state)

### Effect System Tags

#### Root Effect Tag
- **Effect** - Root tag for gameplay effects

#### Core Effects
- **Effect.Cooldown** - Cooldown effect preventing ability usage
- **Effect.Cost** - Resource consumption effect
- **Effect.Damage** - Damage dealing effect

#### Air Bounce Effects
- **Effect.AirBounce** - Root tag for air bounce effects
- **Effect.AirBounce.Increment** - Increments air bounce count by 1
- **Effect.AirBounce.Reset** - Resets air bounce count to 0

#### Effect Types
- **Effect.Buff** - Root for positive effects
- **Effect.Debuff** - Root for negative effects

### Cooldown Management Tags

#### Root Cooldown Tag
- **Cooldown** - Root tag for cooldown effects

#### Ability Cooldowns
- **Cooldown.Dash** - Dash ability cooldown (prevents spam)
- **Cooldown.Bounce** - Bounce ability cooldown (prevents spam)

### Immunity Tags

#### Root Immunity Tag
- **Immune** - Root tag for immunity effects

#### Ability Immunities
- **Immune.Dash** - Immunity to dash effects (prevents interruption)
- **Immune.Bounce** - Immunity to bounce effects (prevents interruption)

### Input System Tags

#### Root Input Tag
- **Input** - Root tag for input-related states

#### Input States
- **Input.Blocked** - Input blocked during interrupts
- **Input.Bounce** - Bounce input (Shift + Jump combination)

#### Dash Input Directions
- **Input.Dash** - Root tag for dash input states
- **Input.Dash.Left** - Left dash input direction
- **Input.Dash.Right** - Right dash input direction

### Combat System Tags (Frame-Accurate Combat)

#### Root Combat Tag
- **Combat** - Root tag for advanced combat system

#### Combat Actions
- **Combat.Action** - Root tag for combat actions

#### Light Attacks
- **Combat.Action.Light** - Light attacks (fast, low commitment)
- **Combat.Action.Light.Jab** - Quick jab attack (12f startup, 6f active, 18f recovery)
- **Combat.Action.Light.Cross** - Cross punch (10f startup, 4f active, 20f recovery)  
- **Combat.Action.Light.Hook** - Hook punch (15f startup, 8f active, 22f recovery)

#### Heavy Attacks
- **Combat.Action.Heavy** - Heavy attacks (slow, high commitment)
- **Combat.Action.Heavy.Overhead** - Overhead smash (25f startup, 12f active, 35f recovery)
- **Combat.Action.Heavy.Sweep** - Leg sweep (20f startup, 15f active, 30f recovery)

#### Special Attacks
- **Combat.Action.Special** - Special attacks (unique mechanics)
- **Combat.Action.Special.Launcher** - Launch attack (sends enemy airborne)
- **Combat.Action.Ultimate** - Ultimate attacks (highest commitment)

#### Combat State Machine
- **Combat.State** - Combat state machine tags
- **Combat.State.Startup** - Attack startup phase (wind-up frames)
- **Combat.State.Active** - Attack active phase (hit detection frames)
- **Combat.State.Recovery** - Attack recovery phase (cool-down frames)
- **Combat.State.CancelWindow** - Currently in a cancel window
- **Combat.State.PerfectCancel** - Perfect cancel timing achieved

#### Combat Priority System
- **Combat.Priority** - Combat priority levels
- **Combat.Priority.Light** - Light attack priority (can be interrupted)
- **Combat.Priority.Heavy** - Heavy attack priority (more resistant to interrupts)
- **Combat.Priority.Special** - Special attack priority (hard to interrupt)
- **Combat.Priority.Ultimate** - Ultimate priority (cannot be interrupted)

#### Combat Weight Classes
- **Combat.Weight** - Attack weight classes
- **Combat.Weight.Light** - Light weight (fast recovery, low impact)
- **Combat.Weight.Medium** - Medium weight (balanced commitment)
- **Combat.Weight.Heavy** - Heavy weight (slow recovery, high impact)
- **Combat.Weight.Ultimate** - Ultimate weight (maximum commitment and impact)

### Momentum System Tags

#### Root Momentum Tag
- **Momentum** - Root tag for momentum transfer system

#### Momentum Sources
- **Momentum.Dash** - Momentum from dash ability
- **Momentum.Jump** - Momentum from jump ability
- **Momentum.Fall** - Momentum from falling

### Damage System Tags

#### Root Damage Tag
- **Damage** - Root damage type tag

#### Damage Types
- **Damage.Type** - Damage type classification
- **Damage.Type.Physical** - Physical damage type
- **Damage.Type.Elemental** - Elemental damage type

#### Damage Calculations
- **Damage.Base** - Base damage calculation tag
- **Damage.Critical** - Critical hit damage tag

### Cancel System Tags

#### Root Cancel Tag
- **Cancel** - Root tag for canceling system

#### Cancel Types
- **Cancel.Perfect** - Perfect cancel timing achieved
- **Cancel.Window** - Currently in cancel window

## Tag Usage by Class

### GameplayAbility_Dash
**Required Tags:**
- State.Dashing (applied during dash execution)
- Cooldown.Dash (cooldown management)
- Immune.Dash (immunity to interruption)

**Optional Tags:**
- Momentum.Dash (momentum tracking)
- Input.Dash.Left/Right (input direction tracking)

### GameplayAbility_Bounce  
**Required Tags:**
- State.Bouncing (applied during bounce execution)
- State.InAir (character state tracking)
- Cooldown.Bounce (cooldown management)
- Immune.Bounce (immunity to interruption)

**Effects Used:**
- Effect.AirBounce.Increment (increments bounce count)
- Effect.AirBounce.Reset (resets bounce count on landing)

### MyCharacter
**State Management:**
- State.* tags for animation system integration
- Input.* tags for input system state
- Ability.* tags for ability availability

**Combat Integration:**
- Combat.State.* tags for combat animation states
- Combat.Priority.* tags for interrupt system

### Combat System (CombatStateMachineComponent)
**Core Combat Tags:**
- Combat.Action.* tags for action identification
- Combat.State.* tags for state machine management
- Combat.Priority.* tags for interrupt resolution
- Combat.Weight.* tags for commitment levels

**Cancel System:**
- Cancel.Window (cancel availability)
- Cancel.Perfect (perfect cancel execution)

### GAS Effects
**GameplayEffect_AirBounceIncrement:**
- Effect.AirBounce.Increment (effect identification)

**GameplayEffect_AirBounceReset:**
- Effect.AirBounce.Reset (effect identification)

**GameplayEffect_Damage:**
- Effect.Damage (damage effect marker)
- Damage.Type.* tags (damage type classification)

## Tag Validation Rules

### For Cline Implementation

When implementing any code that uses gameplay tags:

1. **Check Existing Tags First**
   - Always reference this document before creating new tags
   - Use FGameplayTag::RequestGameplayTag() for existing tags
   - Validate tag exists: `if (MyTag.IsValid())`

2. **Required Include Pattern**
   ```cpp
   #include "CoreMinimal.h"                    // ALWAYS FIRST
   #include "ParentClass.h"                    // Parent class
   #include "GameplayTagContainer.h"           // REQUIRED for tags
   #include "MyClass.generated.h"              // ALWAYS LAST
   ```

3. **Tag Usage Pattern**
   ```cpp
   // Cache tags as static members for performance
   static const FGameplayTag TAG_State_Dashing = FGameplayTag::RequestGameplayTag(FName("State.Dashing"));
   
   // Validate before use
   if (TAG_State_Dashing.IsValid())
   {
       // Use tag safely
       ASC->AddLooseGameplayTag(TAG_State_Dashing);
   }
   ```

4. **Dependency Self-Check**
   ```cpp
   // Dependency Check:
   // ✓ GameplayTagContainer.h included
   // ✓ "GameplayTags" module in Build.cs
   // ✓ Tag registered in GameplayTags.ini
   // ✓ Tag documented in Context/GAMEPLAY_TAG_REGISTRY.md
   // ✓ Tag validation implemented
   ```

### New Tag Creation Process

1. **Design Review**
   - Ensure tag fits existing hierarchy
   - Check for similar existing tags
   - Plan tag relationships and dependencies

2. **Implementation Steps**
   - Add to `Config/Tags/GameplayTags.ini`
   - Update this documentation
   - Implement usage in relevant classes
   - Add validation and error checking
   - Test functionality in-game

3. **Documentation Requirements**
   - Purpose and usage description
   - Associated classes and systems
   - Integration requirements
   - Example usage code

## Common Tag Patterns

### State Management Pattern
```cpp
// Remove old state, add new state
ASC->RemoveLooseGameplayTag(OldStateTag);
ASC->AddLooseGameplayTag(NewStateTag);

// Check current state
if (ASC->HasMatchingGameplayTag(StateTag))
{
    // Handle state-specific logic
}
```

### Ability Activation Pattern
```cpp
// Check cooldown before activation
if (!ASC->HasMatchingGameplayTag(CooldownTag))
{
    // Activate ability
    ASC->TryActivateAbilityByClass(AbilityClass);
}
```

### Combat Priority Resolution
```cpp
// Check if current action can be interrupted
FGameplayTag CurrentPriorityTag = GetActionPriorityTag(CurrentAction);
FGameplayTag NewPriorityTag = GetActionPriorityTag(NewAction);

bool bCanInterrupt = ComparePriority(CurrentPriorityTag, NewPriorityTag);
```

## Tag Performance Considerations

### Optimization Guidelines

1. **Cache Frequently Used Tags**
   - Store as static const FGameplayTag members
   - Initialize once, use many times
   - Avoid repeated RequestGameplayTag() calls

2. **Use Tag Containers Efficiently**
   - Pre-allocate containers for known tag sets
   - Use HasAny() and HasAll() for multi-tag queries
   - Filter containers for hierarchical queries

3. **Minimize Tag Operations**
   - Batch tag additions/removals when possible
   - Use gameplay effects for temporary tags
   - Cache tag query results when appropriate

## Maintenance Schedule

### Weekly Review
- [ ] Verify all documented tags exist in GameplayTags.ini
- [ ] Check for unused tags in codebase
- [ ] Update usage examples based on new implementations

### Monthly Audit
- [ ] Review tag hierarchy for optimization opportunities
- [ ] Validate all classes correctly implement tag usage
- [ ] Update documentation with new patterns discovered

### Release Validation
- [ ] Confirm all tags compile and validate correctly
- [ ] Test tag functionality across all systems
- [ ] Update version information and change log

---

*Last Updated: August 30, 2025*  
*Generated by: AI Assistant (Cline)*  
*Source: Config/Tags/GameplayTags.ini*  
*Maintainer: Development Team*
