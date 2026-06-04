# Action Orchestration Refactor Architecture Decision

## 1. 목적

본 문서는 reaction orchestration을 고도화하면서 확인된 책임 분리 원칙을 기반으로, action orchestration을 어떤 방향으로 재구성할지 정리하기 위한 architecture decision 문서임.

핵심은 reaction 구조를 그대로 복사하는 것이 아니라, `Orchestrator -> Component -> Executor` 책임 분리 원칙을 action domain에 맞게 재적용하는 것임.

---

## 2. 배경

Reaction orchestration 작업을 통해 다음 구조가 비교적 명확해졌음.

```text
ReactionOrchestrator
-> request 해석 / context resolve / policy resolve / conflict decision 담당함

ReactionComponent
-> active runtime state / decision application / side effect 담당함

CReaction
-> montage lifecycle / notify window / feedback / local rule 담당함
```

이 구조는 reaction 실행 흐름을 `damage event -> orchestration -> component apply -> executor lifecycle`로 정리함.

그 결과 action orchestration도 현재 수준에서 유지하기보다, 동일한 수준의 책임 분리로 고도화할 필요가 생김.

현재 action 구조는 `ActionOrchestrator`가 존재하지만, 실제 실행 판단의 많은 부분이 `ActionComponent::ExecuteAction()`과 `CAction::DecideExecution()`에 남아 있음.

현재 기능이 combo attack / equip / unequip 중심일 때는 큰 문제가 아닐 수 있음.

그러나 guard / parry / dodge / counter / cancel / buffered input이 들어오면 action도 transition decision이 빠르게 복잡해짐.

따라서 action orchestration refactor의 목적은 action 실행 판단을 component와 executor에 흩어두지 않고, orchestrator 중심의 decision pipeline으로 정리하는 것임.

---

## 3. 유사하게 가져갈 수 있는 구조

Reaction에서 검증한 외부 구조는 action에도 적용 가능함.

```text
Orchestrator
-> request를 해석함
-> context와 policy를 resolve함
-> current runtime state와 incoming request를 비교함
-> final decision을 생성함

Component
-> active runtime state를 소유함
-> decision을 실제 character state에 적용함
-> executor cache / lookup을 담당함
-> movement / state / side effect를 처리함

Executor
-> 실제 montage lifecycle을 수행함
-> notify window와 feedback을 처리함
-> local rule hook을 제공함
```

이 구조가 action에도 가능한 이유는 action과 reaction 모두 다음 공통점을 갖기 때문임.

```text
외부 request가 들어옴
request를 실행 가능한 context로 해석해야 함
현재 active execution state와 비교해야 함
component가 실제 runtime mutation을 적용해야 함
executor가 montage / notify / feedback lifecycle을 수행함
```

따라서 action도 다음 API 흐름을 갖는 것이 적절함.

```text
RequestAction()
-> CanAcceptActionRequest()
-> ResolveActionCandidates()
-> ResolveActionContexts()
-> ResolveActionPolicy()
-> BuildActionQueries()
-> OrchestrateActionQueries()
-> DispatchActionDecision()
-> BuildRequestResult()
```

Component와 Executor도 reaction과 유사한 외부 형태를 가질 수 있음.

```text
UCActionComponent
-> ApplyActionDecision()
-> StartAction()
-> ChainAction()
-> QueueAction()
-> InterruptAction()
-> CancelAction()
-> CompleteCurrentAction()
-> AbortCurrentAction()

UCAction
-> Start()
-> Chain()
-> Stop()
-> Complete()
-> Abort()
-> local rule hook
-> feedback request
```

---

## 4. 다르게 가져가야 하는 구조

Action과 reaction은 외부 구조를 유사하게 가져갈 수 있지만, 내부 decision의 의미는 다름.

Reaction decision의 핵심은 외부 damage event에 의해 들어온 incoming reaction이 현재 active reaction과 어떤 경쟁 관계를 갖는지 판단하는 것임.

```text
active hit reaction 중 incoming hit reaction이 들어옴
active hit reaction 중 incoming dead reaction이 들어옴
incoming reaction priority가 더 높은가
active executor가 interruption을 허용하는가
incoming executor가 interruption을 원하는가
```

반면 action decision의 핵심은 player / AI intent가 현재 action lifecycle 안에서 어떤 transition으로 처리될 수 있는지 판단하는 것임.

