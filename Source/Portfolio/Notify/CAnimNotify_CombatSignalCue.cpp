#include "Notify/CAnimNotify_CombatSignalCue.h"
#include "ProjectGlobal.h"

#include "AIController.h"
#include "GameFramework/Character.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "Component/CActionComponent.h"
#include "Component/CCombatSignalSourceComponent.h"

#include "AI/Blackboard/CAIKey.h"

UCAnimNotify_CombatSignalCue::UCAnimNotify_CombatSignalCue()
{
}

FString UCAnimNotify_CombatSignalCue::GetNotifyName_Implementation() const
{
	return CueTag.IsNone() ? TEXT("None") : FString::Printf(TEXT("CombatSignalCue: %s"), *CueTag.ToString());
}

void UCAnimNotify_CombatSignalCue::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	UCActionComponent* actionComp = GetActionComponent(MeshComp);
	if (!CanProcessActionNotify(actionComp)) return;

	if (CueTag.IsNone())
	{
		FLog::Log(TEXT("[CombatSignalCueNotify] Rejected | Reason=InvalidCueTag"));
		return;
	}

	ACharacter* ownerCharacter = IsValid(MeshComp) ? Cast<ACharacter>(MeshComp->GetOwner()) : nullptr;
	if (!IsValid(ownerCharacter))
	{
		FLog::Log(TEXT("[CombatSignalCueNotify] Rejected | Reason=InvalidOwnerCharacter"));
		return;
	}

	UCCombatSignalSourceComponent* combatSignalSourceComp = ownerCharacter->FindComponentByClass<UCCombatSignalSourceComponent>();
	if (!IsValid(combatSignalSourceComp))
	{
		FLog::Log(FString::Printf(
			TEXT("[CombatSignalCueNotify] Rejected | Owner=%s | Reason=InvalidCombatSignalSourceComponent"),
			*GetNameSafe(ownerCharacter)));
		return;
	}

	AActor* targetActor = ResolveCueTargetActor(ownerCharacter);
	if (!IsValid(targetActor))
	{
		FLog::Log(FString::Printf(
			TEXT("[CombatSignalCueNotify] Rejected | Owner=%s | CueTag=%s | Reason=InvalidTargetActor"),
			*GetNameSafe(ownerCharacter),
			*CueTag.ToString()));
		return;
	}

	const FVector cueLocation = ownerCharacter->GetActorLocation();
	const FVector cueDirection = (targetActor->GetActorLocation() - ownerCharacter->GetActorLocation()).GetSafeNormal();

	const bool bSent = combatSignalSourceComp->RequestCombatSignalCue(targetActor, CueTag, cueLocation, cueDirection, ownerCharacter);

	FLog::Log(FString::Printf(
		TEXT("[CombatSignalCueNotify] %s | Source=%s | Target=%s | CueTag=%s"),
		bSent ? TEXT("Sent") : TEXT("Rejected"),
		*GetNameSafe(ownerCharacter),
		*GetNameSafe(targetActor),
		*CueTag.ToString()));
}

AActor* UCAnimNotify_CombatSignalCue::ResolveCueTargetActor(const ACharacter* InOwnerCharacter) const
{
	if (!IsValid(InOwnerCharacter)) return nullptr;

	const AAIController* aiController = Cast<AAIController>(InOwnerCharacter->GetController());
	if (!IsValid(aiController)) return nullptr;

	const UBlackboardComponent* blackboardComp = aiController->GetBlackboardComponent();
	if (!IsValid(blackboardComp)) return nullptr;

	return Cast<AActor>(blackboardComp->GetValueAsObject(CAIKey::Targeting::TargetActor));
}
