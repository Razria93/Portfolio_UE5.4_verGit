# Build Reaction Orchestration and Reorganize Reaction Feedback / Damage Impact Flow

## Title

`✨ feat: build reaction orchestration and reorganize reaction feedback / damage impact flow (#46)`

## Summary

- This PR reorganizes the reaction execution flow after `TakeDamage` around `ReactionOrchestrator`, and removes the old pending-based reaction execution flow so Player and AI share the same reaction execution pipeline.

- The main direction is as follows.

	- explicitly build the `TakeDamage -> ReactionOrchestrator -> ReactionComponent -> CReaction` flow.

	- keep `ReactionComponent` focused on active reaction state and executor control.

	- keep `CReaction` responsible for montage lifecycle, stop / finish reason, and reaction feedback requests.

	- reduce BT so it observes active reaction state instead of executing reactions.

	- separate hit feedback from reaction execution feedback, and pass damage impact position metadata into damage feedback.

- This PR also adds / updates reaction feedback notifies, reaction control notifies, damage feedback, feedback matching, action notify base separation, related assets, and design documents.


---

## Completed Items

### 1. Added reaction orchestration request / result structures

- Added `CReactionOrchestrationStructure`

- Added reaction request / result / decision / policy / query structures

- Represented reaction execution requests after `TakeDamage` through `FDamageReactionRequest`

- Added `EReactionRequestResultType`, `EReactionRequestRejectReason`, `EReactionOrchestrationDecision`, and `FReactionExecutionPolicy` to separate request results from orchestration decisions

### 2. Added ReactionOrchestratorComponent

- Added `UCReactionOrchestratorComponent`

- Set `RequestReaction()` as the external entry point for reaction execution requests

- `ResolveReactionContext()` resolves damage result into reaction type / reaction data / reaction executor

- `ResolveReactionPolicy()` interprets priority / interrupt authority / force interrupt policy at request time

- `OrchestrateQuery()` evaluates competition between active reaction and incoming reaction

- `DispatchReactionDecision()` applies the final decision to `ReactionComponent`

### 3. Reorganized ReactionComponent responsibility

- Removed the old pending reaction consumption flow

- Reorganized `ReactionComponent` around active reaction state and executor control

- Added `ApplyReactionDecision()` to apply orchestrator decisions

- Added `TryStartReaction`, `TryInterruptReaction`, and `TryCancelReaction` flows

- Reorganized active reaction lifecycle through `StartActiveReactionInternal`, `StopActiveReactionInternal`, and `EndActiveReactionInternal`

- Strengthened fallback cleanup for stale active reaction state

### 4. Reorganized CReaction lifecycle

- Reorganized `CReaction` as the actual reaction executor responsible for montage execution, control windows, feedback notifies, and stop / finish handling

- `Stop()` now receives external stop requests and confirms finish reason from stop reason

- Split `FinishCompleted`, `FinishInterrupted`, `FinishCancelled`, and `FinishAborted`

- Kept `MontageEnd` focused on normal completion detection, while system stop is handled explicitly in `Stop()`

- Kept `WantToInterrupt`, `WantToCancel`, `AllowInterruptionBy`, and `AllowCancelBy` as local policy hooks

### 5. Removed AI reaction pending consumption and reduced BT to observation

- Removed pending reaction consumption from Player tick

- Removed pending reaction consumption from BT

- Reduced `CBTTask_StartReaction` so it is no longer the owner of reaction execution

- Reorganized `CBTTask_WaitEndReaction` as an observer of active reaction state

- Reorganized BT service / blackboard to observe active reaction state instead of pending requests

### 6. Added Reaction Feedback structure

- Added `CReactionFeedbackComponent`

- Added `CReactionFeedbackStructure` to separate reaction feedback key / timing / request / execution key structures

- Built feedback matching based on reaction type, damage spec key, timing, and trigger key

- Reorganized wildcard matching and duplicate execution key filtering

- Let `CReaction` create reaction feedback requests from its active context

### 7. Added Reaction Feedback Notify / Reaction Control Notify

