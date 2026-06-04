# UE5 Portfolio Bug Report (EN)

## Title

**M05-B09: Active execution is not stopped because Action / Reaction intervention allow windows are missing or notify state ranges are unstable**

### Date

- **2026.05.22**

### Type

- Bug

### Status

- [ ] In Progress

### Branch

- `feature/orchestration-refactor`

---

## Summary

- In the Action / Reaction intervention model, an active execution is not stopped by an incoming execution unless the active side explicitly opens the required `Allow` window.
- The main symptoms were that an attack Action montage kept playing after hit reaction was requested, and a new `HitReaction` was rejected during an active `HitReaction`.
- The issue was caused by missing active-side `Allow` filters, unclear override paths caused by `Super` calls, and unstable begin/end timing when an intervention notify state covered the full montage range from frame 0 to the last frame.

---

## Environment

- Engine: Unreal Engine 5.4
- Branch:
  - `feature/orchestration-refactor`

### Related Code

- `Source/Portfolio/Action/CAction.cpp`
- `Source/Portfolio/Reaction/CReaction.cpp`
- `Source/Portfolio/Reaction/CReaction_Hit.cpp`
- `Source/Portfolio/Reaction/CReaction_Dead.cpp`
- `Source/Portfolio/Component/CActionOrchestratorComponent.cpp`
- `Source/Portfolio/Component/CReactionOrchestratorComponent.cpp`
- `Source/Portfolio/Notify/CAnimNotifyState_ExecutionInterventionWindow.cpp`

### Related Assets

- `Content/04_Montage/Sword/M_Attack_Sword_0.uasset`
- `Content/04_Montage/Sword/M_Attack_Sword_1.uasset`
- `Content/04_Montage/Sword/M_Attack_Sword_2.uasset`
- `Content/04_Montage/Damaged/M_HitReact.uasset`

---

## Reproduction Steps

1. Make the enemy execute `ComboAttack` 0 / 1 / 2 in sequence.
2. Let the player get hit so that `HitReaction` is requested.
3. Add an intervention allow window to the `HitReaction` montage.
4. Compare notify state ranges from frame 0 to the last frame and from frame 1 to the frame before the last frame.
5. Check whether `HitReaction -> HitReaction` interruption works on the second hit.
6. Check whether an active attack Action is interrupted by `HitReaction` when the character is hit during the attack.

---

## Expected Result

- When `HitReaction` arrives during an active Action, the active Action should be interrupted.
- When a new `HitReaction` arrives during an active `HitReaction`, the current reaction should be replaced if it allows interruption.
- Intervention should succeed only when incoming Want, active Allow, and active-side allow filter cache are all valid.

---

## Actual Result

### 1. Action was not stopped by HitReaction

- Without an `Allow Interrupt by HitReaction` window on the attack Action, the attack montage kept playing like a super armor state after being hit.
- Once an interrupt allow window targeting `HitReaction` was added to the Action side, the Action was interrupted correctly.

### 2. HitReaction was not stopped by another HitReaction

- When a new `HitReaction` arrived from `ComboAttack` index 2 while `HitReaction` was already active, incoming Want returned true but active Allow returned false, so the request was rejected.

```text
[ResolveInterventionDirective]
bIncomingWants = true
bActiveAllows = false

[RequestDamageReaction]
ResultType = Rejected
RejectReason = ActiveCannotAcceptIntervention
```

### 3. Filter matching succeeded after adjusting the notify state range

- In the same situation, placing the notify state from frame 1 to the frame before the last frame kept `CounterpartFilters` cached correctly, and matching succeeded.

```text
[UCReaction::MatchesAnyInterventionFilter] Match Complete.
```

---

## Root Cause Analysis

### 1. The Allow window is the responsibility of the active execution

- `Want` expresses what the incoming execution wants to stop.
- `Allow` expresses what the active execution allows itself to be stopped by.
- Therefore, for a Hit reaction to interrupt an Action, both incoming `HitReaction` want interrupt and active `Action` allow interrupt by `HitReaction` are required.

### 2. HitReaction re-entry also requires Allow on the active HitReaction

- The same rule applies when a new `HitReaction` arrives during an active `HitReaction`.
- Even if the incoming executor wants to interrupt, intervention fails unless the active executor allows being interrupted.

### 3. Calling Super inside overrides makes the decision path unclear

- Even when a reaction overrides intervention logic, calling `Super` still passes through the base filter matching path.
- This can combine common filters with class-fixed policy, but it makes debugging harder because the accepted path becomes unclear.

### 4. Notify states covering frame 0 to the last frame can have begin/end timing issues

- When a notify state covers the full montage range, notify begin/end timing may behave unexpectedly during montage transition or chain consume.
- The observed behavior was that filters stayed cached on combo 0 and 1, but were not cached on combo 2, causing allow matching to fail.

---

## Fix Direction

### 1. Explicitly configure the required Allow windows for both Action and Reaction

- If an attack Action should be interrupted by Hit reaction, add an allow window targeting Hit reaction to the Action montage.
- If `HitReaction` should be interrupted by another `HitReaction`, add the corresponding allow window to the HitReaction montage.

### 2. Make Super call policy explicit in overrides

- If a specific executor should fully own its intervention policy, remove `Super` calls.
- If notify-filter policy and class-fixed policy should be combined, separate the two paths clearly with logs and comments.

### 3. Avoid placing intervention notify states exactly from frame 0 to the last frame

- Even when the window should cover almost the entire montage, avoid filling the full range from frame 0 to the last frame.
- Recommended placement is after frame 0 and before the last frame.

---

## Verification Criteria

- Check that an active Action is interrupted when `HitReaction` arrives during the attack.
- Check that `bIncomingWants = true` and `bActiveAllows = true` when a new `HitReaction` arrives during an active `HitReaction`.
- Check that `MatchesAnyInterventionFilter` prints the expected counterpart filter and reports match complete.
- Compare `0 frame ~ last frame` and `1 frame ~ last - 1 frame` placements to verify notify state range dependent cache behavior.

---

## Notes

- The key point is that "Hit reaction wants interrupt" is not enough by itself.
- Intervention requires both incoming Want and active Allow.
- The Action-not-stopped issue and the HitReaction-not-replaced issue are both explained by missing active-side allow configuration or allow filter cache failure.
- Therefore, this bug is not just a montage issue. It is also a validation case for the responsibility split in the intervention window model.

---
