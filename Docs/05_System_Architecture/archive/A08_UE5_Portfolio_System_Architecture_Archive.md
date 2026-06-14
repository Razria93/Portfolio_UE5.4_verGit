# Combat Feedback 모델

## 1. 목적

본 문서는 combat feedback 계층에서 `ActionFeedback`, `ReactionFeedback`, `DamageFeedback`, `PlayerFeedback`, `CombatFeedbackSubsystem`이 각각 어떤 책임을 갖는지 정리하기 위한 문서임.

이번 reaction orchestration 작업에서는 feedback 책임도 함께 재정리됨.

특히 기존에는 damage hit feedback과 reaction feedback의 의미가 섞이기 쉬웠음.

그러나 현재 구조에서는 hit 위치 기반 feedback과 reaction 실행 기반 feedback을 분리하는 것이 더 적절함.

본 문서는 feedback 컴포넌트들의 기준을 명확히 하여 이후 action feedback 개선과 combat feedback 확장 작업의 기준으로 삼는 것을 목적으로 함.

---

## 2. Feedback 계층 구분

현재 combat feedback은 다음과 같이 나눌 수 있음.

```text
ActionFeedback
-> action executor 중심 feedback

ReactionFeedback
-> reaction executor 중심 feedback

DamageFeedback
-> damage packet 중심 feedback

PlayerFeedback
-> player presentation 중심 feedback

CombatFeedbackSubsystem
-> world-level feedback 실행 지원
```

핵심은 feedback을 발생시킨 원인이 아니라, feedback이 어떤 context를 기준으로 해석되는지를 기준으로 나누는 것임.

ActionFeedback은 action execution context를 기준으로 함.

ReactionFeedback은 reaction execution context를 기준으로 함.

DamageFeedback은 take damage packet과 damage impact metadata를 기준으로 함.

---

## 3. ActionFeedback

`ActionFeedback`은 action executor 중심 feedback임.

주요 기준은 다음과 같음.

```text
ActionType
ActionIndex
Timing
TriggerKey
```

ActionFeedback은 action montage notify 또는 action execution event에서 요청됨.

따라서 action 내부 타이밍과 직접 연결됨.

예시는 다음과 같음.

```text
Action start feedback
Action notify point feedback
Action notify window feedback
Action completed feedback
```

ActionFeedback은 action의 표현을 담당하지만, damage가 실제로 들어갔는지 여부를 기준으로 하지 않음.

즉 attack swing VFX와 실제 hit VFX는 같은 feedback 계층이 아님.

---

## 4. ReactionFeedback

`ReactionFeedback`은 reaction executor 중심 feedback임.

주요 기준은 다음과 같음.

```text
ReactionType
ApplyDamageSpecKey
ReactionFeedbackTiming
TriggerKey
```

ReactionFeedback은 `CReaction`이 자신의 active reaction context를 기반으로 요청함.

따라서 reaction montage, reaction finish, reaction notify timing과 직접 연결됨.

예시는 다음과 같음.

```text
ReactionStart
ReactionCompleted
ReactionInterrupted
ReactionCancelled
WindowBegin
WindowEnd
Notify
```

ReactionFeedback은 hit feedback이 아님.

ReactionFeedback은 피격 이후 실행되는 reaction 표현을 담당함.

따라서 hit impact point나 hit normal을 기준으로 feedback을 고르는 책임은 ReactionFeedback에 두지 않음.

---

## 5. DamageFeedback

`DamageFeedback`은 take damage packet 중심 feedback임.

주요 기준은 다음과 같음.

```text
FTakeDamagePacket
FTakeDamageContext
FTakeDamageResult
FDamageImpactInfo
```

DamageFeedback은 damage가 실제로 accepted / committed 된 이후 실행되는 feedback에 적합함.

예시는 다음과 같음.

```text
Hit VFX
Hit SFX
Hit stop
Camera shake
Damage number
Impact decal
```

현재 `FDamageImpactInfo`를 직접 소비하는 책임은 DamageFeedback에 두는 것이 적절함.

