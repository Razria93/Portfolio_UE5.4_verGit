# UE5 Portfolio Bug Report (EN)

## Title

**M3-B02: Fix for attack pattern lock caused by missing `AttackIndex` initialization during AIState transition**

### Date

- **Day 12**

- **2026.03.31**

### Type

- Bug

### Status

- [x] Resolved

### Branch

- `feature/ai-bt-context`


---

## Summary

- In `UCBTService_UpdateAIState::UpdateAIStateTransition()`, Engage-related Blackboard values were being reset, but `AttackIndex` was only cleared through `ClearValue()`.

- Because of this, when the AI returned to Engage after being hit, `AttackIndex` was interpreted using `0` as a base value instead of `INDEX_NONE(-1)`. Then `UCBTTask_SelectAttackIndex` calculated the next index as `1`, which caused the AI to keep selecting the `1st` attack after taking a hit.

- This was corrected by explicitly initializing `AttackIndex` to `INDEX_NONE`, so that attack selection could be recalculated properly after the state transition.


---

## Background

- Engine: Unreal Engine 5.4

- Target: AI BehaviorTree Service (`UpdateAIState`)

- Related Blackboard Keys:

  - `CAIKey::Engage::bInEngageRange`

  - `CAIKey::Engage::bCanAttack`

  - `CAIKey::Engage::bIsAttacking`

  - `CAIKey::Engage::AttackIndex`

- Related scenario:

  - Enemy performs an attack

  - The Player causes a hit reaction or target context change

  - The AIState temporarily transitions out of Engage

  - The AI later returns to Engage and performs the next attack selection


---

## Reproduction Steps

1. Let the Enemy select an attack index and perform an attack while in Engage state.

2. During or right after the attack, cause the AIState to transition out of Engage due to reasons such as Player counterattack, distance break, or HitReact.

3. Let the Enemy return to Engage state again.

4. Check that the next attack selection is interpreted using `0` as the base value instead of `INDEX_NONE`.

5. Observe repeatedly that the Enemy keeps using only the `1st` attack among the `0 / 1 / 2` combo pattern.


---

## Expected Result vs Actual Result

**Expected Result**

- When the AIState transitions out of Engage, Engage-related Blackboard values should be reset consistently.

- When the AI later re-enters Engage, `AttackIndex` should start from `INDEX_NONE` and be selected again based on the new context.

- The Enemy attack pattern should normally rotate or be reselected among indices `0 / 1 / 2` depending on the situation.

**Actual Result**

- `AttackIndex` was not explicitly initialized to an invalid value, and was interpreted as using `0` as the base value in the following selection logic.

- `UCBTTask_SelectAttackIndex` then calculated the next attack index as `1`.

- As a result, after being hit, the Enemy became locked into repeatedly using only the `1st` combo attack.


---

## Cause

The problematic code section was as follows.

```cpp
InBlackboardComp->SetValueAsBool(CAIKey::Engage::bInEngageRange, false);
InBlackboardComp->SetValueAsBool(CAIKey::Engage::bCanAttack, false);
InBlackboardComp->SetValueAsBool(CAIKey::Engage::bIsAttacking, false);
InBlackboardComp->ClearValue(CAIKey::Engage::AttackIndex);
```

- `bInEngageRange`, `bCanAttack`, and `bIsAttacking` were being explicitly reset to false.

- In contrast, only `AttackIndex` was handled through `ClearValue()`.

- Since this Blackboard key is directly interpreted as an integer attack index in the later attack selection logic, `ClearValue()` could not be assumed to always behave the same as the intended invalid state `INDEX_NONE`.

- `UCBTTask_SelectAttackIndex` was structured to calculate the next attack index based on the current `AttackIndex`.

- In the actual flow, after the state transition, `AttackIndex` was interpreted using `0` as the base value, and `UCBTTask_SelectAttackIndex` then calculated the next index as `1`.

- In other words, the essence of this bug was that **although Engage-related state was reset during AIState transition, the key attack selection value `AttackIndex` was not explicitly initialized to `INDEX_NONE`, causing the next selection logic to start from the wrong base value**.


---

## Fix

The initialization logic for `AttackIndex` was changed as follows.

```cpp
InBlackboardComp->SetValueAsBool(CAIKey::Engage::bInEngageRange, false);
InBlackboardComp->SetValueAsBool(CAIKey::Engage::bCanAttack, false);
InBlackboardComp->SetValueAsBool(CAIKey::Engage::bIsAttacking, false);
InBlackboardComp->SetValueAsInt(CAIKey::Engage::AttackIndex, INDEX_NONE);
```

The intention of this fix was as follows.

1. Explicitly represent the invalid state of the attack index in the Blackboard.

2. Guarantee that the later attack selection logic always starts from `INDEX_NONE`.

3. Prevent the biased behavior where the next index shifts to `1` because the flow starts from `0` during Engage exit -> re-entry.


---

## Verification

1. Repeatedly tested scenarios where the Enemy attacked in Engage state and then entered HitReact or another state transition.

2. Verified in Blackboard Debug that `AttackIndex` was initialized to `INDEX_NONE` when leaving Engage.

3. Verified that a new attack index was selected when re-entering Engage.

4. Verified that, even after taking a hit, attack selection no longer became fixed on the `1st` attack and instead branched normally.

5. Verified that the previously observed issue of repeatedly using the `1st` combo attack no longer reproduced.

6. Verified that, before the fix, the first re-attack after taking a hit always started from `1`, while after the fix, no specific attack index became locked and branching worked as intended.


---

## Notes

- For integer-based Blackboard selection values, explicit sentinel initialization is often safer than relying on `ClearValue()`.

- In particular, values such as `AttackIndex`, which are directly used as branching keys in later logic, should consistently enforce an invalid state such as `INDEX_NONE`.

- Although this fix only changes the initialization method, it effectively fixes the starting reference value used by the AI attack selection logic.


---
