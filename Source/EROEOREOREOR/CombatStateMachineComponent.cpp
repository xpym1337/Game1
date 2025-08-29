#include "CombatStateMachineComponent.h"
#include "CombatPrototypeComponent.h"
#include "AoEPrototypeComponent.h"
#include "MyCharacter.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UCombatStateMachineComponent::UCombatStateMachineComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Initialize frame timing
    FrameDuration = 1.0f / TargetFrameRate;
    FrameTimer = 0.0f;
    CurrentFrame = 0;
    StateElapsedTime = 0.0f;
    
    // Initialize state
    CurrentState = ECombatState::Idle;
    CurrentActionTag = FGameplayTag::EmptyTag;
    
    // Initialize combo system
    TimeSinceLastAction = 0.0f;
}

void UCombatStateMachineComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Cache component references
    FindComponentReferences();
    
    // Auto-load data tables
    if (bAutoLoadDefaultTables)
    {
        if (DefaultActionDataTable)
        {
            LoadActionData(DefaultActionDataTable);
        }
        
        if (DefaultHiddenComboDataTable)
        {
            LoadHiddenComboData(DefaultHiddenComboDataTable);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("CombatStateMachineComponent initialized for %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("NULL"));
}

void UCombatStateMachineComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update frame-accurate timing
    UpdateFrameTimer(DeltaTime);
    
    // Update combo system
    UpdateComboSystem(DeltaTime);
    
    // Process input buffer
    ProcessInputBuffer();
    
    // Debug visualization
    if (bDebugVisualization)
    {
        DrawDebugInfo();
    }
}

bool UCombatStateMachineComponent::TryStartAction(const FGameplayTag& ActionTag)
{
    // Check if action is available
    if (!CanStartAction(ActionTag))
    {
        // Buffer the input if we can't start immediately
        BufferInput(ActionTag);
        return false;
    }
    
    // Execute the action
    return ExecuteAction(ActionTag);
}

bool UCombatStateMachineComponent::TryCancel(const FGameplayTag& NewActionTag)
{
    if (!IsInCancelWindow())
    {
        return false;
    }
    
    if (!CanCancelCurrentAction(NewActionTag))
    {
        return false;
    }
    
    // Process the cancel
    return ProcessCancel(NewActionTag);
}

void UCombatStateMachineComponent::ForceEndAction(bool bWasCanceled)
{
    if (CurrentState != ECombatState::Idle)
    {
        EndCurrentAction(bWasCanceled);
    }
}

void UCombatStateMachineComponent::ForceSetState(ECombatState NewState)
{
    SetState(NewState);
}

bool UCombatStateMachineComponent::IsInCancelWindow() const
{
    if (CurrentState != ECombatState::Active && CurrentState != ECombatState::Recovery)
    {
        return false;
    }
    
    const FCombatActionData* ActionData = GetCurrentActionData();
    if (!ActionData)
    {
        return false;
    }
    
    return ActionData->IsInCancelWindow(CurrentFrame);
}

bool UCombatStateMachineComponent::CanStartAction(const FGameplayTag& ActionTag) const
{
    // Must be in idle state or valid cancel window
    if (CurrentState == ECombatState::Idle)
    {
        return IsActionValid(ActionTag);
    }
    
    if (IsInCancelWindow())
    {
        return CanCancelCurrentAction(ActionTag);
    }
    
    return false;
}

float UCombatStateMachineComponent::GetCurrentFrameProgress() const
{
    const FCombatActionData* ActionData = GetCurrentActionData();
    if (!ActionData)
    {
        return 0.0f;
    }
    
    int32 ExpectedFrame = GetExpectedFrameForState();
    return ExpectedFrame > 0 ? FMath::Clamp(static_cast<float>(CurrentFrame) / static_cast<float>(ExpectedFrame), 0.0f, 1.0f) : 0.0f;
}

float UCombatStateMachineComponent::GetCurrentPhaseProgress() const
{
    const FCombatActionData* ActionData = GetCurrentActionData();
    if (!ActionData)
    {
        return 0.0f;
    }
    
    switch (CurrentState)
    {
        case ECombatState::Startup:
            return ActionData->StartupFrames > 0 ? FMath::Clamp(static_cast<float>(CurrentFrame) / static_cast<float>(ActionData->StartupFrames), 0.0f, 1.0f) : 1.0f;
        case ECombatState::Active:
            return ActionData->ActiveFrames > 0 ? FMath::Clamp(static_cast<float>(CurrentFrame - ActionData->StartupFrames) / static_cast<float>(ActionData->ActiveFrames), 0.0f, 1.0f) : 1.0f;
        case ECombatState::Recovery:
            return ActionData->RecoveryFrames > 0 ? FMath::Clamp(static_cast<float>(CurrentFrame - ActionData->StartupFrames - ActionData->ActiveFrames) / static_cast<float>(ActionData->RecoveryFrames), 0.0f, 1.0f) : 1.0f;
        default:
            return 0.0f;
    }
}

void UCombatStateMachineComponent::BufferInput(const FGameplayTag& ActionTag)
{
    if (!ActionTag.IsValid())
    {
        return;
    }
    
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    // Add to buffer
    InputBuffer.Add(ActionTag);
    InputTimestamps.Add(CurrentTime);
    
    // Clean up old inputs
    CleanupOldInputs();
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("Buffered input: %s"), *ActionTag.ToString());
}

