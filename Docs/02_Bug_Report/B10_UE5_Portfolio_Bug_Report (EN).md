# UE5 Portfolio Bug Report (EN)

## Title

**M05-B10: Action trail / collision runtime effects are not cleared after hit or dead interruption**

### Date

- **2026.06.04**

### Type

- Bug

### Status

- [x] Resolved

### Branch

- `feature/orchestration-refactor`

---

## Summary

- When an action is interrupted by hit reaction or dead reaction while trail, collision, or hit context is active, the weapon trail can remain enabled.
- The root cause was that runtime effect cleanup depended on animation notify end or feedback end flow.
- Since notify end is not guaranteed to run in the intended order when a montage is interrupted, external runtime state must be cleared explicitly from the executor finish path.
- `CleanupRuntimeEffects()` was added, and feedback request creation/execution was split into `BuildFeedbackRequest()` and `PlayFeedbackRequest()`.

---

## Environment

- Engine: Unreal Engine 5.4
- Branch:
  - `feature/orchestration-refactor`

### Related Code

- `Source/Portfolio/Action/CAction.cpp`
- `Source/Portfolio/Action/CAction.h`
- `Source/Portfolio/Action/CAction_ComboAttack.cpp`
- `Source/Portfolio/Reaction/CReaction.cpp`
- `Source/Portfolio/Reaction/CReaction.h`
- `Source/Portfolio/Component/CActionFeedbackComponent.cpp`
- `Source/Portfolio/Component/CActionFeedbackComponent.h`
- `Source/Portfolio/Component/CReactionFeedbackComponent.cpp`
- `Source/Portfolio/Component/CReactionFeedbackComponent.h`
- `Source/Portfolio/Component/CWeaponComponent.cpp`
- `Source/Portfolio/Component/CWeaponComponent.h`

---

## Reproduction Steps

1. Run a sword combo attack or any action that enables trail feedback.
2. Interrupt the character with hit reaction while the action trail or collision window is open.
3. Repeat the same scenario with dead reaction interruption.
4. Check whether weapon trail, collision, and hit context are cleared after interruption.

---

## Expected Result

- Action runtime effects should be cleared immediately when an active action is interrupted by hit or dead reaction.
- Weapon trail should be disabled.
- Weapon collision should return to the disabled state.
- Apply damage hit window should be closed.
- Pushed hit context should be cleared.
- Interrupt / complete feedback should still play normally after cleanup.

---

## Actual Result

- When hit or dead interruption occurred while action trail was active, the trail could remain enabled.
- If collision notify end or hit context notify end did not run on the interruption path, collision / hit context could also remain stale.
- Existing `ClearRuntime()` only cleared executor cache and did not clear external component / weapon actor runtime state.

---

## Root Cause

- Trail off, collision disabled, and hit context clear depended on notify end or feedback data.
- `UCAction::Stop()` and `UCReaction::Stop()` did not explicitly clean runtime effects after stopping the montage.
- Existing `RequestFeedback()` created and executed feedback in one function, so terminal feedback was difficult to move after `ClearRuntime()`.
- Cleanup order and terminal feedback execution order were not clearly separated.

---

## Fix

The finish flow was reorganized as follows.

```text
Capture terminal feedback request / event payload
-> StopMontage
-> CleanupRuntimeEffects
-> ClearRuntime
-> PlayFeedbackRequest
-> EmitEvent / HandleFinished
```

### Action

- Added `UCAction::CleanupRuntimeEffects()`.
- Added `UCWeaponComponent::ClearRuntimeWeaponState()` to clear hit context and collision.
- Added `UCActionFeedbackComponent::ClearRuntimeFeedback()` to force weapon trail off.
- Removed `RequestFeedback()` and unified all action feedback calls into `BuildFeedbackRequest()` + `PlayFeedbackRequest()`.
- `Stop()` / `Complete()` now capture feedback request and action index before cleanup / clear, then play feedback and emit events after runtime clear.

### Reaction

- Added `UCReaction::CleanupRuntimeEffects()`.
- Added `UCReactionFeedbackComponent::ClearRuntimeFeedback()`.
- Reaction feedback cleanup is currently no-op, but the hook is aligned for future loop VFX/SFX cleanup.
- Removed `RequestFeedback()` and unified reaction feedback calls into `BuildFeedbackRequest()` + `PlayFeedbackRequest()`.

---

## Compile Verification

- Confirmed that no `RequestFeedback` references remain.
- `git diff --check` passed.
- `PortfolioEditor Win64 Development` build succeeded.

```text
Target is up to date
Total execution time: 0.63 seconds
```

---

## Runtime Verification Checklist

- [x] Verify trail turns off when hit reaction interrupts during attack trail ON.
- [x] Verify trail turns off when dead reaction interrupts during attack trail ON.
- [x] Verify collision is disabled and hit window is closed when interruption occurs during a collision window.
- [x] Verify hit context is cleared when interruption occurs after hit context notify begin.
- [x] Verify normal combo attack 0-1-2 trail / collision / feedback timing regression.
- [x] Verify reaction interrupt / complete feedback still plays through the snapshot-based execution path.

---

## Notes

- `ClearRuntime()` only owns executor internal cache/state cleanup.
- `CleanupRuntimeEffects()` only owns external runtime side effects that may remain on components or weapon actors.
- Terminal feedback is executed after cleanup, and the required request data is captured as a snapshot before cleanup.

---
