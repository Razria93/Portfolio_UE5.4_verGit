# UE5 Portfolio Issue Checklist

## Title

**M05-02: Reorganize Reaction Flow into ReactionOrchestrator / ReactionComponent / CReaction**

### Date

- **Day 17**
  
- **Date : 2026.04.30**


---

### Goals

- Clearly separate the reaction flow into a **ReactionOrchestrator -> ReactionComponent -> CReaction** structure.

- Unify the reaction request entry point through `UCReactionOrchestratorComponent`.

- Reduce `UCReactionComponent` to reaction runtime-state ownership and execution-state application.

- Remove the split reaction execution triggers currently handled by Player Tick and Enemy BT.

- Keep `CReaction` responsible for montage lifecycle, notify-based windows, and reaction-specific interrupt / cancel policies.


---

### Branch
- `feature/reaction-orchestration`


---

### TODO List

#### 1. Organize the Current Reaction Flow

- [x] Organized the execution path as `TakeDamage -> ReactionOrchestrator -> ReactionComponent -> CReaction`

- [x] Organized Player Tick / Enemy BT dependency points caused by pending reaction consumption

- [x] Organized the problem where `ReactionComponent` mixed pending storage, execution decision, and execution application responsibilities


#### 2. Finalize the ReactionOrchestrator Structure

- [x] Organized `UCReactionOrchestratorComponent::RequestReaction()` as the top-level entry point for reaction requests

- [x] Organized reaction type resolution from damage result into `Hit / Dead` inside the orchestrator

- [x] Organized reaction type / data / executor / policy / decision generation inside the orchestrator


#### 3. Redefine ReactionComponent Responsibilities

- [x] Reduced `ReactionComponent` to active reaction state management and decision application

- [x] Removed pending reaction context and reorganized the component around active reaction context

- [x] Organized action abort, movement lock, and execution-state transition responsibility around the component lifecycle


#### 4. Keep CReaction as the Execution Unit

- [x] Kept `CReaction` responsible for montage lifecycle / control window / feedback notify / local policy hooks

- [x] Organized the difference between executor runtime flags, `FReactionExecutionPolicy`, and executor hooks

- [x] Confirmed that `Hit / Dead` reaction types work inside the shared orchestration flow


#### 5. Organize Reaction Decision Policy

- [x] Finalized the first-pass decision scope as `Reject / Ignore / Start / Interrupt / Cancel`

- [x] Organized active reaction priority / interruption evaluation around `CanInterruptActiveReaction()`

- [x] Removed pending replacement and queue from the first-pass scope and separated them as follow-up expansion topics


#### 6. Organize TakeDamage / Feedback Integration

- [x] Connected `TakeDamageComponent` to request reaction through `ReactionOrchestrator` after accepted damage

- [x] Organized zero-damage / rejected-damage / dead-state conditions through take damage result and reaction type resolution

- [x] Separated `ReactionFeedback` by reaction execution timing and `DamageFeedback` by damage event / impact metadata


#### 7. Organize the Shared Player / Enemy Execution Path

- [x] Organized Player / Enemy to use the same reaction request path

- [x] Removed Player Tick-based pending consume flow

- [x] Reduced Enemy BT so it observes active reaction state instead of owning reaction execution


#### 8. Organize Dead Reaction and State Transition

- [x] Organized dead-state transition result to resolve into `EReactionType::Dead`

- [x] Organized dead-state follow-up action / reaction request handling through gate or priority policy

- [x] Connected `EReactionType::Dead` and assigned top-level priority / force interrupt policy


#### 9. Organize the First-Pass Scope

- [x] Limited the first pass to hit / dead reaction orchestration and shared execution-path unification

- [x] Separated Guard / Parry / Counter / Launch / KnockDown / queue expansion scope into follow-up work

- [x] Preserved existing combat / action / feedback stability while separating reaction feedback and damage feedback


#### 10. Organize Minimum Validation Criteria

- [x] Scenario 1: Player gets hit -> reaction starts through `ReactionOrchestrator`

- [x] Scenario 2: Enemy gets hit -> reaction starts without depending on the BT start task

- [x] Scenario 3: Verified action abort during reaction takeover while an active action is running

- [x] Scenario 4: Verified priority / interruption policy when a new hit arrives during an active reaction

- [x] Scenario 5: Verified that dead damage results connect correctly to dead reaction / dead-state flow

- [x] Scenario 6: Verified that movement / execution state is restored only when valid after reaction ends


---

### Notes

- This issue focuses on structural refactoring rather than adding new reaction features.

- `ReactionOrchestratorComponent` owns request routing and decision generation, while `ReactionComponent` owns runtime state and decision application.

- `CReaction` remains the execution unit that handles montage lifecycle and timing events.

- Pending / queue behavior was removed from the first-pass scope and should be revisited as a separate model only if needed.

- Hit feedback is handled by `DamageFeedback`, while reaction execution feedback is handled by `ReactionFeedback`.

- AI BT is now treated as an active reaction state observer, not the owner of reaction execution.


---
