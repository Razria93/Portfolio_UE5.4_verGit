# Reaction Pending Removal and AI Observation Architecture Decision

## 1. 목적

본 문서는 기존 reaction pending 구조를 제거하고, AI Behavior Tree를 reaction 실행 주체가 아니라 active reaction state observer로 축소한 이유를 정리하기 위한 architecture decision 문서임.

archive 문서 `A04`, `A09`의 내용을 하나의 흐름으로 재구성함.

---

## 2. 배경

기존 reaction은 AI부터 구현되었고, 그 과정에서 BT와 pending 구조가 강하게 결합되었음.

당시 Enemy reaction 흐름은 다음에 가까웠음.

```text
TakeDamage
-> ReactionComponent에 pending reaction 저장함
-> Blackboard pending key 갱신함
-> CBTTask_StartReaction
-> pending reaction consume
-> reaction 실행함
```

이 구조는 BT 내부에서 reaction 실행 시점을 제어하기 위해 만들어진 구조임.

그러나 Player는 BT가 없기 때문에 같은 pending 모델을 맞추기 위해 Tick에서 pending reaction을 consume해야 했음.

결과적으로 pending은 reaction의 본질적인 상태 모델이라기보다, AI BT 실행 구조를 연결하기 위한 bridge에 가까웠음.

S06에서 정리한 orchestration 구조는 이 bridge를 제거하고, damage event 이후 reaction request가 직접 orchestration pipeline으로 들어가도록 재구성함.

따라서 본 문서는 그 후속 결정으로서 pending 제거와 AI BT 역할 축소를 정리함.

---

## 3. 문제점

기존 pending 모델의 핵심 문제는 reaction 실행 소유권이 분산된 점임.

동일한 damage event에서 시작했지만, 실제 reaction 실행은 다음처럼 서로 다른 소비자에게 의존했음.

```text
Enemy
-> BT task가 pending reaction을 consume함

Player
-> Tick에서 pending reaction을 consume함
```

Reaction의 자연스러운 흐름은 damage event 이후 reaction execution으로 이어지는 것임.

```text
Action
-> ApplyDamage
-> TakeDamage
-> Reaction
```

그러나 기존 구조에서는 reaction 실행을 BT 또는 Tick으로 우회시키기 위해 pending 상태를 만들었음.

이 방식의 문제는 다음과 같음.

```text
damage 처리 시점과 reaction 실행 시점이 불필요하게 분리됨
Player는 Tick consume이라는 별도 흐름을 가져야 함
Enemy는 BT task가 reaction 실행 owner처럼 동작함
ReactionComponent가 pending storage와 execution manager 역할을 함께 가짐
reaction 실행 경로가 Player / Enemy 사이에서 대칭적이지 않음
```

따라서 pending은 현재 reaction orchestration 구조의 중심 모델로 유지하지 않는 것이 적절함.

---

## 4. 결정 사항

Reaction pending consume 구조를 제거하고, reaction request는 `TakeDamage`에서 `ReactionOrchestrator`로 직접 전달함.

현재 흐름은 다음과 같음.

```text
TakeDamage
-> ReactionOrchestrator::RequestReaction()
-> ResolveReactionContext()
-> ResolveReactionPolicy()
-> OrchestrateQuery()
-> ReactionComponent::ApplyReactionDecision()
-> CReaction 실행함
```

Player와 Enemy는 같은 reaction execution pipeline을 공유함.

```text
Player
-> TakeDamage
-> ReactionOrchestrator
-> ReactionComponent
-> CReaction

Enemy
-> TakeDamage
-> ReactionOrchestrator
-> ReactionComponent
-> CReaction
-> BT는 active reaction state를 관찰함
```

BT는 reaction을 실행하지 않음.

BT는 active reaction state를 읽고, 대기하거나 branch를 막거나 intent 흐름을 제어함.

---

## 5. AI 관찰 모델

AI BT는 reaction trigger source가 아니라 reaction state observer임.

권장 top-level branch 순서는 다음과 같음.

