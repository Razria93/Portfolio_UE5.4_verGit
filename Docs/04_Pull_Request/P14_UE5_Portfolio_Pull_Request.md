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

### 작업 요약

본 PR은 damage result와 action timing을 실제 체감 feedback으로 연결하기 위해 feedback 실행 흐름을 구성한 작업이다.

### 작업 배경

P14 이전에는 전투 feedback을 처리하는 별도 구조가 없었음.

따라서 해당 브랜치에서 다음 feedback 기능을 구현했다.

```yaml
- action 시 timing에 맞춰 발생하는 action VFX / action SFX / trail
- hit 결과를 기준으로 발생하는 hit VFX / hit SFX / hit stop
- hit 결과에 따라 발생하는 camera shake
```

이 중 camera shake는 hit 결과에서 발생하지만 실제 실행 대상이 PlayerController / Camera에 가까우므로, shared hit feedback과 분리된 player-local feedback으로 다루도록 정리했다.

또한 Player와 Enemy가 같은 action notify를 사용하더라도, 각자의 runtime context를 기준으로 feedback request를 생성할 수 있도록 공통 request 생성 흐름이 필요했다.

### 구현 방향

이를 다음 네 가지 축으로 정리했다.

```yaml
1. Feedback 계층 분리
- action-feedback / reaction-feedback / player-feedback 역할 구분

2. Action feedback request 경로 정리
- Player와 Enemy가 공통 notify 경로를 사용할 수 있도록 request 생성 방식 정리

3. Shared feedback과 local feedback 분리
- hit VFX / hit SFX / hit stop은 shared hit feedback으로 처리
- camera shake는 player-local feedback으로 분리

4. Enemy attack-end cleanup 보완
- notify는 종료 signal로 축소
- 실제 cleanup은 BT task 흐름으로 이관
```

---
## 변경 범위

### Combat Feedback Pipeline

#### A. Feedback 계층 분리

- 전투 feedback을 실행 기준에 따라 action-feedback, reaction-feedback, player-feedback으로 구분함.
- P14 기준 reaction-feedback은 hit 결과 기반 feedback을 포함했다.

**Structure**
```yaml
Reaction Feedback
- 기준     : damage result
- 처리 대상 : hit VFX / hit SFX / hit stop

Action Feedback
- 기준     : action montage timing
- 처리 대상 : action VFX / action SFX / trail

Player Feedback
- 기준     : player-local presentation
- 처리 대상 : camera shake
```

#### B. Action Feedback 연결

- action montage timing을 기준으로 trail, VFX, SFX feedback을 실행할 수 있도록 action-feedback 경로를 구성했다.

**Flow**
```yaml
Action Montage Notify / NotifyState
-> Action Feedback Request
-> UCActionFeedbackComponent
-> Trail / VFX / SFX
```

**Structure**
```yaml
UCActionFeedbackComponent
- action feedback request 처리
- Trail on / off
- action VFX / SFX 실행
- ActionStart / ActionEnd timing feedback dispatch

Action Feedback Data
- FActionFeedbackRequest
- FActionFeedbackKey
- EActionFeedbackTiming
```

#### C. Reaction Feedback 연결

- `TakeDamage` 이후 damage result를 기반으로 hit feedback이 실행되도록 reaction-feedback 경로를 연결했다.

**Flow**
```yaml
TakeDamage
-> Damage Result
-> Reaction Feedback Request
-> UCReactionFeedbackComponent
-> Hit VFX / Hit SFX / HitStop
```

**Structure**
```yaml
UCReactionFeedbackComponent
- TakeDamage 이후 reaction-feedback dispatch
- hit VFX / hit SFX / hit stop 실행
- Player-side / Enemy-side 공통 feedback 처리
```

#### D. Player Local Feedback 분리

- camera shake처럼 특정 Player에게만 적용되는 feedback을 player-feedback 계층으로 분리했다.

**Flow**
```yaml
Damage Result
-> Player Feedback Request
-> UCPlayerFeedbackComponent
-> CameraShake
```

**Structure**
```yaml
UCPlayerFeedbackComponent
- Player local feedback 처리
- camera shake 실행
- shared hit feedback과 player-local feedback 책임 분리
```

#### E. Player / Enemy 공통 Action Feedback Request 경로

- Player와 Enemy가 같은 action feedback notify를 사용할 수 있도록 owner-level request provider 흐름을 구성했다.

**Flow**
```yaml
Action Notify
-> Owner-level Feedback Request Provider
-> Player / Enemy runtime context 기반 request 생성
-> UCActionFeedbackComponent
```

