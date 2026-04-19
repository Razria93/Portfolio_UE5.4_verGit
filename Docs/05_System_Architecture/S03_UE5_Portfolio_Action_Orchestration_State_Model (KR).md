# 액션 오케스트레이션 상태 모델

## 1. 목적

본 문서는 Player와 AI가 공통으로 사용할 수 있는 액션 실행 구조에서 `State`, `Type`, `Intent`의 책임 경계를 정의함.

핵심 목표는 다음과 같음.

- 캐릭터의 최상위 실행 상태를 단순하게 유지함.
- 이동, 주도 행동, 무기 상태, 리액션, AI 판단 상태를 서로 다른 축으로 분리함.
- Player 입력과 AI BehaviorTree 요청이 같은 실행 관문을 사용할 수 있도록 함.
- `State` enum이 구체 행동 목록으로 비대해지는 것을 방지함.

---

## 2. 핵심 분리 기준

```text
ExecutionState = 캐릭터 몸의 최상위 실행 상태
Movement       = 이동 입력, 이동 모드, 이동 물리 상태
Action         = 캐릭터가 수행하는 주도 행동
Weapon         = 장착물/무기 보유 및 전환 상태
Reaction       = 외부 자극에 의해 발생한 반응
AIIntent       = AI의 상위 판단 상태
```

각 축은 동시에 존재할 수 있음.

예를 들어 AI가 전투 의도를 가진 상태에서 공격 액션을 실행 중일 수 있음.

```text
AIIntentState  = Engage
ExecutionState = Action
ActionType     = ComboAttack
ActionState    = Playing
WeaponState    = Equipped
AttachmentType = Sword
```

또는 캐릭터가 자유 상태에서 이동 중일 수 있음.

```text
ExecutionState = Idle
MovementState  = Moving
SpeedType      = Run
```

따라서 위 값들을 하나의 `State`로 통합하지 않음.

---

## 3. StateComp

`StateComp`는 캐릭터의 최상위 실행 상태만 관리함.

```cpp
UENUM(BlueprintType)
enum class EExecutionState : uint8
{
	Idle = 0,
	Action,
	Reaction,
	Dead,
	Max,
};
```

### 상태 의미

```text
Idle     = 주도 행동, 피격 반응, 사망 상태가 아님
Action   = 캐릭터가 스스로 시작한 행동 실행 중
Reaction = 외부 자극에 대한 반응 실행 중
Dead     = 사망 또는 비활성 상태
```

`ExecutionState`에는 `Move`, `Walk`, `Run`, `Jump`, `Equip`, `Unequip`, `Dodge`, `Guard`, `LockOn`, `Patrol` 같은 구체 행동이나 판단 상태를 넣지 않음.

이유는 `ExecutionState`가 최상위 배타 상태여야 하기 때문임.

---

## 4. MovementComp

`MovementComp`는 이동 입력, 이동 모드, 이동 물리 상태를 관리함.

이동은 항상 `ActionComp`의 액션으로 취급하지 않음.

`Move`, `Walk`, `Run`, `Jump`, `StopJump`는 우선 `MovementIntent`로 들어오고, 오케스트레이터가 현재 상태를 확인한 뒤 `MovementComp` 또는 `CharacterMovementComponent`로 전달함.

```cpp
UENUM(BlueprintType)
enum class EMovementActionIntent : uint8
{
	None = 0,

	Move,
	Walk,
	Run,
	Jump,
	StopJump,

	Max,
};
```

이동 관련 상태값은 `MovementComp`가 소유함.

예시:

```cpp
UENUM(BlueprintType)
enum class EMovementState : uint8
{
	Idle = 0,
	Moving,
	Falling,
	Max,
};
```

속도 모드가 더 중요한 경우 `SpeedType`을 별도로 관리함.

```cpp
UENUM(BlueprintType)
enum class ESpeedType : uint8
{
	Walk = 0,
	Run,
	Sprint,
	Max,
};
```

### MovementIntent와 ActionType의 관계

의도와 실행 액션은 반드시 1:1로 매칭되지 않음.

