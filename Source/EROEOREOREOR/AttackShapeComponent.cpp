#include "AttackShapeComponent.h"
#include "GameplayEffect_Damage.h"
#include "MyCharacter.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"
#include "CollisionQueryParams.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectTypes.h"

UAttackShapeComponent::UAttackShapeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false; // Enable only when needed
	
	bShowDebugShapes = true;
	bShowHitResults = true;
	HitResultDisplayTime = 2.0f;
}

void UAttackShapeComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UAttackShapeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (bAttackActive)
	{
		UpdateAttack(DeltaTime);
	}
	
	// Handle preview visualization
	if (bShowingPreview)
	{
		const float CurrentTime = GetWorld()->GetTimeSeconds();
		if (CurrentTime >= PreviewEndTime)
		{
			ClearPreview();
		}
		else
		{
			// Draw preview shape
			DrawShapeDebug(PreviewShapeData, FColor::Yellow, -1.0f);
		}
	}
}

void UAttackShapeComponent::StartAttack(const FAttackPrototypeData& AttackData)
{
	// Stop any current attack
	StopAttack();
	
	// Initialize attack state
	bAttackActive = true;
	CurrentAttackData = AttackData;
	AttackStartTime = GetWorld()->GetTimeSeconds();
	CurrentFrame = 0;
	AlreadyHitActors.Empty();
	ActorHitCounts.Empty();
	LastHitTimes.Empty();
	
	// Enable ticking for attack processing
	SetComponentTickEnabled(true);
	
	UE_LOG(LogTemp, Log, TEXT("AttackShapeComponent: Started attack '%s' with %d shapes"), 
		*AttackData.AttackName, AttackData.AttackShapes.Num());
}

void UAttackShapeComponent::StopAttack()
{
	if (bAttackActive)
	{
		bAttackActive = false;
		AlreadyHitActors.Empty();
		ActorHitCounts.Empty();
		LastHitTimes.Empty();
		
		// Disable ticking when not needed for performance
		SetComponentTickEnabled(false);
		
		UE_LOG(LogTemp, Log, TEXT("AttackShapeComponent: Stopped attack"));
	}
}

void UAttackShapeComponent::PreviewAttackShape(const FAttackShapeData& ShapeData, float PreviewTime)
{
	bShowingPreview = true;
	PreviewShapeData = ShapeData;
	PreviewEndTime = GetWorld()->GetTimeSeconds() + PreviewTime;
	
	// Enable ticking for preview
	SetComponentTickEnabled(true);
	
	UE_LOG(LogTemp, Log, TEXT("AttackShapeComponent: Previewing %s shape for %.1f seconds"), 
		*UEnum::GetValueAsString(ShapeData.ShapeType), PreviewTime);
}

void UAttackShapeComponent::ClearPreview()
{
	bShowingPreview = false;
	
	// Disable ticking if no attack is active
	if (!bAttackActive)
	{
		SetComponentTickEnabled(false);
	}
}

void UAttackShapeComponent::TestShapeAtLocation(const FAttackShapeData& ShapeData, const FVector& WorldLocation)
{
	if (!GetWorld())
		return;
		
	// Create a temporary shape data with the specified location
	FAttackShapeData TestShape = ShapeData;
	TestShape.LocalOffset = GetOwner()->GetTransform().InverseTransformPosition(WorldLocation);
	
	// Perform collision check
	TArray<FHitResult> HitResults;
	const bool bHit = CheckShapeCollision(TestShape, HitResults);
	
	// Draw the shape and results
	const FColor ShapeColor = bHit ? FColor::Red : FColor::Green;
	DrawShapeDebug(TestShape, ShapeColor, 3.0f);
	
	// Log results
	UE_LOG(LogTemp, Log, TEXT("AttackShapeComponent: Shape test at %s - Hit %d actors"), 
		*WorldLocation.ToString(), HitResults.Num());
		
	for (const FHitResult& Hit : HitResults)
	{
		if (Hit.GetActor())
		{
			UE_LOG(LogTemp, Log, TEXT("  - Hit: %s"), *Hit.GetActor()->GetName());
			
			// Draw hit location
			if (bShowHitResults)
			{
				DrawDebugSphere(GetWorld(), Hit.Location, 10.0f, 8, FColor::Orange, false, 3.0f);
			}
		}
	}
}

