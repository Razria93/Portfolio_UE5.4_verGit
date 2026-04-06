# UE5 Portfolio – Issue Checklist

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

- [ ] Organize `ComboAction` input entry conditions

- [ ] Confirm that attacking is only possible while equipped

- [ ] Organize attack restriction policy outside Idle state

- [ ] Organize input handling policy during jump/hit/dead states


#### 2. Organize Action State Transitions

- [ ] Validate `Idle -> Action -> Idle` state transitions

- [ ] Confirm linkage between `ActionComponent` and `StateComponent`

- [ ] Confirm state return after attack end

- [ ] Review whether action cancel should be allowed


#### 3. Organize Notify-Based Attack Flow

- [ ] Validate `AnimNotify_Action` begin/end linkage

- [ ] Validate `AnimNotify_Collision` on/off collision timing

- [ ] Confirm attachment context delivery at attack start

- [ ] Confirm attachment context reset at attack end


#### 4. Organize Combo Input

- [ ] Validate `AnimNotify_PreInput` pre-input timing

- [ ] Validate next combo hit entry

- [ ] Confirm state reset after the last hit ends

- [ ] Organize combo miss input handling policy


#### 5. Integrated Validation

- [ ] Scenario 1: start a basic attack after equip

- [ ] Scenario 2: hits occur only during collision windows

- [ ] Scenario 3: combo input links correctly

- [ ] Scenario 4: confirm return to Idle after attack ends


---

### Notes

- The purpose of this issue is to close the player attack loop itself, while hit/death processing is organized in preceding issues.

- Input handling and AnimNotify linkage should be stabilized first because they become the foundation for later combat expansion.


---
