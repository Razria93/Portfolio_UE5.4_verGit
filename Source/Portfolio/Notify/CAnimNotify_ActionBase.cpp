#include "Notify/CAnimNotify_ActionBase.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CActionComponent.h"
#include "Core/Debug/FAnimNotifyDebug.h"

UCAnimNotify_ActionBase::UCAnimNotify_ActionBase()
{
}

bool UCAnimNotify_ActionBase::CanProcessActionNotify(const UCActionComponent* InActionComp) const
{
	if (!IsValid(InActionComp)) return false;
	if (!InActionComp->IsActive()) return false;

	if (TriggerActionType == EActionType::None || TriggerActionType == EActionType::Max)
	{
		FAnimNotifyDebug::ReportActionNotifyTriggerWarning(this, InActionComp->GetOwner(), InActionComp, TriggerActionType, TriggerActionIndex, TEXT("InvalidTriggerActionType"));
		return false;
	}

	const EActionType actionType = InActionComp->GetActiveActionType();
	const int32 actionIndex = InActionComp->GetActiveActionIndex();

	if (TriggerActionType != EActionType::All && actionType != TriggerActionType) return false;
	if (TriggerActionIndex != INDEX_NONE && actionIndex != TriggerActionIndex) return false;

	return true;
}

UCActionComponent* UCAnimNotify_ActionBase::GetActionComponent(USkeletalMeshComponent* InMeshComp) const
{
	if (!IsValid(InMeshComp)) return nullptr;

	ACharacter* ownerCharacter = Cast<ACharacter>(InMeshComp->GetOwner());
	if (!IsValid(ownerCharacter)) return nullptr;

	return ownerCharacter->FindComponentByClass<UCActionComponent>();
}
