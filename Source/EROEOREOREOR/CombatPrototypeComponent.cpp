#include "CombatPrototypeComponent.h"
#include "CombatStateMachineComponent.h"
#include "MyCharacter.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

UCombatPrototypeComponent::UCombatPrototypeComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Set reasonable defaults
    CurrentPhase = ECombatPhase::None;
    CurrentPhaseTime = 0.0f;
    TotalAttackTime = 0.0f;
    bHasConnectedThisAttack = false;
}

void UCombatPrototypeComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Cache owner reference
    OwnerCharacter = Cast<AMyCharacter>(GetOwner());
    
    // Auto-load data table if specified
    if (bAutoLoadDataTable && PrototypeDataTable)
    {
        LoadPrototypeData(PrototypeDataTable);
    }
    
    // Log initialization
    if (bDebugEnabled)
    {
        UE_LOG(LogTemp, Log, TEXT("CombatPrototypeComponent initialized for %s"), 
               GetOwner() ? *GetOwner()->GetName() : TEXT("NULL"));
    }
}

void UCombatPrototypeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (CurrentPhase != ECombatPhase::None)
    {
        UpdateCombatPhase(DeltaTime);
        UpdateMovementControl();
        UpdateTrajectoryMovement(DeltaTime);
        CheckForHits();
        
        if (bDebugEnabled && CurrentAttackData.bDebugVisualization)
        {
            DrawDebugVisualization();
        }
    }
}

void UCombatPrototypeComponent::StartAttack(const FString& PrototypeName)
{
    if (const FCombatPrototypeData* FoundData = LoadedPrototypes.Find(PrototypeName))
    {
        StartAttackWithData(*FoundData);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Combat prototype '%s' not found!"), *PrototypeName);
    }
}

void UCombatPrototypeComponent::StartAttackWithData(const FCombatPrototypeData& AttackData)
{
    // Cancel any current attack
    if (CurrentPhase != ECombatPhase::None)
    {
        CancelAttack();
    }
    
    // Initialize attack
    CurrentAttackData = AttackData;
    AttackStartLocation = GetOwner()->GetActorLocation();
    OriginalLocation = AttackStartLocation;
    OriginalRotation = GetOwner()->GetActorRotation();
    
    // Calculate target location if not already set
    if (AttackTargetLocation.IsZero())
    {
        const FVector ForwardVector = GetOwner()->GetActorForwardVector();
        AttackTargetLocation = AttackStartLocation + (ForwardVector * CurrentAttackData.TrajectoryData.MaxDistance);
    }
    
    // Reset state
    CurrentPhaseTime = 0.0f;
    TotalAttackTime = 0.0f;
    bHasConnectedThisAttack = false;
    AlreadyHitActors.Empty();
    
    // Start startup phase
    SetPhase(ECombatPhase::Startup);
    
    if (bDebugEnabled)
    {
        UE_LOG(LogTemp, Log, TEXT("Started attack: %s"), *CurrentAttackData.PrototypeName);
    }
}

void UCombatPrototypeComponent::CancelAttack()
{
    if (CurrentPhase != ECombatPhase::None && CanCancelCurrentAttack())
    {
        SetPhase(ECombatPhase::Canceled);
        
        // Reset to original position if needed
        if (OwnerCharacter.IsValid())
        {
            // Restore original movement settings
            if (UCharacterMovementComponent* MovementComp = OwnerCharacter->GetCharacterMovement())
            {
                MovementComp->SetMovementMode(MOVE_Walking);
            }
        }
        
        if (bDebugEnabled)
        {
            UE_LOG(LogTemp, Log, TEXT("Attack canceled: %s"), *CurrentAttackData.PrototypeName);
        }
        
        // End the attack after brief cancel phase
        FTimerHandle CancelTimer;
        GetWorld()->GetTimerManager().SetTimer(CancelTimer, [this]()
        {
            SetPhase(ECombatPhase::None);
        }, 0.1f, false);
    }
}

