#include "Notify/CCombatNotifyOwnership.h"
#include "Animation/ActiveMontageInstanceScope.h"
#include "Action/CAction.h"
#include "Reaction/CReaction.h"
#include "Component/CActionComponent.h"
#include "Component/CReactionComponent.h"

bool CCombatNotifyOwnership::MatchesMontageInstance(const FAnimNotifyEventReference& InReference, int32 InExpectedInstanceId)
{
	const auto* context = InReference.GetContextData<UE::Anim::FAnimNotifyMontageInstanceContext>();
	return InExpectedInstanceId != INDEX_NONE && context && context->MontageInstanceID == InExpectedInstanceId;
}

bool CCombatNotifyOwnership::AllowsAction(const UCActionComponent* InComponent, const FAnimNotifyEventReference& InReference)
{
	if (!IsValid(InComponent)) return false;
	const EActionType type = InComponent->GetActiveActionType();
	if (type != EActionType::Dodge && type != EActionType::CounterRush) return true;
	const UCAction* executor = InComponent->GetActiveActionExecutor();
	return IsValid(executor) && executor->IsActive()
		&& MatchesMontageInstance(InReference, executor->GetActiveMontageInstanceId());
}

bool CCombatNotifyOwnership::AllowsReaction(const UCReactionComponent* InComponent, const FAnimNotifyEventReference& InReference)
{
	if (!IsValid(InComponent)) return false;
	if (InComponent->GetActiveReactionType() != EReactionType::PerfectDodge) return true;
	const UCReaction* executor = InComponent->GetActiveReactionExecutor();
	return IsValid(executor) && executor->IsActive()
		&& MatchesMontageInstance(InReference, executor->GetActiveMontageInstanceId());
}
