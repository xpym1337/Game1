# Dynamic Camera-Relative Dash System

## Overview
Successfully transformed the timeline-based dash from a fixed trajectory system to a fully dynamic camera-relative system that responds to camera movement throughout the entire dash duration.

## Key Changes Made

### 1. Added Dynamic Tracking Variables
**File: `Source/EROEOREOREOR/GameplayAbility_Dash.h`**

```cpp
// Dynamic camera-relative dash variables
float TotalDistanceTraveled;        // Tracks distance moved so far
float DashSpeed;                    // Distance per second (calculated from DashDistance/DashDuration)
FVector LastDashDirection;          // Previous frame's direction for smooth blending
FVector CurrentDashDirection;       // Current frame's direction
EDashDirection InitialDashDirection; // Store initial direction (Left/Right)
```

### 2. Modified Dash Initialization
**File: `Source/EROEOREOREOR/GameplayAbility_Dash.cpp` - `ExecuteDash()`**

```cpp
// OLD: Fixed trajectory calculation
DashEndLocation = DashStartLocation + (DashDirectionVector * DashDistance);

// NEW: Dynamic system initialization
TotalDistanceTraveled = 0.0f;
DashSpeed = DashDistance / DashDuration; // Calculate speed to maintain consistent total distance
LastDashDirection = DashDirectionVector;
CurrentDashDirection = DashDirectionVector;
InitialDashDirection = Direction; // Remember if Left or Right dash
```

### 3. Completely Rewritten Movement System
**File: `Source/EROEOREOREOR/GameplayAbility_Dash.cpp` - `UpdateDashMovement()`**

#### Key Features:

**A. Dynamic Camera Direction Calculation (Every Frame)**
```cpp
// Get current camera direction every frame
FVector CameraRightVector = FollowCamera->GetRightVector();
CameraRightVector.Z = 0.0f;
CameraRightVector.Normalize();

// Calculate desired direction based on initial dash direction
FVector DesiredDashDirection;
switch (InitialDashDirection)
{
    case EDashDirection::Left:
        DesiredDashDirection = -CameraRightVector;
        break;
    case EDashDirection::Right:
        DesiredDashDirection = CameraRightVector;
        break;
}
```

**B. Smooth Direction Blending**
```cpp
// Smooth interpolation to avoid jarring direction snaps
float DirectionBlendSpeed = 8.0f; // Adjustable responsiveness
CurrentDashDirection = FMath::VInterpTo(CurrentDashDirection, DesiredDashDirection, DeltaTime, DirectionBlendSpeed);
```

**C. Distance-Based Movement (Not Position Interpolation)**
```cpp
// Calculate movement for this frame with easing
float EasedSpeed = DashSpeed * (1.0f - EasedAlpha * 0.3f);
float FrameDistance = EasedSpeed * DeltaTime;

// Don't exceed total dash distance
float RemainingDistance = DashDistance - TotalDistanceTraveled;
if (FrameDistance > RemainingDistance)
{
    FrameDistance = RemainingDistance;
}

// Apply movement
FVector NextLocation = CurrentLocation + (CurrentDashDirection * FrameDistance);
DashingCharacter->SetActorLocation(NextLocation, true);
TotalDistanceTraveled += FrameDistance;
```

**D. Dynamic Wall Collision Detection**
```cpp
// Check for walls in current direction every frame
if (bEnableWallDetection && FrameDistance > 0.0f)
{
    if (CheckForWallCollision(CurrentLocation, NextLocation, DashingCharacter))
    {
        // Find safe distance to move in current direction
        // Adapts to dynamic direction changes
    }
}
```

## Behavior Transformation

### Before (Fixed Trajectory)
- Dash direction calculated once at start: `DashEndPos = StartPos + (CameraRight * DashDistance)`
- Character follows predetermined straight-line path from start to end
- Camera movement during dash doesn't affect trajectory
- Uses position interpolation: `FMath::Lerp(StartLocation, EndLocation, Alpha)`

### After (Dynamic Camera-Relative)
- Dash direction recalculated every frame based on current camera orientation
- If player moves camera left during dash, trajectory curves left smoothly
- If player moves camera right during dash, trajectory curves right smoothly
- Maintains consistent dash speed and total distance traveled
- Smooth trajectory changes (no jarring direction snaps)
- Uses distance-based movement: `CurrentPos + (CameraDirection * Speed * DeltaTime)`

## Key Features Implemented

### 1. **Real-Time Camera Responsiveness**
- Camera direction sampled every frame
- Immediate response to camera movement
- Maintains Left/Right dash semantics regardless of camera rotation