bool UCombatPrototypeComponent::CanCancelCurrentAttack() const
{
    switch (CurrentPhase)
    {
        case ECombatPhase::Startup:
            return true; // Can always cancel startup
            
        case ECombatPhase::Active:
        {
            const float ActiveProgress = GetPhaseProgress();
            return ActiveProgress >= CurrentAttackData.TimingData.EarliestCancelWindow;
        }
        
        case ECombatPhase::Recovery:
            return CurrentAttackData.TimingData.bCanCancelIntoOtherAttacks || 
                   CurrentAttackData.TimingData.bCanCancelIntoMovement;
            
        default:
            return false;
    }
}

float UCombatPrototypeComponent::GetPhaseProgress() const
{
    float PhaseDuration = 0.0f;
    
    switch (CurrentPhase)
    {
        case ECombatPhase::Startup:
            PhaseDuration = CurrentAttackData.TimingData.StartupDuration;
            break;
        case ECombatPhase::Active:
            PhaseDuration = CurrentAttackData.TimingData.ActiveDuration;
            break;
        case ECombatPhase::Recovery:
            PhaseDuration = CurrentAttackData.TimingData.RecoveryDuration;
            break;
        default:
            return 0.0f;
    }
    
    return PhaseDuration > 0.0f ? FMath::Clamp(CurrentPhaseTime / PhaseDuration, 0.0f, 1.0f) : 1.0f;
}

float UCombatPrototypeComponent::GetTotalAttackDuration() const
{
    return CurrentAttackData.TimingData.StartupDuration + 
           CurrentAttackData.TimingData.ActiveDuration + 
           CurrentAttackData.TimingData.RecoveryDuration;
}

void UCombatPrototypeComponent::SetTrajectoryTarget(const FVector& WorldPosition)
{
    AttackTargetLocation = WorldPosition;
    TargetActor = nullptr;
}

void UCombatPrototypeComponent::SetTrajectoryTargetActor(AActor* NewTargetActor)
{
    TargetActor = NewTargetActor;
    if (NewTargetActor)
    {
        AttackTargetLocation = NewTargetActor->GetActorLocation();
    }
}

FVector UCombatPrototypeComponent::GetCurrentTrajectoryPosition() const
{
    if (CurrentPhase == ECombatPhase::Active)
    {
        const float Progress = GetPhaseProgress();
        return CalculateTrajectoryPosition(Progress);
    }
    
    return GetOwner()->GetActorLocation();
}

FVector UCombatPrototypeComponent::GetPredictedEndPosition() const
{
    return CalculateTrajectoryPosition(1.0f);
}

void UCombatPrototypeComponent::LoadPrototypeData(UDataTable* DataTable)
{
    if (!DataTable)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot load null data table"));
        return;
    }
    
    LoadedPrototypes.Empty();
    
    TArray<FCombatPrototypeData*> AllRows;
    DataTable->GetAllRows<FCombatPrototypeData>(TEXT("LoadPrototypeData"), AllRows);
    
    for (const FCombatPrototypeData* Row : AllRows)
    {
        if (Row)
        {
            LoadedPrototypes.Add(Row->PrototypeName, *Row);
        }
    }
    
    if (bDebugEnabled)
    {
        UE_LOG(LogTemp, Log, TEXT("Loaded %d combat prototypes"), LoadedPrototypes.Num());
    }
}

FCombatPrototypeData UCombatPrototypeComponent::GetPrototypeData(const FString& PrototypeName) const
{
    if (const FCombatPrototypeData* FoundData = LoadedPrototypes.Find(PrototypeName))
    {
        return *FoundData;
    }
    
    return FCombatPrototypeData();
}

TArray<FString> UCombatPrototypeComponent::GetAvailablePrototypes() const
{
    TArray<FString> PrototypeNames;
    LoadedPrototypes.GetKeys(PrototypeNames);
    return PrototypeNames;
}

void UCombatPrototypeComponent::ModifyCurrentTimingData(const FCombatTimingData& NewTimingData)
{
    CurrentAttackData.TimingData = NewTimingData;
    
    if (bDebugEnabled)
    {
        UE_LOG(LogTemp, Log, TEXT("Modified timing data for current attack"));
    }
}

