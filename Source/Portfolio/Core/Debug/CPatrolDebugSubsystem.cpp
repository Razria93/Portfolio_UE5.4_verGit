#include "Core/Debug/CPatrolDebugSubsystem.h"
#include "Core/Debug/CDebugOverlayFocusComponent.h"
#include "AI/Blackboard/CAIKey.h"
#include "AI/Patrol/CPatrolPath.h"
#include "Character/Enemy/CEnemy.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Tasks/BTTask_MoveTo.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "HAL/IConsoleManager.h"
#include "Navigation/PathFollowingComponent.h"

#if !UE_BUILD_SHIPPING
namespace
{
	TAutoConsoleVariable<int32> PatrolDebugEnabled(TEXT("Portfolio.DebugOverlay.Patrol.Enabled"), 0, TEXT("Focused Enemy patrol diagnostics."));
	TAutoConsoleVariable<int32> DrawPoints(TEXT("Portfolio.DebugOverlay.Patrol.DrawPoints"), 1, TEXT("Draw configured patrol points and zero-based indices."));
	TAutoConsoleVariable<int32> DrawLinks(TEXT("Portfolio.DebugOverlay.Patrol.DrawLinks"), 0, TEXT("Draw dashed configured order, not navigation path. Random has no fixed links."));
	TAutoConsoleVariable<int32> DrawTarget(TEXT("Portfolio.DebugOverlay.Patrol.DrawTarget"), 1, TEXT("Highlight selected patrol destination while a patrol MoveTo is active."));
	TAutoConsoleVariable<int32> RecordTrail(TEXT("Portfolio.DebugOverlay.Patrol.RecordTrail"), 0, TEXT("Record actual focused Enemy movement only during active PatrolLocation MoveTo; independent of HUD/event capture."));
	TAutoConsoleVariable<int32> DrawTrail(TEXT("Portfolio.DebugOverlay.Patrol.DrawTrail"), 1, TEXT("Draw recorded patrol movement."));
	TAutoConsoleVariable<int32> DrawAcceptance(TEXT("Portfolio.DebugOverlay.Patrol.DrawAcceptance"), 0, TEXT("Draw configured MoveTo acceptance radius, radius inclusion flags, and observed completion distance. The ring is the base radius, not the full capsule-inclusive reach test."));
	TAutoConsoleVariable<float> SampleInterval(TEXT("Portfolio.DebugOverlay.Patrol.SampleInterval"), 0.1f, TEXT("Sampling seconds, clamped 0.02-1."));
	TAutoConsoleVariable<float> MinDistance(TEXT("Portfolio.DebugOverlay.Patrol.MinDistance"), 10.f, TEXT("Minimum sample displacement in cm, clamped 1-100."));
	TAutoConsoleVariable<float> RetainSeconds(TEXT("Portfolio.DebugOverlay.Patrol.RetainSeconds"), 60.f, TEXT("World seconds retained, clamped 1-300."));
	TAutoConsoleVariable<int32> MaxPoints(TEXT("Portfolio.DebugOverlay.Patrol.MaxPoints"), 600, TEXT("Maximum history points, clamped 2-3000."));
	FAutoConsoleCommandWithWorld ClearCommand(TEXT("Portfolio.DebugOverlay.Patrol.ClearTrail"), TEXT("Clear patrol trail in the current world; preserve focus, event history and settings."),
		FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
		{
			if (World) if (auto* Debug = World->GetSubsystem<UCPatrolDebugSubsystem>()) Debug->ClearTrail();
		}));

	const UBlackboardComponent* Blackboard(const ACEnemy* Enemy)
	{
		const AAIController* Controller = IsValid(Enemy) ? Cast<AAIController>(Enemy->GetController()) : nullptr;
		return Controller ? Controller->GetBlackboardComponent() : nullptr;
	}

	const UBTTask_MoveTo* ActivePatrolMove(const ACEnemy* Enemy)
	{
		const AAIController* Controller = IsValid(Enemy) ? Cast<AAIController>(Enemy->GetController()) : nullptr;
		const UBehaviorTreeComponent* BT = Controller ? Cast<UBehaviorTreeComponent>(Controller->GetBrainComponent()) : nullptr;
		const UBTTaskNode* Task = BT ? Cast<UBTTaskNode>(BT->GetActiveNode()) : nullptr;
		return BT && BT->IsRunning() && UCPatrolDebugSubsystem::IsPatrolMoveTask(Task) && BT->GetTaskStatus(Task) == EBTTaskStatus::Active ? Cast<UBTTask_MoveTo>(Task) : nullptr;
	}

	bool IsMovingOnPatrol(const ACEnemy* Enemy) { return ActivePatrolMove(Enemy) != nullptr; }

	FVector FootLocation(const ACEnemy* Enemy)
	{
		return Enemy->GetActorLocation() - FVector(0, 0, Enemy->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()) + FVector(0, 0, 6);
	}

	void DashedLine(UWorld* World, const FVector& A, const FVector& B)
	{
		const FVector Delta = B-A;
		const float Length = Delta.Size();
		const FVector Direction = Delta.GetSafeNormal();
		// Bound work for accidentally distant points.
		const float Step = FMath::Max(40.f, Length / 100.f);
		for (float D=0; D<Length; D+=Step)
			DrawDebugLine(World, A+Direction*D, A+Direction*FMath::Min(D+Step*0.5f,Length), FColor::Silver, false, 0, 0, 1);
	}
}
#endif

