#include "Component/CPlayerTargetSelectionComponent.h"

#include "ProjectGlobal.h"

#include "Character/Enemy/CEnemy.h"
#include "Component/CCombatTargetComponent.h"
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

// ===== Construction =====

UCPlayerTargetSelectionComponent::UCPlayerTargetSelectionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

// ===== Component Reference =====

void UCPlayerTargetSelectionComponent::InitializeReferences(APlayerController* InOwnerPlayerController)
{
	OwnerPlayerController_Injected = InOwnerPlayerController;
	ValidateRequiredReferences();
}
void UCPlayerTargetSelectionComponent::SetCombatTargetComponent(UCCombatTargetComponent* InCombatTargetComponent)
{
	CombatTargetComponent_Injected = InCombatTargetComponent;
	CurrentTargetMaintenanceElapsedTime = 0.f;
}

bool UCPlayerTargetSelectionComponent::ValidateRequiredReferences() const
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

// ===== Lifecycle =====

void UCPlayerTargetSelectionComponent::EndPlay(const EEndPlayReason::Type InEndPlayReason)
{
	CombatTargetComponent_Injected = nullptr;
	CurrentTargetMaintenanceElapsedTime = 0.f;

	Super::EndPlay(InEndPlayReason);
}

void UCPlayerTargetSelectionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	CurrentTargetMaintenanceElapsedTime += FMath::Max(DeltaTime, 0.f);
	if (CurrentTargetMaintenanceElapsedTime < TargetingTuning.ValidationInterval) return;

	CurrentTargetMaintenanceElapsedTime = 0.f;
	MaintainCurrentTarget();
}

// ===== Player Target Selection Command =====

void UCPlayerTargetSelectionComponent::ToggleCombatTargetSelection()
{
	if (!IsValid(CombatTargetComponent_Injected)) return;

	if (IsValid(CombatTargetComponent_Injected) && CombatTargetComponent_Injected->HasCombatTarget())
	{
		ClearCombatTarget();
		return;
	}

	SelectBestTarget();
}

bool UCPlayerTargetSelectionComponent::SelectBestTarget()
{
	if (!IsValid(OwnerPlayerController_Injected)) return false;
	if (!IsValid(CombatTargetComponent_Injected)) return false;

	UWorld* world = GetWorld();
	if (!IsValid(world)) return false;

	ACEnemy* bestTarget = nullptr;
	float bestScore = -FLT_MAX;

	for (TActorIterator<ACEnemy> iterator(world); iterator; ++iterator)
	{
		ACEnemy* candidate = *iterator;
		float candidateScore = 0.f;

		if (!TryCalculateTargetScore(candidate, candidateScore)) continue;
		if (candidateScore <= bestScore) continue;

		bestTarget = candidate;
		bestScore = candidateScore;
	}

	if (!IsValid(bestTarget)) return false;

	return IsValid(CombatTargetComponent_Injected)
		&& CombatTargetComponent_Injected->RequestSetCombatTarget(bestTarget, ECombatTargetChangeReason::PlayerSelection);
}

bool UCPlayerTargetSelectionComponent::SelectAdjacentTarget(ETargetSwitchDirection InDirection)
{
	if (InDirection != ETargetSwitchDirection::Left && InDirection != ETargetSwitchDirection::Right) return false;
	if (!IsValid(OwnerPlayerController_Injected)) return false;
	if (!IsValid(CombatTargetComponent_Injected)) return false;

	ACEnemy* currentTarget = IsValid(CombatTargetComponent_Injected) ? Cast<ACEnemy>(CombatTargetComponent_Injected->GetCombatTargetActor()) : nullptr;
	if (!IsValid(currentTarget)) return SelectBestTarget();

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

		FTargetingEvaluation targetEvaluation;
		if (!BuildTargetEvaluation(candidate, targetEvaluation)) continue;
		if (!CanSelectTarget(candidate, targetEvaluation)) continue;

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

	return IsValid(CombatTargetComponent_Injected)
		&& CombatTargetComponent_Injected->RequestSetCombatTarget(bestCandidate.Target, ECombatTargetChangeReason::PlayerSelection);
}

void UCPlayerTargetSelectionComponent::ClearCombatTarget()
{
	if (!IsValid(CombatTargetComponent_Injected)) return;

	CombatTargetComponent_Injected->RequestClearCombatTarget(ECombatTargetChangeReason::ManualClear);
}

// ===== Debug Query =====

bool UCPlayerTargetSelectionComponent::BuildSelectionDebugSnapshot(FTargetingEvaluation& OutEvaluation) const
{
	ACEnemy* currentTarget = IsValid(CombatTargetComponent_Injected) ? Cast<ACEnemy>(CombatTargetComponent_Injected->GetCombatTargetActor()) : nullptr;
	if (!BuildTargetEvaluation(currentTarget, OutEvaluation)) return false;

	OutEvaluation.TargetActor = currentTarget;
	return true;
}

// ===== Current Target Maintenance =====

void UCPlayerTargetSelectionComponent::MaintainCurrentTarget()
{
	if (!IsValid(CombatTargetComponent_Injected)) return;

	ACEnemy* currentTarget = Cast<ACEnemy>(CombatTargetComponent_Injected->GetCombatTargetActor());
	if (!IsValid(currentTarget)) return;

	FTargetingEvaluation evaluation;
	if (BuildTargetEvaluation(currentTarget, evaluation) && CanRetainCombatTarget(currentTarget, evaluation)) return;

	CombatTargetComponent_Injected->RequestClearCombatTarget(ECombatTargetChangeReason::PolicyInvalidated);
}

// ===== Target Evaluation =====

bool UCPlayerTargetSelectionComponent::BuildTargetEvaluation(const ACEnemy* InTarget, FTargetingEvaluation& OutEvaluation) const
{
	OutEvaluation = FTargetingEvaluation();
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

// ===== Target Criteria =====

bool UCPlayerTargetSelectionComponent::CanSelectTarget(const ACEnemy* InTarget, const FTargetingEvaluation& InEvaluation) const
{
	return CanRetainCombatTarget(InTarget, InEvaluation) && InEvaluation.bWithinViewCone;
}

bool UCPlayerTargetSelectionComponent::CanRetainCombatTarget(const ACEnemy* InTarget, const FTargetingEvaluation& InEvaluation) const
{
	if (!IsValid(InTarget)) return false;

	const UCHealthComponent* healthComp = InTarget->GetHealthComp();
	if (!IsValid(healthComp) || !healthComp->IsAlive()) return false;
	if (!InEvaluation.bWithinRange) return false;

	return true;
}

// ===== Candidate Selection =====

bool UCPlayerTargetSelectionComponent::TryCalculateTargetScore(const ACEnemy* InTarget, float& OutScore) const
{
	OutScore = 0.f;

	FTargetingEvaluation evaluation;
	if (!BuildTargetEvaluation(InTarget, evaluation)) return false;
	if (!CanSelectTarget(InTarget, evaluation)) return false;

	OutScore = evaluation.FinalScore;
	return true;
}

bool UCPlayerTargetSelectionComponent::ProjectTargetToViewport(const ACEnemy* InTarget, FVector2D& OutScreenPosition) const
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