void UCombatPrototypeComponent::ModifyCurrentTrajectoryData(const FTrajectoryData& NewTrajectoryData)
{
    CurrentAttackData.TrajectoryData = NewTrajectoryData;
    
    if (bDebugEnabled)
    {
        UE_LOG(LogTemp, Log, TEXT("Modified trajectory data for current attack"));
    }
}

void UCombatPrototypeComponent::ModifyCurrentMovementData(const FMovementControlData& NewMovementData)
{
    CurrentAttackData.MovementData = NewMovementData;
    UpdateMovementControl(); // Apply immediately
    
    if (bDebugEnabled)
    {
        UE_LOG(LogTemp, Log, TEXT("Modified movement data for current attack"));
    }
}

void UCombatPrototypeComponent::SetDebugVisualization(bool bEnabled)
{
    bDebugEnabled = bEnabled;
    CurrentAttackData.bDebugVisualization = bEnabled;
}

void UCombatPrototypeComponent::TestCurrentPrototype()
{
    if (LoadedPrototypes.Num() > 0)
    {
        TArray<FString> PrototypeNames;
        LoadedPrototypes.GetKeys(PrototypeNames);
        StartAttack(PrototypeNames[0]);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No prototypes loaded for testing"));
    }
}

void UCombatPrototypeComponent::PreviewTrajectory()
{
    if (GetWorld() && GetOwner())
    {
        const FVector StartLocation = GetOwner()->GetActorLocation();
        const FVector EndLocation = StartLocation + (GetOwner()->GetActorForwardVector() * 500.0f);
        
        // Draw trajectory preview
        const int32 Steps = 20;
        FVector PreviousPos = StartLocation;
        
        for (int32 i = 1; i <= Steps; ++i)
        {
            const float Alpha = static_cast<float>(i) / static_cast<float>(Steps);
            const FVector CurrentPos = FMath::Lerp(StartLocation, EndLocation, Alpha);
            
            DrawDebugLine(GetWorld(), PreviousPos, CurrentPos, FColor::Yellow, false, 2.0f, 0, 2.0f);
            PreviousPos = CurrentPos;
        }
        
        DrawDebugSphere(GetWorld(), StartLocation, 20.0f, 8, FColor::Green, false, 2.0f);
        DrawDebugSphere(GetWorld(), EndLocation, 20.0f, 8, FColor::Red, false, 2.0f);
    }
}

