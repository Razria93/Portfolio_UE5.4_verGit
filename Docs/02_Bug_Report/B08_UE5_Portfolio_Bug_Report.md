# UE5 Portfolio Bug Report

## 제목

**B08: AI가 combo action 도중 피격되면 reaction 이후 전투흐름이 비정상적으로 동작하는 문제**

## 날짜

**2026.04.29**

## 상태

- [x] **완료**

---

## 브랜치

- `feature/action-orchestration`

---

## 요약

- AI가 `ComboAttack` 수행 중 피격당해 Reaction에 진입한 뒤, Reaction 종료 후 action / state / blackboard 상태가 어긋나 이후 전투 흐름이 비정상적으로 동작했다.

- 대표적으로 `ExecutionState`는 Idle로 복귀했지만 `CurrentActionType`은 이전 `ComboAttack` 상태로 남을 수 있었고, 이후 AI가 다시 공격을 시도하면 `NoExecutableAction` reject가 반복될 수 있었다.

- 이를 해결하기 위해 Reaction takeover 시 현재 active action을 먼저 abort하는 구조를 추가했다.

- 동시에 combat availability 계산이 reaction 상태를 반영하도록 정리하여, reaction 이후 전투 복귀 흐름의 일관성을 높였다.

---

## 영향 범위

- AI action / reaction lifecycle 정리

- hit reaction takeover 이후 Enemy combat request 복구 흐름

---

## 환경

- 엔진: Unreal Engine 5.4

- 관련 코드:
	- `Source/Portfolio/Component/CReactionComponent.cpp`
	- `Source/Portfolio/Component/CActionComponent.cpp`
	- `Source/Portfolio/AI/BehaviorTree/Service/CBTService_UpdateEngageContext.cpp`
	- `Source/Portfolio/AI/BehaviorTree/Task/CBTTask_StartCombatAction.cpp`

- 관련 에셋:
	- `Content/02_Controller/02_Enemy/AI/Blackboard/BB_Default.uasset`

---

## 발생 조건

- AI ComboAttack 도중 hit reaction이 active action cleanup 없이 takeover하면 발생한다.

- ExecutionState는 Idle로 보이지만 CurrentActionType이 남아 있으면 재현된다.

---

## 재현 방법

1. AI가 `ComboAttack`을 수행하도록 한다.

2. combo action 도중 플레이어가 AI를 타격하여 Reaction에 진입시킴.

3. Reaction이 종료된 뒤 AI가 다시 combat action을 시도하도록 한다.

4. 이후 전투 흐름이 정상적으로 복구되는지 확인한다.

---

## 기대 결과 vs 실제 결과

**기대 결과**

- Reaction takeover 시 현재 진행 중인 action lifecycle이 정상적으로 정리되어야 한다.

- Reaction 종료 후 `ExecutionState`, `CurrentActionType`, blackboard combat state가 서로 일관된 상태로 복귀해야 한다.

- AI는 이후 다시 정상적으로 combat action을 시작할 수 있어야 한다.

**실제 결과**

- Reaction은 종료되지만 이전 action 상태가 완전히 정리되지 않았다.

- `ExecutionState`는 Idle로 돌아와도 `CurrentActionType`이 `ComboAttack`으로 남는 상태가 발생했다.

- blackboard의 combat 가능 상태는 다시 열리지만 실제 action execution은 reject되어, AI가 멈추거나 비정상적으로 반복 시도하는 현상이 발생했다.

---

## 원인

- Reaction takeover 시 현재 active action을 중단시키는 명시적인 정리 단계가 없었다.

- Reaction은 state 전환은 수행했지만, action lifecycle abort까지 책임지지 못했다.

- 그 결과 ActionComponent, StateComponent, Blackboard가 서로 다른 상태를 바라보는 불일치가 발생했다.

---

## 수정

Reaction 시작 직전에 현재 action이 존재하면 먼저 abort하도록 구조를 추가했다.

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

추가로 다음 정리를 함께 반영했다.

- reaction 상태를 combat availability 계산에 반영했다.

- combat action cooldown commit 책임을 `StartCombatAction` 쪽으로 정리했다.

- reaction 이후 blackboard와 실제 action state가 다시 엇갈리지 않도록 정리했다.

---

## 수정 기준

- reaction takeover 전에 active action을 abort하고 runtime state를 정리한다.

- ActionComponent, StateComponent, Blackboard cleanup을 같은 전환 단위에서 수행한다.

---

## 검증 결과

- AI가 combo action 중 피격되어 Reaction에 들어간 뒤에도, Reaction 종료 후 다시 정상적으로 전투 흐름을 이어갈 수 있는 것을 확인했다.

- `ExecutionState`, `CurrentActionType`, combat blackboard state가 이전보다 일관되게 유지됨을 확인했다.

- 이전에 발생하던 `NoExecutableAction` 반복 및 AI 정지 현상이 재현되지 않는 것을 확인했다.

---

## 회귀 방지 기준

- hit reaction 이후 `NoExecutableAction` 반복이 발생하지 않아야 한다.

- AI가 reaction 종료 뒤 정상적으로 다시 attack action을 실행해야 한다.

---

## 관련 PR / 문서

- Issue Checklist: `D16_UE5_Portfolio_Issue_Checklist.md`

- PR: `P15_UE5_Portfolio_Pull_Request.md`

- Portfolio Technical Document: `T03_Action & Reaction Execution Pipeline.md`

---

## 비고

- 이후 `UCReactionOrchestratorComponent`가 추가되면서 reaction request 판단과 실행 조율 책임은 orchestration surface로 분리되었다.

- 장기적으로는 Action / Reaction takeover를 공통 coordination 구조에서 다룰 수 있도록 정리 여지를 남겨둔다.

---