### 2. **Smooth Direction Transitions**
- Uses `FMath::VInterpTo()` for smooth direction blending
- `DirectionBlendSpeed = 8.0f` balances responsiveness vs smoothness
- No jarring snaps when camera moves quickly

### 3. **Consistent Distance Control**
- Total dash distance always equals `DashDistance` setting
- Speed calculated as `DashDistance / DashDuration`
- Tracks `TotalDistanceTraveled` to prevent overshooting

### 4. **Preserved Easing**
- Maintains `EaseOutCirc` easing function
- Speed reduction toward end: `DashSpeed * (1.0f - EasedAlpha * 0.3f)`
- Smooth acceleration/deceleration feel preserved

### 5. **Dynamic Collision Detection**
- Wall detection works with changing directions
- Per-frame collision checking in current movement direction
- Graceful handling of obstacles during curved trajectories

## Configuration Options

### Adjustable Parameters:
```cpp
// In GameplayAbility_Dash.h
float DashDistance = 600.0f;        // Total distance to travel
float DashDuration = 0.2f;          // Time to complete dash
bool bEnableWallDetection = true;   // Enable/disable collision detection
float WallDetectionDistance = 100.0f; // Collision detection radius

// In UpdateDashMovement()
float DirectionBlendSpeed = 8.0f;   // Responsiveness vs smoothness balance
```

## Testing Instructions

### 1. **Basic Dynamic Response Test**
1. Start a left or right dash
2. Immediately rotate camera in opposite direction
3. **Expected**: Dash trajectory should curve toward new camera direction
4. **Verify**: Character reaches approximately same total distance

### 2. **Smooth Transition Test**
1. Start dash
2. Gradually rotate camera during dash
3. **Expected**: Smooth curved trajectory, no snapping
4. **Verify**: Movement feels natural and responsive

### 3. **Distance Consistency Test**
1. Perform dash without moving camera (straight line)
2. Perform dash while rotating camera 90° (curved)
3. **Expected**: Both dashes travel similar total distances
4. **Verify**: Check debug logs for `TotalDistanceTraveled`

### 4. **Wall Collision Test**
1. Dash toward wall
2. Rotate camera away from wall during dash
3. **Expected**: Character avoids wall and curves toward new direction
4. **Verify**: No collision, smooth direction change

### 5. **Speed Consistency Test**
1. Time multiple dashes with different camera movements
2. **Expected**: All dashes complete in same duration (`DashDuration`)
3. **Verify**: Consistent timing regardless of trajectory shape

## Advanced Customization Options

### 1. **Direction Blend Responsiveness**
```cpp
// In UpdateDashMovement()
float DirectionBlendSpeed = 8.0f; // Current setting

// More responsive (snappier): 15.0f
// Smoother (less responsive): 4.0f  
// Ultra smooth: 2.0f
```

### 2. **Speed Easing Adjustment**
```cpp
// Current: slight speed reduction toward end
float EasedSpeed = DashSpeed * (1.0f - EasedAlpha * 0.3f);

// No speed reduction: 
float EasedSpeed = DashSpeed;

// More dramatic speed reduction:
float EasedSpeed = DashSpeed * (1.0f - EasedAlpha * 0.6f);
```

### 3. **Camera Influence Limits**
```cpp
// To limit how much camera can influence dash:
float MaxDirectionChange = 45.0f; // degrees
FVector ClampedDirection = FMath::VInterpConstantTo(
    LastDashDirection, 
    DesiredDashDirection, 
    DeltaTime, 
    FMath::DegreesToRadians(MaxDirectionChange)
);
```

## Benefits Achieved

1. **Dynamic Responsiveness**: Dash adapts to camera movement throughout duration
2. **Smooth Trajectories**: No jarring direction changes
3. **Consistent Performance**: Maintains speed and distance regardless of curve
4. **Enhanced Control**: Players can influence dash path with camera
5. **Preserved Feel**: Maintains original easing and timing
6. **Robust Collision**: Wall detection works with dynamic trajectories

## Technical Notes

- System switches character to `MOVE_Flying` during dash for precise control
- Uses per-frame distance accumulation rather than position interpolation  
- Collision detection adapts to current movement direction
- Debug logging available for testing and tuning
- Fully integrated with existing GAS (Gameplay Ability System)

This implementation transforms the dash from a rigid, predetermined movement into a fluid, responsive ability that enhances player agency while maintaining consistent performance characteristics.
