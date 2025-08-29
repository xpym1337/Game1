# Advanced Combat System - Detailed Action Plan
## BDO-Style Flow + Dark Souls Weight + Fighting Game Precision

---

## 📊 Video Takeaways & Architecture Foundation

### Key Principles from the Video:
- **70% Reusable Modules**: Independent systems that work across projects
- **30% Glue Systems**: Game-specific coordinators 
- **Data-Driven Design**: All parameters in external data tables
- **Separation of Concerns**: Keep systems independent but use glue to connect them
- **If systems can't be separated, combine them thoughtfully**

---

## 🎯 Combat Design Evaluation (Industry Veteran Assessment)

### ✅ **Outstanding Design Choices:**

**1. Frame-Perfect Cancel System**
- This is **fighting game quality** design
- Creates skill ceiling while maintaining accessibility
- Priority-based interrupts prevent chaos

**2. Movement as Combat Philosophy** 
- **Brilliant** - most action games treat them separately
- Creates unique gameplay identity
- Enables complex combo expressions

**3. Three-Layer Targeting**
- Accommodates **all player preferences**
- Soft → Hard → Focus progression is intuitive
- Each layer serves different combat scenarios

**4. Weight & Impact System**
- **Professional-grade** game feel architecture
- Creates meaningful attack differentiation
- Impact pipeline is industry-standard approach

**5. Skill Expression Framework**
- Perfect timing rewards create **flow state**
- Style rating system encourages mastery
- Hidden combos reward experimentation

### 🏆 **Industry Rating: AAA Quality**
This design successfully combines:
- **Fighting Game Precision** (frame data, cancels)
- **Action RPG Flow** (movement integration) 
- **Souls-like Weight** (commitment, punishment)
- **Modern Juice** (escalation, feedback)

---

# 🚀 Implementation Action Plan

## Phase 1: Enhanced Combat State Machine (Week 1-2)

### **Code Tasks (My Responsibility):**

#### **1.1 Combat Action Data System**
```cpp
// Extends existing CombatPrototypeComponent with frame data
USTRUCT(BlueprintType)
struct EROEOREOREOR_API FCombatActionData : public FTableRowBase
{
    GENERATED_BODY()

    // Identity
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    FGameplayTag ActionTag;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity") 
    FString ActionName = "BasicAttack";

    // Frame Data (60 FPS baseline)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing", meta = (ClampMin = "1"))
    int32 StartupFrames = 12;  // Wind-up frames
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing", meta = (ClampMin = "1"))
    int32 ActiveFrames = 6;    // Hit detection frames
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing", meta = (ClampMin = "1"))
    int32 RecoveryFrames = 18; // Cool-down frames

    // Cancel System  
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Canceling")
    TArray<FGameplayTag> CanCancelInto;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Canceling")
    int32 CancelWindowStart = 8;  // First cancelable frame
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Canceling")
    int32 CancelWindowEnd = 14;   // Last cancelable frame

    // Priority System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Priority")
    int32 PriorityLevel = 0;  // 0=Light, 1=Heavy, 2=Dash, 3=Special, 4=Ultimate
    
    // Weight & Impact
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weight")
    float AttackWeight = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weight")
    bool bHasHyperArmor = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weight")
    float HitStopDuration = 0.1f;

    // Movement Integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float MovementSpeedMultiplier = 0.2f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bLockRotation = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    FVector LaunchVelocity = FVector::ZeroVector;

    // Targeting
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting")
    float Range = 300.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting")
    bool bRequiresTarget = false;
};
```

