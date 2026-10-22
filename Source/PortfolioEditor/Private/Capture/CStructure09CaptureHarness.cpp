// Editor-only, transient capture fixture. No animation, Notify or action data edits.
#include "CoreMinimal.h"
#include "CReplayCompletion.h"
#include "Components/CapsuleComponent.h"
#include "HAL/IConsoleManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "AIController.h"
#include "BrainComponent.h"
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
#include "Component/CCombatTargetComponent.h"
#include "Component/CTargetLockAssistComponent.h"
#include "Component/CExecutionCollaborationComponent.h"
#include "Component/CBalanceComponent.h"
#include "Component/CHealthComponent.h"
#include "Component/CReactionComponent.h"
#include "Core/Debug/CDebugOverlayFocusComponent.h"
#include "Type/CCombatResultTypes.h"

DEFINE_LOG_CATEGORY_STATIC(LogStructure09Capture, Log, All);

namespace Structure09Capture
{
struct FSession
{
    TWeakObjectPtr<UWorld> World;
    TWeakObjectPtr<ACPlayer> Player;
    TWeakObjectPtr<ACEnemy> Fixture;
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

    void Log(const FString& Text) const
    {
        const double Now = World.IsValid() ? World->GetTimeSeconds() : Started;
        UE_LOG(LogStructure09Capture, Display, TEXT("[Capture09] Mode=%s World=%.6f Shot=%.6f Phase=%d %s"),
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
        if (Focus->GetDebugOverlayFocusActor() == Session->Fixture.Get()
            && (Driver == EDebugOverlayFocusDriver::PlayerTargetLive || Driver == EDebugOverlayFocusDriver::PlayerTargetFrozen))
        {
            Focus->SetDebugOverlayFocusActorAndSource(Session->PreviousDebugFocusActor.Get(), Session->PreviousDebugFocusSource);
            Focus->SetDebugOverlayFocusDriver(Session->PreviousDebugFocusDriver);
        }
    }
    if (!bWorldCleanup && Session->Player.IsValid() && Session->bOwnsTargetSelection)
    {
        UCCombatTargetComponent* Target = Session->Player->GetCombatTargetComp();
        // Do not overwrite a user's subsequent selection.
        if (Target && Target->GetCombatTargetActor() == Session->Fixture.Get())
        {
            if (Session->PreviousTarget.IsValid()) Target->RequestSetCombatTarget(Session->PreviousTarget.Get(), ECombatTargetChangeReason::PlayerSelection);
            else Target->RequestClearCombatTarget(ECombatTargetChangeReason::ManualClear);
        }
    }
    if (!bWorldCleanup && Session->Fixture.IsValid()) Session->Fixture->Destroy();
    Session.Reset();
}

static bool StartExecutionObserver()
{
    FSession& S = *Session;
    if (S.Mode != TEXT("execution") || !FParse::Param(FCommandLine::Get(), TEXT("Structure09SideCamera"))) return true;
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
    ReadFinite(TEXT("Structure09SideRight="), Right, -2000.f, 2000.f);
    ReadFinite(TEXT("Structure09SideForward="), Forward, -2000.f, 2000.f);
    ReadFinite(TEXT("Structure09SideHeight="), Height, 0.f, 1500.f);
    ReadFinite(TEXT("Structure09SideAimHeight="), AimHeight, -100.f, 300.f);
    ReadFinite(TEXT("Structure09SideYaw="), YawOffset, -180.f, 180.f);
    ReadFinite(TEXT("Structure09SideFOV="), FOV, 25.f, 100.f);
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
    Camera->Tags.Add(TEXT("Structure09TransientObserver"));
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
        if (FParse::Value(FCommandLine::Get(), TEXT("Structure09ExecutionDistance="), RequestedDistance) && FMath::IsFinite(RequestedDistance))
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
    if (FParse::Param(FCommandLine::Get(), TEXT("Structure09IsolateActors"))
        && FParse::Value(FCommandLine::Get(), TEXT("Structure09Output="), IsolatedCaptureOutput))
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
    Enemy->Tags.Add(TEXT("Structure09TransientCaptureFixture"));
    if (AAIController* AI = Cast<AAIController>(Enemy->GetController()))
    {
        AI->StopMovement();
        if (UBrainComponent* Brain = AI->GetBrainComponent()) Brain->StopLogic(TEXT("Structure09 transient capture fixture"));
    }
    if (UCharacterMovementComponent* Movement = Enemy->GetCharacterMovement()) Movement->StopMovementImmediately();
    S.PreviousTarget = Player->GetCombatTargetComp()->GetCombatTargetActor();
    S.bOwnsTargetSelection = Player->GetCombatTargetComp()->RequestSetCombatTarget(Enemy, ECombatTargetChangeReason::PlayerSelection);
    if (!S.bOwnsTargetSelection) { S.Log(TEXT("FAIL SelectFixture")); return false; }

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
        if (Focus->GetDebugOverlayFocusActor() != Enemy) { S.Log(TEXT("FAIL HitFixtureDebugFocusMismatch")); return false; }
        S.Log(FString::Printf(TEXT("FIXTURE ONLY: HP=%.1f/%.1f; AI brain stopped; DebugFocus=%s Driver=%s Source=%s; actual weapon collision will produce Hit"),
            Health->GetCurrentHP(), Health->GetMaxHP(), *GetNameSafe(Focus->GetDebugOverlayFocusActor()),
            *Focus->GetDebugOverlayFocusDriverText(), *Focus->GetDebugOverlayFocusSourceText()));
    }
    return StartExecutionObserver();
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
        const FString GoalError = PortfolioReplay::ValidateScenarioGoal(S.Mode, S.bEquippedObserved, S.bUnequippedObserved, S.MaxObservedComboIndex);
        if (!GoalError.IsEmpty()) { S.Log(TEXT("FAIL ") + GoalError); Stop(); return; }
        S.Log(FString::Printf(TEXT("GOAL_VERIFIED Equipped=%d Unequipped=%d MaxComboIndex=%d"), S.bEquippedObserved, S.bUnequippedObserved, S.MaxObservedComboIndex));
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
        UE_LOG(LogStructure09Capture, Error, TEXT("Use in PIE/game: Portfolio.Capture09.Start equip|combo|combo2|combo3|execution|hit [warmupSeconds]"));
        return;
    }
    const FString Mode = Args[0].ToLower();
    if (Mode != TEXT("equip") && Mode != TEXT("combo") && Mode != TEXT("combo2") && Mode != TEXT("combo3") && Mode != TEXT("execution") && Mode != TEXT("hit")) return;
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

static FAutoConsoleCommandWithWorldAndArgs StartCommand(TEXT("Portfolio.Capture09.Start"),
    TEXT("Editor-only real-action capture fixture. equip|combo|combo2|combo3|execution|hit [warmupSeconds]."),
    FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&Start));
