#include "Component/CTargetingComponent.h"

#include "ProjectGlobal.h"

#include "Character/Enemy/CEnemy.h"
#include "Component/CHealthComponent.h"

#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"

namespace
{
	struct FTargetSwitchCandidateEvaluation
	{
		ACEnemy* Target = nullptr;
		FVector2D ScreenPosition = FVector2D::ZeroVector;
		float HorizontalDelta = 0.f;
		float VerticalDelta = 0.f;
		float TargetScore = 0.f;
	};

	float CalculateAngleScore(float InMinDot, float InDot)
	{
		const float scoreRange = 1.f - InMinDot;
		if (scoreRange <= SMALL_NUMBER)
		{
			return InDot >= InMinDot ? 1.f : 0.f;
		}

		return FMath::Clamp(FMath::GetRangePct(InMinDot, 1.f, InDot), 0.f, 1.f);
	}

	bool IsBetterSwitchCandidate(const FTargetSwitchCandidateEvaluation& InCandidate, const FTargetSwitchCandidateEvaluation& InBestCandidate)
	{
		const float candidateHorizontalDistance = FMath::Abs(InCandidate.HorizontalDelta);
		const float bestHorizontalDistance = FMath::Abs(InBestCandidate.HorizontalDelta);
		if (!FMath::IsNearlyEqual(candidateHorizontalDistance, bestHorizontalDistance, KINDA_SMALL_NUMBER))
		{
			return candidateHorizontalDistance < bestHorizontalDistance;
		}

		const float candidateVerticalDistance = FMath::Abs(InCandidate.VerticalDelta);
		const float bestVerticalDistance = FMath::Abs(InBestCandidate.VerticalDelta);
		if (!FMath::IsNearlyEqual(candidateVerticalDistance, bestVerticalDistance, KINDA_SMALL_NUMBER))
		{
			return candidateVerticalDistance < bestVerticalDistance;
		}

		return InCandidate.TargetScore > InBestCandidate.TargetScore;
	}
}

// ===== Lifecycle =====

UCTargetingComponent::UCTargetingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

// ===== Component Reference =====

void UCTargetingComponent::InitializeReferences(APlayerController* InOwnerPlayerController)
{
	OwnerPlayerController_Injected = InOwnerPlayerController;
	ValidateRequiredReferences();
}

// ===== Lifecycle =====

void UCTargetingComponent::EndPlay(const EEndPlayReason::Type InEndPlayReason)
{
	UnbindTargetDestroyed(CurrentTarget.Get());
	CurrentTarget.Reset();
	ValidationElapsedTime = 0.f;

	Super::EndPlay(InEndPlayReason);
}

void UCTargetingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	ValidationElapsedTime += FMath::Max(DeltaTime, 0.f);
	if (ValidationElapsedTime < TargetingTuning.ValidationInterval) return;

	ValidationElapsedTime = 0.f;
	ValidateCurrentTarget();
}

// ===== Target Command =====

void UCTargetingComponent::ToggleTargetLock()
{
	if (HasTarget())
	{
		ClearTarget();
		return;
	}

	AcquireBestTarget();
}

bool UCTargetingComponent::AcquireBestTarget()
{
	if (!IsValid(OwnerPlayerController_Injected)) return false;

	UWorld* world = GetWorld();
	if (!IsValid(world)) return false;

	ACEnemy* bestTarget = nullptr;
	float bestScore = -FLT_MAX;

	for (TActorIterator<ACEnemy> iterator(world); iterator; ++iterator)
	{
		ACEnemy* candidate = *iterator;
		float candidateScore = 0.f;

		if (!TryScoreTarget(candidate, candidateScore)) continue;
		if (candidateScore <= bestScore) continue;

		bestTarget = candidate;
		bestScore = candidateScore;
	}

	if (!IsValid(bestTarget)) return false;

	SetCurrentTarget(bestTarget);
	return true;
}

