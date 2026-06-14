# Action Orchestration Refactor의 Decision / Intervention 모델 전환

## 1. 제목

M05-S18: Action Orchestration Refactor의 Decision / Intervention 모델 전환

---
## 2. 목적

본 문서는 `feature/action-orchestration` 이후 사용하던 LocalLevel / ResolvedPolicy / OrchestrationLevel 기반 구조가 `feature/orchestration-refactor`에서 Decision / Relationship / ApplyMode / Intervention 중심 구조로 전환된 이유를 정리함.

`S12`, `S14`, `S15`는 1차 Action Orchestration 단계의 구조와 용어를 보존하는 문서임.

따라서 본 문서는 해당 문서들의 용어를 최신 용어로 덮어쓰지 않고, 이후 refactor 단계에서 어떤 개념이 유지되고 어떤 개념이 분리되었는지를 설명하는 전환 문서임.

---
## 3. 관련 브랜치

- `feature/orchestration-refactor`

---
## 4. 이전 모델의 형태

### Action Orchestration 중간 구조

`feature/action-orchestration` 이후 action request는 다음 중간 구조를 거쳐 실행 결과로 변환되었음.

```yaml
Request
-> Candidate
-> ResolvedContext
-> LocalLevelQuery
-> LocalLevelResult
-> ResolvedPolicy
-> OrchestrationLevelQuery
-> OrchestrationLevelResult
-> Component Apply
-> CAction lifecycle
```

이 구조는 action-only execution pipeline을 정리하는 데에는 유효했음.

하지만 active action / reaction이 서로 개입하는 구조로 확장되면서, local 판단과 orchestration 판단을 더 명확히 분리할 필요가 생겼음.

### Cross-Domain Stop 표현

active reaction을 action orchestration 결과로 멈춰야 하는 경우에는 stop directive 계열 구조가 필요했음.

```yaml
ActionOrchestrationLevelResult
-> StopDirective
-> ReactionComponent stop request
-> ActionComponent applies incoming action
```

이 표현은 active execution을 멈추는 최소 연결점이었지만, stop 요청의 source / target / reason / after-stop action을 함께 표현하기에는 부족했음.

---
## 5. 이전 모델의 문제 분석 및 한계

### LocalLevel / OrchestrationLevel 경계 불명확

LocalLevel은 executor가 원하는 transition을 반환하고, OrchestrationLevel은 이를 최종 result로 변환하는 구조였음.

하지만 실제 판단에는 다음 요소가 함께 필요했음.

```yaml
혼재된 판단 요소
- incoming executor가 원하는 transition
- 현재 execution state
- active action / reaction 존재 여부
- active execution을 멈출 수 있는지 여부
- stop 이후 incoming execution을 시작할 수 있는지 여부
```

이 상태에서는 executor rule, body state, active execution, intervention 가능성이 서로 얽혀 LocalLevel과 OrchestrationLevel의 책임 경계가 흔들릴 수 있음.

### ResolvedPolicy 의미 불명확

`ResolvedPolicy`는 이름상 policy처럼 보이지만, 실제로는 local decision을 bool 조건으로 다시 필터링하는 중간 구조에 가까웠음.

```yaml
ResolvedPolicy 역할
- bCanStart
- bCanChain
- bCanCancel
- bCanInterrupt
```

단순 action에서는 동작할 수 있지만, action / reaction cross-domain intervention에서는 어떤 판단 기준으로 can / cannot이 결정되었는지 추적하기 어려움.

### StopDirective 표현 범위 부족

공통 intervention에는 단순 stop 요청보다 더 많은 정보가 필요함.

```yaml
Intervention에 필요한 정보
- 누가 stop을 요청했는가
- 어떤 domain이 source인가
- 어떤 domain이 target인가
- stop reason은 무엇인가
- stop 이후 incoming execution을 시작할 것인가
```

따라서 StopDirective 계열 구조는 active execution stop만 표현할 수 있고, action / reaction 양쪽에서 같은 규칙으로 개입을 처리하기에는 범위가 좁았음.

