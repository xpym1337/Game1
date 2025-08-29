#include "AoEPrototypeComponent.h"
#include "MyCharacter.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

UAoEPrototypeComponent::UAoEPrototypeComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UAoEPrototypeComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Auto-load data table if specified
    if (bAutoLoadDataTable && AoEDataTable)
    {
        LoadAoEData(AoEDataTable);
    }
    
    if (bDebugEnabled)
    {
        UE_LOG(LogTemp, Log, TEXT("AoEPrototypeComponent initialized for %s"), 
               GetOwner() ? *GetOwner()->GetName() : TEXT("NULL"));
    }
}

void UAoEPrototypeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (ActiveAoEs.Num() > 0)
    {
        UpdateActiveAoEs(DeltaTime);
        RemoveCompletedAoEs();
    }
}

void UAoEPrototypeComponent::StartAoE(const FString& PrototypeName)
{
    if (const FAoEPrototypeData* FoundData = LoadedAoEPrototypes.Find(PrototypeName))
    {
        StartAoEWithData(*FoundData);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("AoE prototype '%s' not found!"), *PrototypeName);
    }
}

void UAoEPrototypeComponent::StartAoEWithData(const FAoEPrototypeData& AoEData)
{
    FActiveAoE NewAoE;
    NewAoE.Name = AoEData.PrototypeName;
    NewAoE.Data = AoEData;
    NewAoE.ElapsedTime = 0.0f;
    NewAoE.Location = GetAoEOriginLocation(AoEData.BehaviorData.Origin);
    NewAoE.ProjectileLocation = NewAoE.Location;
    NewAoE.bIsActive = true;
    NewAoE.bProjectileActive = (AoEData.BehaviorData.Behavior == EAoEBehavior::Projectile);
    
    ActiveAoEs.Add(NewAoE);
    
    if (bDebugEnabled)
    {
        UE_LOG(LogTemp, Log, TEXT("Started AoE: %s at location %s"), 
               *AoEData.PrototypeName, *NewAoE.Location.ToString());
    }
}

void UAoEPrototypeComponent::StartAoEAtLocation(const FString& PrototypeName, const FVector& Location)
{
    if (const FAoEPrototypeData* FoundData = LoadedAoEPrototypes.Find(PrototypeName))
    {
        FActiveAoE NewAoE;
        NewAoE.Name = FoundData->PrototypeName;
        NewAoE.Data = *FoundData;
        NewAoE.ElapsedTime = 0.0f;
        NewAoE.Location = Location;
        NewAoE.ProjectileLocation = Location;
        NewAoE.bIsActive = true;
        NewAoE.bProjectileActive = (FoundData->BehaviorData.Behavior == EAoEBehavior::Projectile);
        
        ActiveAoEs.Add(NewAoE);
        
        if (bDebugEnabled)
        {
            UE_LOG(LogTemp, Log, TEXT("Started AoE: %s at custom location %s"), 
                   *PrototypeName, *Location.ToString());
        }
    }
}

void UAoEPrototypeComponent::StartAoEAtTarget(const FString& PrototypeName, AActor* TargetActor)
{
    if (TargetActor)
    {
        StartAoEAtLocation(PrototypeName, TargetActor->GetActorLocation());
    }
}

void UAoEPrototypeComponent::StopAoE(const FString& PrototypeName)
{
    for (int32 i = ActiveAoEs.Num() - 1; i >= 0; --i)
    {
        if (ActiveAoEs[i].Name == PrototypeName)
        {
            ActiveAoEs.RemoveAt(i);
            if (bDebugEnabled)
            {
                UE_LOG(LogTemp, Log, TEXT("Stopped AoE: %s"), *PrototypeName);
            }
        }
    }
}

void UAoEPrototypeComponent::StopAllAoEs()
{
    int32 StoppedCount = ActiveAoEs.Num();
    ActiveAoEs.Empty();
    
    if (bDebugEnabled && StoppedCount > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("Stopped %d active AoEs"), StoppedCount);
    }
}

TArray<FString> UAoEPrototypeComponent::GetActiveAoEs() const
{
    TArray<FString> ActiveNames;
    for (const FActiveAoE& AoE : ActiveAoEs)
    {
        ActiveNames.Add(AoE.Name);
    }
    return ActiveNames;
}

