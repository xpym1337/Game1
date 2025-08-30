# Gameplay Tags System - Comprehensive Testing Guide

## Overview
This guide provides step-by-step instructions for testing the Gameplay Tags system setup in your GAS project.

## Prerequisites
✅ GameplayTags.ini created in Config/Tags/
✅ GameplayTagTester C++ classes created
✅ Project compiled successfully
✅ GameplayTags module included in build configuration

---

## TEST PHASE 1: Tag Visibility Verification

### Step 1.1: Verify Tags in Project Settings
1. Open Unreal Editor
2. Go to **Edit > Project Settings**
3. Navigate to **Game > Gameplay Tags**
4. Verify the following tags are visible:

#### Expected Tags in Project Settings:
- `Ability` (Root tag for all abilities)
  - `Ability.Dash` (Dash ability)
  - `Ability.Attack` (Basic attack ability)  
  - `Ability.Jump` (Enhanced jump ability)
  - `Ability.Passive` (Root for passive abilities)
- `State` (Root tag for character states)
  - `State.Dashing` (Currently performing dash)
  - `State.Attacking` (Currently attacking)
  - `State.Stunned` (Stunned and cannot act)
  - `State.InAir` (Airborne state)
  - `State.Movement` (Root for movement states)
- `Effect` (Root tag for gameplay effects)
  - `Effect.Cooldown` (Cooldown prevention)
  - `Effect.Cost` (Resource consumption)
  - `Effect.Damage` (Damage dealing)
  - `Effect.Buff` (Positive effects)
  - `Effect.Debuff` (Negative effects)
- `Input` (Root tag for input states)
  - `Input.Blocked` (Input blocking)

**SUCCESS CRITERIA:** All tags visible with proper hierarchy and descriptions

### Step 1.2: Create Test Blueprint
1. In Content Browser, right-click and create **Blueprint Class > Actor**
2. Name it `BP_TagTester`
3. Open the Blueprint
4. In **Variables** panel, click **+ Variable**
5. Set variable name to `TestTag`
6. Set variable type to **GameplayTag**
7. In the dropdown next to the variable, you should see all your created tags

**SUCCESS CRITERIA:** All created tags appear in Blueprint dropdown menus

### Step 1.3: Test Multiple Tag Variables
Create additional GameplayTag variables in BP_TagTester:
- `AbilityTag` (should show Ability.* tags)
- `StateTag` (should show State.* tags)
- `EffectTag` (should show Effect.* tags)
- `InputTag` (should show Input.* tags)

**SUCCESS CRITERIA:** Each variable's dropdown shows the complete tag hierarchy

---

## TEST PHASE 2: Tag Functionality Testing

### Step 2.1: Create C++ Test Blueprint
1. In Content Browser, create **Blueprint Class**
2. Choose **Gameplay Tag Tester** as parent class (from our C++ class)
3. Name it `BP_GameplayTagTester`
4. Compile and save

### Step 2.2: Test Tag Variables in Details Panel
1. Select BP_GameplayTagTester in viewport
2. In **Details** panel, find **Tag Testing** section
3. Set each test tag variable:
   - `TestAbilityDash` → `Ability.Dash`
   - `TestAbilityAttack` → `Ability.Attack`
   - `TestAbilityJump` → `Ability.Jump`
   - `TestStateDashing` → `State.Dashing`
   - `TestStateAttacking` → `State.Attacking`
   - `TestStateStunned` → `State.Stunned`
   - `TestStateInAir` → `State.InAir`
   - `TestEffectCooldown` → `Effect.Cooldown`
   - `TestEffectCost` → `Effect.Cost`
   - `TestEffectDamage` → `Effect.Damage`
   - `TestInputBlocked` → `Input.Blocked`

**SUCCESS CRITERIA:** All tag assignments work without errors

### Step 2.3: Test Tag Container
1. In Details panel, find `TestTagContainer`
2. Add multiple tags to the container:
   - Add `Ability.Dash`
   - Add `State.Dashing`
   - Add `Effect.Cooldown`

**SUCCESS CRITERIA:** Tag container accepts multiple tags successfully