이는 hit 위치와 hit normal이 damage event metadata에 속하기 때문임.

ReactionFeedback이 reaction execution timing을 표현한다면, DamageFeedback은 damage event 자체의 즉각적인 피드백을 표현함.

---

## 6. PlayerFeedback

`PlayerFeedback`은 player presentation 중심 feedback임.

이는 combat event 자체보다 player가 체감해야 하는 화면 / 카메라 / 입력 / UI 반응에 가까움.

예시는 다음과 같음.

```text
local camera shake
local hit stop response
controller vibration
screen effect
player-only UI feedback
```

PlayerFeedback은 combat domain feedback과 겹칠 수 있지만, 책임 기준이 다름.

DamageFeedback은 damage event를 기준으로 함.

PlayerFeedback은 local player presentation을 기준으로 함.

따라서 multiplayer나 AI actor까지 고려하면 PlayerFeedback을 별도 계층으로 유지하는 것이 더 명확함.

---

## 7. CombatFeedbackSubsystem

`CombatFeedbackSubsystem`은 world-level feedback 실행 지원 계층임.

개별 action / reaction / damage component가 직접 모든 feedback 실행 방식을 구현하면 중복이 커짐.

따라서 공통 feedback 실행 기능은 subsystem으로 분리할 수 있음.

예시는 다음과 같음.

```text
hit stop audience 적용
camera shake audience 적용
world VFX spawn 지원
world SFX spawn 지원
feedback 중복 실행 제어
```

Subsystem은 feedback 요청의 의미를 판단하는 계층이 아님.

Subsystem은 이미 해석된 feedback request를 실행하는 지원 계층임.

---

## 8. Feedback Request 생성 위치

Feedback request는 해당 execution context를 가장 잘 아는 객체에서 생성하는 것이 적절함.

권장 기준은 다음과 같음.

```text
CAction
-> action feedback request 생성

CReaction
-> reaction feedback request 생성

TakeDamage / DamageFeedback
-> damage feedback request 생성

PlayerFeedback
-> local player feedback request 생성
```

Component는 필요한 경우 bridge 역할을 수행할 수 있음.

예를 들어 reaction notify는 `ReactionComponent`를 통해 active reaction executor로 전달될 수 있음.

그러나 실제 reaction feedback request는 `CReaction`이 생성하는 것이 적절함.

이는 executor가 현재 실행 context와 timing을 가장 정확히 알고 있기 때문임.

---

## 9. ReactionFeedback과 DamageFeedback의 차이

ReactionFeedback과 DamageFeedback은 모두 피격 상황에서 발생할 수 있으나, 기준 context가 다름.

```text
DamageFeedback
-> damage event가 발생했는가
-> 어디에 맞았는가
-> 얼마의 피해가 적용되었는가
-> hit impact metadata가 무엇인가

ReactionFeedback
-> 어떤 reaction이 실행 중인가
-> reaction이 어떤 timing에 도달했는가
-> reaction notify trigger key가 무엇인가
-> reaction type / damage spec 기준으로 어떤 표현을 쓸 것인가
```

따라서 sword hit VFX는 DamageFeedback에 가깝고, hit reaction montage 중 posture break dust는 ReactionFeedback에 가까움.

이 둘을 분리하면 hit event 표현과 reaction execution 표현을 독립적으로 튜닝할 수 있음.

---

## 10. 결론

Combat feedback은 하나의 거대한 feedback component로 합치는 것보다 context 기준으로 나누는 것이 적절함.

```text
ActionFeedback
-> action execution 표현

ReactionFeedback
-> reaction execution 표현

DamageFeedback
-> damage event 표현

PlayerFeedback
-> local player presentation 표현

CombatFeedbackSubsystem
-> 공통 실행 지원
```

이 구조를 유지하면 hit feedback과 reaction feedback이 섞이지 않고, 각 feedback은 자신이 필요한 context만 의존할 수 있음.

따라서 현재 reaction orchestration 브랜치의 feedback 구조는 실행 context 기준으로 나뉘는 방향이 타당함.