void UAttackShapeComponent::UpdateAttack(float DeltaTime)
{
	if (!bAttackActive || !GetWorld())
		return;
		
	// Update frame counter (60 FPS timing)
	const float ElapsedTime = GetWorld()->GetTimeSeconds() - AttackStartTime;
	const int32 NewFrame = FMath::FloorToInt(ElapsedTime * CombatConstants::TARGET_FRAMERATE);
	
	if (NewFrame != CurrentFrame)
	{
		CurrentFrame = NewFrame;
		ProcessActiveShapes();
	}
}

void UAttackShapeComponent::ProcessActiveShapes()
{
	if (CurrentAttackData.AttackShapes.Num() == 0)
		return;
		
	// Check each shape that should be active this frame
	for (const FAttackShapeData& ShapeData : CurrentAttackData.AttackShapes)
	{
		if (IsShapeActiveThisFrame(ShapeData))
		{
			// Draw debug visualization
			if (bShowDebugShapes && ShapeData.bShowDebugShape)
			{
				DrawShapeDebug(ShapeData, ShapeData.DebugColor, -1.0f);
			}
			
			// Perform collision detection
			TArray<FHitResult> HitResults;
			if (CheckShapeCollision(ShapeData, HitResults))
			{
				// Process hits
				for (const FHitResult& Hit : HitResults)
				{
					if (Hit.GetActor() && CanHitActor(Hit.GetActor(), ShapeData))
					{
						HandleActorHit(Hit.GetActor(), Hit.Location, ShapeData);
					}
				}
			}
		}
	}
}

bool UAttackShapeComponent::CheckShapeCollision(const FAttackShapeData& ShapeData, TArray<FHitResult>& OutHits)
{
	OutHits.Empty();
	
	switch (ShapeData.ShapeType)
	{
		case EAttackShape::Sphere:
			return CheckSphereCollision(ShapeData, OutHits);
		case EAttackShape::Capsule:
			return CheckCapsuleCollision(ShapeData, OutHits);
		case EAttackShape::Box:
			return CheckBoxCollision(ShapeData, OutHits);
		case EAttackShape::Cone:
			return CheckConeCollision(ShapeData, OutHits);
		case EAttackShape::Line:
			return CheckLineCollision(ShapeData, OutHits);
		case EAttackShape::Ring:
			return CheckRingCollision(ShapeData, OutHits);
		case EAttackShape::Arc:
			return CheckArcCollision(ShapeData, OutHits);
		default:
			UE_LOG(LogTemp, Warning, TEXT("AttackShapeComponent: Unknown shape type"));
			return false;
	}
}

void UAttackShapeComponent::DrawShapeDebug(const FAttackShapeData& ShapeData, const FColor& Color, float Duration)
{
	if (!bShowDebugShapes || !GetWorld())
		return;
		
	const float DrawDuration = Duration > 0.0f ? Duration : ShapeData.DebugDrawTime;
	
	switch (ShapeData.ShapeType)
	{
		case EAttackShape::Sphere:
			DrawSphere(ShapeData, Color, DrawDuration);
			break;
		case EAttackShape::Capsule:
			DrawCapsule(ShapeData, Color, DrawDuration);
			break;
		case EAttackShape::Box:
			DrawBox(ShapeData, Color, DrawDuration);
			break;
		case EAttackShape::Cone:
			DrawCone(ShapeData, Color, DrawDuration);
			break;
		case EAttackShape::Line:
			DrawLine(ShapeData, Color, DrawDuration);
			break;
		case EAttackShape::Ring:
			DrawRing(ShapeData, Color, DrawDuration);
			break;
		case EAttackShape::Arc:
			DrawArc(ShapeData, Color, DrawDuration);
			break;
	}
}

