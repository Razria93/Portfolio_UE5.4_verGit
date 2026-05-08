# Orchestrator / Data / Component 책임 분리 모델

## 1. 목적

본 문서는 Reaction Orchestration 설계 과정에서 드러난 책임 분리 문제를 정리하고,  
향후 Action Orchestration에도 적용 가능한 공통 구조 원칙을 기록하기 위한 설계 문서임.

Reaction 작업을 통해 명확해진 핵심 쟁점은 다음과 같음.

- Orchestrator는 단순한 입력 라우터가 아니라 경쟁 상태를 평가하는 계층이 될 수 있음.
- Domain Component는 runtime state와 실행 적용을 담당하는 것이 자연스러움.
- Definition data는 component에 결속될수록 공유성과 확장성이 떨어짐.
- Executor object는 실제 실행 lifecycle과 local policy hook을 담당하는 것이 적절함.


---

## 2. 문제 인식

Action Orchestration 작업 당시에는 오케스트레이터의 조정 책임이 비교적 약하게 드러났음.

당시 핵심 흐름은 다음에 가까웠음.

```text
Player / AI Request
-> Common Gate
-> ActionType Resolve
-> ActionComponent Execute
```

즉 주요 쟁점은 “요청을 받을 수 있는가”와 “어떤 action으로 보낼 것인가”였음.

반면 Reaction에서는 즉시 경쟁 상태가 발생함.

```text
Hit 중 다시 Hit
Hit 중 Dead
Hit 중 더 강한 Hit
Interruptible window 밖에서 incoming reaction
```

따라서 Reaction에서는 단순 라우팅이 아니라,  
현재 실행 상태와 incoming request를 비교하여 최종 decision을 만드는 책임이 중요해짐.


---

## 3. Reaction에서 드러난 책임 충돌

현재 Reaction 구조에서 `UCReactionComponent`는 여러 책임을 동시에 갖고 있음.

```text
ReactionDatas 보관
ReactionDataMap 구성
ReactionExecutorMap 캐싱
ActiveReactionContext 보관
current vs incoming 판정
movement / state / action abort 적용
CReaction 실행 호출
```

이 구조에서는 `ReactionOrchestrator`를 도입할 때 문제가 생김.

오케스트레이터가 reaction 경쟁 상태를 판단하려면 다음 정보가 필요함.

```text
Reaction definition data
Reaction priority
Reaction executor class
ActiveReactionContext
Interruptible / cancelable policy
```

그런데 이 정보들이 모두 `ReactionComponent`에 있으면,  
오케스트레이터가 component 내부 데이터를 getter로 끌어와 평가하는 형태가 됨.

이 경우 구조적으로 다음 의문이 생김.

```text
어차피 데이터를 다 ReactionComponent에서 가져온다면
경쟁 상태 판정도 ReactionComponent에서 해도 되는 것 아닌가?
```

이 의문은 타당함.

문제의 원인은 오케스트레이터의 존재가 아니라,  
definition data와 runtime state가 같은 component 안에 결속되어 있기 때문임.


---

## 4. 핵심 분리 기준

Reaction 구조는 다음 네 책임으로 나누는 것이 적절함.

```text
Definition Data
- 어떤 실행 후보들이 있는가
- 어떤 key로 찾는가
- priority / policy 기본값은 무엇인가

Orchestrator
- 외부 request를 domain intent로 변환함
- definition data를 조회함
- 현재 runtime state를 참조함
- 경쟁 상태를 평가함
- decision을 생성함

Component
- runtime state를 소유함
- executor instance cache를 소유함
- decision을 실제 캐릭터 상태에 적용함
- movement / state / action side effect를 처리함

Executor Object
- 실제 실행 lifecycle을 담당함
- montage / notify / cleanup을 담당함
- local timing과 local policy hook을 제공함
```

이 기준을 적용하면 각 계층의 역할이 명확해짐.


---

## 5. Reaction 기준 권장 구조

Reaction의 장기 목표 구조는 다음과 같음.

```text
ReactionDefinitionDataAsset
-> reaction definitions
-> match key / priority / montage / executor class / play policy

ReactionOrchestratorComponent
-> request gate
-> damage result -> reaction intent
-> data lookup
-> active / incoming conflict resolution
-> decision generation

ReactionComponent
-> active runtime state
-> executor instance cache
-> ApplyReactionDecision
-> movement / state / action abort application

CReaction
-> montage lifecycle
-> notify window runtime flags
-> local interrupt / cancel policy hook
```

이 구조에서 `ReactionComponent`가 외부에 제공해야 하는 정보는 runtime state 중심임.

```text
GetActiveReactionContext()
GetActiveReactionExecutor()
```

반면 다음 책임은 장기적으로 component 밖으로 올리는 것이 적절함.

```text
ReactionDatas
ReactionDataMap
BuildCandidateSpecKeys
ResolveReactionData
Priority decision policy
```

이들은 실행 상태라기보다 definition / matching / policy 데이터에 가까움.


---

## 6. 데이터 에셋 분리 방향

현재 `FReactionData`는 선택 조건, 실행 데이터, policy 값이 섞여 있음.

```text
ReactionDataKey
ReactionExecutorKey
Montage
PlayRate
bCanMove
Priority
```

