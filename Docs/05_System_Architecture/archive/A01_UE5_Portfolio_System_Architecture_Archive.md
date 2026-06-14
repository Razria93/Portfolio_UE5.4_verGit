# Action / Reaction Orchestration 구조 비교

## 1. 목적

본 문서는 `ActionOrchestrator` 구조를 참고하여 `ReactionOrchestrator`를 설계할 때,  
어떤 부분은 대칭적으로 가져가도 되고 어떤 부분은 다르게 구성해야 하는지 정리하기 위한 설계 문서임.

핵심 쟁점은 다음과 같음.

- Action과 Reaction 모두 외부 요청을 받아 처리 결과를 반환하는 구조를 가질 수 있음.
- 따라서 request / result / reject reason 같은 외부 API 형식은 대칭적으로 구성할 수 있음.
- 그러나 내부 decision의 위치와 의미는 Action과 Reaction이 서로 다를 수밖에 없음.
- 그 차이는 두 시스템이 해결하려는 문제의 성격 차이에서 발생함.


---

## 2. 핵심 결론

Action과 Reaction의 가장 중요한 차이는 다음 문장으로 정리할 수 있음.

```text
Action decision은 action 자신의 진행 규칙이 중심이고,
Reaction decision은 reaction들 사이의 충돌 조정 규칙이 중심임.
```

즉 Action은 “내가 지금 어떻게 실행될 수 있는가”가 핵심이고,  
Reaction은 “새로 들어온 반응이 현재 반응과 어떤 관계를 가져야 하는가”가 핵심임.

따라서 외부 요청 구조는 비슷하게 가져가되,  
내부 decision의 책임 위치는 다르게 두는 것이 적절함.


---

## 3. Action Orchestration의 성격

Action은 캐릭터가 스스로 시작하는 주도 행동임.

대표 예시는 다음과 같음.

```text
Equip
Unequip
ComboAttack
Guard
Dodge
```

Action 요청은 Player 입력 또는 AI 판단에서 시작함.

```text
PlayerInput / AI
-> ActionIntent
-> ActionOrchestrator
-> ActionComponent
-> CAction
```

ActionOrchestrator의 주요 책임은 다음과 같음.

- 외부 request를 받을 수 있는지 검사함.
- Dead / Reaction / invalid component 같은 공통 gate를 처리함.
- Player 입력 또는 AI 요청을 공통 action request로 통일함.
- intent를 실행 가능한 `EActionType`으로 변환함.
- 실제 실행은 `ActionComponent`와 `CAction`에 위임함.

Action 내부 decision은 `CAction` 또는 `ActionComponent` 쪽에 비중을 두는 것이 자연스러움.

이유는 action별 진행 규칙이 서로 다르기 때문임.

```text
ComboAttack
- 현재 combo index
- chain window
- 선입력 여부
- 다음 타수 연결 가능 여부

Equip / Unequip
- 현재 장착 상태
- 전환 중인지 여부
- toggle intent 해석

Guard / Dodge
- 시작 가능 타이밍
- cancel / interrupt 가능 여부
- stamina / resource 조건
```

즉 Action decision은 특정 action 자신의 내부 상태와 진행 규칙에 강하게 의존함.


---

## 4. Reaction Orchestration의 성격

Reaction은 캐릭터가 스스로 시작하는 행동이 아니라, 외부 자극에 의해 발생하는 반응임.

현재 1차 입력 소스는 `TakeDamage`임.

```text
TakeDamageResult
-> ReactionIntent
-> ReactionOrchestrator
-> ReactionComponent
-> CReaction
```

Reaction request가 판단해야 하는 정보는 다음과 같음.

```text
CommittedDamage
DeadState_Before
DeadState_After
ApplyDamageSpecKey
Current Active Reaction
Incoming Reaction
Priority
Interruptible Window
Incoming Executor Policy
Current Executor Policy
```

Reaction에서 중요한 문제는 단순히 “이 reaction을 실행할 수 있는가”가 아님.

더 중요한 문제는 다음과 같음.

- 이미 reaction 중인데 새 reaction이 들어왔는가?
- 새 reaction이 active reaction을 대체할 수 있는가?
- Dead reaction은 Hit reaction보다 우선해야 하는가?
- 현재 montage window가 interruptible 상태인가?
- current executor는 interruption을 허용하는가?
- incoming executor는 interruption을 원하는가?
- priority가 더 강한가?

