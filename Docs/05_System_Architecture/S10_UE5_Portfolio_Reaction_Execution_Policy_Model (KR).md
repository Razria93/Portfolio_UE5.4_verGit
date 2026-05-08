# Reaction Execution Policy 모델

## 1. 목적

본 문서는 `CReaction`이 가진 실행 상태값과 `ResolveReactionPolicy()`가 만드는 policy 상태값의 의미 차이를 정리하기 위한 설계 문서임.

핵심 목적은 다음과 같음.

- `CReaction` hook은 execution object의 local rule임을 명확히 함.
- `FReactionExecutionPolicy`는 실행기의 고정 초기 정책이 아니라 request 시점의 resolved policy임을 명확히 함.
- 두 상태값이 서로 중복되는 것이 아니라 서로 다른 계층의 판단 근거임을 정리함.
- 현재는 얇은 구조로 유지하되, 향후 body state / hit resolution / guard / poise 확장 시 policy가 어떤 의미를 갖는지 기록함.

---

## 2. 핵심 쟁점

Reaction Orchestration에서 다음 의문이 생길 수 있음.

```text
incoming reaction이 active reaction을 interrupt할 수 있는지는
CReaction::WantToInterrupt()와 CReaction::AllowInterruptionBy()에서 이미 판단하는데,
왜 ResolveReactionPolicy()와 FReactionExecutionPolicy가 따로 필요한가?
```

이 의문은 현재 1차 구현 기준에서는 타당함.

현재 `FReactionExecutionPolicy`가 다음처럼 얇다면,  
`CReaction` hook과 역할이 겹쳐 보일 수 있음.

```cpp
struct FReactionExecutionPolicy
{
	bool bCanInterrupt = false;
	bool bForceInterrupt = false;
	bool bIgnoreInterruptWindow = false;
	int32 Priority = 0;
};
```

그러나 두 값은 같은 질문에 답하는 것이 아님.

```text
CReaction hook은 실행 객체 자신이 말하는 local rule임
FReactionExecutionPolicy는 orchestrator가 현재 request에 대해 해석한 resolved policy임
```


---

## 3. CReaction의 상태값과 Hook

`CReaction`은 실제 reaction execution object임.

따라서 `CReaction`이 가진 상태값은 현재 실행 중인 reaction object의 local runtime state를 의미함.

예시는 다음과 같음.

```cpp
bool bIsActive;
bool bInterruptible;
bool bCancelable;
UAnimMontage* ActiveMontage_Cached;
```

이 값들은 주로 montage lifecycle과 anim notify window에 의해 바뀜.

```text
reaction montage가 시작되면 active가 됨
interruptible notify window가 열리면 bInterruptible이 true가 됨
cancelable notify window가 열리면 bCancelable이 true가 됨
montage가 끝나거나 stop되면 상태가 clear됨
```

`CReaction` hook도 이 local runtime state를 기반으로 판단함.

```cpp
CReaction::WantToInterrupt()
CReaction::AllowInterruptionBy()
CReaction::WantToCancel()
CReaction::AllowCancelBy()
```

즉 `CReaction` hook은 다음 질문에 답함.

```text
이 reaction executor는 지금 interrupt를 허용하는가?
이 incoming reaction executor는 interrupt를 원하고 있는가?
이 reaction executor는 지금 cancel을 허용하는가?
```

따라서 `CReaction` hook은 실행 객체 내부의 규칙임.


---

## 4. ResolveReactionPolicy의 상태값

`ResolveReactionPolicy()`는 `CReaction` 실행기의 초기 설정을 반환하는 함수가 아님.

`ResolveReactionPolicy()`는 현재 request가 현재 body/runtime/context 안에서 어떤 실행 권한을 갖는지 해석하는 단계임.

즉 `FReactionExecutionPolicy`는 다음 의미에 가까움.

```text
이번 incoming reaction request는
현재 캐릭터 상태에서
어떤 방식으로 처리될 수 있는가
```

이 policy는 request 시점마다 새로 계산되는 resolved policy임.

정책을 만들 때 반영할 수 있는 정보는 다음과 같음.

```text
현재 body state
현재 active reaction
incoming damage / reaction context
dead state
super armor 상태
guard / guard break 상태
poise 상태
external hit resolution result
character trait / buff
```

