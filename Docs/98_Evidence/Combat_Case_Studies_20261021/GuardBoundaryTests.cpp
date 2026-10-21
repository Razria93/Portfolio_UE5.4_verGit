// Investigation test added 2026-10-21 in an isolated source snapshot.
// Real production defense/movement components; public guard lifecycle callbacks.
// No gameplay montage, actual collision, or real Hit executor is simulated.
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UObject/UnrealType.h"
#include "Component/CDefenseComponent.h"
#include "Component/CMovementComponent.h"
#include "Component/CStateComponent.h"
#include "Component/CHealthComponent.h"
#include "Component/CObservableOverlayComponent.h"
#include "Reaction/CReaction_Hit.h"
#include "Animation/AnimMontage.h"

namespace GuardBoundary
{
struct FFixture
{
    UWorld* World;
    ACharacter* Character;
    UCDefenseComponent* Defense;
    UCMovementComponent* Movement;
    FFixture()
    {
        World=UWorld::CreateWorld(EWorldType::Game,false);
        Character=World->SpawnActor<ACharacter>();
        Defense=NewObject<UCDefenseComponent>(Character);
        Movement=NewObject<UCMovementComponent>(Character);
        UCHealthComponent* Health=NewObject<UCHealthComponent>(Character);
        UCStateComponent* State=NewObject<UCStateComponent>(Character);
        Health->InitializeHealth(100.f,100.f,EMaxHPUpdatePolicy::ClampCurrent);
        FCharacterComponentReferences R;
        R.OwnerCharacter=Character;R.CharacterMovementComponent=Character->GetCharacterMovement();
        R.MovementComponent=Movement;R.DefenseComponent=Defense;R.HealthComponent=Health;R.StateComponent=State;
        Movement->InitializeReferences(R);Defense->InitializeReferences(R);
        // Fixture-only speed configuration, equivalent to Blueprint defaults.
        FProperty* Prop=FindFProperty<FProperty>(Movement->GetClass(),TEXT("GaitSpeedMap"));
        auto& Speeds=*Prop->ContainerPtrToValuePtr<TMap<EMovementGait,float>>(Movement);
        Speeds.Add(EMovementGait::Walk,180.f);Speeds.Add(EMovementGait::Run,420.f);Speeds.Add(EMovementGait::Sprint,620.f);
    }
    ~FFixture(){World->DestroyWorld(false);}
    void Begin(int Phase)
    {
        if(Phase==0)return;
        Defense->HandleGuardInputPressed();Defense->HandleGuardInStarted();
        if(Phase>=2)Defense->HandleSwitchToGuard();
        if(Phase>=3){Defense->HandleGuardInputReleased();Defense->HandleGuardOutStarted();}
    }
};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGuardBoundaryRepeatedCleanup,
    "PortfolioVerification.Guard.RepeatedCleanupAndMovement",
    EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FGuardBoundaryRepeatedCleanup::RunTest(const FString& Parameters)
{
    using namespace GuardBoundary;
    int Cases=0;
    for(int Gait=0;Gait<2;++Gait)for(int Phase=0;Phase<4;++Phase)for(int Terminal=0;Terminal<3;++Terminal)
    {
        FFixture F;
        if(Gait)F.Movement->HandleSprintInput();else F.Movement->HandleRunInput();
        const EMovementGait Expected=Gait?EMovementGait::Sprint:EMovementGait::Run;
        const float Speed=Gait?620.f:420.f;
        TestTrue(TEXT("Fixture movement accepts input"),F.Movement->CanAcceptMovementIntent());
        TestEqual(TEXT("Fixture baseline gait"),F.Movement->GetCurrentMovementGait(),Expected);
        F.Begin(Phase);
        if(Phase==1||Phase==2)
        {
            TestEqual(TEXT("Active guard sets walk"),F.Movement->GetCurrentMovementGait(),EMovementGait::Walk);
            TestEqual(TEXT("Active guard sets walk speed"),F.Character->GetCharacterMovement()->MaxWalkSpeed,180.f);
        }
        if(Terminal==1)F.Defense->HandleGuardLifecycleCompleted();
        if(Terminal==2)F.Defense->HandleGuardLifecycleInterrupted();
        for(int Repeat=0;Repeat<25;++Repeat)
        {
            TestTrue(TEXT("Repeated clear accepted"),F.Defense->CanApplyOverlayHandling(EObservableOverlayHandling::ClearGuardState));
            TestTrue(TEXT("Repeated clear executes"),F.Defense->ApplyOverlayHandling(EObservableOverlayHandling::ClearGuardState));
            TestFalse(TEXT("No guard runtime survives clear"),F.Defense->HasGuardRuntimeState());
            TestTrue(TEXT("Next guard permitted"),F.Defense->CanStartGuard());
            TestFalse(TEXT("No intent"),F.Defense->WantsGuarding());
            TestFalse(TEXT("No pose"),F.Defense->IsGuardingPose());
            TestFalse(TEXT("No guard window"),F.Defense->CanGuard());
            TestFalse(TEXT("No parry window"),F.Defense->CanParry());
            TestEqual(TEXT("Prior gait restored"),F.Movement->GetCurrentMovementGait(),Expected);
            TestEqual(TEXT("Prior speed restored"),F.Character->GetCharacterMovement()->MaxWalkSpeed,Speed);
        }
        F.Begin(1);
        TestTrue(TEXT("Following cycle starts pose"),F.Defense->IsGuardingPose());
        TestTrue(TEXT("Following cycle opens parry"),F.Defense->CanParry());
        TestEqual(TEXT("Following cycle applies walk"),F.Movement->GetCurrentMovementGait(),EMovementGait::Walk);
        F.Defense->HandleGuardLifecycleInterrupted();
        TestFalse(TEXT("Following cycle clears"),F.Defense->HasGuardRuntimeState());
        TestEqual(TEXT("Following cycle restores gait"),F.Movement->GetCurrentMovementGait(),Expected);
        ++Cases;
        AddInfo(FString::Printf(TEXT("[GuardBoundary] Case=%d Phase=%d Terminal=%d Baseline=%s Repeats=25 FinalRuntime=%d FinalSpeed=%.0f"),Cases,Phase,Terminal,Gait?TEXT("Sprint"):TEXT("Run"),F.Defense->HasGuardRuntimeState(),F.Character->GetCharacterMovement()->MaxWalkSpeed));
    }
    TestEqual(TEXT("Matrix contains 24 cases"),Cases,24);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGuardBoundaryOverlayScope,
    "PortfolioVerification.Guard.OverlayScopeAndRestore",
    EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FGuardBoundaryOverlayScope::RunTest(const FString& Parameters)
{
    using namespace GuardBoundary;
    for(int Phase=1;Phase<=2;++Phase)
    {
        FFixture F;F.Movement->HandleSprintInput();F.Begin(Phase);
        for(int Repeat=0;Repeat<25;++Repeat)
        {
            TestTrue(TEXT("Overlay clear accepted"),F.Defense->CanApplyOverlayHandling(EObservableOverlayHandling::ClearGuardOverlay));
            TestTrue(TEXT("Overlay clear executes"),F.Defense->ApplyOverlayHandling(EObservableOverlayHandling::ClearGuardOverlay));
            TestTrue(TEXT("Overlay clear preserves held intent"),F.Defense->WantsGuarding());
            TestFalse(TEXT("Overlay clear does not unlock guard start"),F.Defense->CanStartGuard());
            TestFalse(TEXT("Overlay bits cleared"),F.Defense->HasGuardOverlay());
            TestEqual(TEXT("Overlay clear restores sprint"),F.Movement->GetCurrentMovementGait(),EMovementGait::Sprint);
            TestEqual(TEXT("Overlay clear restores speed"),F.Character->GetCharacterMovement()->MaxWalkSpeed,620.f);
        }
        F.Defense->RestoreGuardOverlay();
        TestTrue(TEXT("Held intent restores pose"),F.Defense->IsGuardingPose());
        TestTrue(TEXT("Held intent restores guard window"),F.Defense->CanGuard());
        TestFalse(TEXT("Restore does not reopen parry"),F.Defense->CanParry());
        TestEqual(TEXT("Restored overlay walks"),F.Movement->GetCurrentMovementGait(),EMovementGait::Walk);
        F.Defense->ClearGuardState();F.Defense->RestoreGuardOverlay();
        TestFalse(TEXT("Cleared intent cannot restore overlay"),F.Defense->HasGuardRuntimeState());
        TestEqual(TEXT("Full clear restores sprint"),F.Movement->GetCurrentMovementGait(),EMovementGait::Sprint);
        AddInfo(FString::Printf(TEXT("[GuardOverlayScope] Phase=%d Repeats=25 FinalRuntime=%d FinalSpeed=%.0f"),Phase,F.Defense->HasGuardRuntimeState(),F.Character->GetCharacterMovement()->MaxWalkSpeed));
    }
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGuardBoundaryHitSnapshot,
    "PortfolioVerification.Guard.HitSnapshotAfterInterruption",
    EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FGuardBoundaryHitSnapshot::RunTest(const FString& Parameters)
{
    using namespace GuardBoundary;
    FFixture F;F.Movement->HandleSprintInput();F.Begin(3);
    F.Character->AddInstanceComponent(F.Defense);
    UCObservableOverlayComponent* Overlay=NewObject<UCObservableOverlayComponent>(F.Character);
    F.Character->AddInstanceComponent(Overlay);
    FCharacterComponentReferences References;References.OwnerCharacter=F.Character;
    Overlay->InitializeReferences(References);
    UCReaction_Hit* Hit=NewObject<UCReaction_Hit>();
    FObservableOverlayQuery Query;
    Overlay->BuildOverlaySnapshot(Query.DecisionQuery.Snapshot.ObservableOverlay);
    TestTrue(TEXT("GuardOut snapshot contains runtime"),Query.DecisionQuery.Snapshot.ObservableOverlay.Guard.HasGuardRuntimeState());
    FExecutionParticipant& Incoming=Query.DecisionQuery.IncomingPart;
    Incoming.bIsValid=true;Incoming.ParticipantDomain=EExecutionDomain::Reaction;
    Incoming.ReactionContext.ReactionExecutor=Hit;
    Incoming.ReactionContext.ReactionDataKey.MatchMode=EReactionDataMatchMode::Global;
    Incoming.ReactionContext.ReactionDataKey.ReactionType=EReactionType::Hit;
    Incoming.ReactionContext.ReactionDataKey.ReactionIndex=INDEX_NONE;
    Incoming.ReactionContext.ReactionData.ReactionDataKey=Incoming.ReactionContext.ReactionDataKey;
    Incoming.ReactionContext.ReactionData.ReactionExecutorKey=UCReaction_Hit::StaticClass();
    Incoming.ReactionContext.ReactionData.Montage=NewObject<UAnimMontage>();
    TestTrue(TEXT("Fixture Hit participant valid"),Incoming.IsValidMinimal());
    FObservableOverlayExecutionDecision Decision;
    Hit->ResolveObservableOverlayCondition(Query,Decision);
    TestTrue(TEXT("Actual Hit policy accepts snapshot"),Decision.IsAccepted());
    TestTrue(TEXT("Actual Hit policy requests guard cleanup"),Decision.Handlings.Contains(EObservableOverlayHandling::ClearGuardState));
    TestTrue(TEXT("Interruption event accepted by registry"),Overlay->ApplyOverlayEvent(FObservableOverlayEventContext(EObservableOverlayEventType::GuardLifecycleInterrupted)));
    TestFalse(TEXT("State already empty after interruption"),F.Defense->HasGuardRuntimeState());
    const bool Applied=Overlay->ApplyOverlayHandlings(Decision.Handlings);
    TestTrue(TEXT("Snapshot cleanup remains accepted after interruption"),Applied);
    TestEqual(TEXT("Interrupted snapshot preserves restored sprint speed"),F.Character->GetCharacterMovement()->MaxWalkSpeed,620.f);
    AddInfo(FString::Printf(TEXT("[GuardHitSnapshot] SnapshotRuntime=1 AfterInterruptRuntime=%d Handlings=%d Applied=%d FinalSpeed=%.0f MontagePlayed=0"),F.Defense->HasGuardRuntimeState(),Decision.Handlings.Num(),Applied,F.Character->GetCharacterMovement()->MaxWalkSpeed));
    return true;
}
#endif
