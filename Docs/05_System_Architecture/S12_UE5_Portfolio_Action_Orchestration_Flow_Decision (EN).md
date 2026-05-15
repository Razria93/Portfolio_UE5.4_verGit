# S12 UE5 Portfolio Action Orchestration Flow Decision

## 1. Purpose

This document records how action requests are resolved and converted into execution decisions after the action orchestration refactor.

The goal of this branch is to reorganize action execution around the `Orchestrator -> Component -> CAction` structure and separate request resolution, local rules, policy filtering, orchestration results, and component-side application.

## 2. Previous System Shape

The previous action system had an `ActionOrchestratorComponent`, but much of the actual execution judgment and state mutation still lived inside `ActionComponent` and `CAction`.

The representative flow was:

```text
Request
-> ActionComponent execute-style API
-> CAction DecideExecution
-> Component state mutation
-> Executor montage playback
```

The request was not clearly decomposed into an execution candidate and resolved context. Instead, the component and executor directly observed runtime state and mixed decision-making with execution.

This was acceptable for simple actions such as combo attack, equip, and unequip, but it did not provide a clear path for actions that can collide with an active action or reaction, such as dodge, guard, parry, or counter.

## 3. Problems And Limits

The first problem was that request resolution and execution application were not separated.

The system did not clearly split the stages of resolving an input intent into an action data key, resolving that key into action data and an executor, asking the executor what transition it wants, and checking whether that transition is allowed by the current body/runtime state.

The second problem was that executor-level judgment and orchestration-level judgment were mixed.

`CAction` can judge its own local rule, but it should not own the full coordination of active action, active reaction, body state, and cross-domain stop directives.

The third problem was that the action orchestration level did not yet have enough structure for execution arbitration.

When results such as `Start`, `Chain`, or `Interrupt` were produced, it was difficult to distinguish whether they came from local executor intent, body policy permission, or final orchestration arbitration.

## 4. Refactoring Direction And Content

After the refactor, action requests are processed through the following flow:

```text
Request
-> Candidate
-> ResolvedContext
-> LocalLevelQuery
-> LocalLevelResult
-> ResolvedPolicy
-> OrchestrationLevelQuery
-> OrchestrationLevelResult
-> Component Apply
-> CAction lifecycle
```

Each stage has a distinct role:

```text
Candidate
-> builds an executable key from intent and current state

ResolvedContext
-> resolves the candidate key into ActionData and ActionExecutor

LocalLevelQuery
-> packages incoming context, active context, and execution state for executor local rule queries

LocalLevelResult
-> records the transition requested by the incoming executor

ResolvedPolicy
-> filters whether the local decision has minimum permission in the current body/runtime state

OrchestrationLevelResult
-> builds the final action decision and any required directives
```

Current candidate resolution is single-candidate based.

```text
Equipment Toggle
-> resolves to Equip or Unequip from current weapon state

ComboAttack
-> resolves to active index + 1 when combo attack is active
-> resolves to index 0 when combo attack is not active

Dodge
-> resolves to Dodge action index 0
```

`FActionCandidate` currently contains one key, but it remains a separate structure so fallback candidates or alternate candidates can be added later.

## 5. Future Direction

The current orchestration level is not a full arbitration layer yet.

It currently converts local decisions and resolved policy into a final result, and attaches an active reaction stop directive for `Cancel`-style cases.

Future work should extend this in the following direction:

```text
Action vs Action
-> define chain, cancel, interrupt, and priority rules

Action vs Reaction
-> define flows where actions such as dodge cancel an active reaction

Reaction vs Action
-> define flows where hit or dead reactions interrupt active actions

Common Arbitration
-> promote active/incoming execution intervention rules into a shared model
```

Dodge is the best next validation case.

It is not a simple movement command. It can cancel an active reaction and enter action execution, which exercises local decision, policy, stop directive, and component application together.

## 6. Conclusion

This refactor does not complete the final action arbitration model. It establishes the structural path through which an action request can be resolved and applied.

The current design separates executor-local rules from orchestration coordination, which becomes the foundation for implementing dodge, guard, parry, and counter actions.