static FAutoConsoleCommand StopCommand(TEXT("Portfolio.Capture09.Stop"), TEXT("Stop capture input fixture and remove only its transient enemy."),
    FConsoleCommandDelegate::CreateLambda([] { Stop(); }));

// Opt-in isolated-editor runner. This path is entirely dormant without BOTH
// Structure09Mode and Structure09Output command-line arguments. Never pass those
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
        if (!FParse::Value(FCommandLine::Get(), TEXT("Structure09Mode="), Mode)
            || !FParse::Value(FCommandLine::Get(), TEXT("Structure09Output="), Output)) return;
        FParse::Value(FCommandLine::Get(), TEXT("Structure09Preset="), Preset);
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
        const FText Owner = FText::FromString(TEXT("Structure09 isolated capture"));
        for (FLevelEditorViewportClient* Client : GEditor->GetLevelViewportClients())
        {
            if (!Client || !Client->GetWorld() || Client->GetWorld()->WorldType != EWorldType::Editor) continue;
            if (!Client->HasRealtimeOverride(Owner))
            {
                // SetRealtime(false) changes the setting saved between sessions.
                // The named override is transient and SaveRealtimeStateToConfig
                // explicitly ignores it. The PIE game viewport is not touched.
                Client->AddRealtimeOverride(false, Owner);
                UE_LOG(LogStructure09Capture, Display, TEXT("[Capture09Auto] Suspended editor viewport realtime (transient override, current process only)"));
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
        UE_LOG(LogStructure09Capture, Error, TEXT("[Capture09Auto] FAILED %s"), *Reason);
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
            FParse::Value(FCommandLine::Get(), TEXT("Structure09Width="), PlaySettings->NewWindowWidth);
            FParse::Value(FCommandLine::Get(), TEXT("Structure09Height="), PlaySettings->NewWindowHeight);
            FRequestPlaySessionParams Params;
            Params.SessionDestination = EPlaySessionDestinationType::InProcess;
            Params.WorldType = EPlaySessionWorldType::PlayInEditor;
            Params.EditorPlaySettings = PlaySettings.Get();
            Params.bAllowOnlineSubsystem = false;
            GEditor->RequestPlaySession(Params);
            bRequested = true;
            UE_LOG(LogStructure09Capture, Display, TEXT("[Capture09Auto] Queued PIE Mode=%s Output=%s"), *Mode, *Output);
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
            Fail(TEXT("ScenarioStoppedBeforeTakeComplete; see Capture09 phase log"));
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
        UE_LOG(LogStructure09Capture, Display, TEXT("[Capture09Auto] Scenario and recorder export verified; isolated editor exiting"));
        FPlatformMisc::RequestExit(false);
        return false;
    }
};
static FAutoRunner AutoRunner;
}
