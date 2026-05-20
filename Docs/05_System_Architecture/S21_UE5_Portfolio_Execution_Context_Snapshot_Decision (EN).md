# S21 UE5 Portfolio Execution Context Snapshot Decision

## 1. Purpose

This document defines how Snapshot, Participant, ExecutionContext, and DataKey responsibilities should be separated in execution orchestration.

The core point is to separate global state from execution participant detail and to separate data lookup validity from runtime execution validity.

## 2. Previous System Shape

Action and reaction both use data keys and execution contexts.

```text
ActionDataKey
-> ActionType
-> ActionIndex

ReactionDataKey
-> ApplyDamageSpecKey
-> ReactionType

ExecutionContext
-> DataKey
-> Data
-> Executor
```

Orchestration also needs current body state and active execution information.

## 3. Problems And Limits

Snapshot and participant can overlap.

If snapshot carries detailed active action/reaction data, it duplicates participant responsibility.

If snapshot is too small, decision and intervention do not get enough global state.

Wildcard keys also make `IsValidMinimal()` hard.

`All` and `INDEX_NONE` can be valid for data map fallback lookup, but they may be invalid for runtime execution.

## 4. Refactoring Direction

Separate responsibilities as:

```text
Snapshot
-> common body / execution state
-> execution state, dead state, global flags

Participant
-> incoming or active execution detail
-> action context, reaction context, executor, priority

ExecutionContext
-> resolved runtime execution unit
-> DataKey, Data, Executor
```

Long-term DataKey validity can be split into:

```text
IsValidDataKey
-> can be used as a data map key

IsWildcardKey
-> fallback lookup key

IsValidResolvedKey
-> executable resolved context key
```

## 5. Trial And Error

Putting active details both in snapshot and active participant made authority unclear.

It also made incoming / active perspective harder to read.

DataKey had the same problem because lookup keys and runtime execution keys do not share identical validity rules.

## 6. Conclusion

Snapshot should carry global state. Participant should carry incoming / active execution details.

DataKey validity should be interpreted by usage context: lookup key, wildcard key, or resolved execution key.