bool UAoEPrototypeComponent::IsAoEActive(const FString& PrototypeName) const
{
    for (const FActiveAoE& AoE : ActiveAoEs)
    {
        if (AoE.Name == PrototypeName)
        {
            return true;
        }
    }
    return false;
}

void UAoEPrototypeComponent::LoadAoEData(UDataTable* DataTable)
{
    if (!DataTable)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot load null AoE data table"));
        return;
    }
    
    LoadedAoEPrototypes.Empty();
    
    TArray<FAoEPrototypeData*> AllRows;
    DataTable->GetAllRows<FAoEPrototypeData>(TEXT("LoadAoEData"), AllRows);
    
    for (const FAoEPrototypeData* Row : AllRows)
    {
        if (Row)
        {
            LoadedAoEPrototypes.Add(Row->PrototypeName, *Row);
        }
    }
    
    if (bDebugEnabled)
    {
        UE_LOG(LogTemp, Log, TEXT("Loaded %d AoE prototypes"), LoadedAoEPrototypes.Num());
    }
}

FAoEPrototypeData UAoEPrototypeComponent::GetAoEData(const FString& PrototypeName) const
{
    if (const FAoEPrototypeData* FoundData = LoadedAoEPrototypes.Find(PrototypeName))
    {
        return *FoundData;
    }
    return FAoEPrototypeData();
}

TArray<FString> UAoEPrototypeComponent::GetAvailableAoEPrototypes() const
{
    TArray<FString> PrototypeNames;
    LoadedAoEPrototypes.GetKeys(PrototypeNames);
    return PrototypeNames;
}

void UAoEPrototypeComponent::ModifyAoEShapeData(const FString& PrototypeName, const FAoEShapeData& NewShapeData)
{
    if (FAoEPrototypeData* FoundData = LoadedAoEPrototypes.Find(PrototypeName))
    {
        FoundData->ShapeData = NewShapeData;
        
        // Update active AoEs of this type
        for (FActiveAoE& AoE : ActiveAoEs)
        {
            if (AoE.Name == PrototypeName)
            {
                AoE.Data.ShapeData = NewShapeData;
            }
        }
        
        if (bDebugEnabled)
        {
            UE_LOG(LogTemp, Log, TEXT("Modified shape data for AoE: %s"), *PrototypeName);
        }
    }
}

void UAoEPrototypeComponent::ModifyAoEBehaviorData(const FString& PrototypeName, const FAoEBehaviorData& NewBehaviorData)
{
    if (FAoEPrototypeData* FoundData = LoadedAoEPrototypes.Find(PrototypeName))
    {
        FoundData->BehaviorData = NewBehaviorData;
        
        // Update active AoEs of this type
        for (FActiveAoE& AoE : ActiveAoEs)
        {
            if (AoE.Name == PrototypeName)
            {
                AoE.Data.BehaviorData = NewBehaviorData;
            }
        }
        
        if (bDebugEnabled)
        {
            UE_LOG(LogTemp, Log, TEXT("Modified behavior data for AoE: %s"), *PrototypeName);
        }
    }
}

void UAoEPrototypeComponent::SetDebugVisualization(bool bEnabled)
{
    bDebugEnabled = bEnabled;
}

void UAoEPrototypeComponent::TestAoEPrototype(const FString& PrototypeName)
{
    if (LoadedAoEPrototypes.Contains(PrototypeName))
    {
        StartAoE(PrototypeName);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("AoE prototype '%s' not found for testing"), *PrototypeName);
    }
}

void UAoEPrototypeComponent::PreviewAoEShape(const FString& PrototypeName)
{
    if (const FAoEPrototypeData* FoundData = LoadedAoEPrototypes.Find(PrototypeName))
    {
        const FVector Location = GetAoEOriginLocation(FoundData->BehaviorData.Origin);
        const FVector Forward = GetOwner() ? GetOwner()->GetActorForwardVector() : FVector::ForwardVector;
        
        DrawShapeDebug(FoundData->ShapeData, Location, Forward, 
                      FoundData->DebugColor, FoundData->ShapeData.Radius);
        
        if (bDebugEnabled)
        {
            UE_LOG(LogTemp, Log, TEXT("Previewing AoE shape: %s"), *PrototypeName);
        }
    }
}

// Private Methods

void UAoEPrototypeComponent::UpdateActiveAoEs(float DeltaTime)
{
    for (FActiveAoE& AoE : ActiveAoEs)
    {
        if (AoE.bIsActive)
        {
            UpdateAoE(AoE, DeltaTime);
        }
    }
}