```text
Dead?
-> dead branch

InReaction?
-> wait / blocked branch

Otherwise
-> intent branch
   -> patrol / chase / engage / attack
```

이 구조는 body state를 intent보다 우선함.

AI가 어떤 intent를 가지고 있더라도 body가 dead 또는 reaction 상태라면 해당 상태가 우선 처리되어야 하기 때문임.

따라서 reaction은 AI intent가 아니라 body execution state에 가까움.

---

## 6. 블랙보드 역할

Blackboard는 reaction request를 저장하는 곳이 아니라 reaction state를 관찰하는 곳으로 사용함.

현재 기준의 핵심 key는 다음과 같음.

```text
bIsActiveReaction
```

이 값은 `ReactionComponent::IsActiveReaction()`을 기반으로 갱신됨.

다음과 같은 pending request key는 현재 구조의 핵심이 아님.

```text
PendingReactionContext
PendingReactionVersion
bHasPendingReaction
```

이 값들은 BT가 pending request를 consume하던 구조에서 필요했던 값임.

현재 구조에서는 reaction execution request가 orchestrator로 직접 들어가므로 blackboard에 pending request를 둘 필요가 없음.

---

## 7. BT 태스크 역할

`CBTTask_StartReaction`은 더 이상 reaction execution owner가 아님.

현재 역할은 legacy compatibility 또는 active reaction 여부 확인 수준으로 축소됨.

`CBTTask_WaitEndReaction`은 의미가 남음.

이 task는 reaction을 시작하지 않고, active reaction이 끝날 때까지 기다리는 observer task로 사용할 수 있음.

```text
CBTTask_StartReaction
-> reaction 실행 주체가 아님
-> active reaction 상태 확인 또는 호환용 task로 축소함

CBTTask_WaitEndReaction
-> active reaction state가 false가 될 때까지 대기함
-> reaction execution owner가 아님
```

---

## 8. 현재 구현

현재 구현 기준은 다음과 같음.

```text
CTakeDamageComponent
-> accepted damage 이후 FDamageReactionRequest 생성함
-> ReactionOrchestratorComp_Cached->RequestReaction() 호출함

UCReactionComponent
-> PendingReactionContext 없음
-> IsActiveReaction() 제공함
-> ApplyReactionDecision()으로 decision 적용함

BT Service
-> ReactionComponent::IsActiveReaction()을 읽음
-> Blackboard bIsActiveReaction 갱신함

CBTTask_WaitEndReaction
-> bIsActiveReaction을 관찰함
```

Player Tick 기반 pending consume 흐름은 제거됨.

Enemy BT 기반 pending consume 흐름도 제거됨.

---

## 9. 결과

이 결정의 장점은 다음과 같음.

```text
Player와 Enemy reaction execution path가 동일해짐
Reaction 실행 시점이 damage 처리 흐름과 자연스럽게 연결됨
BT가 execution owner가 아니라 state observer로 단순화됨
ReactionComponent가 pending storage를 소유하지 않아도 됨
추후 body state 기반 BT branch 설계가 명확해짐
```

주의할 점은 다음과 같음.

```text
BT에서 reaction 시작을 직접 제어하던 설계는 더 이상 중심 구조가 아님
reaction 중 AI intent branch를 막는 조건은 blackboard active reaction state에 의존함
future queue / buffer 모델이 필요하면 pending과 다른 별도 모델로 설계해야 함
```

---

## 10. 후속 작업

후속 작업 후보는 다음과 같음.

```text
BT top-level body state branch를 dead / reaction / intent 순서로 더 명확히 정리함
AI combat availability 계산에서 active reaction state 사용 범위를 점검함
future queue / buffered reaction이 필요할 경우 pending과 다른 별도 모델로 설계함
```


---

## 11. 관련 문서

관련 상세 문서는 다음과 같음.

```text
A04_UE5_Portfolio_Reaction_Pending_Model
A09_UE5_Portfolio_AI_Reaction_Observation_Model
```


---