#### **1.2 Enhanced Combat State Machine**
```cpp
UENUM(BlueprintType)
enum class ECombatState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Startup     UMETA(DisplayName = "Startup"),
    Active      UMETA(DisplayName = "Active"), 
    Recovery    UMETA(DisplayName = "Recovery"),
    Canceling   UMETA(DisplayName = "Canceling"),
    Airborne    UMETA(DisplayName = "Airborne"),
    Dashing     UMETA(DisplayName = "Dashing"),
    Stunned     UMETA(DisplayName = "Stunned")
};

UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class EROEOREOREOR_API UCombatStateMachineComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombatStateMachineComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // State Management
    UFUNCTION(BlueprintCallable, Category = "Combat State")
    bool TryStartAction(const FGameplayTag& ActionTag);
    
    UFUNCTION(BlueprintCallable, Category = "Combat State")
    bool TryCancel(const FGameplayTag& NewActionTag);
    
    UFUNCTION(BlueprintPure, Category = "Combat State")
    ECombatState GetCurrentState() const { return CurrentState; }
    
    UFUNCTION(BlueprintPure, Category = "Combat State")
    bool IsInCancelWindow() const;
    
    UFUNCTION(BlueprintPure, Category = "Combat State")
    int32 GetCurrentFrame() const { return CurrentFrame; }

    // Input Buffer (12 frames @ 60fps = 0.2 seconds)
    UFUNCTION(BlueprintCallable, Category = "Input Buffer")
    void BufferInput(const FGameplayTag& ActionTag);
    
    UFUNCTION(BlueprintPure, Category = "Input Buffer")
    bool HasBufferedInput() const;

    // Combo System
    UFUNCTION(BlueprintPure, Category = "Combo")
    TArray<FGameplayTag> GetCurrentComboChain() const { return CurrentComboChain; }
    
    UFUNCTION(BlueprintPure, Category = "Combo")
    int32 GetComboCount() const { return CurrentComboChain.Num(); }

    // Data Management
    UFUNCTION(BlueprintCallable, Category = "Data")
    void LoadActionData(UDataTable* ActionDataTable);

    // Events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnStateChanged, ECombatState, OldState, ECombatState, NewState, const FGameplayTag&, ActionTag);
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnStateChanged OnStateChanged;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPerfectCancel, const FGameplayTag&, CanceledInto);
    UPROPERTY(BlueprintAssignable, Category = "Events") 
    FOnPerfectCancel OnPerfectCancel;

protected:
    // State tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    ECombatState CurrentState = ECombatState::Idle;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    FGameplayTag CurrentActionTag;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    int32 CurrentFrame = 0;

    // Frame timing
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float TargetFrameRate = 60.0f;
    
    float FrameTimer = 0.0f;
    float FrameDuration = 1.0f / 60.0f;

    // Input buffer
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Buffer")
    float BufferWindowSeconds = 0.2f;
    
    TArray<FGameplayTag> InputBuffer;
    TArray<float> InputTimestamps;

    // Combo tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combo")
    TArray<FGameplayTag> CurrentComboChain;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
    float ComboResetTime = 2.0f;
    
    float TimeSinceLastAction = 0.0f;

    // Data storage
    TMap<FGameplayTag, FCombatActionData> LoadedActions;

    // Component references
    UPROPERTY()
    TObjectPtr<UCombatPrototypeComponent> CombatPrototype;

private:
    void UpdateFrameTimer(float DeltaTime);
    void ProcessInputBuffer();
    void CheckForStateTransition();
    void UpdateComboSystem(float DeltaTime);
    bool CanCancelCurrentAction(const FGameplayTag& NewActionTag) const;
    void ExecuteAction(const FCombatActionData& ActionData);
    void HandlePerfectCancel(const FGameplayTag& CanceledInto);
};
```

### **Engine Tasks (Your Responsibility):**

#### **1.1 Create Combat Action Data Tables**
- Create `DT_LightAttacks` with entries:
  - `Combat.Attack.Light.Jab` (12f startup, 6f active, 18f recovery)
  - `Combat.Attack.Light.Cross` (10f startup, 4f active, 20f recovery)
  - `Combat.Attack.Light.Hook` (15f startup, 8f active, 22f recovery)
- Create `DT_HeavyAttacks` with entries:
  - `Combat.Attack.Heavy.Overhead` (25f startup, 12f active, 35f recovery)
  - `Combat.Attack.Heavy.Sweep` (20f startup, 15f active, 30f recovery)

#### **1.2 Combat Testing Environment**
- Create `BP_CombatTrainingDummy` actors with different weight classes
- Build real-time frame data display widget
- Set up combo chain visualization UI
- Create input buffer indicator widget

#### **1.3 Input Integration**
- Map mouse buttons to light/heavy attacks
- Integrate with existing Enhanced Input system
- Test input buffer with rapid button presses

---

## Phase 2: Movement-Combat Integration (Week 2-3)

### **Code Tasks (My Responsibility):**