void UCombatStateMachineComponent::ClearInputBuffer()
{
    InputBuffer.Empty();
    InputTimestamps.Empty();
}

float UCombatStateMachineComponent::GetComboTimeRemaining() const
{
    return FMath::Max(0.0f, ComboResetTime - TimeSinceLastAction);
}

void UCombatStateMachineComponent::ExtendComboTime(float AdditionalTime)
{
    TimeSinceLastAction = FMath::Max(0.0f, TimeSinceLastAction - AdditionalTime);
}

void UCombatStateMachineComponent::ResetCombo()
{
    CurrentComboChain.Empty();
    TimeSinceLastAction = ComboResetTime; // Force reset
    OnComboUpdated.Broadcast(0, CurrentComboChain);
}

void UCombatStateMachineComponent::LoadActionData(UDataTable* ActionDataTable)
{
    if (!ActionDataTable)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot load null action data table"));
        return;
    }
    
    LoadedActions.Empty();
    
    TArray<FCombatActionData*> AllRows;
    ActionDataTable->GetAllRows<FCombatActionData>(TEXT("LoadActionData"), AllRows);
    
    for (const FCombatActionData* Row : AllRows)
    {
        if (Row && Row->ActionTag.IsValid())
        {
            LoadedActions.Add(Row->ActionTag, *Row);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Loaded %d combat actions"), LoadedActions.Num());
}

void UCombatStateMachineComponent::LoadHiddenComboData(UDataTable* HiddenComboDataTable)
{
    if (!HiddenComboDataTable)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot load null hidden combo data table"));
        return;
    }
    
    LoadedHiddenCombos.Empty();
    
    TArray<FHiddenComboData*> AllRows;
    HiddenComboDataTable->GetAllRows<FHiddenComboData>(TEXT("LoadHiddenComboData"), AllRows);
    
    for (const FHiddenComboData* Row : AllRows)
    {
        if (Row)
        {
            LoadedHiddenCombos.Add(Row->ComboName, *Row);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Loaded %d hidden combos"), LoadedHiddenCombos.Num());
}

FCombatActionData UCombatStateMachineComponent::GetActionData(const FGameplayTag& ActionTag) const
{
    if (const FCombatActionData* FoundData = LoadedActions.Find(ActionTag))
    {
        return *FoundData;
    }
    return FCombatActionData();
}

TArray<FGameplayTag> UCombatStateMachineComponent::GetAvailableActions() const
{
    TArray<FGameplayTag> AvailableActions;
    LoadedActions.GetKeys(AvailableActions);
    return AvailableActions;
}

bool UCombatStateMachineComponent::HasActionData(const FGameplayTag& ActionTag) const
{
    return LoadedActions.Contains(ActionTag);
}

TArray<FGameplayTag> UCombatStateMachineComponent::GetCurrentCancelOptions() const
{
    if (!IsInCancelWindow())
    {
        return TArray<FGameplayTag>();
    }
    
    const FCombatActionData* ActionData = GetCurrentActionData();
    return ActionData ? ActionData->CanCancelInto : TArray<FGameplayTag>();
}

bool UCombatStateMachineComponent::CanCancelCurrentAction(const FGameplayTag& NewActionTag) const
{
    if (!IsInCancelWindow())
    {
        return false;
    }
    
    const FCombatActionData* CurrentAction = GetCurrentActionData();
    if (!CurrentAction)
    {
        return false;
    }
    
    // Check if new action is in cancel list
    if (!CurrentAction->CanCancelInto.Contains(NewActionTag))
    {
        return false;
    }
    
    // Check if new action exists
    if (!HasActionData(NewActionTag))
    {
        return false;
    }
    
    // Check priority (higher can interrupt lower)
    const FCombatActionData NewAction = GetActionData(NewActionTag);
    return NewAction.GetPriorityValue() > CurrentAction->GetPriorityValue();
}

bool UCombatStateMachineComponent::IsValidCancel(const FGameplayTag& FromAction, const FGameplayTag& ToAction) const
{
    const FCombatActionData FromActionData = GetActionData(FromAction);
    return FromActionData.CanCancelInto.Contains(ToAction);
}

int32 UCombatStateMachineComponent::GetActionPriority(const FGameplayTag& ActionTag) const
{
    const FCombatActionData ActionData = GetActionData(ActionTag);
    return ActionData.GetPriorityValue();
}

bool UCombatStateMachineComponent::CanInterrupt(const FGameplayTag& InterruptingAction, const FGameplayTag& CurrentAction) const
{
    return GetActionPriority(InterruptingAction) > GetActionPriority(CurrentAction);
}

bool UCombatStateMachineComponent::CheckForHiddenCombo()
{
    for (const auto& ComboEntry : LoadedHiddenCombos)
    {
        const FHiddenComboData& ComboData = ComboEntry.Value;
        
        if (MatchesHiddenComboSequence(ComboData.RequiredSequence))
        {
            ExecuteHiddenCombo(ComboData);
            return true;
        }
    }
    
    return false;
}

TArray<FString> UCombatStateMachineComponent::GetAvailableHiddenCombos() const
{
    TArray<FString> ComboNames;
    LoadedHiddenCombos.GetKeys(ComboNames);
    return ComboNames;
}

void UCombatStateMachineComponent::TestAction(const FGameplayTag& ActionTag)
{
    UE_LOG(LogTemp, Warning, TEXT("Testing action: %s"), *ActionTag.ToString());
    TryStartAction(ActionTag);
}

void UCombatStateMachineComponent::SetDebugVisualization(bool bEnabled)
{
    bDebugVisualization = bEnabled;
}

FString UCombatStateMachineComponent::GetDebugStateInfo() const
{
    const FCombatActionData* ActionData = GetCurrentActionData();
    return FString::Printf(TEXT("State: %d, Frame: %d, Action: %s, ComboCount: %d"), 
                          static_cast<int32>(CurrentState), 
                          CurrentFrame, 
                          CurrentActionTag.IsValid() ? *CurrentActionTag.ToString() : TEXT("None"),
                          GetComboCount());
}

void UCombatStateMachineComponent::PrintCurrentState()
{
    UE_LOG(LogTemp, Warning, TEXT("Combat State: %s"), *GetDebugStateInfo());
}

// Private Methods

void UCombatStateMachineComponent::UpdateFrameTimer(float DeltaTime)
{
    FrameTimer += DeltaTime;
    StateElapsedTime += DeltaTime;
    
    // Process frame-by-frame for consistency
    while (FrameTimer >= FrameDuration)
    {
        FrameTimer -= FrameDuration;
        CurrentFrame++;
        ProcessFrame();
    }
}

void UCombatStateMachineComponent::ProcessFrame()
{
    // Check for state transitions
    CheckForStateTransition();
    
    // Process input buffer during valid windows
    if (CurrentState == ECombatState::Idle || IsInCancelWindow())
    {
        ProcessInputBuffer();
    }
}

void UCombatStateMachineComponent::CheckForStateTransition()
{
    const FCombatActionData* ActionData = GetCurrentActionData();
    if (!ActionData)
    {
        return;
    }
    
    switch (CurrentState)
    {
        case ECombatState::Startup:
            if (CurrentFrame >= ActionData->StartupFrames)
            {
                SetState(ECombatState::Active);
            }
            break;
            
        case ECombatState::Active:
            if (CurrentFrame >= ActionData->StartupFrames + ActionData->ActiveFrames)
            {
                SetState(ECombatState::Recovery);
            }
            break;
            
        case ECombatState::Recovery:
            if (CurrentFrame >= ActionData->StartupFrames + ActionData->ActiveFrames + ActionData->RecoveryFrames)
            {
                EndCurrentAction(false);
            }
            break;
            
        case ECombatState::Canceling:
            // Brief transition state, quickly move to next action
            if (StateElapsedTime >= 0.05f) // 3 frames
            {
                SetState(ECombatState::Idle);
            }
            break;
            
        default:
            break;
    }
}

void UCombatStateMachineComponent::SetState(ECombatState NewState)
{
    const ECombatState OldState = CurrentState;
    CurrentState = NewState;
    StateElapsedTime = 0.0f;
    
    // Reset frame counter for new state
    if (NewState == ECombatState::Startup)
    {
        CurrentFrame = 0;
    }
    
    // Broadcast state change
    OnStateChanged.Broadcast(OldState, NewState, CurrentActionTag);
    
    if (bDebugVisualization)
    {
        LogStateTransition(OldState, NewState, CurrentActionTag);
    }
}

void UCombatStateMachineComponent::ProcessInputBuffer()
{
    if (InputBuffer.Num() == 0)
    {
        return;
    }
    
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    // Try to execute the most recent valid input
    for (int32 i = InputBuffer.Num() - 1; i >= 0; --i)
    {
        const FGameplayTag& BufferedAction = InputBuffer[i];
        
        if (CanStartAction(BufferedAction))
        {
            // Execute this action
            if (ExecuteAction(BufferedAction))
            {
                // Success - clear buffer
                ClearInputBuffer();
                return;
            }
        }
    }
}

void UCombatStateMachineComponent::CleanupOldInputs()
{
    if (!GetWorld())
    {
        return;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Remove inputs older than buffer window
    for (int32 i = InputBuffer.Num() - 1; i >= 0; --i)
    {
        if (CurrentTime - InputTimestamps[i] > BufferWindowSeconds)
        {
            InputBuffer.RemoveAt(i);
            InputTimestamps.RemoveAt(i);
        }
    }
}

bool UCombatStateMachineComponent::ExecuteAction(const FGameplayTag& ActionTag)
{
    const FCombatActionData* ActionData = LoadedActions.Find(ActionTag);
    if (!ActionData)
    {
        UE_LOG(LogTemp, Warning, TEXT("Action data not found for tag: %s"), *ActionTag.ToString());
        return false;
    }
    
    if (!ArePrerequisitesMet(*ActionData))
    {
        return false;
    }
    
    // Start the action
    StartAction(*ActionData);
    return true;
}

void UCombatStateMachineComponent::StartAction(const FCombatActionData& ActionData)
{
    // Set current action
    CurrentActionTag = ActionData.ActionTag;
    CurrentFrame = 0;
    StateElapsedTime = 0.0f;
    
    // Add to combo chain
    AddToCombo(ActionData.ActionTag);
    
    // Check for hidden combos
    CheckForHiddenCombo();
    
    // Set state to startup
    SetState(ECombatState::Startup);
    
    // Notify components to start execution
    NotifyComponentsActionStarted(ActionData);
    
    // Broadcast action started
    OnActionStarted.Broadcast(ActionData.ActionTag, ActionData);
    
    UE_LOG(LogTemp, Log, TEXT("Started combat action: %s (Startup: %df, Active: %df, Recovery: %df)"), 
           *ActionData.DisplayName, ActionData.StartupFrames, ActionData.ActiveFrames, ActionData.RecoveryFrames);
}

void UCombatStateMachineComponent::EndCurrentAction(bool bWasCanceled)
{
    if (CurrentState == ECombatState::Idle)
    {
        return;
    }
    
    FGameplayTag EndingActionTag = CurrentActionTag;
    
    // Notify components
    NotifyComponentsActionEnded(CurrentActionTag, bWasCanceled);
    
    // Reset state
    CurrentActionTag = FGameplayTag::EmptyTag;
    CurrentFrame = 0;
    StateElapsedTime = 0.0f;
    SetState(ECombatState::Idle);
    
    // Broadcast action ended
    OnActionEnded.Broadcast(EndingActionTag, bWasCanceled);
    
    UE_LOG(LogTemp, Log, TEXT("Ended combat action: %s (Canceled: %s)"), 
           *EndingActionTag.ToString(), bWasCanceled ? TEXT("Yes") : TEXT("No"));
}

bool UCombatStateMachineComponent::ProcessCancel(const FGameplayTag& NewActionTag)
{
    const FCombatActionData* CurrentAction = GetCurrentActionData();
    const FCombatActionData NewActionData = GetActionData(NewActionTag);
    
    if (!CurrentAction)
    {
        return false;
    }
    
    // Check for perfect cancel timing
    bool bIsPerfectCancel = IsPerfectCancel(*CurrentAction, NewActionData);
    
    if (bIsPerfectCancel)
    {
        HandlePerfectCancel(NewActionTag);
    }
    
    // End current action as canceled
    EndCurrentAction(true);
    
    // Start new action
    return ExecuteAction(NewActionTag);
}

bool UCombatStateMachineComponent::IsPerfectCancel(const FCombatActionData& FromAction, const FCombatActionData& ToAction) const
{
    // Perfect cancel = canceling within 3 frames of cancel window start
    int32 FramesIntoWindow = CurrentFrame - FromAction.CancelWindowStart;
    return FramesIntoWindow >= 0 && FramesIntoWindow <= 3;
}

void UCombatStateMachineComponent::HandlePerfectCancel(const FGameplayTag& CanceledInto)
{
    // Perfect cancel benefits
    ExtendComboTime(1.0f); // Extend combo window
    
    // Broadcast perfect cancel event
    OnPerfectCancel.Broadcast(CanceledInto);
    
    UE_LOG(LogTemp, Log, TEXT("Perfect Cancel executed into: %s"), *CanceledInto.ToString());
}

void UCombatStateMachineComponent::UpdateComboSystem(float DeltaTime)
{
    TimeSinceLastAction += DeltaTime;
    
    // Check for combo reset
    CheckComboReset();
}

void UCombatStateMachineComponent::AddToCombo(const FGameplayTag& ActionTag)
{
    // Reset combo timer
    TimeSinceLastAction = 0.0f;
    
    // Add to combo chain
    CurrentComboChain.Add(ActionTag);
    
    // Limit combo chain length
    if (CurrentComboChain.Num() > CombatConstants::MAX_COMBO_CHAIN_LENGTH)
    {
        CurrentComboChain.RemoveAt(0);
    }
    
    // Broadcast combo update
    OnComboUpdated.Broadcast(GetComboCount(), CurrentComboChain);
}

void UCombatStateMachineComponent::CheckComboReset()
{
    if (TimeSinceLastAction >= ComboResetTime && CurrentComboChain.Num() > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("Combo reset - final count: %d"), GetComboCount());
        ResetCombo();
    }
}

bool UCombatStateMachineComponent::MatchesHiddenComboSequence(const TArray<FGameplayTag>& Sequence) const
{
    if (Sequence.Num() == 0 || CurrentComboChain.Num() < Sequence.Num())
    {
        return false;
    }
    
    // Check if the last N actions match the sequence
    int32 StartIndex = CurrentComboChain.Num() - Sequence.Num();
    
    for (int32 i = 0; i < Sequence.Num(); ++i)
    {
        if (CurrentComboChain[StartIndex + i] != Sequence[i])
        {
            return false;
        }
    }
    
    return true;
}

void UCombatStateMachineComponent::ExecuteHiddenCombo(const FHiddenComboData& ComboData)
{
    UE_LOG(LogTemp, Log, TEXT("Hidden combo executed: %s"), *ComboData.ComboName);
    
    // Trigger special effects
    if (ComboData.SpecialEffectTag.IsValid())
    {
        // Could trigger special VFX, SFX, or gameplay effects here
    }
    
    // Broadcast hidden combo event
    OnHiddenComboExecuted.Broadcast(ComboData.ComboName);
}

void UCombatStateMachineComponent::FindComponentReferences()
{
    if (AActor* Owner = GetOwner())
    {
        OwnerCharacter = Cast<AMyCharacter>(Owner);
        CombatPrototype = Owner->FindComponentByClass<UCombatPrototypeComponent>();
        AoEComponent = Owner->FindComponentByClass<UAoEPrototypeComponent>();
    }
}

void UCombatStateMachineComponent::NotifyComponentsActionStarted(const FCombatActionData& ActionData)
{
    // Integrate with existing CombatPrototypeComponent
    if (CombatPrototype && ActionData.bUseCombatPrototype)
    {
        CombatPrototype->StartAttack(ActionData.CombatPrototypeName);
    }
    
    // Integrate with existing AoEPrototypeComponent
    if (AoEComponent && ActionData.bTriggerAoE && !ActionData.AoEPrototypeName.IsEmpty())
    {
        AoEComponent->StartAoE(ActionData.AoEPrototypeName);
    }
}

void UCombatStateMachineComponent::NotifyComponentsActionEnded(const FGameplayTag& ActionTag, bool bWasCanceled)
{
    // Stop any ongoing combat prototype actions
    if (CombatPrototype)
    {
        CombatPrototype->CancelAttack();
    }
    
    // AoE effects continue independently unless specifically stopped
}

int32 UCombatStateMachineComponent::GetExpectedFrameForState() const
{
    const FCombatActionData* ActionData = GetCurrentActionData();
    if (!ActionData)
    {
        return 0;
    }
    
    switch (CurrentState)
    {
        case ECombatState::Startup:
            return ActionData->StartupFrames;
        case ECombatState::Active:
            return ActionData->ActiveFrames;
        case ECombatState::Recovery:
            return ActionData->RecoveryFrames;
        default:
            return 0;
    }
}

bool UCombatStateMachineComponent::IsFrameInRange(int32 StartFrame, int32 EndFrame) const
{
    return CurrentFrame >= StartFrame && CurrentFrame <= EndFrame;
}

float UCombatStateMachineComponent::GetFrameProgress() const
{
    int32 ExpectedFrames = GetExpectedFrameForState();
    return ExpectedFrames > 0 ? static_cast<float>(CurrentFrame) / static_cast<float>(ExpectedFrames) : 0.0f;
}

const FCombatActionData* UCombatStateMachineComponent::GetCurrentActionData() const
{
    if (!CurrentActionTag.IsValid())
    {
        return nullptr;
    }
    
    return LoadedActions.Find(CurrentActionTag);
}

bool UCombatStateMachineComponent::IsActionValid(const FGameplayTag& ActionTag) const
{
    return HasActionData(ActionTag);
}

bool UCombatStateMachineComponent::ArePrerequisitesMet(const FCombatActionData& ActionData) const
{
    // Basic checks
    if (!ActionData.ActionTag.IsValid())
    {
        return false;
    }
    
    // Target requirement check
    if (ActionData.bRequiresTarget)
    {
        // Would need targeting system integration here
        // For now, assume target requirements are met
    }
    
    return true;
}

void UCombatStateMachineComponent::DrawDebugInfo()
{
    if (!GetWorld() || !GetOwner())
    {
        return;
    }
    
    const FVector OwnerLocation = GetOwner()->GetActorLocation();
    const FVector DebugLocation = OwnerLocation + FVector(0, 0, 150);
    
    // Draw state indicator
    FColor StateColor = FColor::White;
    switch (CurrentState)
    {
        case ECombatState::Idle: StateColor = FColor::White; break;
        case ECombatState::Startup: StateColor = FColor::Yellow; break;
        case ECombatState::Active: StateColor = FColor::Red; break;
        case ECombatState::Recovery: StateColor = FColor::Blue; break;
        case ECombatState::Canceling: StateColor = FColor::Purple; break;
    }
    
    // Draw debug text
    DrawDebugString(GetWorld(), DebugLocation, GetDebugStateInfo(), nullptr, StateColor, 0.0f);
    
    // Draw cancel window indicator
    if (IsInCancelWindow())
    {
        DrawDebugSphere(GetWorld(), OwnerLocation + FVector(0, 0, 100), 50.0f, 8, FColor::Green, false, -1.0f);
    }
    
    // Draw input buffer indicator
    if (HasBufferedInput())
    {
        FString BufferInfo = FString::Printf(TEXT("Buffered: %d inputs"), InputBuffer.Num());
        DrawDebugString(GetWorld(), DebugLocation + FVector(0, 0, -30), BufferInfo, nullptr, FColor::Cyan, 0.0f);
    }
}

void UCombatStateMachineComponent::LogStateTransition(ECombatState FromState, ECombatState ToState, const FGameplayTag& ActionTag)
{
    UE_LOG(LogTemp, Log, TEXT("Combat State: %d -> %d, Action: %s, Frame: %d"), 
           static_cast<int32>(FromState), static_cast<int32>(ToState), 
           *ActionTag.ToString(), CurrentFrame);
}

void UCombatStateMachineComponent::LogFrameInfo() const
{
    UE_LOG(LogTemp, VeryVerbose, TEXT("Frame: %d, State: %d, Progress: %.2f"), 
           CurrentFrame, static_cast<int32>(CurrentState), GetFrameProgress());
}
