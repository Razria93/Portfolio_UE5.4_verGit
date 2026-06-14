# Reaction Pending 모델

## 1. 목적

본 문서는 기존 Reaction 구조에서 `PendingReaction`이 사용된 배경과 한계를 정리하고,  
Reaction Orchestration 구조에서 pending 흐름을 제거해야 하는 이유를 기록하기 위한 설계 문서임.

핵심 목적은 다음과 같음.

- 기존 pending이 AI reaction 구현 과정에서 도입된 배경을 정리함.
- 기존 pending consume 구조가 Reaction의 기본 실행 흐름과 어긋나는 이유를 정리함.
- Reaction Orchestration 이후 reaction 실행 주체가 어디에 있어야 하는지 정리함.
- BT가 reaction을 실행하는 구조에서 reaction 상태를 관찰하는 구조로 이동해야 하는 이유를 정리함.


---

## 2. Pending을 사용하게 된 배경

기존 `PendingReaction`은 reaction을 AI부터 구현하면서 도입된 구조임.

당시 Enemy reaction은 BT 흐름 안에서 제어되어야 했음.  
따라서 damage event가 발생한 시점에 reaction을 바로 실행하지 않고,  
`ReactionComponent`에 pending으로 저장한 뒤 BT task가 이를 소비하는 형태가 되었음.

기존 Enemy 흐름은 다음에 가까웠음.

```text
TakeDamage
-> ReactionComponent::TryRequestPendingDamageReaction()
-> PendingReactionContext 저장
-> CBTTask_StartReaction
-> ReactionComponent::TryConsumePendingReaction()
-> ReactionComponent::TryExecuteReaction()
```

이 구조에서는 BT가 reaction 실행의 직접 주체가 됨.

이후 Player도 Enemy와 동일한 reaction component 흐름을 사용하려다 보니,  
Player 쪽에서도 pending을 소비할 별도 지점이 필요해졌음.

그 결과 Player는 Tick에서 pending reaction을 소비하는 구조가 되었음.

```text
TakeDamage
-> ReactionComponent::TryRequestPendingDamageReaction()
-> PendingReactionContext 저장
-> Player Tick
-> ConsumePendingReaction()
-> ReactionComponent::TryConsumePendingReaction()
-> ReactionComponent::TryExecuteReaction()
```

즉 기존 pending은 reaction 간 충돌 정책을 표현하기 위한 구조가 아니라,  
AI BT 실행 흐름에 reaction 실행을 맞추기 위해 만들어진 지연 브릿지였음.


---

## 3. Pending 구조의 한계

기존 pending 방식은 구조적으로 정상적인 reaction 흐름과 어긋남.

Reaction의 기본 흐름은 다음과 같아야 함.

```text
Action
-> Apply Damage
-> Take Damage
-> Reaction
```

공격 action이 hit를 만들고, damage가 적용되며, target이 damage를 처리한 뒤,  
그 결과에 따라 reaction이 발생하는 흐름이 자연스러움.

그러나 기존 구조에서는 reaction 실행을 BT로 이관하기 위해 실행 주체를 외부에 두었음.  
이를 위해 damage 처리 시점에 reaction을 바로 실행하지 않고 pending으로 지연시켰음.

이 구조는 Enemy 기준으로는 BT 흐름에 맞아 보일 수 있으나,  
Player 기준으로는 비정상적인 구조를 만들게 됨.

```text
Player는 BT가 없음
그런데 Enemy와 같은 pending consume 구조를 맞추기 위해 Tick에서 reaction을 소비함
damage event와 reaction 실행 사이에 불필요한 frame 지연과 우회 경로가 생김
```

즉 pending은 reaction의 본래 의미를 표현한 것이 아니라,  
BT가 reaction을 실행하도록 만들기 위한 제어상의 우회였음.

이 방식은 다음 한계를 가짐.

```text
reaction request의 최종 decision 위치가 명확하지 않음
TakeDamage 시점과 reaction 실행 시점이 불필요하게 분리됨
Player와 Enemy가 서로 다른 실행 소비 지점을 갖게 됨
BT가 외부 자극 기반 reaction의 실행 주체처럼 동작함
ReactionComponent가 request 저장, 실행 가능성 판단, 실행 적용을 동시에 담당함
```


---

## 4. Reaction Orchestration 이후의 기본 흐름

Reaction Orchestration 구조에서는 damage event가 reaction 실행 요청의 직접 진입점이 됨.

권장 흐름은 다음과 같음.

```text
TakeDamage
-> ReactionOrchestrator::RequestReaction()
-> ResolveReactionContext()
-> ResolveReactionPolicy()
-> OrchestrateQuery()
-> ReactionComponent::ApplyReactionDecision()
-> CReaction 실행
```

이 구조에서는 Player Tick 또는 BTTask가 pending reaction을 consume할 필요가 없음.

Player 기준 흐름은 다음처럼 단순화됨.

```text
TakeDamage
-> ReactionOrchestrator
-> ReactionComponent
-> CReaction
```

Enemy 기준 흐름도 동일함.

```text
TakeDamage
-> ReactionOrchestrator
-> ReactionComponent
-> CReaction
-> BT는 ExecutionState::Reaction을 관찰함
```

즉 BT는 reaction을 시작하는 주체가 아니라,  
reaction 실행 중이라는 runtime state를 보고 기다리거나 다른 intent branch를 막는 역할을 담당함.


---

## 5. 책임 분리 기준

Reaction Orchestration 이후에는 pending을 중심으로 흐름을 구성하지 않고,  
Orchestrator / Component / Execution Object의 책임을 분리하는 방향이 적절함.

### ReactionOrchestrator

