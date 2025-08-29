#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatSystemTypes.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"
#include "AttackShapeComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAttackHit, AActor*, HitActor, FVector, HitLocation);

UCLASS( ClassGroup=(Combat), meta=(BlueprintSpawnableComponent) )
class EROEOREOREOR_API UAttackShapeComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UAttackShapeComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Main interface
	UFUNCTION(BlueprintCallable, Category = "Attack Shape")
	void StartAttack(const FAttackPrototypeData& AttackData);
	
	UFUNCTION(BlueprintCallable, Category = "Attack Shape")
	void StopAttack();
	
	UFUNCTION(BlueprintPure, Category = "Attack Shape")
	bool IsAttackActive() const { return bAttackActive; }

	// Shape testing and visualization
	UFUNCTION(BlueprintCallable, Category = "Attack Shape")
	void PreviewAttackShape(const FAttackShapeData& ShapeData, float PreviewTime = 3.0f);
	
	UFUNCTION(BlueprintCallable, Category = "Attack Shape")
	void ClearPreview();

	// Debug and prototyping functions
	UFUNCTION(BlueprintCallable, Category = "Attack Shape")
	void SetDebugVisualization(bool bEnabled) { bShowDebugShapes = bEnabled; }
	
	UFUNCTION(BlueprintCallable, Category = "Attack Shape")
	void TestShapeAtLocation(const FAttackShapeData& ShapeData, const FVector& WorldLocation);

	// Events
	UPROPERTY(BlueprintAssignable)
	FOnAttackHit OnAttackHit;

	// Configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bShowDebugShapes = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bShowHitResults = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	float HitResultDisplayTime = 2.0f;

protected:
	// Current attack state
	UPROPERTY(Transient)
	bool bAttackActive = false;
	
	UPROPERTY(Transient)
	FAttackPrototypeData CurrentAttackData;
	
	UPROPERTY(Transient)
	float AttackStartTime = 0.0f;
	
	UPROPERTY(Transient)
	int32 CurrentFrame = 0;
	
	UPROPERTY(Transient)
	TArray<AActor*> AlreadyHitActors;
	
	// Multi-hit tracking
	UPROPERTY(Transient)
	TMap<AActor*, int32> ActorHitCounts;
	
	UPROPERTY(Transient)
	TMap<AActor*, float> LastHitTimes;

	// Preview state
	UPROPERTY(Transient)
	bool bShowingPreview = false;
	
	UPROPERTY(Transient)
	FAttackShapeData PreviewShapeData;
	
	UPROPERTY(Transient)
	float PreviewEndTime = 0.0f;

private:
	// Core functionality
	void UpdateAttack(float DeltaTime);
	void ProcessActiveShapes();
	bool CheckShapeCollision(const FAttackShapeData& ShapeData, TArray<FHitResult>& OutHits);
	void DrawShapeDebug(const FAttackShapeData& ShapeData, const FColor& Color, float Duration = -1.0f);
	
	// Shape-specific collision detection
	bool CheckSphereCollision(const FAttackShapeData& ShapeData, TArray<FHitResult>& OutHits);
	bool CheckCapsuleCollision(const FAttackShapeData& ShapeData, TArray<FHitResult>& OutHits);
	bool CheckBoxCollision(const FAttackShapeData& ShapeData, TArray<FHitResult>& OutHits);
	bool CheckConeCollision(const FAttackShapeData& ShapeData, TArray<FHitResult>& OutHits);
	bool CheckLineCollision(const FAttackShapeData& ShapeData, TArray<FHitResult>& OutHits);
	bool CheckRingCollision(const FAttackShapeData& ShapeData, TArray<FHitResult>& OutHits);
	bool CheckArcCollision(const FAttackShapeData& ShapeData, TArray<FHitResult>& OutHits);
	
	// Shape-specific debug drawing
	void DrawSphere(const FAttackShapeData& ShapeData, const FColor& Color, float Duration);
	void DrawCapsule(const FAttackShapeData& ShapeData, const FColor& Color, float Duration);
	void DrawBox(const FAttackShapeData& ShapeData, const FColor& Color, float Duration);
	void DrawCone(const FAttackShapeData& ShapeData, const FColor& Color, float Duration);
	void DrawLine(const FAttackShapeData& ShapeData, const FColor& Color, float Duration);
	void DrawRing(const FAttackShapeData& ShapeData, const FColor& Color, float Duration);
	void DrawArc(const FAttackShapeData& ShapeData, const FColor& Color, float Duration);
	
	// Utility functions
	FVector GetWorldPositionFromShape(const FAttackShapeData& ShapeData) const;
	FRotator GetWorldRotationFromShape(const FAttackShapeData& ShapeData) const;
	bool IsShapeActiveThisFrame(const FAttackShapeData& ShapeData) const;
	bool CanHitActor(AActor* Actor, const FAttackShapeData& ShapeData) const;
	void HandleActorHit(AActor* HitActor, const FVector& HitLocation, const FAttackShapeData& ShapeData);

	// Performance optimization
	float LastCollisionCheckTime = 0.0f;
	static constexpr float CollisionCheckInterval = 1.0f / 60.0f; // 60 FPS
};
