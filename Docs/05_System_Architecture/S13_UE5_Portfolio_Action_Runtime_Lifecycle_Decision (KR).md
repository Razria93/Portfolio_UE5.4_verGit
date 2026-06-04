# S13 UE5 Portfolio Action Runtime Lifecycle Decision

## 1. 목적

본 문서는 action orchestration refactor 이후 `ActionComponent`와 `CAction`의 실행 책임이 어떻게 나뉘었는지 정리하기 위한 문서임.

핵심은 orchestrator가 실행 결정을 만들고, component가 active runtime state와 side effect를 적용하며, `CAction`이 montage / notify / feedback lifecycle을 수행하도록 책임을 분리하는 것임.

## 2. 기존 시스템의 형태

기존 구조에서는 action data가 executor 초기화 시점에 주입되고, executor가 특정 action data set을 내부에 보유하는 형태에 가까웠음.

또한 anim notify가 action-specific API를 직접 호출하거나, component가 구체 action subclass의 동작을 일부 알고 있는 흐름이 섞여 있었음.

대표적인 문제 흐름은 다음과 같았음.

```text
ActionComponent
-> active action 실행
-> CAction 내부 데이터 사용
-> notify별 API 직접 호출
-> component와 executor 상태가 일부 중복 관리됨
```

이 구조에서는 동일한 executor class를 여러 action data에서 재사용하기 어렵고, montage notify command가 늘어날수록 component API가 action 종류별로 증가할 가능성이 있었음.

## 3. 기존 시스템의 문제 분석 및 한계

첫 번째 한계는 executor가 definition data를 오래 보유하는 구조였다는 점임.

`ComboAttack` 같은 executor는 하나의 실행기일 뿐이며, 실제 실행할 combo index와 montage data는 request가 resolve된 결과로 들어와야 함.

따라서 executor가 초기화 시점에 action data 목록을 주입받고 그것을 기준으로 실행하는 구조는 executor 재사용성과 데이터 분리를 약화시킴.

두 번째 한계는 notify command 흐름이 분산되어 있었다는 점임.

Chain window, collision window, hit context처럼 일정 구간 동안 열리고 닫혀야 하는 기능은 순간 notify보다 notify state가 적합함.

세 번째 한계는 component와 executor의 active state 갱신 책임이 불명확했다는 점임.

특히 combo chain처럼 executor 내부에서 다음 action data로 넘어가는 경우, executor의 active data와 component의 active data가 어긋나지 않도록 component에 chain commit을 명확히 알려야 함.

## 4. 리팩터링 방향 및 내용

리팩터링 이후 `ActionComponent`는 action runtime state의 소유자가 됨.

주요 책임은 다음과 같음.

```text
ActionComponent
-> ActionDataMap과 ActionExecutorMap을 관리함
-> active action context를 관리함
-> orchestration result를 실제 실행 API로 적용함
-> action state enter/exit와 movement side effect를 처리함
-> notify command를 active executor로 전달함
```

`CAction`은 실제 실행기의 역할을 수행함.

주요 책임은 다음과 같음.

```text
CAction
-> Start / ApplyChain / Stop / Complete lifecycle을 수행함
-> montage를 재생하고 정지함
-> active runtime data를 실행 중에만 캐싱함
-> notify command를 해석함
-> feedback request를 생성함
-> local level decision을 제공함
```

Action execution은 다음 흐름으로 정리됨.

```text
ActionComponent::ApplyActionDecision
-> TryStartAction / TryChainAction / TryReplaceAction
-> StartActiveActionInternal / ChainActiveActionInternal / StopActiveActionInternal
-> CAction::Start / CAction::ApplyChain / CAction::Stop
```

Notify 흐름은 component를 경유하여 active executor로 전달하는 구조로 정리함.

```text
AnimNotify / AnimNotifyState
-> ActionComponent Handle API
-> active CAction
-> HandleNotifyCommand / HandleNotifyFeedback
-> executor-specific behavior
```

이 구조는 notify가 구체 action subclass를 직접 알지 않아도 되게 하며, component가 active executor routing만 책임지도록 만듦.

## 5. 이후 작업의 방향성

현재 action notify command는 기본 공통 명령을 중심으로 정리되었음.

후속 작업에서는 action별 특수 command가 늘어날 경우 다음 방향 중 하나를 선택해야 함.

```text
공통 command 유지
-> 대부분의 action이 공유하는 window, feedback, complete, hit context에 적합함

executor-specific command 확장
-> action subclass가 command를 해석하고 필요한 동작만 override함

별도 action-specific notify 도입
-> 특정 action에서만 의미 있는 고유 이벤트에 한정함
```

현재 기준에서는 component API를 action 종류별로 늘리는 방식보다, notify command를 active executor로 전달하고 executor가 해석하는 방식이 더 적합함.

또한 reaction 쪽도 다음 브랜치에서 action과 유사한 lifecycle API 흐름으로 맞출 수 있음.

## 6. 결론

이번 refactor는 action component를 runtime state owner로 정리하고, `CAction`을 실행기와 local rule provider로 압축한 작업임.

그 결과 action data는 request 단위로 resolve되어 executor에 전달되고, executor는 실행 중 필요한 데이터만 캐싱하는 구조가 됨.

이 구조는 action executor 재사용성, notify routing 일관성, component/executor 책임 분리를 개선함.
