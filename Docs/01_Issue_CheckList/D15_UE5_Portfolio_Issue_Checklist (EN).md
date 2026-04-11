# UE5 Portfolio – Issue Checklist

## Title

**M03-05: Validate and Organize Enemy Combat Receiver**

### Date

- **Day 15**

- **Date : 2026.04.07**


---

### Goals

- Re-validate the Enemy's existing `TakeDamage / Health / Reaction` structure against real combat usage and supplement missing policies.

- Organize hit, reaction, death, and revive flows so they close stably in actual combat situations.

- Validate that the Player attack loop and Enemy receive loop connect consistently.


---

### Branch

- `feature/enemy-combat-receiver`


---

### TODO List

#### 1. Validate Damage Receive Flow

- [ ] Validate the current `TakeDamage -> Health -> Reaction` flow

- [ ] Confirm `TakeDamageComponent` entry conditions

- [ ] Confirm invalid request handling

- [ ] Confirm minimal Enemy-side logging


#### 2. Validate Dead State Transitions

- [ ] Validate `Alive -> Dying -> Dead` transitions

- [ ] Validate synchronization with `AnimNotify_EnterDeadState`

- [ ] Confirm movement/action stop in Dead state

- [ ] Validate re-hit handling policy in Dead state


#### 3. Review Revive Flow

- [ ] Review revive entry scenario

- [ ] Review revive cancel scenario

- [ ] Validate synchronization with `AnimNotify_EnterAliveState`

- [ ] Confirm state reset policy after revive


#### 4. Validate Reaction Flow

- [ ] Validate HitReact entry on hit

- [ ] Validate state return after Reaction ends

- [ ] Review conflict between death transition and reaction

- [ ] Review additional hit policy during reaction


#### 5. Integrated Validation

- [ ] Scenario 1: player attack -> Enemy HP decreases

- [ ] Scenario 2: player attack -> Enemy enters HitReact

- [ ] Scenario 3: accumulated hits -> Enemy transitions to Dead

- [ ] Scenario 4: confirm return to Alive state after revive


---

### Notes

- This issue focuses more on **validating whether the existing Enemy receive loop is actually closed** than on adding new features.

- Later AI combat loop validation should proceed based on the results of this issue.


---