즉 Reaction decision은 특정 reaction 하나의 내부 진행 규칙보다,  
current reaction과 incoming reaction의 관계를 조정하는 문제가 중심임.


---

## 5. 대칭적으로 가져갈 수 있는 부분

Action과 Reaction은 모두 외부에서 request를 받고 result를 반환할 수 있음.

따라서 외부 API 형식은 대칭적으로 구성하는 것이 적절함.

### Action

```cpp
EActionRequestResultType
EActionRequestRejectReason
FActionRequestResult
```

### Reaction

```cpp
EReactionRequestResultType
EReactionRequestRejectReason
FReactionRequestResult
```

이 대응은 적절함.

이유는 호출자가 “요청이 어떻게 처리되었는가”를 읽는 방식이 일관되기 때문임.

```text
Rejected
Ignored
Started
Chained / Interrupted / Cancelled
```

또한 공통 gate 함수와 result builder 형태도 대칭적으로 가져갈 수 있음.

```text
CanAcceptActionRequest()
BuildActionRequestResult()

CanAcceptReactionRequest()
BuildReactionRequestResult()
```

이런 형식적 대칭은 유지보수성과 가독성을 높임.


---

## 6. 다르게 구성해야 하는 부분

Action의 내부 decision과 Reaction의 내부 decision은 이름과 위치를 다르게 두는 것이 적절함.

### Action 내부 decision

Action에서는 다음 타입이 자연스러움.

```cpp
EActionExecutionDecision
FActionExecutionQuery
FActionExecutionResult
```

이름에 `Execution`이 들어가는 이유는,  
판단의 중심이 “이 action이 지금 어떻게 실행될 수 있는가”이기 때문임.

예시는 다음과 같음.

```text
Start
Chain
Enqueue
Interrupt
Ignore
Reject
```

특히 `Chain`은 `ComboAttack` 같은 action 자신의 진행 규칙에 강하게 의존함.

따라서 action 내부 decision은 `CAction` 또는 `ActionComponent` 쪽에 두는 것이 적절함.


### Reaction 내부 decision

Reaction에서는 다음 타입이 더 적절함.

```cpp
EReactionOrchestrationDecision
FReactionOrchestrationQuery
FReactionOrchestrationResult
```

이름에 `Execution`보다 `Orchestration`을 쓰는 이유는,  
판단의 중심이 “reaction 자체의 실행 가능성”보다 “reaction들 사이의 충돌 조정”에 있기 때문임.

예시는 다음과 같음.

```text
Start
Interrupt
Cancel
Ignore
Reject
```

`Interrupt`와 `Cancel`은 특정 reaction 하나의 고유 실행 규칙이라기보다,
현재 active reaction과 incoming reaction 또는 외부 cancel 요청의 관계를 조정한 결과임.

따라서 reaction 내부 decision은 `ReactionOrchestrator` 쪽에 두는 것이 적절함.


---

## 7. CAction과 CReaction의 역할 차이

`CAction`과 `CReaction`은 모두 실제 실행 단위처럼 보이지만, decision에서 차이가 있음.

### CAction

`CAction`은 action 자신의 진행 규칙을 소유함.

```text
Start 가능한가
Chain 가능한가
입력 window가 열려 있는가
이미 같은 action이 실행 중인가
선입력을 받을 것인가
```

따라서 `CAction`은 decision에 적극적으로 참여할 수 있음.


### CReaction

`CReaction`은 reaction의 montage lifecycle과 local policy hook을 소유함.

```text
Montage Play
Montage Stop
Montage End Callback
Interruptible Flag
Cancelable Flag
WantToInterrupt
AllowInterruptionBy
```

그러나 최종 decision은 `CReaction` 혼자 결정하지 않는 것이 적절함.

이유는 최종 decision에 current / incoming / pending 관계, damage result, priority, dead-state 전이 등이 함께 필요하기 때문임.

따라서 `CReaction`은 다음처럼 local policy만 제공하는 구조가 적절함.

```text
Current CReaction
-> 나는 지금 interrupt를 허용하는가?

Incoming CReaction
-> 나는 현재 reaction을 interrupt하고 싶은가?

ReactionOrchestrator
-> 두 reaction의 관계와 damage result를 종합하여 최종 decision을 내림.
```


