# Player Combat Receiver Implementation and Combat Receive Loop Integration

## Title

`✨ feat: implement Player Combat Receiver and connect combat receive loop (#32)`

## Summary

- This PR expands the Player from an attack-only object into a **combat-receivable entity**, and includes the work required to actually close the core loop of `TakeDamage -> Health -> Reaction -> Dead`.
  
- It connects `TakeDamageComponent`, `HealthComponent`, and `ReactionComponent` to `ACPlayer`, and sets up the receive entry so that AI attacks can be correctly applied to the Player.
  
- It also fixes the missing hit context issue on the Enemy attack side, so that attack index and action type are correctly propagated through the Player receive pipeline.
  
- In addition, this PR improves dead-state lifecycle handling and state sync, and organizes the related issue / architecture documentation together.


---

## Completed Items

### 1. Player Combat Receiver Connection

- Added `TakeDamageComponent` to `ACPlayer`
  
- Added `HealthComponent` to `ACPlayer`
  
- Added `ReactionComponent` to `ACPlayer`
  
- Organized component initialization order in the Player constructor
  
- Added `ACPlayer::TakeDamage()` override
  
- Connected the damage handling flow through `TakeDamageComponent`

### 2. Player Hit / Reaction Loop Connection

- Added a pending reaction consume loop in `ACPlayer::Tick()`
  
- Connected Player reaction execution using `TryConsumePendingReaction()` + `TryExecuteReaction()`
  
- Verified `HitReact` entry on Player hit
  
- Verified reaction replace / interrupt flow on consecutive hits

### 3. Health / Dead Lifecycle Improvement

- Added `UCHealthComponent::IsAlive()` / `IsDead()`
  
- Unified dead-state update paths through `ChangeDeadState()`
  
- Added `OnDeadStateChanged` delegate
  
- Connected dead-state sync from `HealthComponent -> StateComponent`
  
- Organized non-`Alive` states to be handled as dead-category gameplay states

### 4. Player Input Blocking Policy Connection

- Added `CanActionInput()`
  
- Blocked attack / equipment-switch input while in `Reaction`
  
- Blocked movement / action input based on `IsAlive()`
  
- Kept `StopJump` unblocked considering its release-oriented behavior

### 5. Enemy Attack Context Improvement

- Added `AttackActionType` to `CBTTask_StartAttack`
  
- Invoked `WeaponComponent->PushContextToAttachment()` at the start of Enemy attack
  
- Updated Enemy attacks so that hit context containing `Attachment / Equipment / Action / Index` is delivered to the Player
  
- Added a runtime guard for `AttackActionType == EActionType::Max`

### 6. State API and Log Cleanup

- Renamed `SetIdleMode / SetActionMode / SetReactionMode` to `SetIdleState / SetActionState / SetReactionState`
  
- Extracted `PrintStateChangedInfo()`
  
- Added state transition log output

### 7. Documentation

- Updated `D12_UE5_Portfolio_Issue_Checklist (KR)`
  
- Updated `D12_UE5_Portfolio_Issue_Checklist (EN)`
  
- Added `S02_UE5_Portfolio_System_Architecture (KR)`
  
- Added `S02_UE5_Portfolio_System_Architecture (EN)`
  
- Added `B04_UE5_Portfolio_Bug_Report (KR)`
  
- Added `B04_UE5_Portfolio_Bug_Report (EN)`


---

## Test Method

1. Make the Enemy detect and attack the Player
  
2. Verify that, on the first hit, the Player enters the `Reaction` state along with HP reduction
  
3. Verify that `HitReact` is replayed correctly according to the interrupt / replace policy during consecutive hits
  
4. Verify that when HP reaches 0 through accumulated hits, `DeadState` transitions as `Alive -> Dying -> Dead`
  
5. Verify that additional hits after entering the Dead state are invalidated with `FinalAppliedDamage = 0`
  
6. Verify Player input behavior
  
	- Attack input blocked during `Reaction`
	  
	- Movement / action input blocked in `Dead` state
	  
	- `StopJump` still performs normal release behavior
  
7. Verify hit context in Enemy attack logs
  
	- `AttachmentType`
	  
	- `EquipmentType`
	  
	- `ActionType`
	  
	- `ActionIndex`


---

## Related Issue / Branch

- Branch: `feature/player-combat-receiver`
  
- Issue: `#32`


---

## Notes

- This PR focuses on integrating the Player as a combat-receivable entity, while the Player attack loop itself is planned to be organized in a follow-up issue.
  
- In the current structure, `StateComponent` is also partially responsible for dead-state sync, but there is a high possibility that state-axis separation and `StateComponent` redefinition will be needed later during the `Combat Core Shared` work.
  
- `TakeDamageComponent` may be expanded into a `CombatReceiver` layer in the long term, but this PR prioritizes functional connection and loop verification.


---
