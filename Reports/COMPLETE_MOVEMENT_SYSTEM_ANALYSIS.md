# Complete Movement System Analysis

## Movement System Architecture Overview

Your movement system uses a hybrid approach combining:
- **Enhanced Input System** for input handling
- **Camera-relative movement** for directional control
- **Gameplay Ability System (GAS)** for dash abilities
- **Timeline-based dash updates** for dynamic dash behavior

---

## 1. INPUT HANDLING CODE FOR HORIZONTAL MOVEMENT (A/D KEYS)

### Input Processing Flow:
```cpp
// In MyCharacter.cpp - SetupPlayerInputComponent()
if (MoveLeftAction)
    EnhancedInputComponent->BindAction(MoveLeftAction, ETriggerEvent::Triggered, this, &AMyCharacter::MoveLeft);
if (MoveRightAction)
    EnhancedInputComponent->BindAction(MoveRightAction, ETriggerEvent::Triggered, this, &AMyCharacter::MoveRight);
```

### A Key (Move Left) Processing:
```cpp
void AMyCharacter::MoveLeft(const FInputActionValue& Value)
{
    const float InputValue = Value.Get<float>();
    
    // Update movement input tracking (X = left/right, negative for left)
    CurrentMovementInput.X = -InputValue;
    
    if (FollowCamera != nullptr && FMath::Abs(InputValue) > 0.0f)
    {
        // Get camera right direction (projected to ground)
        FVector CameraRight = FollowCamera->GetRightVector();
        CameraRight.Z = 0.0f; // Remove vertical component
        CameraRight.Normalize();
        
        // Move left (negative right - strafe left relative to camera)
        AddMovementInput(CameraRight, -InputValue);
    }
}
```

### D Key (Move Right) Processing:
```cpp
void AMyCharacter::MoveRight(const FInputActionValue& Value)
{
    const float InputValue = Value.Get<float>();
    
    // Update movement input tracking (X = left/right, positive for right)
    CurrentMovementInput.X = InputValue;
    
    if (FollowCamera != nullptr && FMath::Abs(InputValue) > 0.0f)
    {
        // Get camera right direction (projected to ground)
        FVector CameraRight = FollowCamera->GetRightVector();
        CameraRight.Z = 0.0f; // Remove vertical component
        CameraRight.Normalize();
        
        // Move right (positive right - strafe right relative to camera)
        AddMovementInput(CameraRight, InputValue);
    }
}
```

### When Inputs Take Effect:
- **Immediately** on `ETriggerEvent::Triggered`
- Inputs are processed **every frame** while held
- `AddMovementInput()` feeds directly into Unreal's movement system
- `CurrentMovementInput` is updated for dash system reference

---

## 2. DASH IMPLEMENTATION CODE

### Dash Trigger System:
```cpp
// In MyCharacter.cpp - Dash activation
void AMyCharacter::DashLeft(const FInputActionValue& Value)
{
    // Try to activate dash ability with left direction
    FGameplayTagContainer AbilityTags;
    AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Dash")));
    
    if (!AbilitySystemComponent->TryActivateAbilitiesByTag(AbilityTags))
    {
        // Fallback implementation...
    }
}
```

### Core Dash Execution:
```cpp
// In GameplayAbility_Dash.cpp - ExecuteDash()
void UGameplayAbility_Dash::ExecuteDash(AMyCharacter* Character, EDashDirection Direction)
{
    // Get the current movement input from the character
    FVector2D CurrentInput = Character->GetCurrentMovementInput();
    
    // Store the input for dynamic updates
    InitialInputVector = CurrentInput.GetSafeNormal();
    InitialDashDirection = Direction;

    // Calculate dash direction using input and camera orientation
    FVector DashDirectionVector = (CameraRightVector * InitialInputVector.X) + (CameraForwardVector * InitialInputVector.Y);
    DashDirectionVector.Normalize();

    // Calculate dash speed (convert distance/duration to velocity)
    DashSpeed = DashDistance / DashDuration;
    
    // Apply the dash velocity - works with movement system
    FVector DashVelocity = DashDirectionVector * DashSpeed;
    MovementComponent->Velocity = DashVelocity;
}
```

### Dash Duration & Variables:
- **Duration**: `DashDuration = 0.2f` (200ms)
- **Distance**: `DashDistance = 600.0f` units
- **Cooldown**: `DashCooldown = 2.0f` seconds
- **Speed**: Calculated as `DashDistance / DashDuration` = 3000 units/second

### What Triggers Dash:
1. `DashLeft`/`DashRight` input actions (`ETriggerEvent::Completed`)
2. GAS ability activation with "Ability.Dash" tag
3. Requires `CommitAbility()` success (cooldowns, resources, etc.)

---

## 3. MOVEMENT STATE MANAGEMENT

### State Tracking Variables:
```cpp
// In MyCharacter.h
private:
    bool bIsShiftPressed = false;
    bool bCanDash = true;
    FTimerHandle DashCooldownTimer;
    FVector2D CurrentMovementInput;  // Tracks current WASD input

// In GameplayAbility_Dash.h
private:
    bool bIsDashing;
    class AMyCharacter* DashingCharacter;
    EDashDirection InitialDashDirection;
    FVector2D InitialInputVector;
```

### GAS State Tags:
```cpp
// Gameplay Tags for state management
FGameplayTag DashingStateTag = "State.Dashing";
FGameplayTag DashCooldownTag = "Effect.Cooldown";

// Applied during dash
ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Dashing")));
ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Dashing")));
```

### State Transitions:
- **Normal Movement** → **Dashing**: GAS ability activation
- **Dashing** → **Normal Movement**: Timer completion or ability end
- **No State Machine**: Uses boolean flags and GAS tags