따라서 `FReactionExecutionPolicy`는 reaction data의 원본 설정도 아니고,  
`CReaction` executor의 local 상태값도 아님.

`FReactionExecutionPolicy`는 orchestrator가 이번 request에 대해 만든 orchestration-level 권한 해석값임.


---

## 5. 역할 차이

두 계층의 차이는 다음과 같음.

```text
CReaction hook
- reaction executor 자체가 가진 local 실행 규칙임
- montage window, interruptible, cancelable 같은 executor 내부 상태 중심임
- execution object가 지금 무엇을 허용하는지 말함

FReactionExecutionPolicy
- 현재 request가 현재 캐릭터 상태에서 어떤 권한을 갖는지 나타냄
- body/runtime/context를 종합한 orchestration-level policy임
- orchestrator가 이번 request를 어떤 권한으로 평가할지 말함
```

즉 `CReaction` hook은 실행 객체의 내부 판단이고,  
`FReactionExecutionPolicy`는 request와 body state를 함께 본 상위 판단임.


---

## 6. 예시

### Dead Reaction

현재 active reaction이 interruptible window가 아닐 수 있음.

그러나 incoming reaction이 `Dead`라면, 죽음은 일반 hit reaction보다 상위 상태임.

이 경우 policy는 다음 의미를 가질 수 있음.

```cpp
policy.bForceInterrupt = true;
policy.bIgnoreInterruptWindow = true;
```

즉 executor hook보다 상위에서 강제 interrupt가 가능함.

### Super Armor

현재 캐릭터가 super armor 상태일 수 있음.

incoming hit reaction executor는 interrupt를 원할 수 있음.

그러나 body state가 super armor라면 orchestrator policy는 incoming hit reaction을 interrupt 후보로 올리지 않을 수 있음.

```cpp
policy.bCanInterrupt = false;
```

즉 executor가 interrupt를 원해도 orchestration 단계에서 막을 수 있음.

### Guard Break

현재 guard break가 발생한 상태라면 특정 stagger reaction은 강제로 들어와야 할 수 있음.

이 경우 policy는 다음 의미를 가질 수 있음.

```cpp
policy.bForceInterrupt = true;
policy.bIgnoreInterruptWindow = true;
```

즉 일반 hit reaction과 다른 권한으로 incoming reaction을 처리할 수 있음.


---

## 7. 현재 구현 기준

현재 1차 구현에서는 아직 body state, super armor, guard, poise, hit resolution result가 충분히 들어오지 않았음.

따라서 `FReactionExecutionPolicy`는 얇게 유지될 수 있음.

```cpp
struct FReactionExecutionPolicy
{
	bool bCanInterrupt = false;
	bool bForceInterrupt = false;
	bool bIgnoreInterruptWindow = false;
	int32 Priority = 0;
};
```

이 단계에서 `bCanInterrupt`는 최종 interrupt 가능 여부를 단독으로 결정하지 않음.

현재 의미는 다음처럼 제한하는 것이 적절함.

```text
orchestrator가 이번 request를 active reaction interrupt 후보로 평가할 수 있는가
```

최종 판단은 다음 순서로 이루어짐.

```text
1. resolved policy를 확인함
2. priority를 비교함
3. current CReaction::AllowInterruptionBy()를 확인함
4. incoming CReaction::WantToInterrupt()를 확인함
```

즉 policy는 `CReaction` hook을 대체하지 않음.  
policy는 `CReaction` hook에 도달하기 전에 request의 상위 처리 권한을 정리하는 값임.


---

## 8. 결론

`CReaction`의 상태값과 hook은 execution object 내부의 local rule임.

`FReactionExecutionPolicy`는 현재 request와 현재 body/runtime/context를 반영해서 orchestrator가 만든 resolved policy임.

따라서 두 값의 의미는 다음처럼 정리됨.

```text
CReaction
- 지금 실행 중인 reaction object의 local 상태와 규칙임
- animation window와 executor runtime flag 중심임

FReactionExecutionPolicy
- 이번 request가 현재 캐릭터 상태에서 갖는 실행 권한 해석값임
- body/runtime/context를 종합한 orchestration-level policy임
```

현재는 policy 구조가 얇기 때문에 중복처럼 보일 수 있음.  
그러나 body state, hit resolution, guard, poise, super armor가 들어오면,  
policy는 executor hook만으로 표현할 수 없는 상위 판단을 담는 계층이 됨.


---
