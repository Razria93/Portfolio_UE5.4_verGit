# 액션 오케스트레이션 구현 계획

## 1. 목적

본 문서는 액션 오케스트레이션 구조를 실제 코드에 적용하기 전에, 
현재 구조의 어떤 부분을 문제로 보고 있으며 어떤 방향으로 정리해 나갈지를 기록하기 위한 설계 기획 문서임.

핵심 목표는 다음과 같음.

- Player 입력과 AI 행동 요청이 같은 실행 관문을 사용하도록 함.
- `Intent`, `ExecutionState`, `ActionType`, `WeaponState`, `MovementState`의 책임을 분리함.
- 기존 구조를 한 번에 교체하지 않고 낮은 리스크 순서로 점진 적용함.
- StateComp와 MovementComp를 먼저 안정화한 뒤 ActionComp, WeaponComp, ReactionComp, AI 연결로 확장함.


---

## 2. 현재 문제 인식

현재 구조에서 문제로 보는 부분은 다음과 같음.

- 입력 의도와 실제 실행 타입이 충분히 분리되지 않음
- `State`가 최상위 실행 상태와 구체 행동 의미를 함께 들고 있어 비대해질 수 있음
- Player와 AI가 비슷한 실행을 하면서도 진입 경로와 판단 기준이 따로 놀 가능성이 있음
- 장착, 공격, 이동, 리액션의 책임 경계가 아직 명확하지 않음
- 이후 확장 시 공통 실행 규칙이 여러 계층에 중복될 위험이 있음

즉 현재 문제는 기능 하나가 아니라,  
실행 관문과 상태 축이 아직 충분히 정리되지 않았다는 점임.


---

## 3. 목표 구조

전체 흐름은 다음과 같음.

```text
Input / AI Decision
-> Intent Request
-> ActionOrchestrator
-> Domain Component
-> Execution State / Result
```

컴포넌트 책임은 다음과 같이 분리하는 것을 목표로 함.

```text
StateComp
- 최상위 실행 상태만 관리함
- ExecutionState: Idle / Action / Reaction / Dead

MovementComp
- 이동 입력, 속도 모드, 점프, 낙하 상태를 관리함
- MovementIntent 처리 대상임

ActionComp
- 캐릭터가 스스로 시작한 주도 행동을 관리함
- ActionType: Equip / Unequip / LightAttack / ComboAttack / Guard / Dodge
- ActionState: Idle / Playing

WeaponComp
- 장착물 종류와 실제 무기 전환 상태를 관리함
- AttachmentType
- EquipmentType
- WeaponState: Unequipped / Equipping / Equipped / Unequipping

ReactionComp
- 외부 자극에 의한 반응을 관리함
- ReactionType

AI / Behavior Tree
- AI 상위 판단 상태를 관리함
- AIIntentState: Patrol / Chase / Engage 등
```


---

## 4. 입력 데이터 흐름

### 4.1 Player 흐름

Player 입력은 Controller에서 시작하고,  
Player Character가 이를 Intent Request로 변환하는 구조를 목표로 함.

```text
CPlayerController
-> ACPlayer::HandleXXX()
-> FMovementActionRequest / FCombatActionRequest / FEquipmentActionRequest 생성
-> UCActionOrchestratorComponent::RequestXXXAction()
-> MovementComp / ActionComp / WeaponComp 호출
-> StateComp 및 각 도메인 상태 갱신
```

핵심은 다음과 같음.

- PlayerController는 Raw Input 수신 계층으로 유지함
- ACPlayer는 Raw Input을 도메인별 요청으로 변환함
- 실제 실행 판단은 ActionOrchestrator가 담당함


---

### 4.2 AI 흐름

AI는 Behavior Tree와 Blackboard에서 상위 의도를 판단하고,  
Task에서 구체 행동 요청을 생성하는 방향을 목표로 함.

```text
BT Service
-> Blackboard / AIIntentState 갱신

BT Task
-> FCombatActionRequest / FMovementActionRequest / FEquipmentActionRequest 생성
-> UCActionOrchestratorComponent::RequestXXXAction()
-> 공통 실행 규칙 통과
-> ActionComp / MovementComp / WeaponComp 호출
```

