#include "CReplayOutcomeObserver.h"
#include "GameFramework/Character.h"
#include "Component/CActionComponent.h"
#include "Component/CReactionComponent.h"
#include "Component/CExecutionCollaborationComponent.h"
#include "Component/CCombatSignalTargetComponent.h"

bool UCReplayOutcomeObserver::Bind(ACharacter* Source, ACharacter* Target)
{
    Unbind();
    Outcome = PortfolioReplay::FScenarioOutcome();
    SessionId = FExecutionSessionId();
    if (!IsValid(Source) || !IsValid(Target)) return false;
    SourceActor = Source; TargetActor = Target;
    SourceAction = Source->FindComponentByClass<UCActionComponent>();
    TargetReaction = Target->FindComponentByClass<UCReactionComponent>();
    TargetPair = Target->FindComponentByClass<UCExecutionCollaborationComponent>();
    TargetSignal = Target->FindComponentByClass<UCCombatSignalTargetComponent>();
    if (!SourceAction.IsValid() || !TargetReaction.IsValid() || !TargetPair.IsValid() || !TargetSignal.IsValid()) return false;
    SourceAction->OnActionEvent.AddDynamic(this, &UCReplayOutcomeObserver::OnAction);
    CommitHandle = TargetPair->OnExecutionLethalDeathEntryExpected.AddUObject(this, &UCReplayOutcomeObserver::OnLethalCommit);
    ReactionHandle = TargetReaction->OnReactionExecutionLifecycleEvent.AddUObject(this, &UCReplayOutcomeObserver::OnReaction);
    HitHandle = TargetSignal->OnCombatSignalTargetAccepted.AddUObject(this, &UCReplayOutcomeObserver::OnHit);
    return true;
}

void UCReplayOutcomeObserver::Unbind()
{
    if (SourceAction.IsValid()) SourceAction->OnActionEvent.RemoveDynamic(this, &UCReplayOutcomeObserver::OnAction);
    if (TargetPair.IsValid()) TargetPair->OnExecutionLethalDeathEntryExpected.Remove(CommitHandle);
    if (TargetReaction.IsValid()) TargetReaction->OnReactionExecutionLifecycleEvent.Remove(ReactionHandle);
    if (TargetSignal.IsValid()) TargetSignal->OnCombatSignalTargetAccepted.Remove(HitHandle);
    SourceAction.Reset(); TargetPair.Reset(); TargetReaction.Reset(); TargetSignal.Reset();
    SourceActor.Reset(); TargetActor.Reset();
}

void UCReplayOutcomeObserver::OnAction(ACharacter* Owner, EActionType Type, int32 Index, uint32 Serial, EActionEventType Event)
{
    if (Owner != SourceActor.Get() || Type != EActionType::Execution || Serial == 0) return;
    if (Event == EActionEventType::ActionStarted && TargetPair.IsValid())
    {
        const auto Snapshot = TargetPair->GetExecutionCollaborationRuntimeSnapshot();
        const auto& Id = Snapshot.CollaborationContext.SessionId;
        if (Snapshot.bHasActiveSession && Id.SourceActor == Owner && Id.Serial == Serial
            && Snapshot.CollaborationContext.TargetSnapshot.TargetActor == TargetActor.Get()) SessionId = Id;
    }
    if (SessionId.SourceActor != Owner || SessionId.Serial != Serial) return;
    if (Event == EActionEventType::ActionCompleted) Outcome.bSourceCompleted = true;
    if (Event == EActionEventType::ActionInterrupted || Event == EActionEventType::ActionIgnored)
        Outcome.bExecutionInterrupted = true;
}

void UCReplayOutcomeObserver::OnLethalCommit(const FExecutionSessionId& Id)
{
    // These fixtures explicitly request lethal execution. This event is emitted
    // only after the target commits its reservation, before death resets state.
    if (Id.IsValidMinimal() && Id == SessionId) Outcome.bExecutionCommitted = true;
}

void UCReplayOutcomeObserver::OnReaction(const FReactionExecutionLifecycleEvent& Event)
{
    if (!SessionId.IsValidMinimal() || !(Event.Context.ExecutionSessionId == SessionId)) return;
    if (Event.EventType == EReactionExecutionLifecycleEventType::Completed) Outcome.bTargetCompleted = true;
    if (Event.EventType == EReactionExecutionLifecycleEventType::Interrupted || Event.EventType == EReactionExecutionLifecycleEventType::Ignored)
        Outcome.bExecutionInterrupted = true;
}

void UCReplayOutcomeObserver::OnHit(const FCombatSignalTargetPacket& Packet)
{
    if (SourceAction.IsValid() && SourceAction->IsActiveActionType(EActionType::ComboAttack)
        && Packet.Payload.SourceActor == SourceActor.Get() && Packet.Payload.TargetActor == TargetActor.Get()
        && Packet.Result.bAccepted && Packet.Result.CommittedDamage > KINDA_SMALL_NUMBER)
        ++Outcome.AcceptedDamageHits;
}
