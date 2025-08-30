#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "Curves/CurveFloat.h"
#include "GameplayTagContainer.h"
#include "AoEPrototypeComponent.generated.h"

UENUM(BlueprintType)
enum class EAoEShape : uint8
{
    Circle      UMETA(DisplayName = "Circle"),
    Cone        UMETA(DisplayName = "Cone"),
    Rectangle   UMETA(DisplayName = "Rectangle"), 
    Line        UMETA(DisplayName = "Line"),
    Ring        UMETA(DisplayName = "Ring"),
    Sector      UMETA(DisplayName = "Sector (Pie Slice)"),
    Custom      UMETA(DisplayName = "Custom Shape")
};

UENUM(BlueprintType)
enum class EAoEBehavior : uint8
{
    Instant         UMETA(DisplayName = "Instant"),
    Expanding       UMETA(DisplayName = "Expanding Wave"),
    Projectile      UMETA(DisplayName = "Projectile"),
    Delayed         UMETA(DisplayName = "Delayed (Timer)"),
    Persistent      UMETA(DisplayName = "Persistent Field"),
    Traveling       UMETA(DisplayName = "Traveling Wave")
};

UENUM(BlueprintType)
enum class EAoEOrigin : uint8
{
    Caster          UMETA(DisplayName = "From Caster"),
    Target          UMETA(DisplayName = "At Target"),
    Projectile      UMETA(DisplayName = "From Projectile"),
    AboveCaster     UMETA(DisplayName = "Above Caster"),
    AboveTarget     UMETA(DisplayName = "Above Target"),
    Custom          UMETA(DisplayName = "Custom Location")
};

USTRUCT(BlueprintType)
struct FAoEShapeData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shape")
    EAoEShape Shape = EAoEShape::Circle;

    // Circle/Ring parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shape", meta = (ClampMin = "0.0"))
    float Radius = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shape", meta = (ClampMin = "0.0"))
    float InnerRadius = 0.0f; // For ring shapes

    // Cone/Sector parameters  
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shape", meta = (ClampMin = "0.0", ClampMax = "360.0"))
    float Angle = 60.0f;

    // Rectangle/Line parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shape")
    FVector2D Dimensions = FVector2D(400.0f, 200.0f);

    // Line specific
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shape", meta = (ClampMin = "0.0"))
    float Length = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shape", meta = (ClampMin = "0.0"))
    float Width = 100.0f;

    // Height for 3D shapes
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shape")
    float Height = 200.0f;

    // Custom shape points (for complex shapes)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shape")
    TArray<FVector2D> CustomShapePoints;
};

USTRUCT(BlueprintType)
struct FAoEBehaviorData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    EAoEBehavior Behavior = EAoEBehavior::Instant;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    EAoEOrigin Origin = EAoEOrigin::Caster;

    // Timing
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior", meta = (ClampMin = "0.0"))
    float StartDelay = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior", meta = (ClampMin = "0.0"))
    float Duration = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior", meta = (ClampMin = "0.0"))
    float Lifetime = 3.0f; // For persistent effects

    // Expansion (for expanding waves)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior", meta = (ClampMin = "0.0"))
    float ExpansionSpeed = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    UCurveFloat* ExpansionCurve; // Custom expansion timing

    // Projectile (for projectile-based AoEs)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior", meta = (ClampMin = "0.0"))
    float ProjectileSpeed = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior", meta = (ClampMin = "0.0"))
    float ProjectileRange = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bProjectileArc = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float ProjectileArcHeight = 200.0f;

    // Traveling wave
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior", meta = (ClampMin = "0.0"))
    float TravelSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    FVector TravelDirection = FVector(1, 0, 0);
};

USTRUCT(BlueprintType)
struct FAoEDamageData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    float Damage = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    float Knockback = 300.0f;

    // Damage falloff from center
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    bool bUseDamageFalloff = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    UCurveFloat* DamageFalloffCurve;

    // Hit frequency for persistent effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage", meta = (ClampMin = "0.0"))
    float HitInterval = 0.5f; // How often to damage actors in persistent AoE

    // Maximum hits per actor
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    int32 MaxHitsPerActor = 1;

    // Multi-hit timing
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    bool bAllowMultipleHits = false;
};

USTRUCT(BlueprintType)
struct FAoEPrototypeData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AoE Prototype")
    FString PrototypeName = "BasicAoE";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AoE Prototype")
    FGameplayTag AoETag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AoE Prototype")
    FAoEShapeData ShapeData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AoE Prototype")
    FAoEBehaviorData BehaviorData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AoE Prototype")
    FAoEDamageData DamageData;

    // Visual debugging
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AoE Prototype")
    bool bDebugVisualization = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AoE Prototype")
    FLinearColor DebugColor = FLinearColor::Red;
};

USTRUCT()
struct FActiveAoE
{
    GENERATED_BODY()

