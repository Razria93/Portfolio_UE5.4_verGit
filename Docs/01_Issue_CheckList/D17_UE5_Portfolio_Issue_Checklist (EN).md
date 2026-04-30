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

- [ ] Organize the current `TakeDamage -> Reaction request -> execute` path

- [ ] Organize the current Player Tick / Enemy BT dependency points

- [ ] Organize the current `ReactionComponent` responsibilities and problem points


#### 2. Finalize the ReactionOrchestrator Structure

- [ ] Organize `ReactionOrchestrator` as the top-level entry point for reaction requests

- [ ] Organize the responsibility for converting damage results into reaction intents

- [ ] Organize the responsibility for generating reaction type / priority / decision inside the orchestrator


#### 3. Redefine ReactionComponent Responsibilities

- [ ] Reduce `ReactionComponent` to the role of reaction runtime-state manager

- [ ] Organize ownership of active / pending reaction contexts

- [ ] Organize responsibility for action abort, movement lock, and execution-state transition on reaction start / end


#### 4. Keep CReaction as the Execution Unit

- [ ] Keep `CReaction` responsible for montage lifecycle / notify window / interruption policy

- [ ] Organize how executor runtime flags connect to orchestration decisions

- [ ] Confirm that executor-specific policies such as `Hit / Dead` do not conflict with the shared flow


#### 5. Organize Reaction Decision Policy

- [ ] Finalize the first-pass scope for `Reject / Ignore / Start / Replace / Pending`

- [ ] Organize the priority / interruption evaluation rule when a new hit arrives during an active reaction

- [ ] Organize pending replacement and queue scope


#### 6. Organize TakeDamage / Feedback Integration

- [ ] Organize the connection boundary between `TakeDamageComponent` and reaction requests

- [ ] Organize zero-damage / rejected-damage / dead-state conditions

- [ ] Organize the relationship between reaction-feedback and actual reaction execution


#### 7. Organize the Shared Player / Enemy Execution Path

- [ ] Organize Player / Enemy to use the same reaction request path

- [ ] Organize the direction for removing Player Tick-based pending consume

- [ ] Organize the direction for reducing BT ownership over reaction execution


#### 8. Organize Dead Reaction and State Transition

- [ ] Organize the relationship between dead reaction and dead-state transition

- [ ] Organize cleanup / additional request handling rules in dead-state

- [ ] Organize whether `EReactionType::Dead` should be included


#### 9. Organize the First-Pass Scope

- [ ] Limit the first pass to hit / dead reaction orchestration and shared execution-path unification

- [ ] Separate Guard / Parry / Counter / Launch / KnockDown / queue expansion scope

- [ ] Organize the existing combat / action / feedback stability rules that must be preserved in this branch


#### 10. Organize Minimum Validation Criteria

- [ ] Scenario 1: Player gets hit -> reaction starts through `ReactionOrchestrator`

- [ ] Scenario 2: Enemy gets hit -> reaction starts without depending on the BT start task

- [ ] Scenario 3: Verify action abort during reaction takeover while an active action is running

- [ ] Scenario 4: Verify priority / interruption policy when a new hit arrives during an active reaction

- [ ] Scenario 5: Verify that dead damage results connect correctly to dead reaction / dead-state flow

- [ ] Scenario 6: Verify that movement / execution state is restored only when valid after reaction ends


---

### Notes

- This issue focuses on structural refactoring rather than adding new reaction features.

- `ReactionOrchestratorComponent` owns request routing and decision generation, while `ReactionComponent` owns runtime state and decision application.

- `CReaction` remains the execution unit that handles montage lifecycle and timing events.

- If queue behavior is not needed in the first pass, keep only the decision type and defer queue storage / processing to follow-up work.


---
