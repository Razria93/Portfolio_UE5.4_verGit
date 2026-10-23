#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "HAL/IConsoleManager.h"
#include "Engine/World.h"
#include "Controller/CAIController.h"
#include "Character/Player/CPlayer.h"
#include "Component/CHealthComponent.h"
#include "System/Combat/CWorldSubsystem_CombatParticipation.h"

namespace ParticipationCapsTest
{
struct FScopedInt
{
 IConsoleVariable* Variable; FString Previous; EConsoleVariableFlags PreviousFlags;
 FScopedInt(const TCHAR* Name, int32 Value) : Variable(IConsoleManager::Get().FindConsoleVariable(Name)), Previous(Variable->GetString()), PreviousFlags(Variable->GetFlags()) { Variable->Set(Value, ECVF_SetByCode); }
 ~FScopedInt() { Variable->Set(*Previous, ECVF_SetByCode); Variable->SetFlags(PreviousFlags); }
};
struct FCaps { int32 Base, Extra, Total, Alert, Observe; };
struct FCounts { int32 Base=0, Extra=0, Alert=0, Observe=0, None=0; };
FCounts Count(UCWorldSubsystem_CombatParticipation* S, const TArray<ACAIController*>& Controllers)
{
 FCounts R;
 for(auto* C:Controllers) { const auto A=S->GetAssignment(C);
  if(A.EngageAdmission==EEngageAdmissionKind::GeneralBase) ++R.Base;
  else if(A.EngageAdmission==EEngageAdmissionKind::HitReactiveExtra) ++R.Extra;
  else if(A.CombatRole==ECombatRole::Alert) ++R.Alert;
  else if(A.CombatRole==ECombatRole::Observe) ++R.Observe;
  else ++R.None;
 } return R;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FParticipationCapsAllocationTest, "Portfolio.AI.ParticipationCaps.AllocationAndRelease", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FParticipationCapsAllocationTest::RunTest(const FString&)
{
 using namespace ParticipationCapsTest;
 const FCaps Cases[]={{2,3,5,6,12},{1,2,3,1,1},{2,3,3,1,1},{0,0,0,0,0}};
 for(const auto Caps:Cases)
 {
  FScopedInt B(TEXT("Portfolio.AI.RuntimeLOD.EngageAssignmentEngageCap"),Caps.Base), E(TEXT("Portfolio.AI.RuntimeLOD.EngageAssignmentHitReactiveExtraCap"),Caps.Extra), T(TEXT("Portfolio.AI.RuntimeLOD.EngageAssignmentTotalCap"),Caps.Total), A(TEXT("Portfolio.AI.RuntimeLOD.EngageAssignmentAlertCap"),Caps.Alert), O(TEXT("Portfolio.AI.RuntimeLOD.EngageAssignmentObserveCap"),Caps.Observe), Warm(TEXT("Portfolio.AI.RuntimeLOD.EngageAssignmentWarmupTime"),0);
  UWorld* World=UWorld::CreateWorld(EWorldType::Game,false); World->CreateAISystem();
  auto* S=World->GetSubsystem<UCWorldSubsystem_CombatParticipation>();
  auto* Target=World->SpawnActorDeferred<ACPlayer>(ACPlayer::StaticClass(),FTransform::Identity);
  Target->GetHealthComp()->InitializeHealth(100,100,EMaxHPUpdatePolicy::ClampCurrent);
  TArray<ACAIController*> Cs;
  for(int32 I=0;I<24;++I) { auto* C=World->SpawnActor<ACAIController>(); Cs.Add(C); FCombatParticipationEvidenceContext Context; Context.DistanceToTarget=100+I; S->ReportEvidence(C,ECombatParticipationSource::Perception,Target,Context); }
  S->Tick(.11f); auto R=Count(S,Cs);
  TestEqual(TEXT("Sight-only Base obeys both Base and Total"),R.Base,FMath::Min(Caps.Base,Caps.Total));
  TestEqual(TEXT("Sight cannot consume Extra"),R.Extra,0);
  TestEqual(TEXT("Alert cap"),R.Alert,Caps.Alert); TestEqual(TEXT("Observe cap"),R.Observe,Caps.Observe);
  for(int32 I=0;I<Cs.Num();++I) { FCombatParticipationEvidenceContext Context; Context.DistanceToTarget=100+I; S->ReportHitReactiveEvidence(Cs[I],Target,Context,I+1); }
  S->Tick(.11f); R=Count(S,Cs);
  const int32 ExpectedBase=FMath::Min(Caps.Base,Caps.Total), ExpectedExtra=FMath::Min(Caps.Extra,Caps.Total-ExpectedBase);
  TestEqual(TEXT("Live hit evidence fills Extra within Total"),R.Extra,ExpectedExtra);
  TestEqual(TEXT("Base preserved with hit evidence"),R.Base,ExpectedBase);
  TestEqual(TEXT("Unassigned overflow is explicit"),R.None,24-ExpectedBase-ExpectedExtra-Caps.Alert-Caps.Observe);
  if(ExpectedBase>0) {
   ACAIController* Removed=nullptr;
   for(auto* C:Cs) if(S->GetAssignment(C).EngageAdmission==EEngageAdmissionKind::GeneralBase) {Removed=C;break;}
   S->SetParticipationSuppressed(Removed,true);
   TestFalse(TEXT("Suppression clears participant assignment"),S->GetAssignment(Removed).IsValidAssignment());
   R=Count(S,Cs);TestEqual(TEXT("Waiting participant fills released Base slot"),R.Base,ExpectedBase);
  }
  for(auto* C:Cs) S->WithdrawEvidence(C,ECombatParticipationSource::HitReactive,Target,false);
  S->Tick(.11f); R=Count(S,Cs);TestEqual(TEXT("Removing hit evidence revokes Extra admission"),R.Extra,0);
  for(auto* C:Cs) S->HardReleaseParticipationForParticipant(C);
  TestEqual(TEXT("Hard release clears all roles"),Count(S,Cs).None,24);
  World->DestroyWorld(false);
 }
 return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FParticipationCapsLifetimeTest, "Portfolio.AI.ParticipationCaps.TargetIsolationAndLocks", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FParticipationCapsLifetimeTest::RunTest(const FString&)
{
 using namespace ParticipationCapsTest;
 FScopedInt B(TEXT("Portfolio.AI.RuntimeLOD.EngageAssignmentEngageCap"),1), E(TEXT("Portfolio.AI.RuntimeLOD.EngageAssignmentHitReactiveExtraCap"),2), T(TEXT("Portfolio.AI.RuntimeLOD.EngageAssignmentTotalCap"),3), A(TEXT("Portfolio.AI.RuntimeLOD.EngageAssignmentAlertCap"),1), O(TEXT("Portfolio.AI.RuntimeLOD.EngageAssignmentObserveCap"),1), Warm(TEXT("Portfolio.AI.RuntimeLOD.EngageAssignmentWarmupTime"),0);
 UWorld* World=UWorld::CreateWorld(EWorldType::Game,false);World->CreateAISystem();
 auto* S=World->GetSubsystem<UCWorldSubsystem_CombatParticipation>();
 TArray<ACPlayer*> Targets;TArray<ACAIController*> Cs;
 for(int I=0;I<2;++I) {
  auto* Target=World->SpawnActorDeferred<ACPlayer>(ACPlayer::StaticClass(),FTransform::Identity);Target->GetHealthComp()->InitializeHealth(100,100,EMaxHPUpdatePolicy::ClampCurrent);Targets.Add(Target);
  auto* C=World->SpawnActor<ACAIController>();Cs.Add(C);FCombatParticipationEvidenceContext Context;S->ReportEvidence(C,ECombatParticipationSource::Perception,Target,Context);
 }
 S->Tick(.11f);
 TestEqual(TEXT("Each target has an independent Base slot"),Count(S,Cs).Base,2);
 const auto Assignment=S->GetAssignment(Cs[0]);FCombatParticipationAssignmentLock Lock;Lock.TargetActor=Targets[0];Lock.AssignmentRevision=Assignment.AssignmentRevision;
 TestTrue(TEXT("Exact assignment can acquire an action lock"),S->AcquireAssignmentLock(Cs[0],Lock));
 ++Lock.AssignmentRevision;TestFalse(TEXT("Stale revision cannot acquire a lock"),S->AcquireAssignmentLock(Cs[0],Lock));
 S->WithdrawEvidence(Cs[0],ECombatParticipationSource::Perception,Targets[0],false);S->Tick(.11f);
 TestTrue(TEXT("Active lock retains assignment after evidence withdrawal"),S->GetAssignment(Cs[0]).IsValidAssignment());
 S->ReleaseAssignmentLock(Cs[0]);TestFalse(TEXT("Action terminal release removes evidence-free assignment"),S->GetAssignment(Cs[0]).IsValidAssignment());
 Targets[1]->GetHealthComp()->TakeDamage(100);
 TestFalse(TEXT("Target death immediately releases assignment"),S->GetAssignment(Cs[1]).IsValidAssignment());
 TestFalse(TEXT("Target death clears active evidence"),S->HasActiveEvidenceForParticipationPair(Cs[1],Targets[1]));
 World->DestroyWorld(false);return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FParticipationBlockedMoveTest, "Portfolio.AI.ParticipationCaps.BlockedMoveReselection", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FParticipationBlockedMoveTest::RunTest(const FString&)
{
 using namespace ParticipationCapsTest;
 for(const FCaps Caps : {FCaps{2,3,5,6,12},FCaps{1,2,3,1,1}})
 {
  FScopedInt B(TEXT("Portfolio.AI.RuntimeLOD.EngageAssignmentEngageCap"),Caps.Base), E(TEXT("Portfolio.AI.RuntimeLOD.EngageAssignmentHitReactiveExtraCap"),Caps.Extra), T(TEXT("Portfolio.AI.RuntimeLOD.EngageAssignmentTotalCap"),Caps.Total), A(TEXT("Portfolio.AI.RuntimeLOD.EngageAssignmentAlertCap"),Caps.Alert), O(TEXT("Portfolio.AI.RuntimeLOD.EngageAssignmentObserveCap"),Caps.Observe), Warm(TEXT("Portfolio.AI.RuntimeLOD.EngageAssignmentWarmupTime"),0);
  UWorld* World=UWorld::CreateWorld(EWorldType::Game,false);World->CreateAISystem();
  auto* S=World->GetSubsystem<UCWorldSubsystem_CombatParticipation>();
  auto* Target=World->SpawnActorDeferred<ACPlayer>(ACPlayer::StaticClass(),FTransform::Identity);
  Target->GetHealthComp()->InitializeHealth(100,100,EMaxHPUpdatePolicy::ClampCurrent);
  TArray<ACAIController*> Cs;
  for(int I=0;I<8;++I){auto* C=World->SpawnActor<ACAIController>();Cs.Add(C);FCombatParticipationEvidenceContext X;X.DistanceToTarget=100+I;S->ReportEvidence(C,ECombatParticipationSource::Perception,Target,X);}
  S->Tick(.11f);
  ACAIController* Blocked=nullptr;for(auto* C:Cs)if(S->GetAssignment(C).EngageAdmission==EEngageAdmissionKind::GeneralBase){Blocked=C;break;}
  const auto Original=S->GetAssignment(Blocked);
  auto Report=[&](bool bBlocked, FVector Location=FVector(0,0,0)){S->ReportEngageMoveResult(Blocked,Target,Original.AssignmentRevision,bBlocked,Location);S->Tick(.11f);};
  Report(true);TestEqual(TEXT("One block keeps assignment"),S->GetAssignment(Blocked).AssignmentRevision,Original.AssignmentRevision);
  Report(false);Report(true);TestEqual(TEXT("Successful/aborted completion resets streak"),S->GetAssignment(Blocked).AssignmentRevision,Original.AssignmentRevision);
  Report(true,FVector(11,0,0));TestEqual(TEXT("More than 10cm progress restarts streak"),S->GetAssignment(Blocked).AssignmentRevision,Original.AssignmentRevision);
  World->TimeSeconds+=16;Report(true,FVector(11,0,0));TestEqual(TEXT("Failures over 15s apart do not combine"),S->GetAssignment(Blocked).AssignmentRevision,Original.AssignmentRevision);
  FCombatParticipationAssignmentLock Lock;Lock.TargetActor=Target;Lock.AssignmentRevision=Original.AssignmentRevision;
  TestTrue(TEXT("Active action lock acquired"),S->AcquireAssignmentLock(Blocked,Lock));
  Report(true);Report(true);TestEqual(TEXT("Lock prevents eviction"),S->GetAssignment(Blocked).AssignmentRevision,Original.AssignmentRevision);
  S->ReleaseAssignmentLock(Blocked);
  Report(true);TestEqual(TEXT("Lock also clears prior failure history"),S->GetAssignment(Blocked).AssignmentRevision,Original.AssignmentRevision);
  S->ReportEngageMoveResult(Blocked,Target,Original.AssignmentRevision+1,true,FVector::ZeroVector);S->Tick(.11f);
  TestEqual(TEXT("Stale revision ignored"),S->GetAssignment(Blocked).AssignmentRevision,Original.AssignmentRevision);
  Report(true);
  TestTrue(TEXT("Second stationary block yields Engage"),S->GetAssignment(Blocked).CombatRole!=ECombatRole::Engage);
  TestEqual(TEXT("Waiting actor replaces Base within cap"),Count(S,Cs).Base,Caps.Base);
  TestEqual(TEXT("Sight-only replacement cannot use Extra"),Count(S,Cs).Extra,0);
  TestTrue(TEXT("Yield preserves evidence"),S->HasActiveEvidenceForParticipationPair(Blocked,Target));
  // Remove alternatives: retry remains bounded even when nobody can replace the actor.
  for(auto* C:Cs)if(C!=Blocked)S->HardReleaseParticipationForParticipant(C);
  World->TimeSeconds+=4.99;S->Tick(.11f);TestTrue(TEXT("Cooldown prevents immediate reclaim"),S->GetAssignment(Blocked).CombatRole!=ECombatRole::Engage);
  World->TimeSeconds+=.02;S->Tick(.11f);TestEqual(TEXT("No-alternative actor retries after 5s"),S->GetAssignment(Blocked).CombatRole,ECombatRole::Engage);
  auto Current=S->GetAssignment(Blocked);S->ReportEngageMoveResult(Blocked,Target,Current.AssignmentRevision,true,FVector::ZeroVector);
  S->HardReleaseParticipationForParticipant(Blocked);FCombatParticipationEvidenceContext X;S->ReportEvidence(Blocked,ECombatParticipationSource::Perception,Target,X);S->Tick(.11f);
  Current=S->GetAssignment(Blocked);S->ReportEngageMoveResult(Blocked,Target,Current.AssignmentRevision,true,FVector::ZeroVector);S->Tick(.11f);
  TestEqual(TEXT("Release clears failure history"),S->GetAssignment(Blocked).CombatRole,ECombatRole::Engage);
  Target->GetHealthComp()->TakeDamage(100);S->ReportEngageMoveResult(Blocked,Target,Current.AssignmentRevision,true,FVector::ZeroVector);S->Tick(.11f);
  TestFalse(TEXT("Dead target cannot be restored by a late block"),S->GetAssignment(Blocked).IsValidAssignment());
  World->DestroyWorld(false);
 }
 return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(FParticipationBlockedExtraTest, "Portfolio.AI.ParticipationCaps.BlockedExtraAdmission", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FParticipationBlockedExtraTest::RunTest(const FString&)
{
 using namespace ParticipationCapsTest;
 FScopedInt B(TEXT("Portfolio.AI.RuntimeLOD.EngageAssignmentEngageCap"),1), E(TEXT("Portfolio.AI.RuntimeLOD.EngageAssignmentHitReactiveExtraCap"),1), T(TEXT("Portfolio.AI.RuntimeLOD.EngageAssignmentTotalCap"),2), A(TEXT("Portfolio.AI.RuntimeLOD.EngageAssignmentAlertCap"),1), O(TEXT("Portfolio.AI.RuntimeLOD.EngageAssignmentObserveCap"),1), Warm(TEXT("Portfolio.AI.RuntimeLOD.EngageAssignmentWarmupTime"),0);
 UWorld* World=UWorld::CreateWorld(EWorldType::Game,false);World->CreateAISystem();
 auto* S=World->GetSubsystem<UCWorldSubsystem_CombatParticipation>();
 auto* Target=World->SpawnActorDeferred<ACPlayer>(ACPlayer::StaticClass(),FTransform::Identity);Target->GetHealthComp()->InitializeHealth(100,100,EMaxHPUpdatePolicy::ClampCurrent);
 TArray<ACAIController*> Cs;FCombatParticipationEvidenceContext X;
 for(int I=0;I<3;++I){auto* C=World->SpawnActor<ACAIController>();Cs.Add(C);X.DistanceToTarget=100+I;S->ReportEvidence(C,ECombatParticipationSource::Perception,Target,X);}
 S->Tick(.11f);
 auto* Extra=Cs[1];if(S->GetAssignment(Extra).CombatRole==ECombatRole::Engage)Extra=Cs[2];
 auto* Waiting=Extra==Cs[1]?Cs[2]:Cs[1];if(S->GetAssignment(Waiting).CombatRole==ECombatRole::Engage)Waiting=Cs[0];
 S->ReportHitReactiveEvidence(Extra,Target,X,1);S->Tick(.11f);
 auto Assignment=S->GetAssignment(Extra);TestEqual(TEXT("Hit evidence grants Extra"),Assignment.EngageAdmission,EEngageAdmissionKind::HitReactiveExtra);
 auto* OtherTarget=World->SpawnActorDeferred<ACPlayer>(ACPlayer::StaticClass(),FTransform::Identity);OtherTarget->GetHealthComp()->InitializeHealth(100,100,EMaxHPUpdatePolicy::ClampCurrent);
 S->ReportEngageMoveResult(Extra,OtherTarget,Assignment.AssignmentRevision,true,FVector::ZeroVector);
 S->ReportEngageMoveResult(Extra,Target,Assignment.AssignmentRevision,true,FVector::ZeroVector);S->Tick(.11f);
 TestEqual(TEXT("Different target result cannot complete streak"),S->GetAssignment(Extra).AssignmentRevision,Assignment.AssignmentRevision);
 S->ReportEngageMoveResult(Extra,Target,Assignment.AssignmentRevision,true,FVector::ZeroVector);S->Tick(.11f);
 TestEqual(TEXT("Replacement requires live HitReactive evidence"),Count(S,Cs).Extra,0);
 TestEqual(TEXT("Existing Base remains within cap"),Count(S,Cs).Base,1);
 S->ReportHitReactiveEvidence(Waiting,Target,X,2);S->Tick(.11f);
 TestEqual(TEXT("Eligible waiter fills Extra"),S->GetAssignment(Waiting).EngageAdmission,EEngageAdmissionKind::HitReactiveExtra);
 // Soft release must remove backoff as well as evidence; fresh evidence may rejoin immediately.
 S->SoftReleaseParticipationForParticipant(Extra);S->HardReleaseParticipationForParticipant(Waiting);
 S->ReportHitReactiveEvidence(Extra,Target,X,3);S->Tick(.11f);
 TestEqual(TEXT("Soft release clears retry delay"),S->GetAssignment(Extra).EngageAdmission,EEngageAdmissionKind::HitReactiveExtra);
 World->DestroyWorld(false);return true;
}

#endif