void FPatrolTrailHistory::Reset()
{
	Points.Reset(); Segment=0; bActive=false; LastSampleTime=-1;
}

void FPatrolTrailHistory::Finish(const FVector& ActualLocation, double Time, int32 Limit)
{
	if (!bActive) return;
	Limit = FMath::Clamp(Limit, 2, 3000);
	// Capture a real endpoint even below the sampling interval / distance threshold.
	// A teleport must never become a travelled line.
	if (FVector::DistSquared(ActualLocation, LastObserved) > FMath::Square(500.f)) ++Segment;
	if (Points.IsEmpty() || Points.Last().Segment != Segment || !Points.Last().Location.Equals(ActualLocation, 0.1f))
	{
		Points.Add({ActualLocation, Time, Segment});
		if (Points.Num() > Limit) Points.RemoveAt(0, Points.Num() - Limit);
	}
	LastObserved = ActualLocation;
	LastObservedTime = Time;
	bActive = false;
}

void FPatrolTrailHistory::Update(const FVector& Location, double Time, bool bMoving, float Interval, float Minimum, float Retention, int32 Limit)
{
	Interval=FMath::Clamp(Interval,0.02f,1.f); Minimum=FMath::Clamp(Minimum,1.f,100.f);
	Retention=FMath::Clamp(Retention,1.f,300.f); Limit=FMath::Clamp(Limit,2,3000);
	while (Points.Num() && (Time-Points[0].Time > Retention || Points.Num()>Limit)) Points.RemoveAt(0);
	// If completion was not observed, only flush the last position known to be patrol.
	// The current position might already belong to Chase/ReturnHome.
	if (!bMoving)
	{
		if (Time - LastObservedTime <= Retention) Finish(LastObserved, LastObservedTime, Limit);
		else bActive = false;
		return;
	}
	// A large per-tick discontinuity is not a travelled segment.
	const bool bNewSegment = !bActive || FVector::DistSquared(Location,LastObserved)>FMath::Square(500.f) || Time<LastSampleTime;
	LastObserved=Location;
	LastObservedTime=Time;
	if (bNewSegment) { ++Segment; LastSampleTime=-1; }
	bActive=true;
	if (!bNewSegment && (Time-LastSampleTime<Interval || (Points.Num() && FVector::DistSquared(Location,Points.Last().Location)<FMath::Square(Minimum)))) return;
	Points.Add({Location,Time,Segment}); LastSampleTime=Time;
	if (Points.Num()>Limit) Points.RemoveAt(0,Points.Num()-Limit);
}

