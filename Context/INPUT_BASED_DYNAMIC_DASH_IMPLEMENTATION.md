# Input-Based Dynamic Camera-Relative Dash System

## Overview
Successfully upgraded the dash system from simple left/right camera-relative movement to full input-based dynamic movement that supports complex movement patterns like circular strafing, diagonal movement, and any directional combinations.

## Problem Solved
**Original Issue:** The previous system only used camera's right vector regardless of actual movement input, making it inadequate for complex movement like circling around targets.

**Solution:** The dash now captures and preserves the player's actual movement input vector, transforming it by camera orientation each frame to maintain the intended movement direction.

## Key Changes Made

### 1. Character Input Tracking
**File: `Source/EROEOREOREOR/MyCharacter.h` & `MyCharacter.cpp`**

```cpp
// Added to MyCharacter.h
FVector2D GetCurrentMovementInput() const { return CurrentMovementInput; }

// Added input tracking in movement functions
void AMyCharacter::MoveForward(const FInputActionValue& Value)
{
    const float InputValue = Value.Get<float>();
    CurrentMovementInput.Y = InputValue; // Y = forward/backward
    // ... movement logic
}

void AMyCharacter::MoveRight(const FInputActionValue& Value)
{
    const float InputValue = Value.Get<float>();
    CurrentMovementInput.X = InputValue; // X = left/right
    // ... movement logic
}
```

### 2. Input Vector Storage
**File: `Source/EROEOREOREOR/GameplayAbility_Dash.h`**

```cpp
FVector2D InitialInputVector; // Store the input vector that triggered the dash
```

### 3. Input-Based Dash Initialization
**File: `Source/EROEOREOREOR/GameplayAbility_Dash.cpp` - `ExecuteDash()`**

```cpp
// Get the current movement input from the character
FVector2D CurrentInput = Character->GetCurrentMovementInput();

if (CurrentInput.IsNearlyZero())
{
    // Fallback to camera-relative direction if no input
    switch (Direction)
    {
    case EDashDirection::Left:
        CurrentInput = FVector2D(-1.0f, 0.0f);
        break;
    case EDashDirection::Right:
        CurrentInput = FVector2D(1.0f, 0.0f);
        break;
    }
}

// Store the input for dynamic updates
InitialInputVector = CurrentInput.GetSafeNormal();

// Transform the input by current camera orientation
FVector DashDirectionVector = (CameraRightVector * InitialInputVector.X) + (CameraForwardVector * InitialInputVector.Y);
```

### 4. Dynamic Input-Based Direction Calculation
**File: `Source/EROEOREOREOR/GameplayAbility_Dash.cpp` - `UpdateDashMovement()`**

```cpp
// Get current camera vectors (dynamic every frame)
FVector CameraForwardVector = FollowCamera->GetForwardVector();
FVector CameraRightVector = FollowCamera->GetRightVector();

// Keep movement horizontal
CameraForwardVector.Z = 0.0f;
CameraRightVector.Z = 0.0f;
CameraForwardVector.Normalize();
CameraRightVector.Normalize();

// Transform the stored input by current camera orientation
FVector DesiredDashDirection = (CameraRightVector * InitialInputVector.X) + (CameraForwardVector * InitialInputVector.Y);
DesiredDashDirection.Normalize();
```

## Behavior Transformation

### Before (Camera Right Vector Only)
- **Pure Left Dash**: Always moves in camera's negative right direction
- **Pure Right Dash**: Always moves in camera's positive right direction
- **No Support**: For diagonal, forward, backward, or circular movement patterns
- **Limited Use**: Only suitable for simple side-to-side dodging

### After (Input-Based Movement)
- **Diagonal Forward-Left**: Input (-1, +1) → Moves diagonal forward-left relative to camera
- **Diagonal Backward-Right**: Input (+1, -1) → Moves diagonal backward-right relative to camera
- **Pure Forward**: Input (0, +1) → Moves forward relative to camera
- **Pure Backward**: Input (0, -1) → Moves backward relative to camera
- **Circular Strafing**: Any input vector maintains its directional relationship to camera during rotation
- **Complex Combinations**: Supports any movement vector the player was inputting

## Real-World Usage Examples

### 1. **Circular Strafing Around Enemy**
- Player holds **W+A** (forward + left) while circling an enemy
- Input Vector: `(-1, +1)` (left + forward)
- **Dash Behavior**: Maintains forward-left direction relative to camera throughout dash
- **Result**: Dash continues the circular motion instead of breaking it with pure left movement

