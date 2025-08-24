# Proper GAS Setup Guide - Production Workflow

## ✅ Changes Made
- **Removed input blocking** from all movement functions (MoveForward, MoveBackward, MoveLeft, MoveRight)
- **Camera rotation working** - Character now always faces camera direction
- **Movement input flows freely** - No more C++ blocking during abilities

## 🎯 Required Editor Setup (Do This Next)

### Step 1: Create GameplayEffects
Create these GameplayEffect assets in your Content Browser:

#### A. GE_DashState
1. **Right-click in Content Browser** → Create → Blueprint → GameplayEffect
2. **Name**: `GE_DashState`
3. **Open the asset** and configure:
   - **Duration Policy**: Has Duration
   - **Duration**: 0.8 seconds (match your DashDuration)
   - **Granted Tags**: Add `State.Dashing`
   - **Application Tag Requirements**: None needed

#### B. GE_BounceState  
1. **Create**: Another GameplayEffect called `GE_BounceState`
2. **Configure**:
   - **Duration Policy**: Instant (or very short duration like 0.1s)
   - **Granted Tags**: Add `State.Bouncing`

#### C. GE_ResetAirBounce
1. **Create**: GameplayEffect called `GE_ResetAirBounce`
2. **Configure**:
   - **Duration Policy**: Instant
   - **Modifiers**: Add one modifier
     - **Attribute**: AirBounceCount
     - **Modifier Op**: Override 
     - **Magnitude**: Scalable Float = 0.0

#### D. GE_IncrementAirBounce  
1. **Create**: GameplayEffect called `GE_IncrementAirBounce`
2. **Configure**:
   - **Duration Policy**: Instant
   - **Modifiers**: Add one modifier
     - **Attribute**: AirBounceCount
     - **Modifier Op**: Add
     - **Magnitude**: Scalable Float = 1.0

### Step 2: Create Ability Blueprint Assets
Create Blueprint classes that extend your C++ abilities:

#### A. GA_Dash_BP
1. **Right-click** → Create → Blueprint → Class
2. **Parent Class**: Search for "GameplayAbility_Dash" (your C++ class)
3. **Name**: `GA_Dash_BP`
4. **Open and configure**:
   - **Ability Tags**: `Ability.Dash`
   - **Activation Blocked Tags**: `State.Stunned`, `Input.Blocked`
   - **Activation Owned Tags**: `State.Dashing`
   - **Cancel Abilities with Tags**: Leave empty (unless you want dash to cancel other abilities)

#### B. GA_Bounce_BP
1. **Create**: Blueprint extending "GameplayAbility_Bounce"
2. **Name**: `GA_Bounce_BP`  
3. **Configure**:
   - **Ability Tags**: `Ability.Bounce`
   - **Activation Blocked Tags**: `State.Stunned`, `Input.Blocked`
   - **Activation Owned Tags**: `State.Bouncing`

### Step 3: Update C++ Ability Classes
Reference the GameplayEffects you created:

#### In GameplayAbility_Dash.h:
```cpp
// Add these properties:
UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
TSubclassOf<UGameplayEffect> DashStateEffect;

UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Dash Settings")
float DashSpeed = 1875.0f;

UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Dash Settings")  
float DashDuration = 0.8f;
```

#### In GameplayAbility_Bounce.h:
```cpp
// Add these properties:
UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
TSubclassOf<UGameplayEffect> BounceStateEffect;

UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Bounce Settings")
float BounceUpwardVelocity = 800.0f;

UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Bounce Settings")
int32 MaxAirBounces = 2;
```

### Step 4: Set Values in Blueprint Ability Assets

#### A. Configure GA_Dash_BP:
1. **Open GA_Dash_BP**
2. **In Details Panel**:
   - **Dash State Effect**: Set to your `GE_DashState` asset
   - **Dash Speed**: 1875.0 (or whatever you prefer)
   - **Dash Duration**: 0.8
3. **Compile and Save**

#### B. Configure GA_Bounce_BP:
1. **Open GA_Bounce_BP** 
2. **In Details Panel**:
   - **Bounce State Effect**: Set to your `GE_BounceState` asset
   - **Bounce Upward Velocity**: 800.0
   - **Max Air Bounces**: 2
   - **Air Bounce Reset Effect**: Set to your `GE_ResetAirBounce` asset
   - **Air Bounce Increment Effect**: Set to your `GE_IncrementAirBounce` asset
3. **Compile and Save**

### Step 5: Update Character Blueprint
Instead of granting abilities in C++, do it in Blueprint:

#### A. Create/Open Your Character Blueprint
1. **Find your Character Blueprint** (likely `BP_MyCharacter` or similar)
2. **Open BeginPlay event**
3. **Add nodes**:
   - **Give Ability** node (from Ability System Component)
   - **Ability**: Set to `GA_Dash_BP`  
   - **Ability Level**: 1
   
   - **Give Ability** node (second one)
   - **Ability**: Set to `GA_Bounce_BP`
   - **Ability Level**: 1

#### B. Remove C++ Ability Granting
Later, you'll remove the ability granting from C++ BeginPlay and PossessedBy functions.

## 🔄 Proper GAS Workflow Benefits

### What This Achieves:
1. **Designers can tune values** without C++ recompilation
2. **Abilities use proper GAS state management** via GameplayEffects
3. **No more input blocking issues** - GAS handles conflicts via tags
4. **Dash-bounce combos work** because abilities can activate during each other
5. **Network replication works properly** because you're using GAS correctly
6. **Easier debugging** - You can see GameplayEffects in the GAS debugger

### Tag-Based Ability Control:
- Want to prevent bounce during dash? Add `State.Dashing` to bounce's **Activation Blocked Tags**
- Want to allow dash-bounce combos? Remove that tag requirement
- All controllable in Blueprint without C++ changes

## 🎮 Testing Your Setup

### After Creating Everything:
1. **Compile your project** 
2. **Play in editor**
3. **Test dash** - Should apply `State.Dashing` tag (visible in GAS debugger)
4. **Test bounce** - Should work during or after dash
5. **Check console logs** - Should see proper GAS activation messages

### Debug Commands:
- `showdebug abilitysystem` - Shows active GameplayEffects and tags
- `AbilitySystem.DebugAbilityTags 1` - Shows ability activation details

## 📝 Next Steps After Setup
1. **Remove C++ ability granting** code from BeginPlay/PossessedBy
2. **Move more ability values** to Blueprint (cooldowns, ranges, etc.)
3. **Create GameplayEffects for cooldowns** instead of using timers
4. **Add visual/audio cues** in Blueprint events
5. **Set up ability icons and UI** in Blueprint

This workflow separates **systems** (C++) from **content** (Blueprint), following Epic's GAS best practices used in Fortnite, Paragon, and other shipped titles.