bool UCTargetingComponent::SwitchTarget(ETargetSwitchDirection InDirection)
{
	if (InDirection != ETargetSwitchDirection::Left && InDirection != ETargetSwitchDirection::Right) return false;
	if (!IsValid(OwnerPlayerController_Injected)) return false;

	ACEnemy* currentTarget = CurrentTarget.Get();
	if (!IsValid(currentTarget)) return AcquireBestTarget();

	FVector2D currentScreenPosition = FVector2D::ZeroVector;
	if (!ProjectTargetToViewport(currentTarget, currentScreenPosition)) return false;

	UWorld* world = GetWorld();
	if (!IsValid(world)) return false;

	FTargetSwitchCandidateEvaluation bestCandidate;
	bool bHasBestCandidate = false;

	for (TActorIterator<ACEnemy> iterator(world); iterator; ++iterator)
	{
		ACEnemy* candidate = *iterator;
		if (candidate == currentTarget) continue;

		FTargetingDebugSnapshot targetEvaluation;
		if (!BuildTargetEvaluation(candidate, targetEvaluation)) continue;
		if (!IsTargetEvaluationValid(candidate, targetEvaluation, true)) continue;

		FTargetSwitchCandidateEvaluation candidateEvaluation;
		candidateEvaluation.Target = candidate;
		candidateEvaluation.TargetScore = targetEvaluation.FinalScore;
		if (!ProjectTargetToViewport(candidate, candidateEvaluation.ScreenPosition)) continue;

		candidateEvaluation.HorizontalDelta = candidateEvaluation.ScreenPosition.X - currentScreenPosition.X;
		candidateEvaluation.VerticalDelta = candidateEvaluation.ScreenPosition.Y - currentScreenPosition.Y;

		if (FMath::Abs(candidateEvaluation.HorizontalDelta) <= KINDA_SMALL_NUMBER) continue;
		if (InDirection == ETargetSwitchDirection::Left && candidateEvaluation.HorizontalDelta >= 0.f) continue;
		if (InDirection == ETargetSwitchDirection::Right && candidateEvaluation.HorizontalDelta <= 0.f) continue;

		if (bHasBestCandidate && !IsBetterSwitchCandidate(candidateEvaluation, bestCandidate)) continue;

		bestCandidate = candidateEvaluation;
		bHasBestCandidate = true;
	}

	if (!bHasBestCandidate || !IsValid(bestCandidate.Target)) return false;

	SetCurrentTarget(bestCandidate.Target);
	return true;
}

void UCTargetingComponent::ClearTarget()
{
	SetCurrentTarget(nullptr);
}

// ===== Target Query =====

bool UCTargetingComponent::HasTarget() const
{
	return IsValid(CurrentTarget.Get());
}

ACEnemy* UCTargetingComponent::GetCurrentTarget() const
{
	return CurrentTarget.Get();
}

bool UCTargetingComponent::BuildDebugSnapshot(FTargetingDebugSnapshot& OutSnapshot) const
{
	ACEnemy* currentTarget = CurrentTarget.Get();
	if (!BuildTargetEvaluation(currentTarget, OutSnapshot)) return false;

	OutSnapshot.TargetActor = currentTarget;
	return true;
}

// ===== Validation =====

bool UCTargetingComponent::ValidateRequiredReferences() const
{
	const FRequiredReference requiredReferences[] =
	{
		{ OwnerPlayerController_Injected, TEXT("APlayerController Owner") },
	};

	bool bValid = true;
	for (const FRequiredReference& reference : requiredReferences)
	{
		bValid &= FReferenceValidation::EnsureRequiredReference(reference.Object, reference.Label, OwnerPlayerController_Injected, this);
	}

	return bValid;
}

bool UCTargetingComponent::IsTargetValid(const ACEnemy* InTarget, bool bRequireViewCone) const
{
	FTargetingDebugSnapshot evaluation;
	if (!BuildTargetEvaluation(InTarget, evaluation)) return false;

	return IsTargetEvaluationValid(InTarget, evaluation, bRequireViewCone);
}

void UCTargetingComponent::ValidateCurrentTarget()
{
	ACEnemy* currentTarget = CurrentTarget.Get();
	if (!IsValid(currentTarget)) return;
	if (IsTargetValid(currentTarget, false)) return;

	ClearTarget();
}

// ===== Target Evaluation =====

