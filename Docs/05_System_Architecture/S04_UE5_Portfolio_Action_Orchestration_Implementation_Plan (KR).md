# 액션 오케스트레이션 구현 계획

## 1. 목적

본 문서는 액션 오케스트레이션 구조를 실제 코드에 적용하기 전, 구현해야 할 구조와 입력 데이터 흐름, 작업 순서를 정리함.

핵심 목표는 다음과 같음.

- Player 입력과 AI 행동 요청이 같은 실행 관문을 사용하도록 함.
- `Intent`, `ExecutionState`, `ActionType`, `WeaponState`, `MovementState`의 책임을 분리함.
- 기존 구조를 한 번에 뒤엎지 않고 낮은 리스크 순서로 점진 적용함.
- StateComp와 MovementComp를 먼저 안정화한 뒤 ActionComp, WeaponComp, ReactionComp, AI 연결로 확장함.

---

## 2. 목표 구조

전체 흐름은 다음과 같음.

```text
Input / AI Decision
-> Intent Request
-> ActionOrchestrator
-> Domain Component
-> Execution State / Result
```

컴포넌트 책임은 다음과 같이 분리함.

```text
StateComp
- 최상위 실행 상태만 관리함.
- ExecutionState: Idle / Action / Reaction / Dead

MovementComp
- 이동 입력, 속도 모드, 점프, 낙하 상태를 관리함.
- MovementIntent 처리 대상임.

ActionComp
- 캐릭터가 스스로 시작한 주도 행동을 관리함.
- ActionType: Equip / Unequip / LightAttack / ComboAttack / Guard / Dodge
- ActionState: Idle / Playing

WeaponComp
- 장착물 종류와 실제 무기 전환 상태를 관리함.
- AttachmentType
- EquipmentType
- WeaponState: Unequipped / Equipping / Equipped / Unequipping

ReactionComp
- 외부 자극에 의한 반응을 관리함.
- ReactionType

AI / BT
- AI 상위 판단 상태를 관리함.
- AIIntentState: Patrol / Chase / Engage 등
```

---

## 3. 입력 데이터 흐름

### 3.1 Player 흐름

Player 입력은 Controller에서 시작해 Player Character가 Intent Request로 변환함.

```text
CPlayerController
-> ACPlayer::HandleXXX()
-> FMovementActionRequest / FCombatActionRequest / FEquipmentActionRequest 생성
-> UCActionOrchestratorComponent::RequestXXXAction()
-> MovementComp / ActionComp / WeaponComp 호출
-> StateComp 및 각 도메인 상태 갱신
```

PlayerController는 Raw Input을 수신하는 계층으로 유지함.

ACPlayer는 Raw Input을 도메인별 Intent Request로 변환함.

실제 실행 판단은 ActionOrchestrator가 담당함.

### 3.2 AI 흐름

AI는 BehaviorTree와 Blackboard에서 상위 의도를 판단하고, Task에서 구체 행동 요청을 생성함.

```text
BT Service
-> Blackboard / AIIntentState 갱신

BT Task
-> FCombatActionRequest / FMovementActionRequest / FEquipmentActionRequest 생성
-> UCActionOrchestratorComponent::RequestXXXAction()
-> 공통 실행 규칙 통과
-> ActionComp / MovementComp / WeaponComp 호출
```

Player와 AI는 입력 출처만 다르고 실행 관문은 같음.

```text
PlayerInput -> IntentRequest
AI Task     -> IntentRequest
```

---

## 4. Intent와 실행 타입 관계

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

Intent는 아직 실행되기 전의 요청 의도임.

Type과 State는 실행 결과 또는 현재 상태를 나타냄.

따라서 `EActionType`을 모든 입력 요청의 타입으로 사용하지 않음.

---

## 5. Orchestrator 책임

`UCActionOrchestratorComponent`는 세부 실행을 소유하지 않음.

판단과 라우팅만 담당함.

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

단, `StopJump`처럼 입력 해제 정리 이벤트에 가까운 요청은 공통 행동 차단 전에 처리할 수 있음.

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

## 6. 작업 순서

