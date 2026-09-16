#include "Notify/CAnimNotifyState_EquipSocketTransformTransition.h"

UCAnimNotifyState_EquipSocketTransformTransition::UCAnimNotifyState_EquipSocketTransformTransition()
{
	TriggerActionType = EActionType::Equip;
	TargetSlot = EWeaponSocketSlot::Hand;
	CompletionCommand = EActionNotifyCommand::CommitEquipWeapon;
}
