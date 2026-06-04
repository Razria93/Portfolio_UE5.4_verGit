# S12 UE5 Portfolio Action Orchestration Flow Decision

## 1. 목적

본 문서는 action orchestration refactor 이후 action request가 어떤 단계로 해석되고 실행 결정까지 도달하는지 정리하기 위한 문서임.

이번 브랜치의 핵심은 기존 action 실행 흐름을 `Orchestrator -> Component -> CAction` 구조로 재정렬하고, request 해석 / local rule / policy / orchestration result / component apply의 책임을 분리하는 것임.

## 2. 기존 시스템의 형태

기존 action 실행은 `ActionOrchestratorComponent`가 존재하더라도 실제 실행 판단과 상태 변경 책임이 `ActionComponent`와 `CAction` 내부에 강하게 섞여 있던 구조였음.

대표 흐름은 다음과 같았음.

```text
Request
-> ActionComponent Execute 계열 API
-> CAction DecideExecution
-> Component state 변경
-> Executor montage 실행
```

이 구조에서는 입력 request가 명시적인 실행 후보와 실행 컨텍스트로 분해되기보다, action component와 executor가 현재 상태를 직접 참조하면서 판단과 실행을 동시에 처리하는 형태에 가까웠음.

Combo attack, equip, unequip처럼 단순한 action에서는 큰 문제가 드러나지 않았으나, dodge / guard / parry / counter처럼 현재 action 또는 reaction과 충돌할 수 있는 action을 추가하기에는 판단 경로가 명확하지 않았음.

## 3. 기존 시스템의 문제 분석 및 한계

첫 번째 문제는 request 해석과 실행 적용이 분리되지 않았다는 점임.

입력 intent가 어떤 action data key로 해석되는지, 해당 key가 실제 action data와 executor로 resolve되는지, 그 executor가 현재 상태에서 어떤 transition을 원하는지, 그 transition이 body/runtime state에서 허용되는지의 단계가 명확히 나뉘지 않았음.

두 번째 문제는 action executor의 판단과 orchestration 판단이 같은 층위에서 섞여 있었다는 점임.

`CAction`은 자기 자신의 local rule을 판단할 수는 있지만, active action / active reaction / body state / cross-domain stop directive까지 총괄하기에는 책임 범위가 과도함.

세 번째 문제는 action orchestration level이 실제 경쟁 상태를 판단하기 위한 준비 구조를 갖지 못했다는 점임.

기존 구조에서는 `Start`, `Chain`, `Interrupt` 같은 결과가 나와도 이것이 local executor의 의도인지, body policy의 허용 결과인지, orchestration arbitration의 최종 판정인지 구분하기 어려웠음.

## 4. 리팩터링 방향 및 내용

리팩터링 이후 action request는 다음 단계로 정리됨.

```text
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

각 단계의 책임은 다음과 같음.

```text
Candidate
-> intent와 현재 상태를 기반으로 실행 후보 key를 구성함

ResolvedContext
-> candidate key를 ActionData와 ActionExecutor로 구체화함

LocalLevelQuery
-> incoming context, active context, execution state를 executor에게 질의 가능한 형태로 구성함

LocalLevelResult
-> incoming executor가 현재 조건에서 원하는 transition을 반환함

ResolvedPolicy
-> local decision이 현재 body/runtime state에서 최소 실행 권한을 갖는지 필터링함

OrchestrationLevelResult
-> 최종 action decision과 필요한 directive를 구성함
```

현재 구현에서 request별 candidate 해석은 단일 후보 중심으로 구성함.

```text
Equipment Toggle
-> current weapon state를 보고 Equip 또는 Unequip candidate로 해석함

ComboAttack
-> active combo attack이면 active index + 1을 candidate로 해석함
-> active combo attack이 아니면 index 0을 candidate로 해석함

Dodge
-> Dodge action index 0 candidate로 해석함
```

현재 `FActionCandidate`는 단일 key만 들고 있지만, 후속 작업에서 차선 후보나 fallback 후보를 확장할 수 있도록 별도 구조체로 유지함.

## 5. 이후 작업의 방향성

현재 orchestration level은 아직 완전한 arbitration 계층이 아님.

현재는 local decision과 resolved policy를 기반으로 최종 result를 구성하고, `Cancel` 계열에서 active reaction stop directive를 붙이는 수준임.

후속 작업에서는 다음 방향으로 확장해야 함.

```text
Action vs Action
-> chain, cancel, interrupt, priority 판단을 정리함

Action vs Reaction
-> dodge처럼 active reaction을 cancel하고 action으로 진입하는 흐름을 정리함

Reaction vs Action
-> hit reaction이나 dead reaction이 active action을 interrupt하는 흐름을 정리함

Common Arbitration
-> active execution과 incoming execution 사이의 개입 규칙을 공통 모델로 승격함
```

특히 dodge는 다음 단계의 검증 대상으로 적합함.

Dodge는 단순 movement command가 아니라 active reaction을 cancel하고 action execution으로 진입할 수 있는 action이므로, local decision / policy / stop directive / component apply 흐름을 모두 검증할 수 있음.

## 6. 결론

이번 refactor는 action execution을 바로 고도화하기보다, action request가 명확한 단계로 해석되고 실행 결과로 적용될 수 있는 구조적 기반을 만드는 작업임.

현재 구조는 아직 최종 arbitration 모델은 아니지만, action executor의 local rule과 orchestrator의 실행 조율 책임을 분리했다는 점에서 이후 dodge, guard, parry, counter 구현의 기반이 됨.