#### **2.1 Enhanced Movement Combat Component**
```cpp
UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class EROEOREOREOR_API UMovementCombatComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    // Movement Attack Integration
    UFUNCTION(BlueprintCallable, Category = "Movement Combat")
    void ExecuteDashAttack(EDashDirection Direction, const FGameplayTag& AttackTag);
    
    UFUNCTION(BlueprintCallable, Category = "Movement Combat")
    void ExecuteLeapAttack(const FVector& TargetLocation, const FGameplayTag& AttackTag);
    
    UFUNCTION(BlueprintCallable, Category = "Movement Combat")
    void ExecuteTeleportStrike(ETeleportMode Mode, const FGameplayTag& AttackTag);

    // I-Frame System
    UFUNCTION(BlueprintCallable, Category = "Defense")
    void GrantInvulnerabilityFrames(float Duration);
    
    UFUNCTION(BlueprintPure, Category = "Defense")
    bool HasInvulnerabilityFrames() const { return bHasInvulnerabilityFrames; }

    // Cancel Integration
    UFUNCTION(BlueprintPure, Category = "Movement Combat")
    bool CanCancelMovementIntoAttack(const FGameplayTag& AttackTag) const;

protected:
    // I-frame state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Defense")
    bool bHasInvulnerabilityFrames = false;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Defense")
    float InvulnerabilityTimeRemaining = 0.0f;

    // Movement properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Combat")
    TMap<EDashDirection, FVector> DashDirections;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Combat")
    float AerialAttackGravityScale = 0.3f;

    // Component references
    UPROPERTY()
    TObjectPtr<UGameplayAbility_Dash> DashAbility;
    
    UPROPERTY()
    TObjectPtr<UGameplayAbility_Bounce> BounceAbility;
    
    UPROPERTY()
    TObjectPtr<UCombatStateMachineComponent> CombatStateMachine;

private:
    void UpdateInvulnerabilityFrames(float DeltaTime);
    void HandleMovementAttackLanding(const FHitResult& Hit);
};

UENUM(BlueprintType)
enum class ETeleportMode : uint8
{
    BehindTarget    UMETA(DisplayName = "Behind Target"),
    ToCursor        UMETA(DisplayName = "To Cursor"),
    ToFocusTarget   UMETA(DisplayName = "To Focus Target")
};
```

#### **2.2 Enhanced Dash Ability with Combat**
```cpp
UCLASS()
class EROEOREOREOR_API UGameplayAbility_DashCombat : public UGameplayAbility_Dash
{
    GENERATED_BODY()

public:
    // Combat integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Integration")
    TArray<FGameplayTag> CancelableIntoAttacks;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Integration")
    float AttackCancelWindowStart = 0.2f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Integration")
    float AttackCancelWindowEnd = 0.5f;

    // I-frame timing
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense")
    float InvulnerabilityStartTime = 0.1f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense")
    float InvulnerabilityDuration = 0.3f;

protected:
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
    
    void CheckForCombatCancels();
    void ActivateInvulnerabilityFrames();
    void OnDashThroughEnemy(AActor* Enemy);
};
```

### **Engine Tasks (Your Responsibility):**

#### **2.1 New Movement Abilities**
- Create `GA_Leap` with ballistic trajectory calculation
- Create `GA_TeleportStrike` with three targeting modes
- Set up input bindings for movement-attack combos

#### **2.2 Visual Feedback System**
- I-frame indicator (character outline shader)
- Cancel window visualization (color-coded character states)
- Movement trajectory preview lines
- Dash-through-enemy effect indicators

---

## Phase 3: Three-Layer Targeting System (Week 3-4)

### **Code Tasks (My Responsibility):**

#### **3.1 Targeting System Component**
```cpp
UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class EROEOREOREOR_API UTargetingSystemComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    // Three targeting layers
    UFUNCTION(BlueprintPure, Category = "Targeting")
    AActor* GetSoftTarget() const { return SoftTarget.Get(); }
    
    UFUNCTION(BlueprintPure, Category = "Targeting")
    AActor* GetHardLockTarget() const { return HardLockTarget.Get(); }
    
    UFUNCTION(BlueprintPure, Category = "Targeting")
    AActor* GetFocusTarget() const { return FocusTarget.Get(); }

    // Targeting actions
    UFUNCTION(BlueprintCallable, Category = "Targeting")
    void UpdateSoftTarget();
    
    UFUNCTION(BlueprintCallable, Category = "Targeting")
    void CycleLockTarget();
    
    UFUNCTION(BlueprintCallable, Category = "Targeting")
    void SetFocusTarget(AActor* NewFocusTarget);

    // Smart targeting for abilities
    UFUNCTION(BlueprintPure, Category = "Targeting")
    AActor* GetBestTargetForAbility(const FGameplayTag& AbilityTag) const;
    
    UFUNCTION(BlueprintPure, Category = "Targeting")
    FVector GetOptimalAbilityLocation(const FGameplayTag& AbilityTag) const;

    // AoE integration
    UFUNCTION(BlueprintCallable, Category = "AoE Integration")
    void UpdateAoEIndicators();

protected:
    // Targeting layers
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Targeting")
    TWeakObjectPtr<AActor> SoftTarget;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Targeting")
    TWeakObjectPtr<AActor> HardLockTarget;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Targeting")
    TWeakObjectPtr<AActor> FocusTarget;

    // Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float LockOnRadius = 1000.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float SoftTargetUpdateRate = 30.0f;

    // Component integration
    UPROPERTY()
    TObjectPtr<UAoEPrototypeComponent> AoEComponent;

private:
    TArray<AActor*> GetValidTargetsInRadius(float Radius) const;
    AActor* GetNextTargetInCycle(const TArray<AActor*>& ValidTargets) const;
    FVector GetCursorWorldPosition() const;
    void ValidateTargets();
    
    float LastSoftTargetUpdate = 0.0f;
};
```

