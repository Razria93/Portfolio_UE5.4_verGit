# UE5 Portfolio – Issue Checklist

## 제목

**M05-03: Action / Reaction Orchestration의 Decision / ApplyMode / Intervention 흐름 리팩터링**

### 날짜

- **Day 18**

- **Date : 2026.05.10**

---
### 브랜치

- feature/orchestration-refactor

---
### 목표

- action / reaction 실행 판단을 공통 execution orchestration 흐름으로 정리함.

- request에서 candidate / context / decision query를 구성하고, executor decision을 공통 result로 수집함.

- `Decision -> Relationship -> ApplyMode`를 분리하여 실행 가능 여부, active execution과의 관계, 실제 적용 방식을 명확히 구분함.

- exclusive relationship에서는 intervention query와 intervention directive를 구성하여 stop 대상과 이후 실행 방식을 명시함.

- component는 orchestrator가 만든 execution result를 소비하여 start / reserve / intervene 같은 결정 행위를 적용하도록 정리함.

- action과 reaction이 서로를 중단하거나 교체할 수 있는 cross-domain intervention 기반을 마련함.

- intervention 판단에서 stop source, default policy, notify window gate의 책임을 분리함.

---
### TODO 리스트

#### 1. 기존 Action / Reaction 흐름 분석

- [x] `ActionComponent` / `CAction`에 분산된 action execution decision 책임 정리

- [x] damage result 해석 / reaction 선택 / active execution intervention 책임 정리

- [x] action / reaction의 montage 기반 executor lifecycle 공통 기준 정리

- [x] Player input / Enemy BT / TakeDamage request의 orchestration 진입 경로 정리

#### 2. 공통 Execution Decision 구조 정리

- [x] 현재 body / execution 상태를 담는 공용 구조체 구현 
  (`FExecutionSnapshot`)
	- `ExecutionState`: 현재 실행 상태
	- `bIsDead`: 사망 상태 여부

- [x] incoming execution과 active execution에 사용할 action / reaction 공용 구조체 구현 
  (`FExecutionParticipant`)
	- `bIsValid`: participant 유효 여부
	- `ParticipantDomain`: action / reaction domain 구분
	- `ActionContext`: action execution context
	- `ReactionContext`: reaction execution context

- [x] snapshot / incoming participant / active participant 기반 decision query 구성 
  (`FExecutionDecisionQuery`)
	- `Snapshot`: 현재 body / execution 상태
	- `IncomingPart`: 새로 들어온 execution participant
	- `ActivePart`: 현재 실행 중인 execution participant

- [x] decision 결과를 담는 공용 구조체 구성
  (`FExecutionDecisionResult`)
	- `Decision`: 실행 가능 여부
	- `Relationship`: incoming execution과 active execution의 관계

#### 3. Decision 분리

- [x] 실행 가능 여부 판단값 정리 
  (`EExecutionDecision`)
	- `Accept`: 실행 가능
	- `Ignore`: 유효하지만 현재 상태에서 실행하지 않음
	- `Reject`: 요청 또는 실행 조건이 유효하지 않음

#### 4. Relationship 분리

- [x] execution relationship 분리 
  (`EExecutionRelationship`)
	- `Independent`: active execution과 관계없이 독립적으로 시작 가능한 실행
	- `Sequential`: active execution과 연속되는 실행
	- `Exclusive`: active execution과 공존할 수 없어 intervention 판단이 필요한 실행

#### 5. ApplyMode 분리

- [x] execution 적용 방식 분리 
  (`EExecutionApplyMode`)
	- `Start`: active execution이 없을 때 incoming execution을 즉시 시작
	- `Reserve`: active execution과 연속되는 incoming execution을 예약
	- `Intervene`: active execution을 중단한 뒤 incoming execution을 적용

#### 6. Action Orchestrator Pipeline 정리

- [x] 1. movement / equipment / combat action request 진입점 정리
  (`UCActionOrchestratorComponent`)

- [x] 2. action intent -> candidate 변환
  (`FActionCandidate`)

- [x] 3. action candidate -> execution context 구체화
  (`FActionExecutionContext`)

- [x] 4. incoming action / active execution 기반 decision query 구성
  (`FExecutionDecisionQuery`)