**Structure**
```yaml
ActionFeedbackRequestProvider
- Player와 Enemy가 각자의 runtime context로 request 생성
- notify는 공통 trigger 역할만 수행
- UCAction 직접 참조 의존도 감소
```

### AI Attack Lifecycle Cleanup

#### F. Enemy Attack-End Cleanup 정리

- Enemy attack 종료 처리를 notify 직접 cleanup에서 BT task 기반 cleanup으로 이관했다.

**Flow**
```yaml
AnimNotify_EndEnemyAttack
-> attack end signal
-> UCBTTask_EndAttack
-> enemy attack-end cleanup
```

**Structure**
```yaml
UCBTTask_EndAttack
- 정상 attack-end cleanup 담당
- attack state / blackboard cleanup

State Transition Cleanup
- 예상하지 못한 state 이탈에 대한 safety-net 역할 유지
```

---
## 주요 Pipeline

### Reaction Feedback Pipeline

```yaml
TakeDamage
-> Damage Result
-> ReactionFeedbackRequest
-> UCReactionFeedbackComponent
-> Hit VFX / Hit SFX / HitStop
```

### Action Feedback Pipeline

```yaml
Action Notify / NotifyState
-> ActionFeedbackRequest
-> UCActionFeedbackComponent
-> Trail / VFX / SFX
```

### Player Local Feedback Pipeline

```yaml
Damage Result
-> PlayerFeedbackRequest
-> UCPlayerFeedbackComponent
-> CameraShake
```

### Enemy Attack-End Cleanup Pipeline

```yaml
AnimNotify_EndEnemyAttack
-> EndAttack signal
-> UCBTTask_EndAttack
-> attack-end cleanup
```

---
## 테스트 방법

### Reaction Feedback

- Player-side / Enemy-side에서 hit VFX, hit SFX, hit stop이 정상 실행되는지 확인
- `TakeDamage` 이후 reaction-feedback request가 공통 feedback 경로로 전달되는지 확인

### Player Local Feedback

- Player-side에서 camera shake가 player-local feedback으로 실행되는지 확인
- Enemy-side feedback 실행 시 불필요한 player-local feedback이 실행되지 않는지 확인

### Action Feedback

- `ActionStart`, `ActionEnd`, `TriggerOnce`, `TriggerWindowBegin`, `TriggerWindowEnd` timing에서 action feedback이 실행되는지 확인
- `AnimNotifyState_ActionFeedback` 기반 Trail on / off가 정상 동작하는지 확인
- `AnimNotify_ActionFeedback` 기반 Sword SFX, Buff VFX, Buff SFX가 정상 실행되는지 확인

### Player / Enemy 공통 경로

- Player와 Enemy가 동일한 action-feedback notify 경로를 사용할 수 있는지 확인
- Player / Enemy runtime context 기반으로 action feedback request가 생성되는지 확인

### Enemy Attack Cleanup

- Enemy BT attack flow가 정상 순서로 종료되는지 확인
- 정상적인 attack-end cleanup이 `UCBTTask_EndAttack`에서 처리되는지 확인
- 예상하지 못한 state 이탈 상황에서 state transition cleanup이 safety-net으로 동작하는지 확인

---
## 검증 결과

- Player / Enemy hit feedback 정상 동작 확인
- Player camera shake local feedback 정상 동작 확인
- action trail / SFX / buff feedback 정상 동작 확인
- Player / Enemy 공통 action feedback notify 경로 동작 확인
- Enemy attack-end cleanup 경로 정상 동작 확인

---
## 관련 문서

- Issue Checklist: `D15_UE5_Portfolio_Issue_Checklist.md`

---
## 정리

이 PR의 핵심은 feedback 기능을 단순히 추가한 것이 아니라, damage result와 action timing이 실제 체감 feedback으로 이어지는 경로를 계층별로 분리한 것이다.

변경 후에는 reaction-feedback, action-feedback, player-feedback이 서로 다른 기준으로 처리되며, shared combat feedback과 player-local feedback도 분리됐다.

또한 Player와 Enemy가 공통 action feedback notify 경로를 사용할 수 있도록 request 생성 방식을 정리하여, feedback 실행이 특정 `UCAction` 문맥에 과하게 결합되지 않도록 개선했다.

Enemy attack-end cleanup도 notify 직접 처리에서 BT task 기반 cleanup으로 이관하여, feedback timing과 AI attack lifecycle cleanup의 책임을 분리했다.

---
