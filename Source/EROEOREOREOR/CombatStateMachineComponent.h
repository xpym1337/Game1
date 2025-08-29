#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "CombatSystemTypes.h"
#include "CombatStateMachineComponent.generated.h"

// Forward declarations
class UCombatPrototypeComponent;
class UAoEPrototypeComponent;
class AMyCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCombatStateChanged, ECombatState, OldState, ECombatState, NewState, const FGameplayTag&, ActionTag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCombatActionStarted, const FGameplayTag&, ActionTag, const FCombatActionData&, ActionData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCombatActionEnded, const FGameplayTag&, ActionTag, bool, bWasCanceled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPerfectCancel, const FGameplayTag&, CanceledIntoAction);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnComboUpdated, int32, ComboCount, const TArray<FGameplayTag>&, ComboChain);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHiddenComboExecuted, const FString&, ComboName);

UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable)
class EROEOREOREOR_API UCombatStateMachineComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombatStateMachineComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core State Management
    UFUNCTION(BlueprintCallable, Category = "Combat State Machine")
    bool TryStartAction(const FGameplayTag& ActionTag);
    
    UFUNCTION(BlueprintCallable, Category = "Combat State Machine")
    bool TryCancel(const FGameplayTag& NewActionTag);
    
    UFUNCTION(BlueprintCallable, Category = "Combat State Machine")
    void ForceEndAction(bool bWasCanceled = false);
    
    UFUNCTION(BlueprintCallable, Category = "Combat State Machine")
    void ForceSetState(ECombatState NewState);

    // State Queries
    UFUNCTION(BlueprintPure, Category = "Combat State Machine")
    ECombatState GetCurrentState() const { return CurrentState; }
    
    UFUNCTION(BlueprintPure, Category = "Combat State Machine")
    FGameplayTag GetCurrentActionTag() const { return CurrentActionTag; }
    
    UFUNCTION(BlueprintPure, Category = "Combat State Machine")
    bool IsInCancelWindow() const;
    
    UFUNCTION(BlueprintPure, Category = "Combat State Machine")
    bool CanStartAction(const FGameplayTag& ActionTag) const;
    
    UFUNCTION(BlueprintPure, Category = "Combat State Machine")
    int32 GetCurrentFrame() const { return CurrentFrame; }
    
    UFUNCTION(BlueprintPure, Category = "Combat State Machine")
    float GetCurrentFrameProgress() const;
    
    UFUNCTION(BlueprintPure, Category = "Combat State Machine")
    float GetCurrentPhaseProgress() const;

    // Input Buffer System
    UFUNCTION(BlueprintCallable, Category = "Input Buffer")
    void BufferInput(const FGameplayTag& ActionTag);
    
    UFUNCTION(BlueprintPure, Category = "Input Buffer")
    bool HasBufferedInput() const { return InputBuffer.Num() > 0; }
    
    UFUNCTION(BlueprintCallable, Category = "Input Buffer")
    void ClearInputBuffer();
    
    UFUNCTION(BlueprintPure, Category = "Input Buffer")
    TArray<FGameplayTag> GetBufferedInputs() const { return InputBuffer; }

    // Combo System
    UFUNCTION(BlueprintPure, Category = "Combo System")
    TArray<FGameplayTag> GetCurrentComboChain() const { return CurrentComboChain; }
    
    UFUNCTION(BlueprintPure, Category = "Combo System")
    int32 GetComboCount() const { return CurrentComboChain.Num(); }
    
    UFUNCTION(BlueprintCallable, Category = "Combo System")
    void ResetCombo();
    
    UFUNCTION(BlueprintPure, Category = "Combo System")
    float GetComboTimeRemaining() const;
    
    UFUNCTION(BlueprintCallable, Category = "Combo System")
    void ExtendComboTime(float AdditionalTime);

    // Data Management
    UFUNCTION(BlueprintCallable, Category = "Data Management")
    void LoadActionData(UDataTable* ActionDataTable);
    
    UFUNCTION(BlueprintCallable, Category = "Data Management")
    void LoadHiddenComboData(UDataTable* HiddenComboDataTable);
    
    UFUNCTION(BlueprintPure, Category = "Data Management")
    FCombatActionData GetActionData(const FGameplayTag& ActionTag) const;
    
    UFUNCTION(BlueprintPure, Category = "Data Management")
    TArray<FGameplayTag> GetAvailableActions() const;
    
    UFUNCTION(BlueprintPure, Category = "Data Management")
    bool HasActionData(const FGameplayTag& ActionTag) const;

    // Cancel System
    UFUNCTION(BlueprintPure, Category = "Cancel System")
    TArray<FGameplayTag> GetCurrentCancelOptions() const;
    
    UFUNCTION(BlueprintPure, Category = "Cancel System")
    bool CanCancelCurrentAction(const FGameplayTag& NewActionTag) const;
    
    UFUNCTION(BlueprintPure, Category = "Cancel System")
    bool IsValidCancel(const FGameplayTag& FromAction, const FGameplayTag& ToAction) const;

    // Priority System
    UFUNCTION(BlueprintPure, Category = "Priority System")
    int32 GetActionPriority(const FGameplayTag& ActionTag) const;
    
    UFUNCTION(BlueprintPure, Category = "Priority System")
    bool CanInterrupt(const FGameplayTag& InterruptingAction, const FGameplayTag& CurrentAction) const;

    // Hidden Combos
    UFUNCTION(BlueprintCallable, Category = "Hidden Combos")
    bool CheckForHiddenCombo();
    
    UFUNCTION(BlueprintPure, Category = "Hidden Combos")
    TArray<FString> GetAvailableHiddenCombos() const;

    // Debug & Testing
    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor = true)
    void TestAction(const FGameplayTag& ActionTag);
    
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void SetDebugVisualization(bool bEnabled);
    
    UFUNCTION(BlueprintPure, Category = "Debug")
    FString GetDebugStateInfo() const;
    
    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor = true)
    void PrintCurrentState();

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Combat Events")
    FOnCombatStateChanged OnStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Combat Events")
    FOnCombatActionStarted OnActionStarted;

    UPROPERTY(BlueprintAssignable, Category = "Combat Events")
    FOnCombatActionEnded OnActionEnded;

    UPROPERTY(BlueprintAssignable, Category = "Combat Events")
    FOnPerfectCancel OnPerfectCancel;

    UPROPERTY(BlueprintAssignable, Category = "Combat Events")
    FOnComboUpdated OnComboUpdated;

    UPROPERTY(BlueprintAssignable, Category = "Combat Events")
    FOnHiddenComboExecuted OnHiddenComboExecuted;