### 6.1 StateComp 정리

가장 먼저 StateComp를 정리함.

StateComp는 최상위 실행 상태만 관리하도록 축소함.

```text
EExecutionState
- Idle
- Action
- Reaction
- Dead
```

작업 내용:

- 기존 `EStateType`을 `EExecutionState` 개념으로 재정의함.
- `Equip / Unequip`을 StateComp에서 제거함.
- API를 `SetIdleState`, `SetActionState`, `SetReactionState`, `SetDeadState` 으로 정리함.
- 기존 참조부를 수정함.

### 6.2 MovementComp 정리

MovementComp는 상대적으로 독립적이므로 StateComp 다음에 정리함.

작업 내용:

- `OnJump`, `OnStopJump`를 유지함.
- `SpeedType`, `CurrentSpeed`, `CurrentDirection`, `bIsFalling` 책임을 명확히 함.
- 필요 시 `EMovementState`를 추가함.
- `RequestMovementAction`과 연결을 확인함.

### 6.3 ActionComp 정리

ActionComp는 주도 행동 실행의 중심으로 정리함.

작업 내용:

- `EActionType`을 `None / Equip / Unequip / LightAttack / ComboAttack / Guard / Dodge` 기준으로 정리함.
- `EActionState`를 추가함.
- `TryChangeActionMode()` 반환형 API를 도입함.
- Orchestrator의 Combat 결과 정확도를 개선함.

### 6.4 WeaponComp 정리

WeaponComp는 실제 무기 장착 상태와 전환 상태를 관리하도록 정리함.

작업 내용:

- `EWeaponState`를 추가함.
- `Unequipped / Equipping / Equipped / Unequipping` 상태를 정의함.
- 장착/해제 시작, Notify 중간 타이밍, 완료 타이밍에서 상태 전환함.
- `AttachmentType`과 `WeaponState`의 의미를 분리함.

### 6.5 Equipment Action 연결

Equipment 요청을 ActionComp와 WeaponComp가 함께 처리하도록 연결함.

작업 내용:

- `EquipmentIntent::Toggle`을 처리함.
- 현재 `WeaponState / AttachmentType` 기준으로 `ActionType::Equip` 또는 `ActionType::Unequip`을 결정함.
- ActionComp와 WeaponComp 실행을 연결함.

### 6.6 ReactionComp 정리

ReactionComp는 Damage/Health/State와 연결되어 있으므로 뒤쪽 단계에서 정리함.

작업 내용:

- `ExecutionState::Reaction` 진입/종료 기준을 정리함.
- Dead 수렴 규칙을 정리함.
- Damage/Reaction 흐름 회귀를 확인함.

### 6.7 AI 연결

AI는 마지막에 점진적으로 연결함.

작업 내용:

- AIIntentState는 Blackboard/BT에서 유지함.
- BTTask_StartAttack 등 직접 실행부를 Orchestrator 요청으로 점진 이전함.
- Player와 AI가 같은 `UCActionOrchestratorComponent`를 사용하도록 함.

---

## 7. 1차 작업 범위

다음 실제 구현 작업은 넓게 잡지 않고 StateComp와 MovementComp 중심으로 진행함.

```text
작업 1:
- StateComp를 ExecutionState 기준으로 정리함.
- MovementComp의 요청 처리 흐름을 안정화함.
- Orchestrator의 Movement 경로를 확인함.
```

이 작업에서는 ActionComp와 WeaponComp 구조를 깊게 변경하지 않음.

장착/해제와 공격 구조는 다음 작업으로 넘김.

---

## 8. 작업 1 완료 기준

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

## 9. 설계 원칙

```text
Intent = 아직 실행되기 전의 요청 의도
State  = 현재 머무르는 상태
Type   = 종류 또는 식별자
Result = 요청 처리 결과
```

오케스트레이터는 실행 자체를 소유하지 않음.

오케스트레이터는 요청을 판단하고 적절한 도메인 컴포넌트로 전달함.

Player와 AI는 서로 다른 의사결정 경로를 가지더라도 같은 실행 상태와 액션 실행 규칙을 공유함.