### **Engine Tasks (Your Responsibility):**

#### **3.1 Targeting UI System** 
- Create soft target highlight (hover glow effect)
- Design hard lock indicator (Dark Souls style reticle)
- Build focus target persistent UI frame
- Update existing AoE indicators to work with targeting

#### **3.2 Input Configuration**
- Tab key for cycling hard lock targets
- F key for setting/clearing focus target
- Mouse-over updates for soft targeting

---

## Phase 4: Weight & Impact System (Week 4-5)

### **Code Tasks (My Responsibility):**

#### **4.1 Impact Subsystem**
```cpp
UCLASS()
class EROEOREOREOR_API UImpactSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Impact System")
    void ProcessCombatImpact(const FCombatActionData& AttackData, AActor* Attacker, AActor* Target, const FVector& ImpactLocation);
    
    UFUNCTION(BlueprintCallable, Category = "Game Feel")
    void ApplyHitStop(float Duration, float Intensity = 1.0f);
    
    UFUNCTION(BlueprintCallable, Category = "Game Feel")
    void ApplyTimeDilation(float TimeScale, float Duration);
    
    UFUNCTION(BlueprintCallable, Category = "Combo System")
    void UpdateComboFeedback(int32 ComboCount, bool bPerfectTiming = false);

protected:
    // Impact settings by weight class
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact Settings")
    TMap<int32, float> HitStopDurationByWeight;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact Settings")
    TMap<int32, TSubclassOf<UCameraShakeBase>> CameraShakeByWeight;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact Settings")
    TObjectPtr<UCurveFloat> ComboTimeDilationCurve;

    // VFX escalation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Escalation")
    float BaseVFXScale = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Escalation")
    float ComboScaleMultiplier = 1.2f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Escalation")
    float MaxVFXScale = 3.0f;

private:
    void ApplyCameraShake(const FVector& ImpactLocation, int32 WeightClass);
    void SpawnImpactVFX(const FVector& Location, int32 ComboLevel, int32 WeightClass);
    void PlayImpactSFX(const FVector& Location, int32 WeightClass, int32 ComboLevel);
    
    // Effect state
    bool bIsInHitStop = false;
    bool bIsInTimeDilation = false;
    FTimerHandle HitStopTimer;
    FTimerHandle TimeDilationTimer;
};
```

#### **4.2 Game Feel Data Assets**
```cpp
USTRUCT(BlueprintType)
struct EROEOREOREOR_API FGameFeelSettings : public FTableRowBase
{
    GENERATED_BODY()

    // Hit stop by weight class (in seconds)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Stop")
    float LightAttackHitStop = 0.05f;  // 3 frames
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Stop") 
    float HeavyAttackHitStop = 0.1f;   // 6 frames
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Stop")
    float UltimateHitStop = 0.2f;      // 12 frames

    // Time dilation settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Dilation")
    float PerfectCancelTimeScale = 0.3f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Dilation")
    float PerfectCancelDuration = 0.15f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Dilation")
    float ComboFinisherTimeScale = 0.1f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Dilation")
    float ComboFinisherDuration = 0.3f;

    // Camera shake classes
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Shake")
    TSubclassOf<UCameraShakeBase> LightAttackShake;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Shake")
    TSubclassOf<UCameraShakeBase> HeavyAttackShake;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Shake")
    TSubclassOf<UCameraShakeBase> UltimateShake;
};
```

### **Engine Tasks (Your Responsibility):**

#### **4.1 Game Feel Tuning Panel**
- Create developer widget with real-time sliders
- Hit stop, camera shake, time dilation controls  
- Save/load preset system for different feel profiles
- Runtime modification during gameplay

#### **4.2 VFX Escalation System**
- Base impact effects per weight class
- Combo scaling materials and particles
- Screen effects for perfect timing/criticals
- Audio layering system for combo escalation

