# UE5 Portfolio – Issue Checklist

## Title

**M03-01: Build baseline AIController + Blackboard + BehaviorTree pipeline**

### Date

- **Day 11**

- **Date : 2026.01.26**


---

### Goals

- Confirm the baseline setup for `AIController` / `Blackboard` / `BehaviorTree` and build the minimum Enemy AI pipeline.

- Organize combat/chase/idle state transitions through **Blackboard keys and BehaviorTree flow** to establish the foundation for later expansion (tactics/patterns/skills).

- Validate behavior in a test level for a **single Enemy** and secure log-based traceability.


---

### Branch

- feature/ai-behaviortree-core


---

### TODO List

#### 1. Base AIController class and ownership flow

- [ ] Create a `CAIController` (temporary name) class

- [ ] Implement Blackboard/BehaviorTree initialization in `OnPossess`

- [ ] Cache the owning Pawn (cast to Enemy + validity checks)

- [ ] Define minimal debug log rules (Controller, Pawn, BT Asset, BB Asset)


---

#### 2. Blackboard setup (key standardization)

- [ ] Create a Blackboard Asset and define base keys

  - [ ] `TargetActor` (Object)

  - [ ] `HomeLocation` (Vector)

  - [ ] `PatrolLocation` (Vector, optional)

  - [ ] `IsInCombat` (Bool)

  - [ ] `IsDead` (Bool)

  - [ ] `LastKnownTargetLocation` (Vector)

- [ ] Confirm key naming rules (prefix/type inclusion)

- [ ] Define key initialization rules (Spawn/OnPossess timing)


---

#### 3. BehaviorTree skeleton (minimum behavior)

- [ ] Build BT Root → Selector structure

  - [ ] Dead branch (highest priority)

  - [ ] Combat branch (chase/attack)

  - [ ] Idle/Patrol branch (wait/patrol)

- [ ] Define required Decorator conditions for each branch

- [ ] Build minimum runnable nodes (Wait, MoveTo, Simple Sequence)


---

#### 4. BT Task nodes (at least 2)

- [ ] Task: `SetTargetFromSense` or `UpdateTarget` (reflect perception output)

- [ ] Task: `MoveToTarget` or `MoveToLocation`

- [ ] Task: `ClearTarget` (reset on target loss)

- [ ] Add log rules for task results (success/failure reason)


---

#### 5. Service/Decorator minimum design (state updates)

- [ ] Service: `UpdateCombatState` (distance/line-of-sight checks)

- [ ] Decorator: `IsValidTarget` (TargetActor validation)

- [ ] Decorator: `IsInCombat` / `IsDead` (Blackboard Bool-based)

- [ ] Define flow switch policy on condition failure (return to Idle, Search, etc)


---

#### 6. AI Perception integration (optional / follow-up)

- [ ] Decide whether to add `AIPerceptionComponent`

- [ ] Confirm default settings for sight/hearing

- [ ] Define rules for perception events → Blackboard updates


---

#### 7. Integration validation scenarios

- [ ] Scenario 1: Enemy Spawn → Idle maintained (no target)

- [ ] Scenario 2: Target detected → Combat transition → MoveTo executed

- [ ] Scenario 3: Target lost → Combat cleared → return to Idle

- [ ] Scenario 4: Dead state → verify all actions stop


---

### Notes

- The initial BT focuses only on the **minimum flow (Idle/Combat/Dead)**, while attack/pattern/skill expansions are handled in follow-up issues.

- Design Blackboard keys with a **shared standard** in mind so they integrate with the combat system and other components.


---