void UAoEPrototypeComponent::UpdateAoE(FActiveAoE& AoE, float DeltaTime)
{
    AoE.ElapsedTime += DeltaTime;
    
    // Handle start delay
    if (AoE.ElapsedTime < AoE.Data.BehaviorData.StartDelay)
    {
        if (bDebugEnabled && AoE.Data.bDebugVisualization)
        {
            // Draw delayed AoE preview
            DrawShapeDebug(AoE.Data.ShapeData, AoE.Location, 
                          GetOwner()->GetActorForwardVector(), 
                          FLinearColor::Yellow, AoE.Data.ShapeData.Radius * 0.3f);
        }
        return;
    }
    
    // Update based on behavior type
    switch (AoE.Data.BehaviorData.Behavior)
    {
        case EAoEBehavior::Instant:
            CheckAoEHits(AoE);
            AoE.bIsActive = false; // Complete immediately
            break;
            
        case EAoEBehavior::Expanding:
            UpdateExpandingAoE(AoE, DeltaTime);
            break;
            
        case EAoEBehavior::Projectile:
            UpdateProjectileAoE(AoE, DeltaTime);
            break;
            
        case EAoEBehavior::Delayed:
            // Just wait for duration then trigger
            if (AoE.ElapsedTime - AoE.Data.BehaviorData.StartDelay >= AoE.Data.BehaviorData.Duration)
            {
                CheckAoEHits(AoE);
                AoE.bIsActive = false;
            }
            break;
            
        case EAoEBehavior::Persistent:
            UpdatePersistentAoE(AoE, DeltaTime);
            break;
            
        case EAoEBehavior::Traveling:
            UpdateTravelingAoE(AoE, DeltaTime);
            break;
    }
    
    // Draw debug visualization
    if (bDebugEnabled && AoE.Data.bDebugVisualization)
    {
        DrawAoEDebugVisualization(AoE);
    }
    
    // Check for lifetime expiration
    if (AoE.ElapsedTime >= AoE.Data.BehaviorData.Lifetime)
    {
        AoE.bIsActive = false;
        OnAoEComplete.Broadcast(AoE.Name);
    }
}

void UAoEPrototypeComponent::UpdateProjectileAoE(FActiveAoE& AoE, float DeltaTime)
{
    if (AoE.bProjectileActive)
    {
        // Move projectile towards target
        const FVector StartLocation = GetAoEOriginLocation(AoE.Data.BehaviorData.Origin);
        const FVector TargetLocation = AoE.Location;
        const FVector Direction = (TargetLocation - StartLocation).GetSafeNormal();
        
        float TravelDistance = AoE.Data.BehaviorData.ProjectileSpeed * DeltaTime;
        AoE.ProjectileLocation += Direction * TravelDistance;
        
        // Check if projectile reached target or max range
        float DistanceToTarget = FVector::Dist(AoE.ProjectileLocation, TargetLocation);
        float DistanceTraveled = FVector::Dist(AoE.ProjectileLocation, StartLocation);
        
        if (DistanceToTarget <= 50.0f || DistanceTraveled >= AoE.Data.BehaviorData.ProjectileRange)
        {
            // Projectile impact - trigger AoE
            AoE.Location = AoE.ProjectileLocation;
            AoE.bProjectileActive = false;
            CheckAoEHits(AoE);
            
            if (AoE.Data.BehaviorData.Behavior != EAoEBehavior::Persistent)
            {
                AoE.bIsActive = false; // Complete unless it's persistent after impact
            }
        }
        
        // Draw projectile
        if (bDebugEnabled && AoE.Data.bDebugVisualization)
        {
            DrawDebugSphere(GetWorld(), AoE.ProjectileLocation, 20.0f, 8, 
                           AoE.Data.DebugColor.ToFColor(false), false, -1.0f);
            DrawDebugLine(GetWorld(), StartLocation, AoE.ProjectileLocation, 
                         AoE.Data.DebugColor.ToFColor(false), false, -1.0f, 0, 2.0f);
        }
    }
    else if (!AoE.bProjectileActive)
    {
        // Projectile has impacted, now do AoE behavior
        CheckAoEHits(AoE);
    }
}