- [x] 5. action executor decision 수집
  (`ResolveExecutionDecision()`)

- [x] 6. relationship 기반 apply mode 결정
  (`EExecutionApplyMode`)

- [x] 7. action execution result 구성 및 dispatch
  (`FActionExecutionResult`)

#### 7. Reaction Orchestrator Pipeline 정리

- [x] 1. damage reaction request 진입점 정리
  (`UCReactionOrchestratorComponent`)

- [x] 2. damage result 기반 `Hit / Dead` reaction candidate resolve

- [x] 3. reaction candidate -> execution context 구체화
  (`FReactionExecutionContext`)

- [x] 4. incoming reaction / active execution 기반 decision query 구성
  (`FExecutionDecisionQuery`)

- [x] 5. reaction executor decision 수집
  (`ResolveExecutionDecision()`)

- [x] 6. relationship 기반 apply mode 결정
  (`EExecutionApplyMode`)

- [x] 7. reaction execution result 구성 및 dispatch
  (`FReactionExecutionResult`)

#### 8. Intervention 판단 및 Directive 구성

- [x] exclusive relationship에서만 intervention 판단 진입

- [x] intervention 판단 입력값 구성 
  (`FExecutionInterventionQuery`)
	- `IncomingPart`: active execution에 개입하려는 execution participant
	- `ActivePart`: 현재 실행 중이며 중단 대상이 될 수 있는 execution participant
	- `StopReason`: active execution 중단 사유

- [x] intervention 양측 판단 API 분리
	- `WantIntervention()`: incoming execution이 active execution을 중단하려는지 판단
	- `AllowIntervention()`: active execution이 incoming execution에 의해 중단될 수 있는지 판단

- [x] intervention 실행 지시 데이터 구성 
  (`FExecutionInterventionDirective`)
	- `StopSource`: 중단 요청의 출처
	- `SourceDomain`: 중단을 요청한 execution domain
	- `TargetDomain`: 중단 대상 execution domain
	- `StopReason`: 중단 사유
	- `AfterStopAction`: 중단 이후 처리 방식

- [x] 강제 intervention 처리 경로 정리 (`DeadReaction`)

#### 9. Stop Source / Intervention Policy / Gate 정리

- [x] stop reason 의미 재정리
	- 기존: `Cancel / Interrupt` 중심
	- 변경: active execution의 외부 중단 결과를 `Interrupted`로 통합
	- `Intervention`은 중단 가능성 판단 / directive 구성 모델로 사용
	- `Intervened`는 request result를 의미함

- [x] stop source 분리 
  (`EExecutionStopSource`)
	- `ActionOrchestration`: action orchestration에 의한 중단 요청
	- `ReactionOrchestration`: reaction orchestration에 의한 중단 요청
	- `System`: death / cleanup / recovery 같은 시스템 중단
	- `External`: cutscene / debug / scripted event 같은 외부 중단

- [x] intervention policy / gate 분리
	- `Policy`: 누구를 중단하려는가 / 누구에게 중단되어도 되는가
	- `Gate`: 해당 policy가 언제 활성화되는가

- [x] default policy 위치 정리
	- 상시 정책은 `Data`의 want / allow rule 또는 executor override에 배치

- [x] NotifyState 역할 축소
	- `Runtime Window Gate`만 담당하는 역할로 제한

#### 10. Component Apply 책임 정리

- [x] action execution result 적용 책임 정리 (`ActionComponent`)

- [x] reaction execution result 적용 책임 정리 (`ReactionComponent`)

- [x] intervention directive 소비 및 active execution stop 요청 처리

- [x] component operation 분기
	- `Start`: incoming execution 즉시 시작
	- `Reserve`: sequential execution 예약
	- `Intervene`: active execution 중단 후 incoming execution 적용

- [x] active execution context / executor cache 소유 책임 유지

#### 11. Executor Decision / Lifecycle 책임 정리

- [x] executor decision hook 정리
	- `ResolveExecutionDecision()`: executor가 자기 실행 가능 조건만 판단

- [x] executor lifecycle 책임 정리
	- montage play / stop / complete
	- montage end callback
	- runtime cleanup

- [x] notify / feedback 처리 책임 정리
	- notify command 처리
	- feedback timing request 구성

