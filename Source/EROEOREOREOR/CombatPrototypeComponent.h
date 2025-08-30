#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "Curves/CurveFloat.h"
#include "GameplayTagContainer.h"
#include "CombatSystemTypes.h"
#include "CombatPrototypeComponent.generated.h"

class AMyCharacter;
class UCombatStateMachineComponent;

UENUM(BlueprintType)
enum class ECombatPhase : uint8
{
    None        UMETA(DisplayName = "None"),
    Startup     UMETA(DisplayName = "Startup"),
    Active      UMETA(DisplayName = "Active"), 
    Recovery    UMETA(DisplayName = "Recovery"),
    Canceled    UMETA(DisplayName = "Canceled")
};

UENUM(BlueprintType)
enum class ETrajectoryType : uint8
{
    Linear      UMETA(DisplayName = "Linear"),
    Arc         UMETA(DisplayName = "Arc"),
    Homing      UMETA(DisplayName = "Homing"),
    Curve       UMETA(DisplayName = "Custom Curve"),
    Teleport    UMETA(DisplayName = "Instant Teleport")
};

UENUM(BlueprintType)
enum class EMovementControlType : uint8
{
    None            UMETA(DisplayName = "No Movement Control"),
    LockPosition    UMETA(DisplayName = "Lock Position"),
    LockRotation    UMETA(DisplayName = "Lock Rotation"), 
    LockBoth        UMETA(DisplayName = "Lock Both"),
    CustomControl   UMETA(DisplayName = "Custom Control")
};

USTRUCT(BlueprintType)
struct FCombatTimingData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing", meta = (ClampMin = "0.0"))
    float StartupDuration = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing", meta = (ClampMin = "0.0"))
    float ActiveDuration = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing", meta = (ClampMin = "0.0"))
    float RecoveryDuration = 0.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    bool bCanCancelIntoOtherAttacks = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    bool bCanCancelIntoMovement = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float EarliestCancelWindow = 0.5f; // When during active phase canceling becomes possible
};

USTRUCT(BlueprintType)
struct FTrajectoryData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory")
    ETrajectoryType TrajectoryType = ETrajectoryType::Linear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory")
    FVector StartOffset = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory")
    FVector EndOffset = FVector(300.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory")
    float MaxDistance = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory")
    float ArcHeight = 100.0f; // For arc trajectory

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory")
    float HomingStrength = 1.0f; // For homing trajectory

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory")
    UCurveFloat* TrajectorySpeedCurve;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory")
    UCurveFloat* TrajectoryPathCurve; // Custom path modification
};

USTRUCT(BlueprintType)
struct FMovementControlData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Control")
    EMovementControlType StartupControl = EMovementControlType::LockBoth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Control")
    EMovementControlType ActiveControl = EMovementControlType::CustomControl;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Control")
    EMovementControlType RecoveryControl = EMovementControlType::LockRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Control")
    float MovementSpeedMultiplier = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Control")
    bool bCanRotateDuringAttack = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Control")
    float RotationRate = 180.0f;
};

