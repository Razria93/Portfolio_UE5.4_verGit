# Stabilize One Full Player Combat Loop

## Title

`✨ feat: stabilize one full player-combat-loop cycle`

## Summary

- This PR documents and validates that the player attack loop closes through `Input -> Action -> Notify -> Collision -> ApplyDamage -> TakeDamage -> End` as one full cycle.

- It organizes and verifies the current behavior of `ComboAction` input conditions, `Idle -> Action -> Idle` state transitions, notify-based collision timing, and combo pre-input flow at a playable level.

- It confirms the current combo behavior where the flow connects as `1 -> 2 -> 3`, and the next input starts again from hit `1`.

- It also validates that the player attack loop connects naturally with the shared combat core through the `ApplyDamage -> TakeDamage` flow and attachment context delivery.


---

## Completed Work

### 1. Input and Entry Conditions

- Organized and verified `ComboAction` input entry conditions

- Confirmed the current rule that attacking is only possible while equipped

- Organized and verified the attack restriction policy outside Idle state

- Confirmed the input blocking policy during `Reaction / Dead` states

### 2. Action State Transitions

- Validated `Idle -> Action -> Idle` state transitions

- Confirmed linkage between `ActionComponent` and `StateComponent`

- Confirmed state return after attack end

### 3. Notify-Based Attack Flow

- Validated `AnimNotify_Action` begin/end linkage

- Validated `AnimNotify_Collision` on/off collision timing

- Confirmed attachment context delivery at attack start

- Confirmed attachment context reset at attack end

### 4. Combo Input

- Validated `AnimNotify_PreInput` pre-input timing

- Validated next combo hit entry

- Confirmed state reset after the last hit ends

- Organized and verified the current policy that inputs inside the pre-input window buffer the next hit, while inputs outside the window are ignored

### 5. Shared Combat Core Integration

- Confirmed hit window / action context delivery for each hit

- Confirmed correct connection through the `ApplyDamage -> TakeDamage` flow

- Confirmed the current behavior that additional hits after dead / dying are handled with `CommittedDamage = 0`


---

## Test Steps

1. Verify that `Idle -> Action -> Idle` works correctly when performing a basic attack after equipping a weapon

2. Verify that combo inputs inside the pre-input window connect correctly as `1 -> 2 -> 3`

3. Verify that inputs outside the pre-input window are ignored without buffering the next hit

4. Verify that actual hits and `ApplyDamage / TakeDamage` logs occur only during collision windows

5. Verify that hit window ids and action context indices are delivered correctly for each hit

6. Verify that additional hits after HP reaches `0` are handled with `CommittedDamage = 0` and do not reduce HP further


---

## Related Issue / Branch

- Branch: `feature/player-combat-loop`

- Related work:

  - `M03-04: Stabilize One Full Player Combat Loop (#37)`


---

## Notes

- The focus of this PR is to close the player attack loop itself, and extended policies such as air combo, action cancel, guard, or dodge chaining are outside the current scope.

- The current policy is that only inputs inside the pre-input window are treated as buffered requests for the next hit, while inputs outside the window are ignored.

- Combat stat expansion and additional combat policies are planned to continue in later branches.


---