---

## 4. MOVEMENT UPDATE LOOP EXECUTION ORDER

### Main Tick Function:
```cpp
// In MyCharacter.cpp
void AMyCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);  // Calls parent Character::Tick() - handles movement, physics, etc.
}
```

### Frame-by-Frame Execution Order:
1. **Input Processing** (Enhanced Input System)
   - `MoveLeft/MoveRight()` called on input
   - `CurrentMovementInput` updated
   - `AddMovementInput()` called

2. **Character::Tick()** (Unreal Engine)
   - Processes accumulated movement input
   - Updates velocity and position
   - Handles physics, collisions

3. **Dash System Updates** (if dashing)
   - `UpdateDashMovement()` called via timer
   - Modifies `MovementComponent->Velocity` directly
   - Overrides normal movement input

---

## 🚨 CRITICAL ISSUE IDENTIFIED: THE ROOT CAUSE OF MID-DASH DIRECTION CHANGES

### The Problem:
**Input processing continues during dash, causing conflicting movement commands!**

```cpp
// PROBLEMATIC: These functions run DURING dash and call AddMovementInput
void AMyCharacter::MoveLeft(const FInputActionValue& Value) {
    // NO CHECK FOR DASHING STATE!
    CurrentMovementInput.X = -InputValue;
    AddMovementInput(CameraRight, -InputValue);  // ← This conflicts with dash!
}

void AMyCharacter::MoveRight(const FInputActionValue& Value) {
    // NO CHECK FOR DASHING STATE!
    CurrentMovementInput.X = InputValue;
    AddMovementInput(CameraRight, InputValue);   // ← This conflicts with dash!
}
```

### What Happens Frame-by-Frame During Dash:
1. **Frame 1**: Dash starts, sets velocity to 3000 units/sec right
2. **Frame 2**: Player presses A, `MoveLeft()` calls `AddMovementInput(left)`
3. **Frame 3**: Unreal's movement system blends dash velocity + input movement
4. **Frame 4**: `UpdateDashMovement()` tries to restore dash velocity
5. **Frame 5**: Input still active, creates tug-of-war between systems

---

## SPECIFIC TECHNICAL ANSWERS

### 1. How is dash direction determined and stored?
- **Calculated once** at dash start using `Character->GetCurrentMovementInput()`
- **Stored in** `InitialInputVector` and `InitialDashDirection`
- **Continuously updated** by camera orientation in `UpdateDashMovement()`

### 2. Input buffering/queuing systems?
- **No explicit buffering** - relies on Enhanced Input System
- **Inputs ARE processed during dash** (this is the bug!)
- No input queue or state-based input blocking

### 3. Movement variables locked during dash?
- **NOT LOCKED!** This is the core issue
- `CurrentMovementInput` still updates during dash
- `AddMovementInput()` still called during dash
- Only dash ability prevents new dash activation

### 4. Collision/physics constraints?
- Same collision system for both normal movement and dash
- Optional wall detection in dash system (currently unused)
- No physics mode changes during dash

---

## MOVEMENT-RELATED VARIABLES & PURPOSES

```cpp
// Character Movement (Normal)
float BaseTurnRate = 65.f;              // Camera turn speed
float BaseLookUpRate = 65.f;            // Camera pitch speed
FVector2D CurrentMovementInput;         // Current WASD input state

// Dash Properties
float DashDistance = 600.0f;            // Distance per dash
float DashDuration = 0.3f;              // How long dash lasts
float DashCooldown = 2.0f;              // Time between dashes
bool bIsShiftPressed = false;           // Shift modifier tracking
bool bCanDash = true;                   // Cooldown state

// Dash Internal State
bool bIsDashing;                        // Currently dashing flag
float DashSpeed;                        // Calculated velocity
EDashDirection InitialDashDirection;    // Left/Right
FVector2D InitialInputVector;           // Stored input direction
FTimerHandle DashUpdateTimer;           // Timeline update
FTimerHandle DashTimer;                 // Dash duration timer
```

---

## INPUT PRIORITY & OVERRIDE BEHAVIOR

### Current System (Problematic):
- **No Priority System** - all inputs processed simultaneously
- **Dash + Movement Input = Conflict** - they fight each other
- **Camera Rotation** - always takes priority (separate system)

### What Should Happen:
1. **During Dash**: Block normal movement input processing
2. **Camera Input**: Always allowed
3. **Jump Input**: Should be allowed/blocked based on design
4. **New Dash Input**: Blocked by GAS tags (working correctly)

---

## RECOMMENDATIONS FOR FIXES

### 1. Add Dash State Checks to Movement Functions:
```cpp
void AMyCharacter::MoveLeft(const FInputActionValue& Value)
{
    // Check if currently dashing
    if (AbilitySystemComponent && AbilitySystemComponent->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.Dashing"))))
    {
        return; // Don't process movement input during dash
    }
    
    // Rest of function...
}
```

### 2. Create Input State Management:
```cpp
UENUM(BlueprintType)
enum class EMovementInputState : uint8
{
    Normal,
    Dashing,
    Stunned
};
```

### 3. Add Debug Logging:
```cpp
// In movement functions
UE_LOG(LogTemp, Warning, TEXT("MoveLeft called - IsDashing: %s, InputValue: %f"), 
    bIsDashing ? TEXT("TRUE") : TEXT("FALSE"), InputValue);
```

This analysis reveals that your mid-dash direction change issue is caused by the continued processing of normal movement input during dash execution, creating a conflict between the dash velocity system and the regular movement input system.
