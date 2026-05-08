# Orchestrator / Data / Component Responsibility Model

## 1. Purpose

This document summarizes the responsibility separation issues that became clear during Reaction Orchestration design,  
and records a shared structural principle that can also be applied to future Action Orchestration.

The key points clarified through the Reaction work are:

- An Orchestrator can be more than a simple input router; it can be the layer that evaluates competing states.
- A Domain Component naturally owns runtime state and applies execution decisions.
- Definition data becomes less shareable and less extensible when it is tightly bound to a component.
- An Executor object should own the actual execution lifecycle and local policy hooks.


---

## 2. Problem Recognition

During the Action Orchestration work, the orchestrator's coordination responsibility was not strongly exposed.

The core flow at that time was closer to:

```text
Player / AI Request
-> Common Gate
-> ActionType Resolve
-> ActionComponent Execute
```

The main issues were “can this request be accepted?” and “which action should this request be routed to?”

Reaction immediately exposes competing states.

```text
Hit while already in Hit
Dead while in Hit
Stronger Hit while already in Hit
Incoming reaction outside an interruptible window
```

Therefore, Reaction requires more than simple routing.  
It needs a responsibility that compares current execution state against incoming requests and produces the final decision.


---

## 3. Responsibility Conflict Exposed by Reaction

In the current Reaction structure, `UCReactionComponent` owns several responsibilities at the same time.

```text
Stores ReactionDatas
Builds ReactionDataMap
Caches ReactionExecutorMap
Stores ActiveReactionContext
Judges current vs incoming
Applies movement / state / action abort
Calls CReaction execution
```

This creates a problem when introducing `ReactionOrchestrator`.

For the orchestrator to evaluate reaction competition, it needs information such as:

```text
Reaction definition data
Reaction priority
Reaction executor class
ActiveReactionContext
Interruptible / cancelable policy
```

If all this information lives in `ReactionComponent`,  
the orchestrator ends up pulling internal component data through getters to evaluate the decision.

This naturally raises the following question:

```text
If all data comes from ReactionComponent anyway,
shouldn't ReactionComponent also handle the competition decision?
```

This question is valid.

The root issue is not the existence of the orchestrator.  
The root issue is that definition data and runtime state are bound together inside the same component.


---

## 4. Core Separation Criteria

The Reaction structure should be separated into four responsibilities.

```text
Definition Data
- What executable candidates exist
- Which key is used to find them
- What the default priority / policy values are

Orchestrator
- Converts external requests into domain intents
- Looks up definition data
- References current runtime state
- Evaluates competing states
- Generates decisions

Component
- Owns runtime state
- Owns executor instance cache
- Applies decisions to the actual character state
- Handles movement / state / action side effects

Executor Object
- Owns the actual execution lifecycle
- Handles montage / notify / cleanup
- Provides local timing and local policy hooks
```

Applying this separation makes each layer's role clearer.


---

## 5. Recommended Reaction Structure

The long-term target structure for Reaction is:

```text
ReactionDefinitionDataAsset
-> reaction definitions
-> match key / priority / montage / executor class / play policy

ReactionOrchestratorComponent
-> request gate
-> damage result -> reaction intent
-> data lookup
-> active / incoming conflict resolution
-> decision generation

ReactionComponent
-> active runtime state
-> executor instance cache
-> ApplyReactionDecision
-> movement / state / action abort application

CReaction
-> montage lifecycle
-> notify window runtime flags
-> local interrupt / cancel policy hook
```

In this structure, the information exposed by `ReactionComponent` should be centered on runtime state.

```text
GetActiveReactionContext()
GetActiveReactionExecutor()
```

The following responsibilities should move outside the component in the long term.

```text
ReactionDatas
ReactionDataMap
BuildCandidateSpecKeys
ResolveReactionData
Priority decision policy
```

These are closer to definition / matching / policy data than execution state.


---

## 6. Data Asset Separation Direction

Currently, `FReactionData` mixes selection condition, execution data, and policy values.

