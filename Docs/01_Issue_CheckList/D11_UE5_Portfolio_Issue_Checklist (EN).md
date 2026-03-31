# UE5 Portfolio – Issue Checklist

## Title

**M03-01: AI BehaviorTree Core and Enemy AI State/Combat Pipeline Setup**

### Date

- **Day 11**
  
- **Date : 2026.01.26**


---

### Goals

- Establish the core setup of `AIController` / `Blackboard` / `BehaviorTree`, and finalize the Enemy AI state transition pipeline.

- Organize the state transition structure of `Idle / Patrol / Investigate / Chase / Alert / Engage / HitReact / Dead` based on the `Perception -> AIContext -> AIState -> BT Branch` flow.

- Validate state transitions and combat flow for a **single Enemy** in the test level, and secure log-based traceability.


---

### Branch

- `feature/ai-behaviortree-core`


---

### TODO List

#### 1. Build AIController Core and Initialization Routine

- [x] Implement `CAIController` class
      
- [x] Initialize Blackboard in `OnPossess`
      
- [x] Run BehaviorTree in `OnPossess`
      
- [x] Cache owned Pawn and validate Enemy
      
- [x] Add Blackboard Key validation routine
      
- [x] Define initial Blackboard value setup rules
      
- [x] Define minimal debug log rules


---

#### 2. Organize Blackboard Key System

- [x] Organize key system based on `CAIKey` namespace
      
- [x] Define Targeting / State / Perception / Metric / Navigation keys
      
- [x] Define Patrol / Investigate / Chase / Alert / Engage keys
      
- [x] Define Reaction / Dead related keys
      
- [x] Finalize initial value setup rules at Spawn / Possess timing


---

#### 3. Build AI Perception and Target Context

- [x] Integrate `AIPerceptionComponent`
      
- [x] Configure Sight settings
      
- [x] Handle Perception events
      
- [x] Build target cache based on `TargetDataMap`
      
- [x] Apply target priority / LOS / memory timeout policies
      
- [x] Build `BuildPerceptionContext()` flow


---

#### 4. Build AIContext Update Services

- [x] Build `UpdateAIContext` service
      
- [x] Update Perception context
      
- [x] Update Home metric
      
- [x] Compute Alert range
      
- [x] Request/apply Engage assignment
      
- [x] Update Reaction context
      
- [x] Update Dead context
      
- [x] Organize Blackboard clear policies by situation
  

---

#### 5. Build AIState Transition Service

- [x] Build `UpdateAIState` service
      
- [x] Define priority order: `Dead > HitReact > Engage > Investigate/Chase/Alert/Idle`
      
- [x] Decide state based on Target / LOS / AlertRange / Engage conditions
      
- [x] Organize Blackboard clean-up rules on state transition
      
- [x] Organize attack-related key reset rules when leaving Engage
  

---

#### 6. Build BehaviorTree State Branches

- [x] Build root-based state branch structure
      
	- [x] `Idle`
	      
	- [x] `Patrol`
	      
	- [x] `Investigate`
	      
	- [x] `Chase`
	      
	- [x] `Alert`
	      
	- [x] `Engage`
	      
	- [x] `HitReact`
	      
	- [x] `Dead`
		  
	- [x] Connect Decorators for each Branch entry condition


---

#### 7. Build Patrol / Investigate / Chase / Alert Flows

- [x] Build patrol flow based on patrol path / point
      
- [x] Build investigate flow based on investigate location / index
      
- [x] Build chase flow based on distance conditions and movement
      
- [x] Build alert flow based on alert point selection and step movement
      
- [x] Build movement speed / focus control nodes per state


---

#### 8. Build Engage Assignment and Combat Flow

- [x] Add `UCWorldSubsystem_CombatEngage`
      
- [x] Build Engage request / assignment structure
      
- [x] Apply Engage / Alert role distribution rules for multiple AI against the same target
      
- [x] Compute `bInEngageRange`, `bCanAttack` through `UpdateEngageContext`
      
- [x] Build Engage positioning subtree
      
- [x] Organize Attack subtree entry conditions


---

#### 9. Build Attack Subtree and Attack Loop

- [x] `SelectAttackIndex`
      
- [x] `StartAttack`
      
- [x] `WaitAttackEnd`
      
- [x] `CommitAttackCooldown`
      
- [x] Link `AnimNotify_EndEnemyAttack`
      
- [x] Organize `bIsAttacking` maintenance rule during attack
      
- [x] Validate re-entry / cooldown after attack ends


---

#### 10. Build Reaction / Dead State Flow

- [x] Apply pending / active reaction structure
      
- [x] Build `TryStartReaction`, `WaitEndReaction`
      
- [x] Validate `HitReact` state entry and exit
      
- [x] Synchronize `DeadState` with Blackboard
      
- [x] Build `StayDead`, `WaitDeadState`, `StartRevive`
      
- [x] Link `AnimNotify_EnterDeadState`, `AnimNotify_EnterAliveState`
  

---

#### 11. Integrated Validation Scenarios

- [x] Scenario 1: Enemy Spawn -> remain Idle
      
- [x] Scenario 2: Patrol loops when Patrol is configured
      
- [x] Scenario 3: Detect Target -> transition to Chase
      
- [x] Scenario 4: Enter Alert distance -> transition to Alert
      
- [x] Scenario 5: Engage assignment applied -> transition to Engage
      
- [x] Scenario 6: Attack start -> end -> cooldown -> re-attack
      
- [x] Scenario 7: Enter HitReact on hit and return
      
- [x] Scenario 8: Stop all behavior on death and remain in Dead state
      
- [x] Scenario 9: Confirm return to Alive state on revive


---

### Notes

- The purpose of this issue is to close the **Enemy AI BT core and state transition pipeline**, while the player combat loop itself will be expanded in follow-up issues.

- The existing `Combat`-centric terminology should be reorganized around `Engage` in the current structure, and Blackboard keys as well as services/tasks should follow the same convention.

- The core of this issue is not simply creating BT assets, but consistently organizing the `Perception -> Context -> State -> Branch -> Action` flow based on both code and Blackboard.


---