```text
현재 action을 시작할 수 있는가
현재 action에서 chain 가능한가
현재 action에서 queue 가능한가
현재 action을 cancel하고 dodge할 수 있는가
counter attack이 현재 상태를 끊고 들어갈 수 있는가
equipment / stamina / body state가 action을 허용하는가
```

따라서 action orchestration은 reaction conflict resolution이 아니라 action transition orchestration에 가까움.

Action에서 중요한 구조는 `Intent -> Candidate -> Context -> Query -> Decision` 흐름임.

---

## 5. Action 해석 흐름

입력값은 구체 실행 데이터를 직접 선택하지 않음.

입력은 다음처럼 intent 수준만 전달하는 것이 적절함.

```text
LMB
-> Attack intent

Space
-> Dodge intent

RMB
-> Guard intent
```

`SwordLightAttack_02`나 `ComboIndex = 2` 같은 값은 입력에 직접 매핑하지 않음.

이 값들은 orchestrator가 현재 상태를 보고 해석해야 함.

권장 흐름은 다음과 같음.

```text
Raw Input
-> Intent
-> Action Request
-> Global / Body Policy Gate
-> Action Candidate 해석
-> Action Context 구체화
-> Executor Local Rule 질의
-> Final Orchestration Decision
-> Component Decision 적용
-> Executor 실행
```

`ActionCandidate`는 실행 가능 확정값이 아니라, intent를 현재 상태 기준으로 해석한 실행 후보임.

```text
Attack intent
-> Chain candidate
-> Queue candidate
-> Normal start candidate

Dodge intent
-> Reaction cancel candidate
-> Normal dodge candidate
```

`ActionContext`는 candidate를 실제 판단과 실행에 필요한 데이터로 구체화한 값임.

```text
ActionCandidate
-> ActionData 조회함
-> ActionExecutor 조회함
-> action type / index / combo group / cost / target context 구성함
```

Global / Body Policy Gate는 local rule보다 먼저 수행함.

```text
dead
invalid owner
invalid equipment
action lock
명확한 stamina 부족
```

이런 조건은 executor window를 묻기 전에 빠르게 reject하는 것이 적절함.

그 이후 active executor의 local rule을 질의함.

```text
현재 chain window가 열려 있는가
현재 queue window가 열려 있는가
현재 cancel window가 열려 있는가
현재 montage section이 incoming action을 허용하는가
```

최종 `Start / Chain / Queue / Interrupt / Cancel / Reject / Ignore` decision은 `ActionOrchestrator`가 생성함.

`CAction`은 final decision을 내리는 주체가 아니라, executor-local window와 local rule을 제공하는 주체임.

---

## 6. 데이터와 상태 책임

Reaction 작업에서 드러난 또 다른 쟁점은 definition data와 runtime state가 component 안에 함께 결속되어 있다는 점임.

현재 reaction은 1차 구현으로 definition data가 `ReactionComponent`에 남아 있고, orchestrator가 component를 통해 data / executor를 resolve함.

이 방식은 현재 단계에서는 허용 가능함.

다만 장기적으로 action과 reaction 모두 다음 기준으로 정리하는 것이 적절함.

```text
Definition Data
-> DataAsset / DataProvider / Orchestrator 인접 계층으로 이동함

Runtime State
-> Component가 소유함

Executor Instance Cache
-> Component가 소유함

Local Rule
-> Executor hook으로 제공함

Final Decision
-> Orchestrator가 생성함

Runtime Mutation
-> Component가 적용함
```

이 기준은 action orchestration refactor에서도 유지하는 것이 적절함.

---

## 7. 결정 사항

Action orchestration refactor는 reaction orchestration 구조를 기반으로 진행함.

다만 다음 기준을 유지함.

```text
외부 구조는 reaction과 대칭적으로 가져감
internal decision policy는 action domain에 맞게 재정의함
input은 intent까지만 책임짐
orchestrator가 candidate / context / policy / query / decision을 담당함
component는 active action state와 decision application을 담당함
CAction은 montage lifecycle과 local rule을 담당함
definition data 분리는 별도 단계로 고려함
```

이 작업은 reaction orchestration branch에 포함하지 않고 별도 action refactor branch에서 진행하는 것이 적절함.

후속 브랜치 후보는 다음과 같음.

```text
feature/action-orchestration-refactor
```

---

## 8. 리팩터링 범위

