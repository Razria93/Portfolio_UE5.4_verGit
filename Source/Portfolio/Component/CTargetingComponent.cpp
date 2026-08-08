#include "Component/CTargetingComponent.h"

#include "ProjectGlobal.h"

#include "Character/Enemy/CEnemy.h"
#include "Component/CHealthComponent.h"

#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"

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

	ACEnemy* bestTarget = nullptr;
	float bestScore = -FLT_MAX;

	for (TActorIterator<ACEnemy> iterator(GetWorld()); iterator; ++iterator)
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
	OutSnapshot = FTargetingDebugSnapshot();

	ACEnemy* currentTarget = CurrentTarget.Get();
	if (!IsValid(OwnerPlayerController_Injected)) return false;

	FRotator viewRotation = FRotator::ZeroRotator;
	OwnerPlayerController_Injected->GetPlayerViewPoint(OutSnapshot.ViewLocation, viewRotation);

	OutSnapshot.ViewForward = viewRotation.Vector();
	OutSnapshot.MaxTargetDistance = TargetingTuning.MaxTargetDistance;
	if (!IsValid(currentTarget)) return true;

	OutSnapshot.TargetActor = currentTarget;
	OutSnapshot.TargetLocation = currentTarget->GetActorLocation();
	const float safeMaxTargetDistance = FMath::Max(OutSnapshot.MaxTargetDistance, KINDA_SMALL_NUMBER);
	OutSnapshot.Distance = FVector::Distance(OutSnapshot.ViewLocation, OutSnapshot.TargetLocation);
	OutSnapshot.Dot = FVector::DotProduct(
		OutSnapshot.ViewForward,
		(OutSnapshot.TargetLocation - OutSnapshot.ViewLocation).GetSafeNormal());
	OutSnapshot.MinDot = FMath::Cos(FMath::DegreesToRadians(TargetingTuning.MaxTargetAngleDegrees));
	OutSnapshot.AngleScore = FMath::Clamp(FMath::GetRangePct(OutSnapshot.MinDot, 1.f, OutSnapshot.Dot), 0.f, 1.f);
	OutSnapshot.DistanceScore = 1.f - FMath::Clamp(OutSnapshot.Distance / safeMaxTargetDistance, 0.f, 1.f);
	OutSnapshot.FinalScore = (OutSnapshot.AngleScore * TargetingTuning.AngleScoreWeight)
		+ (OutSnapshot.DistanceScore * TargetingTuning.DistanceScoreWeight);
	OutSnapshot.bWithinRange = OutSnapshot.Distance <= TargetingTuning.MaxTargetDistance;
	OutSnapshot.bWithinViewCone = OutSnapshot.Dot >= OutSnapshot.MinDot;
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
	if (!IsValid(OwnerPlayerController_Injected)) return false;
	if (!IsValid(InTarget)) return false;

	const UCHealthComponent* healthComp = InTarget->GetHealthComp();
	if (!IsValid(healthComp) || !healthComp->IsAlive()) return false;

	FVector viewLocation = FVector::ZeroVector;
	FRotator viewRotation = FRotator::ZeroRotator;
	OwnerPlayerController_Injected->GetPlayerViewPoint(viewLocation, viewRotation);

	const float distance = FVector::Distance(viewLocation, InTarget->GetActorLocation());
	if (distance > TargetingTuning.MaxTargetDistance) return false;
	if (!bRequireViewCone) return true;

	const FVector directionToTarget = (InTarget->GetActorLocation() - viewLocation).GetSafeNormal();
	const float minDot = FMath::Cos(FMath::DegreesToRadians(TargetingTuning.MaxTargetAngleDegrees));
	return FVector::DotProduct(viewRotation.Vector(), directionToTarget) >= minDot;
}

void UCTargetingComponent::ValidateCurrentTarget()
{
	ACEnemy* currentTarget = CurrentTarget.Get();
	if (!IsValid(currentTarget)) return;
	if (IsTargetValid(currentTarget, false)) return;

	ClearTarget();
}

// ===== Candidate Selection =====

bool UCTargetingComponent::TryScoreTarget(const ACEnemy* InTarget, float& OutScore) const
{
	OutScore = 0.f;
	if (!IsTargetValid(InTarget, true)) return false;

	FVector viewLocation = FVector::ZeroVector;
	FRotator viewRotation = FRotator::ZeroRotator;
	OwnerPlayerController_Injected->GetPlayerViewPoint(viewLocation, viewRotation);

	const FVector directionToTarget = (InTarget->GetActorLocation() - viewLocation).GetSafeNormal();
	const float dot = FVector::DotProduct(viewRotation.Vector(), directionToTarget);
	const float minDot = FMath::Cos(FMath::DegreesToRadians(TargetingTuning.MaxTargetAngleDegrees));
	const float angleScore = FMath::GetRangePct(minDot, 1.f, dot);
	const float distance = FVector::Distance(viewLocation, InTarget->GetActorLocation());
	const float distanceScore = 1.f - FMath::Clamp(distance / TargetingTuning.MaxTargetDistance, 0.f, 1.f);

	OutScore = (angleScore * TargetingTuning.AngleScoreWeight) + (distanceScore * TargetingTuning.DistanceScoreWeight);
	return true;
}

// ===== Target State =====

void UCTargetingComponent::SetCurrentTarget(ACEnemy* InNewTarget)
{
	ACEnemy* previousTarget = CurrentTarget.Get();
	if (previousTarget == InNewTarget) return;

	CurrentTarget = InNewTarget;
	OnTargetChanged.Broadcast(previousTarget, InNewTarget);
}
