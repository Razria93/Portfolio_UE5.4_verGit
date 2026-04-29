# UE5 Portfolio Bug Report (EN)

## Title

**M4-B08: AI combat flow breaks after reaction when hit during an active combo action**

### Date

- **2026.04.29**

### Type

- Bug

### Status

- [x] Resolved

### Branch

- `feature/action-orchestration`


---

## Summary

- When AI entered Reaction after being hit during `ComboAttack`, the post-reaction action / state / blackboard state could diverge and break later combat flow.

- A representative failure case was that `ExecutionState` returned to Idle while `CurrentActionType` still remained as the previous `ComboAttack`, which caused repeated `NoExecutableAction` rejections when AI tried to attack again.

- This was fixed by introducing a structure that aborts the current active action before reaction takeover begins.

- Combat availability calculation was also aligned with reaction state so that combat recovery after reaction stays consistent.


---

## Environment

- Engine: Unreal Engine 5.4

- Branch:

  - `feature/action-orchestration`

- Related Code:

  - `Source/Portfolio/Component/CReactionComponent.cpp`
  - `Source/Portfolio/Component/CActionComponent.cpp`
  - `Source/Portfolio/AI/BehaviorTree/Service/CBTService_UpdateEngageContext.cpp`
  - `Source/Portfolio/AI/BehaviorTree/Task/CBTTask_StartCombatAction.cpp`

- Related Asset:

  - `Content/02_Controller/02_Enemy/AI/Blackboard/BB_Default.uasset`


---

## Reproduction Steps

1. Let the AI execute `ComboAttack`.

2. Hit the AI during the combo action so that it enters Reaction.

3. Let the reaction finish and allow the AI to attempt combat again.

4. Check whether combat flow recovers normally.


---

## Expected Result

- Reaction takeover should properly clean up the currently active action lifecycle.

- After reaction ends, `ExecutionState`, `CurrentActionType`, and combat blackboard state should return to a consistent state.

- The AI should then be able to start combat actions normally again.


---

## Actual Result

- Reaction could end while the previous action state was not fully cleaned up.

- `ExecutionState` could return to Idle while `CurrentActionType` still remained as `ComboAttack`.

- Combat availability in Blackboard could reopen, but real action execution would still reject, causing stuck AI or repeated invalid retries.


---

## Cause

- Reaction takeover had no explicit step to stop the currently active action.

- In other words, Reaction changed execution state but did not own action lifecycle abort.

- As a result, ActionComponent, StateComponent, and Blackboard could each observe a different effective state.


---

## Fix

Before starting a reaction, the structure now aborts the current action if one is active.

```cpp
if (UCActionComponent* actionComp = OwnerCharacter_Cached
	? OwnerCharacter_Cached->FindComponentByClass<UCActionComponent>()
	: nullptr)
{
	if (actionComp->GetCurrentActionType() != EActionType::Idle)
	{
		actionComp->AbortCurrentAction(EActionAbortReason::Reaction);
	}
}
```

The following cleanup was also aligned with that change.

- reaction state is reflected in combat availability calculation
- combat action cooldown commit ownership is moved into `StartCombatAction`
- combat blackboard state is kept more consistent with actual action state after reaction


---

## Verification Result

- Confirmed that AI can return to normal combat flow after entering Reaction during a combo action.

- Confirmed that `ExecutionState`, `CurrentActionType`, and combat blackboard state now stay more consistent.

- Confirmed that the previous repeated `NoExecutableAction` rejects and stuck-AI behavior no longer reproduce.


---

## Follow-Up Cleanup

- Reaction takeover policy will later be reorganized further as part of `ReactionOrchestrator` work.

- In the long term, Action / Reaction takeover can be lifted into a higher-level coordination structure if needed.


---

## Notes

- The key fix was that reaction must clean up the previous action lifecycle, not just switch execution state.

- The underlying issue was not a single Blackboard flag, but a divergence between ActionComponent, StateComponent, and Blackboard state.


---