// Shape-specific collision implementations
bool UAttackShapeComponent::CheckSphereCollision(const FAttackShapeData& ShapeData, TArray<FHitResult>& OutHits)
{
	const FVector WorldPos = GetWorldPositionFromShape(ShapeData);
	
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
	
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(GetOwner());
	
	TArray<AActor*> OverlapActors;
	const bool bHit = UKismetSystemLibrary::SphereOverlapActors(
		GetWorld(),
		WorldPos,
		ShapeData.PrimarySize,
		ObjectTypes,
		nullptr,
		ActorsToIgnore,
		OverlapActors
	);
	
	// Convert AActor array to FHitResult array
	for (AActor* Actor : OverlapActors)
	{
		if (Actor)
		{
			FHitResult HitResult;
			HitResult.HitObjectHandle = FActorInstanceHandle(Actor);
			HitResult.Location = Actor->GetActorLocation();
			HitResult.ImpactPoint = Actor->GetActorLocation();
			HitResult.Component = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
			OutHits.Add(HitResult);
		}
	}
	
	return bHit;
}

bool UAttackShapeComponent::CheckCapsuleCollision(const FAttackShapeData& ShapeData, TArray<FHitResult>& OutHits)
{
	const FVector WorldPos = GetWorldPositionFromShape(ShapeData);
	const FRotator WorldRot = GetWorldRotationFromShape(ShapeData);
	
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
	
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(GetOwner());
	
	TArray<AActor*> OverlapActors;
	const bool bHit = UKismetSystemLibrary::CapsuleOverlapActors(
		GetWorld(),
		WorldPos,
		ShapeData.PrimarySize, // Radius
		ShapeData.SecondarySize, // Half-height
		ObjectTypes,
		nullptr,
		ActorsToIgnore,
		OverlapActors
	);
	
	// Convert AActor array to FHitResult array
	for (AActor* Actor : OverlapActors)
	{
		if (Actor)
		{
			FHitResult HitResult;
			HitResult.HitObjectHandle = FActorInstanceHandle(Actor);
			HitResult.Location = Actor->GetActorLocation();
			HitResult.ImpactPoint = Actor->GetActorLocation();
			HitResult.Component = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
			OutHits.Add(HitResult);
		}
	}
	
	return bHit;
}

bool UAttackShapeComponent::CheckBoxCollision(const FAttackShapeData& ShapeData, TArray<FHitResult>& OutHits)
{
	const FVector WorldPos = GetWorldPositionFromShape(ShapeData);
	const FRotator WorldRot = GetWorldRotationFromShape(ShapeData);
	const FVector BoxExtent(ShapeData.PrimarySize, ShapeData.SecondarySize, ShapeData.TertiarySize);
	
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
	
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(GetOwner());
	
	TArray<AActor*> OverlapActors;
	const bool bHit = UKismetSystemLibrary::BoxOverlapActors(
		GetWorld(),
		WorldPos,
		BoxExtent,
		ObjectTypes,
		nullptr,
		ActorsToIgnore,
		OverlapActors
	);
	
	// Convert AActor array to FHitResult array
	for (AActor* Actor : OverlapActors)
	{
		if (Actor)
		{
			FHitResult HitResult;
			HitResult.HitObjectHandle = FActorInstanceHandle(Actor);
			HitResult.Location = Actor->GetActorLocation();
			HitResult.ImpactPoint = Actor->GetActorLocation();
			HitResult.Component = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
			OutHits.Add(HitResult);
		}
	}
	
	return bHit;
}

bool UAttackShapeComponent::CheckConeCollision(const FAttackShapeData& ShapeData, TArray<FHitResult>& OutHits)
{
	// Cone collision using sphere overlap + angle check
	const FVector WorldPos = GetWorldPositionFromShape(ShapeData);
	const FRotator WorldRot = GetWorldRotationFromShape(ShapeData);
	const FVector ForwardVector = WorldRot.Vector();
	
	TArray<FHitResult> SphereHits;
	if (!CheckSphereCollision(ShapeData, SphereHits))
		return false;
	
	// Filter hits by cone angle
	const float ConeRadians = FMath::DegreesToRadians(ShapeData.ConeAngle * 0.5f);
	
	for (const FHitResult& Hit : SphereHits)
	{
		if (Hit.GetActor())
		{
			const FVector ToTarget = (Hit.GetActor()->GetActorLocation() - WorldPos).GetSafeNormal();
			const float DotProduct = FVector::DotProduct(ForwardVector, ToTarget);
			const float Angle = FMath::Acos(DotProduct);
			
			if (Angle <= ConeRadians)
			{
				OutHits.Add(Hit);
			}
		}
	}
	
	return OutHits.Num() > 0;
}

