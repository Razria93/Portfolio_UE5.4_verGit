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

- [x] Add `TakeDamageComponent` to `ACPlayer`

- [x] Add `HealthComponent` to `ACPlayer`

- [x] Add `ReactionComponent` to `ACPlayer`

- [x] Organize component initialization order in the Player constructor


#### 2. Connect Player Damage Entry Point

- [x] Add `ACPlayer::TakeDamage()` override

- [x] Connect the processing flow through `TakeDamageComponent`

- [x] Organize fallback handling policy

- [x] Confirm minimal player-side debug logging


#### 3. Connect Health / Dead States

- [x] Confirm HP decrease is applied on hit

- [x] Organize `DeadState` entry rules

- [x] Organize dead-state re-hit policy

- [x] Organize synchronization direction between Anim/State and DeadState


#### 4. Connect Reaction State

- [x] Confirm reaction request on player hit

- [x] Validate HitReact entry conditions

- [x] Organize return rules to Idle/default state after Reaction ends

- [x] Confirm movement/state control impact on the player side


#### 5. Integrated Validation

- [x] Scenario 1: AI attack -> player HP decreases

- [x] Scenario 2: AI attack -> player enters HitReact

- [x] Scenario 3: accumulated hits -> enter DeadState

- [x] Scenario 4: confirm additional hit handling in Dead state


---

### Notes

- The purpose of this issue is to integrate the player as a **combat-receiving entity**, while the player attack loop itself will be organized in a follow-up issue.

- The priority is to close the basic `TakeDamage -> Health -> Reaction -> Dead` loop before advanced combat systems.


---