1차적으로는 이를 그대로 `ReactionDefinitionDataAsset`으로 올리는 방식이 현실적임.

예상 구조는 다음과 같음.

```cpp
UCLASS(BlueprintType)
class PORTFOLIO_API UCReactionDefinitionDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TArray<FReactionData> ReactionDatas;
};
```

이후 필요해지면 다음처럼 더 세분화할 수 있음.

```text
FReactionMatchKey
- ApplyDamageSpecKey
- ReactionType

FReactionExecutionData
- ExecutorClass
- Montage
- PlayRate
- bCanMove

FReactionPolicy
- Priority
- interrupt / replace policy
```

하지만 현재 단계에서는 과도한 분해보다,  
component에서 definition data를 분리하는 것이 우선임.


---

## 7. Orchestrator가 데이터를 조회하는 이유

Orchestrator는 경쟁 상태를 평가하기 위해 여러 정보를 함께 봐야 함.

```text
incoming reaction definition
active reaction context
incoming reaction context
priority
interruptible window
current executor policy
incoming executor policy
```

따라서 Orchestrator가 여러 계층에서 정보를 조회하는 것은 자연스러운 일임.

다만 중요한 기준은 다음과 같음.

```text
definition data는 Orchestrator 또는 DataAsset 계층에서 조회함.
runtime state는 Component에서 조회함.
실제 실행 lifecycle은 Executor에서 처리함.
```

즉 Orchestrator가 모든 것을 “소유”하는 것이 아니라,  
판단에 필요한 데이터를 각 책임 계층에서 읽어 최종 decision을 만드는 구조가 적절함.


---

## 8. Action 구조에 대한 시사점

Action Orchestration은 현재 단계에서는 request gate와 routing 성격이 강함.

```text
ActionOrchestrator v1
-> request gate
-> intent resolve
-> domain component routing
```

그러나 향후 Action에도 경쟁 상태가 발생할 수 있음.

예시는 다음과 같음.

```text
Attack 중 Dodge
Attack 중 Guard
Dodge 중 Attack
Equip 중 Skill
Guard 중 Parry
Skill 중 Cancel
Buffered Action Queue
AI requested action vs forced action
```

이 경우 Action 쪽도 Reaction에서 정리한 원칙을 역수입할 수 있음.

```text
ActionDefinitionDataAsset
-> action definitions
-> execution policy
-> cancel / interrupt / chain policy

ActionOrchestrator v2
-> request gate
-> action data lookup
-> current / incoming action conflict resolution
-> decision generation

ActionComponent
-> current action runtime state
-> action executor cache
-> decision apply

CAction
-> action-specific lifecycle
-> local timing / combo / notify policy
```

즉 Reaction 작업은 향후 Action Orchestration을 고도화할 때 참고할 수 있는 더 성숙한 모델이 될 수 있음.


---

## 9. 지금 당장 적용할 범위

현재 Reaction Orchestration 1차 범위에서는 다음 기준을 적용하는 것이 적절함.

```text
1. Active runtime state는 ReactionComponent에 유지함.
2. Executor instance cache도 ReactionComponent에 유지함.
3. Reaction definition data는 DataAsset 또는 Orchestrator 계층으로 올리는 방향을 잡음.
4. Conflict resolution은 ReactionOrchestrator에서 담당함.
5. ReactionComponent는 decision 적용과 실행 상태 관리에 집중함.
```

이렇게 하면 다음 효과를 얻을 수 있음.

- `ReactionComponent`가 과도한 데이터 컨테이너가 되는 것을 줄임.
- `ReactionOrchestrator`가 이름에 맞게 request 조정과 conflict decision을 담당함.
- 캐릭터별 reaction profile을 DataAsset으로 공유하거나 교체하기 쉬워짐.
- 나중에 Guard / Parry / Launch / KnockDown 같은 reaction source가 늘어나도 같은 decision path를 사용할 수 있음.


---

## 10. 설계 원칙

본 구조에서 지켜야 할 원칙은 다음과 같음.

- Definition data와 runtime state를 분리함.
- Orchestrator는 판단을 위해 데이터를 조회하지만, 실행 lifecycle을 직접 소유하지 않음.
- Component는 현재 캐릭터 인스턴스의 runtime state와 side effect 적용을 담당함.
- Executor object는 실제 실행 타이밍과 local policy hook을 담당함.
- DataAsset은 캐릭터별 또는 타입별 reaction profile을 공유 가능하게 만드는 방향으로 사용함.
- Action도 경쟁 상태가 복잡해지는 시점에는 같은 책임 분리 모델을 재검토함.


---

## 11. 요약

Reaction Orchestration 작업에서 드러난 핵심은 다음과 같음.

```text
Orchestrator = 경쟁 상태를 평가하고 decision을 생성함
Component    = runtime state를 소유하고 decision을 적용함
DataAsset    = definition data와 matching source를 제공함
Executor     = 실제 실행 lifecycle과 local policy를 담당함
```

이 구조는 Reaction에 먼저 필요성이 드러났지만,  
향후 Action 쪽에서도 경쟁 상태가 커지면 같은 방향으로 확장할 수 있음.

따라서 현재 Reaction 작업은 단순 기능 추가가 아니라,  
전투 시스템 전체의 요청 / 판정 / 실행 계층을 정리하는 기준점으로 사용할 수 있음.


---