void UAoEPrototypeComponent::UpdateExpandingAoE(FActiveAoE& AoE, float DeltaTime)
{
    // Calculate current radius based on expansion
    float CurrentRadius = CalculateCurrentRadius(AoE);
    
    // Check hits continuously as it expands
    CheckAoEHits(AoE);
    
    // Complete when fully expanded
    if (CurrentRadius >= AoE.Data.ShapeData.Radius)
    {
        AoE.bIsActive = false;
    }
}

void UAoEPrototypeComponent::UpdateTravelingAoE(FActiveAoE& AoE, float DeltaTime)
{
    // Move the AoE location in travel direction
    FVector TravelDirection = AoE.Data.BehaviorData.TravelDirection.GetSafeNormal();
    if (TravelDirection.IsZero())
    {
        TravelDirection = GetOwner()->GetActorForwardVector();
    }
    
    float TravelDistance = AoE.Data.BehaviorData.TravelSpeed * DeltaTime;
    AoE.Location += TravelDirection * TravelDistance;
    
    // Check hits at current location
    CheckAoEHits(AoE);
}

void UAoEPrototypeComponent::UpdatePersistentAoE(FActiveAoE& AoE, float DeltaTime)
{
    // Check hits based on hit interval
    static float LastHitTime = 0.0f;
    if (AoE.ElapsedTime - LastHitTime >= AoE.Data.DamageData.HitInterval)
    {
        CheckAoEHits(AoE);
        LastHitTime = AoE.ElapsedTime;
    }
}

void UAoEPrototypeComponent::CheckAoEHits(FActiveAoE& AoE)
{
    if (!GetWorld())
        return;
    
    // Get all pawns in range
    TArray<FHitResult> HitResults;
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
    
    TArray<AActor*> ActorsToIgnore;
    ActorsToIgnore.Add(GetOwner());
    
    // Use sphere trace as base check, then refine with shape
    bool bHit = UKismetSystemLibrary::SphereTraceMultiForObjects(
        GetWorld(),
        AoE.Location,
        AoE.Location,
        AoE.Data.ShapeData.Radius,
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
            AActor* HitActor = Hit.GetActor();
            if (!HitActor)
                continue;
            
            // Check if actor is actually within the AoE shape
            FVector HitLocation;
            if (IsActorInAoE(AoE, HitActor, HitLocation))
            {
                // Check hit count limits
                int32* HitCount = AoE.ActorHitCounts.Find(HitActor);
                int32 CurrentHitCount = HitCount ? *HitCount : 0;
                
                if (CurrentHitCount < AoE.Data.DamageData.MaxHitsPerActor || AoE.Data.DamageData.MaxHitsPerActor <= 0)
                {
                    // Check hit interval for multi-hit
                    if (AoE.Data.DamageData.bAllowMultipleHits)
                    {
                        float* LastHitTime = AoE.LastHitTimes.Find(HitActor);
                        float CurrentTime = GetWorld()->GetTimeSeconds();
                        
                        if (!LastHitTime || CurrentTime - *LastHitTime >= AoE.Data.DamageData.HitInterval)
                        {
                            // Apply damage
                            float DamageAmount = CalculateDamageAtLocation(AoE, HitLocation);
                            
                            // Update hit tracking
                            AoE.ActorHitCounts.Add(HitActor, CurrentHitCount + 1);
                            AoE.LastHitTimes.Add(HitActor, CurrentTime);
                            
                            // Broadcast hit event
                            OnAoEHit.Broadcast(HitActor, HitLocation, DamageAmount);
                            
                            if (bDebugEnabled)
                            {
                                UE_LOG(LogTemp, Log, TEXT("AoE '%s' hit %s for %.1f damage"), 
                                       *AoE.Name, *HitActor->GetName(), DamageAmount);
                            }
                        }
                    }
                    else if (!AoE.HitActors.Contains(HitActor))
                    {
                        // Single hit per actor
                        float DamageAmount = CalculateDamageAtLocation(AoE, HitLocation);
                        
                        AoE.HitActors.Add(HitActor);
                        AoE.ActorHitCounts.Add(HitActor, 1);
                        
                        // Broadcast hit event
                        OnAoEHit.Broadcast(HitActor, HitLocation, DamageAmount);
                        
                        if (bDebugEnabled)
                        {
                            UE_LOG(LogTemp, Log, TEXT("AoE '%s' hit %s for %.1f damage"), 
                                   *AoE.Name, *HitActor->GetName(), DamageAmount);
                        }
                    }
                }
            }
        }
    }
}

