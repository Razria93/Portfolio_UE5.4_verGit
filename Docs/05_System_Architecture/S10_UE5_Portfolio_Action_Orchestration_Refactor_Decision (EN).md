# Action Orchestration Refactor Architecture Decision

## 1. Purpose

This document defines how action orchestration should be reorganized based on the responsibility separation clarified during reaction orchestration.

The key point is not to copy the reaction structure directly.

It is to reapply the `Orchestrator -> Component -> Executor` responsibility separation to the action domain.

---

## 2. Background

Reaction orchestration clarified the following structure.

```text
ReactionOrchestrator
-> request interpretation / context resolve / policy resolve / conflict decision

ReactionComponent
-> active runtime state / decision application / side effect

CReaction
-> montage lifecycle / notify window / feedback / local rule
```

This reorganized reaction execution as:

```text
damage event
-> orchestration
-> component apply
-> executor lifecycle
```

As a result, action orchestration should also be improved to the same level of responsibility separation.

The current action structure has `ActionOrchestrator`, but much of the actual execution decision still remains in `ActionComponent::ExecuteAction()` and `CAction::DecideExecution()`.

This may be acceptable while the feature set is mostly combo attack / equip / unequip.

However, transition decisions become quickly complex when guard / parry / dodge / counter / cancel / buffered input are introduced.

Therefore, the purpose of action orchestration refactor is to organize action execution decision into an orchestrator-centered decision pipeline instead of leaving it distributed across component and executor.

---

## 3. Structures That Can Be Shared

The external structure validated by reaction can also be applied to action.

```text
Orchestrator
-> interprets request
-> resolves context and policy
-> compares current runtime state and incoming request
-> generates final decision

Component
-> owns active runtime state
-> applies decision to actual character state
-> handles executor cache / lookup
-> handles movement / state / side effect

Executor
-> runs actual montage lifecycle
-> handles notify window and feedback
-> provides local rule hooks
```

This structure is valid for action because action and reaction share these traits.

```text
external request enters
request must be interpreted into executable context
current active execution state must be compared
component must apply actual runtime mutation
executor must run montage / notify / feedback lifecycle
```

Therefore, action should have the following API flow.

```text
RequestAction()
-> CanAcceptActionRequest()
-> ResolveActionCandidates()
-> ResolveActionContexts()
-> ResolveActionPolicy()
-> BuildActionQueries()
-> OrchestrateActionQueries()
-> DispatchActionDecision()
-> BuildRequestResult()
```

Component and Executor can also keep a similar external shape to reaction.

```text
UCActionComponent
-> ApplyActionDecision()
-> StartAction()
-> ChainAction()
-> QueueAction()
-> InterruptAction()
-> CancelAction()
-> CompleteCurrentAction()
-> AbortCurrentAction()

UCAction
-> Start()
-> Chain()
-> Stop()
-> Complete()
-> Abort()
-> local rule hook
-> feedback request
```

---

## 4. Structures That Must Differ

Action and reaction can share the external structure, but the internal decision meaning must differ.

The core of reaction decision is deciding how incoming reaction caused by external damage event competes with the current active reaction.

```text
incoming hit reaction arrives during active hit reaction
incoming dead reaction arrives during active hit reaction
does incoming reaction have higher priority?
does active executor allow interruption?
does incoming executor want interruption?
```

The core of action decision is deciding how player / AI intent can be handled as a transition inside the current action lifecycle.

```text
can this action start now?
can the current action chain?
can the current action queue?
can current action be cancelled into dodge?
can counter attack interrupt the current state?
does equipment / stamina / body state allow the action?
```

Therefore, action orchestration is closer to action transition orchestration than reaction conflict resolution.

The important action structure is:

```text
Intent -> Candidate -> Context -> Query -> Decision
```

---

## 5. Action Interpretation Flow

Input should not select concrete execution data directly.

Input should only deliver intent-level meaning.

```text
LMB
-> Attack intent

Space
-> Dodge intent

RMB
-> Guard intent
```

Values such as `SwordLightAttack_02` or `ComboIndex = 2` should not be mapped directly to input.

The orchestrator should interpret those values from the current state.

Recommended flow:

```text
Raw Input
-> Intent
-> Action Request
-> Global / Body Policy Gate
-> Action Candidate resolution
-> Action Context resolution
-> Executor Local Rule query
-> Final Orchestration Decision
-> Component Decision application
-> Executor execution
```

`ActionCandidate` is not a confirmed executable value.

It is an execution candidate interpreted from intent and current state.

```text
Attack intent
-> Chain candidate
-> Queue candidate
-> Normal start candidate

Dodge intent
-> Reaction cancel candidate
-> Normal dodge candidate
```

`ActionContext` makes candidate concrete enough for decision and execution.

```text
ActionCandidate
-> resolve ActionData
-> resolve ActionExecutor
-> build action type / index / combo group / cost / target context
```

Global / Body Policy Gate should run before local rule query.

```text
dead
invalid owner
invalid equipment
action lock
clearly insufficient stamina
```

These conditions should be rejected early before querying executor windows.

After that, local rule of the active executor is queried.

