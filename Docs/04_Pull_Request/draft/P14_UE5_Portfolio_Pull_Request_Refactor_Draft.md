# UE5 Portfolio Pull Request

## 제목

**P14: Combat Feedback Pipeline 구성 및 Feedback 책임 분리**

## 날짜

**2026.04.18**

## 상태

- [x] **완료**

---

## 브랜치

- `feature/combat-feedback`

---

## 요약

이번 PR에서는 **전투 중 발생한 피격, 공격 타이밍, 카메라 반응을 실제 체감 연출로 연결하는 기반 흐름을 구성했다.**

이를 통해 피격 표현, 액션 타이밍 표현, Player에게만 필요한 local 표현을 서로 다른 기준으로 처리할 수 있게 정리했다.

성격별 핵심 변경은 다음과 같다.

### Feature

- **ActionFeedback 실행 경로 구성**: 공격 애니메이션 타이밍에 맞춰 trail, VFX, SFX를 실행할 수 있도록 action feedback 경로를 구성했다.

- **ReactionFeedback 실행 경로 구성**: P14 기준으로 `TakeDamage` 이후 hit VFX, hit SFX, hit stop이 실행되도록 reaction feedback 경로를 연결했다.

- **PlayerFeedback 분리**: camera shake처럼 local player controller / camera 기준으로 실행되는 표현을 player-local feedback 경로로 분리했다.

### Refactoring

- **Feedback 계층 분리**: 전투 feedback을 action timing, hit result, player-local presentation 기준으로 나눴다.

- **공통 ActionFeedback Request 생성 경로 구성**: Player와 Enemy가 같은 notify를 사용하더라도 각자의 runtime context로 action feedback request를 만들 수 있도록 정리했다.

- **Enemy Attack 종료 signal 분리**: notify는 Player와 Enemy의 종료 처리 차이를 직접 판단하지 않고 공격 종료 신호만 전달하며, Enemy의 실제 공격 종료 정리는 BehaviorTree task에서 처리하도록 분리했다.

---

## 핵심 개념

아래 용어는 이후 설명을 읽기 위한 최소 용어다.

이 PR의 핵심 흐름은 전투 중 발생한 피격 결과와 action timing을 Feedback Request로 바꾸고, 실행 기준에 맞는 feedback component가 실제 체감 표현을 처리하는 구조다.

```text
Combat Feedback(전투 체감 표현)
-> 전투 결과를 VFX, SFX, trail, hit stop, camera shake 같은 실제 체감 표현으로 연결하는 계층
-> 이 PR에서는 실행 기준에 따라 ActionFeedback, ReactionFeedback, PlayerFeedback으로 나눔
```

```text
ActionFeedback(액션 타이밍 feedback)
-> action montage timing을 기준으로 실행되는 feedback
-> trail, action VFX, action SFX처럼 공격 애니메이션 타이밍에 맞춰야 하는 표현을 담당함
```

```text
ReactionFeedback(피격 결과 feedback)
-> P14 기준에서는 TakeDamage 이후 hit result를 기준으로 실행되는 feedback
-> hit VFX, hit SFX, hit stop 같은 shared hit feedback을 담당함
```

```text
PlayerFeedback(Player local feedback)
-> local player controller / camera 기준으로 실행되어야 하는 feedback
-> camera shake처럼 특정 Player에게만 적용되는 표현을 담당함
```

```text
Feedback Request(feedback 실행 요청)
-> notify나 damage result에서 만들어지는 feedback 실행 요청 단위
-> Player와 Enemy는 같은 notify를 쓰더라도 각자의 runtime context로 request를 구성함
```

---

## 변경 배경

이 섹션은 이번 PR이 필요했던 이유와 기존 구조의 문제를 정리한다.

### Combat Feedback 계층 구성 필요성

P14 이전에는 전투 중 필요한 VFX, SFX, trail, hit stop, camera shake 같은 표현을 어떤 event와 context 기준으로 실행할지 분리한 구조가 부족했다.

전투 결과와 action timing이 실제 체감 표현으로 이어지려면, 피격 결과 기반 표현과 action montage timing 기반 표현을 다른 기준으로 다룰 필요가 있었다.

### Shared Feedback과 Player Local Feedback 분리 필요성

hit VFX, hit SFX, hit stop은 Player와 Enemy 모두에게 적용될 수 있는 shared combat feedback에 가깝다.

반면 camera shake는 hit 결과에서 요청될 수 있지만 실제 실행 대상은 local player controller / camera에 가깝다.

따라서 shared hit feedback과 player-local feedback을 같은 방식으로 직접 실행하지 않고, 별도 경로로 분리할 필요가 있었다.

### Player / Enemy 공통 ActionFeedback 경로 필요성

Player와 Enemy는 같은 action notify를 사용할 수 있지만, feedback request를 만들 때 필요한 runtime context는 서로 다르다.

