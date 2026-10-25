#include "CParryReplayObserver.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Component/CActionComponent.h"
#include "Component/CWeaponComponent.h"
#include "Component/CDefenseComponent.h"
#include "Component/CCombatSignalTargetComponent.h"
#include "Weapon/CWeaponActor.h"

DEFINE_LOG_CATEGORY_STATIC(LogParryReplayDiagnostic, Log, All);

bool UCParryReplayObserver::Bind(ACharacter* Attacker, ACharacter* Defender)
{
    Unbind();
    Diagnostic = PortfolioReplay::FParryReplayDiagnostic();
    bBindingIncomplete = false; WindowId = INDEX_NONE;
    InputAt = ContactAt = OpenAt = CloseAt = -1; GuardDelay = -1;
    Source = Attacker; Target = Defender;
    Signal = Defender ? Defender->FindComponentByClass<UCCombatSignalTargetComponent>() : nullptr;
    if (!IsValid(Attacker) || !IsValid(Defender) || !Signal.IsValid()) return false;
    AcceptedHandle = Signal->OnCombatSignalTargetAccepted.AddUObject(this, &UCParryReplayObserver::OnAccepted);
    Sample();
    return true;
}

void UCParryReplayObserver::Unbind()
{
    if (Weapon.IsValid())
    {
        Weapon->OnWeaponActorCollisionEnabled.RemoveDynamic(this, &UCParryReplayObserver::OnWindowOpened);
        Weapon->OnWeaponActorCollisionDisabled.RemoveDynamic(this, &UCParryReplayObserver::OnWindowClosed);
        Weapon->OnWeaponActorBeginOverlap.RemoveDynamic(this, &UCParryReplayObserver::OnContact);
    }
    if (Signal.IsValid()) Signal->OnCombatSignalTargetAccepted.Remove(AcceptedHandle);
    Weapon.Reset(); Signal.Reset(); Source.Reset(); Target.Reset();
}

double UCParryReplayObserver::Now() const { return Source.IsValid() ? Source->GetWorld()->GetTimeSeconds() : -1; }

bool UCParryReplayObserver::IsFirstAttack() const
{
    const auto* Action = Source.IsValid() ? Source->FindComponentByClass<UCActionComponent>() : nullptr;
    return Action && Action->IsActiveActionType(EActionType::ComboAttack)
        && Diagnostic.Matches(Action->GetActiveActionGeneration(), Action->GetActiveActionIndex());
}

void UCParryReplayObserver::Sample()
{
    if (!Source.IsValid()) return;
    const auto* Action = Source->FindComponentByClass<UCActionComponent>();
    const auto* Weapons = Source->FindComponentByClass<UCWeaponComponent>();
    ACWeaponActor* Current = Weapons ? Weapons->GetWeaponActor() : nullptr;
    if (!Weapon.IsValid() && Current)
    {
        Weapon = Current;
        // A listener installed after a hit window cannot establish absence of contact.
        bBindingIncomplete = Current->GetCurrentHitWindowId() != INDEX_NONE;
        Current->OnWeaponActorCollisionEnabled.AddDynamic(this, &UCParryReplayObserver::OnWindowOpened);
        Current->OnWeaponActorCollisionDisabled.AddDynamic(this, &UCParryReplayObserver::OnWindowClosed);
        Current->OnWeaponActorBeginOverlap.AddDynamic(this, &UCParryReplayObserver::OnContact);
    }
    if (Weapon.IsValid() && Current != Weapon.Get()) bBindingIncomplete = true;
    if (Action && Diagnostic.Generation == 0 && Action->IsActiveActionType(EActionType::ComboAttack)
        && Action->GetActiveActionIndex() == 0)
    {
        Diagnostic.Generation = Action->GetActiveActionGeneration(); Diagnostic.Index = 0;
        FActionData Data; Action->GetActiveActionData(Data);
        const auto* Anim = Source->GetMesh()->GetAnimInstance();
        auto* Instance = Anim && Data.Montage ? Anim->GetActiveInstanceForMontage(Data.Montage) : nullptr;
        UE_LOG(LogParryReplayDiagnostic, Display, TEXT("[ParryDiagnostic] Attack World=%.6f Generation=%llu Index=0 Montage=%s Instance=%d PlayRate=%.6f RateScale=%.6f"),
            Now(), Diagnostic.Generation, *GetNameSafe(Data.Montage), Instance ? Instance->GetInstanceID() : INDEX_NONE,
            Instance ? Instance->GetPlayRate() : Data.PlayRate, Data.Montage ? Data.Montage->RateScale : 0.f);
    }
}

void UCParryReplayObserver::RecordInput(double Time, float Delay) { InputAt = Time; GuardDelay = Delay; }