즉 Player와 AI는 입력 출처만 다르고 실행 관문은 같아야 함.

```text
PlayerInput -> IntentRequest
AI Task     -> IntentRequest
```


---

## 5. Intent와 실행 타입 관계

Intent와 실행 타입은 반드시 1:1로 매칭되지 않음.

```text
MovementIntent::Run
-> SpeedType::Run

MovementIntent::Jump
-> Character::Jump()

EquipmentIntent::Toggle
-> ActionType::Equip 또는 ActionType::Unequip

CombatIntent::ComboAttack
-> ActionType::ComboAttack
```

정리 기준은 다음과 같음.

- Intent는 아직 실행되기 전의 요청 의도임
- Type과 State는 실행 결과 또는 현재 상태를 나타냄
- 따라서 `EActionType`을 모든 입력 요청의 타입으로 사용하지 않음


---

## 6. Orchestrator 책임

`UCActionOrchestratorComponent`는 세부 실행을 소유하지 않음.

판단과 라우팅만 담당하는 구조를 목표로 함.

```text
1. 요청 유효성 검사
2. 공통 실행 가능 여부 확인
3. 도메인별 조건 확인
4. Intent를 실행 타입으로 변환
5. 해당 컴포넌트 호출
6. Result 반환
```

### Combat 요청 예시

```text
RequestCombatAction
- Dead / Reaction 여부 확인
- WeaponState 확인
- ActionState 확인
- CombatIntent를 ActionType으로 변환
- ActionComp 실행 요청
- FActionRequestResult 반환
```

### Movement 요청 예시

```text
RequestMovementAction
- Dead / Reaction 여부 확인
- MovementIntent 확인
- MovementComp 또는 CharacterMovementComponent 실행
- FActionRequestResult 반환
```

단, `StopJump`처럼 입력 해제 정리 이벤트에 가까운 요청은  
공통 행동 차단 전에 처리할 수 있음.

### Equipment 요청 예시

```text
RequestEquipmentAction
- Dead / Reaction 여부 확인
- 현재 WeaponState / AttachmentType 확인
- EquipmentIntent를 ActionType::Equip 또는 ActionType::Unequip으로 변환
- ActionComp와 WeaponComp 실행 요청
- FActionRequestResult 반환
```


---

### 6.1 계층별 실행 책임

현재 구조에서 각 계층의 실행 책임은 다음과 같이 정리하는 것이 적절함.

```text
Orchestrator
-> Intent를 받음
-> 공통 규칙을 검사함
-> 실행 타입과 실행 경로를 resolve함
-> 해당 도메인 컴포넌트에 실행을 위임함

ActionComponent
-> ActionType에 대응하는 실행 객체를 찾음
-> 현재 실행 문맥과 함께 실행 결정을 요청함
-> Start / Chain / Reject 결과에 맞는 흐름을 연결함

Action
-> 실행 조건을 소유함
-> 타이밍 의미를 소유함
-> 실제 실행 로직과 cleanup 책임을 소유함
```

즉 오케스트레이터는 의도를 실행 가능한 데이터와 경로로 연결하고,  
ActionComponent는 그 실행을 실제 Action 객체에 전달하는 허브 역할을 하며,  
실질적인 행동 의미와 세부 실행은 Action 객체가 소유하는 구조를 목표로 함.


---

## 7. 작업 순서

### 7.1 StateComp 정리

가장 먼저 StateComp를 정리함.

StateComp는 최상위 실행 상태만 관리하도록 축소하는 것을 목표로 함.

```text
EExecutionState
- Idle
- Action
- Reaction
- Dead
```

작업 내용:

- 기존 `EStateType`을 `EExecutionState` 개념으로 재정의함
- `Equip / Unequip`을 StateComp에서 제거함
- API를 `SetIdleState`, `SetActionState`, `SetReactionState`, `SetDeadState`로 정리함
- 기존 참조부를 수정함


---

### 7.2 MovementComp 정리

MovementComp는 상대적으로 독립적이므로 StateComp 다음에 정리함.