bool UAttackShapeComponent::CheckLineCollision(const FAttackShapeData& ShapeData, TArray<FHitResult>& OutHits)
{
	const FVector StartPos = GetWorldPositionFromShape(ShapeData);
	const FRotator WorldRot = GetWorldRotationFromShape(ShapeData);
	const FVector EndPos = StartPos + WorldRot.Vector() * ShapeData.PrimarySize;
	
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
	
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(GetOwner());
	
	FHitResult HitResult;
	const bool bHit = UKismetSystemLibrary::LineTraceSingleForObjects(
		GetWorld(),
		StartPos,
		EndPos,
		ObjectTypes,
		false,
		ActorsToIgnore,
		EDrawDebugTrace::None,
		HitResult,
		true
	);
	
	if (bHit)
	{
		OutHits.Add(HitResult);
	}
	
	return bHit;
}

bool UAttackShapeComponent::CheckRingCollision(const FAttackShapeData& ShapeData, TArray<FHitResult>& OutHits)
{
	const FVector WorldPos = GetWorldPositionFromShape(ShapeData);
	
	// Get all actors in outer radius
	TArray<FHitResult> OuterHits;
	FAttackShapeData OuterSphere = ShapeData;
	OuterSphere.ShapeType = EAttackShape::Sphere;
	OuterSphere.PrimarySize = ShapeData.OuterRadius;
	
	if (!CheckSphereCollision(OuterSphere, OuterHits))
		return false;
	
	// Filter out actors within inner radius
	for (const FHitResult& Hit : OuterHits)
	{
		if (Hit.GetActor())
		{
			const float Distance = FVector::Dist(WorldPos, Hit.GetActor()->GetActorLocation());
			if (Distance >= ShapeData.InnerRadius)
			{
				OutHits.Add(Hit);
			}
		}
	}
	
	return OutHits.Num() > 0;
}

bool UAttackShapeComponent::CheckArcCollision(const FAttackShapeData& ShapeData, TArray<FHitResult>& OutHits)
{
	// Arc is essentially a cone with ring distance constraints
	const FVector WorldPos = GetWorldPositionFromShape(ShapeData);
	const FRotator WorldRot = GetWorldRotationFromShape(ShapeData);
	const FVector ForwardVector = WorldRot.Vector();
	
	// First check cone collision
	TArray<FHitResult> ConeHits;
	if (!CheckConeCollision(ShapeData, ConeHits))
		return false;
	
	// Then filter by ring distance
	for (const FHitResult& Hit : ConeHits)
	{
		if (Hit.GetActor())
		{
			const float Distance = FVector::Dist(WorldPos, Hit.GetActor()->GetActorLocation());
			if (Distance >= ShapeData.InnerRadius && Distance <= ShapeData.OuterRadius)
			{
				OutHits.Add(Hit);
			}
		}
	}
	
	return OutHits.Num() > 0;
}

// Shape-specific debug drawing implementations
void UAttackShapeComponent::DrawSphere(const FAttackShapeData& ShapeData, const FColor& Color, float Duration)
{
	const FVector WorldPos = GetWorldPositionFromShape(ShapeData);
	DrawDebugSphere(GetWorld(), WorldPos, ShapeData.PrimarySize, CombatConstants::DEBUG_SPHERE_SEGMENTS, 
		Color, false, Duration, 0, ShapeData.DebugLineThickness);
}