void UCParryReplayObserver::OnWindowOpened()
{
    Sample();
    if (!IsFirstAttack()) return;
    Diagnostic.bWindowOpened = true; WindowId = Weapon->GetCurrentHitWindowId(); OpenAt = Now();
    UE_LOG(LogParryReplayDiagnostic, Display, TEXT("[ParryDiagnostic] WindowOpen World=%.6f Generation=%llu Window=%d"), OpenAt, Diagnostic.Generation, WindowId);
}
void UCParryReplayObserver::OnWindowClosed()
{
    // Parry can end the source action before CollisionDisabled broadcasts.
    if (WindowId == INDEX_NONE || !Weapon.IsValid() || Weapon->GetCurrentHitWindowId() != WindowId) return;
    Diagnostic.bWindowClosed = true; CloseAt = Now();
    UE_LOG(LogParryReplayDiagnostic, Display, TEXT("[ParryDiagnostic] WindowClose World=%.6f Generation=%llu Window=%d"), CloseAt, Diagnostic.Generation, WindowId);
}

void UCParryReplayObserver::OnContact(AActor* Attacker, AActor* Causer, UShapeComponent*, AActor* Defender,
    UPrimitiveComponent*, int32, bool, const FHitResult&)
{
    Sample();
    if (Attacker != Source.Get() || Defender != Target.Get() || Causer != Weapon.Get() || !IsFirstAttack() || Diagnostic.bAccepted) return;
    // ACWeaponActor publishes this event synchronously before RequestCombatSignalSource.
    const auto* Defense = Target->FindComponentByClass<UCDefenseComponent>();
    if (!Defense) { bBindingIncomplete = true; return; }
    const auto* Action = Source->FindComponentByClass<UCActionComponent>();
    if (!Diagnostic.RecordContact(Action->GetActiveActionGeneration(), Action->GetActiveActionIndex(),
        GFrameCounter, Weapon->GetCurrentHitWindowId(), Defense->CanParry())) return;
    ContactAt = Now();
    UE_LOG(LogParryReplayDiagnostic, Display, TEXT("[ParryDiagnostic] Contact World=%.6f Generation=%llu Index=0 Window=%d Frame=%llu ParryOpen=%d"),
        ContactAt, Diagnostic.Generation, Diagnostic.ContactWindowId, Diagnostic.ContactFrame, Diagnostic.bParryOpenAtContact);
}

void UCParryReplayObserver::OnAccepted(const FCombatSignalTargetPacket& Packet)
{
    if (!Diagnostic.bContact || Diagnostic.bAccepted || !IsFirstAttack() || !Weapon.IsValid()
        || Packet.Payload.SourceActor != Source.Get() || Packet.Payload.TargetActor != Target.Get()
        || Packet.Payload.DamageCauser != Weapon.Get() || Packet.Payload.DamageSpecKey.ActionType != EActionType::ComboAttack
        || Packet.Payload.DamageSpecKey.ActionIndex != Diagnostic.Index) return;
    const auto* Action = Source->FindComponentByClass<UCActionComponent>();
    const bool bEligiblePolicy = Packet.Payload.DamageSpec.bCanParry
        && Packet.Context.ExternalInputPolicy == EExternalCombatInputPolicy::Normal
        && Packet.Context.DeadState_Before == EDeadState::Alive
        && Packet.Result.DefenseOutcome != EDamageDefenseOutcome::Dodge
        && Packet.Result.DefenseOutcome != EDamageDefenseOutcome::PerfectDodge;
    if (!Diagnostic.RecordAccepted(Action->GetActiveActionGeneration(), Packet.Payload.DamageSpecKey.ActionIndex,
        GFrameCounter, Weapon->GetCurrentHitWindowId(), Packet.Result.bAccepted, bEligiblePolicy,
        Packet.Result.DefenseOutcome == EDamageDefenseOutcome::Parry)) return;
    UE_LOG(LogParryReplayDiagnostic, Display, TEXT("[ParryDiagnostic] Accepted World=%.6f Serial=%llu Defense=%s Classification=%s"),
        Now(), Packet.ResultSerial, *UEnum::GetValueAsString(Packet.Result.DefenseOutcome), Diagnostic.Classify());
}

bool UCParryReplayObserver::CanComplete() const
{
    return Diagnostic.CanComplete(!bBindingIncomplete && Source.IsValid() && Target.IsValid()
        && Weapon.IsValid() && Signal.IsValid() && AcceptedHandle.IsValid());
}

FString UCParryReplayObserver::Describe() const
{
    return FString::Printf(TEXT("Classification=%s Generation=%llu Index=%d Window=%d ContactWindow=%d Contact=%d Accepted=%d ParryOpenAtContact=%d InputAt=%.6f GuardDelay=%.6f ContactAt=%.6f WindowOpenedAt=%.6f WindowClosedAt=%.6f"),
        bBindingIncomplete ? TEXT("IncompleteObservation") : Diagnostic.Classify(), Diagnostic.Generation, Diagnostic.Index,
        WindowId, Diagnostic.ContactWindowId, Diagnostic.bContact, Diagnostic.bAccepted, Diagnostic.bParryOpenAtContact,
        InputAt, GuardDelay, ContactAt, OpenAt, CloseAt);
}