---
## 6. 전환 방향 및 내용

### 메인 아이디어

전환 방향은 local / orchestration이라는 단계 이름보다 execution 판단에 필요한 책임을 기준으로 구조를 나누는 것임.

```yaml
Decision
- 요청이 실행 가능한지 판단

Relationship
- incoming execution과 active execution의 관계 판단

ApplyMode
- component가 실행 결과를 어떤 방식으로 적용할지 결정

InterventionDirective
- active execution을 중단해야 할 때 source / target / reason / after-stop action 표현
```

이 구조에서는 executor rule과 orchestrator apply 판단이 같은 query / participant / snapshot 기준 위에서 연결됨.

### Decision Query / Result 구조

기존 LocalLevelQuery는 공통 decision query로 재구성함.

```yaml
FExecutionDecisionQuery
- Snapshot     : 현재 body / execution 상태
- IncomingPart : 새로 들어온 execution participant
- ActivePart   : 현재 실행 중인 execution participant

FExecutionDecisionResult
- Decision     : 실행 판단 결과
- Relationship : active / incoming 관계
```

executor는 자신이 처리할 수 있는 rule을 decision result로 반환하고, orchestrator는 snapshot과 relationship을 기준으로 apply mode와 intervention 필요 여부를 확정함.

### Execution ApplyMode 구조

기존 OrchestrationLevelResult가 담당하던 최종 적용 판단은 apply mode 중심으로 정리함.

```yaml
EExecutionApplyMode
- Start     : active execution 없이 incoming execution 시작
- Reserve   : active execution과 sequential 관계로 예약
- Intervene : active execution 중단 후 incoming execution 적용
```

이로써 “실행 가능한가”와 “어떻게 적용할 것인가”가 분리됨.

### Intervention Directive 구조

기존 StopDirective 계열 구조는 공통 intervention directive로 정리함.

```yaml
FExecutionInterventionDirective
- StopSource
- SourceDomain
- TargetDomain
- StopReason
- AfterStopAction
```

이 구조를 통해 action component와 reaction component는 같은 형식의 directive를 소비할 수 있음.

---
## 7. 변경 의미

### 단순 Rename이 아닌 부분

다음 항목은 단순 이름 변경이 아니라 책임 분리임.

```yaml
LocalLevelResult
-> FExecutionDecisionResult
-> executor rule의 판단 결과로 정리됨

ResolvedPolicy
-> ExecutionSnapshot / Relationship / ApplyMode 판단으로 분산됨
-> 단일 bool filter 구조로 유지하지 않음

OrchestrationLevelResult
-> FActionExecutionResult / FReactionExecutionResult
-> component가 소비할 execution result로 정리됨

StopDirective
-> FExecutionInterventionDirective
-> source / target / stop reason / after-stop action까지 표현함
```

### 유지된 개념

다음 문제의식은 구조가 바뀐 이후에도 유지됨.

```yaml
1. Request를 곧바로 executor 실행으로 연결하지 않음
2. Candidate와 Context를 먼저 구성함
3. Executor rule과 component apply 책임을 분리함
4. Active execution과 incoming execution의 관계를 판단함
5. Stop 이후 후속 실행 여부를 명시해야 함
```

---
## 8. 관련 문서

### 선행 구조

- `S12`
- `S14`
- `S15`

### 같은 전환 범위

- `S16`

### 후속 구조

- `S19`

---
## 9. 결론

Action Orchestration Refactor의 핵심은 기존 LocalLevel / ResolvedPolicy / OrchestrationLevel 구조를 단순히 이름만 바꾸는 것이 아니라, execution 판단과 intervention 적용 책임을 분리하는 데 있음.

전환 후에는 executor가 decision result를 만들고, orchestrator가 relationship / apply mode / intervention directive를 확정하며, component가 execution result를 적용하는 구조로 정리됨.

이 문서는 1차 Action Orchestration 문서에서 사용하던 구형 용어와, 후속 orchestration refactor에서 사용되는 Decision / Intervention 모델 사이의 연결 지점을 설명하는 문서임.

---
