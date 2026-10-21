#include "Notify/CAnimNotify_CounterOpportunity.h"
#include "Notify/CCombatNotifyOwnership.h"
#include "Component/CDefenseComponent.h"
#include "Component/CReactionComponent.h"
#include "Reaction/CReaction.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"

FString UCAnimNotify_CounterOpportunity::GetNotifyName_Implementation() const
{
	return bOpen ? TEXT("Counter Opportunity Open") : TEXT("Counter Opportunity Close");
}

void UCAnimNotify_CounterOpportunity::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	auto* owner = MeshComp ? Cast<ACharacter>(MeshComp->GetOwner()) : nullptr;
	if (!owner || owner->GetMesh() != MeshComp) return;
	const auto* reaction = owner->FindComponentByClass<UCReactionComponent>();
	auto* defense = owner->FindComponentByClass<UCDefenseComponent>();
	if (!reaction || !defense || !reaction->IsActiveReactionType(EReactionType::PerfectDodge)
		|| !CCombatNotifyOwnership::AllowsReaction(reaction, EventReference)) return;
	const UCReaction* executor = reaction->GetActiveReactionExecutor();
	if (!IsValid(executor)) return;
	defense->SetCounterOpportunityWindow(bOpen, reaction->GetActiveReactionGeneration(), executor->GetActiveMontageInstanceId());
}
