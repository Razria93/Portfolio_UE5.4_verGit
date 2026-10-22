// Editor-only, transient capture fixture. No animation, Notify or action data edits.
#include "CoreMinimal.h"
#include "CReplayCompletion.h"
#include "CReplayOutcomeObserver.h"
#include "CRecordingLifecycle.h"
#include "Core/Debug/FDebugOverlayFocusRuntimeHelper.h"
#include "DrawDebugHelpers.h"
#include "Core/Debug/FDebugOverlaySnapshotStore.h"
#include "Components/CapsuleComponent.h"
#include "HAL/IConsoleManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISense_Sight.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "UObject/UnrealType.h"
#include "UObject/StrongObjectPtr.h"
#include "Containers/Ticker.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformMisc.h"
#include "Editor.h"
#include "LevelEditorViewport.h"
#include "PlayInEditorDataTypes.h"
#include "Settings/LevelEditorPlaySettings.h"
#include "Character/Player/CPlayer.h"
#include "Character/Enemy/CEnemy.h"
#include "Component/CActionComponent.h"
#include "Component/CWeaponComponent.h"
#include "Weapon/CWeaponActor.h"
#include "Component/CCombatTargetComponent.h"
#include "Component/CTargetLockAssistComponent.h"
#include "Component/CExecutionCollaborationComponent.h"
#include "Component/CBalanceComponent.h"
#include "Component/CHealthComponent.h"
#include "Component/CReactionComponent.h"
#include "Core/Debug/CDebugOverlayFocusComponent.h"
#include "Type/CCombatResultTypes.h"
#include "AI/Patrol/CPatrolPath.h"
#include "Type/CEnemyAIConfigTypes.h"

DEFINE_LOG_CATEGORY_STATIC(LogStructure10Capture, Log, All);

namespace Structure10Capture
{
struct FSession
{
    TWeakObjectPtr<UWorld> World;
    TWeakObjectPtr<ACPlayer> Player;
    TWeakObjectPtr<ACEnemy> Fixture;
    TStrongObjectPtr<UCReplayOutcomeObserver> OutcomeObserver;
    TWeakObjectPtr<ACEnemy> Second;
    int32 DebugStep = 0;
    double DebugEpoch = 0;
    FVector RunGoal = FVector::ZeroVector;
    bool bRunningEntrance = false;
    TWeakObjectPtr<AActor> PreviousTarget;
    TWeakObjectPtr<APlayerController> ObserverController;
    TWeakObjectPtr<AActor> PreviousViewTarget;
    TWeakObjectPtr<ACameraActor> ObserverCamera;
    TWeakObjectPtr<UCTargetLockAssistComponent> ObserverLockAssist;
    bool bPreviousLockAssistTick = false;
    TWeakObjectPtr<UCDebugOverlayFocusComponent> DebugFocus;
    TWeakObjectPtr<AActor> PreviousDebugFocusActor;
    EDebugOverlayFocusSource PreviousDebugFocusSource = EDebugOverlayFocusSource::None;
    EDebugOverlayFocusDriver PreviousDebugFocusDriver = EDebugOverlayFocusDriver::None;
    FTimerHandle Timer;
    FDelegateHandle Cleanup;
    FString Mode;
    FString PreviousSnapshot;
    double Started = 0.0;
    double Deadline = 0.0;
    double NextInput = 0.0;
    double TerminalAt = -1.0;
    int32 Stage = 0;
    int32 MaxComboHits = 3;
    int32 MaxObservedComboIndex = INDEX_NONE;
    bool bEquippedObserved = false;
    bool bUnequippedObserved = false;
    uint64 LastReservedGeneration = 0;
    bool bEverActionActive = false;
    bool bOwnsTargetSelection = false;
    TWeakObjectPtr<AActor> OwnedDebugFocusActor;
    TWeakObjectPtr<AActor> OwnedCombatTarget;
    EDebugOverlayFocusSource OwnedDebugFocusSource = EDebugOverlayFocusSource::None;
    EDebugOverlayFocusDriver OwnedDebugFocusDriver = EDebugOverlayFocusDriver::None;
    PortfolioRecording::FCVarOverride LiveSyncOverride;

    void RememberOwnedFocus()
    {
        if (!DebugFocus.IsValid()) return;
        OwnedDebugFocusActor = DebugFocus->GetDebugOverlayFocusActor();
        OwnedDebugFocusSource = DebugFocus->GetDebugOverlayFocusSource();
        OwnedDebugFocusDriver = DebugFocus->GetDebugOverlayFocusDriver();
    }