void UAttackShapeComponent::DrawCapsule(const FAttackShapeData& ShapeData, const FColor& Color, float Duration)
{
	const FVector WorldPos = GetWorldPositionFromShape(ShapeData);
	const FRotator WorldRot = GetWorldRotationFromShape(ShapeData);
	DrawDebugCapsule(GetWorld(), WorldPos, ShapeData.SecondarySize, ShapeData.PrimarySize, 
		WorldRot.Quaternion(), Color, false, Duration, 0, ShapeData.DebugLineThickness);
}

void UAttackShapeComponent::DrawBox(const FAttackShapeData& ShapeData, const FColor& Color, float Duration)
{
	const FVector WorldPos = GetWorldPositionFromShape(ShapeData);
	const FRotator WorldRot = GetWorldRotationFromShape(ShapeData);
	const FVector BoxExtent(ShapeData.PrimarySize, ShapeData.SecondarySize, ShapeData.TertiarySize);
	DrawDebugBox(GetWorld(), WorldPos, BoxExtent, WorldRot.Quaternion(), 
		Color, false, Duration, 0, ShapeData.DebugLineThickness);
}

void UAttackShapeComponent::DrawCone(const FAttackShapeData& ShapeData, const FColor& Color, float Duration)
{
	const FVector WorldPos = GetWorldPositionFromShape(ShapeData);
	const FRotator WorldRot = GetWorldRotationFromShape(ShapeData);
	const FVector ForwardVector = WorldRot.Vector();
	const float ConeRadians = FMath::DegreesToRadians(ShapeData.ConeAngle * 0.5f);
	
	// Draw cone using lines
	const int32 NumSegments = CombatConstants::DEBUG_CONE_SEGMENTS;
	const FVector ConeEnd = WorldPos + ForwardVector * ShapeData.ConeRange;
	
	// Draw center line
	DrawDebugLine(GetWorld(), WorldPos, ConeEnd, Color, false, Duration, 0, ShapeData.DebugLineThickness);
	
	// Draw cone edges
	for (int32 i = 0; i < NumSegments; ++i)
	{
		const float Angle = (2.0f * PI * i) / NumSegments;
		const FVector Right = WorldRot.Vector().RotateAngleAxis(90.0f, FVector::UpVector);
		const FVector Up = FVector::CrossProduct(ForwardVector, Right);
		
		const FVector ConeDirection = ForwardVector.RotateAngleAxis(FMath::RadiansToDegrees(ConeRadians), 
			Right * FMath::Cos(Angle) + Up * FMath::Sin(Angle));
		const FVector ConePoint = WorldPos + ConeDirection * ShapeData.ConeRange;
		
		DrawDebugLine(GetWorld(), WorldPos, ConePoint, Color, false, Duration, 0, ShapeData.DebugLineThickness);
	}
	
	// Draw end circle
	const float EndRadius = ShapeData.ConeRange * FMath::Tan(ConeRadians);
	DrawDebugCircle(GetWorld(), ConeEnd, EndRadius, NumSegments, Color, false, Duration, 0, ShapeData.DebugLineThickness, ForwardVector, WorldRot.Vector());
}

void UAttackShapeComponent::DrawLine(const FAttackShapeData& ShapeData, const FColor& Color, float Duration)
{
	const FVector StartPos = GetWorldPositionFromShape(ShapeData);
	const FRotator WorldRot = GetWorldRotationFromShape(ShapeData);
	const FVector EndPos = StartPos + WorldRot.Vector() * ShapeData.PrimarySize;
	
	DrawDebugLine(GetWorld(), StartPos, EndPos, Color, false, Duration, 0, ShapeData.DebugLineThickness);
	
	// Draw arrowhead
	const float ArrowSize = 20.0f;
	const FVector ArrowDir = (EndPos - StartPos).GetSafeNormal();
	const FVector Right = FVector::CrossProduct(ArrowDir, FVector::UpVector).GetSafeNormal();
	const FVector ArrowPoint1 = EndPos - ArrowDir * ArrowSize + Right * ArrowSize * 0.5f;
	const FVector ArrowPoint2 = EndPos - ArrowDir * ArrowSize - Right * ArrowSize * 0.5f;
	
	DrawDebugLine(GetWorld(), EndPos, ArrowPoint1, Color, false, Duration, 0, ShapeData.DebugLineThickness);
	DrawDebugLine(GetWorld(), EndPos, ArrowPoint2, Color, false, Duration, 0, ShapeData.DebugLineThickness);
}

