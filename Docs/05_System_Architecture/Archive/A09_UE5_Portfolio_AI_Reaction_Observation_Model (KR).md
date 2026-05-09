# AI Reaction Observation 모델

## 1. 목적

본 문서는 reaction orchestration 도입 이후 AI Behavior Tree가 reaction을 어떻게 다루어야 하는지 정리하기 위한 문서임.

기존 구조에서는 AI reaction 실행이 BT 내부 pending consume 흐름과 강하게 연결되어 있었음.

그러나 reaction은 AI intent가 아니라 외부 damage event에 의해 발생하는 execution임.

따라서 reaction trigger를 BT가 직접 소비하는 구조보다, damage pipeline에서 reaction orchestration으로 직접 연결하고 BT는 active reaction state를 관찰하는 구조가 더 적절함.

본 문서는 해당 방향을 명확히 하여 AI intent와 reaction execution state를 분리하는 것을 목적으로 함.

---

## 2. 기존 구조의 문제

기존 reaction은 AI부터 구현되었기 때문에 BT와 pending 구조가 강하게 결합되어 있었음.

기본 흐름은 다음과 같았음.

```text
TakeDamage
-> ReactionComponent pending reaction 저장
-> Blackboard pending reaction key 갱신
-> CBTTask_StartReaction
-> pending reaction consume
-> reaction 실행
```

이 구조는 AI BT 안에서 reaction 실행을 통제하기 위해 만들어진 구조임.

그러나 player 쪽에서도 같은 pending consume 구조를 맞추려다 보니 tick에서 pending reaction을 소비하는 비정상적인 흐름이 생겼음.

Reaction의 자연스러운 흐름은 다음과 같음.

```text
Action
-> ApplyDamage
-> TakeDamage
-> ReactionOrchestrator
-> ReactionComponent
-> CReaction
```

따라서 reaction 실행을 BT로 우회시키기 위해 pending 상태를 만드는 것은 현재 구조에서는 적절하지 않음.

---

## 3. 현재 권장 흐름

현재 권장 흐름은 다음과 같음.

```text
TakeDamage
-> ReactionOrchestrator
-> ReactionComponent
-> CReaction
-> active reaction state 갱신
-> BT는 active reaction state를 관찰함
```

이 흐름에서 BT는 reaction을 실행하지 않음.

BT는 reaction이 실행 중인지 관찰하고, 필요하면 현재 task를 기다리거나 중단하거나 다른 branch로 이동함.

즉 BT는 reaction trigger source가 아니라 reaction state observer임.

---

## 4. Intent와 Reaction의 차이

AI intent는 AI가 무엇을 하려는지 나타내는 상위 의사결정 상태임.

예시는 다음과 같음.

```text
Patrol
Chase
Engage
Attack
Retreat
```

반면 reaction은 외부 자극에 의해 body가 강제로 수행하는 execution state임.

예시는 다음과 같음.

```text
Hit
Dead
Stagger
Knockback
GuardBreak
```

따라서 reaction은 AI intent의 하위 선택지가 아니라, body state / execution state 쪽에 더 가까움.

AI가 어떤 intent를 가지고 있더라도 damage를 받으면 reaction이 발생할 수 있음.

따라서 BT의 상위 분기는 intent보다 body state를 먼저 고려하는 것이 더 안정적임.

---

## 5. Blackboard의 역할

Blackboard는 reaction 실행 요청을 저장하는 곳이 아니라 reaction state를 관찰하기 위한 곳이어야 함.

권장 역할은 다음과 같음.

```text
IsInReaction
-> 현재 active reaction이 있는지 나타냄

ActiveReactionType
-> 현재 active reaction type을 나타냄

IsDead
-> dead state를 나타냄
```

반대로 다음 정보는 blackboard의 주요 책임으로 두지 않는 것이 적절함.

```text
PendingReactionContext
PendingReactionVersion
PendingReactionRequest
```

이 값들은 reaction 실행 요청을 BT로 이관하기 위해 필요했던 값에 가까움.

현재 구조에서는 reaction 실행 요청이 orchestrator로 직접 들어가므로 blackboard에 pending request를 둘 필요가 없음.

---

## 6. BT Task의 역할 변화

`CBTTask_StartReaction`은 현재 구조에서 핵심 task가 아님.

Reaction execution은 BT task가 아니라 `TakeDamage -> ReactionOrchestrator -> ReactionComponent` 흐름에서 시작됨.

따라서 `CBTTask_StartReaction`은 제거하거나 호환용 no-op에 가깝게 축소하는 것이 적절함.

반면 `CBTTask_WaitEndReaction`은 의미가 남음.

이 task는 reaction을 시작하는 것이 아니라 active reaction이 끝날 때까지 기다리는 observer task로 사용할 수 있음.

권장 의미는 다음과 같음.

```text
CBTTask_StartReaction
-> 제거 또는 legacy compatibility task로 축소함

CBTTask_WaitEndReaction
-> active reaction state가 끝날 때까지 대기함
-> reaction 실행 주체가 아님
```

---

## 7. BT Service의 역할

BT Service는 AI context와 blackboard state를 최신화하는 역할을 담당함.

Reaction orchestration 이후 BT Service는 pending reaction을 소비하지 않음.

대신 character component state를 읽어 blackboard에 관찰 가능한 상태를 반영함.

예시는 다음과 같음.

```text
ReactionComponent::IsActiveReaction()
-> Blackboard.IsInReaction

ReactionComponent::GetActiveReactionType()
-> Blackboard.ActiveReactionType

StateComponent / HealthComponent
-> Blackboard.IsDead
```

이렇게 하면 BT는 직접 reaction을 실행하지 않으면서도 reaction 상태를 기준으로 branch를 제어할 수 있음.

---

## 8. Body State 우선 분기

Reaction은 intent보다 body state에 더 가까움.

따라서 BT 상위 분기는 다음 순서가 더 적절함.

```text
Dead?
-> dead branch

InReaction?
-> wait / blocked branch

Otherwise
-> intent branch
   -> patrol / chase / engage / attack
```

이 구조는 AI intent가 무엇이든 body가 dead 또는 reaction 상태라면 우선 해당 상태를 처리하게 함.

이는 reaction을 intent state로 넣는 방식보다 확장성이 좋음.

Parry, guard, dodge, counter 같은 기능이 추가되어도 body execution state와 intent decision을 분리해서 유지할 수 있음.

---

## 9. Player와 AI의 대칭성

Reaction trigger가 TakeDamage에서 시작되면 player와 AI 모두 같은 흐름을 사용할 수 있음.

```text
TakeDamage
-> ReactionOrchestrator
-> ReactionComponent
-> CReaction
```

Player는 tick에서 pending reaction을 consume하지 않음.

AI는 BT에서 pending reaction을 consume하지 않음.

둘 다 active reaction state를 component가 관리하고, 상위 시스템은 이를 관찰함.

이 구조가 player와 AI의 reaction execution을 가장 대칭적으로 유지함.

---

## 10. 결론

AI BT는 reaction 실행 주체가 아니라 reaction state observer가 되는 것이 적절함.

```text
Damage pipeline
-> reaction을 발생시킴

ReactionOrchestrator
-> reaction 실행 여부와 경쟁 상태를 판단함

ReactionComponent
-> active reaction state를 관리함

BT
-> active reaction state를 관찰하고 branch를 제어함
```

이 구조를 유지하면 pending reaction consume 흐름을 제거할 수 있고, player와 AI가 같은 reaction execution pipeline을 공유할 수 있음.

따라서 reaction은 AI intent가 아니라 body execution state로 다루는 것이 현재 프로젝트 구조에 더 적절함.