---

## 8. 입력 성격의 차이

Action과 Reaction은 입력 성격도 다름.

### Action 입력

Action 입력은 보통 제한적이고 의도적임.

```text
Attack
Equip
Dodge
Guard
Move
```

즉 플레이어 또는 AI가 “무엇을 하겠다”는 의도를 명확히 전달함.

따라서 action에서는 의도된 행동의 실행 조건과 조작감을 정교하게 다루는 것이 중요함.


### Reaction 입력

Reaction 입력은 외부 결과에서 발생하며 불규칙적임.

```text
Hit
Dead
GuardBreak
Launch
KnockDown
TrapHit
ScriptedReaction
```

입력의 발생 시점과 빈도를 통제하기 어렵고,  
이미 reaction이 실행 중인 상태에서 다시 reaction이 들어오는 상황이 자주 발생할 수 있음.

따라서 reaction에서는 입력 자체의 다양성보다,  
불규칙하게 들어오는 reaction request들 사이의 충돌 해결이 중요함.


---

## 9. 권장 대응 관계

Action과 Reaction의 타입 대응은 다음처럼 두는 것이 적절함.

```text
EActionRequestResultType
<-> EReactionRequestResultType

EActionRequestRejectReason
<-> EReactionRequestRejectReason

FActionRequestResult
<-> FReactionRequestResult

EActionExecutionDecision
<-> EReactionOrchestrationDecision

FActionExecutionQuery
<-> FReactionOrchestrationQuery

FActionExecutionResult
<-> FReactionOrchestrationResult
```

여기서 중요한 점은 `ExecutionDecision`이라는 이름을 Reaction에 그대로 가져오지 않는 것임.

Reaction에서는 decision의 핵심이 실행 자체보다 조정에 있으므로,  
`EReactionOrchestrationDecision`이 더 정확한 이름임.


---

## 10. 권장 구조

최종 구조는 다음 방향을 목표로 함.

```text
ActionOrchestrator
-> Request gate
-> Intent resolve
-> Domain component routing

ActionComponent
-> Action storage
-> Current action state
-> ExecuteAction

CAction
-> Action-specific progress rule
-> Montage / notify / cleanup
```

```text
ReactionOrchestrator
-> Request gate
-> Damage result -> Reaction intent
-> Reaction type / data / executor resolve
-> Active / incoming conflict resolution
-> Decision generation

ReactionComponent
-> Reaction data / executor ownership
-> Active runtime state
-> ApplyReactionDecision
-> Movement / state / action abort application

CReaction
-> Montage lifecycle
-> Notify window runtime flags
-> Local interrupt / cancel policy hook
```

즉 형식은 대칭적으로 유지하되,  
decision의 중심 위치는 Action과 Reaction에서 다르게 두는 것이 적절함.


---

## 11. 설계 원칙

본 구조에서 지켜야 할 원칙은 다음과 같음.

- 외부 request / result 형식은 Action과 Reaction을 가능한 대칭적으로 구성함.
- Action의 내부 decision은 action 자신의 진행 규칙을 중심으로 둠.
- Reaction의 내부 decision은 reaction들 사이의 충돌 조정을 중심으로 둠.
- `ReactionOrchestrator`는 decision을 만들되 montage lifecycle을 직접 소유하지 않음.
- `ReactionComponent`는 runtime state와 decision 적용을 담당함.
- `CReaction`은 실행 타이밍과 local policy hook을 담당함.
- Action과 Reaction의 차이는 구현 취향이 아니라 입력 성격과 해결 문제의 차이에서 발생함.


---

## 12. 요약

Action과 Reaction은 모두 orchestration 구조를 가질 수 있음.

하지만 두 구조는 완전히 같아서는 안 됨.

```text
Action
= 의도한 행동의 실행 가능성과 진행 규칙을 정교하게 판단하는 문제

Reaction
= 불규칙하게 들어오는 반응 요청들의 우선순위와 충돌을 안정적으로 조율하는 문제
```

따라서 다음 기준을 따른다.

```text
형식은 대칭
내부 decision 위치는 비대칭
```

이 기준을 적용하면 `ActionOrchestrator` 구조를 참고하면서도,  
Reaction 고유의 충돌 조정 문제를 명확하게 다룰 수 있음.


---