USTRUCT(BlueprintType)
struct FCombatPrototypeData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Prototype")
    FString PrototypeName = "Attack";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Prototype")
    FGameplayTag AttackTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Prototype")
    FCombatTimingData TimingData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Prototype")
    FTrajectoryData TrajectoryData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Prototype")
    FMovementControlData MovementData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Prototype")
    float Damage = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Prototype")
    float Knockback = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Prototype")
    bool bDebugVisualization = true;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCombatPhaseChanged, ECombatPhase, OldPhase, ECombatPhase, NewPhase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAttackConnected, AActor*, HitActor);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class EROEOREOREOR_API UCombatPrototypeComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombatPrototypeComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core Combat Control
    UFUNCTION(BlueprintCallable, Category = "Combat Prototype")
    void StartAttack(const FString& PrototypeName);

    UFUNCTION(BlueprintCallable, Category = "Combat Prototype")
    void StartAttackWithData(const FCombatPrototypeData& AttackData);

    UFUNCTION(BlueprintCallable, Category = "Combat Prototype")
    void CancelAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat Prototype")
    bool CanCancelCurrentAttack() const;

    // Phase Management
    UFUNCTION(BlueprintPure, Category = "Combat Prototype")
    ECombatPhase GetCurrentPhase() const { return CurrentPhase; }

    UFUNCTION(BlueprintPure, Category = "Combat Prototype")
    float GetPhaseProgress() const;

    UFUNCTION(BlueprintPure, Category = "Combat Prototype")
    float GetTotalAttackDuration() const;

    // Trajectory Control
    UFUNCTION(BlueprintCallable, Category = "Combat Prototype")
    void SetTrajectoryTarget(const FVector& WorldPosition);

    UFUNCTION(BlueprintCallable, Category = "Combat Prototype")
    void SetTrajectoryTargetActor(AActor* TargetActor);

    UFUNCTION(BlueprintPure, Category = "Combat Prototype")
    FVector GetCurrentTrajectoryPosition() const;

    UFUNCTION(BlueprintPure, Category = "Combat Prototype")
    FVector GetPredictedEndPosition() const;

    // Data Management
    UFUNCTION(BlueprintCallable, Category = "Combat Prototype")
    void LoadPrototypeData(UDataTable* DataTable);

    UFUNCTION(BlueprintCallable, Category = "Combat Prototype")
    FCombatPrototypeData GetPrototypeData(const FString& PrototypeName) const;

    UFUNCTION(BlueprintCallable, Category = "Combat Prototype")
    TArray<FString> GetAvailablePrototypes() const;

    // Runtime Modification (for live tweaking)
    UFUNCTION(BlueprintCallable, Category = "Combat Prototype")
    void ModifyCurrentTimingData(const FCombatTimingData& NewTimingData);

    UFUNCTION(BlueprintCallable, Category = "Combat Prototype")
    void ModifyCurrentTrajectoryData(const FTrajectoryData& NewTrajectoryData);

    UFUNCTION(BlueprintCallable, Category = "Combat Prototype")
    void ModifyCurrentMovementData(const FMovementControlData& NewMovementData);

    // Debug & Visualization
    UFUNCTION(BlueprintCallable, Category = "Combat Prototype")
    void SetDebugVisualization(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Combat Prototype", CallInEditor)
    void TestCurrentPrototype();

    UFUNCTION(BlueprintCallable, Category = "Combat Prototype", CallInEditor)
    void PreviewTrajectory();

    // State Machine Integration
    UFUNCTION(BlueprintCallable, Category = "State Machine Integration")
    void StartActionFromStateMachine(const FCombatActionData& ActionData);
    
    UFUNCTION(BlueprintCallable, Category = "State Machine Integration")
    void EndActionFromStateMachine(bool bWasCanceled);
    
    UFUNCTION(BlueprintPure, Category = "State Machine Integration")
    bool IsExecutingAction() const { return CurrentPhase != ECombatPhase::None; }

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Combat Prototype")
    FOnCombatPhaseChanged OnPhaseChanged;

    UPROPERTY(BlueprintAssignable, Category = "Combat Prototype")
    FOnAttackConnected OnAttackConnected;

protected:
    // Internal State
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat Prototype")
    ECombatPhase CurrentPhase = ECombatPhase::None;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat Prototype")
    FCombatPrototypeData CurrentAttackData;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat Prototype")
    float CurrentPhaseTime = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat Prototype")
    float TotalAttackTime = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat Prototype")
    FVector AttackStartLocation = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat Prototype")
    FVector AttackTargetLocation = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat Prototype")
    TWeakObjectPtr<AActor> TargetActor;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    UDataTable* PrototypeDataTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bAutoLoadDataTable = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bDebugEnabled = true;

private:
    // Internal Methods
    void UpdateCombatPhase(float DeltaTime);
    void UpdateMovementControl();
    void UpdateTrajectoryMovement(float DeltaTime);
    void CheckForHits();
    void DrawDebugVisualization();
    
    void SetPhase(ECombatPhase NewPhase);
    FVector CalculateTrajectoryPosition(float Alpha) const;
    FVector CalculateLinearTrajectory(float Alpha) const;
    FVector CalculateArcTrajectory(float Alpha) const;
    FVector CalculateHomingTrajectory(float Alpha) const;
    FVector CalculateCurveTrajectory(float Alpha) const;

    // Cache references
    UPROPERTY()
    TWeakObjectPtr<AMyCharacter> OwnerCharacter;

    UPROPERTY()
    TMap<FString, FCombatPrototypeData> LoadedPrototypes;

    // Runtime state
    bool bHasConnectedThisAttack = false;
    FVector OriginalLocation = FVector::ZeroVector;
    FRotator OriginalRotation = FRotator::ZeroRotator;
    TArray<AActor*> AlreadyHitActors;
};