notify가 특정 owner type이나 `UCAction` 내부 구조에 강하게 결합되면, Player와 Enemy가 공통 notify를 사용하기 어려워진다.

따라서 notify는 timing trigger 역할에 집중하고, 실제 request 생성은 owner runtime context 기준으로 처리할 필요가 있었다.

### Enemy Attack 종료 signal 분리 필요성

Player와 Enemy는 action 종료 이후 처리 구조가 달라, notify가 두 구조의 cleanup을 직접 처리하기 어려웠다.

따라서 notify는 공격 종료 signal만 전달하고, Enemy attack-end cleanup은 BehaviorTree task 흐름에서 처리하도록 분리할 필요가 있었다.

---

## 변경 범위

이 섹션은 문제를 어떻게 고쳤고, 그 결과 동작이 어떻게 달라졌는지 정리한다.

### 1. ActionFeedback 실행 경로 구성

- **왜**:
  trail, action VFX, action SFX는 action montage timing에 맞춰 실행되어야 했다.

- **어떻게**:
  action notify와 notify state에서 action timing 기반 Feedback Request(feedback 실행 요청)를 만들고, `UCActionFeedbackComponent`로 전달해 trail / VFX / SFX를 처리하도록 구성했다.

- **결과**:
  action start, action end, trigger point, trigger window timing에 맞춰 action feedback을 실행할 수 있다.

### 2. ReactionFeedback 실행 경로 구성

- **왜**:
  `TakeDamage` 이후 hit result를 기준으로 hit VFX, hit SFX, hit stop을 실행할 공통 경로가 필요했다.

- **어떻게**:
  `TakeDamage` 이후 damage result 기반 Feedback Request(feedback 실행 요청)를 만들고, `UCReactionFeedbackComponent`로 전달해 hit VFX / hit SFX / hit stop을 처리하도록 구성했다.

- **결과**:
  Player-side와 Enemy-side에서 공통 reaction feedback 경로를 통해 hit feedback을 실행할 수 있다.

### 3. PlayerFeedback 분리

- **왜**:
  camera shake는 hit 결과에서 발생할 수 있지만, 실제 실행 대상은 local player controller / camera에 가깝다.

- **어떻게**:
  hit 결과에서 player-local 기준 Feedback Request(feedback 실행 요청)를 만들고, `UCPlayerFeedbackComponent`로 전달해 camera shake를 처리하도록 분리했다.

- **결과**:
  shared hit feedback과 player-local feedback이 서로 다른 책임으로 관리된다.

### 4. 공통 ActionFeedback Request 생성 경로 구성

- **왜**:
  Player와 Enemy가 같은 action notify를 사용하더라도, feedback request에 필요한 runtime context는 owner마다 달랐다.

- **어떻게**:
  notify는 timing trigger 역할에 집중하고, owner-level feedback request provider가 Player / Enemy runtime context를 기준으로 request를 만들도록 정리했다.

- **결과**:
  Player와 Enemy는 같은 action feedback notify 경로를 공유하면서도 각자의 실행 context에 맞는 feedback request를 만들 수 있다.

### 5. Enemy Attack 종료 signal 분리

- **왜**:
  Player와 Enemy는 action 종료 이후 처리 구조가 달라, notify가 Enemy attack-end cleanup까지 직접 처리하기 어려웠다.
  P14에서는 feedback timing 정리와 함께 공격 종료 signal과 Enemy cleanup 처리를 분리했다.

- **어떻게**:
  `AnimNotify_EndEnemyAttack`은 attack end signal만 전달하고, `UCBTTask_EndAttack`이 attack state와 Blackboard cleanup을 처리하도록 이관했다.

- **결과**:
  notify는 종료 신호를 전달하는 역할로 축소되고, 실제 Enemy attack-end cleanup은 BehaviorTree task 흐름에서 처리된다.

---

## 주요 처리 흐름

이 섹션은 주요 실행 순서와 분기 기준을 코드 구현 전에 흐름으로 먼저 설명한다.

### ActionFeedback 흐름

```text
Action Notify / NotifyState
-> Feedback Request 생성
-> ActionFeedback 진입
-> action timing 확인
-> trail / VFX / SFX 실행
```

이 흐름은 action montage timing에 맞춰 trail과 action VFX / SFX를 실행하는 과정을 의미한다.

### ReactionFeedback 흐름

```text
TakeDamage
-> Damage Result 생성
-> ReactionFeedbackRequest 생성
-> ReactionFeedback 진입
-> hit VFX / hit SFX / hit stop 실행
```

이 흐름은 P14 기준으로 피격 결과를 shared hit feedback으로 연결하는 과정을 의미한다. 이후 후속 reaction-orchestration 작업에서는 이 책임이 DamageFeedback과 ReactionFeedback으로 다시 분리된다.

### PlayerFeedback 흐름

