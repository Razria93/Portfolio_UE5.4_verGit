# UE5 Portfolio Issue Checklist

## 제목

**M05-03: Action Orchestration을 Orchestrator / Component / CAction 구조로 리팩터링함**

### 날짜

- **Day 18**
  
- **Date : 2026.05.10**


---

### 목표

- action execution decision을 `ActionOrchestrator` 중심의 pipeline으로 재정리함.

- `ActionComponent::ExecuteAction()`과 `CAction::DecideExecution()`에 분산된 판단 책임을 단계적으로 orchestrator로 이동함.

- action request를 `Intent -> Candidate -> Context -> Query -> Decision` 흐름으로 해석함.

- `ActionComponent`는 active action state와 decision application 중심으로 축소함.

- `CAction`은 montage lifecycle, notify window, feedback, executor-local rule을 담당하도록 정리함.

- 기존 combo attack / equip / unequip 동작 결과를 유지하면서 구조를 먼저 정렬함.


---

### 브랜치
- `feature/action-orchestration-refactor`


---

### TODO List

#### 1. 현재 Action 흐름 분석

- [ ] `ActionOrchestrator -> ActionComponent::ExecuteAction() -> CAction::DecideExecution()` 흐름을 코드 기준으로 정리함

- [ ] `CAction_ComboAttack`, `CAction_Equip`, `CAction_Unequip`의 decision 책임과 lifecycle 책임을 분리해 기록함

- [ ] Player input / AI BT combat action request가 action request로 진입하는 경로를 정리함

- [ ] 기존 chain / start / abort / complete 흐름에서 gameplay 결과가 바뀌면 안 되는 지점을 정리함


#### 2. Action Orchestration Structure 정의

- [ ] `FActionRequest`, `FActionCandidate`, `FActionContext`, `FActionExecutionPolicy`, `FActionOrchestrationQuery`, `FActionOrchestrationResult` 범위를 정의함

- [ ] request result type과 reject reason을 현재 action domain 기준으로 재정리함

- [ ] `Start / Chain / Queue / Interrupt / Cancel / Reject / Ignore` decision 의미를 action domain 기준으로 정의함

- [ ] 기존 `EActionExecutionDecision`, `FActionExecutionQuery`, `FActionRequestResult`와의 유지 / 교체 / 호환 범위를 결정함


#### 3. ActionOrchestrator API Scaffold 정렬

- [ ] `CanAcceptActionRequest()`를 common request gate로 정리함

- [ ] `ResolveActionCandidates()`에서 input intent를 현재 상태 기준의 실행 후보로 해석하도록 구조를 추가함

- [ ] `ResolveActionContexts()`에서 candidate를 action data / executor / action type / index context로 구체화하도록 구조를 추가함

- [ ] `ResolveActionPolicy()`에서 body state / equipment state / action lock / stamina 기준 정책을 해석하도록 구조를 추가함

- [ ] `BuildActionQueries()`와 `OrchestrateActionQueries()`를 통해 active action과 incoming context를 비교하도록 구조를 추가함

- [ ] `DispatchActionDecision()`과 `BuildRequestResult()`를 reaction orchestration과 유사한 형식으로 정리함


#### 4. ActionComponent 책임 축소

- [ ] `ActionComponent`가 action decision owner가 아니라 decision application owner가 되도록 API를 정리함

- [ ] `ApplyActionDecision()`을 추가하고 Start / Chain / Queue / Interrupt / Cancel decision을 component operation으로 분기함

- [ ] active action state, active action context, queued action state의 소유 범위를 정리함

- [ ] executor cache / lookup 책임은 component에 유지하되 definition data 조회 책임은 장기 분리 대상으로 기록함

- [ ] 기존 `ExecuteAction()`은 호환 wrapper로 축소하거나 orchestrator 전용 경로로 대체함


#### 5. CAction Local Rule Hook 정리

- [ ] `CAction::DecideExecution()`을 final decision 함수가 아니라 local rule hook으로 축소할 수 있는지 정리함

- [ ] chain window / queue window / cancel window / interrupt window를 executor-local 상태로 정리함

- [ ] active executor가 incoming candidate를 받을 수 있는지 답하는 query API를 설계함

- [ ] `CAction_ComboAttack`의 combo index / chain window 판단을 local rule hook으로 이동 또는 정리함

- [ ] `CAction_Equip` / `CAction_Unequip`은 equipment transition local rule로 단순화함


#### 6. Lifecycle 의미 정리

- [ ] `Start`, `Chain`, `Queue`, `Interrupt`, `Cancel`, `Complete`, `Abort`, `Stop`의 action domain 의미를 정리함

- [ ] action finish / abort cleanup 경계를 component와 executor 사이에서 다시 정리함

- [ ] current action을 중단하고 incoming action으로 넘어가는 경우와 current action만 종료하는 경우를 분리함

- [ ] action feedback request 위치를 executor 중심으로 유지하되 decision result와 충돌하지 않도록 정리함

- [ ] montage end / notify / action complete callback의 책임 경계를 reaction lifecycle과 비교하여 정리함


#### 7. Combo / Equipment Action 이관

- [ ] `CAction_ComboAttack`의 start / chain / index resolve 흐름을 orchestrator candidate / context 구조와 연결함

- [ ] combo chain 가능 여부를 active executor local rule query로 정리함

- [ ] buffered input / queue는 1차 구현에서 실제 기능으로 추가하지 않고 구조적 확장 지점만 확보함

- [ ] `CAction_Equip` / `CAction_Unequip`의 실행 가능 여부를 policy / context resolve 기준으로 정리함

- [ ] 기존 combo attack / equip / unequip gameplay 결과가 유지되는지 확인함


#### 8. Player / Enemy Request 경로 정리

- [ ] Player input이 action intent까지만 전달하고 구체 action index를 직접 선택하지 않도록 경계를 확인함

- [ ] Enemy BT combat action request가 새 orchestrator request 흐름을 사용하도록 정리함

- [ ] `CBTTask_StartCombatAction`이 ActionComponent 직접 실행에 의존하는지 확인하고 필요 시 orchestrator request로 교체함

- [ ] movement / equipment / combat action request 경로가 같은 result model을 사용하도록 정리함


#### 9. 검증 기준

- [ ] Scenario 1: Player basic attack이 기존과 동일하게 시작되는지 확인함

- [ ] Scenario 2: Player combo chain이 기존과 동일한 window / index 기준으로 이어지는지 확인함

- [ ] Scenario 3: Equip / Unequip action이 기존 weapon state 조건에서 정상 동작하는지 확인함

- [ ] Scenario 4: Enemy BT combat action request가 기존과 동일하게 action을 실행하는지 확인함

- [ ] Scenario 5: action 실행 중 complete / abort / montage end cleanup이 기존보다 누락되지 않는지 확인함

- [ ] Scenario 6: action feedback / notify / hit window 동작이 리팩터링 이후에도 유지되는지 확인함


---

### Notes

- 이 이슈는 신규 guard / parry / dodge 기능 추가가 아니라 action orchestration 책임 재분배에 집중함.

- action은 reaction과 외부 구조를 유사하게 가져가되, 내부 decision은 action transition orchestration 기준으로 정의함.

- `Candidate`는 실행 가능 확정값이 아니라 intent를 현재 상태 기준으로 해석한 실행 후보임.

- `Context`는 candidate를 action data / executor / action type / index로 구체화한 실행 판단 자료임.

- `CAction`은 final decision을 내리지 않고 executor-local window와 local rule을 제공하는 방향으로 축소함.


---
