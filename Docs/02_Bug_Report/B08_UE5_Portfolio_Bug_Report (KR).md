# UE5 Portfolio Bug Report (KR)

## 제목

**M05-B03: AI가 combo action 도중 피격되면 reaction 이후 action 상태가 꼬여 전투 흐름이 고장남**

### Date

- **2026.04.29**

### Type

- Bug

### Status

- [x] Resolved

### Branch

- `feature/action-orchestration`


---

## 요약

- AI가 `ComboAttack` 수행 중 피격당해 Reaction에 진입한 뒤, Reaction 종료 후 action / state / blackboard 상태가 어긋나 이후 전투 흐름이 비정상적으로 동작함.

- 대표적으로 `ExecutionState`는 Idle로 복귀했지만 `CurrentActionType`은 이전 `ComboAttack` 상태로 남을 수 있었고, 이후 AI가 다시 공격을 시도하면 `NoExecutableAction` reject가 반복될 수 있었음.

- 이를 해결하기 위해 Reaction takeover 시 현재 active action을 먼저 abort하는 구조를 추가함.

- 동시에 combat availability 계산이 reaction 상태를 반영하도록 정리하여, reaction 이후 전투 복귀 흐름의 일관성을 높임.


---

## 환경

- Engine: Unreal Engine 5.4

- Branch:

  - `feature/action-orchestration`

- Related Code:

  - `Source/Portfolio/Component/CReactionComponent.cpp`
  - `Source/Portfolio/Component/CActionComponent.cpp`
  - `Source/Portfolio/AI/BehaviorTree/Service/CBTService_UpdateEngageContext.cpp`
  - `Source/Portfolio/AI/BehaviorTree/Task/CBTTask_StartCombatAction.cpp`

- Related Asset:

  - `Content/02_Controller/02_Enemy/AI/Blackboard/BB_Default.uasset`


---

## 재현 방법

1. AI가 `ComboAttack`을 수행하도록 함.

2. combo action 도중 플레이어가 AI를 타격하여 Reaction에 진입시킴.

3. Reaction이 종료된 뒤 AI가 다시 combat action을 시도하도록 함.

4. 이후 전투 흐름이 정상적으로 복구되는지 확인함.


---

## 기대 결과

- Reaction takeover 시 현재 진행 중인 action lifecycle이 정상적으로 정리되어야 함.

- Reaction 종료 후 `ExecutionState`, `CurrentActionType`, blackboard combat state가 서로 일관된 상태로 복귀해야 함.

- AI는 이후 다시 정상적으로 combat action을 시작할 수 있어야 함.


---

## 실제 결과

- Reaction은 종료되지만 이전 action 상태가 완전히 정리되지 않음.

- `ExecutionState`는 Idle로 돌아와도 `CurrentActionType`이 `ComboAttack`으로 남는 상태가 발생함.

- blackboard의 combat 가능 상태는 다시 열리지만 실제 action execution은 reject되어, AI가 멈추거나 비정상적으로 반복 시도하는 현상이 발생했음.


---

## 원인

- Reaction takeover 시 현재 active action을 중단시키는 명시적인 정리 단계가 없었음.

- 즉 Reaction은 state 전환은 수행했지만, action lifecycle abort까지 책임지지 못했음.

- 그 결과 ActionComponent, StateComponent, Blackboard가 서로 다른 상태를 바라보는 불일치가 발생했음.


---

## 수정

Reaction 시작 직전에 현재 action이 존재하면 먼저 abort하도록 구조를 추가함.

```cpp
if (UCActionComponent* actionComp = OwnerCharacter_Cached
	? OwnerCharacter_Cached->FindComponentByClass<UCActionComponent>()
	: nullptr)
{
	if (actionComp->GetCurrentActionType() != EActionType::Idle)
	{
		// Abort
		actionComp->AbortCurrentAction(EActionAbortReason::Reaction);
	}
}
```

추가로 다음 정리를 함께 반영함.

- reaction 상태를 combat availability 계산에 반영함
- combat action cooldown commit 책임을 `StartCombatAction` 쪽으로 정리함
- reaction 이후 blackboard와 실제 action state가 다시 엇갈리지 않도록 정리함


---

## 검증 결과

- AI가 combo action 중 피격되어 Reaction에 들어간 뒤에도, Reaction 종료 후 다시 정상적으로 전투 흐름을 이어갈 수 있음을 확인함.

- `ExecutionState`, `CurrentActionType`, combat blackboard state가 이전보다 일관되게 유지됨을 확인함.

- 이전에 발생하던 `NoExecutableAction` 반복 및 AI 정지 현상이 재현되지 않음을 확인함.


---

## 후속 정리

- Reaction takeover 정책은 이후 `ReactionOrchestrator` 작업에서 더 명확한 orchestration surface로 정리할 예정임.

- 장기적으로는 Action / Reaction takeover를 상위 coordination 구조에서 다룰 수 있도록 정리 여지를 남겨둠.


---

## Notes

- 이 수정의 핵심은 reaction이 state만 바꾸는 것이 아니라, 기존 action lifecycle도 함께 정리해야 한다는 점임.

- 즉 문제는 단순히 blackboard 값 하나가 아니라, ActionComponent / StateComponent / Blackboard의 상태 불일치였음.


---
