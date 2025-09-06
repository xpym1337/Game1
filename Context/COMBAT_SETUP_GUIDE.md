# Combat System Setup Guide - Editor Configuration

## 🎯 Overview
This guide walks through setting up the combat modules in Unreal Editor to get combat and abilities working.

## 📋 Step 1: Configure BP_MyMyCharacter

### Open BP_MyMyCharacter Blueprint
1. Double-click `BP_MyMyCharacter` in Content Browser
2. Go to the **Components** tab (left panel)

### Add Required Components
Click **Add Component** button and add these C++ components:
- `CombatStateMachineComponent`
- `CombatPrototypeComponent` 
- `AttackShapeComponent`
- `VelocitySnapshotComponent`

### Configure AbilitySystemComponent
1. Select the **AbilitySystemComponent** in the Components panel
2. In Details panel, set:
   - **Replication Mode**: Mixed (for future multiplayer)
   - **Attribute Set Classes**: Add `MyAttributeSet`

### Set Default Attribute Values
1. Find **Default Starting Data** section
2. Add entries for each attribute:
   - Health: 100
   - MaxHealth: 100
   - Stamina: 100
   - MaxStamina: 100
   - AttackPower: 1.0
   - CriticalHitChance: 0.1

## 📋 Step 2: Grant Abilities to Character

### In BP_MyMyCharacter Event Graph:
1. Open the **Event Graph** tab
2. Find or create **BeginPlay** event

### Add Ability Granting Logic:
```blueprint
BeginPlay Event → 
  Get AbilitySystemComponent →
    Give Ability (GameplayAbility_Dash) →
    Give Ability (GameplayAbility_Bounce)
```

### Blueprint Node Setup:
1. Right-click, search for "Give Ability"
2. Connect to AbilitySystemComponent
3. In **Ability Class**, select:
   - `GameplayAbility_Dash`
   - `GameplayAbility_Bounce`
4. Set **Level**: 1
5. Store the returned **Ability Spec Handle** for later use

## 📋 Step 3: Setup Input Bindings

### Configure Enhanced Input
1. Open **BP_1ThirdPersonPlayerController**
2. In Details panel, find **Enhanced Input Local Player Subsystem**
3. Add Input Mapping Context: `NewInputMappingContext`

### Bind Abilities to Input
In BP_MyMyCharacter Event Graph:
1. Add Input Action events:
   - `IA_DashLeft` → Try Activate Ability by Class (GameplayAbility_Dash)
   - `IA_DashRight` → Try Activate Ability by Class (GameplayAbility_Dash)
   - `IA_Bounce` → Try Activate Ability by Class (GameplayAbility_Bounce)

### Input Setup Example:
```blueprint
IA_DashLeft (Started) →
  Get AbilitySystemComponent →
    Try Activate Ability by Class →
      Ability Class: GameplayAbility_Dash
```

## 📋 Step 4: Setup Combat DataTables

### Create DataTable Assets:
1. Right-click in Content Browser
2. **Miscellaneous** → **Data Table**
3. Choose Row Structure:
   - For combat actions: `FCombatActionData`
   - For attack prototypes: `FAttackPrototypeData`

### Import CSV Data:
1. Open the DataTable asset
2. Click **Reimport** button
3. Select the CSV file:
   - `Content/Data/Combat/DT_BasicCombatActions.csv`
   - `Content/Data/Combat/DT_AttackPrototypes.csv`

### Link DataTables to Components:
1. In BP_MyMyCharacter, select `CombatStateMachineComponent`
2. In Details panel, set:
   - **Combat Actions Table**: DT_BasicCombatActions
   - **Attack Prototypes Table**: DT_AttackPrototypes

## 📋 Step 5: Configure GameMode

### Setup BP_ThirdPersonGameMode:
1. Open `BP_ThirdPersonGameMode`
2. Set **Default Pawn Class**: BP_MyMyCharacter
3. Set **Player Controller Class**: BP_1ThirdPersonPlayerController

### Set as Default GameMode:
1. **Edit** → **Project Settings**
2. **Maps & Modes**
3. Set **Default GameMode**: BP_ThirdPersonGameMode

## 📋 Step 6: Create Test Target

### Setup TargetDummy:
1. Create Blueprint from `TargetDummy` C++ class
2. Name it `BP_TargetDummy`
3. Add to level for testing combat

### Configure TargetDummy:
1. Add `AbilitySystemComponent`
2. Add `MyAttributeSet`
3. Set Health: 100, MaxHealth: 100

## 📋 Step 7: Testing Combat

### Quick Test Checklist:
1. **Play in Editor (PIE)**
2. **Test Movement**: WASD keys should move character
3. **Test Dash**: Press assigned dash keys
4. **Test Bounce**: Press bounce key
5. **Check Console**: Open console (~) for debug messages

### Debug Commands:
```
showdebug abilitysystem    // Shows ability system info
showdebug combat           // Shows combat debug info
stat fps                   // Shows frame rate
```

## 🔧 Common Issues & Fixes

### Abilities Not Activating:
- Check AbilitySystemComponent is initialized
- Verify abilities are granted in BeginPlay
- Check input bindings are correct
- Ensure GameplayTags are registered

### No Damage Applied:
- Verify GameplayEffect_Damage is configured
- Check target has AbilitySystemComponent
- Ensure AttributeSet is assigned

### Input Not Working:
- Verify Input Mapping Context is added
- Check PlayerController has input enabled
- Ensure input actions are bound

## 📊 Component Configuration Reference

### CombatStateMachineComponent Settings:
- **Debug Draw**: Enable for visualization
- **Frame Rate**: 60 (for frame-based timing)
- **State Transition Time**: 0.1

### AttackShapeComponent Settings:
- **Draw Debug Shapes**: True (for testing)
- **Collision Channel**: Pawn
- **Collision Response**: Block

### VelocitySnapshotComponent Settings:
- **Snapshot Interval**: 0.016 (60fps)
- **Max History Size**: 30

## 🎯 Testing Your First Combat Ability

### Simple Attack Test:
1. Place BP_TargetDummy in level
2. PIE (Play in Editor)
3. Move near target
4. Press dash key toward target
5. Observe:
   - Character dashes
   - Debug shapes appear (if enabled)
   - Target takes damage (check output log)

### Console Testing:
```
// In PIE, open console (~) and type:
AbilitySystem.Debug.NextCategory    // Cycle debug info
GAS.ShowDebug 1                     // Show GAS debug
```

## 🚀 Next Steps

Once basic combat works:
1. Add visual effects to abilities
2. Create combo chains
3. Add audio feedback
4. Implement UI health bars
5. Create more complex attack patterns

---
*Remember: Compile C++ changes before testing!*
*Use debug visualization to understand what's happening*