bool UCPatrolDebugSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
#if !UE_BUILD_SHIPPING
	const UWorld* World=Cast<UWorld>(Outer);
	return World && (World->WorldType==EWorldType::Game || World->WorldType==EWorldType::PIE) && Super::ShouldCreateSubsystem(Outer);
#else
	return false;
#endif
}

TStatId UCPatrolDebugSubsystem::GetStatId() const { RETURN_QUICK_DECLARE_CYCLE_STAT(UCPatrolDebugSubsystem, STATGROUP_Tickables); }
void UCPatrolDebugSubsystem::Deinitialize() { UnbindFollowing(); ClearTrail(); Focus.Reset(); RecordedPath.Reset(); Super::Deinitialize(); }
void UCPatrolDebugSubsystem::ClearTrail()
{
	History.Reset(); LastResult.Reset(); LastEndTime = -1;
	TrackedRequest = FAIRequestID::InvalidRequest;
}

void UCPatrolDebugSubsystem::UnbindFollowing()
{
	if (ObservedFollowing.IsValid()) ObservedFollowing->OnRequestFinished.Remove(FinishHandle);
	FinishHandle.Reset(); ObservedFollowing.Reset();
	TrackedRequest = FAIRequestID::InvalidRequest;
}

void UCPatrolDebugSubsystem::OnMoveFinished(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
#if !UE_BUILD_SHIPPING
	if (PatrolDebugEnabled.GetValueOnGameThread() == 0 || !Focus.IsValid() || TrackedRequest != RequestID || !TrackedRequest.IsValid()) return;
	LastEndLocation = FootLocation(Focus.Get());
	LastEndGoal = TrackedGoal;
	LastEndIndex = TrackedIndex;
	LastEndTime = GetWorld()->GetTimeSeconds();
	LastResult = ObservedFollowing.IsValid() ? ObservedFollowing->GetResultDesc(Result.Code) : TEXT("Unknown");
	if (RecordTrail.GetValueOnGameThread() != 0)
		History.Finish(LastEndLocation, LastEndTime, MaxPoints.GetValueOnGameThread());
	else History.bActive = false;
	TrackedRequest = FAIRequestID::InvalidRequest;
#endif
}

bool UCPatrolDebugSubsystem::IsPatrolMoveTask(const UBTTaskNode* Task)
{
	const UBTTask_MoveTo* Move=Cast<UBTTask_MoveTo>(Task);
	return Move && Move->GetSelectedBlackboardKey()==CAIKey::Patrol::PatrolLocation.KeyName;
}

