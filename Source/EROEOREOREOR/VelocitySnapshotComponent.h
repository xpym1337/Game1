// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Engine/Engine.h"
#include "VelocitySnapshotComponent.generated.h"

UENUM(BlueprintType)
enum class EVelocitySource : uint8
{
    None        UMETA(DisplayName = "None"),
    Dash        UMETA(DisplayName = "Dash"),
    Jump        UMETA(DisplayName = "Jump"),
    Fall        UMETA(DisplayName = "Fall"),
    External    UMETA(DisplayName = "External")
};

USTRUCT(BlueprintType)
struct EROEOREOREOR_API FVelocitySnapshot
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Velocity")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Velocity")
    FVector Direction = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Velocity")
    float Speed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Velocity")
    EVelocitySource Source = EVelocitySource::None;

    UPROPERTY(BlueprintReadOnly, Category = "Velocity")
    FGameplayTag SourceTag;

    UPROPERTY(BlueprintReadOnly, Category = "Velocity")
    float Timestamp = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Velocity")
    float ValidityDuration = 0.2f;

    FVelocitySnapshot() = default;

    FVelocitySnapshot(const FVector& InVelocity, EVelocitySource InSource, const FGameplayTag& InSourceTag)
        : Velocity(InVelocity)
        , Direction(InVelocity.GetSafeNormal())
        , Speed(InVelocity.Size())
        , Source(InSource)
        , SourceTag(InSourceTag)
        , Timestamp(0.0f)
        , ValidityDuration(0.2f)
    {
    }

    bool IsValid(float CurrentTime) const
    {
        return (CurrentTime - Timestamp) <= ValidityDuration && Speed > 1.0f;
    }
};

/**
 * PERFORMANCE-OPTIMIZED Component for capturing and managing velocity snapshots
 * Features:
 * - Conditional ticking (only when snapshots exist)
 * - Fixed-size ring buffer (zero allocations)
 * - O(1) lookups via cached indices
 * - Automatic cleanup without expensive array operations
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType)
class EROEOREOREOR_API UVelocitySnapshotComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UVelocitySnapshotComponent();

    // Core API - Blueprint accessible for design iteration
    UFUNCTION(BlueprintCallable, Category = "Velocity Snapshot")
    void CaptureSnapshot(const FVector& Velocity, EVelocitySource Source, const FGameplayTag& SourceTag);

    UFUNCTION(BlueprintCallable, Category = "Velocity Snapshot", BlueprintPure)
    bool GetLatestSnapshot(FVelocitySnapshot& OutSnapshot) const;

    UFUNCTION(BlueprintCallable, Category = "Velocity Snapshot", BlueprintPure)
    bool GetSnapshotBySource(EVelocitySource Source, FVelocitySnapshot& OutSnapshot) const;

    UFUNCTION(BlueprintCallable, Category = "Velocity Snapshot", BlueprintPure)
    bool GetSnapshotByTag(const FGameplayTag& SourceTag, FVelocitySnapshot& OutSnapshot) const;

    UFUNCTION(BlueprintCallable, Category = "Velocity Snapshot")
    void ClearAllSnapshots();

    // Configuration - Exposed for tuning in engine
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Velocity Snapshot",
        meta = (ClampMin = "0.05", ClampMax = "1.0", UIMin = "0.1", UIMax = "0.5"))
    float DefaultValidityDuration = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Velocity Snapshot|Debug")
    bool bLogSnapshots = false;

    // Testing utilities - Exposed for engine debugging
    UFUNCTION(BlueprintCallable, Category = "Velocity Snapshot|Testing", CallInEditor)
    void LogCurrentSnapshots() const;

    UFUNCTION(BlueprintPure, Category = "Velocity Snapshot|Testing")
    int32 GetValidSnapshotCount() const;

    UFUNCTION(BlueprintPure, Category = "Velocity Snapshot|Testing")
    bool HasValidSnapshots() const;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    // PERFORMANCE OPTIMIZATION: Fixed-size ring buffer eliminates allocations
    static constexpr int32 MAX_SNAPSHOTS = 8;
    FVelocitySnapshot SnapshotRingBuffer[MAX_SNAPSHOTS];
    int32 CurrentIndex = 0;
    int32 ValidCount = 0;
    
    // PERFORMANCE OPTIMIZATION: O(1) lookups via cached indices
    TMap<EVelocitySource, int32> SourceIndexCache;
    
    // PERFORMANCE OPTIMIZATION: Cached GameplayTag handles for fast comparison
    mutable TMap<FGameplayTag, int32> TagIndexCache;
    
    void OptimizedCleanup();
    void UpdateCaches();
    int32 GetMostRecentValidIndex() const;
};