    void Log(const FString& Text) const
    {
        const double Now = World.IsValid() ? World->GetTimeSeconds() : Started;
        UE_LOG(LogStructure10Capture, Display, TEXT("[Capture10] Mode=%s World=%.6f Shot=%.6f Phase=%d %s"),
            *Mode, Now, Now - Started, Stage, *Text);
    }
    void Observe()
    {
        if (!Player.IsValid()) return;
        const UCActionComponent* Action = Player->GetActionComp();
        if (Stage == 4 && Action->IsActiveActionType(EActionType::ComboAttack))
            MaxObservedComboIndex = FMath::Max(MaxObservedComboIndex, Action->GetActiveActionIndex());
        const UCWeaponComponent* Weapon = Player->GetWeaponComp();
        const UCExecutionCollaborationComponent* Pair = Player->GetExecutionCollaborationComp();
        const FString Snapshot = FString::Printf(TEXT("Action=%s Index=%d Gen=%llu Scope=%u Weapon=%s Pair=%s Target=%s"),
            *UEnum::GetValueAsString(Action->GetActiveActionType()), Action->GetActiveActionIndex(),
            Action->GetActiveActionGeneration(), Weapon->GetActiveWeaponActionPoseScopeHandle(),
            *UEnum::GetValueAsString(Weapon->GetCurrentWeaponType()),
            *UEnum::GetValueAsString(Pair->GetExecutionCollaborationState()),
            *GetNameSafe(Player->GetCombatTargetComp()->GetCombatTargetActor()));
        if (Snapshot != PreviousSnapshot)
        {
            Log(TEXT("OBSERVED ") + Snapshot);
            PreviousSnapshot = Snapshot;
        }
    }
};
static TUniquePtr<FSession> Session;
static bool bUnattendedFinished = false;
static bool bTakeCompleted = false;

static void Stop(bool bWorldCleanup = false)
{
    if (!Session) return;
    bUnattendedFinished = true;
    Session->Log(bWorldCleanup ? TEXT("STOP WorldCleanup") : TEXT("STOP CompletedOrManual"));
    if (Session->World.IsValid()) Session->World->GetTimerManager().ClearTimer(Session->Timer);
    FWorldDelegates::OnWorldCleanup.Remove(Session->Cleanup);
    if (Session->OutcomeObserver.IsValid()) Session->OutcomeObserver->Unbind();
    if (!bWorldCleanup && Session->ObserverCamera.IsValid())
    {
        // Restore only a view target still owned by this fixture. Never rotate the
        // controller/pawn or overwrite a subsequent user-selected view target.
        APlayerController* PC = Session->ObserverController.Get();
        if (PC && PC->GetViewTarget() == Session->ObserverCamera.Get())
        {
            AActor* RestoreTarget = Session->PreviousViewTarget.Get();
            if (!RestoreTarget) RestoreTarget = Session->Player.Get();
            if (RestoreTarget) PC->SetViewTarget(RestoreTarget);
        }
        Session->ObserverCamera->Destroy();
    }
    if (!bWorldCleanup && Session->ObserverLockAssist.IsValid())
    {
        // Camera tracking derives its desired control rotation from the active
        // view point. Resume only after the observer view has been restored.
        Session->ObserverLockAssist->SetComponentTickEnabled(Session->bPreviousLockAssistTick);
    }
    // OnWorldCleanup owns destruction of the transient observer with its world.
    if (!bWorldCleanup && Session->DebugFocus.IsValid())
    {
        UCDebugOverlayFocusComponent* Focus = Session->DebugFocus.Get();
        const EDebugOverlayFocusDriver Driver = Focus->GetDebugOverlayFocusDriver();
        // Restore only our diagnostic selection, before restoring the combat target.
        const bool bOwnsDebugSelection = Session->Mode == TEXT("debug")
            && Focus->GetDebugOverlayFocusActor() == Session->OwnedDebugFocusActor.Get()
            && Focus->GetDebugOverlayFocusSource() == Session->OwnedDebugFocusSource
            && Driver == Session->OwnedDebugFocusDriver;
        if (bOwnsDebugSelection || (Session->Mode != TEXT("debug") && Focus->GetDebugOverlayFocusActor() == Session->Fixture.Get()
            && (Driver == EDebugOverlayFocusDriver::PlayerTargetLive || Driver == EDebugOverlayFocusDriver::PlayerTargetFrozen || (Session->Mode == TEXT("patrol") && Driver == EDebugOverlayFocusDriver::ManualOutliner))))
        {
            Focus->SetDebugOverlayFocusActorAndSource(Session->PreviousDebugFocusActor.Get(), Session->PreviousDebugFocusSource);
            Focus->SetDebugOverlayFocusDriver(Session->PreviousDebugFocusDriver);
        }
    }
    Session->LiveSyncOverride.Restore();
    if (!bWorldCleanup && Session->Player.IsValid() && Session->bOwnsTargetSelection)
    {
        UCCombatTargetComponent* Target = Session->Player->GetCombatTargetComp();
        // Do not overwrite a user's subsequent selection.
        const AActor* OwnedTarget = Session->Mode == TEXT("debug") ? Session->OwnedCombatTarget.Get() : Session->Fixture.Get();
        if (Target && Target->GetCombatTargetActor() == OwnedTarget)
        {
            if (Session->PreviousTarget.IsValid()) Target->RequestSetCombatTarget(Session->PreviousTarget.Get(), ECombatTargetChangeReason::PlayerSelection);
            else Target->RequestClearCombatTarget(ECombatTargetChangeReason::ManualClear);
        }
    }
    if (!bWorldCleanup && Session->Second.IsValid()) Session->Second->Destroy();
    if (!bWorldCleanup && Session->Fixture.IsValid()) Session->Fixture->Destroy();
    Session.Reset();
}

static bool StartExecutionObserver()
{
    FSession& S = *Session;
    if (S.Mode != TEXT("execution") || !FParse::Param(FCommandLine::Get(), TEXT("Structure10SideCamera"))) return true;
    APlayerController* PC = Cast<APlayerController>(S.Player->GetController());
    if (!PC || !S.Fixture.IsValid()) { S.Log(TEXT("FAIL SideCameraControllerOrFixtureMissing")); return false; }
    UCTargetLockAssistComponent* LockAssist = PC->FindComponentByClass<UCTargetLockAssistComponent>();
    if (!LockAssist) { S.Log(TEXT("FAIL SideCameraLockAssistMissing")); return false; }

    // Offsets are centimetres relative to the player's initial root and yaw basis.
    // Aim at both capsule centres plus a small upper-body bias; remain fixed for
    // the entire action so root motion and local pivot rotation are observable.
    float Right = 550.f, Forward = -100.f, Height = 200.f, AimHeight = 40.f, YawOffset = 0.f, FOV = 55.f;
    const auto ReadFinite = [](const TCHAR* Key, float& Value, float Min, float Max)
    {
        float Parsed = Value;
        if (FParse::Value(FCommandLine::Get(), Key, Parsed) && FMath::IsFinite(Parsed)) Value = FMath::Clamp(Parsed, Min, Max);
    };
    ReadFinite(TEXT("Structure10SideRight="), Right, -2000.f, 2000.f);
    ReadFinite(TEXT("Structure10SideForward="), Forward, -2000.f, 2000.f);
    ReadFinite(TEXT("Structure10SideHeight="), Height, 0.f, 1500.f);
    ReadFinite(TEXT("Structure10SideAimHeight="), AimHeight, -100.f, 300.f);
    ReadFinite(TEXT("Structure10SideYaw="), YawOffset, -180.f, 180.f);
    ReadFinite(TEXT("Structure10SideFOV="), FOV, 25.f, 100.f);
    const FRotator Basis(0.f, S.Player->GetActorRotation().Yaw + YawOffset, 0.f);
    const FVector Origin = S.Player->GetActorLocation();
    const FVector Location = Origin + Basis.RotateVector(FVector(Forward, Right, Height));
    const FVector Aim = (Origin + S.Fixture->GetActorLocation()) * 0.5f + FVector(0.f, 0.f, AimHeight);
    if (FVector::DistSquared(Location, Aim) < FMath::Square(100.f))
    {
        S.Log(TEXT("FAIL SideCameraTooCloseToAim"));
        return false;
    }
    FActorSpawnParameters Params;
    Params.ObjectFlags |= RF_Transient;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    ACameraActor* Camera = S.World->SpawnActor<ACameraActor>(ACameraActor::StaticClass(), Location, (Aim - Location).Rotation(), Params);
    if (!Camera) { S.Log(TEXT("FAIL SideCameraSpawn")); return false; }
    Camera->Tags.Add(TEXT("Structure10TransientObserver"));
    Camera->GetCameraComponent()->SetFieldOfView(FOV);
    Camera->GetCameraComponent()->bConstrainAspectRatio = false;
    S.ObserverController = PC;
    S.PreviousViewTarget = PC->GetViewTarget();
    S.ObserverCamera = Camera;
    // The normal lock camera uses GetPlayerViewPoint to drive ControlRotation.
    // A side observer would otherwise steer ControllerDesired away from the
    // stationary fixture and fail the real execution facing-angle gate. Suspend
    // only that camera-tracking Tick; keep target selection, rotation policy,
    // control rotation value, movement and real action entry untouched.
    S.ObserverLockAssist = LockAssist;
    S.bPreviousLockAssistTick = LockAssist->IsComponentTickEnabled();
    LockAssist->SetComponentTickEnabled(false);
    PC->SetViewTarget(Camera);
    S.Log(FString::Printf(TEXT("OBSERVER SideCamera Location=%s Aim=%s FOV=%.1f Right=%.1f Forward=%.1f Height=%.1f AimHeight=%.1f YawOffset=%.1f; fixed view only, control rotation unchanged"),
        *Location.ToString(), *Aim.ToString(), FOV, Right, Forward, Height, AimHeight, YawOffset));
    S.Log(TEXT("OBSERVER Lock-camera tracking Tick suspended until observer cleanup; no ControlRotation assignment"));
    return true;
}

static bool CreateFixture()
{
    FSession& S = *Session;
    ACPlayer* Player = S.Player.Get();
    UClass* EnemyClass = LoadClass<ACEnemy>(nullptr, TEXT("/Game/01_Character/02_Enemy/BP_CEnemy.BP_CEnemy_C"));
    if (!EnemyClass) { S.Log(TEXT("FAIL EnemyClassMissing")); return false; }
    float Distance = S.Mode == TEXT("execution") ? 180.f : 135.f;
    if (S.Mode == TEXT("execution"))
    {
        float RequestedDistance = Distance;
        if (FParse::Value(FCommandLine::Get(), TEXT("Structure10ExecutionDistance="), RequestedDistance) && FMath::IsFinite(RequestedDistance))
        {
            const ACEnemy* EnemyDefaults = EnemyClass->GetDefaultObject<ACEnemy>();
            const float MinDistance = Player->GetCapsuleComponent()->GetScaledCapsuleRadius()
                + EnemyDefaults->GetCapsuleComponent()->GetScaledCapsuleRadius() + 2.f;
            Distance = FMath::Clamp(RequestedDistance, MinDistance, 500.f);
            S.Log(FString::Printf(TEXT("FIXTURE DistanceRequested=%.1f MinimumNonOverlap=%.1f SpawnDistance=%.1f"), RequestedDistance, MinDistance, Distance));
        }
    }
    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    Params.ObjectFlags |= RF_Transient;
    ACEnemy* Enemy = S.World->SpawnActor<ACEnemy>(EnemyClass,
        Player->GetActorLocation() + Player->GetActorForwardVector() * Distance,
        FRotator(0.f, Player->GetActorRotation().Yaw + 180.f, 0.f), Params);
    if (!Enemy) { S.Log(TEXT("FAIL FixtureSpawn")); return false; }
    S.Fixture = Enemy;
    FString IsolatedCaptureOutput;
    if (FParse::Param(FCommandLine::Get(), TEXT("Structure10IsolateActors"))
        && FParse::Value(FCommandLine::Get(), TEXT("Structure10Output="), IsolatedCaptureOutput))
    {
        // Only the disposable unattended PIE world. The process exits after this
        // one take; no level/package is saved and manual PIE never enters here.
        int32 HiddenCount = 0;
        for (TActorIterator<ACEnemy> It(S.World.Get()); It; ++It)
        {
            ACEnemy* Other = *It;
            if (Other == Enemy) continue;
            Other->SetActorHiddenInGame(true);
            Other->SetActorEnableCollision(false);
            // Spawned weapons are independent actors; hiding the pawn is not enough.
            TArray<AActor*> Attached;
            Other->GetAttachedActors(Attached, true, true);
            for (TActorIterator<AActor> Owned(S.World.Get()); Owned; ++Owned)
                if (Owned->GetOwner() == Other) Attached.AddUnique(*Owned);
            for (AActor* Child : Attached)
            {
                Child->SetActorHiddenInGame(true);
                Child->SetActorEnableCollision(false);
            }
            if (AAIController* OtherAI = Cast<AAIController>(Other->GetController()))
            {
                OtherAI->StopMovement();
                if (UBrainComponent* Brain = OtherAI->GetBrainComponent()) Brain->StopLogic(TEXT("Isolated capture background"));
            }
            if (UCharacterMovementComponent* Move = Other->GetCharacterMovement()) Move->StopMovementImmediately();
            ++HiddenCount;
        }
        S.Log(FString::Printf(TEXT("ISOLATED BACKGROUND HiddenEnemies=%d; disposable PIE world only"), HiddenCount));
    }
    Enemy->Tags.Add(TEXT("Structure10TransientCaptureFixture"));
    if (AAIController* AI = Cast<AAIController>(Enemy->GetController()))
    {
        AI->StopMovement();
        if (UBrainComponent* Brain = AI->GetBrainComponent()) Brain->StopLogic(TEXT("Structure10 transient capture fixture"));
    }
    if (UCharacterMovementComponent* Movement = Enemy->GetCharacterMovement()) Movement->StopMovementImmediately();
    S.PreviousTarget = Player->GetCombatTargetComp()->GetCombatTargetActor();
    S.bOwnsTargetSelection = Player->GetCombatTargetComp()->RequestSetCombatTarget(Enemy, ECombatTargetChangeReason::PlayerSelection);
    if (!S.bOwnsTargetSelection) { S.Log(TEXT("FAIL SelectFixture")); return false; }
    S.OwnedCombatTarget = Enemy;

    if (S.Mode == TEXT("execution"))
    {
        // Only the disposable fixture's setup is changed. Playback still enters the
        // normal paired request, alignment, reservation, action and Notify gates.
        UCHealthComponent* Health = Enemy->FindComponentByClass<UCHealthComponent>();
        UCBalanceComponent* Balance = Enemy->FindComponentByClass<UCBalanceComponent>();
        UCExecutionCollaborationComponent* Pair = Enemy->FindComponentByClass<UCExecutionCollaborationComponent>();
        if (!Health || !Balance || !Pair) { S.Log(TEXT("FAIL FixtureComponents")); return false; }
        Health->InitializeHealth(100.f, 20.f, EMaxHPUpdatePolicy::ClampCurrent);
        FEnumProperty* Condition = FindFProperty<FEnumProperty>(Pair->GetClass(), TEXT("LethalCondition"));
        FFloatProperty* Ratio = FindFProperty<FFloatProperty>(Pair->GetClass(), TEXT("LethalHealthRatio"));
        if (!Condition || !Ratio) { S.Log(TEXT("FAIL FixtureOutcomeProperties")); return false; }
        Condition->GetUnderlyingProperty()->SetIntPropertyValue(Condition->ContainerPtrToValuePtr<void>(Pair),
            static_cast<int64>(EExecutionLethalCondition::HealthRatio));
        Ratio->SetPropertyValue_InContainer(Pair, 0.25f);
        // Fixture-only accepted parry-result packets enter the normal result pipeline;
        // its real CollapseIn montage must complete before execution can be requested.
        FCombatResultPacket Packet;
        Packet.SourceActor = Enemy;
        Packet.TargetActor = Player;
        Packet.DefenseOutcome = EDamageDefenseOutcome::Parry;
        for (int32 I = 0; I < Balance->GetBalanceThreshold(); ++I)
        {
            Packet.CombatSignalResultSerial = static_cast<uint64>(I + 1);
            Enemy->ReceiveCombatResultPacket(Packet);
        }
        S.Log(TEXT("FIXTURE ONLY: HP=20/100 LethalHealthRatio=.25; injected accepted parry results; awaiting real CollapseIn completion"));
    }
    else
    {
        UCHealthComponent* Health = Enemy->FindComponentByClass<UCHealthComponent>();
        AController* Controller = Player->GetController();
        UCDebugOverlayFocusComponent* Focus = Controller ? Controller->FindComponentByClass<UCDebugOverlayFocusComponent>() : nullptr;
        if (!Health || !Focus) { S.Log(TEXT("FAIL HitFixtureHealthOrDebugFocusMissing")); return false; }
        // Keep the disposable target alive through all four genuine collision hits.
        Health->InitializeHealth(200.f, 200.f, EMaxHPUpdatePolicy::ClampCurrent);
        S.DebugFocus = Focus;
        S.PreviousDebugFocusActor = Focus->GetDebugOverlayFocusActor();
        S.PreviousDebugFocusSource = Focus->GetDebugOverlayFocusSource();
        S.PreviousDebugFocusDriver = Focus->GetDebugOverlayFocusDriver();
        // Public diagnostic component only: select the exact existing combat target.
        // No gameplay AI Focus or enemy controller state is changed here.
        Focus->SetDebugOverlayFocusActorAndSource(Player->GetCombatTargetComp()->GetCombatTargetActor(), EDebugOverlayFocusSource::PlayerTargetFocus);
        Focus->SetDebugOverlayFocusDriver(EDebugOverlayFocusDriver::PlayerTargetLive);
        if (S.Mode == TEXT("debug"))
            FDebugOverlayFocusRuntimeHelper::UpdateFocusPlayerTarget(Focus, Cast<APlayerController>(Player->GetController()));
        S.RememberOwnedFocus();
        if (Focus->GetDebugOverlayFocusActor() != Enemy) { S.Log(TEXT("FAIL HitFixtureDebugFocusMismatch")); return false; }
        S.Log(FString::Printf(TEXT("FIXTURE ONLY: HP=%.1f/%.1f; AI brain stopped; DebugFocus=%s Driver=%s Source=%s; actual weapon collision will produce Hit"),
            Health->GetCurrentHP(), Health->GetMaxHP(), *GetNameSafe(Focus->GetDebugOverlayFocusActor()),
            *Focus->GetDebugOverlayFocusDriverText(), *Focus->GetDebugOverlayFocusSourceText()));
    }
    S.OutcomeObserver.Reset(NewObject<UCReplayOutcomeObserver>(GetTransientPackage()));
    if (!S.OutcomeObserver->Bind(Player, Enemy)) { S.Log(TEXT("FAIL OutcomeObserverMissingComponents")); return false; }
    return StartExecutionObserver();
}


static void TickDebug()
{
    FSession& S=*Session;
    UWorld* World=S.World.Get(); ACPlayer* P=S.Player.Get();
    APlayerController* PC=Cast<APlayerController>(P->GetController());
    auto Cmd=[&](const FString& C)
    {
        const bool bLiveSync = C.StartsWith(TEXT("Portfolio.DebugOverlay.Focus.LiveSyncPlayerTarget "));
        if (bLiveSync) S.LiveSyncOverride.Apply(TEXT("Portfolio.DebugOverlay.Focus.LiveSyncPlayerTarget"), C.EndsWith(TEXT(" 0")) ? 0 : 1);
        else PC->ConsoleCommand(C);
        if (bLiveSync || C.StartsWith(TEXT("DebugOverlaySelect")))
        {
            FDebugOverlayFocusRuntimeHelper::UpdateFocusPlayerTarget(S.DebugFocus.Get(), PC);
            S.RememberOwnedFocus();
        }
        S.Log(TEXT("CONTROL ")+C);
    };
    const double Now=World->GetTimeSeconds();
    if(S.DebugStep==0)
    {
        if(Now<S.Deadline) return;
        P->HandleEquipmentAction(EEquipmentActionIntent::Equip);
        S.DebugStep=1; S.Deadline=Now+3.; return;
    }
    if(S.DebugStep==1)
    {
        if(Now<S.Deadline || P->GetActionComp()->IsActive()) return;
        if(!CreateFixture()){Stop();return;}
        S.Fixture->SetActorLabel(TEXT("Observe_A"));
        FActorSpawnParameters Params; Params.ObjectFlags|=RF_Transient;
        Params.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
        ACEnemy* B=World->SpawnActor<ACEnemy>(S.Fixture->GetClass(),
            S.Fixture->GetActorLocation()+P->GetActorRightVector()*260.f+P->GetActorForwardVector()*100.f,
            S.Fixture->GetActorRotation(),Params);
        if(!B){Stop();return;}
        S.Second=B; B->SetActorLabel(TEXT("Observe_B"));
        if(AAIController* AI=Cast<AAIController>(B->GetController())) {AI->StopMovement(); if(AI->BrainComponent) AI->BrainComponent->StopLogic(TEXT("Capture fixture"));}
        B->GetCharacterMovement()->StopMovementImmediately();
        B->FindComponentByClass<UCHealthComponent>()->InitializeHealth(150,150,EMaxHPUpdatePolicy::ClampCurrent);
        Cmd(TEXT("Portfolio.DebugOverlay.ClearEventLog"));
        S.DebugEpoch=Now; S.DebugStep=2;
        S.Log(FString::Printf(TEXT("FIXTURES A=%s B=%s"),*S.Fixture->GetName(),*B->GetName()));
    }
    const double T=Now-S.DebugEpoch;
    DrawDebugString(World,S.Fixture->GetActorLocation()+FVector(0,0,130),TEXT("A"),nullptr,FColor::Cyan,0,true,1.5f);
    DrawDebugString(World,S.Second->GetActorLocation()+FVector(0,0,130),TEXT("B"),nullptr,FColor::Yellow,0,true,1.5f);
    const int32 Step=S.DebugStep;
    if(Step==2 && T>=1) {Cmd(TEXT("DebugOverlaySelectOutlinerFocus ")+S.Fixture->GetName());S.DebugStep++;}
    if(Step==3 && T>=5) {Cmd(TEXT("DebugOverlaySelectOutlinerFocus ")+S.Second->GetName());S.DebugStep++;}
    if(Step==4 && T>=9) {Cmd(TEXT("DebugOverlaySelectPlayerTargetFocus"));S.DebugStep++;}
    if(Step==5 && T>=13) {Cmd(TEXT("Portfolio.DebugOverlay.Focus.LiveSyncPlayerTarget 0"));S.DebugStep++;}
    if(Step==6 && T>=15) {if(P->GetCombatTargetComp()->RequestSetCombatTarget(S.Second.Get(),ECombatTargetChangeReason::PlayerSelection)) S.OwnedCombatTarget=S.Second;S.Log(TEXT("GAME TARGET=B; observer frozen"));S.DebugStep++;}
    if(Step==7 && T>=18) {Cmd(TEXT("Portfolio.DebugOverlay.Focus.LiveSyncPlayerTarget 1"));S.DebugStep++;}
    if(Step==8 && T>=21) {if(P->GetCombatTargetComp()->RequestSetCombatTarget(S.Fixture.Get(),ECombatTargetChangeReason::PlayerSelection)) S.OwnedCombatTarget=S.Fixture;Cmd(TEXT("DebugOverlaySelectOutlinerFocus ")+S.Fixture->GetName());Cmd(TEXT("Portfolio.DebugOverlay.ClearEventLog"));Cmd(TEXT("Portfolio.DebugOverlay.EventLog.Visible 0"));S.DebugStep++;}
    if(Step==9 && T>=23) {auto R=P->HandleCombatAction(ECombatActionIntent::ComboAttack);S.Log(TEXT("REAL SINGLE ATTACK ")+UEnum::GetValueAsString(R.ResultType));S.DebugStep++;}
    if(Step==10 && T>=27) {Cmd(TEXT("Portfolio.DebugOverlay.EventLog.Visible 1"));S.DebugStep++;}
    if(Step==11 && T>=31) {Cmd(TEXT("Portfolio.DebugOverlay.EventLogScope FocusedEnemy"));S.DebugStep++;}
    if(Step==12 && T>=35) {Cmd(TEXT("Portfolio.DebugOverlay.EventLogFilter Combat"));S.DebugStep++;}
    if(Step==13 && T>=39) {Cmd(TEXT("Portfolio.DebugOverlay.EventLogFilter ActionReaction"));S.DebugStep++;}
    if(Step==14 && T>=43) {Cmd(TEXT("Portfolio.DebugOverlay.EventLogFilter All"));S.DebugStep++;}
    if(Step!=S.DebugStep)
    {
        FString PanelDir; FParse::Value(FCommandLine::Get(),TEXT("Structure10Output="),PanelDir);
        Cmd(FString::Printf(TEXT("Portfolio.Capture10.ExportPanel \"%s/panel_%02d.png\""),*PanelDir,S.DebugStep));
        S.Log(FString::Printf(TEXT("EVIDENCE Focus=%s Driver=%s WorldEvents=%d ActorAEvents=%d ActorBEvents=%d HP_A=%.1f HP_B=%.1f"),
            *GetNameSafe(S.DebugFocus->GetDebugOverlayFocusActor()),*S.DebugFocus->GetDebugOverlayFocusDriverText(),
            FDebugOverlaySnapshotStore::GetRecentEventsCopy(World,256,TEXT("All")).Num(),
            FDebugOverlaySnapshotStore::GetRecentEventsForActorCopy(World,128,TEXT("All"),S.Fixture.Get()).Num(),
            FDebugOverlaySnapshotStore::GetRecentEventsForActorCopy(World,128,TEXT("All"),S.Second.Get()).Num(),
            S.Fixture->FindComponentByClass<UCHealthComponent>()->GetCurrentHP(),S.Second->FindComponentByClass<UCHealthComponent>()->GetCurrentHP()));
    }
    if(T>=47) {bTakeCompleted=true;S.Log(TEXT("TAKE_COMPLETE"));GEngine->Exec(World,TEXT("Portfolio.Capture09.RecordStop"));Stop();}
}


static void TickOverview()
{
    FSession& S=*Session;
    ACPlayer* P=S.Player.Get(); UWorld* World=S.World.Get();
    APlayerController* PC=Cast<APlayerController>(P->GetController());
    auto Cmd=[&](const FString& C){PC->ConsoleCommand(C);S.Log(TEXT("OVERVIEW ")+C);};
    const double Now=World->GetTimeSeconds();
    if(S.DebugStep==0){if(Now<S.Deadline)return;P->HandleEquipmentAction(EEquipmentActionIntent::Equip);S.Deadline=Now+3;S.DebugStep=1;return;}
    if(S.DebugStep==1){if(Now<S.Deadline || P->GetActionComp()->IsActive())return;if(!CreateFixture()){Stop();return;}S.DebugEpoch=Now;S.DebugStep=2;}
    const double T=Now-S.DebugEpoch;
    FString Dir;FParse::Value(FCommandLine::Get(),TEXT("Structure10Output="),Dir);
    auto Panel=[&](const FString& Name){Cmd(FString::Printf(TEXT("Portfolio.Capture10.ExportPanel \"%s/%s.png\""),*Dir,*Name));};
    auto Attack=[&](){const auto Result=P->HandleCombatAction(ECombatActionIntent::ComboAttack);S.Log(TEXT("REAL ATTACK ")+UEnum::GetValueAsString(Result.ResultType));};
    if(S.DebugStep==2 && T>=1){Attack();S.DebugStep++;}
    if(S.DebugStep==3 && T>=5){Panel(TEXT("panel_before"));S.DebugStep++;}
    if(S.DebugStep==4 && T>=8){Cmd(TEXT("Portfolio.Capture10.ToggleEventLog"));Panel(TEXT("panel_off"));S.DebugStep++;}
    if(S.DebugStep==5 && T>=11){Cmd(TEXT("Portfolio.Capture10.ToggleEventLog"));Panel(TEXT("panel_on"));S.DebugStep++;}
    if(S.DebugStep==6 && T>=16){Attack();S.DebugStep++;}
    if(S.DebugStep==7 && T>=24){Attack();S.DebugStep++;}
    if(T>=31){bTakeCompleted=true;S.Log(TEXT("TAKE_COMPLETE"));GEngine->Exec(World,TEXT("Portfolio.Capture09.RecordStop"));Stop();}
}

// Opt-in disposable capture world only; ordinary patrol BT executes unchanged.
static void TickPatrol()
{
    FSession& S=*Session; UWorld* World=S.World.Get(); ACPlayer* P=S.Player.Get();
    const double Now=World->GetTimeSeconds();
    if(S.DebugStep==0)
    {
        if(Now<S.Deadline)return;
        FString Output;
        if(!FParse::Value(FCommandLine::Get(),TEXT("Structure10Output="),Output)){Stop();return;}
        ACPatrolPath* Path=nullptr;
        for(TActorIterator<ACPatrolPath> It(World);It;++It)if(It->Num()>=3){Path=*It;break;}
        if(!Path){S.Log(TEXT("FAIL NoPatrolPath"));Stop();return;}
        FBox Bounds(ForceInit); FPatrolPointSnapshot First;
        for(int I=0;I<Path->Num();++I){FPatrolPointSnapshot Point;if(Path->GetPointSnapshot(I,Point)){Bounds+=Point.Location;S.Log(FString::Printf(TEXT("PATROL P%d %s"),I,*Point.Location.ToString()));}}
        if(!Path->GetPointSnapshot(0,First)){Stop();return;}
        for(TActorIterator<ACEnemy> It(World);It;++It) {
            if(auto* AI=Cast<AAIController>(It->GetController())) {
                if(auto* Perception=AI->FindComponentByClass<UAIPerceptionComponent>()) if(auto* Sight=Cast<UAISenseConfig_Sight>(Perception->GetSenseConfig(UAISense::GetSenseID<UAISense_Sight>()))) S.Log(FString::Printf(TEXT("PLACED_AI Actor=%s Controller=%s Sight=%.1f LoseSight=%.1f FOV=%.1f"),*It->GetName(),*AI->GetClass()->GetName(),Sight->SightRadius,Sight->LoseSightRadius,Sight->PeripheralVisionAngleDegrees));
                AI->StopMovement();if(AI->GetBrainComponent())AI->GetBrainComponent()->StopLogic(TEXT("Disposable patrol capture"));
            }
            It->SetActorHiddenInGame(true);It->SetActorEnableCollision(false);
        }
        float RunX=240.f,RunY=1750.f,GoalX=0.f,GoalY=842.f;
        FParse::Value(FCommandLine::Get(),TEXT("PatrolRunX="),RunX);FParse::Value(FCommandLine::Get(),TEXT("PatrolRunY="),RunY);
        FParse::Value(FCommandLine::Get(),TEXT("PatrolGoalX="),GoalX);FParse::Value(FCommandLine::Get(),TEXT("PatrolGoalY="),GoalY);
        S.RunGoal=FVector(GoalX,GoalY,100.f);
        P->SetActorLocation(FVector(RunX,RunY,100.f),false,nullptr,ETeleportType::TeleportPhysics);
        P->SetActorHiddenInGame(false);P->GetCharacterMovement()->StopMovementImmediately();P->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
        P->FindComponentByClass<UCHealthComponent>()->InitializeHealth(5000.f,5000.f,EMaxHPUpdatePolicy::ClampCurrent);
        S.Log(FString::Printf(TEXT("PLAYER_PRESET Offscreen=%s Goal=%s; movement-input entrance only"),*P->GetActorLocation().ToString(),*S.RunGoal.ToString()));
        UClass* Cls=LoadClass<ACEnemy>(nullptr,TEXT("/Game/01_Character/02_Enemy/BP_CEnemy.BP_CEnemy_C"));
        FTransform T(FRotator::ZeroRotator,First.Location+FVector(0,0,100));
        ACEnemy* Enemy=World->SpawnActorDeferred<ACEnemy>(Cls,T,nullptr,nullptr,ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
        if(!Enemy){Stop();return;} S.Fixture=Enemy;Enemy->SetFlags(RF_Transient);
        FStructProperty* Prop=FindFProperty<FStructProperty>(Enemy->GetClass(),TEXT("PatrolConfig"));
        if(!Prop){Stop();return;}
        auto* Config=Prop->ContainerPtrToValuePtr<FEnemyPatrolConfig>(Enemy);Config->bUsePatrol=true;Config->PatrolPath=Path;Config->PatrolMode=EPatrolMode::Loop;
        UGameplayStatics::FinishSpawningActor(Enemy,T); Enemy->SpawnDefaultController();
        if(auto* Perception=Enemy->GetController()->FindComponentByClass<UAIPerceptionComponent>()) if(auto* Sight=Cast<UAISenseConfig_Sight>(Perception->GetSenseConfig(UAISense::GetSenseID<UAISense_Sight>()))) S.Log(FString::Printf(TEXT("FIXTURE_AI Sight=%.1f LoseSight=%.1f FOV=%.1f; Blueprint defaults unchanged"),Sight->SightRadius,Sight->LoseSightRadius,Sight->PeripheralVisionAngleDegrees));
        float CaptureSight=0.f;
        if(FParse::Value(FCommandLine::Get(),TEXT("PatrolCaptureSight="),CaptureSight) && CaptureSight>0.f) {
            if(auto* Perception=Enemy->GetController()->FindComponentByClass<UAIPerceptionComponent>()) if(auto* Sight=Cast<UAISenseConfig_Sight>(Perception->GetSenseConfig(UAISense::GetSenseID<UAISense_Sight>()))) {
                Sight->SightRadius=CaptureSight;Sight->LoseSightRadius=CaptureSight+300.f;Perception->ConfigureSense(*Sight);Perception->RequestStimuliListenerUpdate();
                S.Log(FString::Printf(TEXT("CAPTURE_ONLY_CALIBRATION Sight=%.1f LoseSight=%.1f; original historical value unknown; native BT and decisions unchanged; no asset save"),Sight->SightRadius,Sight->LoseSightRadius));
            }
        }
        APlayerController* PC=Cast<APlayerController>(P->GetController());
        S.ObserverController=PC;S.PreviousViewTarget=PC->GetViewTarget();
        if(auto* Lock=PC->FindComponentByClass<UCTargetLockAssistComponent>()){S.ObserverLockAssist=Lock;S.bPreviousLockAssistTick=Lock->IsComponentTickEnabled();Lock->SetComponentTickEnabled(false);}
        float CamX=694.31f,CamY=2453.75f,CamZ=1349.22f,CamYaw=-112.57f,CamPitch=-27.89f,CamFov=64.11f;
        FParse::Value(FCommandLine::Get(),TEXT("PatrolCamX="),CamX);FParse::Value(FCommandLine::Get(),TEXT("PatrolCamY="),CamY);FParse::Value(FCommandLine::Get(),TEXT("PatrolCamZ="),CamZ);
        FParse::Value(FCommandLine::Get(),TEXT("PatrolCamYaw="),CamYaw);FParse::Value(FCommandLine::Get(),TEXT("PatrolCamPitch="),CamPitch);FParse::Value(FCommandLine::Get(),TEXT("PatrolCamFov="),CamFov);
        FActorSpawnParameters Params;Params.ObjectFlags|=RF_Transient;
        auto* Camera=World->SpawnActor<ACameraActor>(FVector(CamX,CamY,CamZ),FRotator(CamPitch,CamYaw,0.f),Params);S.ObserverCamera=Camera;Camera->GetCameraComponent()->SetFieldOfView(CamFov);PC->SetViewTarget(Camera);
        S.Log(FString::Printf(TEXT("CAMERA Location=%s Rotation=%s FOV=%.3f"),*Camera->GetActorLocation().ToString(),*Camera->GetActorRotation().ToString(),CamFov));
        if(auto* Focus=PC->FindComponentByClass<UCDebugOverlayFocusComponent>()){S.DebugFocus=Focus;S.PreviousDebugFocusActor=Focus->GetDebugOverlayFocusActor();S.PreviousDebugFocusSource=Focus->GetDebugOverlayFocusSource();S.PreviousDebugFocusDriver=Focus->GetDebugOverlayFocusDriver();Focus->SetDebugOverlayFocusActorAndSource(Enemy,EDebugOverlayFocusSource::OutlinerFocus);Focus->SetDebugOverlayFocusDriver(EDebugOverlayFocusDriver::ManualOutliner);}
        S.Log(TEXT("PATROL_READY transient enemy; level patrol path; normal AI/BT"));S.DebugEpoch=Now;S.DebugStep=1;
    }
    for (TActorIterator<ACWeaponActor> It(World); It; ++It) { if (AActor* Owner=It->GetOwner()) { const bool Hidden=Owner->IsHidden(); if (It->IsHidden()!=Hidden) { S.Log(FString::Printf(TEXT("WEAPON_VISIBILITY Weapon=%s Owner=%s Hidden=%d"),*It->GetName(),*Owner->GetName(),Hidden)); It->SetActorHiddenInGame(Hidden); } } }
    const bool bFollowCombat = FParse::Param(FCommandLine::Get(), TEXT("Structure07PatrolCombat"));
    if (bFollowCombat && S.Fixture.IsValid())
    {
        ACEnemy* Enemy = S.Fixture.Get();
        if(auto* CV=IConsoleManager::Get().FindConsoleVariable(TEXT("Portfolio.DebugOverlay.Patrol.DrawAcceptance"))) {
            auto* AI=Cast<AAIController>(Enemy->GetController());auto* BB=AI?AI->GetBlackboardComponent():nullptr;
            CV->Set((Enemy->GetVelocity().Size2D()<5.f || (BB && BB->GetValueAsEnum(TEXT("AIIntentState"))!=0)) ? 1 : 0,ECVF_SetByConsole);
        }
        float EntranceAt=40.f;FParse::Value(FCommandLine::Get(),TEXT("PatrolEntranceAt="),EntranceAt);
        if(S.DebugStep==1 && Now-S.DebugEpoch>=EntranceAt) { S.DebugStep=2;S.bRunningEntrance=true;S.Log(TEXT("PLAYER_RUN_BEGIN real HandleMove input; no teleport or visibility change")); }
        if(S.bRunningEntrance) {
            const FVector Delta=S.RunGoal-P->GetActorLocation();
            if(Delta.Size2D()>65.f) { Cast<APlayerController>(P->GetController())->SetControlRotation(Delta.Rotation());P->HandleLocomotionGaitInput(false,false);P->HandleMove(FVector2D(0.f,1.f)); }
            else {S.bRunningEntrance=false;S.Log(FString::Printf(TEXT("PLAYER_RUN_INPUT_END Location=%s; normal braking"),*P->GetActorLocation().ToString()));}
        }
        if (Now >= S.NextInput)
        {
            S.NextInput = Now + .1;
            AAIController* AI = Cast<AAIController>(Enemy->GetController());
            UBlackboardComponent* BB = AI ? AI->GetBlackboardComponent() : nullptr;
            const FString Snapshot = FString::Printf(TEXT("PATROL_COMBAT Intent=%d Target=%s Action=%s Index=%d HP=%.1f Enemy=%s Player=%s"),
                BB ? BB->GetValueAsEnum(TEXT("AIIntentState")) : 255,
                *GetNameSafe(Enemy->GetCombatTargetComp()->GetCombatTargetActor()),
                *UEnum::GetValueAsString(Enemy->GetActionComp()->GetActiveActionType()),
                Enemy->GetActionComp()->GetActiveActionIndex(),
                P->FindComponentByClass<UCHealthComponent>()->GetCurrentHP(),
                *Enemy->GetActorLocation().ToString(), *P->GetActorLocation().ToString());
            S.Log(Snapshot);
            FVector2D Header;Cast<APlayerController>(P->GetController())->ProjectWorldLocationToScreen(Enemy->GetActorLocation()+FVector(0,0,120),Header);
            S.Log(FString::Printf(TEXT("HEADER_SCREEN X=%.2f Y=%.2f"),Header.X,Header.Y));
        }
        if (S.DebugStep == 2 && P->FindComponentByClass<UCHealthComponent>()->GetCurrentHP() < 5000.f)
        {
            S.DebugStep = 3; S.TerminalAt = Now + 3.; S.Log(TEXT("FIRST_DAMAGE; retaining recovery tail"));
        }
        if ((S.TerminalAt > 0 && Now >= S.TerminalAt) || Now-S.DebugEpoch >= 75.)
        {bTakeCompleted=true;S.Log(TEXT("TAKE_COMPLETE"));GEngine->Exec(World,TEXT("Portfolio.Capture09.RecordStop"));Stop();}
    }
    else if(Now-S.DebugEpoch>=32){bTakeCompleted=true;S.Log(TEXT("TAKE_COMPLETE"));GEngine->Exec(World,TEXT("Portfolio.Capture09.RecordStop"));Stop();}
}

static void Tick()
{
    if (!Session || !Session->World.IsValid() || !Session->Player.IsValid()) { Stop(true); return; }
    const UCHealthComponent* PlayerHealth = Session->Player->FindComponentByClass<UCHealthComponent>();
    if (!PlayerHealth || !PlayerHealth->IsAlive())
    {
        Session->Log(TEXT("FAIL PlayerDiedDuringScenario"));
        Stop();
        return;
    }
    if(Session->Mode==TEXT("patrol")) { TickPatrol(); return; }
    if(Session->Mode==TEXT("overview")) { TickOverview(); return; }
    if(Session->Mode==TEXT("debug")) { TickDebug(); return; }
    FSession& S = *Session;
    UWorld* World = S.World.Get();
    if (World->IsPaused()) return;
    ACPlayer* P = S.Player.Get();
    UCActionComponent* A = P->GetActionComp();
    UCWeaponComponent* W = P->GetWeaponComp();
    const double Now = World->GetTimeSeconds();
    S.Observe();
    if (Now - S.Started > 40.0) { S.Log(TEXT("FAIL WatchdogTimeout")); Stop(); return; }
    if (S.Stage == 0)
    {
        if (Now < S.Deadline) return;
        const FActionRequestResult R = P->HandleEquipmentAction(S.Mode == TEXT("equip") ? EEquipmentActionIntent::Unequip : EEquipmentActionIntent::Equip);
        S.Log(FString::Printf(TEXT("INPUT PrepareEquipment Result=%s Reject=%s"), *UEnum::GetValueAsString(R.ResultType), *UEnum::GetValueAsString(R.RejectReason)));
        S.Stage = 1;
        S.Deadline = Now + 2.5;
        return;
    }
    if (S.Stage == 1)
    {
        if (Now < S.Deadline || A->IsActive()) return;
        if (S.Mode == TEXT("equip"))
        {
            if (W->GetCurrentWeaponType() != EWeaponType::Unarmed) { S.Log(TEXT("FAIL PrepareUnequipNotObserved")); Stop(); return; }
            const FActionRequestResult R = P->HandleEquipmentAction(EEquipmentActionIntent::Equip);
            S.Log(TEXT("INPUT Equip Result=") + UEnum::GetValueAsString(R.ResultType));
            if (!R.IsAccepted()) { S.Log(TEXT("FAIL EquipRequestRejected")); Stop(); return; }
            S.Stage = 2;
            S.Deadline = Now + 3.0;
            return;
        }
        if (W->GetCurrentWeaponType() == EWeaponType::Unarmed) { S.Log(TEXT("FAIL EquipmentNotReady")); Stop(); return; }
        if (S.Mode == TEXT("execution") || S.Mode == TEXT("hit"))
        {
            if (!CreateFixture()) { Stop(); return; }
            S.Stage = 3;
            S.Deadline = Now + (S.Mode == TEXT("execution") ? 0.25 : 2.0);
            return;
        }
        S.Stage = 3;
        S.Deadline = Now;
    }
    if (S.Stage == 2)
    {
        if (Now < S.Deadline || A->IsActive()) return;
        S.bEquippedObserved = W->GetCurrentWeaponType() != EWeaponType::Unarmed;
        if (!S.bEquippedObserved) { S.Log(TEXT("FAIL EquipTransitionNotObserved")); Stop(); return; }
        const FActionRequestResult R = P->HandleEquipmentAction(EEquipmentActionIntent::Unequip);
        S.Log(TEXT("INPUT Unequip Result=") + UEnum::GetValueAsString(R.ResultType));
        if (!R.IsAccepted()) { S.Log(TEXT("FAIL UnequipRequestRejected")); Stop(); return; }
        S.Stage = 5;
        S.Deadline = Now + 3.0;
        return;
    }
    if (S.Stage == 3)
    {
        if (Now < S.Deadline || A->IsActive()) return;
        if (S.Mode == TEXT("execution"))
        {
            EExecutionAvailabilityBlock Block;
            if (!P->GetExecutionCollaborationComp()->QueryCombatExecutionAvailability(Block))
            {
                if (Now >= S.NextInput)
                {
                    const AActor* Target = P->GetCombatTargetComp()->GetCombatTargetActor();
                    const FVector ToTarget = Target ? (Target->GetActorLocation() - P->GetActorLocation()).GetSafeNormal2D() : FVector::ZeroVector;
                    const float FacingAngle = Target ? FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(FVector::DotProduct(P->GetActorForwardVector().GetSafeNormal2D(), ToTarget), -1.f, 1.f))) : -1.f;
                    S.Log(FString::Printf(TEXT("WAIT ExecutionAvailability=%d Distance2D=%.1f FacingAngle=%.1f ActorYaw=%.1f ControlYaw=%.1f"),
                        static_cast<int32>(Block), Target ? FVector::Dist2D(P->GetActorLocation(), Target->GetActorLocation()) : -1.f,
                        FacingAngle, P->GetActorRotation().Yaw, P->GetControlRotation().Yaw));
                    S.NextInput = Now + 0.5;
                }
                return;
            }
            if (S.Fixture.IsValid()) S.Log(FString::Printf(TEXT("EXECUTION START Distance2D=%.2f PlayerRadius=%.2f EnemyRadius=%.2f"),
                FVector::Dist2D(P->GetActorLocation(), S.Fixture->GetActorLocation()),
                P->GetCapsuleComponent()->GetScaledCapsuleRadius(), S.Fixture->GetCapsuleComponent()->GetScaledCapsuleRadius()));
            const bool bStarted = P->HandleCombatExecution();
            S.Log(FString::Printf(TEXT("INPUT RealCombatExecution Accepted=%d"), bStarted));
            if (!bStarted) { Stop(); return; }
        }
        else
        {
            const FActionRequestResult R = P->HandleCombatAction(ECombatActionIntent::ComboAttack);
            S.Log(TEXT("INPUT ComboStart Result=") + UEnum::GetValueAsString(R.ResultType));
            if (!R.IsAccepted()) { Stop(); return; }
        }
        S.Stage = 4;
        S.bEverActionActive = A->IsActive();
        if (A->IsActiveActionType(EActionType::ComboAttack)) S.MaxObservedComboIndex = A->GetActiveActionIndex();
        S.NextInput = Now + 0.025;
        return;
    }
    if (S.Stage == 4)
    {
        S.bEverActionActive |= A->IsActive();
        if (A->IsActiveActionType(EActionType::ComboAttack) && A->GetActiveActionIndex() < S.MaxComboHits - 1 && Now >= S.NextInput)
        {
            // Repeated normal input is only accepted by the actual Notify-opened
            // reserve window. No direct OpenReserveChainWindow or ConsumeChain call.
            S.NextInput = Now + 0.025;
            if (S.LastReservedGeneration != A->GetActiveActionGeneration())
            {
                const FActionRequestResult R = P->HandleCombatAction(ECombatActionIntent::ComboAttack);
                if (R.IsReservedResult())
                {
                    S.LastReservedGeneration = A->GetActiveActionGeneration();
                    S.Log(TEXT("INPUT ComboReserve Result=Reserved"));
                }
            }
        }
        if (S.bEverActionActive && !A->IsActive() && !P->GetExecutionCollaborationComp()->HasActiveExecutionSession())
        {
            S.TerminalAt = Now;
            S.Stage = 5;
            S.Deadline = Now + 2.5;
            S.Log(TEXT("OBSERVED ActionAndPairTerminal; holding recovery tail 2.5s"));
        }
        return;
    }
    if (S.Stage == 5 && Now >= S.Deadline && !A->IsActive())
    {
        S.bUnequippedObserved = W->GetCurrentWeaponType() == EWeaponType::Unarmed;
        const FString GoalError = PortfolioReplay::ValidateScenarioGoal(S.Mode, S.bEquippedObserved, S.bUnequippedObserved, S.MaxObservedComboIndex,
            S.OutcomeObserver.IsValid() ? S.OutcomeObserver->Outcome : PortfolioReplay::FScenarioOutcome());
        if (!GoalError.IsEmpty()) { S.Log(TEXT("FAIL ") + GoalError); Stop(); return; }
        S.Log(FString::Printf(TEXT("GOAL_VERIFIED Equipped=%d Unequipped=%d MaxComboIndex=%d"), S.bEquippedObserved, S.bUnequippedObserved, S.MaxObservedComboIndex));
        if (S.OutcomeObserver.IsValid())
        {
            const auto& O = S.OutcomeObserver->Outcome;
            S.Log(FString::Printf(TEXT("OUTCOME_VERIFIED Hits=%d Commit=%d SourceComplete=%d TargetComplete=%d Interrupted=%d"),
                O.AcceptedDamageHits, O.bExecutionCommitted, O.bSourceCompleted, O.bTargetCompleted, O.bExecutionInterrupted));
        }
        S.Log(TEXT("TAKE_COMPLETE; stop recorder at/before subsequent fixture cleanup"));
        bTakeCompleted = true;
        if (GEngine) GEngine->Exec(World, TEXT("Portfolio.Capture09.RecordStop"));
        Stop();
    }
}