bool UCTargetingComponent::BuildTargetEvaluation(const ACEnemy* InTarget, FTargetingDebugSnapshot& OutEvaluation) const
{
	OutEvaluation = FTargetingDebugSnapshot();
	if (!IsValid(OwnerPlayerController_Injected)) return false;

	FRotator viewRotation = FRotator::ZeroRotator;
	OwnerPlayerController_Injected->GetPlayerViewPoint(OutEvaluation.ViewLocation, viewRotation);

	OutEvaluation.ViewForward = viewRotation.Vector();
	OutEvaluation.MaxTargetDistance = TargetingTuning.MaxTargetDistance;
	if (!IsValid(InTarget)) return true;

	OutEvaluation.TargetLocation = InTarget->GetActorLocation();

	const float safeMaxTargetDistance = FMath::Max(OutEvaluation.MaxTargetDistance, KINDA_SMALL_NUMBER);
	OutEvaluation.Distance = FVector::Distance(OutEvaluation.ViewLocation, OutEvaluation.TargetLocation);
	OutEvaluation.DistanceScore = 1.f - FMath::Clamp(OutEvaluation.Distance / safeMaxTargetDistance, 0.f, 1.f);

	OutEvaluation.Dot = FVector::DotProduct(OutEvaluation.ViewForward, (OutEvaluation.TargetLocation - OutEvaluation.ViewLocation).GetSafeNormal());
	OutEvaluation.MinDot = FMath::Cos(FMath::DegreesToRadians(TargetingTuning.MaxTargetAngleDegrees));
	OutEvaluation.AngleScore = CalculateAngleScore(OutEvaluation.MinDot, OutEvaluation.Dot);

	OutEvaluation.FinalScore = (OutEvaluation.AngleScore * TargetingTuning.AngleScoreWeight) + (OutEvaluation.DistanceScore * TargetingTuning.DistanceScoreWeight);
	OutEvaluation.bWithinRange = OutEvaluation.Distance <= OutEvaluation.MaxTargetDistance;
	OutEvaluation.bWithinViewCone = OutEvaluation.Dot >= OutEvaluation.MinDot;
	return true;
}

bool UCTargetingComponent::IsTargetEvaluationValid(
	const ACEnemy* InTarget,
	const FTargetingDebugSnapshot& InEvaluation,
	bool bRequireViewCone) const
{
	if (!IsValid(InTarget)) return false;

	const UCHealthComponent* healthComp = InTarget->GetHealthComp();
	if (!IsValid(healthComp) || !healthComp->IsAlive()) return false;
	if (!InEvaluation.bWithinRange) return false;

	return !bRequireViewCone || InEvaluation.bWithinViewCone;
}

// ===== Candidate Selection =====

bool UCTargetingComponent::TryScoreTarget(const ACEnemy* InTarget, float& OutScore) const
{
	OutScore = 0.f;

	FTargetingDebugSnapshot evaluation;
	if (!BuildTargetEvaluation(InTarget, evaluation)) return false;
	if (!IsTargetEvaluationValid(InTarget, evaluation, true)) return false;

	OutScore = evaluation.FinalScore;
	return true;
}

bool UCTargetingComponent::ProjectTargetToViewport(const ACEnemy* InTarget, FVector2D& OutScreenPosition) const
{
	OutScreenPosition = FVector2D::ZeroVector;
	if (!IsValid(OwnerPlayerController_Injected) || !IsValid(InTarget)) return false;
	if (!OwnerPlayerController_Injected->ProjectWorldLocationToScreen(InTarget->GetActorLocation(), OutScreenPosition, true)) return false;

	int32 viewportSizeX = 0;
	int32 viewportSizeY = 0;
	OwnerPlayerController_Injected->GetViewportSize(viewportSizeX, viewportSizeY);
	if (viewportSizeX <= 0 || viewportSizeY <= 0) return false;

	return OutScreenPosition.X >= 0.f
		&& OutScreenPosition.X < static_cast<float>(viewportSizeX)
		&& OutScreenPosition.Y >= 0.f
		&& OutScreenPosition.Y < static_cast<float>(viewportSizeY);
}

// ===== Target Lifecycle =====

void UCTargetingComponent::BindTargetDestroyed(ACEnemy* InTarget)
{
	if (!IsValid(InTarget)) return;

	InTarget->OnDestroyed.AddUniqueDynamic(this, &UCTargetingComponent::HandleCurrentTargetDestroyed);
}

void UCTargetingComponent::UnbindTargetDestroyed(ACEnemy* InTarget)
{
	if (!IsValid(InTarget)) return;

	InTarget->OnDestroyed.RemoveDynamic(this, &UCTargetingComponent::HandleCurrentTargetDestroyed);
}

void UCTargetingComponent::HandleCurrentTargetDestroyed(AActor* InDestroyedActor)
{
	ACEnemy* destroyedTarget = Cast<ACEnemy>(InDestroyedActor);
	if (!destroyedTarget) return;

	CurrentTarget.Reset();
	ValidationElapsedTime = 0.f;
	OnTargetChanged.Broadcast(destroyedTarget, nullptr);
}

// ===== Target State =====

void UCTargetingComponent::SetCurrentTarget(ACEnemy* InNewTarget)
{
	ACEnemy* previousTarget = CurrentTarget.Get();
	if (previousTarget == InNewTarget) return;

	UnbindTargetDestroyed(previousTarget);
	CurrentTarget = InNewTarget;
	BindTargetDestroyed(InNewTarget);
	OnTargetChanged.Broadcast(previousTarget, InNewTarget);
}