void UAttackShapeComponent::DrawRing(const FAttackShapeData& ShapeData, const FColor& Color, float Duration)
{
	const FVector WorldPos = GetWorldPositionFromShape(ShapeData);
	const FRotator WorldRot = GetWorldRotationFromShape(ShapeData);
	
	// Draw inner and outer circles
	DrawDebugCircle(GetWorld(), WorldPos, ShapeData.InnerRadius, CombatConstants::DEBUG_SPHERE_SEGMENTS, 
		Color, false, Duration, 0, ShapeData.DebugLineThickness, FVector::ForwardVector, FVector::RightVector);
	DrawDebugCircle(GetWorld(), WorldPos, ShapeData.OuterRadius, CombatConstants::DEBUG_SPHERE_SEGMENTS, 
		Color, false, Duration, 0, ShapeData.DebugLineThickness, FVector::ForwardVector, FVector::RightVector);
}

void UAttackShapeComponent::DrawArc(const FAttackShapeData& ShapeData, const FColor& Color, float Duration)
{
	const FVector WorldPos = GetWorldPositionFromShape(ShapeData);
	const FRotator WorldRot = GetWorldRotationFromShape(ShapeData);
	
	// Draw arc boundaries
	const float HalfAngle = ShapeData.ConeAngle * 0.5f;
	const FVector Forward = WorldRot.Vector();
	const FVector Right = FVector::CrossProduct(Forward, FVector::UpVector).GetSafeNormal();
	
	// Draw arc edges
	const FVector LeftBoundary = Forward.RotateAngleAxis(-HalfAngle, FVector::UpVector);
	const FVector RightBoundary = Forward.RotateAngleAxis(HalfAngle, FVector::UpVector);
	
	DrawDebugLine(GetWorld(), WorldPos + LeftBoundary * ShapeData.InnerRadius, 
		WorldPos + LeftBoundary * ShapeData.OuterRadius, Color, false, Duration, 0, ShapeData.DebugLineThickness);
	DrawDebugLine(GetWorld(), WorldPos + RightBoundary * ShapeData.InnerRadius, 
		WorldPos + RightBoundary * ShapeData.OuterRadius, Color, false, Duration, 0, ShapeData.DebugLineThickness);
	
	// Draw arc curves (simplified)
	const int32 NumSegments = 8;
	for (int32 i = 0; i <= NumSegments; ++i)
	{
		const float Angle = FMath::Lerp(-HalfAngle, HalfAngle, float(i) / NumSegments);
		const FVector Direction = Forward.RotateAngleAxis(Angle, FVector::UpVector);
		
		const FVector InnerPoint = WorldPos + Direction * ShapeData.InnerRadius;
		const FVector OuterPoint = WorldPos + Direction * ShapeData.OuterRadius;
		
		if (i > 0)
		{
			const float PrevAngle = FMath::Lerp(-HalfAngle, HalfAngle, float(i-1) / NumSegments);
			const FVector PrevDirection = Forward.RotateAngleAxis(PrevAngle, FVector::UpVector);
			const FVector PrevInner = WorldPos + PrevDirection * ShapeData.InnerRadius;
			const FVector PrevOuter = WorldPos + PrevDirection * ShapeData.OuterRadius;
			
			DrawDebugLine(GetWorld(), PrevInner, InnerPoint, Color, false, Duration, 0, ShapeData.DebugLineThickness);
			DrawDebugLine(GetWorld(), PrevOuter, OuterPoint, Color, false, Duration, 0, ShapeData.DebugLineThickness);
		}
	}
}

// Utility function implementations
FVector UAttackShapeComponent::GetWorldPositionFromShape(const FAttackShapeData& ShapeData) const
{
	if (!GetOwner())
		return FVector::ZeroVector;
		
	return GetOwner()->GetTransform().TransformPosition(ShapeData.LocalOffset);
}