```text
Damage Result
-> PlayerFeedbackRequest 생성
-> PlayerFeedback 진입
-> local player controller / camera 기준 확인
-> camera shake 실행
```

이 흐름은 hit 결과에서 만들어진 local 표현 요청을 특정 Player 기준으로 실행하는 과정을 의미한다.

### 공통 ActionFeedback Request 생성 흐름

```text
Action Notify
-> owner runtime context 확인
-> Player 또는 Enemy context로 Feedback Request 생성
-> ActionFeedback 진입
```

이 흐름은 Player와 Enemy가 같은 notify를 사용하더라도 각자의 runtime context를 기준으로 feedback request를 구성하는 과정을 의미한다.

### Enemy Attack 종료 signal 흐름

```text
AnimNotify_EndEnemyAttack
-> attack end signal 전달
-> UCBTTask_EndAttack 진입
-> attack state / Blackboard cleanup
```

이 흐름은 animation notify가 Player와 Enemy의 종료 처리 차이를 직접 판단하지 않고 공격 종료 신호만 전달하며, BehaviorTree task가 Enemy 공격 종료 상태를 정리하는 과정을 의미한다.

---

## 구현 결과

이 섹션은 변경 이후 시스템이 어떤 동작으로 정리됐는지 요약한다.

- Combat Feedback은 action timing, hit result, player-local presentation 기준으로 나뉜다.

- ActionFeedback은 action montage timing을 기준으로 trail, VFX, SFX를 실행한다.

- P14 기준 ReactionFeedback은 `TakeDamage` 이후 hit result를 기준으로 hit VFX, hit SFX, hit stop을 실행한다.

- PlayerFeedback은 camera shake처럼 local player 기준으로 실행되어야 하는 표현을 담당한다.

- Player와 Enemy는 같은 action feedback notify를 사용하더라도 각자의 runtime context로 Feedback Request를 만들 수 있다.

- Enemy attack 종료 notify는 signal 전달 역할로 축소되고, Enemy attack-end cleanup은 BehaviorTree task 기반 cleanup으로 정리된다.

- P14의 ReactionFeedback 의미는 후속 reaction-orchestration 작업에서 DamageFeedback과 ReactionFeedback으로 다시 분리된다.

---

## 테스트 방법

### ReactionFeedback

- Player-side / Enemy-side에서 hit VFX, hit SFX, hit stop이 정상 실행되는지 확인했다.

- `TakeDamage` 이후 reaction feedback request가 공통 feedback 경로로 전달되는지 확인했다.

### PlayerFeedback

- Player-side에서 camera shake가 player-local feedback으로 실행되는지 확인했다.

- Enemy-side feedback 실행 시 불필요한 player-local feedback이 실행되지 않는지 확인했다.

### ActionFeedback

- `ActionStart`, `ActionEnd`, `TriggerOnce`, `TriggerWindowBegin`, `TriggerWindowEnd` timing에서 action feedback이 실행되는지 확인했다.

- `AnimNotifyState_ActionFeedback` 기반 trail on / off가 정상 동작하는지 확인했다.

- `AnimNotify_ActionFeedback` 기반 Sword SFX, Buff VFX, Buff SFX가 정상 실행되는지 확인했다.

### Player / Enemy 공통 경로

- Player와 Enemy가 동일한 action feedback notify 경로를 사용할 수 있는지 확인했다.

- Player / Enemy runtime context 기반으로 action feedback request가 생성되는지 확인했다.

### Enemy Attack 종료 signal / cleanup

- Enemy BehaviorTree attack flow가 정상 순서로 종료되는지 확인했다.

- 정상적인 attack-end cleanup이 `UCBTTask_EndAttack`에서 처리되는지 확인했다.

- 예상하지 못한 state 이탈 상황에서 state transition cleanup이 safety-net으로 동작하는지 확인했다.

---

## 검증 결과

- Player / Enemy hit feedback이 정상 동작하는 것을 확인했다.

- Player camera shake local feedback이 정상 동작하는 것을 확인했다.

- action trail / SFX / buff feedback이 정상 동작하는 것을 확인했다.

- Player / Enemy 공통 action feedback notify 경로가 동작하는 것을 확인했다.

- Enemy attack-end cleanup 경로가 정상 동작하는 것을 확인했다.

---

## 관련 문서

- Issue Checklist: `D15_UE5_Portfolio_Issue_Checklist.md`

---

## 정리

P14는 전투 결과와 action timing을 실제 체감 feedback으로 연결하기 위해 Combat Feedback 계층을 1차로 구성한 PR이다.

ActionFeedback, ReactionFeedback, PlayerFeedback을 분리해 action timing 표현, hit result 표현, player-local 표현을 서로 다른 기준으로 처리할 수 있게 했다. Enemy attack 종료 notify도 signal 전달 역할로 축소해, Player / Enemy 구조 차이를 notify가 직접 떠안지 않도록 정리했다.
