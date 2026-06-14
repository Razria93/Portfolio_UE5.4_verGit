# Reaction Local Level과 Orchestration Level 역할 분리

## 1. 목적

본 문서는 reaction orchestration 구조에서 `CReaction`의 로컬 실행 상태/규칙과 `FReactionExecutionPolicy`의 오케스트레이션 레벨 정책이 어떤 의미를 갖는지 정리하기 위한 architecture decision 문서임.

archive 문서 `A05`, `A07`의 내용을 하나의 흐름으로 재구성함.

핵심은 `FReactionExecutionPolicy`가 executor의 고정 초기 설정이 아니라, request 시점의 body/runtime/context를 반영해 orchestrator가 계산한 resolved policy라는 점을 명확히 하는 것임.

---

## 2. 배경

Reaction orchestration에서 다음 의문이 생길 수 있음.

```text
incoming reaction이 active reaction을 interrupt할 수 있는지는
CReaction::WantToInterrupt()
CReaction::AllowInterruptionBy()
에서 이미 판단하는데,
왜 FReactionExecutionPolicy가 필요한가?
```

현재 1차 구현에서 policy가 얇기 때문에 이 의문은 타당함.

그러나 `CReaction` hook과 `FReactionExecutionPolicy`는 같은 축의 값이 아님.

```text
CReaction local state / rule
-> 현재 실행 중인 reaction executor 내부의 상태와 규칙임

FReactionExecutionPolicy
-> 현재 request가 현재 body/runtime/context에서 갖는 orchestration-level 권한 해석값임
```

즉 `CReaction`은 "현재 실행 객체가 지금 허용하는가"를 답하고, `FReactionExecutionPolicy`는 "이번 request가 현재 캐릭터 상태에서 어떤 권한을 갖는가"를 표현함.

최종 decision은 둘 중 하나가 단독으로 내리지 않음.

Orchestrator가 policy와 executor local rule을 함께 확인한 뒤 `Start / Interrupt / Ignore / Reject`를 결정함.

---

## 3. 로컬 실행 상태와 규칙

`CReaction`은 실제 reaction executor임.

따라서 `CReaction`이 가진 상태값은 현재 실행 중인 reaction object의 local runtime state임.

예시는 다음과 같음.

```text
bIsReaction
bInterruptible
bCancelable
ActiveReactionMontage_Cached
ActiveReactionData_Cached
ActiveReactionMontage section
Anim notify window
```

이 값들은 montage lifecycle과 anim notify window에 의해 바뀜.

따라서 이 값은 외부 body state 전체를 설명하지 않음.

이 값은 "현재 이 reaction executor가 자기 실행 구간 안에서 어떤 제어를 허용하는가"를 설명함.

`CReaction` hook은 이 local runtime state를 기반으로 orchestrator의 질문에 답하는 API임.

```cpp
WantToInterrupt(const FReactionQueryContext& InContext)
AllowInterruptionBy(const FReactionQueryContext& InContext)
WantToCancel(const FReactionQueryContext& InContext)
AllowCancelBy(const FReactionQueryContext& InContext)
```

즉 executor hook은 다음 질문에 답함.

```text
incoming reaction executor는 현재 active reaction을 interrupt하고 싶은가
current reaction executor는 지금 interruption을 허용하는가

incoming reaction executor는 현재 active reaction을 cancel하고 싶은가
current reaction executor는 지금 cancel을 허용하는가
```

예시는 다음과 같음.

```text
Hit reaction의 interruptible window가 열려 있음
-> AllowInterruptionBy()가 true를 반환할 수 있음

Hit reaction의 recovery section에 진입함
-> AllowInterruptionBy()가 false를 반환할 수 있음

Dead reaction executor임
-> WantToInterrupt()가 true를 반환할 수 있음

특정 reaction이 같은 hit reaction에 의해 다시 끊기고 싶지 않음
-> WantToInterrupt() 또는 AllowInterruptionBy()가 false를 반환할 수 있음
```

이 판단은 실행 객체 내부의 window / montage phase / reaction-specific rule에 가까움.

따라서 `CReaction`은 최종 orchestration decision을 내리는 주체가 아니라, orchestrator가 decision을 내리기 위해 필요한 executor-local 판단 근거를 제공하는 주체임.

---

## 4. 해석된 정책

`FReactionExecutionPolicy`는 executor의 local state가 아님.

이는 orchestrator가 현재 request에 대해 계산한 orchestration-level policy임.

따라서 policy는 "실행 객체가 지금 어떤 window에 있는가"를 의미하지 않음.

Policy는 "이번 incoming reaction request가 현재 캐릭터의 body/runtime/context에서 어떤 권한을 갖는가"를 의미함.

현재 구조는 다음 필드를 가짐.

```cpp
struct FReactionExecutionPolicy
{
	bool bCanInterrupt = false;
	bool bForceInterrupt = false;
	bool bIgnoreInterruptWindow = false;
	int32 Priority = 0;
};
```

이 값은 request 시점마다 새로 계산됨.

반영할 수 있는 정보는 다음과 같음.

```text
incoming reaction type
incoming reaction data priority
current active reaction
dead state transition
super armor
guard / guard break
poise
external hit resolution result
character trait / buff
```

현재 1차 구현에서는 대부분 reaction data priority와 dead reaction special case를 중심으로 얇게 사용함.

예시는 다음과 같음.

```text
incoming reaction이 Dead임
-> active reaction의 local interrupt window와 무관하게 강제 interrupt 권한을 부여함

현재 캐릭터가 super armor 상태임
-> incoming hit reaction executor가 interrupt를 원해도 interrupt 권한을 제거함

현재 guard break 상태임
-> 일반 hit reaction보다 높은 priority와 window 무시 권한을 부여할 수 있음

현재 poise가 충분함
-> hit reaction request를 Ignore로 처리할 수 있도록 policy를 제한할 수 있음
```