static void Start(const TArray<FString>& Args, UWorld* World)
{
    if (!World || !World->IsGameWorld() || Args.Num() == 0)
    {
        UE_LOG(LogStructure10Capture, Error, TEXT("Use in PIE/game: Portfolio.Capture10.Start equip|combo|combo2|combo3|execution|hit|debug|overview|patrol [warmupSeconds]"));
        return;
    }
    const FString Mode = Args[0].ToLower();
    if (Mode != TEXT("patrol") && Mode != TEXT("overview") && Mode != TEXT("debug") && Mode != TEXT("equip") && Mode != TEXT("combo") && Mode != TEXT("combo2") && Mode != TEXT("combo3") && Mode != TEXT("execution") && Mode != TEXT("hit")) return;
    ACPlayer* Player = Cast<ACPlayer>(UGameplayStatics::GetPlayerPawn(World, 0));
    if (!Player || !Player->GetActionComp() || !Player->GetWeaponComp() || !Player->GetCombatTargetComp() || !Player->GetExecutionCollaborationComp()) return;
    Stop();
    bUnattendedFinished = false;
    bTakeCompleted = false;
    Session = MakeUnique<FSession>();
    Session->World = World;
    Session->Player = Player;
    Session->Mode = Mode;
    Session->MaxComboHits = Mode == TEXT("combo2") ? 2 : (Mode == TEXT("hit") ? 4 : 3);
    Session->Started = World->GetTimeSeconds();
    Session->Deadline = Session->Started + (Args.Num() > 1 ? FMath::Clamp(FCString::Atof(*Args[1]), 0.f, 30.f) : 3.0f);
    Session->Cleanup = FWorldDelegates::OnWorldCleanup.AddLambda([](UWorld* EndingWorld, bool, bool)
    {
        if (Session && Session->World.Get() == EndingWorld) Stop(true);
    });
    World->GetTimerManager().SetTimer(Session->Timer, FTimerDelegate::CreateStatic(&Tick), 0.01f, true);
    Session->Log(TEXT("START; playrate/action data unchanged; optional execution observer is view-target only; recorder must already run"));
}