```text
MovementIntent::Walk
-> MovementComp::SetSpeedType(Walk)

MovementIntent::Run
-> MovementComp::SetSpeedType(Run)

MovementIntent::Move
-> MovementComp::AddMovementInput(...)

MovementIntent::Jump
-> Character::Jump()
```

위 요청들은 기본적으로 `EActionType`으로 승격하지 않음.

단, 이동이 전투 액션 또는 주도 행동으로 설계되는 경우에는 `ActionType`으로 승격할 수 있음.

예시:

```text
CombatIntent::Dodge
-> ActionType::Dodge

MovementIntent::Jump
-> 단순 점프면 MovementComp 처리
-> 점프 공격 또는 회피 점프면 ActionType으로 승격 가능
```

---

## 5. ActionComp

`ActionComp`는 캐릭터가 스스로 시작한 주도 행동을 관리함.

장착과 해제도 캐릭터가 직접 시작한 행동이므로 액션으로 취급함.

```cpp
UENUM(BlueprintType)
enum class EActionType : uint8
{
	None = 0,

	Equip,
	Unequip,

	LightAttack,
	ComboAttack,
	Guard,
	Dodge,

	All,
	Max,
};
```

액션의 실행 상태는 처음에는 최소 단위로 둠.

```cpp
UENUM(BlueprintType)
enum class EActionState : uint8
{
	Idle = 0,
	Playing,
	Max,
};
```

필요해지면 이후 `Pending`, `Buffered`, `Recovery`, `Canceled` 같은 상태를 추가함.

실제 정책 없이 enum만 먼저 늘리지 않음.

### 예시

공격 액션 중:

```text
ExecutionState = Action
ActionType     = ComboAttack
ActionState    = Playing
```

장착 액션 중:

```text
ExecutionState = Action
ActionType     = Equip
ActionState    = Playing
```

---

## 6. WeaponComp

`WeaponComp`는 현재 장착 형태와 무기 전환 상태를 관리함.

```cpp
UENUM(BlueprintType)
enum class EAttachmentType : uint8
{
	Unarmed = 0,
	Sword,
	All,
	Max,
};
```

`EquipmentType`은 장비 실행 객체 또는 장비 데이터의 큰 분류로 사용함.

실제 장비 동작이 `UObject` 기반으로 확장된다면 enum을 과도하게 늘리지 않음.

```cpp
UENUM(BlueprintType)
enum class EEquipmentType : uint8
{
	None = 0,
	Default,
	All,
	Max,
};
```

장착 모션 중간에 실제 무기 부착 또는 해제 타이밍이 있으므로 `WeaponState`는 액션 상태와 별도로 둠.

```cpp
UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	Unequipped = 0,
	Equipping,
	Equipped,
	Unequipping,
	Max,
};
```

### ActionState와 WeaponState를 분리하는 이유

`ActionState::Playing`은 액션이 재생 중이라는 뜻임.

`WeaponState`는 실제 무기가 사용 가능한 상태인지, 아직 전환 중인지 표현함.

장착 액션 시작 직후:

```text
ExecutionState = Action
ActionType     = Equip
ActionState    = Playing
WeaponState    = Equipping
AttachmentType = Unarmed
```

장착 몽타주 중간 Notify에서 무기가 손에 붙은 순간:

```text
ExecutionState = Action
ActionType     = Equip
ActionState    = Playing
WeaponState    = Equipped
AttachmentType = Sword
```

장착 액션 완료:

```text
ExecutionState = Idle
ActionType     = None
ActionState    = Idle
WeaponState    = Equipped
AttachmentType = Sword
```

해제도 같은 방식으로 처리함.

---

## 7. ReactionComp

`ReactionComp`는 외부 자극에 의해 발생한 반응을 관리함.

```cpp
UENUM(BlueprintType)
enum class EReactionType : uint8
{
	None = 0,
	Hit,
	Knockback,
	Dead,
	Max,
};
```

피격 반응 중:

```text
ExecutionState = Reaction
ReactionType   = Hit
```

사망 반응은 최종적으로 `ExecutionState::Dead`로 수렴함.

---

## 8. AI Intent

AI의 상위 판단 상태는 `StateComp`에 넣지 않음.

AI 판단 상태는 BehaviorTree, Blackboard, AIController가 관리함.

