# Action Orchestration Implementation Plan

## 1. Purpose

This document records the design-planning view of the action orchestration structure before it is fully applied to code.  
It focuses on what is considered problematic in the current structure and how that structure should be reorganized.

The main goals are as follows.

- Ensure that Player input and AI action requests use the same execution gate.
- Separate the responsibilities of `Intent`, `ExecutionState`, `ActionType`, `WeaponState`, and `MovementState`.
- Apply the new structure gradually in a low-risk order instead of replacing everything at once.
- Stabilize `StateComp` and `MovementComp` first, then expand toward `ActionComp`, `WeaponComp`, `ReactionComp`, and AI integration.


---

## 2. Current Problem Recognition

The current structure is considered problematic in the following ways.

- input intent and executable type are not cleanly separated
- `State` can grow by mixing top-level execution meaning and concrete behavior meaning
- Player and AI may end up using different execution gates for similar behavior
- the responsibility boundaries between movement, equipment, attack, and reaction are not yet explicit
- common execution rules may become duplicated across multiple layers as the system expands

In other words, the core problem is not one feature.  
It is that the execution gate and state axes are not yet fully organized.


---

## 3. Target Structure

The intended overall flow is as follows.

```text
Input / AI Decision
-> Intent Request
-> ActionOrchestrator
-> Domain Component
-> Execution State / Result
```

The target component responsibility split is as follows.

```text
StateComp
- manages only top-level execution state
- ExecutionState: Idle / Action / Reaction / Dead

MovementComp
- manages movement input, speed mode, jump, and falling state
- target of MovementIntent handling

ActionComp
- manages self-initiated authored actions
- ActionType: Equip / Unequip / LightAttack / ComboAttack / Guard / Dodge
- ActionState: Idle / Playing

WeaponComp
- manages attachment type and actual weapon transition state
- AttachmentType
- EquipmentType
- WeaponState: Unequipped / Equipping / Equipped / Unequipping

ReactionComp
- manages reactions caused by external stimulus
- ReactionType

AI / Behavior Tree
- manages high-level AI decision state
- AIIntentState: Patrol / Chase / Engage and so on
```


---

## 4. Input Data Flow

### 4.1 Player Flow

Player input is intended to start from the controller,  
and be converted into Intent Requests by the Player Character.

```text
CPlayerController
-> ACPlayer::HandleXXX()
-> create FMovementActionRequest / FCombatActionRequest / FEquipmentActionRequest
-> UCActionOrchestratorComponent::RequestXXXAction()
-> call MovementComp / ActionComp / WeaponComp
-> update StateComp and each domain state
```

The important points are:

- `PlayerController` remains the raw-input layer
- `ACPlayer` converts raw input into domain-specific requests
- execution validity is decided by the ActionOrchestrator


---

### 4.2 AI Flow

AI is intended to determine high-level intent through Behavior Tree and Blackboard,  
then generate concrete action requests inside tasks.

```text
BT Service
-> update Blackboard / AIIntentState

BT Task
-> create FCombatActionRequest / FMovementActionRequest / FEquipmentActionRequest
-> UCActionOrchestratorComponent::RequestXXXAction()
-> pass shared execution rules
-> call ActionComp / MovementComp / WeaponComp
```

The key idea is that Player and AI differ only in the source of input,  
while sharing the same execution gate.

```text
PlayerInput -> IntentRequest
AI Task     -> IntentRequest
```


---

## 5. Relationship Between Intent and Execution Type

Intent and execution type are not always matched 1:1.

```text
MovementIntent::Run
-> SpeedType::Run

MovementIntent::Jump
-> Character::Jump()

EquipmentIntent::Toggle
-> ActionType::Equip or ActionType::Unequip

CombatIntent::ComboAttack
-> ActionType::ComboAttack
```

The intended separation is:

- Intent = request intention before execution
- Type / State = execution result or current execution state
- therefore `EActionType` is not used as the type for every input request


---

## 6. Orchestrator Responsibilities

`UCActionOrchestratorComponent` is not intended to own detailed execution.

It should own only decision and routing.

```text
1. validate request
2. check common executability
3. check domain-specific conditions
4. convert Intent into execution type
5. call target component
6. return Result
```