```text
is chain window open?
is queue window open?
is cancel window open?
does current montage section accept incoming action?
```

The final `Start / Chain / Queue / Interrupt / Cancel / Reject / Ignore` decision is generated by `ActionOrchestrator`.

`CAction` does not make the final decision.

It provides executor-local windows and local rules.

---

## 6. Data and State Responsibility

Another issue found during reaction work is that definition data and runtime state are tied together inside the component.

In the current first reaction implementation, definition data remains in `ReactionComponent`, and orchestrator resolves data / executor through the component.

This is acceptable at the current stage.

However, in the long term, both action and reaction should follow this responsibility split.

```text
Definition Data
-> move to DataAsset / DataProvider / orchestrator-adjacent layer

Runtime State
-> owned by Component

Executor Instance Cache
-> owned by Component

Local Rule
-> provided by Executor hook

Final Decision
-> generated by Orchestrator

Runtime Mutation
-> applied by Component
```

This standard should also be kept in action orchestration refactor.

---

## 7. Decision

Action orchestration refactor should proceed based on the reaction orchestration structure.

The following standards should be kept.

```text
external structure should stay symmetric with reaction
internal decision policy should be redefined for the action domain
input is responsible only for intent
orchestrator handles candidate / context / policy / query / decision
component handles active action state and decision application
CAction handles montage lifecycle and local rule
definition data separation is considered as a separate stage
```

This work should not be included in the reaction orchestration branch.

It should be done in a separate action refactor branch.

Recommended branch:

```text
feature/action-orchestration-refactor
```

---

## 8. Refactor Scope

Recommended stages:

```text
Stage 1
-> align ActionOrchestrator API scaffold
-> add CanAccept / Candidate / Context / Policy / Query / Orchestrate / Dispatch stages
-> keep existing gameplay result

Stage 2
-> move decision responsibility from ActionComponent::ExecuteAction() to orchestrator
-> reduce CAction::DecideExecution() into local rule hook
-> organize ActionCandidate / ActionContext / ActionQuery / ActionResult

Stage 3
-> clarify Start / Chain / Queue / Cancel / Interrupt / Complete / Abort lifecycle
-> align ComboAttack / Equip / Unequip with the new structure
-> connect Guard / Parry / Dodge / Counter extension points

Stage 4
-> review action definition data separation
-> review ActionDataAsset / DataProvider structure
```

---

## 9. Expected Files

Expected files to modify:

```text
Source/Portfolio/Component/CActionOrchestratorComponent.h
Source/Portfolio/Component/CActionOrchestratorComponent.cpp
Source/Portfolio/Component/CActionComponent.h
Source/Portfolio/Component/CActionComponent.cpp
Source/Portfolio/Action/CAction.h
Source/Portfolio/Action/CAction.cpp
Source/Portfolio/Action/CAction_ComboAttack.h/.cpp
Source/Portfolio/Action/CAction_Equip.h/.cpp
Source/Portfolio/Action/CAction_Unequip.h/.cpp
Source/Portfolio/Type/CActionOrchestrationStructure.h
Source/Portfolio/Type/CWeaponStructure.h
Source/Portfolio/Character/Player/CPlayer.cpp
Source/Portfolio/Character/Enemy/CEnemy.cpp
Source/Portfolio/AI/BehaviorTree/Task/CBTTask_StartCombatAction.cpp
```

During the first refactor pass, action feedback, notify, and weapon hit context should be touched as little as possible.

However, during lifecycle cleanup, action notify and complete / abort flow should also be reviewed.

---

## 10. Workload Estimate

The workload is medium or higher.

This is not just renaming functions. It changes where action execution decisions live.

The rough difficulty is:

```text
Stage 1 API alignment
-> low to medium

Stage 2 decision responsibility movement
-> medium

Stage 3 lifecycle cleanup
-> medium to high

Stage 4 data separation review
-> medium
```

`CAction_ComboAttack` needs the most care because it is connected to chain window, action index, feedback request, and hit context.

`Equip` / `Unequip` are simpler, but they are connected to weapon state, so request reject reasons must remain clear.

---

## 11. Consequences

Benefits:

```text
Responsibility separation validated by reaction can be applied to action
Action decisions can be less distributed across component / executor
Decision pipeline for guard / parry / dodge / counter / cancel can be prepared
External structure of action and reaction can stay symmetric
Internal decision policy of each domain can stay independent
```

Notes:

```text
Copying reaction structure directly can miss the meaning of action transition
Action is centered on transition orchestration rather than conflict resolution
Candidate / Context / Policy / Local Rule responsibilities should be clear from the beginning
Definition data separation is not required in the first action refactor pass, but remains a long-term task
```

---

## 12. Follow-up

Follow-up candidates:

```text
Redefine ActionOrchestrationStructure
Design ActionCandidate / ActionContext / ActionQuery / ActionResult
Refactor ActionOrchestrator into Request / Resolve / Policy / Query / Orchestrate / Dispatch structure
Reduce ActionComponent to active state and decision application
Redefine CAction::DecideExecution() as local rule hook
Connect Guard / Parry / Dodge / Counter decision policy to action orchestration
```

---