```cpp
UENUM(BlueprintType)
enum class EAIIntentState : uint8
{
	Idle = 0,
	Patrol,
	Investigate,
	Chase,
	Alert,
	Engage,
	Dead,
	Max,
};
```

`EAIIntentState`는 AI가 무엇을 하려고 판단 중인지 나타냄.

`EExecutionState`는 캐릭터 몸이 실제로 어떤 실행 상태인지 나타냄.

예시:

```text
AIIntentState  = Engage
ExecutionState = Idle
```

공격 요청 가능.

```text
AIIntentState  = Engage
ExecutionState = Reaction
```

피격 반응 중이므로 공격 요청 거절.

---

## 9. Orchestrator 판단 기준

`ActionOrchestratorComponent`는 Player 입력 또는 AI Task에서 생성한 요청을 받아 공통 실행 규칙을 적용함.

### Movement 요청

```text
RequestMovementAction
- ExecutionState 확인
- MovementIntent 확인
- 이동 가능 여부 확인
- MovementComp 또는 CharacterMovementComponent 실행
```

예:

```text
MovementIntent = Run
-> SpeedType = Run
```

### Combat 요청

```text
RequestCombatAction
- ExecutionState 확인
- WeaponState 확인
- ActionState 확인
- CombatIntent를 ActionType으로 변환
- ActionComp 실행
```

예:

```text
CombatIntent = ComboAttack
-> ActionType = ComboAttack
```

### Equipment 요청

```text
RequestEquipmentAction
- ExecutionState 확인
- WeaponState 확인
- EquipmentIntent를 ActionType::Equip 또는 ActionType::Unequip으로 변환
- ActionComp와 WeaponComp 실행
```

예:

```text
EquipmentIntent = Toggle
WeaponState     = Unequipped
-> ActionType   = Equip
```

---

## 10. 최종 구조

```text
[StateComp]
EExecutionState
- Idle
- Action
- Reaction
- Dead

[MovementComp]
EMovementActionIntent
- None
- Move
- Walk
- Run
- Jump
- StopJump
- Max

EMovementState
- Idle
- Moving
- Falling
- Max

ESpeedType
- Walk
- Run
- Sprint
- Max

[ActionComp]
EActionType
- None
- Equip
- Unequip
- LightAttack
- ComboAttack
- Guard
- Dodge
- All
- Max

EActionState
- Idle
- Playing
- Max

[WeaponComp]
EAttachmentType
- Unarmed
- Sword
- All
- Max

EEquipmentType
- None
- Default
- All
- Max

EWeaponState
- Unequipped
- Equipping
- Equipped
- Unequipping
- Max

[ReactionComp]
EReactionType
- None
- Hit
- Knockback
- Dead
- Max

[AI]
EAIIntentState
- Idle
- Patrol
- Investigate
- Chase
- Alert
- Engage
- Dead
- Max
```

---

## 11. 설계 원칙

```text
ExecutionState = 최상위 배타 실행 상태
MovementState  = 이동 물리 또는 이동 진행 상태
SpeedType      = 이동 속도 모드
ActionType     = 캐릭터가 스스로 시작한 행동 종류
ActionState    = 액션 실행 상태
WeaponState    = 무기 전환 및 실제 사용 가능 상태
AttachmentType = 현재 장착물 종류
ReactionType   = 외부 반응 종류
AIIntentState  = AI 판단 상태
```

`State`로 모든 것을 통합하지 않음.

`State`, `Type`, `Intent`, `Result`의 책임을 분리함.

```text
State  = 현재 머무르는 상태
Type   = 종류 또는 식별자
Intent = 아직 실행되기 전의 요청 의도
Result = 처리 결과
```

의도와 실행은 반드시 1:1로 매칭되지 않음.

```text
EquipmentIntent::Toggle
-> ActionType::Equip 또는 ActionType::Unequip

MovementIntent::Run
-> SpeedType::Run

CombatIntent::ComboAttack
-> ActionType::ComboAttack
```

이 구조는 Player와 AI가 서로 다른 의사결정 경로를 가지더라도 같은 실행 상태와 액션 실행 규칙을 공유할 수 있도록 함.
