#include "Misc/AutomationTest.h"
#include "Core/Debug/FWeaponPresentationDebug.h"
#include "Core/Debug/FDebugOverlayTextPanelTypes.h"
#include "Core/Debug/FCombatFeedbackDebug.h"
#include "Component/CWeaponComponent.h"
#include "Weapon/CWeaponActor.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "HAL/IConsoleManager.h"
#include "UObject/UnrealType.h"

#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
namespace WeaponDebugTests
{
 struct FVars
 {
  struct FSaved { FString Value; EConsoleVariableFlags Priority; };
  TMap<FString,FSaved> Saved;
  FAutomationTestBase& Test;
  explicit FVars(FAutomationTestBase& InTest) : Test(InTest)
  {
   Set(TEXT("Portfolio.DebugOverlay.WeaponPresentation.PresentationMode"),TEXT("0"));
   Set(TEXT("Portfolio.DebugOverlay.WeaponPresentation.RetainSeconds"),TEXT("10"));
  }
  void Set(const TCHAR* Name,const TCHAR* Value)
  {
   IConsoleVariable* V=IConsoleManager::Get().FindConsoleVariable(Name);
   if(!Test.TestNotNull(FString::Printf(TEXT("CVar exists: %s"),Name),V))return;
   if(!Saved.Contains(Name))Saved.Add(Name,{V->GetString(),static_cast<EConsoleVariableFlags>(V->GetFlags() & ECVF_SetByMask)});
   // Use the existing priority so UI/console settings can be overridden without promoting them.
   V->Set(Value,Saved.FindChecked(Name).Priority);
  }
  ~FVars()
  {
   for(const auto& P:Saved)
   {
    IConsoleVariable* V=IConsoleManager::Get().FindConsoleVariable(*P.Key);
    V->Set(*P.Value.Value,P.Value.Priority);
    Test.TestEqual(FString::Printf(TEXT("CVar value restored: %s"),*P.Key),V->GetString(),P.Value.Value);
    Test.TestEqual(FString::Printf(TEXT("CVar priority restored: %s"),*P.Key),uint32(V->GetFlags() & ECVF_SetByMask),uint32(P.Value.Priority));
   }
  }
 };
 FString Text(const AActor* Owner,bool Enemy=false)
 {FDebugOverlayTextPanel Panel;FWeaponPresentationDebug::AppendAndDraw(Owner,Enemy,Panel);FString S;for(const auto& L:Panel.Lines)S+=L.Text+TEXT("\n");return S;}
 void Property(UObject* Object,const TCHAR* Name,UObject* Value)
 {if(auto* P=FindFProperty<FObjectPropertyBase>(Object->GetClass(),Name))P->SetObjectPropertyValue_InContainer(Object,Value);}
}
using namespace WeaponDebugTests;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWeaponDiagnosticEvidenceTest,"Portfolio.Weapon.Diagnostics.EvidenceAndGates",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FWeaponDiagnosticEvidenceTest::RunTest(const FString&)
{
 FVars Vars(*this);Vars.Set(TEXT("Portfolio.DebugOverlay.Player.Enabled"),TEXT("1"));Vars.Set(TEXT("Portfolio.DebugOverlay.Player.Feedback.Enabled"),TEXT("1"));
 Vars.Set(TEXT("Portfolio.DebugOverlay.WeaponPresentation.Enabled"),TEXT("0"));Vars.Set(TEXT("Portfolio.DebugOverlay.Feedback.Enabled"),TEXT("1"));
 Vars.Set(TEXT("Portfolio.Debug.FeedbackAudit"),TEXT("0"));
 UWorld* World=UWorld::CreateWorld(EWorldType::Game,false);AActor* Owner=World->SpawnActor<AActor>();AActor* Other=World->SpawnActor<AActor>();
 FActionFeedbackRequest Request;Request.TriggerKey=TEXT("DiagnosticTrigger");
 FCombatFeedbackDebug::RecordActionFeedbackRequestAcceptedForAudit(Owner,nullptr,Request);
 TestTrue(TEXT("HUD capture independent of audit log"),Text(Owner).Contains(TEXT("DiagnosticTrigger")));
 FWeaponPresentationDebug::SpawnResult(Owner,TEXT("Action"),nullptr,nullptr);
 TestTrue(TEXT("Null spawn never reported as success"),Text(Owner).Contains(TEXT("NoComponent")));
 TestFalse(TEXT("Other actor isolated"),Text(Other).Contains(TEXT("NoComponent")));
 FWeaponPresentationDebug::Feedback(Owner,TEXT("Hit"),TEXT("SFX"),TEXT("Requested Play"));
 TestTrue(TEXT("Sound is a request only"),Text(Owner).Contains(TEXT("Requested Play")));
 Vars.Set(TEXT("Portfolio.DebugOverlay.Player.Enabled"),TEXT("0"));TestTrue(TEXT("Player master gate"),Text(Owner).IsEmpty());Vars.Set(TEXT("Portfolio.DebugOverlay.Player.Enabled"),TEXT("1"));
 Vars.Set(TEXT("Portfolio.DebugOverlay.Feedback.Enabled"),TEXT("0"));
 FWeaponPresentationDebug::Feedback(Owner,TEXT("Hit"),TEXT("SFX"),TEXT("DisabledRecord"));
 Vars.Set(TEXT("Portfolio.DebugOverlay.Feedback.Enabled"),TEXT("1"));
 TestFalse(TEXT("Disabled capture cannot leave old records"),Text(Owner).Contains(TEXT("Requested Play")));
 TestFalse(TEXT("Disabled capture does not collect"),Text(Owner).Contains(TEXT("DisabledRecord")));
 for(int32 i=0;i<100;++i)FWeaponPresentationDebug::Feedback(Owner,TEXT("Action"),TEXT("VFX"),FString::Printf(TEXT("Bounded%d"),i));
 TestFalse(TEXT("Old feedback evicted"),Text(Owner).Contains(TEXT("Bounded0")));
 TestTrue(TEXT("Latest feedback remains"),Text(Owner).Contains(TEXT("Bounded99")));
 FWeaponPresentationDebug::Feedback(Owner,TEXT("Action"),TEXT("VFX"),FString::ChrN(240,TEXT('A')));
 FDebugOverlayTextPanel Wrapped;FWeaponPresentationDebug::AppendAndDraw(Owner,false,Wrapped);
 for(const auto& L:Wrapped.Lines)TestTrue(TEXT("Long evidence stays within panel line budget"),L.Text.Len()<=78);
 FWeaponPresentationDebug::ClearWorld(World,false,false);TestFalse(TEXT("World cleanup removes evidence"),Text(Owner).Contains(TEXT("Bounded99")));
 World->DestroyWorld(false);return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWeaponDiagnosticRuntimeTest,"Portfolio.Weapon.Diagnostics.RuntimeTransitions",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FWeaponDiagnosticRuntimeTest::RunTest(const FString&)
{
 FVars Vars(*this);
 for(const TCHAR* Name:{TEXT("Portfolio.DebugOverlay.Player.Enabled"),TEXT("Portfolio.DebugOverlay.Player.WeaponPresentation.Enabled"),TEXT("Portfolio.DebugOverlay.WeaponPresentation.Enabled")})Vars.Set(Name,TEXT("1"));
 for(const TCHAR* Name:{TEXT("Portfolio.DebugOverlay.Feedback.Enabled"),TEXT("Portfolio.DebugOverlay.WeaponPresentation.DrawSockets"),TEXT("Portfolio.DebugOverlay.WeaponPresentation.DrawPivot"),TEXT("Portfolio.DebugOverlay.WeaponPresentation.DrawTrailAnchor"),TEXT("Portfolio.DebugOverlay.WeaponPresentation.DrawHistory")})Vars.Set(Name,TEXT("0"));
 auto* WeaponClass=LoadClass<ACWeaponActor>(nullptr,TEXT("/Game/06_Weapon/BP_CWeaponActor_SciFi_Sword.BP_CWeaponActor_SciFi_Sword_C"));
 auto* Sword=LoadObject<USkeletalMesh>(nullptr,TEXT("/Game/06_Weapon/Asset/SciFi_Sword/scifi_sword"));
 if(!TestNotNull(TEXT("Weapon BP"),WeaponClass)||!TestNotNull(TEXT("Mesh"),Sword))return false;
 UWorld* World=UWorld::CreateWorld(EWorldType::Game,false);GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
 ACharacter* Owner=World->SpawnActor<ACharacter>();ACWeaponActor* Weapon=World->SpawnActor<ACWeaponActor>(WeaponClass);
 auto* Mesh=DuplicateObject<USkeletalMesh>(Sword,GetTransientPackage());
 for(int32 i=0;i<2;++i){auto* Socket=NewObject<USkeletalMeshSocket>(Mesh);Socket->SocketName=i==0?TEXT("DebugHolster"):TEXT("DebugHand");Socket->BoneName=Mesh->GetRefSkeleton().GetBoneName(0);Socket->RelativeLocation=FVector(i*100,0,0);Mesh->GetMeshOnlySocketList().Add(Socket);}
 Owner->GetMesh()->SetSkeletalMesh(Mesh);
 Property(Weapon,TEXT("OwnerCharacter_Injected"),Owner);Weapon->SetOwner(Owner);Weapon->SocketName_Holster=TEXT("DebugHolster");Weapon->SocketName_Hand=TEXT("DebugHand");
 auto* C=NewObject<UCWeaponComponent>(Owner);Owner->AddInstanceComponent(C);C->RegisterComponent();Property(C,TEXT("WeaponActor"),Weapon);Property(C,TEXT("OwnerCharacter_Injected"),Owner);
 TestTrue(TEXT("Initial attachment"),Weapon->AttachToSocketSlot(EWeaponSocketSlot::Holster));
 TestTrue(TEXT("Pivot exists"),Weapon->HasValidPivot());
 TestTrue(TEXT("Scope begins"),C->BeginWeaponActionPoseScope(101));
 uint32 H=0;TestTrue(TEXT("Transition begins"),C->BeginWeaponSocketTransformTransition(EWeaponSocketSlot::Hand,H));
 TestTrue(TEXT("Update midpoint"),C->UpdateWeaponSocketTransformTransition(H,.5f));
 const FTransform Before=Weapon->GetActorTransform();
 TestFalse(TEXT("Stale handle cannot mutate"),C->UpdateWeaponSocketTransformTransition(H+100,.9f));TestTrue(TEXT("Stale update preserves transform"),Before.Equals(Weapon->GetActorTransform()));
 TestTrue(TEXT("HUD reads eased alpha"),Text(Owner).Contains(TEXT("BlendAlpha=0.50")));
 FTransform Source,Target;Weapon->GetOwnerSocketWorldTransform(TEXT("DebugHolster"),Source);Weapon->GetOwnerSocketWorldTransform(TEXT("DebugHand"),Target);
 TestTrue(TEXT("Actual root at midpoint"),Weapon->GetActorLocation().Equals(FMath::Lerp(Source.GetLocation(),Target.GetLocation(),.5f),.01f));
 TestTrue(TEXT("Complete attaches target"),C->CompleteWeaponSocketTransformTransition(H));TestTrue(TEXT("Completion recorded"),Text(Owner).Contains(TEXT("Completed: TargetAttached")));
 TestTrue(TEXT("Return transition begins"),C->BeginWeaponSocketTransformTransition(EWeaponSocketSlot::Holster,H));C->UpdateWeaponSocketTransformTransition(H,.7f);C->CancelWeaponSocketTransformTransition(H);
 FName Attached;Weapon->GetCurrentOwnerSocketName(Attached);TestEqual(TEXT("Cancel restores source hand"),Attached,FName(TEXT("DebugHand")));
 TestTrue(TEXT("Cancel evidence"),Text(Owner).Contains(TEXT("Cancelled: SourceRestored")));
 FWeaponPivotRotationSpec Spec;Spec.Mode=EWeaponPivotRotationMode::SignedLocalAxisAngle;Spec.LocalAxis=FVector::UpVector;Spec.SignedAngleDegrees=720;
 TestTrue(TEXT("Pivot transition begins"),C->BeginWeaponPivotRotationTransition(Spec,H));C->UpdateWeaponPivotRotationTransition(H,.5f);
 TestTrue(TEXT("Full winding preserved in HUD"),Text(Owner).Contains(TEXT("requested +720.0")));
 C->CompleteWeaponPivotRotationTransition(H);Spec.SignedAngleDegrees=-90;TestTrue(TEXT("Override begins"),C->BeginWeaponPivotRotationOverride(Spec,H));C->UpdateWeaponPivotRotationOverride(H,1);
 C->EndWeaponPivotRotationOverride(H,TEXT("NotifyCancelled: BaseRestored"));TestTrue(TEXT("Override cancellation distinguished"),Text(Owner).Contains(TEXT("NotifyCancelled: BaseRestored")));
 C->EndWeaponActionPoseScope(101);TestTrue(TEXT("Restoration checked against actual attachment"),Text(Owner).Contains(TEXT("Socket=Verified | Pivot=Verified")));
 Weapon->GetCurrentOwnerSocketName(Attached);TestEqual(TEXT("Scope baseline restored"),Attached,FName(TEXT("DebugHolster")));
 C->BeginWeaponActionPoseScope(102);TestFalse(TEXT("Missing target rejected"),C->BeginWeaponSocketTransformTransition(NAME_None,TEXT("NoSuchSocket"),H));TestTrue(TEXT("Rejection has reason"),Text(Owner).Contains(TEXT("TargetSocketUnavailable")));C->EndWeaponActionPoseScope(102);
 Vars.Set(TEXT("Portfolio.DebugOverlay.WeaponPresentation.Enabled"),TEXT("0"));C->BeginWeaponActionPoseScope(103);C->BeginWeaponSocketTransformTransition(EWeaponSocketSlot::Hand,H);C->UpdateWeaponSocketTransformTransition(H,.3f);
 Vars.Set(TEXT("Portfolio.DebugOverlay.WeaponPresentation.Enabled"),TEXT("1"));TestTrue(TEXT("Late enable does not invent progress"),Text(Owner).Contains(TEXT("Progress unobserved")));C->CancelWeaponSocketTransformTransition(H);C->EndWeaponActionPoseScope(103);
 GEngine->DestroyWorldContext(World);World->DestroyWorld(false);return true;
}
#endif