bool UAoEPrototypeComponent::IsActorInAoE(const FActiveAoE& AoE, AActor* Actor, FVector& OutHitLocation) const
{
    if (!Actor)
        return false;
    
    FVector ActorLocation = Actor->GetActorLocation();
    FVector Forward = GetOwner() ? GetOwner()->GetActorForwardVector() : FVector::ForwardVector;
    
    OutHitLocation = ActorLocation;
    
    return IsPointInShape(AoE.Data.ShapeData, ActorLocation, AoE.Location, Forward);
}

bool UAoEPrototypeComponent::IsPointInShape(const FAoEShapeData& Shape, const FVector& Point, const FVector& Center, const FVector& Forward) const
{
    FVector LocalPoint = Point - Center;
    float Distance2D = FVector2D(LocalPoint.X, LocalPoint.Y).Size();
    
    switch (Shape.Shape)
    {
        case EAoEShape::Circle:
        {
            return Distance2D <= Shape.Radius;
        }
        
        case EAoEShape::Ring:
        {
            return Distance2D >= Shape.InnerRadius && Distance2D <= Shape.Radius;
        }
        
        case EAoEShape::Cone:
        {
            if (Distance2D > Shape.Radius)
                return false;
                
            FVector ToPoint = LocalPoint.GetSafeNormal();
            float Dot = FVector::DotProduct(Forward, ToPoint);
            float AngleDegrees = FMath::RadiansToDegrees(FMath::Acos(Dot));
            
            return AngleDegrees <= Shape.Angle * 0.5f;
        }
        
        case EAoEShape::Rectangle:
        {
            FVector Right = FVector::CrossProduct(Forward, FVector::UpVector);
            
            float ForwardDist = FVector::DotProduct(LocalPoint, Forward);
            float RightDist = FVector::DotProduct(LocalPoint, Right);
            
            return FMath::Abs(ForwardDist) <= Shape.Dimensions.X * 0.5f &&
                   FMath::Abs(RightDist) <= Shape.Dimensions.Y * 0.5f;
        }
        
        case EAoEShape::Line:
        {
            FVector Right = FVector::CrossProduct(Forward, FVector::UpVector);
            
            float ForwardDist = FVector::DotProduct(LocalPoint, Forward);
            float RightDist = FVector::DotProduct(LocalPoint, Right);
            
            return ForwardDist >= 0 && ForwardDist <= Shape.Length &&
                   FMath::Abs(RightDist) <= Shape.Width * 0.5f;
        }
        
        case EAoEShape::Sector:
        {
            if (Distance2D < Shape.InnerRadius || Distance2D > Shape.Radius)
                return false;
                
            FVector ToPoint = LocalPoint.GetSafeNormal();
            float Dot = FVector::DotProduct(Forward, ToPoint);
            float AngleDegrees = FMath::RadiansToDegrees(FMath::Acos(Dot));
            
            return AngleDegrees <= Shape.Angle * 0.5f;
        }
        
        default:
            return Distance2D <= Shape.Radius;
    }
}

float UAoEPrototypeComponent::CalculateCurrentRadius(const FActiveAoE& AoE) const
{
    if (AoE.Data.BehaviorData.Behavior != EAoEBehavior::Expanding)
    {
        return AoE.Data.ShapeData.Radius;
    }
    
    float ElapsedSinceStart = AoE.ElapsedTime - AoE.Data.BehaviorData.StartDelay;
    float ExpansionAlpha = 0.0f;
    
    if (AoE.Data.BehaviorData.ExpansionCurve)
    {
        ExpansionAlpha = AoE.Data.BehaviorData.ExpansionCurve->GetFloatValue(ElapsedSinceStart / AoE.Data.BehaviorData.Duration);
    }
    else
    {
        ExpansionAlpha = ElapsedSinceStart * AoE.Data.BehaviorData.ExpansionSpeed / AoE.Data.ShapeData.Radius;
    }
    
    return FMath::Clamp(ExpansionAlpha, 0.0f, 1.0f) * AoE.Data.ShapeData.Radius;
}