작업 범위는 다음 단계로 나누는 것이 적절함.

```text
1차
-> ActionOrchestrator API scaffold 정렬함
-> CanAccept / Candidate / Context / Policy / Query / Orchestrate / Dispatch 단계 추가함
-> 기존 gameplay 결과는 유지함

2차
-> ActionComponent::ExecuteAction()의 decision 책임을 orchestrator로 이동함
-> CAction::DecideExecution()을 local rule hook으로 축소함
-> ActionCandidate / ActionContext / ActionQuery / ActionResult 구조를 정리함

3차
-> Start / Chain / Queue / Cancel / Interrupt / Complete / Abort lifecycle을 정리함
-> ComboAttack / Equip / Unequip을 새 구조에 맞춤
-> Guard / Parry / Dodge / Counter 확장 지점을 연결함

4차
-> action definition data 분리 가능성을 검토함
-> ActionDataAsset / DataProvider 구조를 검토함
```

---

## 9. 예상 수정 영역

예상 수정 파일은 다음과 같음.

```text
Source/Portfolio/Component/CActionOrchestratorComponent.h
Source/Portfolio/Component/CActionOrchestratorComponent.cpp
Source/Portfolio/Component/CActionComponent.h
Source/Portfolio/Component/CActionComponent.cpp
Source/Portfolio/Action/CAction.h
Source/Portfolio/Action/CAction.cpp
Source/Portfolio/Action/CAction_ComboAttack.h/.cpp
Source/Portfolio/Action/CAction_Equip.h/.cpp
Source/Portfolio/Action/CAction_Unequip.h/.cpp
Source/Portfolio/Type/CActionOrchestrationStructure.h
Source/Portfolio/Type/CWeaponStructure.h
Source/Portfolio/Character/Player/CPlayer.cpp
Source/Portfolio/Character/Enemy/CEnemy.cpp
Source/Portfolio/AI/BehaviorTree/Task/CBTTask_StartCombatAction.cpp
```

1차 리팩터링에서는 action feedback, notify, weapon hit context는 최대한 건드리지 않는 것이 적절함.

다만 lifecycle 정리 단계에서는 action notify와 complete / abort 흐름도 함께 검토해야 함.

---

## 10. 작업량 판단

작업량은 중간 이상임.

단순한 함수 이름 변경이 아니라 action execution decision의 책임 위치를 바꾸는 작업이기 때문임.

대략적인 난이도는 다음과 같음.

```text
1차 API 정렬
-> 낮음 ~ 중간

2차 판단 책임 이동
-> 중간

3차 lifecycle 정리
-> 중간 ~ 높음

4차 data 분리 검토
-> 중간
```

특히 `CAction_ComboAttack`은 chain window, action index, feedback request, hit context와 연결되어 있으므로 가장 주의가 필요함.

`Equip` / `Unequip`은 상대적으로 단순하지만, weapon state와 연결되어 있어 request reject reason을 명확히 유지해야 함.

---

## 11. 결과

이 결정의 장점은 다음과 같음.

```text
reaction에서 검증한 책임 분리 원칙을 action에도 적용할 수 있음
action decision이 component / executor에 분산되는 것을 줄일 수 있음
guard / parry / dodge / counter / cancel 확장을 위한 decision pipeline을 확보함
action과 reaction의 외부 구조를 대칭적으로 유지할 수 있음
각 domain의 내부 decision policy는 독립적으로 유지할 수 있음
```

주의할 점은 다음과 같음.

```text
reaction 구조를 그대로 복사하면 action transition 의미를 놓칠 수 있음
action은 conflict resolution보다 transition orchestration이 중심임
Candidate / Context / Policy / Local Rule의 책임을 초기에 명확히 해야 함
definition data 분리는 action refactor의 필수 1차 조건은 아니지만 장기 과제임
```

---

## 12. 후속 작업

후속 작업 후보는 다음과 같음.

```text
ActionOrchestrationStructure 재정의함
ActionCandidate / ActionContext / ActionQuery / ActionResult 설계함
ActionOrchestrator를 Request / Resolve / Policy / Query / Orchestrate / Dispatch 구조로 리팩터링함
ActionComponent를 active state와 decision application 중심으로 축소함
CAction::DecideExecution()을 local rule hook으로 재정의함
Guard / Parry / Dodge / Counter decision policy를 action orchestration에 연결함
```

---