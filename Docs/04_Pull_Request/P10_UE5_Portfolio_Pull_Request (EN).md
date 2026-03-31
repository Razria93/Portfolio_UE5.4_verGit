# AI BehaviorTree Core Implementation and Combat/State Flow Setup

## Title

✨ feat: Build AI BehaviorTree Core and organize enemy combat state flow (#26)

## Summary

- Built the overall Blackboard / BehaviorTree-based core structure for enemy AI starting from `CAIController`.

- Organized AI state transitions and context update flow around BT-driven states:
  `Patrol / Investigate / Chase / Alert / Engage / HitReact / Dead`.

- Structured attack execution as an `SBT_Attack` subtree inside `Engage`


---

## Completed Work

### 1. Built AI Controller / Blackboard / BT core structure

- Added `CAIController`

- Implemented `Blackboard` / `BehaviorTree` initialization flow

- Organized blackboard key categories around the `CAIKey` namespace

- Introduced service-driven AIState / context update flow

### 2. Expanded AIState / Context pipeline

- Added `CBTService_UpdateAIState`

- Added `CBTService_UpdateAIContext`

- Added target cache / target priority / stale target memory timeout flow

- Organized state transition criteria:
	  
	- Idle
	  
	- Patrol
	  
	- Investigate
	  
	- Chase
	  
	- Alert
	  
	- Engage
	  
	- HitReact
	  
	- Dead

### 3. Implemented Patrol / Investigate / Chase / Alert flow

- Added `CPatrolPoint`, `CPatrolPath`, and related patrol tasks/services

- Implemented investigate flow based on `LastKnownLocation`

- Introduced EQS-based investigate flow

- Added chase hysteresis and alert-stage tasks / BT assets

### 4. Implemented Engage assignment and combat context flow

- Added `UCWorldSubsystem_CombatEngage`

- Implemented engage assignment request / result flow

- Separated distance-based alert decision from assignment-based engage decision

- Added `UpdateEngageContext` to compute:
	  
	- `bInEngageRange`
	  
	- `bCanAttack`

### 5. Migrated Combat structure into Engage structure

- Renamed `Combat`-centric naming into `Engage`-based naming

- Reorganized combat-related context / key / service / task semantics around `Engage`

- Moved `Attack` out of top-level state handling and into the `Engage` subtree

### 6. Added Attack subtree

- Added `SBT_Attack` and `SBT_Engage_Positioning`

- Added:
	  
	- `StartAttack`
	  
	- `WaitAttackEnd`
	  
	- `CommitAttackCooldown`
	  
	- `SelectAttackIndex`
	  
	- `AnimNotify_EndEnemyAttack`
	
- Updated `UpdateAIState` so the top-level state remains `Engage` while attacking

### 7. Reorganized Reaction / HitReact flow

- Refactored reaction flow into a request-consume-execute model

- Split pending and active reaction states

- Added reaction priority policy

- Added:
	  
	- `BT_HitReact`
	  
	- `TryStartReaction`
	  
	- `WaitEndReaction`
	
- Controlled reaction windows via `AnimNotifyState_Reaction`

### 8. Expanded Dead / Revive state structure

- Replaced `bIsDead` with explicit states:	`Alive / Dying / Dead / Reviving`

- Synchronized `DeadState` across Blackboard / AnimInstance

- Added dead / revive BT tasks

- Added:
	  
	- `AnimNotify_EnterDeadState`
	
	- `AnimNotify_EnterAliveState`
	
- Added explicit kill / revive entry points

- Refined revive health initialization policy

### 9. Organized Anim / Montage / Weapon / Asset setup

- Organized animation and montage assets for:
	  
	- Dead
	  
	- Dying
	  
	- HitReact
	  
	- Attack
	
- Cleaned up sword / unarmed animation naming

- Added AI-only sword attack / draw / sheath montages

- Replaced old sword assets with the medieval sword set


---

## Test Steps

1. Run a test level with enemy AI placed in the scene
	
2. Verify Blackboard / BT initialization
	
	- `CAIController` possess flow
	  
	- `BB_Default` / `BT_Default` startup
	
3. Verify state transitions
	   
	- Idle / Patrol / Investigate / Chase / Alert / Engage / HitReact / Dead
	
4. Verify combat flow
	
	- Alert -> Engage transition
	  
	- engage assignment result
	  
	- `bInEngageRange` / `bCanAttack` update
	
5. Verify Attack subtree
	   
	- `SBT_Attack` entry
	  
	- `StartAttack`
	  
	- `bIsAttacking == true`
	  
	- `WaitAttackEnd`
	  
	- normal exit through `AnimNotify_EndEnemyAttack`
	
6. Verify HitReact / Dead / Revive flow
	   
	- reaction start / end
	  
	- dead phase transition
	  
	- revive restores health / state / anim sync correctly
	
7. Verify movement branch gating
	   
	- `CanMove` decorator blocks movement sequences when movement is locked


---

## Related Issue / Branch

- Branch: `feature/ai-behaviortree-core`

- Issue: `#26`


---

## Notes

- Current movement gating still relies on a single `bCanMove` flag, so lock ownership conflicts remain a follow-up improvement area.


---
