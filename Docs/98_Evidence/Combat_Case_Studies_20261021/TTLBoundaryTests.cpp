// Investigation-only engine tests, 2026-10-21. Not gameplay footage.
// Synthetic accepted-hit packets and reaction events enter real production delegates.
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "HAL/IConsoleManager.h"
#include "Character/Enemy/CEnemy.h"
#include "Character/Player/CPlayer.h"
#include "Controller/CAIController.h"
#include "Component/CEnemyHitReactiveComponent.h"
#include "Component/CEnemyCombatParticipationComponent.h"
#include "Component/CCombatSignalTargetComponent.h"
#include "Component/CReactionComponent.h"
#include "Component/CHealthComponent.h"
#include "System/Combat/CWorldSubsystem_CombatParticipation.h"
#include "Type/CCharacterComponentReferenceTypes.h"
#include "Type/CCombatSignalTargetTypes.h"
#include "Type/CReactionOrchestrationTypes.h"

struct FTTLBoundaryProbe
{
    static int Pending(const UCEnemyHitReactiveComponent* H){return H->PendingCombatantTargetByResultSerial.Num();}
    static FCombatParticipationEvidence Read(const UCWorldSubsystem_CombatParticipation* S,ACAIController* C,AActor* A)
    {
        const auto* E=S->EvidenceRegistry.Find({C,ECombatParticipationSource::HitReactive,A});
        return E?*E:FCombatParticipationEvidence();
    }
    static int Count(const UCWorldSubsystem_CombatParticipation* S){return S->EvidenceRegistry.Num();}
    static void Begin(UCEnemyHitReactiveComponent* H){H->RegisterAllComponentTickFunctions(true);H->BeginPlay();}
    static void End(UCEnemyHitReactiveComponent* H){if(H->HasBegunPlay())H->EndPlay(EEndPlayReason::Destroyed);}
};

