# UE5 Portfolio Issue Checklist

## Title

**M03-04: Stabilize One Full Player Combat Loop**

### Date

- **Day 14**

- **Date : 2026.04.06**


---

### Goals

- Stabilize one full cycle of the player loop from input to attack end.

- Organize weapon equipped state, action state transitions, notify-based collision timing, and combo pre-input flow into a playable state.

- Organize the player attack loop so it connects naturally with the shared combat core.


---

### Branch

- `feature/player-combat-loop`


---

### TODO List

#### 1. Organize Input and Entry Conditions

- [x] Organize `ComboAction` input entry conditions

- [x] Confirm that attacking is only possible while equipped

- [x] Organize attack restriction policy outside Idle state

- [x] Organize input handling policy during Reaction / Dead states

#### 2. Organize Action State Transitions

- [x] Validate `Idle -> Action -> Idle` state transitions

- [x] Confirm linkage between `ActionComponent` and `StateComponent`

- [x] Confirm state return after attack end


#### 3. Organize Notify-Based Attack Flow

- [x] Validate `AnimNotify_Action` begin/end linkage

- [x] Validate `AnimNotify_Collision` on/off collision timing

- [x] Confirm attachment context delivery at attack start

- [x] Confirm attachment context reset at attack end


#### 4. Organize Combo Input

- [x] Validate `AnimNotify_PreInput` pre-input timing

- [x] Validate next combo hit entry

- [x] Confirm state reset after the last hit ends

- [x] Organize combo miss input handling policy


#### 5. Integrated Validation

- [x] Scenario 1: start a basic attack after equip

- [x] Scenario 2: hits occur only during collision windows

- [x] Scenario 3: combo input links correctly

- [x] Scenario 4: confirm return to Idle after attack ends


---

### Current Outcome

- The player attack loop has been organized so that one cycle closes through `Input -> Action -> Notify -> Collision -> ApplyDamage -> TakeDamage -> End`.

- Combo flow works as `1 -> 2 -> 3`, and the next input starts a new cycle again from `1`.

- Inputs inside the pre-input window are treated as buffered requests for the next hit, while inputs outside the pre-input window are ignored.

- Hit windows and action context are delivered for each hit and are organized to connect correctly with the shared combat core.


---

### Notes

- The purpose of this issue is to close the player attack loop itself, while hit / death handling expansion was organized in preceding issues.

- Input handling and AnimNotify linkage were stabilized first because they become the foundation for later combat expansion.

---

### TODO

- Review whether attack input should be allowed during Jump state

- Review whether action cancel should be allowed and define the allowed timing policy

- If needed, define the minimum combo debug logs to keep


---
