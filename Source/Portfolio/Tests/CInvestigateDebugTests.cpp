#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Misc/AutomationTest.h"
#include "Core/Debug/FInvestigateDebug.h"
#include "Character/Enemy/CEnemy.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "Components/CapsuleComponent.h"
#include "Components/LineBatchComponent.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "Misc/ScopeExit.h"

namespace
{
bool CheckInvestigateAnchor(FAutomationTestBase& Test, const FVector& Anchor, bool InitialGoal)
{
    IConsoleVariable* Enabled = IConsoleManager::Get().FindConsoleVariable(TEXT("Portfolio.DebugOverlay.Investigate.Enabled"));
    if (!Test.TestNotNull(TEXT("Investigate toggle"), Enabled)) return false;
    const int32 SavedValue = Enabled->GetInt();
    const auto Priority = static_cast<EConsoleVariableFlags>(Enabled->GetFlags() & ECVF_SetByMask);
    Enabled->Set(1, Priority);
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
    ON_SCOPE_EXIT
    {
        FInvestigateDebug::Clear(World);
        if (World) World->DestroyWorld(false);
        Enabled->Set(SavedValue, Priority);
    };
    if (!Test.TestNotNull(TEXT("Test world"), World)) return false;
    if (!Test.TestNotNull(TEXT("Test AI system"), World->CreateAISystem())) return false;
    ACEnemy* Enemy = World->SpawnActor<ACEnemy>();
    AAIController* Controller = World->SpawnActor<AAIController>();
    if (!Test.TestNotNull(TEXT("Observer"), Enemy) || !Test.TestNotNull(TEXT("Controller"), Controller)) return false;
    Controller->Possess(Enemy);
    UBlackboardData* Data = NewObject<UBlackboardData>(World);
    for (const FName Name : {FName(TEXT("LastKnownLocation")), FName(TEXT("InvestigateLocation")), FName(TEXT("bIsInvestigating"))})
    {
        FBlackboardEntry Entry;
        Entry.EntryName = Name;
        Entry.KeyType = Name == TEXT("bIsInvestigating")
            ? static_cast<UBlackboardKeyType*>(NewObject<UBlackboardKeyType_Bool>(Data))
            : static_cast<UBlackboardKeyType*>(NewObject<UBlackboardKeyType_Vector>(Data));
        Data->Keys.Add(Entry);
    }
    UBlackboardComponent* BB = nullptr;
    if (!Test.TestTrue(TEXT("Fixture blackboard"), Controller->UseBlackboard(Data, BB))) return false;
    if (!Test.TestTrue(TEXT("Blackboard initialized"), BB && BB->GetBlackboardAsset() == Data)) return false;
    if (!Test.TestNotNull(TEXT("Debug geometry batch"), World->LineBatcher.Get())) return false;
    const FVector Goal = InitialGoal ? Anchor : Anchor + FVector(300, 0, 0);
    BB->SetValueAsBool(TEXT("bIsInvestigating"), true);
    BB->SetValueAsVector(TEXT("LastKnownLocation"), Anchor);
    BB->SetValueAsVector(TEXT("InvestigateLocation"), Goal);
    // Both source coordinate conventions must be independent of the observing enemy's size.
    for (const float HalfHeight : {88.f, 160.f})
    {
        Enemy->GetCapsuleComponent()->SetCapsuleHalfHeight(HalfHeight);
        World->LineBatcher->Flush();
        FInvestigateDebug::Draw(World, Enemy);
        int32 AnchorLines = 0;
        FVector Center = FVector::ZeroVector;
        for (const FBatchedLine& Line : World->LineBatcher->BatchedLines)
        {
            if (!Line.Color.Equals(FLinearColor(FColor(255, 190, 60)))) continue;
            ++AnchorLines;
            Center += Line.Start;
            Test.TestTrue(TEXT("Anchor circle remains above the observed source height"),
                FMath::IsNearlyEqual(Line.Start.Z, Anchor.Z + 10., 0.01) && FMath::IsNearlyEqual(Line.End.Z, Anchor.Z + 10., 0.01));
        }
        Test.TestEqual(TEXT("Last Seen circle exists even without a separate selected goal"), AnchorLines, 32);
        if (AnchorLines) Test.TestTrue(TEXT("Circle preserves observed XY"), (Center / AnchorLines).Equals(Anchor + FVector(0, 0, 10), 0.01));
        Test.TestTrue(TEXT("Observation preserves source"), BB->GetValueAsVector(TEXT("LastKnownLocation")).Equals(Anchor));
        Test.TestTrue(TEXT("Observation preserves destination"), BB->GetValueAsVector(TEXT("InvestigateLocation")).Equals(Goal));
        Test.TestTrue(TEXT("Observation preserves search state"), BB->GetValueAsBool(TEXT("bIsInvestigating")));
    }
    BB->SetValueAsBool(TEXT("bIsInvestigating"), false);
    FInvestigateDebug::Clear(World);
    World->LineBatcher->Flush();
    FInvestigateDebug::Draw(World, Enemy);
    Test.TestEqual(TEXT("Ended search clears geometry history"), World->LineBatcher->BatchedLines.Num(), 0);
    Test.TestTrue(TEXT("Clear preserves destination"), BB->GetValueAsVector(TEXT("InvestigateLocation")).Equals(Goal));
    return true;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInvestigateFloorAnchorTest, "Portfolio.Debug.Investigate.FloorAnchor", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FInvestigateFloorAnchorTest::RunTest(const FString& Parameters)
{
    return CheckInvestigateAnchor(*this, FVector(200, 300, 0), false);
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInvestigateActorAnchorTest, "Portfolio.Debug.Investigate.ActorCenterAnchor", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FInvestigateActorAnchorTest::RunTest(const FString& Parameters)
{
    return CheckInvestigateAnchor(*this, FVector(200, 300, 96), false);
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInvestigateInitialGoalTest, "Portfolio.Debug.Investigate.InitialGoalEqualsAnchor", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FInvestigateInitialGoalTest::RunTest(const FString& Parameters)
{
    return CheckInvestigateAnchor(*this, FVector(200, 300, 0), true);
}
#endif
