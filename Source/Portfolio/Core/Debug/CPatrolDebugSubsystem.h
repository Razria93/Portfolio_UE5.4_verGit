#pragma once

#include "CoreMinimal.h"
#include "AITypes.h"
#include "Subsystems/WorldSubsystem.h"
#include "CPatrolDebugSubsystem.generated.h"

class ACEnemy;
class UBTTaskNode;
class UPathFollowingComponent;
struct FPathFollowingResult;

struct FPatrolTrailPoint
{
	FVector Location;
	double Time = 0;
	uint32 Segment = 0;
};

// Bounded, world-time history. Gaps never connect separate patrol moves.
struct PORTFOLIO_API FPatrolTrailHistory
{
	TArray<FPatrolTrailPoint> Points;
	uint32 Segment = 0;
	bool bActive = false;
	double LastSampleTime = -1;
	FVector LastObserved = FVector::ZeroVector;
	double LastObservedTime = 0;
	void Reset();
	void Finish(const FVector& ActualLocation, double Time, int32 MaxPoints);
	void Update(const FVector& Location, double Time, bool bMoving, float Interval, float MinDistance, float RetainSeconds, int32 MaxPoints);
};

UCLASS()
class PORTFOLIO_API UCPatrolDebugSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()
public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual void Deinitialize() override;
	void ClearTrail();
	void Draw(const ACEnemy* Enemy) const;
	static bool IsPatrolMoveTask(const UBTTaskNode* Task);
private:
	TWeakObjectPtr<ACEnemy> Focus;
	TWeakObjectPtr<AActor> RecordedPath;
	FPatrolTrailHistory History;
	TWeakObjectPtr<UPathFollowingComponent> ObservedFollowing;
	FDelegateHandle FinishHandle;
	FAIRequestID TrackedRequest = FAIRequestID::InvalidRequest;
	FVector TrackedGoal = FVector::ZeroVector;
	int32 TrackedIndex = INDEX_NONE;
	FVector LastEndLocation = FVector::ZeroVector;
	FVector LastEndGoal = FVector::ZeroVector;
	FString LastResult;
	int32 LastEndIndex = INDEX_NONE;
	double LastEndTime = -1;
	void UnbindFollowing();
	void OnMoveFinished(FAIRequestID RequestID, const FPathFollowingResult& Result);
};
