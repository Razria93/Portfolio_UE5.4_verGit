# UE5 Portfolio Issue Checklist

## Title

**M05-03: Refactor Action Orchestration into Orchestrator / Component / CAction**

### Date

- **Day 18**
  
- **Date : 2026.05.10**


---

### Goals

- Reorganize action execution decisions into an `ActionOrchestrator` centered pipeline.

- Gradually move decision responsibility currently distributed across `ActionComponent::ExecuteAction()` and `CAction::DecideExecution()` into the orchestrator.

- Interpret action requests through `Intent -> Candidate -> Context -> Query -> Decision`.

- Reduce `ActionComponent` to active action state and decision application.

- Keep `CAction` responsible for montage lifecycle, notify windows, feedback, and executor-local rules.

- Preserve existing combo attack / equip / unequip gameplay results while aligning structure first.


---

### Branch
- `feature/action-orchestration-refactor`


---

### TODO List

#### 1. Analyze Current Action Flow

- [x] Document the current `ActionOrchestrator -> ActionComponent::ExecuteAction() -> CAction::DecideExecution()` flow from code

- [x] Separate decision responsibility and lifecycle responsibility of `CAction_ComboAttack`, `CAction_Equip`, and `CAction_Unequip`

- [x] Organize Player input / AI BT combat action request entry paths

- [x] Identify gameplay result points that must not change in existing chain / start / abort / complete flow

> Current action execution flow analysis is documented in `S12_UE5_Portfolio_Current_Action_Execution_Flow`.


#### 2. Define Action Orchestration Structures

- [ ] Define the scope of `FActionRequest`, `FActionCandidate`, `FActionContext`, `FActionExecutionPolicy`, `FActionOrchestrationQuery`, and `FActionOrchestrationResult`

- [ ] Reorganize request result type and reject reason for the action domain

- [ ] Define `Start / Chain / Queue / Interrupt / Cancel / Reject / Ignore` decision meanings for the action domain

- [ ] Decide whether existing `EActionExecutionDecision`, `FActionExecutionQuery`, and `FActionRequestResult` should be kept, replaced, or bridged


#### 3. Align ActionOrchestrator API Scaffold

- [ ] Organize `CanAcceptActionRequest()` as the common request gate

- [ ] Add `ResolveActionCandidates()` to interpret input intent into execution candidates based on current state

- [ ] Add `ResolveActionContexts()` to concretize candidates into action data / executor / action type / index context

- [ ] Add `ResolveActionPolicy()` to interpret body state / equipment state / action lock / stamina policy

- [ ] Add `BuildActionQueries()` and `OrchestrateActionQueries()` to compare active action and incoming context

- [ ] Align `DispatchActionDecision()` and `BuildRequestResult()` with the reaction orchestration style


#### 4. Reduce ActionComponent Responsibility

- [ ] Reorganize `ActionComponent` as decision application owner instead of action decision owner

- [ ] Add `ApplyActionDecision()` and branch Start / Chain / Queue / Interrupt / Cancel decisions into component operations

- [ ] Organize ownership of active action state, active action context, and queued action state

- [ ] Keep executor cache / lookup in the component while documenting definition data lookup as long-term separation scope

- [ ] Reduce existing `ExecuteAction()` into a compatibility wrapper or replace it with orchestrator-only entry


#### 5. Organize CAction Local Rule Hooks

- [ ] Review whether `CAction::DecideExecution()` can be reduced from final decision function into local rule hook

- [ ] Organize chain window / queue window / cancel window / interrupt window as executor-local state

- [ ] Design query API for whether active executor accepts incoming candidate

- [ ] Move or reorganize combo index / chain window judgment of `CAction_ComboAttack` into local rule hooks

- [ ] Simplify `CAction_Equip` / `CAction_Unequip` as equipment transition local rules


#### 6. Clarify Lifecycle Semantics

- [ ] Define action-domain meanings of `Start`, `Chain`, `Queue`, `Interrupt`, `Cancel`, `Complete`, `Abort`, and `Stop`

- [ ] Reorganize action finish / abort cleanup boundary between component and executor

- [ ] Separate cases that stop current action and move to incoming action from cases that only end current action

- [ ] Keep action feedback request centered on executor without conflicting with decision result

- [ ] Compare montage end / notify / action complete callback boundaries with reaction lifecycle


#### 7. Move Combo / Equipment Actions into the New Structure

- [ ] Connect `CAction_ComboAttack` start / chain / index resolve flow to orchestrator candidate / context structure

- [ ] Organize combo chain availability as active executor local rule query

- [ ] Do not add buffered input / queue behavior in the first implementation, but keep structural extension points

- [ ] Organize `CAction_Equip` / `CAction_Unequip` availability around policy / context resolve

- [ ] Verify that existing combo attack / equip / unequip gameplay results are preserved


#### 8. Organize Player / Enemy Request Paths

- [ ] Confirm that Player input only delivers action intent and does not directly select concrete action index

- [ ] Update Enemy BT combat action request to use the new orchestrator request flow

- [ ] Check whether `CBTTask_StartCombatAction` depends on direct ActionComponent execution and replace with orchestrator request if needed

- [ ] Align movement / equipment / combat action requests to use the same result model


#### 9. Verification Criteria

- [ ] Scenario 1: Player basic attack starts the same as before

- [ ] Scenario 2: Player combo chain continues with the same window / index rule as before

- [ ] Scenario 3: Equip / Unequip actions work with existing weapon state conditions

- [ ] Scenario 4: Enemy BT combat action request executes action the same as before

- [ ] Scenario 5: Complete / abort / montage end cleanup during action execution has no missing cleanup

- [ ] Scenario 6: Action feedback / notify / hit window behavior remains intact after refactor


---

### Notes

- This issue focuses on action orchestration responsibility redistribution, not new guard / parry / dodge features.

- Action can share an external structure with reaction, but internal decision should be defined around action transition orchestration.

- `Candidate` is not a confirmed executable value, but an execution candidate interpreted from intent and current state.

- `Context` is execution decision data that concretizes candidate into action data / executor / action type / index.

- `CAction` should provide executor-local windows and local rules instead of making the final decision.


---
