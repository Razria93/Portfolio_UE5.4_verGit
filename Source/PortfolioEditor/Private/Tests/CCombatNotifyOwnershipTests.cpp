#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Notify/CCombatNotifyOwnership.h"
#include "Animation/ActiveMontageInstanceScope.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCombatNotifyOwnershipTest, "Portfolio.Combat.Dodge.NotifyInstanceOwnership",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCombatNotifyOwnershipTest::RunTest(const FString& Parameters)
{
	using namespace CCombatNotifyOwnership;
	FAnimNotifyEventReference oldExecution;
	oldExecution.AddContextData<UE::Anim::FAnimNotifyMontageInstanceContext>(41);
	FAnimNotifyEventReference newExecution;
	newExecution.AddContextData<UE::Anim::FAnimNotifyMontageInstanceContext>(42);
	FAnimNotifyEventReference withoutOwnership;
	TestTrue(TEXT("Original execution accepts its queued notification"), MatchesMontageInstance(oldExecution, 41));
	TestFalse(TEXT("Old completion/end cannot mutate successor of same montage"), MatchesMontageInstance(oldExecution, 42));
	TestTrue(TEXT("Successor accepts its own notification"), MatchesMontageInstance(newExecution, 42));
	TestFalse(TEXT("Cleared execution rejects remaining notification"), MatchesMontageInstance(newExecution, INDEX_NONE));
	TestFalse(TEXT("Scoped defensive execution fails closed without instance context"), MatchesMontageInstance(withoutOwnership, 42));
	TestFalse(TEXT("Missing context cannot match cleared execution"), MatchesMontageInstance(withoutOwnership, INDEX_NONE));
	return true;
}
#endif
