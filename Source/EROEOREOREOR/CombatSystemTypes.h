#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "CombatSystemTypes.generated.h"

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

UENUM(BlueprintType)
enum class ECombatPriority : uint8
{
    Light       UMETA(DisplayName = "Light (0)"),
    Heavy       UMETA(DisplayName = "Heavy (1)"),
    Dash        UMETA(DisplayName = "Dash (2)"),
    Special     UMETA(DisplayName = "Special (3)"),
    Ultimate    UMETA(DisplayName = "Ultimate (4)")
};

UENUM(BlueprintType)
enum class EStyleRating : uint8
{
    C           UMETA(DisplayName = "C"),
    B           UMETA(DisplayName = "B"), 
    A           UMETA(DisplayName = "A"),
    S           UMETA(DisplayName = "S"),
    SSS         UMETA(DisplayName = "SSS")
};

USTRUCT(BlueprintType)
struct EROEOREOREOR_API FCombatActionData : public FTableRowBase
{
    GENERATED_BODY()

    // Identity
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    FGameplayTag ActionTag;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    FString DisplayName = "Basic Attack";

    // Frame Data (60 FPS baseline)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing", meta = (ClampMin = "1", ClampMax = "180", ToolTip = "Wind-up frames before attack becomes active"))
    int32 StartupFrames = 12;  // 0.2 seconds at 60fps
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing", meta = (ClampMin = "1", ClampMax = "60", ToolTip = "Active frames where attack can hit"))  
    int32 ActiveFrames = 6;    // 0.1 seconds at 60fps
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing", meta = (ClampMin = "1", ClampMax = "240", ToolTip = "Recovery frames after attack"))
    int32 RecoveryFrames = 18; // 0.3 seconds at 60fps

    // Cancel System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Canceling", meta = (ToolTip = "Actions this move can be canceled into"))
    TArray<FGameplayTag> CanCancelInto;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Canceling", meta = (ClampMin = "0", ToolTip = "First frame where canceling becomes available"))
    int32 CancelWindowStart = 8;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Canceling", meta = (ClampMin = "0", ToolTip = "Last frame where canceling is possible"))
    int32 CancelWindowEnd = 14;

    // Priority System  
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Priority", meta = (ToolTip = "Higher priority moves can interrupt lower priority ones"))
    ECombatPriority PriorityLevel = ECombatPriority::Light;
    
    // Weight & Impact
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weight", meta = (ClampMin = "0.1", ClampMax = "5.0", ToolTip = "Attack weight affects hit-stop and impact"))
    float AttackWeight = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weight", meta = (ToolTip = "Cannot be interrupted during active frames"))
    bool bHasHyperArmor = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weight", meta = (ClampMin = "0.0", ClampMax = "0.5", ToolTip = "Duration of hit-stop effect on successful hit"))
    float HitStopDuration = 0.05f;  // 3 frames at 60fps

    // Movement Integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "0.0", ClampMax = "1.0", ToolTip = "Movement speed multiplier during attack"))
    float MovementSpeedMultiplier = 0.2f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ToolTip = "Prevent character rotation during attack"))
    bool bLockRotation = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ToolTip = "Launch velocity for movement-based attacks"))
    FVector LaunchVelocity = FVector::ZeroVector;

    // Targeting & Range
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting", meta = (ClampMin = "0.0", ToolTip = "Maximum effective range of attack"))
    float Range = 300.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting", meta = (ToolTip = "Requires valid target to execute"))
    bool bRequiresTarget = false;
    
    // Integration with existing systems
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration", meta = (ToolTip = "Use CombatPrototypeComponent for execution"))
    bool bUseCombatPrototype = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration", meta = (ToolTip = "Which combat prototype to execute"))
    FString CombatPrototypeName = "BasicAttack";
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration", meta = (ToolTip = "Trigger AoE effect with this attack"))
    bool bTriggerAoE = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration", meta = (ToolTip = "Which AoE prototype to trigger"))
    FString AoEPrototypeName;

    // Combo System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo", meta = (ClampMin = "0.0", ToolTip = "Style points awarded for successful hit"))
    float StylePoints = 10.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo", meta = (ClampMin = "1.0", ToolTip = "Damage multiplier for combo hits"))
    float ComboDamageMultiplier = 1.0f;

    // Helper functions
    FORCEINLINE float GetTotalDurationSeconds() const
    {
        return (StartupFrames + ActiveFrames + RecoveryFrames) / 60.0f;
    }
    
    FORCEINLINE float GetStartupDurationSeconds() const
    {
        return StartupFrames / 60.0f;
    }
    
    FORCEINLINE float GetActiveDurationSeconds() const
    {
        return ActiveFrames / 60.0f;
    }
    
    FORCEINLINE float GetRecoveryDurationSeconds() const
    {
        return RecoveryFrames / 60.0f;
    }
    
    FORCEINLINE bool IsInCancelWindow(int32 CurrentFrame) const
    {
        return CurrentFrame >= CancelWindowStart && CurrentFrame <= CancelWindowEnd;
    }
    
    FORCEINLINE int32 GetPriorityValue() const
    {
        return static_cast<int32>(PriorityLevel);
    }
};

