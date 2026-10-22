#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/Composites/BTComposite_Selector.h"
#include "BehaviorTree/Composites/BTComposite_Sequence.h"
#include "BehaviorTree/Decorators/BTDecorator_Blackboard.h"
#include "BehaviorTree/Tasks/BTTask_Wait.h"
#include "Character/Enemy/CEnemy.h"
#include "Component/CActionComponent.h"
#include "Animation/AnimMontage.h"
#include "Type/CStateTypes.h"
#include "UObject/UnrealType.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAICoreAssetContractTest, "Portfolio.AI.Assets.CoreContracts",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAICoreAssetContractTest::RunTest(const FString&)
{
	UBehaviorTree* tree = LoadObject<UBehaviorTree>(nullptr, TEXT("/Game/02_Controller/02_Enemy/AI/BehaviorTree/BT_Default.BT_Default"));
	if (!TestNotNull(TEXT("Default behavior tree loads"), tree)) return false;
	UBTComposite_Sequence* root = Cast<UBTComposite_Sequence>(tree->RootNode);
	if (!TestNotNull(TEXT("Root sequence keeps context services active"), root)) return false;
	UBTComposite_Selector* router = Cast<UBTComposite_Selector>(root->GetChildNode(0));
	if (!TestNotNull(TEXT("State router exists"), router)) return false;
	const FIntProperty* enumValue = FindFProperty<FIntProperty>(UBTDecorator_Blackboard::StaticClass(), TEXT("IntValue"));
	if (!TestNotNull(TEXT("Blackboard enum storage"), enumValue)) return false;
	bool foundDead = false, foundHitReact = false;
	for (const FBTCompositeChild& child : router->Children)
	{
		for (const UBTDecorator* decorator : child.Decorators)
		{
			if (!Cast<UBTDecorator_Blackboard>(decorator)) continue;
			const FString name = decorator->GetNodeName();
			if (name == TEXT("AI Intent State_IsDead"))
			{
				foundDead = true;
				TestEqual(TEXT("Serialized Dead value matches native enum"), enumValue->GetPropertyValue_InContainer(decorator), int32(EAIIntentState::Dead));
			}
			if (name == TEXT("AI Intent State_IsHitReact"))
			{
				foundHitReact = true;
				TestEqual(TEXT("Serialized HitReact value matches native enum"), enumValue->GetPropertyValue_InContainer(decorator), int32(EAIIntentState::HitReact));
			}
		}
	}
	TestTrue(TEXT("Dead and HitReact routes are connected"), foundDead && foundHitReact);
	if (!TestTrue(TEXT("Router has a fallback child"), router->Children.Num() > 0)) return false;
	const FBTCompositeChild& fallback = router->Children.Last();
	TestNotNull(TEXT("Unconditional wait prevents service starvation"), Cast<UBTTask_Wait>(fallback.ChildTask));
	TestEqual(TEXT("Fallback is not gated by a decorator"), fallback.Decorators.Num(), 0);

	UClass* enemyClass = LoadClass<ACEnemy>(nullptr, TEXT("/Game/01_Character/02_Enemy/BP_CEnemy.BP_CEnemy_C"));
	if (!TestNotNull(TEXT("Enemy Blueprint loads"), enemyClass)) return false;
	UCActionComponent* action = enemyClass->GetDefaultObject<ACEnemy>()->FindComponentByClass<UCActionComponent>();
	if (!TestNotNull(TEXT("Enemy action template"), action)) return false;
	const FArrayProperty* property = FindFProperty<FArrayProperty>(action->GetClass(), TEXT("ActionDatas"));
	if (!TestNotNull(TEXT("Authored action data"), property)) return false;
	TSet<int32> indices;
	for (const FActionData& data : *property->ContainerPtrToValuePtr<TArray<FActionData>>(action))
	{
		if (data.ActionDataKey.ActionType != EActionType::ComboAttack) continue;
		indices.Add(data.ActionDataKey.ActionIndex);
		TestTrue(TEXT("Enemy combo enables start-facing"), data.bFaceCombatTargetOnStart);
		TestNotNull(TEXT("Enabling facing must not remove the authored montage"), data.Montage);
		if (data.Montage) TestTrue(TEXT("Enemy combo montage remains playable"), data.Montage->GetPlayLength() > 0.f);
	}
	TestTrue(TEXT("All three authored combo entries remain available"), indices.Num() == 3 && indices.Contains(0) && indices.Contains(1) && indices.Contains(2));
	return true;
}
#endif
