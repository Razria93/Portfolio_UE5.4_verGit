# A19 UE5 Portfolio Execution Refactor Difficulty Analysis

## 0. Document Role

This is a difficulty and retrospective document. It records why the execution refactor was hard and which concepts were confusing.

A18 records refactor history. A20 records formal architecture issues. This document preserves the confusion points that shaped the final decisions.

## 1. Purpose

This document explains the structural points that increased refactor difficulty and how the work moved toward clearer responsibility boundaries.

## 2. Background That Made The Refactor Difficult

The component-centered structure blurred responsibility boundaries.

Action and reaction had asymmetric flows even though both execute montage-based execution objects.

Combo chain looked like an exception because input time and execution time are separated.

## 3. Major Confusion Points

The responsibility boundary between orchestrator, component, executor, and notify was not clear.

Local and orchestration terminology hid responsibility overlap. Local could start deciding cancel and interrupt semantics, policy could re-filter local decisions, and orchestration could become only a conversion layer.

Intervention was easy to misunderstand as an execution API instead of an arbitration step.

Cancel and interrupt looked like API combinations rather than semantic stop reasons.

Want and Allow looked like the same window even though one is incoming-side intent and the other is active-side permission.

Snapshot and participant responsibilities could overlap if snapshot carried too much active detail.

Wildcard keys made validity checks difficult because lookup validity and runtime execution validity are different.

MontageEnd, Complete, and Stop can overlap in lifecycle handling.

Active and incoming perspectives were easy to confuse when reading intervention rules.

## 4. Criteria That Reduced Confusion

The structure was clarified around:

```text
Orchestrator
-> request interpretation and decision/directive construction

Component
-> runtime state and directive consumption

Executor
-> montage lifecycle and local rule

Notify
-> timing event delivery
```

Policy moved toward snapshot-based judgment.

Intervention decision and intervention execution were separated.

Chain was redefined as sequential relationship, not a special exception.

ApplyChain / AdvanceCombo were reinterpreted as reserve / consume.

## 5. Major Work Turning Points

The work moved from component-centered decisions to orchestrator-centered decisions.

Replacement thinking became intervention directive thinking.

Policy booleans became snapshot-based state input.

Want / Allow were split.

Chain became a sequential relationship.

## 6. Future Direction

Execution decision names should be reviewed.

Relationship branching should happen before intervention resolution.

Snapshot and participant structures should remain separated.

DataKey validity rules should distinguish lookup validity from resolved runtime validity.

Montage lifecycle rules should continue to distinguish Complete, Stop, and MontageEnd.

## 7. Conclusion

The difficulty came from mixed concepts, not only from code volume.

The core separation is:

```text
Component decision -> Orchestrator decision / Component consumption
Replace -> Intervention directive
Policy bool -> Snapshot + Decision + Intervention
Single interrupt/cancel window -> Want / Allow split
Combo exception -> Sequential execution relationship
Apply / Advance chain -> Reserve / Consume chain
```
