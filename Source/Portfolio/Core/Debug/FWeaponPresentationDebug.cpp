#include "Core/Debug/FWeaponPresentationDebug.h"
#include "Core/Debug/FDebugOverlayTextPanelTypes.h"
#include "Component/CWeaponComponent.h"
#include "Component/CActionComponent.h"
#include "Component/CWeaponTrailComponent.h"
#include "Weapon/CWeaponActor.h"
#include "Components/SkeletalMeshComponent.h"
#include "NiagaraComponent.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "DrawDebugHelpers.h"

#if !UE_BUILD_SHIPPING
namespace WeaponPresentationDebug
{
 TAutoConsoleVariable<int32> Enabled(TEXT("Portfolio.DebugOverlay.WeaponPresentation.Enabled"),0,TEXT("Capture weapon presentation diagnostics. Does not modify gameplay."));
 TAutoConsoleVariable<int32> Player(TEXT("Portfolio.DebugOverlay.Player.WeaponPresentation.Enabled"),0,TEXT("Show player weapon diagnostics."));
 TAutoConsoleVariable<int32> Enemy(TEXT("Portfolio.DebugOverlay.Enemy.WeaponPresentation.Enabled"),0,TEXT("Show focused enemy weapon diagnostics."));
 TAutoConsoleVariable<int32> PresentationMode(TEXT("Portfolio.DebugOverlay.WeaponPresentation.PresentationMode"),0,TEXT("0 legacy; 1 compact socket; 2 compact pivot override; 3 feedback only. Display filtering only."));
 TAutoConsoleVariable<int32> SocketPair(TEXT("Portfolio.DebugOverlay.WeaponPresentation.SocketPair"),0,TEXT("0 automatic/current; 1 authored Holster/Hand; 2 Hand_R_Sword/Hand_L_Sword. Read actual existing socket transforms only."));
 TAutoConsoleVariable<int32> PivotDrawX(TEXT("Portfolio.DebugOverlay.WeaponPresentation.PivotDrawX"),0,TEXT("Show the small actual local X direction in compact pivot mode."));
 TAutoConsoleVariable<int32> HideInactiveTrail(TEXT("Portfolio.DebugOverlay.WeaponPresentation.HideInactiveTrail"),0,TEXT("Omit world anchors of inactive or invisible Trail components; does not change Trail state."));
 TAutoConsoleVariable<int32> Sockets(TEXT("Portfolio.DebugOverlay.WeaponPresentation.DrawSockets"),0,TEXT("Active transition: blue source, orange target. Inactive: orange current attached socket only. White actual weapon root; live socket positions."));
 TAutoConsoleVariable<int32> Pivot(TEXT("Portfolio.DebugOverlay.WeaponPresentation.DrawPivot"),0,TEXT("Actual pivot RGB axes labelled X/Y/Z; grey rest X/Y/Z (Y0/Z0), cyan actual X. Rest follows the live root transform."));
 TAutoConsoleVariable<int32> Trail(TEXT("Portfolio.DebugOverlay.WeaponPresentation.DrawTrailAnchor"),0,TEXT("Actual cached Trail component anchors; green active/visible, grey inactive, red invalid attachment."));
 TAutoConsoleVariable<int32> History(TEXT("Portfolio.DebugOverlay.WeaponPresentation.DrawHistory"),0,TEXT("Recorded weapon root path, not a blade collision path."));
 TAutoConsoleVariable<int32> Foreground(TEXT("Portfolio.DebugOverlay.WeaponPresentation.DrawForeground"),0,TEXT("Draw diagnostics through geometry."));
 TAutoConsoleVariable<float> Retain(TEXT("Portfolio.DebugOverlay.WeaponPresentation.RetainSeconds"),2.f,TEXT("Retain last outcomes and root samples in world seconds (0..10)."));
 TAutoConsoleVariable<int32> FeedbackEnabled(TEXT("Portfolio.DebugOverlay.Feedback.Enabled"),0,TEXT("Capture basic Action/Reaction/Hit feedback evidence, independently from audit logs."));
 TAutoConsoleVariable<int32> PlayerFeedback(TEXT("Portfolio.DebugOverlay.Player.Feedback.Enabled"),0,TEXT("Show player feedback records."));
 TAutoConsoleVariable<int32> EnemyFeedback(TEXT("Portfolio.DebugOverlay.Enemy.Feedback.Enabled"),0,TEXT("Show focused enemy feedback records."));
 TAutoConsoleVariable<int32> Audit(TEXT("Portfolio.Debug.WeaponPresentationAudit"),0,TEXT("Log weapon begin/end/rejection/restoration. No per-frame log."));
 struct FChannel
 {
  uint32 Handle=0, Scope=0;
  float Alpha=0, Elapsed=-1, Duration=0, Time=0;
  FName Source, Target;
  FWeaponPivotRotationSpec Spec;
  FString Phase, Result;
 };
 struct FSample { FVector Position; float Time; };
 struct FEvent { FString Text; float Time; };
 struct FRecord
 {
  TWeakObjectPtr<UWorld> World;
  FChannel Current[3], Last[3];
  TArray<FSample> Samples;
  TArray<FEvent> Events;
  FString Restore;
  float RestoreTime=-100;
  TWeakObjectPtr<const ACWeaponActor> SocketPairWeapon;
  FName PairSource, PairTarget;
 };
 TMap<TWeakObjectPtr<const AActor>,FRecord> Records;
 const TCHAR* ChannelName(EWeaponDebugChannel Channel)
 {
  switch(Channel) { case EWeaponDebugChannel::Socket:return TEXT("Socket"); case EWeaponDebugChannel::Pivot:return TEXT("Pivot"); default:return TEXT("Override"); }
 }
 void ClearWeapon(IConsoleVariable*)
 {
  for (auto& Pair : Records) { for(int32 i=0;i<3;++i) {Pair.Value.Current[i]=FChannel();Pair.Value.Last[i]=FChannel();} Pair.Value.Samples.Reset();Pair.Value.Restore.Reset();Pair.Value.SocketPairWeapon.Reset();Pair.Value.PairSource=NAME_None;Pair.Value.PairTarget=NAME_None; }
 }
 void ClearFeedback(IConsoleVariable*) { for(auto& Pair:Records) Pair.Value.Events.Reset(); }
 void Initialize()
 {
  static bool Initialized=false;
  if(Initialized) return;
  Initialized=true;
  Enabled.AsVariable()->SetOnChangedCallback(FConsoleVariableDelegate::CreateStatic(&ClearWeapon));
  FeedbackEnabled.AsVariable()->SetOnChangedCallback(FConsoleVariableDelegate::CreateStatic(&ClearFeedback));
  FWorldDelegates::OnWorldCleanup.AddStatic(&FWeaponPresentationDebug::ClearWorld);
 }
 FRecord* Get(const AActor* Owner)
 {
  if(!IsValid(Owner)||!IsValid(Owner->GetWorld())) return nullptr;
  Initialize();
  for(auto It=Records.CreateIterator();It;++It) if(!It.Key().IsValid()||!It.Value().World.IsValid()) It.RemoveCurrent();
  // Bound actor histories; do not evict an unrelated actor on every frame.
  if(!Records.Contains(Owner)&&Records.Num()>=64) return nullptr;
  FRecord& Record=Records.FindOrAdd(Owner);Record.World=Owner->GetWorld();return &Record;
 }
 float Retention() { return FMath::Clamp(Retain.GetValueOnGameThread(),0.f,10.f); }
 bool Fresh(float Now,float Then) {return Now>=Then&&Now-Then<=Retention();}
 void Log(const UCWeaponComponent* Component,const FString& Detail)
 {
  if(Audit.GetValueOnGameThread()!=0) UE_LOG(LogTemp,Log,TEXT("[WeaponPresentation] Owner=%s | %s"),*GetNameSafe(Component?Component->GetOwner():nullptr),*Detail);
 }
 void Line(FDebugOverlayTextPanel& Panel,const FString& Text,bool Header=false)
 {
  // The shared main panel does not wrap. Keep diagnostic evidence within its width.
  FString Remaining=Text;
  while(!Header&&Remaining.Len()>78)
  {
   int32 Cut=78;
   for(int32 i=78;i>=40;--i)if(Remaining[i]==TEXT(' ')){Cut=i;break;}
   Panel.Lines.Emplace(Remaining.Left(Cut),EDebugOverlayTextLineRole::Normal);
   Remaining=Remaining.Mid(Cut).TrimStart();
  }
  Panel.Lines.Emplace(Remaining,Header?EDebugOverlayTextLineRole::PanelHeader:EDebugOverlayTextLineRole::Normal);
 }
 bool OtherGate(const TCHAR* Name)
 {
  const IConsoleVariable* Var=IConsoleManager::Get().FindConsoleVariable(Name);return Var&&Var->GetInt()!=0;
 }
}