### Step 2.4: Runtime Tag Testing
1. Place BP_GameplayTagTester in the level
2. Play in Editor (PIE)
3. Open **Output Log** (Window > Output Log)
4. Select the BP_GameplayTagTester actor
5. In Details panel, find the **Tag Testing** functions:
   - Click `RunBasicTagTests`
   - Click `RunAdvancedTagTests`

**Expected Log Output:**
```
=== STARTING BASIC TAG TESTS ===
GameplayTagTester: Added tag 'Ability.Dash' to ASC
GameplayTagTester: Added tag 'Ability.Attack' to ASC
GameplayTagTester: Current ASC Tags (2 total):
  - Ability.Dash
  - Ability.Attack
GameplayTagTester: ASC HAS tag 'Ability.Dash'
GameplayTagTester: ASC HAS tag 'Ability.Attack'
GameplayTagTester: ASC does NOT have tag 'State.Dashing'
=== BASIC TAG TESTS COMPLETE ===

=== STARTING ADVANCED TAG TESTS ===
GameplayTagTester: ASC HAS all tags in container (2 tags)
GameplayTagTester: ASC does NOT have all tags in container (3 tags)
GameplayTagTester: ASC HAS any tags in container (3 tags)
=== ADVANCED TAG TESTS COMPLETE ===
```

**SUCCESS CRITERIA:** All tag operations execute without errors and produce expected results

---

## TEST PHASE 3: Integration Testing

### Step 3.1: Test with Existing Character
1. Open `BP_MyMyCharacter` Blueprint
2. In the Construction Script or BeginPlay:
   - Get Ability System Component
   - Add a gameplay tag (e.g., `State.InAir`)
3. Create a function to check if character has the tag
4. Test in PIE

### Step 3.2: Test Tag Queries
Create Blueprint functions to test:
- `Has Matching Gameplay Tag`
- `Has All Matching Gameplay Tags`  
- `Has Any Matching Gameplay Tags`

### Step 3.3: Test Tag Events
1. Create an event that triggers when specific tags are added/removed
2. Test using `OnGameplayTagCountChanged` delegate

**SUCCESS CRITERIA:** Tags integrate seamlessly with existing GAS setup, no conflicts or errors

---

## POST-COMPLETION VERIFICATION CHECKLIST

### ✅ Configuration Verification
- [ ] GameplayTags.ini exists and is properly formatted
- [ ] All required tags are defined with descriptions
- [ ] Project Settings shows all tags correctly
- [ ] No compilation errors in C++ code

### ✅ Editor Integration
- [ ] Tags appear in all Blueprint dropdowns
- [ ] FGameplayTag variables can be created and assigned
- [ ] FGameplayTagContainer works with multiple tags
- [ ] Tag hierarchies display correctly

### ✅ Runtime Functionality
- [ ] Tags can be added to AbilitySystemComponent
- [ ] Tags can be removed from AbilitySystemComponent
- [ ] Tag queries return correct results
- [ ] Tag containers work with query functions
- [ ] Debug logging shows tag operations correctly

### ✅ GAS Integration
- [ ] No conflicts with existing AbilitySystemComponent
- [ ] Character tags work in PIE environment
- [ ] Tag system ready for ability implementation

---

## Troubleshooting

### Tags Not Appearing in Dropdowns
1. Ensure GameplayTags.ini is in correct location
2. Restart Unreal Editor
3. Check for syntax errors in GameplayTags.ini
4. Verify GameplayTags module is included in Build.cs

### Compilation Errors
1. Check all #include statements
2. Ensure GameplayTags module is in PublicDependencyModuleNames
3. Regenerate project files
4. Clean and rebuild project

### Tags Not Working at Runtime
1. Verify AbilitySystemComponent is properly initialized
2. Check that InitAbilityActorInfo is called
3. Ensure tags are valid before using
4. Check Output Log for error messages

---

## Success Confirmation

When all tests pass, your Gameplay Tags system is ready for:
- ✅ Ability implementation with proper tagging
- ✅ State management through tags
- ✅ Effect categorization and filtering
- ✅ Input blocking and ability interrupts
- ✅ Complex gameplay logic using tag queries

The foundation is now established for advanced GAS features!
