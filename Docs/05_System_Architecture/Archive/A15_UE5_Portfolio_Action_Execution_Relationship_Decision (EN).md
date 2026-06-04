# A15 UE5 Portfolio Action Execution Relationship Decision

## 1. Purpose

This document defines why action execution must be modeled by relationship, not only by executability.

The key point is that chain is not an exceptional case. It is a sequential relationship between the active action and the incoming action.

## 2. Previous System Shape

The action request flow was:

```text
Request
-> Candidate
-> ActionExecutionContext
-> ExecutionDecisionQuery
-> ExecutionDecisionResult
-> Intervention resolve
-> Common ExecutionResult
-> ActionExecutionResult
-> Component Apply
-> Executor Lifecycle
```

The previous decision values were close to:

```text
Executable
-> incoming action can execute

Chainable
-> incoming action can be reserved as the next action

Reject
-> cannot execute

Ignore
-> ignore the request
```

## 3. Problems And Limits

`Executable` and `Chainable` mix executability and execution relationship.

Chain also has a different nature from intervention. Intervention stops an active execution to apply an incoming execution. Chain continues the same flow and reserves the next execution until a notify timing consumes it.

The input timing and execution timing are separated in chain.

```text
Input timing
-> reserve next action data

Notify timing
-> consume reserved action data
```

## 4. Refactoring Direction

Execution should be split into three axes.

```text
Execution decision
-> Accept / Reject / Ignore

Execution relationship
-> Independent / Sequential / Exclusive

Execution apply mode
-> Start / Reserve / Intervene / StopOnly
```

Relationship-based branching should be:

```text
No active
-> start immediately

Sequential
-> reserve and later consume

Exclusive
-> resolve intervention

Invalid
-> reject or ignore
```

Chain should be expressed with reserve / consume terms.

## 5. Future Direction

Action execution result should carry relationship and apply mode explicitly.

Reaction should later be aligned with the same execution result model where possible.

Combat interaction can then add defensive actions such as dodge, guard, parry, and counter without treating each as a special case.

## 6. Conclusion

Action execution should not be represented only by "can execute".

The relationship between active and incoming execution determines whether the system should start, reserve, intervene, or reject.