즉 policy는 executor-local rule보다 상위의 캐릭터 상태 / 전투 상태 / 데미지 해석 결과를 반영함.

---

## 5. 결정 사항

`FReactionExecutionPolicy`를 유지함.

다만 현재 단계에서는 얇은 resolved policy로 유지함.

최종 interrupt 가능 여부는 policy 단독으로 결정하지 않음.

현재 판단 순서는 다음과 같음.

```text
1. incoming context가 유효한지 확인함
2. current active context가 있는지 확인함
3. incoming policy의 bCanInterrupt를 확인함
4. policy priority와 current reaction priority를 비교함
5. bIgnoreInterruptWindow가 아니면 current executor의 AllowInterruptionBy()를 확인함
6. bForceInterrupt가 아니면 incoming executor의 WantToInterrupt()를 확인함
7. 최종 decision을 Start / Interrupt / Ignore / Reject 중 하나로 정리함
```

즉 policy는 executor hook을 대체하지 않음.

Policy는 executor hook에 도달하기 전에 request가 어떤 상위 권한을 갖는지 정리함.

Executor local rule은 policy가 허용한 후보가 실제 executor 내부 상태에서도 허용되는지 확인함.

이 둘을 종합해 최종 decision을 내리는 주체는 `ReactionOrchestrator`임.

---

## 6. 예시

### 사망 리액션

Dead reaction은 일반 hit reaction보다 상위 body state transition임.

따라서 active reaction의 interruptible window가 닫혀 있어도 force interrupt가 필요함.

```cpp
policy.bCanInterrupt = true;
policy.bForceInterrupt = true;
policy.bIgnoreInterruptWindow = true;
policy.Priority = TNumericLimits<int32>::Max();
```

이 경우 orchestrator는 current executor의 `AllowInterruptionBy()`가 false여도 policy에 의해 window 검사를 무시할 수 있음.

즉 dead override는 executor-local window보다 상위의 orchestration policy임.

### 슈퍼아머

현재 body state가 super armor라면 incoming hit reaction executor가 interrupt를 원해도 orchestration 단계에서 막을 수 있음.

```cpp
policy.bCanInterrupt = false;
```

이 경우 incoming executor의 `WantToInterrupt()`가 true여도 orchestrator는 local rule까지 내려가지 않고 reject 또는 ignore할 수 있음.

즉 super armor는 executor-local 의사보다 상위에서 incoming reaction의 권한을 제한하는 policy임.

### 가드 브레이크

guard break 상태라면 특정 stagger reaction은 일반 interrupt window를 무시하고 들어갈 수 있음.

```cpp
policy.bCanInterrupt = true;
policy.bForceInterrupt = true;
policy.bIgnoreInterruptWindow = true;
```

이 경우 current executor가 아직 interruptible window를 열지 않았더라도, guard break라는 body/runtime state가 incoming reaction에 더 강한 권한을 부여함.

### 일반 피격 리액션

일반 hit reaction은 policy에서 별도 강제 권한을 받지 않을 수 있음.

```cpp
policy.bCanInterrupt = true;
policy.bForceInterrupt = false;
policy.bIgnoreInterruptWindow = false;
policy.Priority = InContext.ReactionData.Priority;
```

이 경우 current executor의 `AllowInterruptionBy()`와 incoming executor의 `WantToInterrupt()`가 모두 중요함.

즉 일반 hit reaction은 orchestration policy와 executor-local rule이 모두 허용해야 interrupt decision으로 이어짐.

---

## 7. 현재 구현

현재 구현 기준은 다음과 같음.

```text
ResolveReactionPolicy()
-> ReactionData.Priority를 policy.Priority로 설정함
-> 기본 bCanInterrupt를 true로 둠
-> Dead reaction이면 force interrupt / ignore interrupt window / max priority를 설정함

CanInterruptActiveReaction()
-> policy와 priority를 먼저 확인함
-> CReaction local hook을 확인함
-> 최종 interrupt 가능 여부를 반환함
```

현재 policy는 얇지만, guard / parry / poise / super armor가 들어오면 중요한 확장 지점이 됨.

---

## 8. 결과

이 결정의 장점은 다음과 같음.

```text
executor local rule과 orchestration-level policy가 분리됨
Dead reaction 같은 상위 상태를 executor hook보다 위에서 처리할 수 있음
super armor / guard / poise 같은 body state 기반 정책을 추가할 공간이 생김
현재 구현은 얇게 유지하면서도 확장 지점을 보존함
```

주의할 점은 다음과 같음.

```text
policy가 얇은 동안에는 executor hook과 중복처럼 보일 수 있음
bCanInterrupt는 최종 interrupt 가능 여부가 아니라 interrupt 후보로 평가할 수 있는 권한에 가까움
force / ignore window 정책은 실제 body state 확장과 함께 의미가 더 분명해짐
```

---

## 9. 후속 작업

후속 작업 후보는 다음과 같음.

```text
guard / parry / dodge / counter 결과를 reaction policy에 반영함
super armor / poise / guard break 상태를 ResolveReactionPolicy에 연결함
cancel policy를 별도로 정의함
action orchestration에도 유사한 resolved policy 계층이 필요한지 검토함
```

---

## 10. 관련 문서

관련 상세 문서는 다음과 같음.

```text
A05_UE5_Portfolio_Reaction_Execution_Policy_Model
A07_UE5_Portfolio_Reaction_Lifecycle_Model
```

---
