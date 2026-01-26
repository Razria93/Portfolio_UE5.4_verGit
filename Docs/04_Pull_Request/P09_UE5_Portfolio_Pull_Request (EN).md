# Reaction Pipeline Implementation: UCReaction / UCReactionComponent / AnimNotifyState_Reaction

## Title

✨ feat: Implement reaction pipeline (#24)

## Summary

* Added a `Data-Driven` reaction selection model based on `FReactionDataKey` + `FReactionData`.

* Built the full pipeline in `UCReactionComponent` from reaction type/data resolution → play/swap → restore on end.

* Standardized the `Validate/Begin/Stop/End` lifecycle and montage-end handling via the base `UCReaction` `Executor`.

* Routed `Interruptible/Cancelable/Immune Window` events through `UCAnimNotifyState_Reaction` to control external interference on active reactions.

* Implemented `Hit/Dead Reaction` policies so `Dead` is not interruptible/cancelable, and `Hit` allows interruption by `Dead`.

* Added `Reaction` mode to the `State` system to make state transitions explicit.


---

## Completed Tasks

### 1. Reaction data/executor registration structure

* Defined the reaction key-data model using `FReactionDataKey` / `FReactionData`.

* Implemented key-based data lookup and executor reuse caching in `UCReactionComponent`.

### 2. Reaction pipeline with state/movement control

* Implemented play or swap handling after resolving type/data in the `ProcessReaction` flow.

* Restricted movement during reactions, switched `State` to `Reaction`, and restored to `Idle` on end.

### 3. Executor-based lifecycle standardization

* Standardized montage play/end binding and end routing in `UCReaction`.

* Guaranteed the forced end/cleanup path via `End(true)` on `Stop`.

### 4. AnimNotifyState-based reaction window routing

* Opened and closed `Reaction Window` on `Notify Begin/End` and used it as a policy gate.

### 5. Hit/Dead reaction policy implementation

* `Hit` allows interruption by `Dead`, and otherwise follows the `Window` state.

* `Dead` is fixed as non-interruptible and non-cancelable.


---

## How to Test

1. Verify `ReactionDatas`/`ReactionClasses` are configured in `UCReactionComponent`.

2. Verify `RequestReaction` is called via `Hit`/`Death` flags in `FTakeDamageResult`.

3. Verify movement/state transitions behave correctly on `Reaction Montage` play/end.

4. Verify `Interruptible`/`Cancelable` behavior is applied correctly within `Reaction Window` sections.


---

## Related Issues / Branch

* Branch: `feature/combat-reaction`

* Issue: `#24`

---

## Notes

* Reaction selection is data-driven via `ApplyDamageSpecKey` + `ReactionType` mapping to ensure scalability.

* Window-based policies separate interrupt/cancel control and keep executor responsibilities minimal.


---