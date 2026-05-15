# S15 UE5 Portfolio Action Orchestration Refactor Retrospective

## 1. Purpose

This document records the main points of confusion during the action orchestration refactor and the structural conclusions reached through those iterations.

The difficulty of this branch was not simply renaming APIs or cleaning up action execution code.

The core issue was deciding who judges, who stops execution, who starts follow-up execution, and which layer resolves competition between action and reaction execution.

## 2. Previous System Shape

The previous action flow still had many query-like and decision-like responsibilities inside action component.

As action types became more diverse, a request could no longer be connected directly to an execution API. A candidate stage became necessary to build execution candidates from intent.

The previous structure can be simplified as:

```text
Input / AI request
-> ActionComponent-centered execution judgment
-> CAction decision
-> Start / Chain / Replace style execution
-> Montage / Notify handling
```

Reaction already had a flow close to `Replace`, where active reaction could be replaced by another reaction.

However, during action refactor, it became clear that action/reaction intervention cannot be handled as simple replace.

When an action stops active reaction and then runs itself, this is not reaction-internal replace. The action orchestration result must request stop from reaction component.

## 3. Problems And Limits

The first confusion was why candidates were needed.

Actions have more varied input and execution types than reactions.

`ComboAttack`, `Equip`, `Unequip`, `Dodge`, and future `Guard`, `Parry`, `Counter` can all enter through similar request flows but resolve into different action data keys depending on current state.

Therefore, action request should first build a candidate and then resolve it into an execution context, instead of calling an executor directly.

```text
Intent
-> Candidate
-> ActionData
-> ActionExecutor
-> ActionExecutionContext
```

The second confusion was the definition of cancel and interrupt.

At first, an intentional stop could be treated as cancel, and an external stop as interrupt.

But in practice, intervention can happen between actions, between reactions, and across action/reaction boundaries. Follow-up behavior also differs.

Examples:

```text
Reaction -> Reaction
-> stop then start felt natural as replace

Action -> Reaction
-> hit reaction entering over active action is closer to interrupt

Action -> Reaction stop only
-> active reaction may be stopped without starting follow-up action

Reaction -> Action
-> dodge can cancel active reaction and enter action
```

If `TryReplace` binds stop and start together, every stop assumes "stop then start".

However, cross-domain intervention can require stop only, stop then start, ignore without stop, or reject when stop fails.

Solving cancel and interrupt by adding one-dimensional APIs would create many unnecessary APIs.

The third confusion was the order of layers inside orchestration.

The current implementation first resolves local decision and then filters that decision with policy.

```text
Local decision
-> Policy filter
-> Orchestration result
```

This can work, but it makes local rule inspect external state such as body state or active reaction existence, while policy also checks whether the local decision is allowed.

The responsibilities overlap.

A better direction is to reduce the ambiguous middle concept of `policy`, first compress the current execution environment into `ExecutionSnapshot`, and then pass it to local rule.

```text
ResolvedContext
-> ExecutionSnapshot
-> LocalDecision
-> Arbitration
-> Directive / Command
```

`ExecutionSnapshot` should compress body state, active action/reaction existence, active context validity, and alive state so local rule does not directly inspect global components.

Local rule should use that snapshot and incoming execution context to propose one of `Start`, `Chain`, `Cancel`, `Interrupt`, `Reject`, or `Ignore`.

The fourth confusion was how to resolve competition after local decision.

When a decision intervenes in active execution, such as `Cancel` or `Interrupt`, a simple bool policy is not enough.

Both incoming and active sides must be queried.

```text
Incoming executor
-> does it want to intervene in active execution?

Active executor
-> does it allow being stopped by incoming execution?

Orchestrator
-> combines both answers, priority, force, window, and body state into the final judgment
```

This stage is clearer as `InterventionQuery -> InterventionAssessment -> InterventionDirective` rather than `Policy`.

```text
InterventionQuery
-> contains active/incoming domain, context, requested intent, and snapshot

InterventionAssessment
-> contains incoming want rule, active allow rule, priority, and force judgment

InterventionDirective
-> expresses what to stop, why to stop, and what happens after stop for component consumption
```

The fifth confusion was the boundary between orchestration and component.

Orchestration should judge competition and build directives, but component and executor should actually mutate action/reaction state and start or stop montages.

Therefore, orchestration result should not be just an enum. It should contain a command or directive that component can consume.

```text
Orchestration
-> decides what should stop
-> decides why it should stop
-> decides what should happen afterward
-> builds component-consumable directive

Component
-> consumes directive
-> stops active action/reaction
-> starts follow-up execution if needed
-> updates runtime state
```

The sixth confusion was naming.

`FActionResolvedContext` and `FReactionContext` represent the same conceptual layer but have different names.

Both are execution contexts that contain execution key, execution data, and execution executor. Long term, they should be aligned as:

```text
FActionExecutionContext
FReactionExecutionContext
```

Similarly, the common arbitration layer should not carry action local decision enum or reaction local decision enum directly. It should receive a converted common intervention intent.

```text
ActionLocalDecision::Cancel
ReactionLocalDecision::Interrupt
-> converted into ExecutionInterventionIntent
```

The seventh confusion was the relationship with future system-level coordination.

Current action/reaction orchestration judges mostly from the owning character's execution state.

However, guard, parry, counter, execution, and damage control may need the opponent's state.

In that case, orchestration should not directly inspect the opponent object. It is more appropriate to receive state snapshots or judgment results from a mediator subsystem or interaction subsystem.

## 4. Refactoring Direction And Content

The first direction established in this branch is:

```text
ActionOrchestrator
-> builds incoming action data and executor from intent and state
-> compresses current execution environment into snapshot
-> builds execution decision through local rule and orchestration result
-> creates reaction stop directive when needed

ActionComponent
-> consumes orchestration result
-> manages active action state
-> requests reaction component stop when needed
-> actually performs action start / chain / replace / stop

CAction
-> owns action execution behavior
-> handles montage start, finish, notify, and feedback
-> caches only runtime data needed while executing
-> provides local rule
```

The current implementation still has `FActionResolvedPolicy` after local decision.

Therefore, the current structure is an intermediate stage, not the final shape.

The important change in this branch is that query and decision flow previously inside action component has moved toward orchestration.

Also, cross-domain intervention between action and reaction is no longer treated as simple replace. It is now represented as a component-consumable stop directive.

Current `FReactionStopDirective` is not a complete common intervention model. It is a minimum skeleton that lets action orchestration stop active reaction.

## 5. Future Direction

The follow-up structure should proceed in this order.

First, redefine `FActionResolvedPolicy` into the `FExecutionSnapshot` family.

Recommended flow:

```text
Request
-> Candidate
-> ExecutionContext
-> ExecutionSnapshot
-> LocalDecision
-> Arbitration
-> Directive / Command
-> Component Apply
```

`ExecutionSnapshot` should compress current execution environment so local rule does not directly inspect external components.

Examples:

```text
ExecutionState
bIsAlive
bHasActiveAction
bHasActiveReaction
ActiveActionContext
ActiveReactionContext
```

Second, split intervention arbitration into `Query / Assessment / Directive`.

```text
FExecutionInterventionQuery
-> contains incoming/active context, domain, intervention intent, and snapshot

FExecutionInterventionAssessment
-> contains incoming wants, active allows, priority, force, and window judgment

FExecutionInterventionDirective
-> contains stop target, stop reason, stop source, and after stop action for component consumption
```

Third, align context naming and API layers between action and reaction.

Recommended names:

```text
FActionExecutionContext
FReactionExecutionContext

FActionLocalLevelQuery
FReactionLocalLevelQuery

FActionLocalLevelResult
FReactionLocalLevelResult
```

The common arbitration layer should use a converted common intent instead of directly storing domain-specific local decision enums.

```text
EExecutionDomain
EExecutionInterventionIntent
EExecutionStopReason
EExecutionStopSource
EExecutionAfterStopAction
```

Fourth, align action and reaction component API layers.

Both should follow:

```text
Apply
-> official entry point for consuming orchestration result

Request
-> entry point for cooperation requests from another domain or external layer

Try
-> component-internal conditional execution API

Internal
-> performs actual state mutation and executor calls
```

Fifth, define the relationship with future subsystems.

Future combat structure may require:

```text
Mediator / Interaction Subsystem
-> provides other actor state snapshots or interaction judgments

Control Subsystem
-> allows controlling opponent execution outside collision

Damage Request Subsystem
-> builds and dispatches damage requests outside collision
```

These layers should not replace action/reaction component responsibilities. They should help orchestration safely inspect and judge state outside its owning actor.

## 6. Conclusion

The main trial in this branch was deciding whether action orchestration is just an execution request router or a decision pipeline capable of resolving competition between action/reaction execution.

The current structure is not a completed orchestration model yet.

In particular, `FActionResolvedPolicy` is closer to a middle structure that filters local decision again despite being named policy. Long term, it is clearer to split it into `ExecutionSnapshot` and `InterventionQuery / Assessment / Directive`.

However, this branch clarified the following criteria:

```text
Orchestrator
-> builds incoming execution, compresses execution environment into snapshot, and builds competition-resolution directives

Component
-> consumes directives and actually changes active state and side effects

Executor
-> owns execution behavior and handles montage / notify / feedback / local rule

Data Provider
-> provides data and executor cache but does not own runtime state judgment
```

This refactor is an intermediate step toward unifying action and reaction structures.

The next steps should align reaction API/flow, validate dodge cancel, introduce common intervention directive, and expand toward subsystem-based interaction judgment.
