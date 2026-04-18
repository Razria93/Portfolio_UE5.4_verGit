# Build the Combat Feedback Pipeline and Unify Player / Enemy Feedback Flow

## Title

`✨ feat: build the combat-feedback pipeline and unify player / enemy feedback flow (#39)`

## Summary

- This PR implements **action-feedback / reaction-feedback on both the Player-side and Enemy-side**, and additionally introduces **player-feedback** for feedback that should be applied locally on the Player-side.

- **action-feedback** is handled as follows.

  - `AnimNotifyState_ActionFeedback` executes **Trail On / Off**.

  - `AnimNotify_ActionFeedback` executes **Sword SFX** and **Buff VFX / Buff SFX**.

  - `CAction::BeginPlayAction` and `CAction::EndPlayAction` execute **ActionStart / ActionEnd**.

- **reaction-feedback** is connected to the flow that dispatches `TakeDamage` results and implements **hit VFX / hit SFX / Hit Stop**, while **player-feedback** implements **CameraShake**.

- In addition, the **action-feedback request generation flow** is reorganized from a `CAction`-direct lookup structure into a **shared owner-level interface-based flow**, and **Enemy attack-end cleanup** is moved into `EndAttackTask`.


---

## Completed Items

### 1. Separated feedback layers

- Organized combat feedback into **reaction-feedback**, **action-feedback**, and **player-feedback**

- Separated **shared Player / Enemy feedback** from **Player-local feedback**

### 2. Organized the shared combat-feedback path

- Added `CWorldSubsystem_CombatFeedback`

- Built the world-level combat feedback handling path

- Organized a shared path for **hit-stop** and **camera shake**

- Separated **player-facing feedback** from **shared feedback** execution

### 3. Implemented reaction feedback

- Added the `CReactionFeedbackComponent` structure

- Connected **reaction-feedback** to the dispatch flow after `TakeDamage`

- Implemented **hit VFX / hit SFX / Hit Stop** on both the Player-side and Enemy-side

- Organized reaction results to flow into the shared combat-feedback path

### 4. Implemented player feedback

- Added `CPlayerFeedbackComponent`

- Separated Player-local feedback handling flow

- Implemented **`CameraShake`** for feedback that should be applied only to the corresponding Player

- Delegated camera-shake handling to the player-feedback layer

### 5. Implemented action feedback

- Added `CActionFeedbackComponent`

- Separated **Trail / VFX / SFX** execution paths

- Implemented **`Trail On / Off`** through `AnimNotifyState_ActionFeedback`

- Implemented **`Sword SFX`** on both the Player-side and Enemy-side through `AnimNotify_ActionFeedback`

- Implemented **`Buff VFX / Buff SFX`** through `AnimNotify_ActionFeedback` so that one-shot feedback timing can be controlled through notifies

- Added action-feedback APIs that can run at **`ActionStart` / `ActionEnd`**

### 6. Reorganized action feedback data and execution structure

- Reorganized **action-feedback** around **request / timing / trigger** based matching

- Added `FActionFeedbackRequest`, `FActionFeedbackKey`, and `EActionFeedbackTiming`

- Split Trail / VFX / SFX data into separate execution paths

- Added **duplicate execution-key filtering**

- Organized request / execution log output

### 7. Unified Player / Enemy action feedback request flow

- Removed the old direct **`notify -> ActionComponent -> CAction`** request generation path

- Added `ActionFeedbackRequestProvider`

- Changed Player and Enemy to build `FActionFeedbackRequest` through their own runtime context

- Organized action feedback notifies to share one execution path for both Player and Enemy

- Connected **`ActionStart` / `ActionEnd`** feedback dispatch to base `CAction`

- Changed `GetCurAction()` to return `UCAction*`

### 8. Reorganized Enemy attack-end flow

- Added `CBTTask_EndAttack`

- Reduced `CAnimNotify_EndEnemyAttack` to an **end signal**

- Moved normal **enemy attack-end cleanup** into the **BT end-attack flow**

- Cached the **active action feedback key** on Enemy at attack start

- Organized the roles of `AttackIndex`, `AttackActionType`, and `LastAttackIndex`

- Kept **AI state transition cleanup** as a **safety-net** for unexpected state exit

### 9. Reflected assets and related data

- Added and connected VFX / SFX assets for **reaction-feedback**

- Added and connected Sword / Buff feedback assets for **action-feedback**

- Updated montage / behavior-tree / blackboard / camera-shake related assets

- Updated related issue / PR / branch documents


---

## Test Method

1. Verify that reaction-feedback reaches hit VFX / hit SFX / hit-stop correctly on both the Player-side and Enemy-side

2. Verify that camera shake runs correctly as local feedback on the Player-side

3. Verify that action-feedback runs correctly at `ActionStart`, `ActionEnd`, `TriggerOnce`, `TriggerWindowBegin`, and `TriggerWindowEnd`

4. Verify that `Trail On / Off` works correctly through `AnimNotifyState_ActionFeedback`

5. Verify that Sword SFX, Buff VFX, and Buff SFX run correctly through `AnimNotify_ActionFeedback`

6. Verify that both the Player-side and Enemy-side can use the same action-feedback notify path

7. Verify that the Enemy BT attack flow completes in the order:

   - `SelectAttackIndex`

   - `StartAttack`

   - `CommitAttackCooldown`

   - `WaitAttackEnd`

   - `EndAttack`

8. Verify that normal enemy attack-end cleanup is handled by `EndAttackTask`

9. Verify that state-transition cleanup still works as a safety-net when leaving the expected attack-end flow


---

## Related Issue / Branch

- Branch: `feature/combat-feedback`

- Related work:

  - `M03-05: Build the Combat Feedback Pipeline and Unify Player / Enemy Feedback Flow (#39)`


---

## Notes

- The core of this PR is not a single feedback feature, but the organization of the whole combat-feedback path from combat result generation to actual player-facing output

- Feedback is currently divided into **reaction-feedback**, **action-feedback**, and **player-feedback**

- While connecting **action-feedback** to Enemy, it was confirmed that the old request-generation path had been tightly coupled to `CAction`, so the flow is reorganized into an **interface-based request-provider path** that can be shared by both Player and Enemy

- **Enemy attack-end cleanup** is currently organized through two paths:

  - normal attack-end cleanup

  - safety-net cleanup for unexpected state exit

- This PR does not finish the larger action-structure rework, and later branches will continue with action orchestration, state/action ordering cleanup, and reducing Player-specific assumptions inside `CAction`


---