float UAoEPrototypeComponent::CalculateDamageAtLocation(const FActiveAoE& AoE, const FVector& Location) const
{
    float BaseDamage = AoE.Data.DamageData.Damage;
    
    if (!AoE.Data.DamageData.bUseDamageFalloff)
    {
        return BaseDamage;
    }
    
    // Calculate distance falloff
    float Distance = FVector::Dist(Location, AoE.Location);
    float FalloffAlpha = 1.0f - FMath::Clamp(Distance / AoE.Data.ShapeData.Radius, 0.0f, 1.0f);
    
    if (AoE.Data.DamageData.DamageFalloffCurve)
    {
        FalloffAlpha = AoE.Data.DamageData.DamageFalloffCurve->GetFloatValue(FalloffAlpha);
    }
    
    return BaseDamage * FalloffAlpha;
}

void UAoEPrototypeComponent::DrawAoEDebugVisualization(const FActiveAoE& AoE)
{
    if (!GetWorld())
        return;
    
    FVector Forward = GetOwner() ? GetOwner()->GetActorForwardVector() : FVector::ForwardVector;
    float CurrentRadius = CalculateCurrentRadius(AoE);
    
    DrawShapeDebug(AoE.Data.ShapeData, AoE.Location, Forward, AoE.Data.DebugColor, CurrentRadius);
    
    // Draw center point
    DrawDebugSphere(GetWorld(), AoE.Location, 15.0f, 8, 
                   AoE.Data.DebugColor.ToFColor(false), false, -1.0f);
}

void UAoEPrototypeComponent::DrawShapeDebug(const FAoEShapeData& Shape, const FVector& Location, const FVector& Forward, const FLinearColor& Color, float CurrentRadius) const
{
    if (!GetWorld())
        return;
    
    float Radius = CurrentRadius >= 0 ? CurrentRadius : Shape.Radius;
    FColor DebugColor = Color.ToFColor(false);
    
    switch (Shape.Shape)
    {
        case EAoEShape::Circle:
            DrawDebugSphere(GetWorld(), Location, Radius, 16, DebugColor, false, -1.0f, 0, 3.0f);
            break;
            
        case EAoEShape::Ring:
            DrawDebugSphere(GetWorld(), Location, Radius, 16, DebugColor, false, -1.0f, 0, 3.0f);
            DrawDebugSphere(GetWorld(), Location, Shape.InnerRadius, 16, FColor::Black, false, -1.0f, 0, 2.0f);
            break;
            
        case EAoEShape::Cone:
        {
            FVector Right = FVector::CrossProduct(Forward, FVector::UpVector);
            float HalfAngle = FMath::DegreesToRadians(Shape.Angle * 0.5f);
            
            FVector LeftEdge = (Forward * FMath::Cos(HalfAngle) + Right * FMath::Sin(HalfAngle)) * Radius;
            FVector RightEdge = (Forward * FMath::Cos(HalfAngle) - Right * FMath::Sin(HalfAngle)) * Radius;
            
            DrawDebugLine(GetWorld(), Location, Location + LeftEdge, DebugColor, false, -1.0f, 0, 3.0f);
            DrawDebugLine(GetWorld(), Location, Location + RightEdge, DebugColor, false, -1.0f, 0, 3.0f);
            
            // Arc
            int32 Segments = 20;
            for (int32 i = 0; i <= Segments; ++i)
            {
                float CurrentAngle = FMath::Lerp(-HalfAngle, HalfAngle, static_cast<float>(i) / static_cast<float>(Segments));
                FVector CurrentEdge = (Forward * FMath::Cos(CurrentAngle) + Right * FMath::Sin(CurrentAngle)) * Radius;
                
                if (i > 0)
                {
                    float PrevAngle = FMath::Lerp(-HalfAngle, HalfAngle, static_cast<float>(i - 1) / static_cast<float>(Segments));
                    FVector PrevEdge = (Forward * FMath::Cos(PrevAngle) + Right * FMath::Sin(PrevAngle)) * Radius;
                    DrawDebugLine(GetWorld(), Location + PrevEdge, Location + CurrentEdge, DebugColor, false, -1.0f, 0, 2.0f);
                }
            }
            break;
        }
        
        case EAoEShape::Rectangle:
        {
            FVector Right = FVector::CrossProduct(Forward, FVector::UpVector);
            FVector HalfExtentX = Forward * Shape.Dimensions.X * 0.5f;
            FVector HalfExtentY = Right * Shape.Dimensions.Y * 0.5f;
            
            // Draw rectangle corners
            FVector Corners[4] = {
                Location - HalfExtentX - HalfExtentY,
                Location - HalfExtentX + HalfExtentY,
                Location + HalfExtentX + HalfExtentY,
                Location + HalfExtentX - HalfExtentY
            };
            
            for (int32 i = 0; i < 4; ++i)
            {
                int32 NextIndex = (i + 1) % 4;
                DrawDebugLine(GetWorld(), Corners[i], Corners[NextIndex], DebugColor, false, -1.0f, 0, 3.0f);
            }
            break;
        }
        
        case EAoEShape::Line:
        {
            FVector Right = FVector::CrossProduct(Forward, FVector::UpVector);
            FVector StartPoint = Location;
            FVector EndPoint = Location + Forward * Shape.Length;
            FVector Width = Right * Shape.Width * 0.5f;
            
            // Draw line with width
            DrawDebugLine(GetWorld(), StartPoint - Width, StartPoint + Width, DebugColor, false, -1.0f, 0, 3.0f);
            DrawDebugLine(GetWorld(), EndPoint - Width, EndPoint + Width, DebugColor, false, -1.0f, 0, 3.0f);
            DrawDebugLine(GetWorld(), StartPoint - Width, EndPoint - Width, DebugColor, false, -1.0f, 0, 3.0f);
            DrawDebugLine(GetWorld(), StartPoint + Width, EndPoint + Width, DebugColor, false, -1.0f, 0, 3.0f);
            break;
        }
        
        case EAoEShape::Sector:
        {
            // Draw both inner and outer arcs
            DrawDebugSphere(GetWorld(), Location, Radius, 16, DebugColor, false, -1.0f, 0, 3.0f);
            if (Shape.InnerRadius > 0)
            {
                DrawDebugSphere(GetWorld(), Location, Shape.InnerRadius, 16, FColor::Black, false, -1.0f, 0, 2.0f);
            }
            
            // Draw sector edges
            FVector Right = FVector::CrossProduct(Forward, FVector::UpVector);
            float HalfAngle = FMath::DegreesToRadians(Shape.Angle * 0.5f);
            
            FVector LeftEdge = (Forward * FMath::Cos(HalfAngle) + Right * FMath::Sin(HalfAngle)) * Radius;
            FVector RightEdge = (Forward * FMath::Cos(HalfAngle) - Right * FMath::Sin(HalfAngle)) * Radius;
            
            DrawDebugLine(GetWorld(), Location, Location + LeftEdge, DebugColor, false, -1.0f, 0, 3.0f);
            DrawDebugLine(GetWorld(), Location, Location + RightEdge, DebugColor, false, -1.0f, 0, 3.0f);
            break;
        }
        
        default:
            DrawDebugSphere(GetWorld(), Location, Radius, 16, DebugColor, false, -1.0f, 0, 3.0f);
            break;
    }
}

