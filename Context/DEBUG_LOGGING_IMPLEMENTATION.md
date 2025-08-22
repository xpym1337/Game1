# Debug Logging Implementation for Movement System Analysis

## Purpose
Add comprehensive logging to track exactly when inputs are received, when dash starts/ends, and how they interact frame-by-frame.

## Debug Logging Code to Add

### 1. Enhanced Movement Input Logging

Add these debug logs to your movement functions in `MyCharacter.cpp`:

```cpp
void AMyCharacter::MoveLeft(const FInputActionValue& Value)
{
    const float InputValue = Value.Get<float>();
    
    // DEBUG: Check if we're currently dashing
    bool bCurrentlyDashing = AbilitySystemComponent && 
        AbilitySystemComponent->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.Dashing")));
    
    UE_LOG(LogTemp, Warning, TEXT("🔍 MoveLeft called - InputValue: %.2f, CurrentlyDashing: %s, CurrentMovementInput.X: %.2f"), 
        InputValue, bCurrentlyDashing ? TEXT("TRUE") : TEXT("FALSE"), CurrentMovementInput.X);
    
    // Update movement input tracking (X = left/right, negative for left)
    CurrentMovementInput.X = -InputValue;
    
    if (FollowCamera != nullptr && FMath::Abs(InputValue) > 0.0f)
    {
        // Get camera right direction (projected to ground)
        FVector CameraRight = FollowCamera->GetRightVector();
        CameraRight.Z = 0.0f; // Remove vertical component
        CameraRight.Normalize();
        
        UE_LOG(LogTemp, Log, TEXT("📍 MoveLeft - Adding movement input: Direction=%s, Scale=%.2f"), 
            *(-CameraRight).ToString(), -InputValue);
        
        // Move left (negative right - strafe left relative to camera)
        AddMovementInput(CameraRight, -InputValue);
    }
}

void AMyCharacter::MoveRight(const FInputActionValue& Value)
{
    const float InputValue = Value.Get<float>();
    
    // DEBUG: Check if we're currently dashing
    bool bCurrentlyDashing = AbilitySystemComponent && 
        AbilitySystemComponent->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.Dashing")));
    
    UE_LOG(LogTemp, Warning, TEXT("🔍 MoveRight called - InputValue: %.2f, CurrentlyDashing: %s, CurrentMovementInput.X: %.2f"), 
        InputValue, bCurrentlyDashing ? TEXT("TRUE") : TEXT("FALSE"), CurrentMovementInput.X);
    
    // Update movement input tracking (X = left/right, positive for right)
    CurrentMovementInput.X = InputValue;
    
    if (FollowCamera != nullptr && FMath::Abs(InputValue) > 0.0f)
    {
        // Get camera right direction (projected to ground)
        FVector CameraRight = FollowCamera->GetRightVector();
        CameraRight.Z = 0.0f; // Remove vertical component
        CameraRight.Normalize();
        
        UE_LOG(LogTemp, Log, TEXT("📍 MoveRight - Adding movement input: Direction=%s, Scale=%.2f"), 
            *CameraRight.ToString(), InputValue);
        
        // Move right (positive right - strafe right relative to camera)
        AddMovementInput(CameraRight, InputValue);
    }
}
```

### 2. Dash System Debug Logging

Add these debug logs to your dash functions in `MyCharacter.cpp`:

```cpp
void AMyCharacter::DashLeft(const FInputActionValue& Value)
{
    UE_LOG(LogTemp, Error, TEXT("🚀 DASH LEFT TRIGGERED - CurrentMovementInput: (%.2f, %.2f)"), 
        CurrentMovementInput.X, CurrentMovementInput.Y);
    
    if (!AbilitySystemComponent || !Controller)
    {
        UE_LOG(LogTemp, Error, TEXT("❌ DashLeft failed - Missing AbilitySystemComponent or Controller"));
        return;
    }

    // Try to activate dash ability with left direction
    FGameplayTagContainer AbilityTags;
    AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Dash")));
    
    UE_LOG(LogTemp, Error, TEXT("🔄 Attempting to activate dash ability with tag: Ability.Dash"));
    
    if (!AbilitySystemComponent->TryActivateAbilitiesByTag(AbilityTags))
    {
        UE_LOG(LogTemp, Error, TEXT("❌ GAS Dash activation failed - using fallback"));
        // Fallback implementation...
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("✅ GAS Dash activation successful"));
    }
}

void AMyCharacter::DashRight(const FInputActionValue& Value)
{
    UE_LOG(LogTemp, Error, TEXT("🚀 DASH RIGHT TRIGGERED - CurrentMovementInput: (%.2f, %.2f)"), 
        CurrentMovementInput.X, CurrentMovementInput.Y);
    
    if (!AbilitySystemComponent || !Controller)
    {
        UE_LOG(LogTemp, Error, TEXT("❌ DashRight failed - Missing AbilitySystemComponent or Controller"));
        return;
    }

    // Try to activate dash ability with right direction
    FGameplayTagContainer AbilityTags;
    AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Dash")));
    
    UE_LOG(LogTemp, Error, TEXT("🔄 Attempting to activate dash ability with tag: Ability.Dash"));
    
    if (!AbilitySystemComponent->TryActivateAbilitiesByTag(AbilityTags))
    {
        UE_LOG(LogTemp, Error, TEXT("❌ GAS Dash activation failed - using fallback"));
        // Fallback implementation...
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("✅ GAS Dash activation successful"));
    }
}
```

### 3. Gameplay Ability Dash Debug Logging

Add these debug logs to your `GameplayAbility_Dash.cpp`:

