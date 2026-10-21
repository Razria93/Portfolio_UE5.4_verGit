#include "Action/CAction_Dodge.h"

#include "ProjectGlobal.h"

#include "GameFramework/Character.h"
#include "Component/CHealthComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

void UCAction_Dodge::StopMontage(float InBlendOutTime)
{
	const auto* health = IsValid(OwnerCharacter_Injected) ? OwnerCharacter_Injected->FindComponentByClass<UCHealthComponent>() : nullptr;
	if (!IsValid(health) || !health->IsDead())
	{
		Super::StopMontage(InBlendOutTime);
		return;
	}

	// A death stop must not extract more dodge root motion during blend-out.
	USkeletalMeshComponent* mesh = OwnerCharacter_Injected->GetMesh();
	UAnimInstance* anim = mesh ? mesh->GetAnimInstance() : nullptr;
	if (FAnimMontageInstance* instance = anim ? anim->GetMontageInstanceForID(ActiveMontageInstanceId) : nullptr)
		instance->Stop(FAlphaBlend(0.f), true);
	if (mesh) mesh->ConsumeRootMotion();

	if (UCharacterMovementComponent* movement = OwnerCharacter_Injected->GetCharacterMovement())
	{
		movement->RootMotionParams.Clear();
		movement->Velocity.X = 0.f;
		movement->Velocity.Y = 0.f;
	}
}

// Decision

FExecutionDecisionResult UCAction_Dodge::ResolveExecutionDecision(const FExecutionDecisionQuery& InQuery) const
{
	FExecutionDecisionResult result;

	if (!IsValid(OwnerCharacter_Injected) || InQuery.Snapshot.IsDead() || InQuery.Snapshot.IsInReaction())
	{
		result.Decision = EExecutionDecision::Reject;
		return result;
	}

	if (!IsIncomingActionType(InQuery, EActionType::Dodge))
	{
		result.Decision = EExecutionDecision::Reject;
		return result;
	}

	EExecutionRelationship relationship = EExecutionRelationship::None;

	if (!TryResolveIndependentOrExclusiveRelationship(InQuery, relationship))
	{
		result.Decision = EExecutionDecision::Reject;
		return result;
	}

	result.Decision = EExecutionDecision::Accept;
	result.Relationship = relationship;
	return result;
}

// Observable Overlay

void UCAction_Dodge::ResolveObservableOverlayCondition(const FObservableOverlayQuery& InQuery, FObservableOverlayExecutionDecision& OutDecision) const
{
	OutDecision = FObservableOverlayExecutionDecision();

	const bool bIsDodge = IsIncomingActionType(InQuery.DecisionQuery, EActionType::Dodge);
	if (!bIsDodge)
	{
		// Reject non-Dodge overlay queries.
		OutDecision.Decision = EExecutionDecision::Reject;
		return;
	}

	// Dodge clears an active Guard state before it starts.
	const bool bHasGuardState = InQuery.DecisionQuery.Snapshot.ObservableOverlay.Guard.HasGuardRuntimeState();
	if (bHasGuardState)
	{
		OutDecision.Decision = EExecutionDecision::Accept;
		OutDecision.Handlings.AddUnique(EObservableOverlayHandling::ClearGuardState);
		return;
	}

	// Dodge can start without overlay cleanup.
	OutDecision.Decision = EExecutionDecision::Accept;
}

// Intervention

bool UCAction_Dodge::WantIntervention(const FExecutionInterventionQuery& InQuery) const
{
	if (!InQuery.IsValidMinimal()) return false;
	if (!IsIncomingActionType(InQuery, EActionType::Dodge)) return false;
	if (InQuery.Snapshot.IsInReaction() || InQuery.ActivePart.IsReactionParticipant()) return false;

	if (Super::WantIntervention(InQuery)) return true;

	return InQuery.ActivePart.IsActionParticipant() && InQuery.StopReason == EExecutionStopReason::Interrupted;
}

bool UCAction_Dodge::AllowIntervention(const FExecutionInterventionQuery& InQuery) const
{
	if (InQuery.IsValidMinimal() && InQuery.IncomingPart.IsReactionParticipant()
		&& InQuery.IncomingPart.ReactionContext.ReactionDataKey.ReactionType == EReactionType::PerfectDodge)
		return true;
	return Super::AllowIntervention(InQuery);
}
