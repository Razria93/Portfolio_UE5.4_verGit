# S19 UE5 Portfolio Execution Intervention Directive Decision

## 1. Purpose

This document defines why cross-domain execution intervention should be represented as a shared directive.

The core point is to separate what stops, why it stops, who requested it, and what happens after stop.

## 2. Previous System Shape

The early reaction model could look like replacement.

```text
incoming reaction
-> stop active reaction if needed
-> start incoming reaction
```

This worked for reaction/reaction cases, but it was not enough once action and reaction could intervene in each other.

```text
Reaction -> Action
Action -> Reaction
Reaction -> Reaction
Action -> Action
```

## 3. Problems And Limits

Intervention cannot be represented only as a stop API.

The system must know:

```text
who intervenes
what should stop
why it should stop
what happens after stop
```

Expanding APIs such as `TryInterruptAndStart`, `TryCancelAndStart`, `TryInterruptAndEnd`, and `TryCancelAndEnd` causes combinatorial growth.

Not every stop is followed by a new start.

## 4. Refactoring Direction

Use a common intervention directive.

```text
FExecutionInterventionDirective
-> TargetDomain
-> StopReason
-> StopSource
-> AfterStopAction
```

Meanings:

```text
TargetDomain
-> what to stop

StopReason
-> why to stop

StopSource
-> who decided the stop

AfterStopAction
-> what to do after stop
```

The orchestrator builds the directive. The component consumes it.

## 5. Trial And Error

`TryReplace` style APIs were initially natural because reaction replacement looked like stop followed by start.

However, action-to-reaction cancellation is not reaction replacement. It is an action orchestration result that requests the reaction component to stop.

## 6. Conclusion

Cross-domain intervention is an arbitration result, not just a stop call.

The directive model gives a common representation for action/action, action/reaction, reaction/action, and reaction/reaction intervention.