- [x] incoming / active participant 기반 executor 판단

- [x] executor runtime intervention window 관리

#### 12. Sequential Execution / Combo Reserve Consume 정리

- [x] ComboAttack을 sequential relationship 대표 사례로 정리

- [x] sequential execution 처리 단계 분리
	- `Reserve`: chain input 시 다음 action data 예약
	- `Consume`: notify timing에서 reserved data 소비

- [x] combo notify command 정리
	- `ReserveChainWindow`: chain input을 받을 수 있는 구간
	- `ConsumeChain`: reserved data를 실제 실행으로 전환하는 지점

- [x] Enemy combo chain request 재발행 경로 정리

#### 13. Orchestrator 공통화 검토 범위

- [ ] 공통 알고리즘 추출 범위 검토
	- `BuildSnapshot`
	- `BuildActiveExecutionParticipant`
	- `BuildInterventionQuery`
	- `BuildInterventionDirective`

- [ ] 도메인별 유지 책임 분리
	- `Request Parse`
	- `Context Resolve`
	- `Result Dispatch`

- [ ] 상속 전 helper / utility 분리 우선 적용

#### 14. 검증 기준

- [x] Scenario 1: Player basic attack 시작 검증

- [x] Scenario 2: Player combo reserve / consume 검증

- [x] Scenario 3: Enemy combo request 경로 검증

- [x] Scenario 4: Equip / Unequip weapon state 조건 검증

- [x] Scenario 5: HitReaction -> active action intervention 검증

- [x] Scenario 6: DodgeAction -> active reaction intervention 검증

- [x] Scenario 7: health commit 이후 DeadReaction request / intervention 검증

#### 15. 후속 보완 범위

- [ ] notify window 기반 상시 정책 문제 재현 및 개선 기준 정리

- [ ] intervention directive source 추적 로그 보강
	- `FExecutionInterventionDirective`에는 `StopSource / SourceDomain / TargetDomain / StopReason / AfterStopAction` 구조가 있음
	- 실제 intervention 발생 시 directive 정보가 로그로 한 번에 확인되도록 orchestrator 생성 시점과 component 소비 시점 로그 보강 필요

---
### 비고

- 이 이슈는 action / reaction orchestration 공통화 작업임을 명시함.

- 핵심은 실행 가능 여부를 판단한 뒤, active execution과의 관계를 해석하고, 필요한 경우 intervention directive를 구성한 다음, component가 결정 행위를 소비하도록 만드는 것임.

- `Decision`은 실행 가능 여부, `Relationship`은 incoming과 active의 관계, `ApplyMode`는 component가 실제로 수행할 적용 방식을 의미함.

- `InterventionDirective`는 "누가 누구를 왜 멈추고, 이후 무엇을 할 것인가"를 명시하는 실행 지시 데이터임.

- 용어 기준은 다음과 같이 정리함.
	- `Intervention`: active execution과 incoming execution이 충돌할 때 중단 가능성을 판단하고 directive를 구성하는 모델
	- `Intervene`: active execution을 중단한 뒤 incoming execution을 적용하는 apply mode
	- `Interrupt / Interrupted`: 실제 active execution이 외부 요청으로 중단되는 처리 결과
	- `Intervened`: intervention directive가 생성되어 request가 받아들여진 result

- Combo는 sequential execution의 대표 사례라는 기준으로 정리함.

- 현재 구조에서는 active execution의 외부 중단 결과를 `Interrupted`로 통합함.

- Want / Allow는 유지하되, policy와 timing gate를 분리해야 함. NotifyState는 특정 구간에서 runtime window를 열고 닫는 gate라는 의미로 사용함.

- Action / Reaction Orchestrator 중복은 현재 구조 안정화를 위해 유지하되, 이후 공통 알고리즘을 helper / utility로 분리하는 방향을 검토함.

- `Intervention` 계열 명칭은 현재 구조 설명에는 남기되, 혼선 가능성이 있으므로 후속 리팩터링에서 `InterruptQuery` / `InterruptDirective` / `WantInterrupt` / `AllowInterrupt` 계열로 축소할지 검토함.

- Guard / Parry / Counter와 완성형 Combat Resolution은 이 구조 위에서 확장할 후속 범위로 남김.

---