void UCPatrolDebugSubsystem::Tick(float DeltaTime)
{
#if !UE_BUILD_SHIPPING
	if (PatrolDebugEnabled.GetValueOnGameThread()==0) { UnbindFollowing(); ClearTrail(); Focus.Reset(); RecordedPath.Reset(); return; }
	const APlayerController* PC=GetWorld()->GetFirstPlayerController();
	const auto* FocusComp=PC ? PC->FindComponentByClass<UCDebugOverlayFocusComponent>() : nullptr;
	ACEnemy* Enemy=FocusComp ? Cast<ACEnemy>(FocusComp->GetDebugOverlayFocusActor()) : nullptr;
	const UBlackboardComponent* BB=Blackboard(Enemy);
	AActor* Path=BB ? Cast<AActor>(BB->GetValueAsObject(CAIKey::Patrol::PatrolPath.KeyName)) : nullptr;
	if (Focus.Get()!=Enemy || RecordedPath.Get()!=Path) { UnbindFollowing(); ClearTrail(); Focus=Enemy; RecordedPath=Path; }
	const AAIController* Controller = IsValid(Enemy) ? Cast<AAIController>(Enemy->GetController()) : nullptr;
	UPathFollowingComponent* Following = Controller ? Controller->GetPathFollowingComponent() : nullptr;
	if (ObservedFollowing.Get() != Following)
	{
		UnbindFollowing();
		ObservedFollowing = Following;
		if (Following) FinishHandle = Following->OnRequestFinished.AddUObject(this, &UCPatrolDebugSubsystem::OnMoveFinished);
	}
	if (Following && ActivePatrolMove(Enemy))
	{
		const FAIRequestID Request = Following->GetCurrentRequestId();
		if (TrackedRequest != Request)
		{
			History.Finish(History.LastObserved, History.LastObservedTime, MaxPoints.GetValueOnGameThread());
			TrackedRequest = Request;
			TrackedGoal = BB->GetValueAsVector(CAIKey::Patrol::PatrolLocation.KeyName);
			TrackedIndex = BB->GetValueAsInt(CAIKey::Patrol::PatrolIndex.KeyName);
		}
	}
	else TrackedRequest = FAIRequestID::InvalidRequest;
	const bool bRecording=IsValid(Enemy) && RecordTrail.GetValueOnGameThread()!=0 && IsMovingOnPatrol(Enemy);
	History.Update(IsValid(Enemy)?FootLocation(Enemy):FVector::ZeroVector,GetWorld()->GetTimeSeconds(),bRecording,
		SampleInterval.GetValueOnGameThread(),MinDistance.GetValueOnGameThread(),RetainSeconds.GetValueOnGameThread(),MaxPoints.GetValueOnGameThread());
#endif
}

