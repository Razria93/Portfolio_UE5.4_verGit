#include "Component/CTargetingComponent.h"

#include "ProjectGlobal.h"

#include "Character/Enemy/CEnemy.h"
#include "Component/CHealthComponent.h"

#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"

namespace
{
	float CalculateAngleScore(float InMinDot, float InDot)
	{
		const float scoreRange = 1.f - InMinDot;
		if (scoreRange <= SMALL_NUMBER)
		{
			return InDot >= InMinDot ? 1.f : 0.f;
		}

		return FMath::Clamp(FMath::GetRangePct(InMinDot, 1.f, InDot), 0.f, 1.f);
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