USTRUCT(BlueprintType)
struct EROEOREOREOR_API FHiddenComboData : public FTableRowBase
{
    GENERATED_BODY()

    // Combo sequence required
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hidden Combo")
    TArray<FGameplayTag> RequiredSequence;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hidden Combo")
    FString ComboName = "Secret Combo";
    
    // Timing requirements
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hidden Combo", meta = (ClampMin = "0.0", ToolTip = "Maximum time between inputs"))
    float MaxTimeBetweenInputs = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hidden Combo", meta = (ToolTip = "Must be executed with perfect timing"))
    bool bRequiresPerfectTiming = false;
    
    // Rewards
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hidden Combo", meta = (ClampMin = "0.0"))
    float BonusDamageMultiplier = 2.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hidden Combo", meta = (ClampMin = "0.0"))
    float BonusStylePoints = 100.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hidden Combo")
    FGameplayTag SpecialEffectTag;
};

// Attack shape types for prototyping
UENUM(BlueprintType)
enum class EAttackShape : uint8
{
    Sphere      UMETA(DisplayName = "Sphere - Point Attack"),
    Capsule     UMETA(DisplayName = "Capsule - Sword Swing"), 
    Box         UMETA(DisplayName = "Box - Wide Attack"),
    Cone        UMETA(DisplayName = "Cone - Directional Blast"),
    Line        UMETA(DisplayName = "Line - Thrust/Spear"),
    Ring        UMETA(DisplayName = "Ring - Around Character"),
    Arc         UMETA(DisplayName = "Arc - Partial Circle"),
    Custom      UMETA(DisplayName = "Custom Shape")
};

// Attack shape data for prototyping without art
USTRUCT(BlueprintType)
struct EROEOREOREOR_API FAttackShapeData
{
    GENERATED_BODY()

    // Shape type
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shape")
    EAttackShape ShapeType = EAttackShape::Sphere;
    
    // Basic dimensions
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shape", meta = (ClampMin = "1.0", ToolTip = "Primary size (radius for sphere, width for box)"))
    float PrimarySize = 100.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shape", meta = (ClampMin = "1.0", ToolTip = "Secondary size (height for capsule, depth for box)"))
    float SecondarySize = 200.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shape", meta = (ClampMin = "1.0", ToolTip = "Third dimension (length for box)"))
    float TertiarySize = 100.0f;
    
    // Position and rotation relative to character
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Position")
    FVector LocalOffset = FVector(150.0f, 0.0f, 0.0f);  // Forward by default
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Position")
    FRotator LocalRotation = FRotator::ZeroRotator;
    
    // Cone/Arc specific
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cone/Arc", meta = (ClampMin = "1.0", ClampMax = "360.0", ToolTip = "Angle in degrees"))
    float ConeAngle = 90.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cone/Arc", meta = (ClampMin = "1.0", ToolTip = "Range of cone/arc"))
    float ConeRange = 300.0f;
    
    // Ring specific
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ring", meta = (ClampMin = "1.0", ToolTip = "Inner radius of ring"))
    float InnerRadius = 50.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ring", meta = (ClampMin = "1.0", ToolTip = "Outer radius of ring"))
    float OuterRadius = 200.0f;
    