### Combat Request Example

```text
RequestCombatAction
- check Dead / Reaction
- check WeaponState
- check ActionState
- convert CombatIntent into ActionType
- request execution from ActionComp
- return FActionRequestResult
```

### Movement Request Example

```text
RequestMovementAction
- check Dead / Reaction
- check MovementIntent
- execute through MovementComp or CharacterMovementComponent
- return FActionRequestResult
```

Requests closer to input-release cleanup, such as `StopJump`,  
may be processed before common action blocking.

### Equipment Request Example

```text
RequestEquipmentAction
- check Dead / Reaction
- check current WeaponState / AttachmentType
- convert EquipmentIntent into ActionType::Equip or ActionType::Unequip
- request execution from ActionComp and WeaponComp
- return FActionRequestResult
```


---

### 6.1 Execution Responsibility by Layer

The execution responsibility of each layer should be organized as follows.

```text
Orchestrator
-> receives Intent
-> checks shared rules
-> resolves execution type and execution path
-> delegates execution to the appropriate domain component

ActionComponent
-> finds the execution object for the given ActionType
-> asks for an execution decision using the current execution context
-> connects the flow according to Start / Chain / Reject

Action
-> owns execution conditions
-> owns timing meaning
-> owns concrete execution logic and cleanup responsibility
```

In other words, the Orchestrator connects intent to executable data and route,  
the ActionComponent acts as the execution hub that forwards into the actual Action object,  
and the Action object owns the concrete behavior meaning and detailed execution.


---

## 7. Work Order

### 7.1 StateComp Cleanup

StateComp should be cleaned up first.

It should be reduced to top-level execution state only.

```text
EExecutionState
- Idle
- Action
- Reaction
- Dead
```

Work items:

- redefine the old `EStateType` around the `EExecutionState` concept
- remove `Equip / Unequip` from StateComp
- reorganize APIs into `SetIdleState`, `SetActionState`, `SetReactionState`, `SetDeadState`
- update existing references


---

### 7.2 MovementComp Cleanup

MovementComp is relatively independent, so it should be cleaned up after StateComp.

Work items:

- keep `OnJump` and `OnStopJump`
- clarify ownership of `SpeedType`, `CurrentSpeed`, `CurrentDirection`, and `bIsFalling`
- add `EMovementState` if needed
- verify linkage with `RequestMovementAction`


---

### 7.3 ActionComp Cleanup

ActionComp should be reorganized as the center of authored action execution.

Work items:

- reorganize `EActionType` around `None / Equip / Unequip / LightAttack / ComboAttack / Guard / Dodge`
- add `EActionState`
- introduce a result-returning API such as `TryChangeActionMode()`
- improve combat result accuracy returned by the Orchestrator


---

### 7.4 WeaponComp Cleanup

WeaponComp should be reorganized to manage actual weapon equipment state and transition state.

Work items:

- add `EWeaponState`
- define `Unequipped / Equipping / Equipped / Unequipping`
- update state during equip/unequip start, mid-notify timing, and completion timing
- separate the meaning of `AttachmentType` and `WeaponState`


---

### 7.5 Equipment Action Linkage

Equipment requests should be linked so that ActionComp and WeaponComp handle them together.

Work items:

- handle `EquipmentIntent::Toggle`
- decide `ActionType::Equip` or `ActionType::Unequip` based on current `WeaponState / AttachmentType`
- connect execution between ActionComp and WeaponComp


---

### 7.6 ReactionComp Cleanup

ReactionComp is connected to Damage, Health, and State,  
so it should be cleaned up in a later stage.

Work items:

- organize entry and exit rules for `ExecutionState::Reaction`
- organize convergence rules toward Dead
- verify damage/reaction regressions


---

### 7.7 AI Integration

AI should be integrated last and gradually.

Work items:

- keep AIIntentState in Blackboard / BT
- gradually move direct execution code such as `BTTask_StartAttack` toward Orchestrator requests
- make both Player and AI use the same `UCActionOrchestratorComponent`


---

## 8. First Work Scope

The initial implementation scope should stay narrow and focus on StateComp and MovementComp first.