작업 내용:

- `OnJump`, `OnStopJump`를 유지함
- `SpeedType`, `CurrentSpeed`, `CurrentDirection`, `bIsFalling` 책임을 명확히 함
- 필요 시 `EMovementState`를 추가함
- `RequestMovementAction`과 연결을 확인함


---

### 7.3 ActionComp 정리

ActionComp는 주도 행동 실행의 중심으로 정리함.

작업 내용:

- `EActionType`을 `None / Equip / Unequip / LightAttack / ComboAttack / Guard / Dodge` 기준으로 정리함
- `EActionState`를 추가함
- `TryChangeActionMode()` 같은 반환형 API를 도입함
- Orchestrator의 Combat 결과 정확도를 개선함


---

### 7.4 WeaponComp 정리

WeaponComp는 실제 무기 장착 상태와 전환 상태를 관리하도록 정리함.

작업 내용:

- `EWeaponState`를 추가함
- `Unequipped / Equipping / Equipped / Unequipping` 상태를 정의함
- 장착/해제 시작, Notify 중간 타이밍, 완료 타이밍에서 상태 전환함
- `AttachmentType`과 `WeaponState`의 의미를 분리함


---

### 7.5 Equipment Action 연결

Equipment 요청을 ActionComp와 WeaponComp가 함께 처리하도록 연결함.

작업 내용:

- `EquipmentIntent::Toggle`을 처리함
- 현재 `WeaponState / AttachmentType` 기준으로 `ActionType::Equip` 또는 `ActionType::Unequip`을 결정함
- ActionComp와 WeaponComp 실행을 연결함


---

### 7.6 ReactionComp 정리

ReactionComp는 Damage / Health / State와 연결되어 있으므로 뒤쪽 단계에서 정리함.

작업 내용:

- `ExecutionState::Reaction` 진입/종료 기준을 정리함
- Dead 수렴 규칙을 정리함
- Damage / Reaction 흐름 회귀를 확인함


---

### 7.7 AI 연결

AI는 마지막에 점진적으로 연결함.

작업 내용:

- AIIntentState는 Blackboard / BT에서 유지함
- BTTask_StartAttack 등 직접 실행부를 Orchestrator 요청으로 점진 이전함
- Player와 AI가 같은 `UCActionOrchestratorComponent`를 사용하도록 함


---

## 8. 1차 작업 범위

초기 구현 범위는 StateComp와 MovementComp 중심으로 한정함.

```text
작업 1:
- StateComp를 ExecutionState 기준으로 정리함
- MovementComp의 요청 처리 흐름을 안정화함
- Orchestrator의 Movement 경로를 확인함
```

이 단계에서는 ActionComp와 WeaponComp 구조를 깊게 변경하지 않음.


---

## 9. 작업 1 완료 기준

작업 1은 다음 조건을 만족하면 완료로 봄.

```text
- MoveForward / MoveRight 기존 동작 유지
- Walk / Run이 Orchestrator 경유로 동작
- Jump / StopJump가 Orchestrator 경유로 동작
- Dead / Reaction 중 Walk / Run / Jump 차단
- StopJump는 정리 이벤트로 정상 처리
- StateComp는 Idle / Action / Reaction / Dead 기준으로 정리됨
```

위 기준을 만족하면 다음 단계인 ActionComp / WeaponComp 리팩터링으로 이동함.


---

## 10. 설계 원칙

```text
Intent = 아직 실행되기 전의 요청 의도
State  = 현재 머무르는 상태
Type   = 종류 또는 식별자
Result = 요청 처리 결과
```

오케스트레이터는 실행 자체를 소유하지 않음.

오케스트레이터는 요청을 판단하고 적절한 도메인 컴포넌트로 전달함.

Player와 AI는 서로 다른 의사결정 경로를 가지더라도  
같은 실행 상태와 액션 실행 규칙을 공유해야 함.


---

## 11. 실패 처리 정책

액션 오케스트레이션 구조에서 실패 처리 정책은 다음 기준으로 정리하는 것이 적절함.

### 11.1 기본 원칙