void UCPatrolDebugSubsystem::Draw(const ACEnemy* Enemy) const
{
#if !UE_BUILD_SHIPPING
	if (PatrolDebugEnabled.GetValueOnGameThread()==0 || !IsValid(Enemy) || Focus.Get()!=Enemy) return;
	UWorld* World=GetWorld();
	const auto* BB=Blackboard(Enemy);
	const auto* Path=BB ? Cast<ACPatrolPath>(BB->GetValueAsObject(CAIKey::Patrol::PatrolPath.KeyName)) : nullptr;
	const bool bMoving=IsMovingOnPatrol(Enemy);
	if (DrawAcceptance.GetValueOnGameThread() != 0)
	{
		if (const UBTTask_MoveTo* Move = ActivePatrolMove(Enemy))
		{
			const FVector Goal = BB->GetValueAsVector(CAIKey::Patrol::PatrolLocation.KeyName) + FVector(0,0,8);
			const auto* Controller = Cast<AAIController>(Enemy->GetController());
			const auto* Following = Controller ? Controller->GetPathFollowingComponent() : nullptr;
			const float Radius = Following ? Following->GetAcceptanceRadius() : Move->AcceptableRadius;
			if (Radius > 0) DrawDebugCircle(World,Goal,Radius,48,FColor::Orange,false,0,0,1.5f,FVector::ForwardVector,FVector::RightVector,false);
			float AgentRadius = 0, AgentHeight = 0;
			Enemy->GetSimpleCollisionCylinder(AgentRadius, AgentHeight);
			DrawDebugString(World,Goal+FVector(0,0,70),FString::Printf(TEXT("Acceptance (base): %.1f cm\nAgent radius: %s (%.1f cm) | Goal radius: %s\nDistance XY: %.1f cm"),
				Radius,Move->bReachTestIncludesAgentRadius?TEXT("included"):TEXT("excluded"),AgentRadius,
				Move->bReachTestIncludesGoalRadius?TEXT("included"):TEXT("excluded"),FVector::Dist2D(Enemy->GetActorLocation(),Goal)),nullptr,FColor::Orange,0,true,1.2f);
		}
		if (LastEndTime >= 0 && World->GetTimeSeconds()-LastEndTime < 5)
		{
			DrawDebugPoint(World,LastEndLocation,12,FColor::Green,false,0);
			DrawDebugString(World,LastEndLocation+FVector(0,0,35),FString::Printf(TEXT("P%d end: %s\nRemaining XY: %.1f cm"),LastEndIndex,*LastResult,FVector::Dist2D(LastEndLocation,LastEndGoal)),nullptr,FColor::Green,0,true,1.2f);
		}
	}
	if (IsValid(Path))
	{
		const int32 Index=BB->GetValueAsInt(CAIKey::Patrol::PatrolIndex.KeyName);
		const EPatrolMode Mode=static_cast<EPatrolMode>(BB->GetValueAsEnum(CAIKey::Patrol::PatrolMode.KeyName));
		const TCHAR* ModeName=Mode==EPatrolMode::Loop?TEXT("Loop"):Mode==EPatrolMode::Reverse?TEXT("Reverse"):Mode==EPatrolMode::Random?TEXT("Random"):TEXT("None");
		for (int32 I=0;I<Path->Num();++I)
		{
			FPatrolPointSnapshot Point;
			if (!Path->GetPointSnapshot(I,Point)) continue;
			const FVector P=Point.Location+FVector(0,0,8);
			const bool bTarget=bMoving && I==Index && DrawTarget.GetValueOnGameThread()!=0;
			if (DrawPoints.GetValueOnGameThread()!=0 || bTarget)
			{
				const FColor Color=bTarget?FColor::Yellow:FColor::Silver;
				DrawDebugCircle(World,P,bTarget?32.f:20.f,24,Color,false,0,0,bTarget?2.f:1.f,FVector::ForwardVector,FVector::RightVector,false);
				DrawDebugString(World,P+FVector(0,0,30),FString::Printf(TEXT("%sP%d"),bTarget?TEXT("Target "):TEXT(""),I),nullptr,Color,0,true,1.3f);
			}
			if (DrawLinks.GetValueOnGameThread()!=0 && (Mode==EPatrolMode::Loop || Mode==EPatrolMode::Reverse))
			{
				const int32 Next=I+1<Path->Num()?I+1:(Mode==EPatrolMode::Loop && Path->Num()>2?0:INDEX_NONE);
				FPatrolPointSnapshot Other;
				if (Next!=INDEX_NONE && Path->GetPointSnapshot(Next,Other)) DashedLine(World,P,Other.Location+FVector(0,0,8));
			}
		}
		if (DrawTarget.GetValueOnGameThread()!=0)
		{
			FPatrolPointSnapshot Target;
			if (bMoving && Path->GetPointSnapshot(Index,Target))
				DrawDebugDirectionalArrow(World,FootLocation(Enemy),Target.Location+FVector(0,0,8),25,FColor::Yellow,false,0,0,1);
			const TCHAR* Direction=Mode==EPatrolMode::Reverse?(BB->GetValueAsBool(CAIKey::Patrol::bPatrolReverse.KeyName)?TEXT(" / Backward"):TEXT(" / Forward")):TEXT("");
			const TCHAR* Status=RecordTrail.GetValueOnGameThread()==0?TEXT("Off"):(bMoving?TEXT("Active"):TEXT("Paused"));
			DrawDebugString(World,Enemy->GetActorLocation()+FVector(0,0,120),FString::Printf(TEXT("Patrol: %s%s | Selected: %d\nTrail: %s | %d samples"),ModeName,Direction,Index,Status,History.Points.Num()),nullptr,FColor::White,0,true,1.2f);
		}
	}
	if (DrawTrail.GetValueOnGameThread()!=0)
		for (int32 I=1;I<History.Points.Num();++I)
			if (History.Points[I-1].Segment==History.Points[I].Segment)
				DrawDebugLine(World,History.Points[I-1].Location,History.Points[I].Location,FColor::Cyan,false,0,0,2);
#endif
}
