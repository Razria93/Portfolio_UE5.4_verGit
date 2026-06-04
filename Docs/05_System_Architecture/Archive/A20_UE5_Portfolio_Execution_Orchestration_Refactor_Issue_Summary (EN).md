# A20 UE5 Portfolio Execution Orchestration Refactor Issue Summary

## 0. Document Role

This is the formal issue summary for the execution orchestration refactor.

A18 can be used as a history reference, and A19 can be used as a retrospective reference. This document is the primary architecture summary.

## 1. Purpose

This document summarizes the main architecture issues discovered during action / reaction execution orchestration refactoring and defines the direction for later execution refinement.

The central issues are:

```text
responsibility separation
action/reaction symmetry
cross-domain intervention
snapshot/participant separation
cancel/interrupt semantics
chain and execution relationship
```

## 2. Previous System Shape

The previous flow was component-centered.

```text
Request
-> Component decision
-> Executor execution
-> Notify
-> Component state update
```

Action and reaction evolved asymmetrically. Action was input/AI intent driven, while reaction was damage event driven and already had stronger active/incoming comparison.

Chain looked exceptional because it reserves data at input time and consumes it at notify time.

## 3. Problems And Limits

The main problems were:

```text
orchestrator/component/executor/notify responsibility overlap
action/reaction execution flow asymmetry
cross-domain intervention not represented as a common model
decision/intervention responsibility overlap
intervention decision mixed with runtime execution
cancel/interrupt and window responsibility confusion
snapshot/participant data duplication
data key wildcard validity ambiguity
component-owned data/executor cache responsibility
montage lifecycle stop/complete/end overlap
chain exposing missing relationship modeling
execution decision mixing executability and apply mode
```

## 4. Refactoring Direction

Execution should follow this model:

```text
Intent / Event
-> Candidate
-> ExecutionContext
-> Snapshot
-> Decision
-> Intervention
-> Directive
-> Component Apply
-> Executor Lifecycle
```

Decision, intervention, and component responsibilities should be separated.

Action and reaction should share an execution flow where possible.

Intervention should be represented through a directive.

Snapshot should store common body / execution state, while participant stores incoming / active execution detail.

Execution should later be modeled by relationship:

```text
Independent
Sequential
Exclusive
```

Chain should be represented as reserve / consume.

## 5. Future Direction

Future work should refine execution decision structure, relationship branching, DataKey validity rules, montage lifecycle behavior, and combat interaction integration.

Combat interaction may later coordinate attacker/defender state, damage requests, control requests, and action/reaction orchestration calls.

However, a combat subsystem should not own action/reaction runtime execution.

## 6. Conclusion

The refactor is not just about making action and reaction APIs look similar.

The more important change is redefining execution as:

```text
Intent / Event
-> Candidate
-> ExecutionContext
-> Snapshot
-> Decision
-> Intervention
-> Directive
-> Component Apply
-> Executor Lifecycle
```

The current structure is a foundation for more explicit execution relationship and intervention handling.