FRotator UAttackShapeComponent::GetWorldRotationFromShape(const FAttackShapeData& ShapeData) const
{
	if (!GetOwner())
		return FRotator::ZeroRotator;
		
	const FRotator OwnerRotation = GetOwner()->GetActorRotation();
	return OwnerRotation + ShapeData.LocalRotation;
}

bool UAttackShapeComponent::IsShapeActiveThisFrame(const FAttackShapeData& ShapeData) const
{
	return CurrentFrame >= ShapeData.ActivationFrame && CurrentFrame <= ShapeData.DeactivationFrame;
}

bool UAttackShapeComponent::CanHitActor(AActor* Actor, const FAttackShapeData& ShapeData) const
{
	if (!Actor || Actor == GetOwner())
		return false;
		
	// Check if already hit (for non-multi-hit attacks)
	if (!ShapeData.bAllowMultiHit && AlreadyHitActors.Contains(Actor))
		return false;
		
	// Check multi-hit constraints
	if (ShapeData.bAllowMultiHit)
	{
		const int32* HitCount = ActorHitCounts.Find(Actor);
		if (HitCount && *HitCount >= ShapeData.MaxHitsPerTarget)
			return false;
			
		const float* LastHitTime = LastHitTimes.Find(Actor);
		if (LastHitTime)
		{
			const float TimeSinceLastHit = GetWorld()->GetTimeSeconds() - *LastHitTime;
			if (TimeSinceLastHit < ShapeData.MultihitInterval)
				return false;
		}
	}
	
	return true;
}

void UAttackShapeComponent::HandleActorHit(AActor* HitActor, const FVector& HitLocation, const FAttackShapeData& ShapeData)
{
	if (!HitActor)
		return;
		
	// Update hit tracking
	if (!ShapeData.bAllowMultiHit)
	{
		AlreadyHitActors.AddUnique(HitActor);
	}
	else
	{
		int32& HitCount = ActorHitCounts.FindOrAdd(HitActor, 0);
		HitCount++;
		LastHitTimes.FindOrAdd(HitActor) = GetWorld()->GetTimeSeconds();
	}
	
	// Apply damage through GAS - integrate with MyCharacter's ability system
	if (AMyCharacter* MyChar = Cast<AMyCharacter>(GetOwner()))
	{
		// Use GAS to apply damage effect
		if (UAbilitySystemComponent* ASC = MyChar->GetAbilitySystemComponent())
		{
			// Create and apply damage gameplay effect
			if (UGameplayEffect* DamageEffect = CreateDamageEffect(CurrentAttackData))
			{
				FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
				EffectContext.AddHitResult(FHitResult()); // Add basic hit result
				
				FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(DamageEffect->GetClass(), 1.0f, EffectContext);
				if (SpecHandle.IsValid())
				{
					ASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), ASC);
				}
			}
		}
	}
	
	// Broadcast hit event
	OnAttackHit.Broadcast(HitActor, HitLocation);
	
	// Debug visualization
	if (bShowHitResults)
	{
		DrawDebugSphere(GetWorld(), HitLocation, 15.0f, 8, FColor::Orange, false, HitResultDisplayTime);
		DrawDebugString(GetWorld(), HitLocation + FVector(0, 0, 50), FString::Printf(TEXT("HIT: %s"), *HitActor->GetName()), 
			nullptr, FColor::White, HitResultDisplayTime);
	}
	
	UE_LOG(LogTemp, Log, TEXT("AttackShapeComponent: Hit actor '%s' at %s"), 
		*HitActor->GetName(), *HitLocation.ToString());
}

UGameplayEffect* UAttackShapeComponent::CreateDamageEffect(const FAttackPrototypeData& AttackData)
{
	// For now, return the default damage effect class
	// In a full implementation, you'd create different effects based on attack data
	return UGameplayEffect_Damage::StaticClass()->GetDefaultObject<UGameplayEffect_Damage>();
}
