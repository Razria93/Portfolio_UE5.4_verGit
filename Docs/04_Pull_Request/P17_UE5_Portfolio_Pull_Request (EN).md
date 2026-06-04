# Clean Up Action / Reaction Intervention Rules and Reinforce Runtime Cleanup

## Title

`refactor: clean up action / reaction intervention rules and reinforce runtime cleanup (#52)`

## Summary

- This PR **reorganizes Action / Reaction intervention decision logic around data-driven rules** and unifies actual stop results under `Interrupted`.

- The core direction is as follows.

	- Move Action / Reaction intervention filters from notify-injected filters to `ActionData` / `ReactionData` rules.

	- Remove `Cancel`-family stop / finish / feedback timing values, and unify actual stop results under `Interrupted`.

	- Split `WantInterventionRules` and `AllowInterventionRules` to clarify responsibility between incoming intent and active-side allow policy.

	- Reorganize `AllowInterventionWindow` as `WindowKey`-based runtime state, and reduce the notify responsibility to delivering window timing only.

	- Use Dodge as a real action intervention validation case, and update related timing / allow window asset data.

	- Reinforce cleanup paths so runtime side effects such as trail, collision, and hit context do not remain after action / reaction finish.

	- Split feedback request creation and playback into `BuildFeedbackRequest()` / `PlayFeedbackRequest()` to clarify feedback execution order after finish.

- This PR also documents intervention allow window / notify range issues and runtime effect cleanup issues as bug reports.

---

## Completed Items

### 1. Data-driven Intervention Rules

- Removed the old structure where notifies directly opened and closed owner / counterpart filters.

- Added `FExecutionInterventionWantRule` and `FExecutionInterventionAllowRule`.

- Added the following rule arrays to `FActionData` / `FReactionData`.

	- `WantInterventionRules`

	- `AllowInterventionRules`

- Reorganized `WantIntervention()` so it matches the active participant based on the incoming context's want rules.

- Reorganized `AllowIntervention()` so it matches the incoming participant based on the active data's allow rules.

- Reorganized window timing so it is evaluated only by Allow rules.

### 2. Allow Intervention Window Cleanup

- Reduced `CAnimNotifyState_ExecutionInterventionWindow` responsibility to delivering `WindowKey` begin / end events.

- Kept the notify class / file name for asset stability, but cleaned up the display name as `Allow Intervention Window`.

- Renamed the `ActiveInterventionWindowKeys` family to `AllowInterventionWindowKeys` to match its actual meaning.

- Reorganized Action / Reaction component window handlers as `HandleActionAllowInterventionWindowBegin/End` and `HandleReactionAllowInterventionWindowBegin/End`.

- The active executor now manages open `WindowKey` values as a runtime set, and uses them only when evaluating `Window` timing in `AllowInterventionRules`.

### 3. Cancel / Interrupt Unification

- Removed `EExecutionStopReason::Cancelled`.

- Removed `Cancel`-family values from Action / Reaction stop reasons, finish reasons, and feedback timing.

- Unified the result of external execution stopping an active execution under `Interrupted`.

- Kept `EExecutionApplyMode::Intervene` as the meaning of applying an intervention, while the directive's `Interrupted` now represents the actual stop reason.

### 4. Reaction-specific Policy Cleanup

- Removed the `UCReaction_Hit::WantIntervention()` override and moved it into the data-rule flow.

- Removed the `UCReaction_Dead::WantIntervention()` override.

- Kept the dead reaction force-intervention policy in the orchestrator.

- Kept `UCReaction_Dead::AllowIntervention()` as a terminal guard that prevents an active dead reaction from being interrupted by another incoming execution.

### 5. Runtime Effect Cleanup Reinforcement

- Added `UCAction::CleanupRuntimeEffects()` and `UCReaction::CleanupRuntimeEffects()`.

- Added `UCWeaponComponent::ClearRuntimeWeaponState()` to clean up hit context and collision runtime state.

- Added `UCActionFeedbackComponent::ClearRuntimeFeedback()` so weapon trail can be forcibly turned off.

- Added `UCReactionFeedbackComponent::ClearRuntimeFeedback()` as a no-op hook to provide an extension point for future loop VFX / SFX cleanup.

- Reorganized action / reaction `Stop()` and `Complete()` so internal runtime state is cleared after runtime cleanup.

### 6. Feedback Request Snapshot Split

- Removed the old `RequestFeedback()`.

- Unified all feedback calls through `BuildFeedbackRequest()` + `PlayFeedbackRequest()`.

- Reorganized terminal feedback so the request snapshot is created before cleanup / clear, then played after cleanup / clear.

- Captured required Action event payload values before `ClearRuntime()` so the correct action index can still be delivered after clear.

### 7. Dodge / Asset / Data Changes

- Configured data rules and allow windows so Dodge can interrupt an active action through an `Interrupted` intervention and then execute.

- Updated Dodge timing, effect socket name, and allow intervention window asset data.

- Updated Player / Enemy BP intervention rule data.

- Adjusted allow window / timing settings for hit reaction montage and sword attack montage.

- Reflected the Quinn skeleton socket change.

### 8. Added Bug Report Documents

- `B09`: Documents the issue where active execution was not interrupted because of missing Action / Reaction intervention window settings and notify range problems.

- `B10`: Documents the issue where Action trail / collision runtime effects were not cleaned up after hit or dead interrupt.

---

## Verification Details

### Build

- `PortfolioEditor Win64 Development` build succeeded.

```text
Target is up to date
Total execution time: 0.54 seconds
```

### Static Checks

- Code search found no remaining `Cancelled`, `RequestFeedback`, or old intervention window API usage.

- The only remaining `Cancel` search result is `UCHealthComponent::TryCancelRevive()`, which is unrelated to this orchestration change.

- `git diff --check` still needs to pass.

### Manual Verification

- Verified sword equip / unequip works correctly.

- Verified combo attack 0-1-2 chain works correctly while the weapon is equipped.

- Verified combo attack is rejected while the weapon is not equipped.

- Verified input outside the combo chain window is not reserved.

- Verified Dodge interrupts the active action and executes within an allowed allow window.

- Verified Hit reaction intervenes correctly during an action-interruptible window.

- Verified Dead reaction executes with final priority regardless of active action / reaction state.

- Verified collision / hit context / allow intervention window notify begin / end work correctly.

- Verified trail, collision, and hit context are cleaned up after hit / dead interrupt while trail is ON.

- Verified reaction interrupt / complete feedback plays correctly through the snapshot-based execution path.

---

## Review Points

- Confirm whether the structure matches the intent: Want matches the active participant through incoming data rules, while Allow matches the incoming participant through active data rules.

- Confirm whether the current policy where window timing applies only to Allow rules matches the intended intervention responsibility split.

- Confirm whether keeping dead reaction force intervention as orchestrator policy rather than a data rule is appropriate.

- `OnMontageEnd(bInterrupted=true)` was not changed in this PR to preserve single ownership of context; unexpected montage interruption handling policy should be reviewed separately in the next branch.

- Confirm whether the structure where notifies deliver only `WindowKey` and filter policy is owned by data rules works well for asset configuration UX.

---

## Notes

- This PR is the intervention rule / runtime cleanup pass for closing `feature/orchestration-refactor`.

- The `S23` planning document differs from the final implementation in some places, so it is not included in this PR scope.

- Additional global policy, unexpected montage interruption report API, and loop feedback cleanup will be reviewed for extension in the next branch.