```text
ReactionDataKey
ReactionExecutorKey
Montage
PlayRate
bCanMove
Priority
```

As a first pass, moving this as-is into `ReactionDefinitionDataAsset` is realistic.

The expected structure is:

```cpp
UCLASS(BlueprintType)
class PORTFOLIO_API UCReactionDefinitionDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TArray<FReactionData> ReactionDatas;
};
```

If needed later, it can be split further.

```text
FReactionMatchKey
- ApplyDamageSpecKey
- ReactionType

FReactionExecutionData
- ExecutorClass
- Montage
- PlayRate
- bCanMove

FReactionPolicy
- Priority
- interrupt / replace policy
```

At the current stage, the priority is not excessive decomposition.  
The priority is to separate definition data from the component.


---

## 7. Why the Orchestrator Looks Up Data

The orchestrator needs to see multiple kinds of information together to evaluate competing states.

```text
incoming reaction definition
active reaction context
incoming reaction context
priority
interruptible window
current executor policy
incoming executor policy
```

Therefore, it is natural for the orchestrator to read information from multiple layers.

The important rule is:

```text
Definition data is looked up from the Orchestrator or DataAsset layer.
Runtime state is read from the Component.
Actual execution lifecycle is handled by the Executor.
```

In other words, the orchestrator does not need to own everything.  
It should read the data required for judgment from each responsibility layer and produce the final decision.


---

## 8. Implications for the Action Structure

At the current stage, Action Orchestration is mostly request gate and routing.

```text
ActionOrchestrator v1
-> request gate
-> intent resolve
-> domain component routing
```

However, competing states may also appear in Action later.

Examples:

```text
Dodge during Attack
Guard during Attack
Attack during Dodge
Skill during Equip
Parry during Guard
Cancel during Skill
Buffered Action Queue
AI requested action vs forced action
```

In that case, Action can import the same principles organized from Reaction.

```text
ActionDefinitionDataAsset
-> action definitions
-> execution policy
-> cancel / interrupt / chain policy

ActionOrchestrator v2
-> request gate
-> action data lookup
-> current / incoming action conflict resolution
-> decision generation

ActionComponent
-> current action runtime state
-> action executor cache
-> decision apply

CAction
-> action-specific lifecycle
-> local timing / combo / notify policy
```

In this sense, Reaction work can become a more mature model for future Action Orchestration improvements.


---

## 9. Immediate Application Scope

For the first pass of Reaction Orchestration, the following criteria are appropriate.

```text
1. Keep active runtime state in ReactionComponent.
2. Keep executor instance cache in ReactionComponent.
3. Move Reaction definition data toward DataAsset or Orchestrator layer.
4. Put conflict resolution in ReactionOrchestrator.
5. Let ReactionComponent focus on decision application and execution-state management.
```

This gives the following benefits.

- Reduces `ReactionComponent` becoming an overloaded data container.
- Lets `ReactionOrchestrator` handle request coordination and conflict decisions as its name implies.
- Makes character-specific reaction profiles easier to share or swap through DataAssets.
- Allows future reaction sources such as Guard / Parry / Launch / KnockDown to use the same decision path.


---

## 10. Design Principles

The principles for this structure are:

- Separate definition data from runtime state.
- The orchestrator reads data for decision-making, but does not directly own execution lifecycle.
- The component owns runtime state and applies side effects for the current character instance.
- The executor object owns actual execution timing and local policy hooks.
- DataAssets should be used to make character-specific or type-specific reaction profiles shareable.
- Action should revisit the same responsibility separation model once its competing states become more complex.


---

## 11. Summary

The core lesson from Reaction Orchestration is:

```text
Orchestrator = evaluates competing states and generates decisions
Component    = owns runtime state and applies decisions
DataAsset    = provides definition data and matching source
Executor     = owns actual execution lifecycle and local policy
```

This structure first became necessary in Reaction,  
but the same direction can be applied to Action later if action competing states become more complex.

Therefore, the current Reaction work is not just a feature addition.  
It can become a reference point for organizing the request / decision / execution layers of the entire combat system.


---
