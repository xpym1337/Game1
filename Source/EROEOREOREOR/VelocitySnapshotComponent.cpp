// Fill out your copyright notice in the Description page of Project Settings.

#include "VelocitySnapshotComponent.h"
#include "Engine/World.h"
#include "GameplayTagsModule.h"

UVelocitySnapshotComponent::UVelocitySnapshotComponent()
{
    // CRITICAL PERFORMANCE FIX: Start with ticking DISABLED
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;
    
    DefaultValidityDuration = 0.2f;
    bLogSnapshots = false;
    
    // Initialize ring buffer state
    CurrentIndex = 0;
    ValidCount = 0;
}

void UVelocitySnapshotComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // PERFORMANCE OPTIMIZATION: Start disabled, enable only when needed
    SetComponentTickEnabled(false);
    
    // Pre-allocate cache maps to avoid rehashing
    SourceIndexCache.Reserve(static_cast<int32>(EVelocitySource::External) + 1);
    TagIndexCache.Reserve(MAX_SNAPSHOTS);
    
    UE_LOG(LogTemp, Log, TEXT("VelocitySnapshotComponent: Initialized with conditional ticking"));
}

void UVelocitySnapshotComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // PERFORMANCE OPTIMIZATION: Efficient cleanup without array operations
    OptimizedCleanup();
    
    // CRITICAL PERFORMANCE FIX: Disable ticking when no valid snapshots remain
    if (ValidCount == 0)
    {
        SetComponentTickEnabled(false);
        if (bLogSnapshots)
        {
            UE_LOG(LogTemp, VeryVerbose, TEXT("VelocitySnapshot: Disabled ticking - no valid snapshots"));
        }
    }
}

void UVelocitySnapshotComponent::CaptureSnapshot(const FVector& Velocity, EVelocitySource Source, const FGameplayTag& SourceTag)
{
    // Skip zero velocity snapshots
    if (Velocity.IsNearlyZero(1.0f))
    {
        return;
    }

    // Create snapshot using ring buffer - NO ALLOCATIONS
    FVelocitySnapshot& NewSnapshot = SnapshotRingBuffer[CurrentIndex];
    NewSnapshot = FVelocitySnapshot(Velocity, Source, SourceTag);
    NewSnapshot.Timestamp = GetWorld()->GetTimeSeconds();
    NewSnapshot.ValidityDuration = DefaultValidityDuration;

    // Update caches for O(1) lookups
    SourceIndexCache.Add(Source, CurrentIndex);
    TagIndexCache.Add(SourceTag, CurrentIndex);

    // Advance ring buffer
    CurrentIndex = (CurrentIndex + 1) % MAX_SNAPSHOTS;
    ValidCount = FMath::Min(ValidCount + 1, MAX_SNAPSHOTS);

    // CRITICAL PERFORMANCE FIX: Enable ticking only when we have data
    if (!IsComponentTickEnabled())
    {
        SetComponentTickEnabled(true);
        if (bLogSnapshots)
        {
            UE_LOG(LogTemp, VeryVerbose, TEXT("VelocitySnapshot: Enabled ticking"));
        }
    }

    if (bLogSnapshots)
    {
        UE_LOG(LogTemp, Log, TEXT("VelocitySnapshot: Captured %s velocity: %.1f units/s"), 
            *SourceTag.ToString(), NewSnapshot.Speed);
    }
}

bool UVelocitySnapshotComponent::GetLatestSnapshot(FVelocitySnapshot& OutSnapshot) const
{
    const int32 RecentIndex = GetMostRecentValidIndex();
    if (RecentIndex != INDEX_NONE)
    {
        OutSnapshot = SnapshotRingBuffer[RecentIndex];
        return true;
    }
    return false;
}

bool UVelocitySnapshotComponent::GetSnapshotBySource(EVelocitySource Source, FVelocitySnapshot& OutSnapshot) const
{
    // PERFORMANCE OPTIMIZATION: O(1) lookup via cache
    if (const int32* IndexPtr = SourceIndexCache.Find(Source))
    {
        const FVelocitySnapshot& Snapshot = SnapshotRingBuffer[*IndexPtr];
        if (Snapshot.IsValid(GetWorld()->GetTimeSeconds()))
        {
            OutSnapshot = Snapshot;
            return true;
        }
    }
    return false;
}

bool UVelocitySnapshotComponent::GetSnapshotByTag(const FGameplayTag& SourceTag, FVelocitySnapshot& OutSnapshot) const
{
    // PERFORMANCE OPTIMIZATION: O(1) lookup via cache
    if (const int32* IndexPtr = TagIndexCache.Find(SourceTag))
    {
        const FVelocitySnapshot& Snapshot = SnapshotRingBuffer[*IndexPtr];
        if (Snapshot.IsValid(GetWorld()->GetTimeSeconds()))
        {
            OutSnapshot = Snapshot;
            return true;
        }
    }
    return false;
}