- Renamed `CAnimNotifyState_Reaction` to `CAnimNotifyState_ReactionControl`

- Separated reaction control windows from reaction feedback windows

- Added `CAnimNotify_ReactionFeedback` for point reaction feedback

- Added `CAnimNotifyState_ReactionFeedback` for window begin / end reaction feedback

- Routed reaction notifies through `ReactionComponent` to the active reaction executor

### 8. Split DamageFeedbackComponent

- Split and clarified hit feedback behavior into `CDamageFeedbackComponent`

- Processed hit VFX / hit SFX / hit stop / camera shake request from `TakeDamagePacket`

- Separated reaction feedback from damage feedback

- Kept `DamageFeedback` based on damage event and impact metadata, while `ReactionFeedback` is based on reaction execution timing

### 9. Added DamageImpactInfo

- Added `FDamageImpactInfo` and `EDamageImpactInfoSource`

- Passed damage impact info through `FHitContext`, apply damage payload / context, default damage event, and take damage payload / context

- Built impact point in `ACWeaponActor` using `SweepResult` first and `GetClosestPointOnCollision()` as fallback

- Let `CDamageFeedbackComponent` resolve hit feedback location / rotation from `TakeDamagePacket.Context.DamageImpactInfo`

### 10. Reorganized Action Notify base

- Moved action-only notify trigger filters into `CAnimNotify_ActionBase` and `CAnimNotifyState_ActionBase`

- Kept shared notify bases `CAnimNotify` and `CAnimNotifyState` free from action trigger fields

- Prevented reaction notifies from carrying unnecessary action trigger fields

### 11. Updated assets and blueprints

- Updated Player / Enemy blueprints with reaction orchestrator, reaction feedback, and damage feedback setup

- Updated hit reaction montages and related skeleton / blackboard assets

- Added / updated Slash hit VFX and HealPositive feedback assets

- Updated TestRoom and related unit test assets

### 12. Added design and issue documentation

- `D17`: Reaction Orchestration issue checklist

- `S06`: Action / Reaction Orchestration comparison

- `S07`: Orchestrator / Data / Component responsibility separation

- `S08`: Execution Orchestration API model

- `S09`: Reaction Pending model

- `S10`: Reaction Execution Policy model

- `S11`: Weapon Trail Trace model

- `S12`: Reaction Lifecycle model

- `S13`: Combat Feedback model

- `S14`: AI Reaction Observation model

- `S15`: Action Orchestration Refactor model


---

## Test Method

1. Verify that Player hit reaction executes through `TakeDamage -> ReactionOrchestrator -> ReactionComponent -> CReaction`

2. Verify that Enemy hit reaction executes without BT pending consumption

3. Verify that higher-priority reaction or dead reaction can interrupt the active reaction correctly

4. Verify that active reaction state is cleaned up correctly after completed / interrupted / cancelled / aborted flows

5. Verify that movement / action state does not become inconsistent during reaction

6. Verify that BT observes active reaction state instead of executing reaction directly

7. Verify that reaction feedback point notify and window notify execute through the active reaction executor

8. Verify that damage feedback plays hit VFX / hit SFX from `DamageImpactInfo` based position

9. Verify that closest point fallback produces a natural hit VFX position when `bFromSweep == false`

10. Verify that action notify and reaction notify no longer share unnecessary trigger fields

11. Verify that Player / Enemy blueprints and montage assets load correctly and visual feedback plays correctly

12. Verify that `PortfolioEditor Win64 Development` build succeeds


---

## Verification

- `git diff --check` passed

- `PortfolioEditor Win64 Development` build passed

- Verified Player / Enemy hit visual feedback works correctly

- Verified damage impact based hit VFX position works correctly


---

## Related Issue / Branch

- Branch: `feature/reaction-orchestration`

- Related work:

  - `D17: Reaction Orchestration issue checklist`

  - `S06 ~ S15: Reaction Orchestration and follow-up architecture documents`

- Follow-up candidate:

  - `feature/action-orchestration-refactor`

  - refactor action orchestration using the responsibility separation principles established by reaction orchestration


---
