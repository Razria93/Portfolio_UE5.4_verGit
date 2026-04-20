#include "Component/CActionComponent.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Action/CAction.h"

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
	if (!CreateAction(OwnerCharacter_Cached, ActionType, ActionClass, ActionDatas))
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

UCAction* UCActionComponent::GetCurrentAction() const
{
	auto curActionPtr = ActionContainer.Find(CurrentActionType_Cached);
	if (!curActionPtr) return nullptr;

	UCAction* curAction = *curActionPtr;
	if (!IsValid(curAction)) return nullptr;

	return curAction;
}

bool UCActionComponent::TryStartAction(EActionType InActionType)
{
	if (!IsValid(OwnerCharacter_Cached)) return false;

	UCAction** actionPtr = ActionContainer.Find(InActionType);
	if (actionPtr == nullptr) return false;

	UCAction* action = *actionPtr;
	if (!IsValid(action)) return false;

	if (!action->PlayAction()) return false;

	ChangeActionType(InActionType);
	return true;
}

void UCActionComponent::ChangeActionType(EActionType InNewActionType)
{
	if (!IsValid(OwnerCharacter_Cached)) return;

	EActionType previousActionType = CurrentActionType_Cached;
	CurrentActionType_Cached = InNewActionType;

	if (OnActionTypeChanged.IsBound())
		OnActionTypeChanged.Broadcast(OwnerCharacter_Cached, previousActionType, CurrentActionType_Cached);
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