```text
Work 1:
- reorganize StateComp around ExecutionState
- stabilize MovementComp request flow
- verify the Movement path in the Orchestrator
```

This stage should not deeply modify ActionComp and WeaponComp yet.


---

## 9. Completion Criteria for Work 1

Work 1 is considered complete when the following conditions are satisfied.

```text
- existing MoveForward / MoveRight behavior remains intact
- Walk / Run operate through the Orchestrator
- Jump / StopJump operate through the Orchestrator
- Walk / Run / Jump are blocked during Dead / Reaction
- StopJump works properly as a cleanup event
- StateComp is organized around Idle / Action / Reaction / Dead
```

Once these conditions are satisfied,  
the work can move to the next stage: ActionComp / WeaponComp refactoring.


---

## 10. Design Principles

```text
Intent = request intention before execution
State  = state currently being occupied
Type   = category or identifier
Result = request processing result
```

The Orchestrator should not own execution itself.

It should decide on requests and forward them to the appropriate domain component.

Even if Player and AI follow different decision paths,  
they should still share the same execution states and action execution rules.


---

## 11. Failure-Handling Policy

The action orchestration structure should follow the failure-handling policy below.

### 11.1 Basic Rule

```text
Reject / Ignore = no observable state change
Start / Chain / Enqueue / Interrupt = commit allowed
```

Observable state change includes values such as:

- `ExecutionState`
- `CurrentActionType`
- `WeaponState`
- combat-related Blackboard values
- cooldown timestamps
- event broadcasts
- montage / feedback / hit context

In other words, if execution is rejected or ignored,  
the structure should not leave externally observable state changes behind.


---

### 11.2 Commit Timing Rule

Commit should be delayed as much as possible until execution success is confirmed.

```text
before success = decision / validation / resolve phase
after success  = commit allowed phase
```

This leads to the following rule set.

- request validation and execution-type resolve should remain side-effect free
- state commit is allowed only after `Start / Chain / Enqueue / Interrupt` is confirmed
- cooldown, blackboard, and action lifecycle values should be applied only after success

This means the goal is not to always change state first and execute second,  
but to commit only the state that belongs to the confirmed success path.


---

### 11.3 Orchestrator Rule

`UCActionOrchestratorComponent` should not become a layer that performs large rollback.  
Instead, it should minimize rollback needs by delaying side effects.

Its responsibility should remain focused on:

```text
1. validate request
2. check common blocking conditions
3. convert Intent into execution type
4. delegate execution
5. return result
```

In other words, the Orchestrator should avoid mutating execution state before success whenever possible.


---

### 11.4 ActionComponent Rule

`UCActionComponent::ExecuteAction()` should satisfy the following contract.

```text
Reject / Ignore
-> no change in current action / execution state / event / feedback

Start
-> action start commit allowed

Chain
-> chain input commit allowed
```

This means failure should behave as if nothing happened,  
while success should leave only completed, valid state transitions behind.


---

### 11.5 Action Cleanup Rule

`Abort()` and `Complete()` in `UCAction`-derived classes should act as cleanup endpoints.

Typical cleanup targets include:

- internal execution flags
- chain window state
- chained input state
- hit context
- action-local transient data

In short, start and chain commit only on success,  
while abort and complete must guarantee cleanup.

Feedback should also be treated as timing-driven behavior defined by Action,  
not as a single batch that runs only after execution.


---

### 11.6 Blackboard Rule

Blackboard should not be treated as a place to speculate about execution state.  
It should store actual lifecycle-driven state or service-derived state.

Examples:

- `bCanCombatAction` = service-derived value
- `bIsCombatAction` = actual action-lifecycle-driven value
- `NextCombatActionTime` = committed only after successful combat start

The rule is that BT Tasks should not pre-commit guessed execution state into Blackboard.


---

### 11.7 Reaction Takeover Rule

Reaction takeover should be treated as a transition that includes cleanup of any active action,  
not just a state switch.

Before a reaction starts, the following should be guaranteed.

- detect whether an active action exists
- abort that action if needed
- keep execution state / current action / Blackboard state consistent after reaction starts

This rule should become a base contract for later Reaction orchestration and higher-level execution coordination work.
