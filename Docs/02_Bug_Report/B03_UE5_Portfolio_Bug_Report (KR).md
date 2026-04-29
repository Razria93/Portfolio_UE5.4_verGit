# UE5 Portfolio – Bug Report (KR)

## 제목

**M03-B01: UpdateAIContext early-return로 인한 상태 전환 지연 수정**

### Date

- **Day 11**
  
- **2026.03.03**

### Type

- Bug

### Status

- [x] Resolved

### Branch

- `feature/ai-bt-context`


---

## 요약

- `UCBTService_UpdateAIContext::TickNode()`에서 컨텍스트 빌드 실패 시 즉시 `return`하던 흐름으로 인해 Blackboard 정리 로직이 누락되던 문제를 수정함.
  
- 실패 상태를 `Success / NoData / Error`로 분리하고, 각 상태에서 Blackboard를 명시적으로 `Set/Clear`하도록 변경함.
  

---

## 환경

- Engine: Unreal Engine 5.4
  
- 대상: AI BehaviorTree Service (`UpdateAIContext`)
  
- 관련 키: `TargetActor`, `bHasLOS`, `LastKnownLocation`, `DistanceToTarget`, `bInRange`, `bReturnHome`


---

## 재현 방법

1. AI가 타겟을 인지하여 Blackboard에 타겟/시야 정보가 세팅된 상태를 만듦.
   
2. 플레이어를 시야 밖으로 이동시켜 타겟 로스트를 유도함.
   
3. `BuildPerceptionContext()`가 실패 또는 무효 컨텍스트를 반환하는 프레임을 만듦.
   
4. 기존 코드에서 `TickNode()`가 early-return 되며 Blackboard 정리가 누락되는지 확인함.
   
5. AI가 `Investigate`에서 `Idle` 로 복귀하지 못하는 문제가 발생함을 확인함.


---

## 기대 결과 vs 실제 결과

**기대 결과**

- 타겟 로스트 시 Blackboard 키가 정책에 맞게 즉시 정리됨.
  
- 상태 전환이 `Investigate -> Idle`로 안정적으로 진행됨.

**실제 결과**

- early-return으로 인해 이전 프레임 데이터가 남아 상태 판단이 꼬임.
  
- AI가 `Investigate`에서 `Idle` 로 복귀하지 못함.


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

- `ACAIController::SelectTopPriority` 에서 `TargetDataMap`에 데이터가 없는 경우에도 `false`을 반환함

- `UCBTService_UpdateAIContext::TickNode`에서 예외 상황에서 대응하기 위해 `earlyreturn` 사용했으나 이 부분과 위의 요소가 결합되어 `Target`이 `lost` 되었음에도 `forget` 되지 않는 버그가 발생함.
  
- 그 결과 AI가 `Investigate`에서 `Idle`로 복귀하지 못게 됨.


---

## 수정

1. 컨텍스트 빌드 결과 타입 도입:
   
	- `Success`
     
	- `NoData` (정상적인 타겟 부재)
     
	- `Error` (비정상 오류)
	
2. `TickNode()` 분기 정책 변경:
	   
	- `Error`: 
	  `ClearPerceptionContext`, `ClearCombatContext`, `ClearNavigationContext`
     
	- `NoData`: 
	  `ClearPerceptionContext`, `ClearCombatContext`, `UpdateNavigationContext_NoTarget`
	
	- `Success`: 
	  `UpdatePerceptionContext`, `UpdateCombatContext`, `UpdateNavigationContext`
	
3. 실패 시에도 Blackboard 정리/갱신이 반드시 실행되도록 보장.


---

## 검증

1. 대상 인식 -> 해제 시나리오 반복 테스트
   
2. Blackboard Debug 확인:
   
	- `TargetActor` 클리어
     
	- `bHasLOS` false 전환
	  
	- `DistanceToTarget`, `bInRange` 정리
	  
3. 상태 전환 확인:
   
	- `Investigate` 진입 후 조건 만료 시 `Idle` 정상 복귀
	  
4. 회귀 확인:
	   
	- 타겟 재인식 시 `Chase/Combat` 재진입 정상 동작


---

## Notes

- AI 서비스에서는 “실패=즉시 return”보다 “실패 타입 분기 + 명시적 Set/Clear”가 안전해야함.
  
- Blackboard 입력 키는 주기마다 결정적으로 갱신해 잔여 데이터가 남지 않게 해야 함.


---