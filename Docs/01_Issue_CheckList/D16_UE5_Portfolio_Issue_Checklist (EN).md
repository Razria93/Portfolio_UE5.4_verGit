# UE5 Portfolio Issue Checklist

## Title

**M05-01: Organize Action Orchestration Structure and strengthen AI Combo / Reaction integration**

### Date

- **Day 16**
  
- **Date : 2026.04.19**


---

### Goals

- Organize the flow from Player input to action execution around an **Orchestrator-driven structure**.

- Separate state transition and action execution so the flow clearly follows `confirm state transition -> execute action`.

- Reorganize the flow so that Player and AI reuse the same combat request path and combo chain execution path.

- Add a minimum safety structure so that combat flow remains stable after Reaction takeover during an active combo action.


---

### Branch
- `feature/action-orchestration`


---

### TODO List

#### 1. Organize Player Input Flow

- [x] Review direct Player input calls into `ActionComponent`

- [x] Review the flow for forwarding input requests to the Orchestrator

- [x] Organize the separation between global input blocks and action-specific execution conditions


#### 2. Build First-Pass Orchestrator Structure

- [x] Design Player action request handling flow

- [x] Organize where global rules are checked

- [x] Separate state transition decision from state transition confirmation

- [x] Finalize a generic rollback policy for execution failure


#### 3. Organize ActionComponent Responsibilities

- [x] Organize responsibilities for action storage, lookup, and current action management

- [x] Review action execution request APIs

- [x] Redefine the role of `ChangeActionMode`-style APIs


#### 4. Organize CAction Responsibilities

- [x] Review removing direct state changes from `CAction`

- [x] Separate action-specific execution conditions from execution logic

- [x] Verify that existing ComboAttack behavior remains intact


#### 5. Organize AI Combo Integration

- [x] Reorganize AI combat blackboard keys (`bCanCombatAction`, `bIsCombatAction`, `NextCombatActionTime`)

- [x] Apply `StartCombatAction` / `WaitEndCombatAction` structure

- [x] Connect AI combo chain follow-up through action event callbacks

- [x] Confirm that Player and AI now share the same combo chain execution path


#### 6. Strengthen Reaction Takeover Safety

- [x] Add active action abort on reaction entry

- [x] Reflect reaction state in combat availability calculation

- [x] Verify combat flow recovery after being hit during an active combo action


#### 7. Organize Minimum Validation Criteria

- [x] Scenario 1: Player input -> Orchestrator -> Action execution

- [x] Scenario 2: Confirm action execution after state transition

- [x] Scenario 3: Finalize a generic rollback policy for execution failure

- [x] Scenario 4: Verify Player ComboAttack behavior

- [x] Scenario 5: Verify that AI ComboAttack chains to the next combo step

- [x] Scenario 6: Verify combat flow recovery after reaction during an active combo action


---

### Notes

- This issue focuses on **separating responsibilities in the shared action execution flow**, rather than adding new actions.

- AI combo chain integration and minimum reaction takeover safety are included in this branch scope.

- Guard / Parry / Counter, advanced Reaction orchestration, and a higher-level coordination layer remain follow-up work for later branches.


---
