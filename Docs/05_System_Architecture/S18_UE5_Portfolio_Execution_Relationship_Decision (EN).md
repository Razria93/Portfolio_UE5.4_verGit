# S18 UE5 Portfolio Execution Relationship Decision

## 1. Purpose

This document defines why execution should be modeled by active/incoming relationship, not only by executability.

The core idea is to classify execution as Independent, Sequential, or Exclusive.

## 2. Previous System Shape

The previous execution decision was close to a simple executability result.

```text
Executable
-> can execute

Chainable
-> can chain

Reject
-> cannot execute

Ignore
-> ignore request
```

Combo chain looked exceptional because input timing and execution timing are separated.

```text
Input timing
-> reserve next chain data

Notify timing
-> consume reserved chain data
-> play next montage
```

## 3. Problems And Limits

`Executable` and `Chainable` mix executability and apply behavior.

Execution should be split into:

```text
executability
-> Accept / Reject / Ignore

relationship
-> Independent / Sequential / Exclusive

apply mode
-> Start / Reserve / Intervene / StopOnly
```

Chain is not an intervention. It does not stop the active execution. It continues the same execution flow.

## 4. Refactoring Direction

Relationship should drive the next step.

```text
Independent
-> starts independently or when no active execution exists

Sequential
-> belongs to the same flow as the active execution
-> uses reserve / consume

Exclusive
-> cannot coexist with active execution
-> requires intervention
```

Branching should be:

```text
No active
-> immediate start

Sequential
-> reserve / consume

Exclusive
-> intervention query / directive

Invalid
-> reject / ignore
```

## 5. Trial And Error

Chain initially looked like a special exception because accepted execution often moved toward intervention resolution.

The important realization was that chain does not bypass intervention as a special case. It does not need intervention because its relationship is sequential, not competitive.

## 6. Conclusion

Execution decisions should not only answer whether something can execute.

They must also describe how the incoming execution relates to the active execution.