    // Visual prototyping
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Visualization")
    bool bShowDebugShape = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Visualization")
    FColor DebugColor = FColor::Red;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Visualization", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float DebugDrawTime = 2.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Visualization", meta = (ClampMin = "1.0"))
    float DebugLineThickness = 3.0f;
    
    // Timing - when during attack frames is this shape active
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing", meta = (ToolTip = "Frame when this hitbox becomes active"))
    int32 ActivationFrame = 1;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing", meta = (ToolTip = "Frame when this hitbox deactivates"))
    int32 DeactivationFrame = 6;
    
    // Multi-hit configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Multi-Hit")
    bool bAllowMultiHit = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Multi-Hit", meta = (ClampMin = "1"))
    int32 MaxHitsPerTarget = 1;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Multi-Hit", meta = (ClampMin = "0.0"))
    float MultihitInterval = 0.1f;  // 6 frames at 60fps

    FAttackShapeData()
    {
        ShapeType = EAttackShape::Sphere;
        PrimarySize = 100.0f;
        SecondarySize = 200.0f;
        TertiarySize = 100.0f;
        LocalOffset = FVector(150.0f, 0.0f, 0.0f);
        LocalRotation = FRotator::ZeroRotator;
        ConeAngle = 90.0f;
        ConeRange = 300.0f;
        InnerRadius = 50.0f;
        OuterRadius = 200.0f;
        bShowDebugShape = true;
        DebugColor = FColor::Red;
        DebugDrawTime = 2.0f;
        DebugLineThickness = 3.0f;
        ActivationFrame = 1;
        DeactivationFrame = 6;
        bAllowMultiHit = false;
        MaxHitsPerTarget = 1;
        MultihitInterval = 0.1f;
    }
};

// Enhanced attack data with shape information
USTRUCT(BlueprintType)
struct EROEOREOREOR_API FAttackPrototypeData : public FTableRowBase
{
    GENERATED_BODY()

    // Basic info
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    FString AttackName = "Basic Attack";
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    FGameplayTag AttackTag;
    
    // Multiple hitboxes for complex attacks
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Shapes", meta = (ToolTip = "Attack can have multiple hitboxes active at different times"))
    TArray<FAttackShapeData> AttackShapes;
    
    // Damage and effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects", meta = (ClampMin = "0.0"))
    float BaseDamage = 25.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    float Knockback = 300.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    FVector KnockbackDirection = FVector(1.0f, 0.0f, 0.2f);  // Forward and slightly up
    
    // Audio/Visual placeholder info (for when art comes online)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placeholder Art")
    FString PlaceholderAnimationName;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placeholder Art")
    FString PlaceholderSoundEffect;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placeholder Art")
    FString PlaceholderParticleEffect;

    FAttackPrototypeData()
    {
        AttackName = "Basic Attack";
        BaseDamage = 25.0f;
        Knockback = 300.0f;
        KnockbackDirection = FVector(1.0f, 0.0f, 0.2f);
        
        // Default single hitbox
        FAttackShapeData DefaultShape;
        AttackShapes.Add(DefaultShape);
    }
};

// Combat system constants
namespace CombatConstants
{
    constexpr float TARGET_FRAMERATE = 60.0f;
    constexpr float FRAME_DURATION = 1.0f / TARGET_FRAMERATE;
    constexpr float INPUT_BUFFER_DURATION = 0.2f;  // 12 frames
    constexpr int32 INPUT_BUFFER_FRAMES = 12;
    constexpr float PERFECT_TIMING_WINDOW = 3.0f / TARGET_FRAMERATE;  // 3 frames
    constexpr float COMBO_RESET_TIME = 2.0f;
    constexpr int32 MAX_COMBO_CHAIN_LENGTH = 20;
    
    // Debug visualization constants
    constexpr float DEBUG_SHAPE_ALPHA = 0.3f;
    constexpr int32 DEBUG_SPHERE_SEGMENTS = 16;
    constexpr int32 DEBUG_CONE_SEGMENTS = 12;
}
