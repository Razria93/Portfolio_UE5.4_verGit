# Organize Combat Core Shared Rules and Unify the Damage Pipeline

## Title

`♻️ refactor: organize combat-core-shared rules and unify the damage pipeline`

## Summary

- This PR organizes the combat core shared by attackers and receivers, and includes the work to restructure the `ApplyDamage -> DefaultDamageEvent -> TakeDamage -> Health -> Reaction` flow under a common rule set.

- `ApplyDamage` and `TakeDamage` are each organized into a `Payload / Context / Result` flow so that sender-side and receiver-side responsibilities are clearly separated.

- An attachment hit window concept was introduced to add duplicate-hit prevention rules within the same attack window, and the bug where `HitWindowId` was delivered as `INDEX_NONE(-1)` at the first overlap timing was fixed together.

- In addition, dead target protection, reaction connection rules, and the related issue checklist and bug report documents were organized together.


---

## Completed Items

### 1. Organize Attachment Hit Window Rules

- Added a hit window lifecycle to `ACAttachment`

- Organized overlap metadata delivery based on `CurrentHitWindowId`

- Organized hit window open / close flow through `CollisionEnabled()` / `CollisionDisabled()`

- Adjusted collision enable ordering so that a valid `HitWindowId` is prepared before the first overlap

### 2. Unify the ApplyDamage Pipeline

- Organized `UCApplyDamageComponent` into the flow `ValidateRequest -> BuildPayload -> BuildContext -> ValidateContext -> CanApplyDamage -> ResolveApplyDamageSpec -> ComputeApplyDamage -> CommitApplyDamage -> BuildResult`

- Introduced `FApplyDamagePayload`, `FApplyDamageContext`, `FApplyDamageAmount`, and `FApplyDamageResult`

- Organized spec lookup and spec miss handling based on `FApplyDamageSpecKey`

- Organized self-hit prevention, duplicate-hit prevention, and invalid request reject paths

### 3. Unify the TakeDamage Pipeline

- Organized `UCTakeDamageComponent` into the flow `ValidateRequest -> BuildPayload -> BuildContext -> ValidateContext -> CanTakeDamage -> ComputeTakeDamage -> CommitTakeDamage -> BuildResult`

- Organized the meaning of `Requested / Mitigated / FinalTaken / Committed` from the receiver-side perspective

- Organized dead target reject handling and follow-up flow based on `CommittedDamage`

- Organized accepted / rejected dispatch flow

### 4. Organize Reaction Connection Rules

- Organized reaction connection based on the `CommittedDamage > 0` condition

- Reflected dead-state before/after conditions together so that the priority flow between death transition and reaction is clearly defined

- Verified reaction replace / interrupt flow during consecutive hit situations

### 5. Documentation

- Updated `D13_UE5_Portfolio_Issue_Checklist (KR)`

- Updated `D13_UE5_Portfolio_Issue_Checklist (EN)`

- Added `B05_UE5_Portfolio_Bug_Report (KR)`

- Added `B05_UE5_Portfolio_Bug_Report (EN)`


---

## Test Method

1. Verify that `ApplyDamage -> TakeDamage` works under the same attack flow for both Player and Enemy sides

2. Verify that duplicate hits on the same target are prevented within the same attack window

3. Verify that `HitWindowId` is delivered as a valid value at the first overlap timing so that the first hit is not rejected as `InvalidRequest`

4. Verify that the `Request / FinalTaken / Committed` values of `ApplyDamageResult` and `TakeDamageResult` are connected consistently in logs

5. Verify that when HP reaches 0, `CommittedDamage` is applied only up to the remaining HP

6. Verify that additional hits after dead / dying are processed with `CommittedDamage = 0` and do not cause further HP reduction


---

## Related Issue / Branch

- Branch: `feature/combat-core-shared`

- Related Work:

  - `M03-03: Organize Combat Core Shared Rules (#34)`

  - `M3-B05: Fix first-hit InvalidRequest reject caused by the CurrentHitWindowId increment timing issue (#35)`


---

## Notes

- The focus of this PR is **shared combat rules, responsibility separation, and pipeline organization**, rather than numeric balancing.

- Extension policies such as Team / Friendly Fire and Guard / Armor / Resistance are planned as follow-up work on top of the current shared combat core.

- Dead target protection is currently validated through both code and integrated logs, but if finer policy granularity becomes necessary later, the relationship between the receiver-side `AlreadyDead` reject path and the sender-side `CommitFailed` result can be refined further.


---