void UCombatPrototypeComponent::StartActionFromStateMachine(const FCombatActionData& ActionData)
{
    // Convert FCombatActionData to FCombatPrototypeData and start attack
    if (!ActionData.bUseCombatPrototype || ActionData.CombatPrototypeName.IsEmpty())
    {
        return;
    }
    
    // Find the prototype data
    if (const FCombatPrototypeData* PrototypeData = LoadedPrototypes.Find(ActionData.CombatPrototypeName))
    {
        StartAttackWithData(*PrototypeData);
    }
    else
    {
        // Create temporary prototype data from action data
        FCombatPrototypeData TempPrototypeData;
        TempPrototypeData.PrototypeName = ActionData.DisplayName;
        TempPrototypeData.AttackTag = ActionData.ActionTag;
        TempPrototypeData.TimingData.StartupDuration = ActionData.GetStartupDurationSeconds();
        TempPrototypeData.TimingData.ActiveDuration = ActionData.GetActiveDurationSeconds(); 
        TempPrototypeData.TimingData.RecoveryDuration = ActionData.GetRecoveryDurationSeconds();
        TempPrototypeData.MovementData.MovementSpeedMultiplier = ActionData.MovementSpeedMultiplier;
        TempPrototypeData.TrajectoryData.MaxDistance = ActionData.Range;
        TempPrototypeData.Damage = 25.0f; // Default damage, should be in ActionData
        
        StartAttackWithData(TempPrototypeData);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Started action from state machine: %s"), *ActionData.DisplayName);
}

void UCombatPrototypeComponent::EndActionFromStateMachine(bool bWasCanceled)
{
    if (bWasCanceled)
    {
        CancelAttack();
    }
    else
    {
        // Natural end - just reset phase
        SetPhase(ECombatPhase::None);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Ended action from state machine (Canceled: %s)"), bWasCanceled ? TEXT("Yes") : TEXT("No"));
}

// Private Methods

void UCombatPrototypeComponent::UpdateCombatPhase(float DeltaTime)
{
    CurrentPhaseTime += DeltaTime;
    TotalAttackTime += DeltaTime;
    
    float CurrentPhaseDuration = 0.0f;
    ECombatPhase NextPhase = ECombatPhase::None;
    
    switch (CurrentPhase)
    {
        case ECombatPhase::Startup:
            CurrentPhaseDuration = CurrentAttackData.TimingData.StartupDuration;
            NextPhase = ECombatPhase::Active;
            break;
            
        case ECombatPhase::Active:
            CurrentPhaseDuration = CurrentAttackData.TimingData.ActiveDuration;
            NextPhase = ECombatPhase::Recovery;
            break;
            
        case ECombatPhase::Recovery:
            CurrentPhaseDuration = CurrentAttackData.TimingData.RecoveryDuration;
            NextPhase = ECombatPhase::None;
            break;
            
        default:
            return;
    }
    
    // Transition to next phase
    if (CurrentPhaseTime >= CurrentPhaseDuration)
    {
        SetPhase(NextPhase);
    }
}

void UCombatPrototypeComponent::UpdateMovementControl()
{
    if (!OwnerCharacter.IsValid())
        return;
        
    UCharacterMovementComponent* MovementComp = OwnerCharacter->GetCharacterMovement();
    if (!MovementComp)
        return;
    
    EMovementControlType ControlType = EMovementControlType::None;
    
    switch (CurrentPhase)
    {
        case ECombatPhase::Startup:
            ControlType = CurrentAttackData.MovementData.StartupControl;
            break;
        case ECombatPhase::Active:
            ControlType = CurrentAttackData.MovementData.ActiveControl;
            break;
        case ECombatPhase::Recovery:
            ControlType = CurrentAttackData.MovementData.RecoveryControl;
            break;
        default:
            return;
    }
    
    switch (ControlType)
    {
        case EMovementControlType::LockPosition:
            MovementComp->SetMovementMode(MOVE_None);
            break;
            
        case EMovementControlType::LockBoth:
            MovementComp->SetMovementMode(MOVE_None);
            // Lock rotation is handled in trajectory movement
            break;
            
        case EMovementControlType::CustomControl:
            MovementComp->SetMovementMode(MOVE_Flying);
            MovementComp->MaxFlySpeed = MovementComp->MaxWalkSpeed * CurrentAttackData.MovementData.MovementSpeedMultiplier;
            break;
            
        default:
            MovementComp->SetMovementMode(MOVE_Walking);
            break;
    }
}

void UCombatPrototypeComponent::UpdateTrajectoryMovement(float DeltaTime)
{
    if (CurrentPhase != ECombatPhase::Active || !OwnerCharacter.IsValid())
        return;
    
    // Update target location if following an actor
    if (TargetActor.IsValid())
    {
        AttackTargetLocation = TargetActor->GetActorLocation();
    }
    
    const float Progress = GetPhaseProgress();
    FVector TargetPosition = CalculateTrajectoryPosition(Progress);
    
    // Apply movement based on trajectory type
    if (CurrentAttackData.TrajectoryData.TrajectoryType == ETrajectoryType::Teleport)
    {
        OwnerCharacter->SetActorLocation(TargetPosition);
    }
    else
    {
        // Smooth movement toward target position
        const FVector CurrentLocation = OwnerCharacter->GetActorLocation();
        const FVector Direction = (TargetPosition - CurrentLocation).GetSafeNormal();
        
        float MovementSpeed = 1000.0f; // Base speed
        if (CurrentAttackData.TrajectoryData.TrajectorySpeedCurve)
        {
            MovementSpeed *= CurrentAttackData.TrajectoryData.TrajectorySpeedCurve->GetFloatValue(Progress);
        }
        
        const FVector NewLocation = FMath::VInterpTo(CurrentLocation, TargetPosition, DeltaTime, MovementSpeed / 100.0f);
        OwnerCharacter->SetActorLocation(NewLocation);
        
        // Handle rotation
        if (CurrentAttackData.MovementData.bCanRotateDuringAttack && !Direction.IsZero())
        {
            const FRotator TargetRotation = FRotationMatrix::MakeFromX(Direction).Rotator();
            const FRotator NewRotation = FMath::RInterpTo(
                OwnerCharacter->GetActorRotation(), 
                TargetRotation, 
                DeltaTime, 
                CurrentAttackData.MovementData.RotationRate
            );
            OwnerCharacter->SetActorRotation(NewRotation);
        }
    }
}

void UCombatPrototypeComponent::CheckForHits()
{
    if (CurrentPhase != ECombatPhase::Active || !OwnerCharacter.IsValid())
        return;
    
    // Simple sphere trace for hits
    const FVector StartLocation = OwnerCharacter->GetActorLocation();
    const float HitRadius = 100.0f;
    
    TArray<FHitResult> HitResults;
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
    
    TArray<AActor*> ActorsToIgnore;
    ActorsToIgnore.Add(OwnerCharacter.Get());
    ActorsToIgnore.Append(AlreadyHitActors);
    
    bool bHit = UKismetSystemLibrary::SphereTraceMultiForObjects(
        GetWorld(),
        StartLocation,
        StartLocation,
        HitRadius,
        ObjectTypes,
        false,
        ActorsToIgnore,
        bDebugEnabled ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None,
        HitResults,
        true
    );
    
    if (bHit)
    {
        for (const FHitResult& Hit : HitResults)
        {
            if (Hit.GetActor() && !AlreadyHitActors.Contains(Hit.GetActor()))
            {
                // Add to hit actors to prevent multiple hits
                AlreadyHitActors.Add(Hit.GetActor());
                bHasConnectedThisAttack = true;
                
                // Broadcast hit event
                OnAttackConnected.Broadcast(Hit.GetActor());
                
                if (bDebugEnabled)
                {
                    UE_LOG(LogTemp, Log, TEXT("Attack hit: %s"), *Hit.GetActor()->GetName());
                }
            }
        }
    }
}

void UCombatPrototypeComponent::DrawDebugVisualization()
{
    if (!GetWorld() || !OwnerCharacter.IsValid())
        return;
    
    const FVector CurrentLocation = OwnerCharacter->GetActorLocation();
    
    // Draw phase indicator
    FColor PhaseColor = FColor::White;
    switch (CurrentPhase)
    {
        case ECombatPhase::Startup: PhaseColor = FColor::Yellow; break;
        case ECombatPhase::Active: PhaseColor = FColor::Red; break;
        case ECombatPhase::Recovery: PhaseColor = FColor::Blue; break;
        case ECombatPhase::Canceled: PhaseColor = FColor::Purple; break;
    }
    
    // Draw character outline
    DrawDebugCapsule(GetWorld(), CurrentLocation, 88.0f, 44.0f, FQuat::Identity, PhaseColor, false, -1.0f, 0, 3.0f);
    
    // Draw trajectory path
    if (CurrentPhase == ECombatPhase::Active)
    {
        const int32 Steps = 10;
        FVector PreviousPos = AttackStartLocation;
        
        for (int32 i = 1; i <= Steps; ++i)
        {
            const float Alpha = static_cast<float>(i) / static_cast<float>(Steps);
            const FVector StepPos = CalculateTrajectoryPosition(Alpha);
            
            DrawDebugLine(GetWorld(), PreviousPos, StepPos, FColor::Orange, false, -1.0f, 0, 2.0f);
            PreviousPos = StepPos;
        }
    }
    
    // Draw target location
    DrawDebugSphere(GetWorld(), AttackTargetLocation, 30.0f, 8, FColor::Green, false, -1.0f);
    
    // Draw phase progress bar
    const FVector ProgressBarStart = CurrentLocation + FVector(0, 0, 120);
    const FVector ProgressBarEnd = ProgressBarStart + FVector(100, 0, 0);
    const float Progress = GetPhaseProgress();
    const FVector ProgressPoint = FMath::Lerp(ProgressBarStart, ProgressBarEnd, Progress);
    
    DrawDebugLine(GetWorld(), ProgressBarStart, ProgressBarEnd, FColor::White, false, -1.0f, 0, 5.0f);
    DrawDebugLine(GetWorld(), ProgressBarStart, ProgressPoint, PhaseColor, false, -1.0f, 0, 8.0f);
}

void UCombatPrototypeComponent::SetPhase(ECombatPhase NewPhase)
{
    const ECombatPhase OldPhase = CurrentPhase;
    CurrentPhase = NewPhase;
    CurrentPhaseTime = 0.0f;
    
    // Broadcast phase change
    OnPhaseChanged.Broadcast(OldPhase, NewPhase);
    
    if (bDebugEnabled)
    {
        UE_LOG(LogTemp, Log, TEXT("Phase changed from %d to %d"), static_cast<int32>(OldPhase), static_cast<int32>(NewPhase));
    }
}

FVector UCombatPrototypeComponent::CalculateTrajectoryPosition(float Alpha) const
{
    switch (CurrentAttackData.TrajectoryData.TrajectoryType)
    {
        case ETrajectoryType::Linear:
            return CalculateLinearTrajectory(Alpha);
        case ETrajectoryType::Arc:
            return CalculateArcTrajectory(Alpha);
        case ETrajectoryType::Homing:
            return CalculateHomingTrajectory(Alpha);
        case ETrajectoryType::Curve:
            return CalculateCurveTrajectory(Alpha);
        case ETrajectoryType::Teleport:
            return Alpha >= 1.0f ? AttackTargetLocation : AttackStartLocation;
        default:
            return CalculateLinearTrajectory(Alpha);
    }
}

FVector UCombatPrototypeComponent::CalculateLinearTrajectory(float Alpha) const
{
    const FVector StartPos = AttackStartLocation + CurrentAttackData.TrajectoryData.StartOffset;
    const FVector EndPos = AttackTargetLocation + CurrentAttackData.TrajectoryData.EndOffset;
    return FMath::Lerp(StartPos, EndPos, Alpha);
}

FVector UCombatPrototypeComponent::CalculateArcTrajectory(float Alpha) const
{
    const FVector StartPos = AttackStartLocation + CurrentAttackData.TrajectoryData.StartOffset;
    const FVector EndPos = AttackTargetLocation + CurrentAttackData.TrajectoryData.EndOffset;
    const FVector LinearPos = FMath::Lerp(StartPos, EndPos, Alpha);
    
    // Add arc height using sine wave
    const float ArcHeight = CurrentAttackData.TrajectoryData.ArcHeight * FMath::Sin(Alpha * PI);
    return LinearPos + FVector(0, 0, ArcHeight);
}

FVector UCombatPrototypeComponent::CalculateHomingTrajectory(float Alpha) const
{
    // Update target position if following an actor
    FVector CurrentTarget = AttackTargetLocation;
    if (TargetActor.IsValid())
    {
        CurrentTarget = TargetActor->GetActorLocation();
    }
    
    const FVector StartPos = AttackStartLocation + CurrentAttackData.TrajectoryData.StartOffset;
    const FVector EndPos = CurrentTarget + CurrentAttackData.TrajectoryData.EndOffset;
    
    // Apply homing strength
    const float HomingStrength = FMath::Clamp(CurrentAttackData.TrajectoryData.HomingStrength, 0.0f, 1.0f);
    const FVector LinearPos = FMath::Lerp(StartPos, EndPos, Alpha);
    const FVector DirectPos = StartPos + (EndPos - StartPos).GetSafeNormal() * (EndPos - StartPos).Size() * Alpha;
    
    return FMath::Lerp(DirectPos, LinearPos, HomingStrength);
}

FVector UCombatPrototypeComponent::CalculateCurveTrajectory(float Alpha) const
{
    FVector BasePos = CalculateLinearTrajectory(Alpha);
    
    if (CurrentAttackData.TrajectoryData.TrajectoryPathCurve)
    {
        const float CurveValue = CurrentAttackData.TrajectoryData.TrajectoryPathCurve->GetFloatValue(Alpha);
        // Apply curve modification to trajectory
        BasePos += OwnerCharacter->GetActorRightVector() * CurveValue * 100.0f;
    }
    
    return BasePos;
}
