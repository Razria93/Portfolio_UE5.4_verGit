# UE5 Portfolio Issue Analysis Report

## 제목

**I01: OnTargetPerceptionForgotten 이벤트 미호출 이슈 분석**

## 날짜

**2026.01.28**

## 상태

- [x] **완료**

---

## 브랜치

- `feature/ai-behaviortree-core`

---

## 요약

- `OnTargetPerceptionForgotten` 델리게이트가 호출되지 않는 현상을 분석하고, 프로젝트 설정에서 `bForgetStaleActors`를 활성화하여 정상 호출을 확인했다.


---

## 재현 절차

1. `ACAIController::InitializePerception()`에서 `OnTargetPerceptionForgotten` 델리게이트를 바인딩한다.
   
2. AI Perception의 Stimulus가 **Lost** 상태로 전환된다.
   
3. 일정 시간 경과 후 Stimulus **Age**가 Expired 되는 상황을 기다린다.


---

## 기대 동작 vs 실제 동작

**기대 동작**
- Stimulus가 Expired 되어 Forget 되었을 때 `PrintTargetPerceptionForgotten()`가 호출되어 로그가 출력되어야 한다.

**실제 동작**
- `OnTargetPerceptionForgotten`가 호출되지 않고, `Updated` 이벤트의 **Gained/Lost** 로그만 출력됐다.


---

## 이슈 코드

```cpp
bool ACAIController::InitializePerception()
{
	if (!IsValid(AIPerceptionComp)) return false;

	AIPerceptionComp->OnTargetPerceptionForgotten.AddDynamic(
	this,
	&ACAIController::OnTargetPerceptionForgotten);

	return true;
}

void ACAIController::OnTargetPerceptionForgotten(AActor* Actor)
{
	PrintTargetPerceptionForgotten(Actor);
}

void ACAIController::PrintTargetPerceptionForgotten(AActor* Actor) const
{
	FLog::Log(TEXT("== Target Perception Forgotten =="));

	FLog::Log(FString::Printf(TEXT("TargetActor = %s"), *GetNameSafe(Actor)));

	FLog::Log(TEXT("================================="));
}
```


---

## 실행 결과

```cpp
Custom_FLog: Display: === Target Perception Updated ===
Custom_FLog: Display: TargetActor = BP_CPlayer_C_0
Custom_FLog: Display: Sense = AISense_Sight | Perceived = Gained | Age = 0.00
Custom_FLog: Display: =================================
Custom_FLog: Display: ====== Perception Updated =======
Custom_FLog: Display: -------- Updated Actors ---------
Custom_FLog: Display: UpdateActors        : 1
Custom_FLog: Display: - BP_CPlayer_C_0
Custom_FLog: Display: =================================
Custom_FLog: Display: === Target Perception Updated ===
Custom_FLog: Display: TargetActor = BP_CPlayer_C_0
Custom_FLog: Display: Sense = AISense_Sight | Perceived = Lost | Age = 0.00
Custom_FLog: Display: =================================
Custom_FLog: Display: ====== Perception Updated =======
Custom_FLog: Display: -------- Updated Actors ---------
Custom_FLog: Display: UpdateActors        : 1
Custom_FLog: Display: - BP_CPlayer_C_0
Custom_FLog: Display: =================================
```


---

## 원인 분석

- `OnTargetPerceptionForgotten` 호출 여부는 `UAIPerceptionComponent` 내부 변수인 `bForgetStaleActors` 값에 의해 결정된다.
  
- `bForgetStaleActors`는 `UAIPerceptionComponent` 내부 **private 변수**로, 개별 컴포넌트에서 직접 설정할 수 없다.
  
- 해당 값은 프로젝트 단위 정책으로 `AIModule.AISystem`에서 설정해야 한다.


---

## 해결 방법

프로젝트 설정 파일에 아래 값을 추가/수정하여 `bForgetStaleActors`를 활성화한다.

```ini
[/Script/AIModule.AISystem]
bForgetStaleActors=True
```


---

## 실행 결과 (해결 확인)

```cpp
Custom_FLog: Display: === Target Perception Updated ===
Custom_FLog: Display: TargetActor = BP_CPlayer_C_0
Custom_FLog: Display: Sense = AISense_Sight | Perceived = Gained | Age = 0.00
Custom_FLog: Display: =================================
Custom_FLog: Display: ====== Perception Updated =======
Custom_FLog: Display: -------- Updated Actors ---------
Custom_FLog: Display: UpdateActors        : 1
Custom_FLog: Display: - BP_CPlayer_C_0
Custom_FLog: Display: =================================
Custom_FLog: Display: === Target Perception Updated ===
Custom_FLog: Display: TargetActor = BP_CPlayer_C_0
Custom_FLog: Display: Sense = AISense_Sight | Perceived = Lost | Age = 0.00
Custom_FLog: Display: =================================
Custom_FLog: Display: ====== Perception Updated =======
Custom_FLog: Display: -------- Updated Actors ---------
Custom_FLog: Display: UpdateActors        : 1
Custom_FLog: Display: - BP_CPlayer_C_0
Custom_FLog: Display: =================================
Custom_FLog: Display: == Target Perception Forgotten ==
Custom_FLog: Display: TargetActor = BP_CPlayer_C_0
Custom_FLog: Display: =================================
```


---

## 결론

- `OnTargetPerceptionForgotten`는 **Stimulus 만료 처리 정책(`bForgetStaleActors`)이 활성화되어야만** 호출된다.
  
- 해당 정책은 컴포넌트별 설정이 아니라 프로젝트 단위 설정이므로, 추후 유사 문제에 대비해 **AIModule.AISystem** 설정을 확인해야 한다.


---

## 참고

- 관련 기능 변경 커밋: 
	1. `chore(ai-behaviortree-core): turn on bForgetStaleActors in AISystem settings (#26, #27)`
	   
- 관련 리팩터링 커밋: 
	1. `refactor(ai-behaviortree-core): rename BP_CAIController_Melee to BP_CAIController (#26)`
	   
	2. `feat(ai-behaviortree-core): add debug print functions (#26)`
	   
	3. `refactor(ai-behaviortree-core): bind perception delegates without handler implementation (#26)`


---