void UVelocitySnapshotComponent::ClearAllSnapshots()
{
    // Reset ring buffer state - no memory operations needed
    ValidCount = 0;
    CurrentIndex = 0;
    
    // Clear caches
    SourceIndexCache.Empty();
    TagIndexCache.Empty();
    
    // Disable ticking since we have no data
    if (IsComponentTickEnabled())
    {
        SetComponentTickEnabled(false);
    }
    
    if (bLogSnapshots)
    {
        UE_LOG(LogTemp, Log, TEXT("VelocitySnapshot: Cleared all snapshots"));
    }
}

void UVelocitySnapshotComponent::LogCurrentSnapshots() const
{
    UE_LOG(LogTemp, Warning, TEXT("=== VELOCITY SNAPSHOTS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Valid Count: %d/%d"), ValidCount, MAX_SNAPSHOTS);
    UE_LOG(LogTemp, Warning, TEXT("Current Index: %d"), CurrentIndex);
    UE_LOG(LogTemp, Warning, TEXT("Ticking Enabled: %s"), IsComponentTickEnabled() ? TEXT("true") : TEXT("false"));
    
    const float CurrentTime = GetWorld()->GetTimeSeconds();
    
    for (int32 i = 0; i < ValidCount; ++i)
    {
        const int32 Index = (CurrentIndex - ValidCount + i + MAX_SNAPSHOTS) % MAX_SNAPSHOTS;
        const FVelocitySnapshot& Snapshot = SnapshotRingBuffer[Index];
        
        UE_LOG(LogTemp, Warning, TEXT("  [%d] Source: %d, Speed: %.1f, Valid: %s, Age: %.3fs"),
            Index,
            static_cast<int32>(Snapshot.Source),
            Snapshot.Speed,
            Snapshot.IsValid(CurrentTime) ? TEXT("YES") : TEXT("NO"),
            CurrentTime - Snapshot.Timestamp);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("========================"));
}

int32 UVelocitySnapshotComponent::GetValidSnapshotCount() const
{
    const float CurrentTime = GetWorld()->GetTimeSeconds();
    int32 Count = 0;
    
    for (int32 i = 0; i < ValidCount; ++i)
    {
        const int32 Index = (CurrentIndex - ValidCount + i + MAX_SNAPSHOTS) % MAX_SNAPSHOTS;
        if (SnapshotRingBuffer[Index].IsValid(CurrentTime))
        {
            Count++;
        }
    }
    
    return Count;
}

bool UVelocitySnapshotComponent::HasValidSnapshots() const
{
    return GetValidSnapshotCount() > 0;
}

void UVelocitySnapshotComponent::OptimizedCleanup()
{
    const float CurrentTime = GetWorld()->GetTimeSeconds();
    int32 ExpiredCount = 0;
    
    // Check all valid entries in ring buffer
    for (int32 i = 0; i < ValidCount; ++i)
    {
        const int32 Index = (CurrentIndex - ValidCount + i + MAX_SNAPSHOTS) % MAX_SNAPSHOTS;
        const FVelocitySnapshot& Snapshot = SnapshotRingBuffer[Index];
        
        if (!Snapshot.IsValid(CurrentTime))
        {
            // Remove from caches
            SourceIndexCache.Remove(Snapshot.Source);
            TagIndexCache.Remove(Snapshot.SourceTag);
            ExpiredCount++;
        }
    }
    
    if (ExpiredCount > 0)
    {
        ValidCount = FMath::Max(0, ValidCount - ExpiredCount);
        UpdateCaches(); // Rebuild caches for remaining valid entries
        
        if (bLogSnapshots)
        {
            UE_LOG(LogTemp, VeryVerbose, TEXT("VelocitySnapshot: Cleaned %d expired snapshots"), ExpiredCount);
        }
    }
}

void UVelocitySnapshotComponent::UpdateCaches()
{
    // Rebuild caches for all valid entries
    SourceIndexCache.Empty();
    TagIndexCache.Empty();
    
    const float CurrentTime = GetWorld()->GetTimeSeconds();
    
    for (int32 i = 0; i < ValidCount; ++i)
    {
        const int32 Index = (CurrentIndex - ValidCount + i + MAX_SNAPSHOTS) % MAX_SNAPSHOTS;
        const FVelocitySnapshot& Snapshot = SnapshotRingBuffer[Index];
        
        if (Snapshot.IsValid(CurrentTime))
        {
            SourceIndexCache.Add(Snapshot.Source, Index);
            TagIndexCache.Add(Snapshot.SourceTag, Index);
        }
    }
}

int32 UVelocitySnapshotComponent::GetMostRecentValidIndex() const
{
    const float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Search backwards from most recent
    for (int32 i = ValidCount - 1; i >= 0; --i)
    {
        const int32 Index = (CurrentIndex - ValidCount + i + MAX_SNAPSHOTS) % MAX_SNAPSHOTS;
        if (SnapshotRingBuffer[Index].IsValid(CurrentTime))
        {
            return Index;
        }
    }
    
    return INDEX_NONE;
}
