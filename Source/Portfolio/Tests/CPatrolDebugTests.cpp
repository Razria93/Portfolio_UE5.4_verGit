#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Misc/AutomationTest.h"
#include "Core/Debug/CPatrolDebugSubsystem.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BTCompositeNode.h"
#include "BehaviorTree/Tasks/BTTask_MoveTo.h"
#include "UObject/UnrealType.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPatrolDebugHistoryTest,"Portfolio.Debug.Patrol.History",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FPatrolDebugHistoryTest::RunTest(const FString& Parameters)
{
	FPatrolTrailHistory H;
	auto Sample=[&](double T,float X,bool Active=true){H.Update(FVector(X,0,0),T,Active,0.1f,10,60,600);};
	Sample(0,0); Sample(0.2,20); TestEqual(TEXT("Moving positions recorded"),H.Points.Num(),2);
	Sample(0.4,20); TestEqual(TEXT("Stationary position not duplicated"),H.Points.Num(),2);
	Sample(0.5,30,false); Sample(0.7,100); TestTrue(TEXT("Resume starts disconnected segment"),H.Points[1].Segment!=H.Points[2].Segment);
	Sample(0.9,1000); TestTrue(TEXT("Teleport starts disconnected segment"),H.Points[2].Segment!=H.Points[3].Segment);
	H.Update(FVector(1020,0,0),1.1,true,0.1,10,60,2); TestEqual(TEXT("History capped"),H.Points.Num(),2);
	H.Update(FVector::ZeroVector,70,false,0.1,10,60,600); TestEqual(TEXT("History expires while recording paused"),H.Points.Num(),0);
	Sample(71,0); H.Reset(); TestEqual(TEXT("Clear removes samples"),H.Points.Num(),0); TestFalse(TEXT("Clear ends segment"),H.bActive);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPatrolDebugTaskTest,"Portfolio.Debug.Patrol.TaskAndAsset",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FPatrolDebugTaskTest::RunTest(const FString& Parameters)
{
	UBTTask_MoveTo* Move=NewObject<UBTTask_MoveTo>();
	FStructProperty* Property=FindFProperty<FStructProperty>(Move->GetClass(),TEXT("BlackboardKey"));
	if (!TestNotNull(TEXT("MoveTo selector"),Property)) return false;
	auto* Key=Property->ContainerPtrToValuePtr<FBlackboardKeySelector>(Move);
	Key->SelectedKeyName=TEXT("PatrolLocation"); TestTrue(TEXT("Patrol destination is recognized"),UCPatrolDebugSubsystem::IsPatrolMoveTask(Move));
	Key->SelectedKeyName=TEXT("HomeLocation"); TestFalse(TEXT("ReturnHome excluded"),UCPatrolDebugSubsystem::IsPatrolMoveTask(Move));
	Key->SelectedKeyName=TEXT("TargetActor"); TestFalse(TEXT("Chase excluded"),UCPatrolDebugSubsystem::IsPatrolMoveTask(Move));
	TestFalse(TEXT("No active task excluded"),UCPatrolDebugSubsystem::IsPatrolMoveTask(nullptr));
	UBehaviorTree* Tree=LoadObject<UBehaviorTree>(nullptr,TEXT("/Game/02_Controller/02_Enemy/AI/BehaviorTree/State/BT_Idle.BT_Idle"));
	if (!TestNotNull(TEXT("Existing idle BT"),Tree)) return false;
	int32 PatrolMoves=0;
	TFunction<void(const UBTNode*)> Visit=[&](const UBTNode* Node)
	{
		if (const auto* Task=Cast<UBTTaskNode>(Node)) if (UCPatrolDebugSubsystem::IsPatrolMoveTask(Task))
        {
            ++PatrolMoves;
            const auto* PatrolMove=Cast<UBTTask_MoveTo>(Task);
            AddInfo(FString::Printf(TEXT("BT_Idle Patrol MoveTo: AcceptableRadius=%.2f, IncludesAgentRadius=%d, IncludesGoalRadius=%d, ProjectGoal=%d, PartialPath=%d"),
                PatrolMove->AcceptableRadius, PatrolMove->bReachTestIncludesAgentRadius, PatrolMove->bReachTestIncludesGoalRadius, PatrolMove->bProjectGoalLocation, PatrolMove->bAllowPartialPath));
        }
		if (const auto* Composite=Cast<UBTCompositeNode>(Node)) for(int32 I=0;I<Composite->GetChildrenNum();++I) Visit(Composite->GetChildNode(I));
	};
	Visit(Tree->RootNode);
	TestTrue(TEXT("BT_Idle uses a PatrolLocation MoveTo without asset changes"),PatrolMoves>0);
	AddInfo(FString::Printf(TEXT("Patrol MoveTo nodes in BT_Idle: %d"),PatrolMoves));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPatrolDebugEndpointTest,"Portfolio.Debug.Patrol.Endpoint",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FPatrolDebugEndpointTest::RunTest(const FString& Parameters)
{
    FPatrolTrailHistory H;
    H.Update(FVector(0,0,0),0,true,0.1,10,60,600);
    H.Update(FVector(20,0,0),0.2,true,0.1,10,60,600);
    H.Finish(FVector(23,0,0),0.21,600);
    TestEqual(TEXT("Endpoint survives distance and time thresholds"),H.Points.Num(),3);
    TestTrue(TEXT("Actual endpoint, not target, recorded"),H.Points.Last().Location.Equals(FVector(23,0,0)));
    TestEqual(TEXT("Endpoint belongs to completed segment"),H.Points[1].Segment,H.Points[2].Segment);
    H.Finish(FVector(40,0,0),0.22,600);
    TestEqual(TEXT("Repeated finish cannot append another behavior"),H.Points.Num(),3);
    H.Update(FVector(24,0,0),0.3,true,0.1,10,60,600);
    TestTrue(TEXT("Next patrol leg is disconnected"),H.Points[2].Segment!=H.Points[3].Segment);
    H.Update(FVector(27,0,0),0.31,true,0.1,10,60,600);
    H.Update(FVector(200,0,0),0.32,false,0.1,10,60,600);
    TestTrue(TEXT("Fallback flushes last observed patrol position, not Chase position"),H.Points.Last().Location.Equals(FVector(27,0,0)));
    H.Reset(); H.Update(FVector::ZeroVector,0,true,0.1,10,60,600);
    H.Finish(FVector(1000,0,0),0.02,600);
    TestTrue(TEXT("Teleport at finish does not create a false line"),H.Points[0].Segment!=H.Points[1].Segment);
    H.Reset(); H.Update(FVector::ZeroVector,0,true,0.1,10,60,2);
    H.Update(FVector(20,0,0),0.2,true,0.1,10,60,2); H.Finish(FVector(23,0,0),0.21,2);
    TestEqual(TEXT("Forced endpoint respects cap"),H.Points.Num(),2);
    H.Reset(); H.Finish(FVector(1,0,0),0,600);
    TestEqual(TEXT("No artificial point when capture was not active"),H.Points.Num(),0);
    return true;
}

#endif
