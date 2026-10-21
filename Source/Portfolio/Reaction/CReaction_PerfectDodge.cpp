#include "Reaction/CReaction_PerfectDodge.h"

#include "Component/CDefenseComponent.h"
#include "Component/CReactionComponent.h"
#include "Component/CHealthComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

void UCReaction_PerfectDodge::StopMontage(float InBlendOutTime)
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

bool UCReaction_PerfectDodge::Start(const FReactionData& InData)
{
	FReactionExecutionContext context;
	if (!IsValid(ReactionComp_Injected) || !ReactionComp_Injected->GetActiveReactionContext(context)
		|| !IsValid(DefenseComp_Injected)
		|| !DefenseComp_Injected->CanCommitPerfectDodge(context.CombatSignalResultSerial)) return false;
	return Super::Start(InData);
}

void UCReaction_PerfectDodge::InitializeReferences(const FCharacterComponentReferences& InReferences)
{
	Super::InitializeReferences(InReferences);
	DefenseComp_Injected = InReferences.DefenseComponent;
}

FExecutionDecisionResult UCReaction_PerfectDodge::ResolveExecutionDecision(const FExecutionDecisionQuery& InQuery) const
{
	FExecutionDecisionResult result;
	result.Decision = EExecutionDecision::Reject;
	if (!IsIncomingReactionType(InQuery, EReactionType::PerfectDodge) || InQuery.Snapshot.IsDead()
		|| !IsValid(DefenseComp_Injected)
		|| !DefenseComp_Injected->CanStartPerfectDodge(InQuery.IncomingPart.ReactionContext.CombatSignalResultSerial)
		|| !CanResolveExclusiveRelationship(InQuery)) return result;
	result.Decision = EExecutionDecision::Accept;
	result.Relationship = EExecutionRelationship::Exclusive;
	return result;
}

bool UCReaction_PerfectDodge::WantIntervention(const FExecutionInterventionQuery& InQuery) const
{
	return InQuery.IsValidMinimal() && IsIncomingReactionType(InQuery, EReactionType::PerfectDodge)
		&& InQuery.ActivePart.IsActionParticipant()
		&& InQuery.ActivePart.ActionContext.ActionDataKey.ActionType == EActionType::Dodge
		&& IsValid(DefenseComp_Injected)
		&& DefenseComp_Injected->CanStartPerfectDodge(InQuery.IncomingPart.ReactionContext.CombatSignalResultSerial);
}

bool UCReaction_PerfectDodge::AllowIntervention(const FExecutionInterventionQuery& InQuery) const
{
	if (InQuery.IsValidMinimal() && InQuery.IncomingPart.IsActionParticipant()
		&& InQuery.IncomingPart.ActionContext.ActionDataKey.ActionType == EActionType::CounterRush)
		return IsValid(DefenseComp_Injected) && DefenseComp_Injected->GetCounterRushTarget() != nullptr;
	return Super::AllowIntervention(InQuery);
}