`ReactionOrchestrator`는 external event 기반 reaction request를 해석하고,  
현재 runtime state에서 어떤 reaction 처리가 가능한지 결정하는 계층임.

주요 책임은 다음과 같음.

```text
TakeDamage 기반 reaction request 수신함
damage result를 기반으로 reaction type을 결정함
reaction data와 executor를 resolve함
active reaction과 incoming reaction을 비교함
Start / Interrupt / Ignore / Reject decision을 생성함
decision을 ReactionComponent로 dispatch함
```

즉 orchestrator는 reaction을 직접 실행하지 않음.  
orchestrator는 reaction request를 평가하고 실행 방향을 결정함.

### ReactionComponent

`ReactionComponent`는 orchestrator가 내린 decision을 실제 캐릭터 runtime state에 적용하는 계층임.

주요 책임은 다음과 같음.

```text
ActiveReactionContext를 저장함
reaction executor instance를 캐싱함
Start decision을 실제 active reaction 시작으로 적용함
Interrupt decision을 active reaction 중단 후 incoming reaction 시작으로 적용함
Cancel decision을 active reaction 중단으로 적용함
movement / state / action abort side effect를 처리함
reaction 종료 시 active context를 정리함
```

즉 component는 reaction 실행 상태를 관리하는 객체임.  
reaction request를 pending으로 저장하고 외부 consume을 기다리는 객체가 아님.

### CReaction

`CReaction`은 실제 reaction 실행 lifecycle을 담당하는 execution object임.

주요 책임은 다음과 같음.

```text
montage 재생을 시작함
reaction notify window를 처리함
interruptible / cancelable 같은 local runtime flag를 제공함
Stop / End / cleanup 흐름을 처리함
```

즉 `CReaction`은 최종 실행 객체이며,  
request resolve나 orchestration decision을 직접 담당하지 않음.


---

## 6. BT 구조 변화

Reaction Orchestration 이후 `CBTTask_StartReaction`은 제거하거나 호환용 no-op으로 축소하는 것이 적절함.

기존 BT 흐름은 다음에 가까웠음.

```text
Blackboard bHasPendingReaction
-> CBTTask_StartReaction
-> ReactionComponent::TryConsumePendingReaction()
-> Reaction 실행
```

새 BT 흐름은 다음에 가까워야 함.

```text
ExecutionState == Reaction
-> CBTTask_WaitEndReaction 또는 decorator wait
-> reaction 종료 후 기존 intent 흐름 재개
```

따라서 BT blackboard key도 다음 방향으로 정리하는 것이 적절함.

```text
bHasPendingReaction
- 제거 대상임
- pending consume bridge가 사라지면 필요하지 않음

PendingReactionVersion
- 제거 대상임
- BT가 pending 변경을 감지하여 reaction을 시작할 필요가 없음

bHasActiveReaction
- 유지 가능함
- 다만 ExecutionState::Reaction에서 파생 가능한 값이면 중복 상태가 될 수 있음

EAIIntentState::HitReact
- 장기적으로 제거 후보임
- hit reaction은 AI intent가 아니라 external event 기반 execution state임
```


---

## 7. 구현 단계 제안

1차 구현에서는 기존 pending consume bridge를 제거하는 방향이 적절함.

```text
1. ReactionComponent에서 PendingReactionContext를 제거함
2. ReactionComponent에서 TryRequestPendingDamageReaction을 제거함
3. ReactionComponent에서 TryConsumePendingReaction을 제거함
4. ReactionComponent에서 TryExecuteReaction을 StartReaction / InterruptReaction으로 분리함
5. Player Tick의 ConsumePendingReaction 흐름을 제거함
6. Enemy CBTTask_StartReaction을 제거하거나 no-op으로 축소함
7. CBTTask_WaitEndReaction은 active reaction wait 역할로 축소함
8. TakeDamageComponent가 ReactionOrchestratorComponent를 직접 호출함
9. ReactionOrchestratorComponent가 RequestReaction 진입점을 담당함
```

이후 구현에서는 pending 흐름을 다시 구성하지 않고,  
orchestrator의 `Request -> Resolve -> Orchestrate -> Dispatch` 흐름을 완성하는 것이 핵심임.

```text
1. ReactionOrchestratorComponent가 damage result를 reaction request로 변환함
2. ReactionOrchestratorComponent가 reaction type / data / executor를 resolve함
3. ReactionOrchestratorComponent가 active reaction과 incoming reaction을 비교함
4. ReactionOrchestratorComponent가 Start / Interrupt / Ignore / Reject decision을 생성함
5. ReactionComponent가 decision에 따라 runtime state와 execution object를 갱신함
```


---

## 8. 결론

기존 `PendingReaction`은 reaction의 본질적인 상태 모델이 아니라,  
AI BT 안에서 reaction을 실행하기 위해 만들어진 지연 브릿지였음.

이 구조는 Enemy 기준에서는 동작할 수 있었지만,  
Player 쪽에서는 Tick에서 pending을 소비하는 비정상적인 흐름을 만들었음.

Reaction의 기본 흐름은 다음과 같이 정리되어야 함.

```text
Action
-> Apply Damage
-> Take Damage
-> ReactionOrchestrator
-> ReactionComponent
-> CReaction
```

따라서 현재 단계에서 권장되는 방향은 다음과 같음.

```text
old pending bridge 제거함
BT는 reaction 실행 주체가 아니라 관찰자로 축소함
ReactionOrchestrator는 reaction request decision을 담당함
ReactionComponent는 active runtime state와 실행 적용에 집중함
CReaction은 실제 montage lifecycle과 local runtime flag를 담당함
```