protected:
    // Current state tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", meta = (AllowPrivateAccess = "true"))
    ECombatState CurrentState = ECombatState::Idle;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", meta = (AllowPrivateAccess = "true"))
    FGameplayTag CurrentActionTag;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", meta = (AllowPrivateAccess = "true"))
    int32 CurrentFrame = 0;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", meta = (AllowPrivateAccess = "true"))
    float StateElapsedTime = 0.0f;

    // Frame timing
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frame Timing", meta = (AllowPrivateAccess = "true"))
    float TargetFrameRate = 60.0f;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Frame Timing", meta = (AllowPrivateAccess = "true"))
    float FrameTimer = 0.0f;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Frame Timing", meta = (AllowPrivateAccess = "true"))
    float FrameDuration = 1.0f / 60.0f;

    // Input buffer
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Buffer", meta = (ClampMin = "0.05", ClampMax = "0.5", AllowPrivateAccess = "true"))
    float BufferWindowSeconds = 0.2f;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Input Buffer", meta = (AllowPrivateAccess = "true"))
    TArray<FGameplayTag> InputBuffer;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Input Buffer", meta = (AllowPrivateAccess = "true"))
    TArray<float> InputTimestamps;

    // Combo tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combo System", meta = (AllowPrivateAccess = "true"))
    TArray<FGameplayTag> CurrentComboChain;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo System", meta = (ClampMin = "0.5", ClampMax = "5.0", AllowPrivateAccess = "true"))
    float ComboResetTime = 2.0f;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combo System", meta = (AllowPrivateAccess = "true"))
    float TimeSinceLastAction = 0.0f;

    // Data storage
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Data", meta = (AllowPrivateAccess = "true"))
    TMap<FGameplayTag, FCombatActionData> LoadedActions;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Data", meta = (AllowPrivateAccess = "true"))
    TMap<FString, FHiddenComboData> LoadedHiddenCombos;

    // Component references
    UPROPERTY(BlueprintReadOnly, Category = "Component References", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UCombatPrototypeComponent> CombatPrototype;
    
    UPROPERTY(BlueprintReadOnly, Category = "Component References", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UAoEPrototypeComponent> AoEComponent;
    
    UPROPERTY(BlueprintReadOnly, Category = "Component References", meta = (AllowPrivateAccess = "true"))
    TWeakObjectPtr<AMyCharacter> OwnerCharacter;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UDataTable> DefaultActionDataTable;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UDataTable> DefaultHiddenComboDataTable;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration", meta = (AllowPrivateAccess = "true"))
    bool bAutoLoadDefaultTables = true;

    // Debug
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug", meta = (AllowPrivateAccess = "true"))
    bool bDebugVisualization = false;

private:
    // Internal state management
    void UpdateFrameTimer(float DeltaTime);
    void ProcessFrame();
    void CheckForStateTransition();
    void SetState(ECombatState NewState);
    
    // Input processing
    void ProcessInputBuffer();
    void CleanupOldInputs();
    bool CheckInputTiming(float InputTime) const;
    
    // Action execution
    bool ExecuteAction(const FGameplayTag& ActionTag);
    void StartAction(const FCombatActionData& ActionData);
    void EndCurrentAction(bool bWasCanceled = false);
    
    // Cancel system implementation
    bool ProcessCancel(const FGameplayTag& NewActionTag);
    bool IsPerfectCancel(const FCombatActionData& FromAction, const FCombatActionData& ToAction) const;
    void HandlePerfectCancel(const FGameplayTag& CanceledInto);
    
    // Combo system implementation  
    void UpdateComboSystem(float DeltaTime);
    void AddToCombo(const FGameplayTag& ActionTag);
    void CheckComboReset();
    
    // Hidden combo system
    bool MatchesHiddenComboSequence(const TArray<FGameplayTag>& Sequence) const;
    void ExecuteHiddenCombo(const FHiddenComboData& ComboData);
    
    // Component integration
    void FindComponentReferences();
    void NotifyComponentsActionStarted(const FCombatActionData& ActionData);
    void NotifyComponentsActionEnded(const FGameplayTag& ActionTag, bool bWasCanceled);
    
    // Frame calculations
    int32 GetExpectedFrameForState() const;
    bool IsFrameInRange(int32 StartFrame, int32 EndFrame) const;
    float GetFrameProgress() const;
    
    // Utility functions
    const FCombatActionData* GetCurrentActionData() const;
    bool IsActionValid(const FGameplayTag& ActionTag) const;
    bool ArePrerequisitesMet(const FCombatActionData& ActionData) const;
    
    // Debug helpers
    void DrawDebugInfo();
    void LogStateTransition(ECombatState FromState, ECombatState ToState, const FGameplayTag& ActionTag);
    void LogFrameInfo() const;
};
