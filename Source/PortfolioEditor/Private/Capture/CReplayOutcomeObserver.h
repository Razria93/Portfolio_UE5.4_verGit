#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Type/CActionTypes.h"
#include "Type/CExecutionCollaborationTypes.h"
#include "CReplayCompletion.h"
#include "CReplayOutcomeObserver.generated.h"

class ACharacter;
class UCActionComponent;
class UCReactionComponent;
class UCExecutionCollaborationComponent;
class UCCombatSignalTargetComponent;
struct FReactionExecutionLifecycleEvent;
struct FCombatSignalTargetPacket;

// Editor-only observer of existing runtime events. Never drives gameplay or HUD collection.
UCLASS()
class UCReplayOutcomeObserver : public UObject
{
    GENERATED_BODY()
public:
    bool Bind(ACharacter* Source, ACharacter* Target);
    void Unbind();
    PortfolioReplay::FScenarioOutcome Outcome;

private:
    UFUNCTION()
    void OnAction(ACharacter* Owner, EActionType Type, int32 Index, uint32 Serial, EActionEventType Event);
    void OnLethalCommit(const FExecutionSessionId& Id);
    void OnReaction(const FReactionExecutionLifecycleEvent& Event);
    void OnHit(const FCombatSignalTargetPacket& Packet);
    TWeakObjectPtr<ACharacter> SourceActor, TargetActor;
    TWeakObjectPtr<UCActionComponent> SourceAction;
    TWeakObjectPtr<UCReactionComponent> TargetReaction;
    TWeakObjectPtr<UCExecutionCollaborationComponent> TargetPair;
    TWeakObjectPtr<UCCombatSignalTargetComponent> TargetSignal;
    FExecutionSessionId SessionId;
    FDelegateHandle CommitHandle, ReactionHandle, HitHandle;
};