```text
Reject / Ignore = observable state change 없음
Start / Chain / Enqueue / Interrupt = commit 허용
```

여기서 observable state change는 다음 값을 포함함.

- ExecutionState
- CurrentActionType
- WeaponState
- Blackboard 전투 상태값
- cooldown timestamp
- event broadcast
- montage / feedback / hit context

즉 실행이 거절되거나 무시된 경우, 외부에서 관찰 가능한 상태 변화가 남지 않는 구조를 목표로 함.


---

### 11.2 commit 시점 원칙

실행 성공 전에는 가능한 한 commit을 지연하는 것이 적절함.

```text
실행 성공 전 = 판단 / 검증 / resolve 단계
실행 성공 후 = state commit 허용 단계
```

즉 다음 기준을 따름.

- 요청 검증과 실행 타입 resolve는 side effect 없이 처리함
- 실제 `Start / Chain / Enqueue / Interrupt`가 확정된 뒤에만 상태를 commit함
- cooldown, blackboard, action lifecycle 값은 성공 이후에만 반영함

즉 상태를 무조건 먼저 바꾸고 나서 실행하는 것이 아니라,  
실행 성공이 확정된 경로에서만 필요한 상태를 반영하는 구조를 목표로 함.


---

### 11.3 Orchestrator 원칙

`UCActionOrchestratorComponent`는 rollback을 많이 수행하는 계층이 아니라,  
rollback이 거의 필요 없도록 side effect를 늦추는 계층으로 두는 것이 적절함.

즉 역할은 다음에 집중함.

```text
1. 요청 유효성 검사
2. 공통 차단 조건 확인
3. Intent -> 실행 타입 변환
4. 실행 위임
5. 결과 반환
```

오케스트레이터는 가능하면 실행 전 상태를 먼저 바꾸지 않고,  
판단과 라우팅만 담당하는 구조를 유지함.


---

### 11.4 ActionComponent 원칙

`UCActionComponent::ExecuteAction()`은 다음 계약을 만족하는 것이 적절함.

```text
Reject / Ignore
-> current action / execution state / event / feedback 변화 없음

Start
-> action start commit 허용

Chain
-> chain input commit 허용
```

즉 실패하면 아무 일도 일어나지 않은 것처럼 끝나고,  
성공하면 완결된 상태 변화만 남는 구조를 목표로 함.


---

### 11.5 Action cleanup 원칙

`UCAction` 계열의 `Abort()`와 `Complete()`는 cleanup endpoint로 동작해야 함.

정리 대상 예시는 다음과 같음.

- 내부 실행 플래그
- chain window 상태
- chained input 상태
- hit context
- action-local transient data

즉 시작과 체인은 성공 시점에만 commit하고,  
중단과 완료는 반드시 cleanup을 보장하는 구조가 적절함.

또한 feedback은 실행 이후 일괄 처리되는 개념이 아니라,  
Action이 정의한 타이밍에 따라 발생하는 것으로 정리하는 것이 적절함.


---

### 11.6 Blackboard 원칙

Blackboard는 실행 의도를 추측해서 기록하는 계층이 아니라,  
실제 lifecycle signal 또는 service 파생 계산 결과를 담는 계층으로 유지하는 것이 적절함.

예:

- `bCanCombatAction` = service 파생값
- `bIsCombatAction` = 실제 action lifecycle 기반 값
- `NextCombatActionTime` = 성공한 combat start 이후 commit

즉 BT Task가 실행 상태를 직접 추측해서 blackboard에 먼저 반영하지 않는 것을 원칙으로 함.


---

### 11.7 Reaction takeover 원칙

Reaction takeover는 단순히 state를 바꾸는 작업이 아니라,  
기존 active action cleanup까지 포함하는 전환으로 보는 것이 적절함.

즉 reaction 시작 전에는 다음을 보장해야 함.

- active action 존재 여부 확인
- 필요 시 action abort 수행
- reaction 시작 이후 execution state / current action / blackboard 상태 일관성 유지

이 기준은 이후 Reaction orchestration 및 execution coordination 작업의 기본 계약으로 사용함.
