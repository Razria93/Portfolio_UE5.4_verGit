# UE5 Portfolio – Issue Checklist

## Title

**M03-02: Build Player Combat Receiver**

### Date

- **Day 12**

- **Date : 2026.03.31**


---

### Goals

- Extend the player from an attacker-only object into a combat entity that can process hit, reaction, and death states.

- Connect the `TakeDamage / Health / Reaction` axis to the player so it shares the same combat receive pipeline foundation as the Enemy.

- Organize the player so AI attacks are received correctly and state changes are reflected consistently based on Anim/State.


---

### Branch

- `feature/player-combat-receiver`


---

### TODO List

#### 1. Build Player Receive Components

- [ ] Add `TakeDamageComponent` to `ACPlayer`

- [ ] Add `HealthComponent` to `ACPlayer`

- [ ] Add `ReactionComponent` to `ACPlayer`

- [ ] Organize component initialization order in the Player constructor


#### 2. Connect Player Damage Entry Point

- [ ] Add `ACPlayer::TakeDamage()` override

- [ ] Connect the processing flow through `TakeDamageComponent`

- [ ] Organize fallback handling policy

- [ ] Confirm minimal player-side debug logging


#### 3. Connect Health / Dead States

- [ ] Confirm HP decrease is applied on hit

- [ ] Organize `DeadState` entry rules

- [ ] Organize dead-state re-hit policy

- [ ] Organize synchronization direction between Anim/State and DeadState


#### 4. Connect Reaction State

- [ ] Confirm reaction request on player hit

- [ ] Validate HitReact entry conditions

- [ ] Organize return rules to Idle/default state after Reaction ends

- [ ] Confirm movement/state control impact on the player side


#### 5. Integrated Validation

- [ ] Scenario 1: AI attack -> player HP decreases

- [ ] Scenario 2: AI attack -> player enters HitReact

- [ ] Scenario 3: accumulated hits -> enter DeadState

- [ ] Scenario 4: confirm additional hit handling in Dead state


---

### Notes

- The purpose of this issue is to integrate the player as a **combat-receiving entity**, while the player attack loop itself will be organized in a follow-up issue.

- The priority is to close the basic `TakeDamage -> Health -> Reaction -> Dead` loop before advanced combat systems.


---
