#include "Core/Debug/FInvestigateDebug.h"
#include "Core/Debug/CPatrolDebugSubsystem.h"
#include "AI/Blackboard/CAIKey.h"
#include "Character/Enemy/CEnemy.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Tasks/BTTask_MoveTo.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "Navigation/PathFollowingComponent.h"

#if !UE_BUILD_SHIPPING
namespace
{
	TAutoConsoleVariable<int32> Enabled(TEXT("Portfolio.DebugOverlay.Investigate.Enabled"), 0, TEXT("Last observed position, selected search destinations and actual Investigate movement. Collected while the custom HUD is visible; never changes AI."));
	TAutoConsoleVariable<float> TextScale(TEXT("Portfolio.DebugOverlay.Investigate.TextScale"), 1.3f, TEXT("Search world label scale, clamped 0.5-3."));
	struct FSearchRecord
	{
		TWeakObjectPtr<const ACEnemy> Focus;
		FPatrolTrailHistory Trail;
		TArray<FVector> Goals;
		FVector Anchor = FVector::ZeroVector;
		FAIRequestID Request = FAIRequestID::InvalidRequest;
		bool bSearching = false;
		bool bHasSearch = false;
		double LastActiveTime = 0;
	};
	TMap<TWeakObjectPtr<UWorld>, FSearchRecord> Records;
	FAutoConsoleCommandWithWorld ClearCommand(TEXT("Portfolio.DebugOverlay.Investigate.ClearTrail"), TEXT("Clear focused search visualization without changing AI state."),
		FConsoleCommandWithWorldDelegate::CreateStatic(&FInvestigateDebug::Clear));
	const FColor SearchColor(220, 110, 255);
	const FColor AnchorColor(255, 190, 60);
	bool Usable(const FVector& V) { return !V.ContainsNaN() && V.SizeSquared() < FMath::Square(1.e8); }
}
#endif

void FInvestigateDebug::Clear(UWorld* World)
{
#if !UE_BUILD_SHIPPING
	Records.Remove(World);
#endif
}

void FInvestigateDebug::Draw(UWorld* World, const ACEnemy* Enemy)
{
#if !UE_BUILD_SHIPPING
	for (auto It = Records.CreateIterator(); It; ++It) if (!It.Key().IsValid()) It.RemoveCurrent();
	if (!World) return;
	if (Enabled.GetValueOnGameThread() == 0 || !IsValid(Enemy)) { Clear(World); return; }
	const AAIController* Controller = Cast<AAIController>(Enemy->GetController());
	const UBlackboardComponent* BB = Controller ? Controller->GetBlackboardComponent() : nullptr;
	if (!BB) { Clear(World); return; }
	FSearchRecord& R = Records.FindOrAdd(World);
	if (R.Focus.Get() != Enemy) { R = FSearchRecord(); R.Focus = Enemy; }
	const double Now = World->GetTimeSeconds();
	const bool Searching = BB->GetValueAsBool(CAIKey::Investigate::bIsInvestigating.KeyName);
	const FVector Anchor = BB->GetValueAsVector(CAIKey::Perception::LastKnownLocation.KeyName);
	if (Searching && Usable(Anchor) && (!R.bSearching || !R.Anchor.Equals(Anchor, 1.f)))
	{
		R.Trail.Reset(); R.Goals.Reset(); R.Request = FAIRequestID::InvalidRequest;
		R.Anchor = Anchor; R.bHasSearch = true;
	}
	const UBehaviorTreeComponent* BT = Controller ? Cast<UBehaviorTreeComponent>(Controller->GetBrainComponent()) : nullptr;
	const UBTTask_MoveTo* Move = BT ? Cast<UBTTask_MoveTo>(BT->GetActiveNode()) : nullptr;
	const UPathFollowingComponent* Following = Controller->GetPathFollowingComponent();
	const bool Moving = Searching && BT && BT->IsRunning() && Move
		&& Move->GetSelectedBlackboardKey() == CAIKey::Investigate::InvestigateLocation.KeyName
		&& BT->GetTaskStatus(Move) == EBTTaskStatus::Active && Following && Following->GetStatus() == EPathFollowingStatus::Moving;
	if (Moving && R.Request != Following->GetCurrentRequestId())
	{
		R.Trail.Finish(R.Trail.LastObserved, R.Trail.LastObservedTime, 1200);
		R.Request = Following->GetCurrentRequestId();
		const FVector Goal = BB->GetValueAsVector(CAIKey::Investigate::InvestigateLocation.KeyName);
		// Observe the Blackboard destination; its selection source is not known here.
		// The initial last-seen destination is labeled separately.
		if (Usable(Goal) && !Goal.Equals(R.Anchor, 1.f))
		{
			R.Goals.Add(Goal);
			if (R.Goals.Num() > 32) R.Goals.RemoveAt(0);
		}
	}
	if (!Moving) R.Request = FAIRequestID::InvalidRequest;
	const float HalfHeight = Enemy->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const FVector Foot = Enemy->GetActorLocation() - FVector(0,0,HalfHeight) + FVector(0,0,8);
	R.Trail.Update(Foot, Now, Moving, 0.08f, 8.f, 120.f, 1200);
	R.bSearching = Searching;
	if (Searching) R.LastActiveTime = Now;
	if (!R.bHasSearch || (!Searching && Now - R.LastActiveTime > 5)) return;
	const float Scale = FMath::Clamp(TextScale.GetValueOnGameThread(), 0.5f, 3.f);
	// LastKnownLocation can be a projected navigation point or a target actor position.
	// Preserve its source height; the observing enemy's capsule is unrelated to it.
	const FVector AnchorMarker = R.Anchor + FVector(0,0,10);
	DrawDebugCircle(World, AnchorMarker, 35, 32, AnchorColor, false, 0, 0, 2, FVector::ForwardVector, FVector::RightVector, false);
	DrawDebugString(World, AnchorMarker + FVector(0,0,65), TEXT("Last Seen"), nullptr, AnchorColor, 0, true, Scale);
	for (int32 I=0; I<R.Goals.Num(); ++I)
	{
		const FVector P = R.Goals[I] + FVector(0,0,10);
		DrawDebugCircle(World, P, 25, 24, SearchColor, false, 0, 0, 2, FVector::ForwardVector, FVector::RightVector, false);
		DrawDebugString(World, P + FVector(0,0,45), FString::Printf(TEXT("S%d | Search target"), I+1), nullptr, SearchColor, 0, true, Scale);
	}
	for (int32 I=1; I<R.Trail.Points.Num(); ++I)
		if (R.Trail.Points[I-1].Segment == R.Trail.Points[I].Segment)
			DrawDebugLine(World, R.Trail.Points[I-1].Location, R.Trail.Points[I].Location, SearchColor, false, 0, 0, 3);
	DrawDebugString(World, Enemy->GetActorLocation()+FVector(0,0,HalfHeight+45),
		FString::Printf(TEXT("Search: %s | %d observed search goals\nPurple: actual search trail"), Searching?TEXT("Active"):TEXT("Ended"), R.Goals.Num()), nullptr, SearchColor, 0, true, Scale);
#endif
}