---

## Phase 5: Skill Expression & Polish (Week 5-6)

### **Code Tasks (My Responsibility):**

#### **5.1 Skill Expression Component**
```cpp
UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class EROEOREOREOR_API USkillExpressionComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    // Perfect timing detection
    UFUNCTION(BlueprintCallable, Category = "Skill Expression")
    bool CheckPerfectCancelTiming(float CancelWindow, float InputTiming);
    
    UFUNCTION(BlueprintCallable, Category = "Skill Expression")
    void AddRhythmBonus(float BeatAccuracy);
    
    UFUNCTION(BlueprintCallable, Category = "Skill Expression")
    void AddPositionBonus(EPositionType Position);

    // Style rating system
    UFUNCTION(BlueprintPure, Category = "Style System")
    EStyleRating GetCurrentStyleRating() const { return CurrentStyleRating; }
    
    UFUNCTION(BlueprintCallable, Category = "Style System")
    void UpdateStyleRating(int32 ComboCount, bool bPerfectTiming, bool bPositionBonus);

    // Hidden combo detection
    UFUNCTION(BlueprintCallable, Category = "Hidden Combos")
    bool CheckForHiddenCombo(const TArray<FGameplayTag>& ComboSequence);

protected:
    // Style rating
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Style")
    EStyleRating CurrentStyleRating = EStyleRating::C;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Style") 
    float StylePoints = 0.0f;

    // Perfect timing settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perfect Timing")
    float PerfectWindowFrames = 3.0f;  // 3-frame window
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perfect Timing")
    float PerfectBonusMultiplier = 1.5f;

    // Hidden combos
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hidden Combos")
    TMap<FString, FHiddenComboData> HiddenCombos;

private:
    void CalculateStyleDecay(float DeltaTime);
    void TriggerPerfectTimingEffect();
    void ExecuteHiddenCombo(const FHiddenComboData& ComboData);
};

UENUM(BlueprintType)
enum class EStyleRating : uint8
{
    C       UMETA(DisplayName = "C"),
    B       UMETA(DisplayName = "B"), 
    A       UMETA(DisplayName = "A"),
    S       UMETA(DisplayName = "S"),
    SSS     UMETA(DisplayName = "SSS")
};

UENUM(BlueprintType)
enum class EPositionType : uint8
{
    Front       UMETA(DisplayName = "Front"),
    Back        UMETA(DisplayName = "Back"),
    Side        UMETA(DisplayName = "Side"),
    Aerial      UMETA(DisplayName = "Aerial")
};
```

### **Engine Tasks (Your Responsibility):**

#### **5.1 Style & Feedback System**
- Create style rating display widget (C → B → A → S → SSS)
- Perfect timing flash effect
- Combo counter with escalating VFX
- Audio layering that builds with style rating

#### **5.2 Final Polish Pass**
- Animation blending for smooth transitions
- Particle effect scaling with combo count
- Screen space effects for ultimate attacks
- Audio mixing for combat intensity

---

# 📋 Complete Development Checklist

## **Phase 1: Enhanced Combat State Machine**
- [ ] Implement FCombatActionData structure
- [ ] Create UCombatStateMachineComponent
- [ ] Build frame-accurate timing system
- [ ] Implement input buffer (12 frames)
- [ ] Create combo chain tracking
- [ ] Set up data table loading
- [ ] **Engine:** Create combat action data tables
- [ ] **Engine:** Build training dummy environment
- [ ] **Engine:** Create frame data display UI
- [ ] **Engine:** Test input buffering

## **Phase 2: Movement-Combat Integration**  
- [ ] Create UMovementCombatComponent
- [ ] Implement I-frame system
- [ ] Enhance dash ability with combat integration
- [ ] Create leap attack system
- [ ] Build teleport strike mechanics
- [ ] **Engine:** Create new movement abilities
- [ ] **Engine:** Set up movement-attack input bindings
- [ ] **Engine:** Implement visual feedback for I-frames
- [ ] **Engine:** Test movement cancels into attacks

## **Phase 3: Three-Layer Targeting**
- [ ] Implement UTargetingSystemComponent
- [ ] Create soft targeting (cursor hover)
- [ ] Build hard lock system (tab targeting)
- [ ] Implement focus target (persistent targeting)
- [ ] Integrate with existing AoE system
- [ ] **Engine:** Create targeting UI elements
- [ ] **Engine:** Set up targeting input bindings
- [ ] **Engine:** Test targeting in combat scenarios