    FString Name;
    FAoEPrototypeData Data;
    float ElapsedTime = 0.0f;
    FVector Location = FVector::ZeroVector;
    FVector ProjectileLocation = FVector::ZeroVector;
    TArray<AActor*> HitActors;
    TMap<AActor*, int32> ActorHitCounts;
    TMap<AActor*, float> LastHitTimes;
    bool bIsActive = false;
    bool bProjectileActive = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnAoEHit, AActor*, HitActor, FVector, HitLocation, float, DamageAmount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAoEComplete, const FString&, AoEName);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class EROEOREOREOR_API UAoEPrototypeComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAoEPrototypeComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core AoE Control
    UFUNCTION(BlueprintCallable, Category = "AoE Prototype")
    void StartAoE(const FString& PrototypeName);

    UFUNCTION(BlueprintCallable, Category = "AoE Prototype")
    void StartAoEWithData(const FAoEPrototypeData& AoEData);

    UFUNCTION(BlueprintCallable, Category = "AoE Prototype")
    void StartAoEAtLocation(const FString& PrototypeName, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "AoE Prototype")
    void StartAoEAtTarget(const FString& PrototypeName, AActor* TargetActor);

    UFUNCTION(BlueprintCallable, Category = "AoE Prototype")
    void StopAoE(const FString& PrototypeName);

    UFUNCTION(BlueprintCallable, Category = "AoE Prototype")
    void StopAllAoEs();

    // Query active AoEs
    UFUNCTION(BlueprintPure, Category = "AoE Prototype")
    TArray<FString> GetActiveAoEs() const;

    UFUNCTION(BlueprintPure, Category = "AoE Prototype")
    bool IsAoEActive(const FString& PrototypeName) const;

    // Data Management
    UFUNCTION(BlueprintCallable, Category = "AoE Prototype")
    void LoadAoEData(UDataTable* DataTable);

    UFUNCTION(BlueprintCallable, Category = "AoE Prototype")
    FAoEPrototypeData GetAoEData(const FString& PrototypeName) const;

    UFUNCTION(BlueprintCallable, Category = "AoE Prototype")
    TArray<FString> GetAvailableAoEPrototypes() const;

    // Runtime Modification
    UFUNCTION(BlueprintCallable, Category = "AoE Prototype")
    void ModifyAoEShapeData(const FString& PrototypeName, const FAoEShapeData& NewShapeData);

    UFUNCTION(BlueprintCallable, Category = "AoE Prototype")
    void ModifyAoEBehaviorData(const FString& PrototypeName, const FAoEBehaviorData& NewBehaviorData);

    // Debug & Testing
    UFUNCTION(BlueprintCallable, Category = "AoE Prototype")
    void SetDebugVisualization(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "AoE Prototype", CallInEditor)
    void TestAoEPrototype(const FString& PrototypeName);

    UFUNCTION(BlueprintCallable, Category = "AoE Prototype", CallInEditor)
    void PreviewAoEShape(const FString& PrototypeName);

    // Events
    UPROPERTY(BlueprintAssignable, Category = "AoE Prototype")
    FOnAoEHit OnAoEHit;

    UPROPERTY(BlueprintAssignable, Category = "AoE Prototype")
    FOnAoEComplete OnAoEComplete;

protected:
    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    UDataTable* AoEDataTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bAutoLoadDataTable = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bDebugEnabled = true;

private:
    // Active AoE tracking
    UPROPERTY()
    TArray<FActiveAoE> ActiveAoEs;

    UPROPERTY()
    TMap<FString, FAoEPrototypeData> LoadedAoEPrototypes;

    // Internal Methods
    void UpdateActiveAoEs(float DeltaTime);
    void UpdateAoE(FActiveAoE& AoE, float DeltaTime);
    void UpdateProjectileAoE(FActiveAoE& AoE, float DeltaTime);
    void UpdateExpandingAoE(FActiveAoE& AoE, float DeltaTime);
    void UpdateTravelingAoE(FActiveAoE& AoE, float DeltaTime);
    void UpdatePersistentAoE(FActiveAoE& AoE, float DeltaTime);

    void CheckAoEHits(FActiveAoE& AoE);
    bool IsActorInAoE(const FActiveAoE& AoE, AActor* Actor, FVector& OutHitLocation) const;
    bool IsPointInShape(const FAoEShapeData& Shape, const FVector& Point, const FVector& Center, const FVector& Forward) const;
    
    float CalculateCurrentRadius(const FActiveAoE& AoE) const;
    float CalculateDamageAtLocation(const FActiveAoE& AoE, const FVector& Location) const;
    
    void DrawAoEDebugVisualization(const FActiveAoE& AoE);
    void DrawShapeDebug(const FAoEShapeData& Shape, const FVector& Location, const FVector& Forward, const FLinearColor& Color, float CurrentRadius = -1.0f) const;

    FVector GetAoEOriginLocation(EAoEOrigin Origin, const FVector& CustomLocation = FVector::ZeroVector) const;
    void RemoveCompletedAoEs();
};
