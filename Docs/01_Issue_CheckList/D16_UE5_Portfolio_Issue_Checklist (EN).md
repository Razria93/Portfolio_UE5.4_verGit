# UE5 Portfolio Issue Checklist

## Title

**M04-02: Organize Player Action Orchestration Structure**

### Date

- **Day 16**
  
- **Date : 2026.04.19**


---

### Goals

- Organize the flow from Player input to action execution around an **Orchestrator-driven structure**.

- Separate state transition and action execution so the flow clearly follows `confirm state transition -> execute action`.

- Prepare a foundation for later AI synchronization, Reaction orchestration, and special actions such as Guard / Parry / Counter.


---

### Branch
- `feature/action-orchestration`


---

### TODO List

#### 1. Organize Player Input Flow

- [ ] Review direct Player input calls into `ActionComponent`

- [ ] Review the flow for forwarding input requests to the Orchestrator

- [ ] Organize the separation between global input blocks and action-specific execution conditions


#### 2. Build First-Pass Orchestrator Structure

- [ ] Design Player action request handling flow

- [ ] Organize where global rules are checked

- [ ] Separate state transition decision from state transition confirmation

- [ ] Review rollback flow when action execution fails


#### 3. Organize ActionComponent Responsibilities

- [ ] Organize responsibilities for action storage, lookup, and current action management

- [ ] Review action execution request APIs

- [ ] Redefine the role of `ChangeActionMode`-style APIs


#### 4. Organize CAction Responsibilities

- [ ] Review removing direct state changes from `CAction`

- [ ] Separate action-specific execution conditions from execution logic

- [ ] Verify that existing ComboAttack / LightAttack behavior remains intact


#### 5. Organize Minimum Validation Criteria

- [ ] Scenario 1: Player input -> Orchestrator -> Action execution

- [ ] Scenario 2: Confirm action execution after state transition

- [ ] Scenario 3: Verify state rollback when action execution fails

- [ ] Scenario 4: Verify existing ComboAttack / LightAttack behavior


---

### Notes

- This issue focuses on **separating responsibilities in the Player action execution flow**, rather than adding new actions.

- AI, Reaction, Guard / Parry / Counter will be expanded in later branches based on this structure.

- Arbiter will not be implemented as a complete system in this branch; its required scope will be reviewed while organizing the Orchestrator decision flow.


---