```cpp
void UGameplayAbility_Dash::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    UE_LOG(LogTemp, Error, TEXT("🎯 DASH ABILITY ACTIVATED"));
    
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        UE_LOG(LogTemp, Error, TEXT("❌ Dash ability commit failed"));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // Get the character
    AMyCharacter* Character = Cast<AMyCharacter>(ActorInfo->AvatarActor.Get());
    if (!Character)
    {
        UE_LOG(LogTemp, Error, TEXT("❌ Dash ability - invalid character"));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    UE_LOG(LogTemp, Error, TEXT("✅ Dash ability committed successfully"));
    
    // Execute the dash
    ExecuteDash(Character, DashDirection);

    // Set timer to end ability
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(DashTimer, this, &UGameplayAbility_Dash::OnDashComplete, DashDuration, false);
        UE_LOG(LogTemp, Error, TEXT("⏰ Dash timer set for %.2f seconds"), DashDuration);
    }
}

void UGameplayAbility_Dash::ExecuteDash(AMyCharacter* Character, EDashDirection Direction)
{
    UCameraComponent* FollowCamera = Character ? Character->GetFollowCamera() : nullptr;
    UCharacterMovementComponent* MovementComponent = Character ? Character->GetCharacterMovement() : nullptr;
    
    if (!Character || !FollowCamera || !MovementComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("❌ ExecuteDash - Invalid components"));
        return;
    }

    // Get the current movement input from the character
    FVector2D CurrentInput = Character->GetCurrentMovementInput();
    
    UE_LOG(LogTemp, Error, TEXT("🎮 ExecuteDash - Direction: %s, CurrentInput: (%.2f, %.2f)"), 
        Direction == EDashDirection::Left ? TEXT("LEFT") : TEXT("RIGHT"), CurrentInput.X, CurrentInput.Y);
    
    if (CurrentInput.IsNearlyZero())
    {
        UE_LOG(LogTemp, Warning, TEXT("⚠️ No current input - using fallback direction"));
        // Fallback to camera-relative direction if no input
        switch (Direction)
        {
        case EDashDirection::Left:
            CurrentInput = FVector2D(-1.0f, 0.0f);
            break;
        case EDashDirection::Right:
            CurrentInput = FVector2D(1.0f, 0.0f);
            break;
        default:
            UE_LOG(LogTemp, Error, TEXT("❌ Invalid dash direction"));
            return;
        }
    }

    // Store the input for dynamic updates
    InitialInputVector = CurrentInput.GetSafeNormal();
    InitialDashDirection = Direction;

    UE_LOG(LogTemp, Error, TEXT("📦 Stored dash data - InitialInputVector: (%.2f, %.2f), Direction: %s"), 
        InitialInputVector.X, InitialInputVector.Y, 
        InitialDashDirection == EDashDirection::Left ? TEXT("LEFT") : TEXT("RIGHT"));

    // Calculate dash direction using input and camera orientation
    FVector CameraForwardVector = FollowCamera->GetForwardVector();
    FVector CameraRightVector = FollowCamera->GetRightVector();

    // Keep movement horizontal
    CameraForwardVector.Z = 0.0f;
    CameraRightVector.Z = 0.0f;
    CameraForwardVector.Normalize();
    CameraRightVector.Normalize();

    // Transform the input by current camera orientation
    FVector DashDirectionVector = (CameraRightVector * InitialInputVector.X) + (CameraForwardVector * InitialInputVector.Y);
    DashDirectionVector.Normalize();

    // Calculate dash speed (convert distance/duration to velocity)
    DashSpeed = DashDistance / DashDuration;
    
    // Apply the dash velocity - works with movement system
    FVector DashVelocity = DashDirectionVector * DashSpeed;
    
    // Store current velocity before dash
    FVector PreviousVelocity = MovementComponent->Velocity;
    
    MovementComponent->Velocity = DashVelocity;
    
    // Optional: Add slight upward component for more dynamic feel
    MovementComponent->Velocity.Z += 100.0f; // Small upward boost
    
    UE_LOG(LogTemp, Error, TEXT("🚀 DASH VELOCITY SET - Previous: %s, New: %s"), 
        *PreviousVelocity.ToString(), *MovementComponent->Velocity.ToString());
    
    // Setup dash state
    DashingCharacter = Character;
    bIsDashing = true;
    DashStartTime = GetWorld()->GetTimeSeconds();

    UE_LOG(LogTemp, Error, TEXT("🏁 Dash state initialized - StartTime: %.2f, Speed: %.2f"), DashStartTime, DashSpeed);

    // Start the timeline update for dynamic direction changes
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(DashUpdateTimer, [this]()
        {
            if (bIsDashing && DashingCharacter)
            {
                float CurrentTime = GetWorld()->GetTimeSeconds();
                float ElapsedTime = CurrentTime - DashStartTime;
                float Alpha = FMath::Clamp(ElapsedTime / DashDuration, 0.0f, 1.0f);
                
                UpdateDashMovement(Alpha);
            }
        }, GetWorld()->GetDeltaSeconds(), true);
        
        UE_LOG(LogTemp, Error, TEXT("⚙️ Dash update timer started - UpdateRate: %.4f"), GetWorld()->GetDeltaSeconds());
    }

    FString DirectionString = FString::Printf(TEXT("Input-Based (%.2f, %.2f)"), InitialInputVector.X, InitialInputVector.Y);
    UE_LOG(LogTemp, Error, TEXT("✅ Velocity-based dash executed! Direction: %s, Velocity: %s"), *DirectionString, *DashVelocity.ToString());
}

void UGameplayAbility_Dash::UpdateDashMovement(float Alpha)
{
    if (!bIsDashing || !DashingCharacter)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("🔄 UpdateDashMovement - Alpha: %.3f"), Alpha);

    // Check if dash duration is complete
    if (Alpha >= 1.0f)
    {
        UE_LOG(LogTemp, Error, TEXT("🏁 Dash completed - Alpha >= 1.0"));
        
        // Stop the dash by reducing velocity
        UCharacterMovementComponent* MovementComponent = DashingCharacter->GetCharacterMovement();
        if (MovementComponent)
        {
            FVector PreviousVelocity = MovementComponent->Velocity;
            // Gradual stop to preserve some momentum
            MovementComponent->Velocity *= 0.1f;
            
            UE_LOG(LogTemp, Error, TEXT("🛑 Velocity reduced - Previous: %s, New: %s"), 
                *PreviousVelocity.ToString(), *MovementComponent->Velocity.ToString());
        }

        // Clear timeline update timer
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().ClearTimer(DashUpdateTimer);
        }

        bIsDashing = false;
        DashingCharacter = nullptr;
        
        UE_LOG(LogTemp, Error, TEXT("✅ Velocity-based dash completed!"));
        return;
    }

    // Update direction dynamically during dash
    UCameraComponent* FollowCamera = DashingCharacter->GetFollowCamera();
    UCharacterMovementComponent* MovementComponent = DashingCharacter->GetCharacterMovement();
    
    if (FollowCamera && MovementComponent)
    {
        // Get current camera vectors
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

        // Calculate desired velocity
        FVector DesiredDashVelocity = DesiredDashDirection * DashSpeed;
        
        // Preserve Z component (for jumps, gravity, etc.)
        DesiredDashVelocity.Z = MovementComponent->Velocity.Z;

        // Store current velocity before interpolation
        FVector PreviousVelocity = MovementComponent->Velocity;

        // Smoothly interpolate velocity direction
        MovementComponent->Velocity = FMath::VInterpTo(MovementComponent->Velocity, DesiredDashVelocity, GetWorld()->GetDeltaSeconds(), 5.0f);
        
        UE_LOG(LogTemp, Verbose, TEXT("🔄 Dynamic velocity update - Previous: %s, Desired: %s, Final: %s"), 
            *PreviousVelocity.ToString(), *DesiredDashVelocity.ToString(), *MovementComponent->Velocity.ToString());
    }
}

void UGameplayAbility_Dash::OnDashComplete()
{
    UE_LOG(LogTemp, Error, TEXT("🏁 OnDashComplete called"));
    
    // Clean up dash state
    if (bIsDashing && DashingCharacter)
    {
        UE_LOG(LogTemp, Error, TEXT("🧹 Cleaning up dash state"));
        
        // Clear timeline update timer
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().ClearTimer(DashUpdateTimer);
        }

        bIsDashing = false;
        DashingCharacter = nullptr;
    }

    // End the ability
    if (IsActive())
    {
        UE_LOG(LogTemp, Error, TEXT("🔚 Ending dash ability"));
        K2_EndAbility();
    }
}
```

