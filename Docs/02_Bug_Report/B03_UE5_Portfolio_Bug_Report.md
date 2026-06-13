# UE5 Portfolio Bug Report

## 제목

**B03: CBTService_UpdateAIContext 내부 early-return로 인한 상태 전환 지연 문제**

## 날짜

**2026.03.03**

## 상태

- [x] **완료**

---

## 브랜치

- `feature/ai-bt-context`

---

## 요약

- `UCBTService_UpdateAIContext::TickNode()`에서 컨텍스트 빌드 실패 시 즉시 `return`하던 흐름으로 인해 Blackboard 정리 로직이 누락되던 문제를 수정했다.

- 실패 상태를 `Success / NoData / Error`로 분리하고, 각 상태에서 Blackboard를 명시적으로 `Set/Clear`하도록 변경했다.

---

## 영향 범위

- AI Blackboard context 갱신

- 실패 상태 이후 target / patrol / combat context 정리 흐름

---

## 환경

- 엔진: Unreal Engine 5.4

- 대상: AI BehaviorTree Service (`UpdateAIContext`)

- 관련 키: `TargetActor`, `bHasLOS`, `LastKnownLocation`, `DistanceToTarget`, `bInRange`, `bReturnHome`

---

## 발생 조건

- `TickNode()`에서 context build가 실패했을 때 단순 early return으로 빠지면 발생한다.

- Blackboard 값을 성공 / NoData / Error 상태에 맞게 정리하지 않으면 재현된다.

---

## 재현 방법

1. AI가 타겟을 인지하여 Blackboard에 Target/Perception 정보가 세팅된 상태를 만든다.

2. Player를 시야 밖으로 이동시켜 Target 관련 정보가 소실되는 상황을 유도한다.

3. `BuildPerceptionContext()`가 실패 또는 무효 컨텍스트를 반환하는 프레임을 만든다.

4. 기존 코드에서 `TickNode()`가 early-return 되며 Blackboard 정리가 누락되는지 확인한다.

5. AI가 `Investigate`에서 `Idle`로 복귀하지 못하는 문제가 발생하는지 확인한다.

---

## 기대 결과 vs 실제 결과

**기대 결과**

- 타겟 로스트 시 Blackboard 키가 정책에 맞게 즉시 정리되어야 한다.

- 상태 전환이 `Investigate -> Idle`로 안정적으로 진행되어야 한다.

**실제 결과**

- early-return으로 인해 이전 프레임 데이터가 남아 상태 판단이 꼬였다.

- AI가 `Investigate`에서 `Idle`로 복귀하지 못했다.

---

## 원인

```cpp
void UCBTService_UpdateAIContext::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	// ...

	FAIContext aiContext;
	if (!BuildPerceptionContext(ownerPawn, aiContext))
		return; // Error Point
	if (!ComputeMetricContext(ownerPawn, blackboardComp, aiContext))
		return;

	UpdatePerceptionContext(blackboardComp, aiContext);
	UpdateCombatContext(blackboardComp, aiContext);
	UpdateNavigationContext(blackboardComp, aiContext);
}

bool UCBTService_UpdateAIContext::BuildPerceptionContext(APawn* InOwnerPawn, FAIContext& OutAIContext)
{
	// ...

	FTargetData topData;
	if (!aiController->BuildPerceptionContext(topData) || !topData.IsValidData())
		return false; // Error Point

	// ...
}

bool ACAIController::BuildPerceptionContext(FTargetData& OutTargetData)
{
	UpdateTargetDataMap();
	return SelectTopPriority(OutTargetData); // Error Point
}

bool ACAIController::SelectTopPriority(FTargetData& OutTargetData)
{
	// ...

	FTargetData topData;
	for (TPair<AActor*, FTargetData>& pair : TargetDataMap)
	{
		AActor* actorKey = pair.Key;
		FTargetData& data = pair.Value;

		if (!IsValid(actorKey) || !data.IsValidData()) continue;

		if (data.TargetPriority < bestPriority)
		{
			bestPriority = data.TargetPriority;
			topData = data;
			continue;
		}
	}

	if (bestPriority == INT_MAX || !topData.IsValidData())
		return false; // Error Point

	// ...
}
```

- `ACAIController::SelectTopPriority`에서 `TargetDataMap`에 데이터가 없는 경우에도 `false`를 반환했다.

- `UCBTService_UpdateAIContext::TickNode`에서 예외 상황에 대응하기 위해 `early return`을 사용했으나, 위 조건과 결합되면서 Target이 소실된 것으로 판단해야 하는 상황에서도 기존 Blackboard 값이 정리되지 않는 버그가 발생했다.

- 그 결과 AI가 `Investigate`에서 `Idle`로 복귀하지 못했다.

---

## 수정

1. 컨텍스트 빌드 결과 타입을 도입한다.
	- `Success`
	- `NoData` (정상적인 타겟 부재)
	- `Error` (비정상 오류)

2. `TickNode()` 분기 정책을 변경한다.
	- `Error`:
	  `ClearPerceptionContext`, `ClearCombatContext`, `ClearNavigationContext`
	- `NoData`:
	  `ClearPerceptionContext`, `ClearCombatContext`, `UpdateNavigationContext_NoTarget`
	- `Success`:
	  `UpdatePerceptionContext`, `UpdateCombatContext`, `UpdateNavigationContext`

3. 실패 시에도 Blackboard 정리/갱신이 반드시 실행되도록 보장한다.

---

## 수정 기준

- context build 실패를 단순 early return으로 처리하지 않는다.

- `Success`, `NoData`, `Error` 결과에 따라 Blackboard 값을 명시적으로 유지하거나 정리한다.

---

## 검증 결과

1. 대상 인식 -> 해제 시나리오 반복 테스트

2. Blackboard Debug 확인:
	- `TargetActor` 클리어
	- `bHasLOS` false 전환
	- `DistanceToTarget`, `bInRange` 정리

3. 상태 전환 확인:
	- `Investigate` 진입 후 조건 만료 시 `Idle`로 정상 복귀하는 것을 확인했다.

4. 회귀 확인:
	- 타겟 재인식 시 `Chase/Combat` 재진입이 정상 동작하는 것을 확인했다.

---

## 회귀 방지 기준

- context build 실패 뒤 stale Blackboard value가 남지 않아야 한다.

- AI state transition이 이전 frame 값에 의존하지 않아야 한다.

---

## 관련 PR / 문서

- Issue Checklist: `D11_UE5_Portfolio_Issue_Checklist.md`

- PR: `P10_UE5_Portfolio_Pull_Request.md`

- Portfolio Technical Document: `T04_Enemy AI Combat Behavior Design.md`

---

## 비고

- AI 서비스에서는 “실패=즉시 return”보다 “실패 타입 분기 + 명시적 Set/Clear”가 안전하다.

- Blackboard 입력 키는 주기마다 결정적으로 갱신해 잔여 데이터가 남지 않게 해야 한다.

---