FVector UAoEPrototypeComponent::GetAoEOriginLocation(EAoEOrigin Origin, const FVector& CustomLocation) const
{
    if (!GetOwner())
        return FVector::ZeroVector;
    
    switch (Origin)
    {
        case EAoEOrigin::Caster:
            return GetOwner()->GetActorLocation();
            
        case EAoEOrigin::Target:
            // For now, use forward direction from caster
            return GetOwner()->GetActorLocation() + GetOwner()->GetActorForwardVector() * 500.0f;
            
        case EAoEOrigin::AboveCaster:
            return GetOwner()->GetActorLocation() + FVector(0, 0, 300.0f);
            
        case EAoEOrigin::AboveTarget:
        {
            FVector TargetLocation = GetOwner()->GetActorLocation() + GetOwner()->GetActorForwardVector() * 500.0f;
            return TargetLocation + FVector(0, 0, 300.0f);
        }
        
        case EAoEOrigin::Custom:
            return CustomLocation.IsZero() ? GetOwner()->GetActorLocation() : CustomLocation;
            
        default:
            return GetOwner()->GetActorLocation();
    }
}

void UAoEPrototypeComponent::RemoveCompletedAoEs()
{
    for (int32 i = ActiveAoEs.Num() - 1; i >= 0; --i)
    {
        if (!ActiveAoEs[i].bIsActive)
        {
            if (bDebugEnabled)
            {
                UE_LOG(LogTemp, Log, TEXT("Removing completed AoE: %s"), *ActiveAoEs[i].Name);
            }
            ActiveAoEs.RemoveAt(i);
        }
    }
}