static FAutoConsoleCommandWithWorldAndArgs StartCommand(TEXT("Portfolio.Capture10.Start"),
    TEXT("Editor-only real-action capture fixture. equip|combo|combo2|combo3|execution|hit|debug|overview|patrol [warmupSeconds]."),
    FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&Start));
static FAutoConsoleCommand StopCommand(TEXT("Portfolio.Capture10.Stop"), TEXT("Stop capture input fixture and remove only its transient enemy."),
    FConsoleCommandDelegate::CreateLambda([] { Stop(); }));

// Opt-in isolated-editor runner. This path is entirely dormant without BOTH
// Structure10Mode and Structure10Output command-line arguments. Never pass those
// arguments to the user's interactive editor process.
struct FAutoRunner
{
    FString Mode, Output, Preset;
    FTSTicker::FDelegateHandle Ticker;
    TStrongObjectPtr<ULevelEditorPlaySettings> PlaySettings;
    double RunnerStarted = 0.0, FirstReady = 0.0, PIEReady = 0.0, FinishedAt = 0.0, EndRequestedAt = 0.0;
    bool bRequested = false, bStarted = false, bFailed = false;

    FAutoRunner()
    {
        if (!FParse::Value(FCommandLine::Get(), TEXT("Structure10Mode="), Mode)
            || !FParse::Value(FCommandLine::Get(), TEXT("Structure10Output="), Output)) return;
        FParse::Value(FCommandLine::Get(), TEXT("Structure10Preset="), Preset);
        RunnerStarted = FPlatformTime::Seconds();
        Ticker = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateRaw(this, &FAutoRunner::TickAuto), 0.1f);
    }
    ~FAutoRunner()
    {
        if (Ticker.IsValid()) FTSTicker::GetCoreTicker().RemoveTicker(Ticker);
    }
    void ApplyPreset(UWorld* World)
    {
        TArray<FString> Lines;
        if (!Preset.IsEmpty() && FFileHelper::LoadFileToStringArray(Lines, *Preset))
        {
            for (FString Line : Lines)
            {
                Line.TrimStartAndEndInline();
                if (!Line.IsEmpty() && !Line.StartsWith(TEXT("#")) && !Line.StartsWith(TEXT("//")))
                    GEngine->Exec(World, *Line);
            }
        }
        GEngine->Exec(World, TEXT("Portfolio.DebugOverlay.PIE.StartPaused 0"));
        GEngine->Exec(World, TEXT("Portfolio.CaptureCamera.Enabled 0"));
    }
    void SuspendEditorViewportRealtime()
    {
        if (!GEditor) return;
        const FText Owner = FText::FromString(TEXT("Structure10 isolated capture"));
        for (FLevelEditorViewportClient* Client : GEditor->GetLevelViewportClients())
        {
            if (!Client || !Client->GetWorld() || Client->GetWorld()->WorldType != EWorldType::Editor) continue;
            if (!Client->HasRealtimeOverride(Owner))
            {
                // SetRealtime(false) changes the setting saved between sessions.
                // The named override is transient and SaveRealtimeStateToConfig
                // explicitly ignores it. The PIE game viewport is not touched.
                Client->AddRealtimeOverride(false, Owner);
                UE_LOG(LogStructure10Capture, Display, TEXT("[Capture10Auto] Suspended editor viewport realtime (transient override, current process only)"));
            }
        }
    }
    void Fail(const FString& Reason)
    {
        if (bFailed) return;
        bFailed = true;
        bStarted = true;
        bUnattendedFinished = true;
        FinishedAt = FPlatformTime::Seconds();
        const FString Report = FString::Printf(TEXT("FAILED\nMode=%s\nOutput=%s\nReason=%s\nElapsed=%.3f\nNo successful take is claimed.\n"),
            *Mode, *Output, *Reason, FinishedAt - RunnerStarted);
        UE_LOG(LogStructure10Capture, Error, TEXT("[Capture10Auto] FAILED %s"), *Reason);
        // Use a sibling marker, so a failed initialization does not make an empty
        // capture directory which the recorder later mistakes for an existing take.
        IFileManager::Get().MakeDirectory(*FPaths::GetPath(Output), true);
        FFileHelper::SaveStringToFile(Report, *(Output + TEXT(".failed.txt")));
        if (GEngine && GEditor && GEditor->PlayWorld)
            GEngine->Exec(GEditor->PlayWorld, TEXT("Portfolio.Capture09.RecordStop"));
        Stop();
    }
    bool TickAuto(float)
    {
        const double Now = FPlatformTime::Seconds();
        if (!bFailed && Now - RunnerStarted > 120.0) Fail(TEXT("OverallTimeout120Seconds"));
        if (bFailed)
        {
            IConsoleVariable* Busy = IConsoleManager::Get().FindConsoleVariable(TEXT("Portfolio.Capture09.RecorderBusy"));
            if (Busy && Busy->GetInt() != 0 && Now - FinishedAt < 15.0) return true;
            if (GEditor && GEditor->PlayWorld && EndRequestedAt == 0.0)
            {
                GEditor->RequestEndPlayMap();
                EndRequestedAt = Now;
                return true;
            }
            if (GEditor && GEditor->PlayWorld && Now - EndRequestedAt < 5.0) return true;
            FPlatformMisc::RequestExitWithStatus(false, 1);
            return false;
        }
        if (!GEditor || !GEngine || IsAsyncLoading()) return true;
        SuspendEditorViewportRealtime();
        if (!bRequested)
        {
            UWorld* EditorWorld = GEditor->GetEditorWorldContext().World();
            if (!EditorWorld || EditorWorld->WorldType != EWorldType::Editor) return true;
            if (FirstReady == 0.0) { FirstReady = Now; return true; }
            if (Now - FirstReady < 5.0) return true;
            ApplyPreset(EditorWorld);
            PlaySettings.Reset(DuplicateObject<ULevelEditorPlaySettings>(GetDefault<ULevelEditorPlaySettings>(), GetTransientPackage()));
            PlaySettings->SetPlayNetMode(EPlayNetMode::PIE_Standalone);
            PlaySettings->SetPlayNumberOfClients(1);
            PlaySettings->EnableGameSound = true;
            PlaySettings->NewWindowWidth = 1920;
            PlaySettings->NewWindowHeight = 1080;
            FParse::Value(FCommandLine::Get(), TEXT("Structure10Width="), PlaySettings->NewWindowWidth);
            FParse::Value(FCommandLine::Get(), TEXT("Structure10Height="), PlaySettings->NewWindowHeight);
            FRequestPlaySessionParams Params;
            Params.SessionDestination = EPlaySessionDestinationType::InProcess;
            Params.WorldType = EPlaySessionWorldType::PlayInEditor;
            Params.EditorPlaySettings = PlaySettings.Get();
            Params.bAllowOnlineSubsystem = false;
            GEditor->RequestPlaySession(Params);
            bRequested = true;
            UE_LOG(LogStructure10Capture, Display, TEXT("[Capture10Auto] Queued PIE Mode=%s Output=%s"), *Mode, *Output);
            return true;
        }
        if (!bStarted)
        {
            UWorld* PlayWorld = GEditor->PlayWorld;
            if (!PlayWorld || !PlayWorld->HasBegunPlay() || !UGameplayStatics::GetPlayerPawn(PlayWorld, 0)) return true;
            if (PIEReady == 0.0)
            {
                ApplyPreset(PlayWorld);
                UGameplayStatics::SetGamePaused(PlayWorld, false);
                PIEReady = Now;
                return true;
            }
            if (Now - PIEReady < 2.0) return true;
            const FString Record = FString::Printf(TEXT("Portfolio.Capture09.RecordStart \"%s\""), *Output);
            GEngine->Exec(PlayWorld, *Record);
            IConsoleVariable* Busy = IConsoleManager::Get().FindConsoleVariable(TEXT("Portfolio.Capture09.RecorderBusy"));
            if (!Busy || Busy->GetInt() != 1)
            {
                Fail(TEXT("RecorderStartRejectedOrUnavailable; scenario inputs were not started"));
                return true;
            }
            TArray<FString> Args = {Mode, TEXT("3")};
            Start(Args, PlayWorld);
            bStarted = true;
            if (!Session) Fail(TEXT("ScenarioStartRejected"));
            return true;
        }
        if (!bUnattendedFinished)
        {
            const IConsoleVariable* Busy = IConsoleManager::Get().FindConsoleVariable(TEXT("Portfolio.Capture09.RecorderBusy"));
            if (!Busy || Busy->GetInt() == 0)
                Fail(TEXT("RecorderStoppedBeforeScenarioComplete"));
            return true;
        }
        if (!bTakeCompleted)
        {
            Fail(TEXT("ScenarioStoppedBeforeTakeComplete; see Capture10 phase log"));
            return true;
        }
        if (FinishedAt == 0.0)
        {
            FinishedAt = Now;
            if (GEditor->PlayWorld) GEngine->Exec(GEditor->PlayWorld, TEXT("Portfolio.Capture09.RecordStop"));
        }
        // Backend must expose Busy until queued frame writes, encoder and WAV export
        // have drained. If absent, deliberately keep the isolated editor alive.
        IConsoleVariable* Busy = IConsoleManager::Get().FindConsoleVariable(TEXT("Portfolio.Capture09.RecorderBusy"));
        if (!Busy || Busy->GetInt() != 0 || Now - FinishedAt < 2.0) return true;
        if (EndRequestedAt == 0.0)
        {
            const FString ExportError = PortfolioReplay::ValidateExport(Output, FParse::Param(FCommandLine::Get(), TEXT("ReplayAllowSilentAudio")));
            if (!ExportError.IsEmpty())
            {
                Fail(TEXT("RecorderExportRejected:") + ExportError);
                return true;
            }
            GEditor->RequestEndPlayMap();
            EndRequestedAt = Now;
            return true;
        }
        if (GEditor->PlayWorld || Now - EndRequestedAt < 1.0) return true;
        FString ExportSummary;
        FFileHelper::LoadFileToString(ExportSummary, *(Output / TEXT("record_done.txt")));
        if (!FFileHelper::SaveStringToFile(FString(TEXT("SUCCESS\nscenario=complete\nexport=verified\n")) + ExportSummary, *(Output + TEXT(".succeeded.txt"))))
        {
            Fail(TEXT("SuccessMarkerWriteFailed"));
            return true;
        }
        UE_LOG(LogStructure10Capture, Display, TEXT("[Capture10Auto] Scenario and recorder export verified; isolated editor exiting"));
        FPlatformMisc::RequestExit(false);
        return false;
    }
};
static FAutoRunner AutoRunner;
}