### 2. **Diagonal Combat Movement**
- Player moves **W+D** (forward + right) while approaching enemy
- Input Vector: `(+1, +1)` (right + forward) 
- **Dash Behavior**: Dashes diagonally forward-right relative to camera
- **Result**: Maintains aggressive forward movement while dodging

### 3. **Retreating While Strafing**
- Player moves **S+A** (backward + left) while retreating
- Input Vector: `(-1, -1)` (left + backward)
- **Dash Behavior**: Dashes diagonally backward-left relative to camera
- **Result**: Enhanced retreat with lateral displacement

### 4. **Camera Rotation During Dash**
- Player starts dash with **W+A** input
- During dash, player rotates camera 90° right
- **Behavior**: Dash smoothly curves to maintain forward-left direction relative to new camera orientation
- **Result**: Fluid, responsive dash that adapts to camera movement

## Technical Features

### Input Vector Mapping
```cpp
CurrentMovementInput.X = -1.0f; // A key (left)
CurrentMovementInput.X = +1.0f; // D key (right)
CurrentMovementInput.Y = +1.0f; // W key (forward)
CurrentMovementInput.Y = -1.0f; // S key (backward)

// Combined examples:
// W+A = FVector2D(-1, +1) → Forward-Left
// W+D = FVector2D(+1, +1) → Forward-Right  
// S+A = FVector2D(-1, -1) → Backward-Left
// S+D = FVector2D(+1, -1) → Backward-Right
```

### Frame-by-Frame Processing
1. **Capture**: Store initial input vector when dash starts
2. **Transform**: Each frame, transform input by current camera orientation
3. **Blend**: Smooth interpolation prevents jarring direction changes
4. **Apply**: Move character in the transformed direction
5. **Adapt**: System responds immediately to camera rotation during dash

### Fallback System
```cpp
if (CurrentInput.IsNearlyZero())
{
    // If no movement input detected, fall back to traditional left/right behavior
    // Ensures dash always works even when player isn't moving
}
```

## Benefits Achieved

1. **Universal Movement Support**: Handles any combination of movement input
2. **Circular Motion Compatibility**: Perfect for strafing and orbiting tactics
3. **Diagonal Movement**: Supports complex combat maneuvering
4. **Camera Responsiveness**: Maintains input relationship during camera rotation
5. **Smooth Transitions**: No jarring direction snaps during camera movement
6. **Backward Compatible**: Still works with simple left/right dashes
7. **Enhanced Gameplay**: Enables more sophisticated movement techniques

## Testing Instructions

### 1. **Complex Movement Test**
1. Hold **W+A** (forward-left) and start dash
2. **Expected**: Dash moves diagonally forward-left relative to camera
3. **Verify**: Movement feels natural and maintains diagonal direction

### 2. **Circular Motion Test**
1. Circle around an object while holding **W+A**
2. Dash during the circular motion
3. **Expected**: Dash continues circular trajectory
4. **Verify**: No jarring transition to pure left movement

### 3. **Camera Rotation During Dash Test**
1. Start dash with any diagonal input (e.g., **W+D**)
2. Rotate camera 90° during dash
3. **Expected**: Dash smoothly curves to maintain forward-right direction
4. **Verify**: Responsive to camera movement throughout dash

### 4. **All Direction Test**
1. Test dashes with: W, S, A, D, W+A, W+D, S+A, S+D
2. **Expected**: Each combination produces appropriate directional dash
3. **Verify**: All 8 cardinal/diagonal directions work correctly

### 5. **Fallback Test**
1. Stand still (no movement input) and dash
2. **Expected**: Falls back to traditional left/right behavior
3. **Verify**: System remains reliable even without input

## Configuration Options

```cpp
// In UpdateDashMovement()
float DirectionBlendSpeed = 8.0f; // Responsiveness vs smoothness balance

// More responsive (snappier): 15.0f
// Smoother (less responsive): 4.0f
// Ultra smooth: 2.0f
```

## Advanced Use Cases

This system now supports sophisticated movement techniques:
- **Combat Orbiting**: Maintaining circular strafing patterns through dashes
- **Diagonal Retreats**: Backing away while maintaining lateral movement
- **Dynamic Repositioning**: Adapting dash direction based on current movement context
- **Fluid Camera Work**: Players can adjust camera mid-dash without losing movement intent

The implementation transforms the dash from a simple left/right dodge into a versatile movement tool that enhances any playstyle while maintaining the responsive, dynamic feel of camera-relative controls.
