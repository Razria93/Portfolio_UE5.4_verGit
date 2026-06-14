# Action Orchestration 이전 Player / AI Action 실행 흐름 비대칭 분석

## 1. 제목

M05-S10: Action Orchestration 이전 Player / AI Action 실행 흐름 비대칭 분석

---

## 2. 목적

본 문서는 `feature/action-orchestration` 브랜치에서 action orchestration을 도입하기 전에, Player와 AI의 action 실행 흐름이 어떻게 다르게 구성되어 있었는지 정리하는 문서임.

핵심은 Player는 input 이후 `CAction` 기반 실행 흐름을 사용했고, AI는 BehaviorTree / Blackboard 내부에서 attack montage 실행과 state 갱신을 처리했다는 점임.

이 문서는 Player / AI 실행 흐름 비대칭을 해소하기 위해 공통 action request entry를 도입하고, `ActionOrchestrator -> ActionComponent -> CAction` 구조로 책임을 재분배해야 했던 이유를 설명하는 기준으로 사용함.

---

## 3. 관련 브랜치

- `feature/action-orchestration`

---

## 4. 기존 시스템의 형태

### 1) Player 입력 기반 실행

기존 Player action 실행은 input과 action 실행 객체가 비교적 직접적으로 연결된 구조였음.

```yaml
Player Input
-> Player / WeaponComponent
-> ActionComponent 또는 Action Object
-> CAction
-> Montage 실행
```

이 구조에서는 입력이 들어온 뒤 component와 action object를 거쳐 action 실행으로 이어졌음.

### 2) AI BehaviorTree 기반 Attack 실행

기존 AI attack은 Player의 `CAction` 실행 흐름과 별도로 BehaviorTree task와 Blackboard 값을 중심으로 전개되었음.

```yaml
BehaviorTree
-> CBTTask_StartAttack
-> AttackIndex / AttackMontage 확인
-> Montage_Play
-> WeaponContext push
-> Blackboard attack state / cooldown 갱신
```

이 구조에서 BehaviorTree task는 AI 판단뿐 아니라 attack montage 실행, attack context 전달, blackboard state 갱신까지 함께 처리했음.

### 3) ActionComponent / CAction 중심 실행

기존 action 실행 구조에서는 `ActionComponent`와 `CAction`이 다음 책임을 나누어 가졌음.

```yaml
ActionComponent
- action object 보관
- 현재 action 상태 관리
- action 실행 요청 전달

CAction
- action별 실행 가능 조건 확인
- montage 실행
- state 변경
- notify command 처리
- complete / abort 처리
```

이 구조에서 `CAction`은 action별 실행 조건 확인과 montage lifecycle 처리를 함께 수행했음.

### 4) ComboAttack 실행 흐름

기존 Player ComboAttack은 `PreInput` notify와 내부 buffering 값을 기준으로 다음 attack을 이어가는 구조였음.

```yaml
Player ComboAttack
-> Montage 재생
-> PreInput notify
-> bEnablePreInput / bExistPreInput 갱신
-> NextPlayAction
-> 다음 ActionIndex montage 재생
```

이 구조에서 combo chain은 외부 request pipeline이 아니라 `UCAction_ComboAttack` 내부의 pre-input state와 `ActionIndex`를 기준으로 이어졌음.

### 5) Notify 기반 상태 전환

기존 action 종료와 chain timing은 montage notify에 강하게 의존했음.

```yaml
AnimNotify
-> EndAction / PreInput / CollisionWindow
-> ActionComponent 또는 CAction API 호출
-> state 변경 또는 다음 실행 처리
```

이 구조에서 Notify는 action 종료, pre-input, collision window 같은 montage timing event를 action 실행 흐름에 전달했음.

---

## 5. 기존 시스템의 문제 분석 및 한계

### 1) Player / AI 실행 흐름 비대칭

기존 구조의 우선적인 문제는 Player와 AI가 같은 combat action 계열 동작을 서로 다른 실행 절차로 처리했다는 점임.

```yaml
Player
-> input
-> ActionComponent / CAction
-> UCAction_ComboAttack
-> montage lifecycle

AI
-> BehaviorTree / Blackboard
-> CBTTask_StartAttack
-> Montage_Play
-> WeaponContext push
-> Blackboard attack state / cooldown 갱신
```

이 상태에서는 Player와 AI가 같은 attack montage를 재생하더라도 실행 조건, context 전달, state 갱신, cleanup 기준이 서로 다른 계층에서 관리되었음.

### 2) Combo Chain 처리 경로 분리

Player combo는 `UCAction_ComboAttack` 내부 pre-input buffering으로 이어졌고, AI attack은 Blackboard의 `AttackIndex`와 BT task의 montage 선택으로 실행되었음.