### 4. Tick Function Debug Logging

Add this enhanced tick logging to `MyCharacter.cpp`:

```cpp
void AMyCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // DEBUG: Log movement state every few frames (to avoid spam)
    static int32 FrameCounter = 0;
    FrameCounter++;
    
    if (FrameCounter % 30 == 0) // Log every 30 frames (~0.5 seconds at 60fps)
    {
        bool bCurrentlyDashing = AbilitySystemComponent && 
            AbilitySystemComponent->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.Dashing")));
        
        FVector CurrentVelocity = GetCharacterMovement() ? GetCharacterMovement()->Velocity : FVector::ZeroVector;
        
        UE_LOG(LogTemp, Log, TEXT("📊 TICK - CurrentMovementInput: (%.2f, %.2f), IsDashing: %s, Velocity: %s"), 
            CurrentMovementInput.X, CurrentMovementInput.Y, 
            bCurrentlyDashing ? TEXT("TRUE") : TEXT("FALSE"), 
            *CurrentVelocity.ToString());
    }
}
```

## How to Test the Debug Logging

### 1. Compile and Run
1. Add the debug logging code to your files
2. Compile your project
3. Open the Output Log in Unreal Editor (Window > Developer Tools > Output Log)
4. Filter by "LogTemp" to see your debug messages

### 2. Test Scenario
1. Start playing in PIE (Play in Editor)
2. Press Shift+D to dash right
3. **IMMEDIATELY** press A (left) while dashing
4. Watch the Output Log for the sequence of messages

### 3. What to Look For
You should see a pattern like:
```
🚀 DASH RIGHT TRIGGERED - CurrentMovementInput: (0.00, 0.00)
🎯 DASH ABILITY ACTIVATED
🚀 DASH VELOCITY SET - Previous: (0,0,0), New: (3000,0,100)
🔍 MoveLeft called - InputValue: 1.00, CurrentlyDashing: TRUE, CurrentMovementInput.X: -1.00
📍 MoveLeft - Adding movement input: Direction=(-1,0,0), Scale=1.00
🔄 UpdateDashMovement - Alpha: 0.150
```

This will show you **exactly when** the conflict occurs: the MoveLeft function is being called DURING the dash (`CurrentlyDashing: TRUE`) and still adding movement input that conflicts with the dash velocity.

## Expected Output Analysis

The debug logs will confirm:
1. **Dash starts** with correct velocity (3000 units/sec)
2. **Input received during dash** - this is the problem!
3. **AddMovementInput called** while dashing - creates conflict
4. **UpdateDashMovement tries to correct** but fights with input system

This logging will provide definitive proof of the root cause and help you verify when the fix is working correctly.