namespace TTLBoundary
{
struct FFixture
{
    UWorld* W=nullptr; ACEnemy* Enemy=nullptr; ACAIController* C=nullptr;
    ACPlayer* A=nullptr; ACPlayer* B=nullptr;
    UCEnemyHitReactiveComponent* H=nullptr; UCEnemyCombatParticipationComponent* P=nullptr;
    UCCombatSignalTargetComponent* Signal=nullptr; UCReactionComponent* Reaction=nullptr;
    UCHealthComponent* Health=nullptr; UCWorldSubsystem_CombatParticipation* S=nullptr;
    FAutomationTestBase& T; FString Case;
    IConsoleVariable* TTL=nullptr; float OldTTL=0;
    FFixture(FAutomationTestBase& InT,const TCHAR* InCase):T(InT),Case(InCase)
    {
        TTL=IConsoleManager::Get().FindConsoleVariable(TEXT("Portfolio.AI.CombatParticipation.HitReactivePostReactionTTL"));
        OldTTL=TTL->GetFloat();TTL->Set(1.f,ECVF_SetByCode);
        W=UWorld::CreateWorld(EWorldType::Game,false);
        GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(W);
        FActorSpawnParameters Params;Params.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        Enemy=W->SpawnActor<ACEnemy>(FVector::ZeroVector,FRotator::ZeroRotator,Params);
        C=W->SpawnActor<ACAIController>();
        // Link possession without starting a Behavior Tree or perception scenario.
        C->SetPawn(Enemy);Enemy->APawn::PossessedBy(C);
        A=W->SpawnActor<ACPlayer>(FVector(100,0,0),FRotator::ZeroRotator,Params);
        B=W->SpawnActor<ACPlayer>(FVector(0,100,0),FRotator::ZeroRotator,Params);
        for(AActor* Actor:{static_cast<AActor*>(Enemy),static_cast<AActor*>(A),static_cast<AActor*>(B)})
            Actor->FindComponentByClass<UCHealthComponent>()->InitializeHealth(100,100,EMaxHPUpdatePolicy::ClampCurrent);
        Health=Enemy->FindComponentByClass<UCHealthComponent>();
        H=Enemy->FindComponentByClass<UCEnemyHitReactiveComponent>();
        P=Enemy->FindComponentByClass<UCEnemyCombatParticipationComponent>();
        Signal=Enemy->FindComponentByClass<UCCombatSignalTargetComponent>();
        Reaction=Enemy->FindComponentByClass<UCReactionComponent>();
        S=W->GetSubsystem<UCWorldSubsystem_CombatParticipation>();
        FCharacterComponentReferences R;R.OwnerCharacter=Enemy;R.HealthComponent=Health;
        R.EnemyCombatParticipationComponent=P;R.CombatSignalTargetComponent=Signal;R.ReactionComponent=Reaction;
        H->InitializeReferences(R);FTTLBoundaryProbe::Begin(H);P->InitializeReferences(R);P->SetAIController(C);
        T.TestNotNull(TEXT("World production subsystem exists"),S);
    }
    ~FFixture(){FTTLBoundaryProbe::End(H);P->ClearAIController();W->DestroyWorld(false);GEngine->DestroyWorldContext(W);TTL->Set(OldTTL,ECVF_SetByCode);}
    void Step(float Dt)
    {
        const float Before=W->GetTimeSeconds();float Left=Dt;
        while(Left>KINDA_SMALL_NUMBER){const float D=FMath::Min(.05f,Left);W->Tick(LEVELTICK_TimeOnly,D);S->Tick(D);Left-=D;}
        T.TestTrue(TEXT("Requested world time actually elapsed"),FMath::IsNearlyEqual(W->GetTimeSeconds()-Before,Dt,.002f));
    }
    FCombatSignalTargetPacket Packet(AActor* Source,uint64 Serial)
    {
        FCombatSignalTargetPacket K;K.ResultSerial=Serial;K.Context.SourceActor=Source;K.Context.TargetActor=Enemy;
        K.Result.bAccepted=true;K.Result.bShouldCommitDamage=true;K.Result.CommittedDamage=5;K.Result.DeadState_After=EDeadState::Alive;
        return K;
    }
    void Hit(AActor* Source,uint64 Serial){Signal->OnCombatSignalTargetAccepted.Broadcast(Packet(Source,Serial));}
    void Resolve(AActor* Source,uint64 Serial,EReactionRequestResultType Result)
    {
        FReactionRequestResult R;R.ResultType=Result;
        Signal->OnCombatSignalTargetReactionResolved.Broadcast(Packet(Source,Serial),R);
    }
    void End(uint64 Serial,EReactionExecutionLifecycleEventType Kind=EReactionExecutionLifecycleEventType::Completed)
    {
        FReactionExecutionLifecycleEvent E;E.EventType=Kind;E.Context.CombatSignalResultSerial=Serial;
        Reaction->OnReactionExecutionLifecycleEvent.Broadcast(E);
    }
    FCombatParticipationEvidence Read(AActor* Source){return FTTLBoundaryProbe::Read(S,C,Source);}
    void Log(const TCHAR* Stage)
    {
        auto EA=Read(A);auto EB=Read(B);
        T.AddInfo(FString::Printf(TEXT("[TTLBoundary] Case=%s Stage=%s Time=%.4f Pending=%d Registry=%d ASerial=%llu AStarted=%d AExpiry=%.4f BSerial=%llu BStarted=%d BExpiry=%.4f"),*Case,Stage,W->GetTimeSeconds(),FTTLBoundaryProbe::Pending(H),FTTLBoundaryProbe::Count(S),EA.HitReactiveResultSerial,EA.bHasStartedHitReactivePostReactionTTL,EA.HitReactiveExpireTimeSeconds,EB.HitReactiveResultSerial,EB.bHasStartedHitReactivePostReactionTTL,EB.HitReactiveExpireTimeSeconds));
    }
    void AssertAwait(AActor* Source,uint64 Serial)
    {
        auto E=Read(Source);T.TestEqual(TEXT("Correct latest serial"),E.HitReactiveResultSerial,Serial);
        T.TestFalse(TEXT("TTL remains deferred"),E.bHasStartedHitReactivePostReactionTTL);
        T.TestEqual(TEXT("Deferred expiry remains zero"),E.HitReactiveExpireTimeSeconds,0.f);
    }
    void AssertStarted(AActor* Source,uint64 Serial)
    {
        auto E=Read(Source);T.TestEqual(TEXT("Correct serial at TTL start"),E.HitReactiveResultSerial,Serial);
        T.TestTrue(TEXT("TTL started"),E.bHasStartedHitReactivePostReactionTTL);
        T.TestTrue(TEXT("Expiry equals world time plus one second"),FMath::IsNearlyEqual(E.HitReactiveExpireTimeSeconds,W->GetTimeSeconds()+1.f,.001f));
    }
};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTTLSameSource,"PortfolioVerification.TTL.SameAttackerSerialAndDuplicate",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FTTLSameSource::RunTest(const FString&)
{
    using namespace TTLBoundary;
    for(int Order=0;Order<2;++Order)
    {
        FFixture F(*this,Order?TEXT("T-B1_new_first"):TEXT("T-B1_old_first"));
        F.Hit(F.A,1);F.Resolve(F.A,1,EReactionRequestResultType::Started);F.Step(.2f);
        F.Hit(F.A,2);F.Resolve(F.A,2,EReactionRequestResultType::Intervened);F.AssertAwait(F.A,2);F.Log(TEXT("S2_accepted"));
        if(!Order){F.End(1,EReactionExecutionLifecycleEventType::Interrupted);F.AssertAwait(F.A,2);F.Log(TEXT("old_end_does_not_start_S2"));}
        F.Step(.3f);F.End(2);F.AssertStarted(F.A,2);const float Exp=F.Read(F.A).HitReactiveExpireTimeSeconds;
        F.Log(TEXT("S2_end"));F.Step(.2f);F.End(1);F.End(2);F.End(2);F.Resolve(F.A,2,EReactionRequestResultType::Rejected);
        TestEqual(TEXT("Duplicate/old delegate events do not extend expiry"),F.Read(F.A).HitReactiveExpireTimeSeconds,Exp);
        TestEqual(TEXT("Both pending serials removed"),FTTLBoundaryProbe::Pending(F.H),0);F.Log(TEXT("duplicate_callbacks"));
        F.Step(.9f);TestFalse(TEXT("Evidence expires after TTL"),F.S->HasActiveEvidenceForParticipationPair(F.C,F.A));F.Log(TEXT("expired"));
    }
    { FFixture F(*this,TEXT("T-B1_new_hit_redefers"));F.Hit(F.A,1);F.End(1);F.Step(.3f);F.Hit(F.A,2);F.AssertAwait(F.A,2);F.End(1);F.AssertAwait(F.A,2);F.Log(TEXT("old_timer_reset_by_new_hit")); }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTTLDifferentSources,"PortfolioVerification.TTL.CrossAttackerOrdering",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FTTLDifferentSources::RunTest(const FString&)
{
    using namespace TTLBoundary;
    for(int Order=0;Order<2;++Order)
    {
        FFixture F(*this,Order?TEXT("T-B2_B_then_A"):TEXT("T-B2_A_then_B"));F.Hit(F.A,10);F.Hit(F.B,11);F.Log(TEXT("two_attackers"));
        AActor* First=Order?F.B:F.A;AActor* Last=Order?F.A:F.B;uint64 S1=Order?11:10,S2=Order?10:11;
        F.Step(.2f);F.End(S1);F.AssertStarted(First,S1);F.AssertAwait(Last,S2);float Exp=F.Read(First).HitReactiveExpireTimeSeconds;F.Log(TEXT("first_end"));
        F.Step(.3f);F.End(S2);F.AssertStarted(Last,S2);TestEqual(TEXT("Other attacker's expiry untouched"),F.Read(First).HitReactiveExpireTimeSeconds,Exp);
        F.End(S1);F.End(S2);TestEqual(TEXT("No pending serial after crossed ends"),FTTLBoundaryProbe::Pending(F.H),0);F.Log(TEXT("second_end"));
        F.Step(.75f);TestFalse(TEXT("First expiry reached"),F.S->HasActiveEvidenceForParticipationPair(F.C,First));TestTrue(TEXT("Other attacker still active"),F.S->HasActiveEvidenceForParticipationPair(F.C,Last));F.Log(TEXT("independent_expiry"));
        F.Step(.3f);TestFalse(TEXT("Second expiry reached"),F.S->HasActiveEvidenceForParticipationPair(F.C,Last));
    }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTTLTerminalPaths,"PortfolioVerification.TTL.TerminalAndRequestPaths",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FTTLTerminalPaths::RunTest(const FString&)
{
    using namespace TTLBoundary;
    for(auto Kind:{EReactionExecutionLifecycleEventType::Completed,EReactionExecutionLifecycleEventType::Interrupted,EReactionExecutionLifecycleEventType::Ignored})
    {
        FString Name=FString::Printf(TEXT("T-B3_terminal_%d"),int(Kind));FFixture F(*this,*Name);F.Hit(F.A,1);F.Step(.2f);F.End(1,Kind);F.AssertStarted(F.A,1);TestEqual(TEXT("Terminal removes pending"),FTTLBoundaryProbe::Pending(F.H),0);F.Log(TEXT("terminal"));
    }
    for(auto Result:{EReactionRequestResultType::None,EReactionRequestResultType::Rejected,EReactionRequestResultType::Ignored,EReactionRequestResultType::Started,EReactionRequestResultType::Intervened})
    {
        FString Name=FString::Printf(TEXT("T-B3_request_%d"),int(Result));FFixture F(*this,*Name);F.Hit(F.A,1);F.Step(.2f);F.Resolve(F.A,1,Result);
        bool Accepted=Result==EReactionRequestResultType::Started||Result==EReactionRequestResultType::Intervened;
        if(Accepted)F.AssertAwait(F.A,1);else F.AssertStarted(F.A,1);
        TestEqual(TEXT("Pending follows acceptance"),FTTLBoundaryProbe::Pending(F.H),Accepted?1:0);F.Log(TEXT("request_resolved"));
    }
    { FFixture F(*this,TEXT("T-B3_unrelated"));F.Hit(F.A,1);F.End(0);F.End(999);F.End(1,EReactionExecutionLifecycleEventType::None);F.AssertAwait(F.A,1);TestEqual(TEXT("Unrelated events preserve pending"),FTTLBoundaryProbe::Pending(F.H),1);F.Log(TEXT("unrelated_ignored")); }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTTLLifecycle,"PortfolioVerification.TTL.InvalidActorAndTeardown",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FTTLLifecycle::RunTest(const FString&)
{
    using namespace TTLBoundary;
    { FFixture F(*this,TEXT("T-B4_attacker_destroyed"));F.Hit(F.A,1);F.A->Destroy();F.Step(.2f);F.End(1);F.Log(TEXT("after_destroy_callback"));
      TestFalse(TEXT("Destroyed attacker is not active evidence"),F.S->HasActiveEvidenceForParticipationPair(F.C,F.A));
      TestEqual(TEXT("Invalid evidence removed from registry"),FTTLBoundaryProbe::Count(F.S),0);
      // Observe pending separately: retention is not equivalent to targeting a destroyed actor.
      AddInfo(FString::Printf(TEXT("[TTLFinding] AttackerDestroyed PendingAfterCallback=%d"),FTTLBoundaryProbe::Pending(F.H)));
      FTTLBoundaryProbe::End(F.H);TestEqual(TEXT("EndPlay clears pending"),FTTLBoundaryProbe::Pending(F.H),0);F.Log(TEXT("after_component_end")); }
    { FFixture F(*this,TEXT("T-B4_attacker_dead"));F.Hit(F.A,1);F.A->FindComponentByClass<UCHealthComponent>()->TryKill();F.End(1);F.Step(.2f);
      TestFalse(TEXT("Dead attacker not active"),F.S->HasActiveEvidenceForParticipationPair(F.C,F.A));TestEqual(TEXT("Dead attacker callback pending removed"),FTTLBoundaryProbe::Pending(F.H),0);F.Log(TEXT("dead_attacker")); }
    { FFixture F(*this,TEXT("T-B4_owner_release"));F.Hit(F.A,1);F.Health->TryKill();F.P->HardReleaseParticipationForOwnerDeath();F.Log(TEXT("owner_death_public_release"));
      TestEqual(TEXT("Owner death release removes evidence"),FTTLBoundaryProbe::Count(F.S),0);F.End(1);TestEqual(TEXT("Late callback does not recreate evidence"),FTTLBoundaryProbe::Count(F.S),0);
      TestEqual(TEXT("Late callback pending removed"),FTTLBoundaryProbe::Pending(F.H),0);F.Hit(F.A,2);TestEqual(TEXT("Dead owner rejects new evidence"),FTTLBoundaryProbe::Count(F.S),0);F.Log(TEXT("late_event_after_death")); }
    { FFixture F(*this,TEXT("T-B4_hit_component_end"));F.Hit(F.A,1);FTTLBoundaryProbe::End(F.H);F.End(1);F.Hit(F.B,2);
      TestEqual(TEXT("EndPlay removes pending and delegate listeners"),FTTLBoundaryProbe::Pending(F.H),0);
      AddInfo(FString::Printf(TEXT("[TTLFinding] HitComponentEnd Registry=%d"),FTTLBoundaryProbe::Count(F.S)));F.Log(TEXT("hit_component_only_ended"));
      F.P->ClearAIController();TestEqual(TEXT("Participation teardown releases registry"),FTTLBoundaryProbe::Count(F.S),0);F.Log(TEXT("participation_teardown")); }
    { FFixture F(*this,TEXT("T-B4_repeated_destroy"));
      for(uint64 Serial=1;Serial<=8;++Serial){
        FActorSpawnParameters Params;Params.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        ACPlayer* Target=F.W->SpawnActor<ACPlayer>(FVector(100,0,0),FRotator::ZeroRotator,Params);
        Target->FindComponentByClass<UCHealthComponent>()->InitializeHealth(100,100,EMaxHPUpdatePolicy::ClampCurrent);
        F.Hit(Target,Serial);Target->Destroy();F.Step(.05f);F.End(Serial);
      }
      F.Step(2.f);TestEqual(TEXT("Repeated destroyed actors leave no target evidence"),FTTLBoundaryProbe::Count(F.S),0);
      F.Log(TEXT("eight_destroys_after_two_seconds"));
      AddInfo(FString::Printf(TEXT("[TTLFinding] RepeatedDestroyedAttackers Count=8 Pending=%d"),FTTLBoundaryProbe::Pending(F.H)));
      FTTLBoundaryProbe::End(F.H);TestEqual(TEXT("EndPlay clears all invalid pending entries"),FTTLBoundaryProbe::Pending(F.H),0);
    }
    return true;
}
#endif
