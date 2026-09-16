#include "Notify/CAnimNotifyState_UnequipSocketTransformTransition.h"

UCAnimNotifyState_UnequipSocketTransformTransition::UCAnimNotifyState_UnequipSocketTransformTransition()
{
	TriggerActionType = EActionType::Unequip;
	TargetSlot = EWeaponSocketSlot::Holster;
	CompletionCommand = EActionNotifyCommand::UnequipSocketTransformTransition;
}
