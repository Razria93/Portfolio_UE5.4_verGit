# AI Action Event Bridge Structure

## 1. Purpose

This document organizes the current structure that lets AI reference important Action timing  
without directly depending on Notify names, Montage timing, or Action implementation details.

The main goals are as follows.

- Broadcast Action-internal timing events through a stable event bus.
- Let AI combo reuse the existing combat request path and chain execution path.
- Leave room for future buffer / interrupt / cancel expansion without leaking Action internals into BT code.


---

## 2. Problem Definition

The Player can respond to timing windows directly through input re-entry.

```text
Player Input
-> Character API
-> ActionOrchestrator
-> ActionComponent
-> Action
```

AI does not work like frame-precise input replay.

```text
Behavior Tree
-> decide
-> request
-> wait
-> decide again
```

If AI tries to consume Notify timing directly, the following problems appear.

- BT must know Notify names or Montage timing.
- AI tasks become coupled to Action implementation details.
- Combo timing logic is duplicated outside Action.
- Action expansion makes BT-side timing code harder to maintain.

The current structure solves this by separating:

- timing ownership
- event broadcasting
- high-level combat gating
- combo chain follow-up


---

## 3. Design Principles

### 3.1 Action Owns Timing

Timing windows remain inside Action.

Examples:

- `UCAction_ComboAttack::OpenChainWindow()`
- `UCAction_ComboAttack::CloseChainWindow()`
- `UCAction::Begin()`
- `UCAction::Complete()`
- `UCAction::Abort()`

Notify only calls Action methods.  
It does not interpret AI policy or BT logic.


---

### 3.2 ActionComponent Is the Stable Event Bus

Individual Action instances are execution units.

`UCActionComponent` is the stable character-side observation point.

Therefore, Action emits timing and lifecycle events through `UCActionComponent`.

```text
Action
- owns timing meaning
- emits event

ActionComponent
- owns current action execution state
- broadcasts action events
```


---

### 3.3 Combo Chain Follow-Up Reuses the Existing Combat Request Path

Current AI combo chain follow-up works as follows.

```text
Action event occurs
-> Enemy receives OnActionEvent(...)
-> Enemy requests the same combat action again
-> Action decides Start / Chain / Reject
```

This keeps AI combo chaining on the same execution path as Player combo chaining.


---

## 4. Overall Flow

### 4.1 Player Combat Flow

```text
Player Input
-> ACPlayer::HandleXXX()
-> UCActionOrchestratorComponent::RequestCombatAction()
-> UCActionComponent::ExecuteAction()
-> UCAction::DecideExecution()
-> Start / Chain / Reject
```

### 4.2 AI First Combat Start Flow

```text
Behavior Tree
-> ACEnemy::HandleAICombatAction()
-> UCActionOrchestratorComponent::RequestCombatAction()
-> UCActionComponent::ExecuteAction()
-> UCAction::DecideExecution()
-> Start / Chain / Reject
```

### 4.3 AI Combo Chain Follow-Up Flow

```text
AnimNotify
-> UCAction_ComboAttack::OpenChainWindow()
-> UCAction::EmitActionEvent(...)

-> UCActionComponent::OnActionEvent.Broadcast(...)

-> ACEnemy::OnActionEvent(...)
-> ACEnemy::RequestChainCombatAction(...)
-> ACEnemy::HandleAICombatAction(...)

-> UCActionOrchestratorComponent::RequestCombatAction()
-> UCActionComponent::ExecuteAction()
-> UCAction::DecideExecution()

-> Chain
```


---

## 5. Responsibility Separation

### 5.1 Notify

Notify only calls Action methods.

```text
Notify knows:
- which Action method to call

Notify does not know:
- Blackboard
- Behavior Tree
- AI state policy
- combat follow-up mapping policy
```


---

### 5.2 Action

Action knows the meaning of its own timing.

Examples:

- chain window open
- chain window close
- action started
- action completed
- action aborted

Action emits these events through `ActionComponent`.


---

### 5.3 ActionComponent

`UCActionComponent` has two responsibilities.

- manage current action execution state
- provide the official action event bus

External systems observe Action through this component, not through individual Action instances.


---

### 5.4 Enemy / AI Layer

The Enemy subscribes to `OnActionEvent`.

Current responsibilities are:

- map current `EActionType` to the appropriate AI combat intent
- re-enter the existing combat request path for chain follow-up
- keep combo follow-up logic out of Notify and out of BT task internals

Current example:

```text
ChainWindowOpened
-> RequestChainCombatAction(...)
-> HandleAICombatAction(...)
```


---

### 5.5 Behavior Tree

BT does not process combo timing windows directly.

BT owns:

- engage entry
- combat availability
- first combat action start
- waiting until combat action ends
- high-level branch switching


---

## 6. Current Event Model

The current Action event layer is appropriately kept at the following level.

```text
ChainWindowOpened
ChainWindowClosed
ActionStarted
ActionCompleted
ActionAborted
```

Under the current code:

- `ChainWindowOpened` is the main combo chain follow-up event.
- `ChainWindowClosed` is a timing boundary event owned by Action.
- `ActionStarted`, `ActionCompleted`, and `ActionAborted` are lifecycle events.

The important point is that these events are not all consumed in the same way.

- combo follow-up uses `ChainWindowOpened`
- lifecycle events remain available for debugging, synchronization, and future expansion


---

## 7. Principles for Combo Handling

### 7.1 Player Combo

Player combo uses the existing `Chain` path.

```text
same action request
-> Action decides Chain
-> ApplyChain()
-> AdvanceCombo()
```


---

### 7.2 AI Combo

AI combo now follows the same execution path.

```text
Action emits chain window event
-> Enemy receives action event callback
-> Enemy requests the same combat action again
-> Action decides Chain
-> ApplyChain()
-> AdvanceCombo()
```

This means:

- AI does not interpret Notify timing directly.
- AI does not need a separate combo-chain Blackboard protocol.
- Player and AI share the same chain execution mechanism inside Action.


---

## 8. Core of the Current Structure

The core of the current structure is as follows.

- Notify only calls Action methods.
- Action owns chain timing.
- ActionComponent broadcasts Action events.
- Enemy receives the event and calls the existing combat request path again.
- Actual chain decision stays inside `UCAction_ComboAttack`.


---

## 9. Expansion Points

The current structure is organized around linear combo chaining.

If future requirements add:

- combo branches
- enqueue / interrupt / cancel windows
- Action / Reaction takeover coordination

then additional expansion is needed.

For example, combo branches can map follow-up differently based on  
`ActionType + ActionIndex`.


---

## 10. Conclusion

The current structure can be summarized in one sentence.

```text
AI combo calls the existing combat request path again from an Action event,
and actual chain decision stays inside Action just like Player combo flow.
```

Under this structure:

- Notify knows only Action.
- Action owns timing.
- ActionComponent exposes timing to the outside.
- Enemy bridges chain timing back into the existing combat request path.