```yaml
문제
- Player combo chain과 AI attack selection 기준이 서로 다름
- AI가 Player와 같은 combo lifecycle을 재사용하기 어려움
- chain timing 이후 follow-up 실행을 공통 규칙으로 처리하기 어려움
- 같은 ComboAttack이라도 Player / AI 유지보수 경로가 분리됨
```

따라서 AI도 Player와 같은 combat action 실행 경로에서 chain 가능 여부와 다음 실행을 처리할 수 있는 구조가 필요했음.

### 3) 실행 판단과 실행 적용의 책임 혼재

기존 구조에서는 실행 가능 여부 판단과 실제 state 변경이 같은 계층에 섞여 있었음.

```yaml
문제
- CAction이 실행 가능 조건과 montage 실행을 함께 담당함
- ActionComponent가 active action state와 실행 요청 처리를 함께 담당함
- 실패 시 rollback 기준이 명확하지 않음
- 실행 판단 / runtime state 변경 / montage lifecycle 책임이 분리되지 않음
```

이 상태에서는 action 실행이 실패했을 때 observable state change를 남기지 않는 기준을 일관되게 적용하기 어려웠음.

### 4) Notify 의존 구조의 한계

기존 action 종료와 combo timing은 montage notify를 통해 action 실행 흐름에 전달되었음.

```yaml
문제
- Notify가 timing event 전달 외에 실행 판단 경로에도 관여할 수 있음
- action state와 montage lifecycle 책임이 섞일 수 있음
- Complete / Abort cleanup 기준이 notify timing에 과하게 의존할 수 있음
```

Notify는 timing event를 전달하는 데 적합하지만, 실행 판단 계층으로 사용되면 책임 경계가 흐려질 수 있음.

### 5) Reaction Takeover 확장 한계

combo action 도중 hit reaction이 들어오면 active action을 정리하고 reaction으로 넘어가야 함.

기존 구조에서는 action state, current action, blackboard combat flag가 서로 다른 계층에서 관리되었기 때문에, reaction 이후 combat flow가 어긋날 수 있었음.

```yaml
문제
- active action cleanup 누락 가능
- ExecutionState와 CurrentActionType 불일치 가능
- blackboard combat flag가 실제 action lifecycle과 어긋날 수 있음
```

---

## 6. 리팩터링 방향

### 1) 실행 흐름 비대칭 해소

리팩터링의 메인 방향은 Player와 AI의 실행 절차 차이를 줄이고, 실제 action 실행을 같은 pipeline에서 처리하도록 정리하는 것이었음.

```yaml
Before
- Player : input -> CAction 기반 action 실행
- AI     : BT / BB -> attack montage 직접 실행

After
- Player : input -> action request -> common action pipeline
- AI     : BT / BB -> action request -> common action pipeline
```

공통 action request entry는 이 비대칭을 해소하기 위한 수단으로 도입함.

### 2) Orchestrator 중심 Request Entry

Player input과 AI BehaviorTree가 결정한 실행 의도를 action request로 변환하고, 이를 orchestrator로 진입시키는 구조가 필요했음.

```yaml
Player Input / AI BT
-> Action Request
-> ActionOrchestrator
-> ActionComponent
-> CAction
```

### 3) 책임 분리 방향

각 계층의 책임은 다음처럼 분리하는 것이 목표였음.

```yaml
ActionOrchestrator
- request validation
- intent 해석
- action 실행 후보 구성
- execution result 생성

ActionComponent
- action object / active action state 소유
- execution result 적용
- state enter / exit 처리

CAction
- montage lifecycle 수행
- action별 local rule 처리
- notify command / feedback timing 처리
```

### 4) Player / AI 공통 실행 구조

Player와 AI는 서로 다른 판단 source를 가지지만, 실제 combat action 실행은 같은 request pipeline을 사용하도록 정리해야 했음.

```yaml
Player
-> input
-> combat action request

AI
-> BehaviorTree
-> combat action request

Common
-> ActionOrchestrator
-> ActionComponent
-> CAction
```

---

## 7. 관련 문서

이 문서는 아래 문서들과 같은 작업 시점 기준으로 함께 읽을 수 있음.

### 1) 같은 작업 단위

- `D16`
- `P15`

---

## 8. 결론

Action orchestration 도입 전 구조의 핵심 한계는 Player와 AI의 combat action 실행 절차가 분리되어 있었다는 점임.

Player는 input 이후 `CAction` 기반 lifecycle로 action을 실행했지만, AI는 BehaviorTree task와 Blackboard 값을 기준으로 attack montage 실행, context 전달, state 갱신을 직접 처리했음.

따라서 `feature/action-orchestration` 브랜치에서는 Player / AI combat action 흐름을 공통화하고, 이를 위한 수단으로 action request entry와 orchestrator 진입 구조를 도입해야 했음.

이 문서는 해당 리팩터링 이전의 Player / AI 실행 흐름 비대칭과 공통 action execution pipeline이 필요해진 이유를 정리하는 기준 문서임.

---