#endif

bool FWeaponPresentationDebug::IsEnabled()
{
#if !UE_BUILD_SHIPPING
 return WeaponPresentationDebug::Enabled.GetValueOnGameThread()!=0;
#else
 return false;
#endif
}
bool FWeaponPresentationDebug::IsFeedbackEnabled()
{
#if !UE_BUILD_SHIPPING
 return WeaponPresentationDebug::FeedbackEnabled.GetValueOnGameThread()!=0;
#else
 return false;
#endif
}
void FWeaponPresentationDebug::Begin(const UCWeaponComponent* C,EWeaponDebugChannel Channel)
{
#if !UE_BUILD_SHIPPING
 if(!IsValid(C))return;
 WeaponPresentationDebug::Log(C,FString::Printf(TEXT("%s Begin | Scope=%u"),WeaponPresentationDebug::ChannelName(Channel),C->ActionPoseScopeState.ActiveHandle));
 if(!IsEnabled())return;
 if(WeaponPresentationDebug::FRecord* R=WeaponPresentationDebug::Get(C->GetOwner()))
 {
  WeaponPresentationDebug::FChannel& D=R->Current[int32(Channel)];D=WeaponPresentationDebug::FChannel();D.Scope=C->ActionPoseScopeState.ActiveHandle;D.Time=C->GetWorld()->GetTimeSeconds();
  if(Channel==EWeaponDebugChannel::Socket) {D.Handle=C->SocketTransformTransitionState.ActiveHandle;D.Source=C->SocketTransformTransitionState.SourceSocketName;D.Target=C->SocketTransformTransitionState.TargetSocketName;R->SocketPairWeapon=C->GetWeaponActor();R->PairSource=D.Source;R->PairTarget=D.Target;}
  else if(Channel==EWeaponDebugChannel::Pivot) {D.Handle=C->PivotRotationTransitionState.ActiveHandle;D.Spec=C->PivotRotationTransitionState.RotationSpec;}
  else {D.Handle=C->PivotRotationOverrideState.ActiveHandle;D.Spec=C->PivotRotationOverrideState.RotationSpec;}
  if(Channel==EWeaponDebugChannel::Socket)R->Samples.Reset();
 }
#endif
}
void FWeaponPresentationDebug::Update(const UCWeaponComponent* C,EWeaponDebugChannel Channel,float Alpha)
{
#if !UE_BUILD_SHIPPING
 if(!IsEnabled()||!IsValid(C))return;
 if(WeaponPresentationDebug::FRecord* R=WeaponPresentationDebug::Get(C->GetOwner()))
 {
  WeaponPresentationDebug::FChannel& D=R->Current[int32(Channel)];
  const uint32 Handle=Channel==EWeaponDebugChannel::Socket?C->SocketTransformTransitionState.ActiveHandle:Channel==EWeaponDebugChannel::Pivot?C->PivotRotationTransitionState.ActiveHandle:C->PivotRotationOverrideState.ActiveHandle;
  if(Handle==0)return;
  if(D.Handle!=Handle)Begin(C,Channel);
  D.Alpha=FMath::Clamp(Alpha,0.f,1.f);D.Time=C->GetWorld()->GetTimeSeconds();
  if(WeaponPresentationDebug::History.GetValueOnGameThread()!=0&&IsValid(C->WeaponActor))
  {
   const FVector P=C->WeaponActor->GetActorLocation();
   if(R->Samples.IsEmpty()||D.Time-R->Samples.Last().Time>=.03f)
   {if(R->Samples.Num()>=96)R->Samples.RemoveAt(0);R->Samples.Add({P,D.Time});}
  }
 }
#endif
}
void FWeaponPresentationDebug::NotifyTime(const UCWeaponComponent* C,EWeaponDebugChannel Channel,uint32 Handle,float Elapsed,float Duration,const TCHAR* Phase)
{
#if !UE_BUILD_SHIPPING
 if(!IsEnabled()||!IsValid(C))return;
 if(WeaponPresentationDebug::FRecord* R=WeaponPresentationDebug::Get(C->GetOwner())) {WeaponPresentationDebug::FChannel& D=R->Current[int32(Channel)];if(Handle&&D.Handle==Handle){D.Elapsed=Elapsed;D.Duration=Duration;D.Phase=Phase;}}
#endif
}
void FWeaponPresentationDebug::End(const UCWeaponComponent* C,EWeaponDebugChannel Channel,const TCHAR* Result)
{
#if !UE_BUILD_SHIPPING
 if(!IsValid(C))return;
 WeaponPresentationDebug::Log(C,FString::Printf(TEXT("%s %s"),WeaponPresentationDebug::ChannelName(Channel),Result));
 if(!IsEnabled())return;
 if(WeaponPresentationDebug::FRecord* R=WeaponPresentationDebug::Get(C->GetOwner()))
 {
  WeaponPresentationDebug::FChannel& D=R->Current[int32(Channel)];
  // Never turn an unobserved end into an invented start/progress record.
  WeaponPresentationDebug::FChannel& Last=R->Last[int32(Channel)];Last=D;Last.Result=Result;Last.Time=C->GetWorld()->GetTimeSeconds();D=WeaponPresentationDebug::FChannel();
 }
#endif
}
bool FWeaponPresentationDebug::Reject(const UCWeaponComponent* C,EWeaponDebugChannel Channel,const TCHAR* Reason)
{
#if !UE_BUILD_SHIPPING
 if(!IsValid(C))return false;
 WeaponPresentationDebug::Log(C,FString::Printf(TEXT("%s Rejected: %s"),WeaponPresentationDebug::ChannelName(Channel),Reason));
 if(IsEnabled()) if(WeaponPresentationDebug::FRecord* R=WeaponPresentationDebug::Get(C->GetOwner()))
 {WeaponPresentationDebug::FChannel& Last=R->Last[int32(Channel)];Last=WeaponPresentationDebug::FChannel();Last.Result=FString::Printf(TEXT("Rejected: %s"),Reason);Last.Time=C->GetWorld()->GetTimeSeconds();}
#endif
 return false;
}
void FWeaponPresentationDebug::ScopeRestored(const UCWeaponComponent* C,uint32 Scope,FName Socket,const FQuat& Rotation,bool CanRestore,bool HasSocket)
{
#if !UE_BUILD_SHIPPING
 if(!IsValid(C)||(!IsEnabled()&&WeaponPresentationDebug::Audit.GetValueOnGameThread()==0))return;
 const ACWeaponActor* W=C->GetWeaponActor();FName Actual;
 const bool SocketOK=CanRestore&&HasSocket&&W&&W->GetCurrentOwnerSocketName(Actual)&&Actual==Socket
  &&W->ActorRootComponent&&W->ActorRootComponent->GetRelativeTransform().Equals(FTransform::Identity,.01f);
 const bool PivotOK=CanRestore&&W&&W->HasValidPivot()&&W->PivotComponent
  &&W->PivotComponent->GetRelativeTransform().Equals(FTransform(Rotation)*W->PivotRestTransform,.01f);
 const FString Text=FString::Printf(TEXT("Scope %u restore | Socket=%s | Pivot=%s"),Scope,
  !HasSocket?TEXT("NoBaseline"):SocketOK?TEXT("Verified"):TEXT("Failed"),PivotOK?TEXT("Verified"):TEXT("Unavailable/Failed"));
 WeaponPresentationDebug::Log(C,Text);
 if(IsEnabled())if(WeaponPresentationDebug::FRecord* R=WeaponPresentationDebug::Get(C->GetOwner())) {R->Restore=Text;R->RestoreTime=C->GetWorld()->GetTimeSeconds();}
#endif
}
void FWeaponPresentationDebug::Feedback(const AActor* Owner,const TCHAR* Route,const TCHAR* Channel,const FString& Detail)
{
#if !UE_BUILD_SHIPPING
 if(!IsFeedbackEnabled())return;
 if(WeaponPresentationDebug::FRecord* R=WeaponPresentationDebug::Get(Owner))
 {
  if(R->Events.Num()>=24)R->Events.RemoveAt(0);
  R->Events.Add({FString::Printf(TEXT("%s/%s: %s"),Route,Channel,*Detail),float(Owner->GetWorld()->GetTimeSeconds())});
 }
#endif
}
void FWeaponPresentationDebug::SpawnResult(const AActor* Owner,const TCHAR* Route,const UObject* Asset,const UNiagaraComponent* Spawned)
{
#if !UE_BUILD_SHIPPING
 if(!IsFeedbackEnabled())return;
 Feedback(Owner,Route,TEXT("VFX"),FString::Printf(TEXT("%s | %s"),IsValid(Spawned)?TEXT("ComponentCreated"):TEXT("NoComponent"),*GetNameSafe(Asset)));
#endif
}
void FWeaponPresentationDebug::ClearWorld(UWorld* World,bool,bool)
{
#if !UE_BUILD_SHIPPING
 for(auto It=WeaponPresentationDebug::Records.CreateIterator();It;++It)if(!It.Value().World.IsValid()||It.Value().World.Get()==World)It.RemoveCurrent();
#endif
}
void FWeaponPresentationDebug::AppendAndDraw(const AActor* Owner,bool bEnemy,FDebugOverlayTextPanel& Panel)
{
#if !UE_BUILD_SHIPPING
 if(!IsValid(Owner)||!WeaponPresentationDebug::OtherGate(bEnemy?TEXT("Portfolio.DebugOverlay.Enemy.Enabled"):TEXT("Portfolio.DebugOverlay.Player.Enabled")))return;
 const bool ShowWeapon=IsEnabled()&&(bEnemy?WeaponPresentationDebug::Enemy:WeaponPresentationDebug::Player).GetValueOnGameThread()!=0;
 const bool ShowFeedback=IsFeedbackEnabled()&&(bEnemy?WeaponPresentationDebug::EnemyFeedback:WeaponPresentationDebug::PlayerFeedback).GetValueOnGameThread()!=0;
 if(!ShowWeapon&&!ShowFeedback)return;
 const UCWeaponComponent* C=Owner->FindComponentByClass<UCWeaponComponent>();
 const ACWeaponActor* W=C?C->GetWeaponActor():nullptr;
 WeaponPresentationDebug::FRecord* R=WeaponPresentationDebug::Get(Owner);const float Now=Owner->GetWorld()->GetTimeSeconds();
 const uint8 Depth=WeaponPresentationDebug::Foreground.GetValueOnGameThread()!=0?SDPG_Foreground:SDPG_World;
 UWorld* World=Owner->GetWorld();
 const int32 Mode=FMath::Clamp(WeaponPresentationDebug::PresentationMode.GetValueOnGameThread(),0,3);
 if(ShowWeapon)
 {
  if(Mode!=3)WeaponPresentationDebug::Line(Panel,FString::Printf(TEXT("[%s %s]"),bEnemy?TEXT("Enemy"):TEXT("Player"),Mode==1?TEXT("Weapon Socket"):Mode==2?TEXT("Weapon Pivot Override"):TEXT("Weapon Presentation")),true);
  if(!C||!W) { if(Mode!=3)WeaponPresentationDebug::Line(Panel,TEXT("Current: Weapon unavailable")); }
  else
  {
   FName Socket;W->GetCurrentOwnerSocketName(Socket);
   if(Mode==0)WeaponPresentationDebug::Line(Panel,FString::Printf(TEXT("Current: %s | Attached=%s | Scope=%u"),*UEnum::GetValueAsString(C->CurrentWeaponType),*Socket.ToString(),C->ActionPoseScopeState.ActiveHandle));
   if(Mode==1)WeaponPresentationDebug::Line(Panel,FString::Printf(TEXT("Attached now: %s"),*Socket.ToString()));
   if(Mode==0&&W->HasValidPivot()&&W->PivotComponent)
    WeaponPresentationDebug::Line(Panel,FString::Printf(TEXT("Pivot relative NOW: %s"),*W->PivotComponent->GetRelativeRotation().ToCompactString()));
   if(Mode==0)if(const UCActionComponent* Action=Owner->FindComponentByClass<UCActionComponent>())
     WeaponPresentationDebug::Line(Panel,FString::Printf(TEXT("Action=%s[%d] | ActionGen=%llu"),*UEnum::GetValueAsString(Action->GetActiveActionType()),Action->GetActiveActionIndex(),Action->GetActiveActionGeneration()));
   const uint32 Handles[]={C->SocketTransformTransitionState.ActiveHandle,C->PivotRotationTransitionState.ActiveHandle,C->PivotRotationOverrideState.ActiveHandle};
   for(int32 i=0;i<3;++i)
   {
    if(Mode==3||(Mode==1&&i!=0)||(Mode==2&&i!=2))continue;
    const auto Channel=EWeaponDebugChannel(i);const WeaponPresentationDebug::FChannel* D=R?&R->Current[i]:nullptr;
    if(!Handles[i])WeaponPresentationDebug::Line(Panel,FString::Printf(TEXT("%s: Inactive"),WeaponPresentationDebug::ChannelName(Channel)));
    else if(!D||D->Handle!=Handles[i])WeaponPresentationDebug::Line(Panel,FString::Printf(TEXT("%s: Active H%u | Progress unobserved"),WeaponPresentationDebug::ChannelName(Channel),Handles[i]));
    else
    {
     WeaponPresentationDebug::Line(Panel,FString::Printf(TEXT("%s: Active H%u | BlendAlpha=%.2f %s"),WeaponPresentationDebug::ChannelName(Channel),D->Handle,D->Alpha,*D->Phase));
     if(D->Elapsed>=0)WeaponPresentationDebug::Line(Panel,FString::Printf(TEXT("Notify elapsed %.2f / %.2f s"),D->Elapsed,D->Duration));
     if(i==0)WeaponPresentationDebug::Line(Panel,FString::Printf(TEXT("%s -> %s (live socket transforms)"),*D->Source.ToString(),*D->Target.ToString()));
     else if(D->Spec.Mode==EWeaponPivotRotationMode::SignedLocalAxisAngle)
      WeaponPresentationDebug::Line(Panel,FString::Printf(TEXT("Signed axis %s | requested %+.1f deg | weighted %+.1f"),*D->Spec.LocalAxis.GetSafeNormal().ToCompactString(),D->Spec.SignedAngleDegrees,D->Spec.SignedAngleDegrees*D->Alpha));
     else WeaponPresentationDebug::Line(Panel,FString::Printf(TEXT("Target orientation %s"),*D->Spec.TargetOrientation.ToCompactString()));
    }
    if(R&&!R->Last[i].Result.IsEmpty()&&WeaponPresentationDebug::Fresh(Now,R->Last[i].Time))WeaponPresentationDebug::Line(Panel,FString::Printf(TEXT("Last %s: %s (%.2fs ago)"),WeaponPresentationDebug::ChannelName(Channel),*R->Last[i].Result,Now-R->Last[i].Time));
   }
   if(Mode==0&&R&&!R->Restore.IsEmpty()&&WeaponPresentationDebug::Fresh(Now,R->RestoreTime))WeaponPresentationDebug::Line(Panel,R->Restore);
   if(WeaponPresentationDebug::Sockets.GetValueOnGameThread()!=0)
   {
    const bool TransitionActive=Handles[0]!=0;
    const int32 PairMode=FMath::Clamp(WeaponPresentationDebug::SocketPair.GetValueOnGameThread(),0,2);
    const bool CompactSockets=Mode==1||PairMode!=0;
    FName Names[]={NAME_None,NAME_None};
    if(TransitionActive)
    {
     // During movement these are intended endpoints, never a completion claim.
     Names[0]=C->SocketTransformTransitionState.SourceSocketName;
     Names[1]=C->SocketTransformTransitionState.TargetSocketName;
    }
    else if(PairMode==1)
    {
     Names[0]=W->SocketName_Holster;Names[1]=W->SocketName_Hand;
    }
    else if(PairMode==2)
    {
     // Explicit display selection for this project's sword sockets. Missing names
     // are omitted by GetOwnerSocketWorldTransform; no substitute positions exist.
     Names[0]=FName(TEXT("Hand_R_Sword"));Names[1]=FName(TEXT("Hand_L_Sword"));
    }
    else if(Mode==1)
    {
     if(R&&R->SocketPairWeapon.Get()==W&&!R->PairSource.IsNone()&&!R->PairTarget.IsNone())
     {Names[0]=R->PairSource;Names[1]=R->PairTarget;}
     else {Names[0]=W->SocketName_Holster;Names[1]=W->SocketName_Hand;}
    }
    else Names[1]=Socket; // Legacy automatic display: current attachment only.
    if(Names[0]==Names[1])Names[0]=NAME_None;
    FTransform T[2];bool Valid[2]={false,false};
    for(int32 i=0;i<2;++i)
    {
     Valid[i]=W->GetOwnerSocketWorldTransform(Names[i],T[i]);if(!Valid[i])continue;
     const bool AttachedNow=!TransitionActive&&!Socket.IsNone()&&Names[i]==Socket;
     const FColor Color=TransitionActive?(i==0?FColor(90,160,255):FColor(255,180,80)):
      CompactSockets?(AttachedNow?FColor(80,240,170):FColor(150,160,170)):FColor(255,180,80);
     const float Radius=CompactSockets?2.3f:4.f;
     DrawDebugSphere(World,T[i].GetLocation(),Radius,10,Color,false,0,Depth,1.f);
     const FString Label=CompactSockets?FString::Printf(TEXT("%s%s"),*Names[i].ToString(),
      TransitionActive?(i==0?TEXT(" [Source]"):TEXT(" [Target]")):AttachedNow?TEXT(" [Attached]"):TEXT("")):Names[i].ToString();
     DrawDebugString(World,T[i].GetLocation()+FVector(0,0,CompactSockets?10.f:8.f),Label,nullptr,Color,0,true,CompactSockets?1.15f:1.f);
    }
    if(TransitionActive&&Valid[0]&&Valid[1])DrawDebugLine(World,T[0].GetLocation(),T[1].GetLocation(),FColor::Silver,false,0,Depth,1);
    const FVector Origin=W->ActorRootComponent?W->ActorRootComponent->GetComponentLocation():W->GetActorLocation();
    DrawDebugPoint(World,Origin,9,FColor::White,false,0,Depth);
    if(CompactSockets)
    {
     // True measured root center. Cross arms extend beyond the socket sphere so
     // coincident attachment/origin markers remain distinguishable in projection.
     const FVector Arms[]={FVector(3.5f,0,0),FVector(0,3.5f,0),FVector(0,0,3.5f)};
     for(const FVector& Arm:Arms)DrawDebugLine(World,Origin-Arm,Origin+Arm,FColor::White,false,0,Depth,1.f);
     DrawDebugString(World,Origin-FVector(0,0,12),TEXT("Origin"),nullptr,FColor::White,0,true,1.15f);
    }
   }
   if(WeaponPresentationDebug::Pivot.GetValueOnGameThread()!=0&&W->HasValidPivot()&&W->PivotComponent&&W->ActorRootComponent)
   {
    const FTransform Actual=W->PivotComponent->GetComponentTransform();
    const FTransform Rest=W->PivotRestTransform*W->ActorRootComponent->GetComponentTransform();
    if(Mode==2)
    {
     const FVector Origin=W->ActorRootComponent->GetComponentLocation();
     const FVector Center=Actual.GetLocation();
     const bool Coincident=Origin.Equals(Center,.01f);
     DrawDebugPoint(World,Origin,9,FColor::White,false,0,Depth);
     if(!Coincident)DrawDebugPoint(World,Center,7,FColor(255,180,80),false,0,Depth);
     DrawDebugString(World,Origin-FVector(0,0,12),Coincident?TEXT("Origin / Pivot"):TEXT("Origin"),nullptr,FColor::White,0,true);
     if(!Coincident)
     {
      DrawDebugString(World,Center+FVector(0,0,9),TEXT("Pivot"),nullptr,FColor(255,180,80),0,true);
      DrawDebugLine(World,Origin,Center,FColor(105,115,125),false,0,Depth,.6f);
     }
     // Rest follows the CURRENT root; it is not a frozen world-space direction.
     // Rotation never alters either displayed center. Do not move markers apart.
     DrawDebugDirectionalArrow(World,Rest.GetLocation(),Rest.GetLocation()+Rest.GetUnitAxis(EAxis::Y)*30,4,FColor::Silver,false,0,Depth,.8f);
     DrawDebugString(World,Rest.GetLocation()+Rest.GetUnitAxis(EAxis::Y)*35,TEXT("Y0"),nullptr,FColor::Silver,0,true);
     DrawDebugDirectionalArrow(World,Center,Center+Actual.GetUnitAxis(EAxis::Y)*25,5,FColor::Green,false,0,Depth,2.f);
     DrawDebugString(World,Center+Actual.GetUnitAxis(EAxis::Y)*28,TEXT("Y"),nullptr,FColor::Green,0,true);
     if(WeaponPresentationDebug::PivotDrawX.GetValueOnGameThread()!=0)
     {
      DrawDebugDirectionalArrow(World,Center,Center+Actual.GetUnitAxis(EAxis::X)*12,3,FColor::Red,false,0,Depth,.8f);
      DrawDebugString(World,Center+Actual.GetUnitAxis(EAxis::X)*15,TEXT("X"),nullptr,FColor::Red,0,true);
     }
    }
    else
    {
    DrawDebugCoordinateSystem(World,Actual.GetLocation(),Actual.Rotator(),18,false,0,Depth,1.5f);
    DrawDebugDirectionalArrow(World,Rest.GetLocation(),Rest.GetLocation()+Rest.GetUnitAxis(EAxis::X)*28,5,FColor::Silver,false,0,Depth,1.5f);
    DrawDebugDirectionalArrow(World,Actual.GetLocation(),Actual.GetLocation()+Actual.GetUnitAxis(EAxis::X)*35,5,FColor::Cyan,false,0,Depth,2);
    // A rotation about local X leaves the X direction unchanged. The actual Y/Z
    // axes must be compared with their rest directions to make that roll visible.
    // Rest uses the authored pivot rest pose under the CURRENT root transform;
    // it is not a frozen world-space direction or a synthesized rotation result.
    const EAxis::Type Axes[]={EAxis::X,EAxis::Y,EAxis::Z};
    const TCHAR* AxisLabels[]={TEXT("X"),TEXT("Y"),TEXT("Z")};
    const FColor AxisColors[]={FColor::Red,FColor::Green,FColor::Blue};
    for(int32 i=0;i<3;++i)
    {
     DrawDebugString(World,Actual.GetLocation()+Actual.GetUnitAxis(Axes[i])*22,AxisLabels[i],nullptr,AxisColors[i],0,true);
     if(i==0)continue;
     const FVector RestAxis=Rest.GetUnitAxis(Axes[i]);
     DrawDebugDirectionalArrow(World,Rest.GetLocation(),Rest.GetLocation()+RestAxis*28,4,FColor::Silver,false,0,Depth,.65f);
     DrawDebugString(World,Rest.GetLocation()+RestAxis*33,i==1?TEXT("Y0"):TEXT("Z0"),nullptr,FColor::Silver,0,true);
    }
    }
   }
   if(R&&WeaponPresentationDebug::History.GetValueOnGameThread()!=0)
    for(int32 i=1;i<R->Samples.Num();++i)if(WeaponPresentationDebug::Fresh(Now,R->Samples[i-1].Time))DrawDebugLine(World,R->Samples[i-1].Position,R->Samples[i].Position,FColor::White,false,0,Depth,1);
  }
 }
 if(ShowFeedback&&(Mode==0||Mode==3))
 {
  WeaponPresentationDebug::Line(Panel,FString::Printf(TEXT("[%s Feedback / Recent Evidence]"),bEnemy?TEXT("Enemy"):TEXT("Player")),true);
  int32 Count=0;
  if(R)for(int32 i=R->Events.Num()-1;i>=0&&Count<(Mode==3?3:6);--i)if(WeaponPresentationDebug::Fresh(Now,R->Events[i].Time))
  {WeaponPresentationDebug::Line(Panel,FString::Printf(TEXT("%.2fs ago | %s"),Now-R->Events[i].Time,*R->Events[i].Text));++Count;}
  if(!Count)WeaponPresentationDebug::Line(Panel,TEXT("No recent observed feedback; enable before playback."));
 }
 if(W&&(ShowFeedback||ShowWeapon))
 {
  int32 Cached=0,Active=0,Visible=0,Invalid=0;
  for(const auto& Pair:W->TrailComponents_Cached)for(const auto& Ptr:Pair.Value)
  {
   const UWeaponTrailComponent* TrailComp=Ptr.Get();if(!IsValid(TrailComp))continue;
   ++Cached;Active+=TrailComp->IsActive()?1:0;Visible+=TrailComp->IsVisible()?1:0;Invalid+=TrailComp->IsTrailAttachmentValid()?0:1;
   if(ShowWeapon&&WeaponPresentationDebug::Trail.GetValueOnGameThread()!=0
    &&(WeaponPresentationDebug::HideInactiveTrail.GetValueOnGameThread()==0||(TrailComp->IsActive()&&TrailComp->IsVisible())))
   {
    const FColor Color=!TrailComp->IsTrailAttachmentValid()?FColor::Red:TrailComp->IsActive()&&TrailComp->IsVisible()?FColor::Green:FColor::Silver;
    DrawDebugSphere(World,TrailComp->GetComponentLocation(),3,8,Color,false,0,Depth,1);
    DrawDebugString(World,TrailComp->GetComponentLocation()+FVector(0,0,12),FString::Printf(TEXT("Trail %s / %s"),*Pair.Key.ToString(),*TrailComp->GetAttachSocketName().ToString()),nullptr,Color,0,true);
   }
  }
  if(Mode==0||(Mode==3&&ShowFeedback))WeaponPresentationDebug::Line(Panel,FString::Printf(TEXT("Trail NOW: cached=%d active=%d visible=%d invalid=%d"),Cached,Active,Visible,Invalid));
 }
#endif
}
