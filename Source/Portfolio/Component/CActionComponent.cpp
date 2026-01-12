#include "Component/CActionComponent.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Weapon/CAction.h"

#include "Type/CWeaponStructure.h"

UCActionComponent::UCActionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCActionComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter_Cached = Cast<ACharacter>(GetOwner());
	check(OwnerCharacter_Cached);

	// TODO: Refactor 'for-each'
	if(!CreateAction(OwnerCharacter_Cached, ActionType, ActionClass, ActionDatas))
	{
		ensureMsgf(false, TEXT("UCActionComponent: CreateAction is failed (%d)."), (int32)ActionType);
	}

	CurrentActionType_Cached = EActionType::Idle;
}

void UCActionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	for (TPair<EActionType, UCAction*>& Pair : ActionContainer)
	{
		UCAction* Action = Pair.Value;

		if (!IsValid(Action)) continue;

		Action->Tick(DeltaTime);
	}
}

UObject* UCActionComponent::GetAction(EActionType InNewActionType)
{
	UCAction** actionPtr = ActionContainer.Find(InNewActionType);

	if (!actionPtr) return nullptr;

	UCAction* action = *actionPtr;
	if (!IsValid(action)) return nullptr;

	return action;
}

void UCActionComponent::SetIdleMode()
{
	ChangeActionMode(EActionType::Idle);
}

void UCActionComponent::SetComboAttackMode()
{
	ChangeActionMode(EActionType::ComboAttack);
}

void UCActionComponent::ChangeActionMode(EActionType InNewActionType)
{
	if (!IsValid(OwnerCharacter_Cached)) return;

	UCAction** actionPtr = ActionContainer.Find(InNewActionType);
	if (actionPtr == nullptr) return;

	UCAction* action = *actionPtr;
	if (action == nullptr) return;

	action->PlayAction();

	ChangeActionType(InNewActionType);
}

void UCActionComponent::ChangeActionType(EActionType InNewActionType)
{
	if (!IsValid(OwnerCharacter_Cached)) return;

	EActionType prevActionType = CurrentActionType_Cached;
	CurrentActionType_Cached = InNewActionType;

	if (OnActionTypeChanged.IsBound())
		OnActionTypeChanged.Broadcast(OwnerCharacter_Cached, prevActionType, CurrentActionType_Cached);
}

bool UCActionComponent::CreateAction(AActor* InOwnerCharacter, EActionType InActionType, TSubclassOf<UCAction> InActionClass, const TArray<FActionData> InActionDatas)
{
	if (!IsValid(InOwnerCharacter)) return false;

	if (!ensureMsgf(*InActionClass, TEXT("UCActionComponent: InActionClass is not set.")))
		return false;

	if (ActionContainer.Contains(InActionType))
	{
		ensureMsgf(false, TEXT("UCActionComponent: InActionType already exists (%d)."), (int32)InActionType);
		return false;
	}

	UCAction* action = NewObject<UCAction>(this, InActionClass);

	if (!ensureMsgf(IsValid(action), TEXT("UCActionComponent: Action was not created")))
		return false;

	ACharacter* character = Cast<ACharacter>(InOwnerCharacter);

	if (!ensureMsgf(IsValid(character), TEXT("UCActionComponent:InOwnerCharacter cast failed.")))
		return false;

	action->InitializeAction(character, InActionType, InActionDatas);

	ActionContainer.Add(InActionType, action);

	return true;
}