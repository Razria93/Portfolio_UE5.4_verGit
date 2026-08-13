# Combat Feedback 계층 구성

## 1. 제목

M04-S09: Combat Feedback 계층 구성

---
## 2. 목적

본 문서는 `feature/combat-feedback` 브랜치에서 combat feedback을 계층화한 이유와 구성 기준을 정리하기 위한 문서임.

핵심은 damage result와 action montage timing에서 발생하는 feedback을 실제 체감 표현으로 연결하고, shared feedback과 player-local feedback의 실행 책임을 분리하는 것임.

---
## 3. 관련 브랜치

- `feature/combat-feedback`

### 문서 기준 및 후속 구조 안내

- 후속 관련 브랜치: `feature/reaction-orchestration`
- 후속 관련 문서: `S17`

본 문서는 `feature/combat-feedback` 기준의 combat feedback 계층 구성 문서임.

본문의 `ReactionFeedback`은 후속 구조에서 말하는 순수 reaction execution timing feedback이 아니라, 해당 브랜치에서 `TakeDamage` 이후 hit result 기반 feedback까지 포함하던 feedback 계층을 의미함.

이후 `feature/reaction-orchestration`에서 hit result 기반 feedback은 `DamageFeedback`으로 분리되고, `ReactionFeedback`은 reaction execution timing 기준으로 재정의되었으며, 자세한 내용은 `S17`에서 다룸.

Enemy 사망 제거 연출은 이 Combat Feedback 계층의 후속 적용 사례다. `UCCharacterFeedbackComponent`가 Blueprint 표현 요청과 결과 통지 경계를 제공하지만, Dead 상태와 Actor Destroy 권한은 소유하지 않는다. 최신 책임 계약은 [S31 Enemy Dead / Presentation / Destroy 생명주기 설계](S31_UE5_Portfolio_System_Architecture.md)를 기준으로 한다.

---
## 4. 기존 시스템의 형태

### Combat Feedback 계층 부재

`feature/combat-feedback` 이전에는 combat feedback을 담당하는 별도 계층이 명확히 분리되어 있지 않았음.

전투 중 필요한 표현은 있었지만, 어떤 event와 context를 기준으로 실행되는지 구분하는 구조가 부족했음.

```yaml
기존 feedback 성격
- damage result 이후 hit 표현
- action montage timing 기반 표현
- local player presentation
```

### Feedback 발생 지점

전투에서 필요한 feedback은 크게 다음 지점에서 발생함.

```yaml
Damage Result 기반 feedback
- hit VFX
- hit SFX
- hit stop
- camera shake request 생성

Action Timing 기반 feedback
- action VFX
- action SFX
- weapon trail

Player Local 기반 feedback
- camera shake 실행
```

이 feedback들은 모두 combat 중에 발생하지만, 실행 기준과 소비 대상이 같지 않음.

---
## 5. Feedback 계층 도입 시 분리해야 할 기준

### Feedback 실행 기준 분리

`feature/combat-feedback` 이전에는 combat feedback 계층이 없었기 때문에, 새 feedback 구조를 만들 때 먼저 실행 기준을 분리해야 함.

전투 중 발생하는 feedback은 모두 체감 표현으로 보일 수 있지만, 실제 실행 기준은 같지 않음.

```yaml
Feedback 분리 기준
- Damage Result 기준
  -> damage accepted / committed 여부
  -> hit VFX / hit SFX / hit stop

- Action Timing 기준
  -> action montage timing
  -> action VFX / action SFX / weapon trail

- Shared Feedback 기준
  -> source / target / both audience
  -> hit stop

- Player Local 기준
  -> local player controller / camera
  -> camera shake
```

이 기준을 분리하지 않고 하나의 feedback 흐름으로 만들면, hit feedback, action feedback, player-local feedback을 같은 규칙으로 확장해야 하는 문제가 생김.

### Action Feedback 공통 경로 필요

Action feedback은 Player와 Enemy가 모두 사용할 수 있어야 함.

다만 Player와 Enemy는 같은 action montage notify를 사용하더라도, 각자가 가진 runtime context는 다름.

```yaml
ActionFeedback에 필요한 context
- 실행 중인 action
- action data key
- action feedback timing
- trigger key
- owner actor
```

따라서 notify가 특정 `UCAction` 또는 특정 owner type에 강하게 결합되지 않고, owner runtime context를 기준으로 request를 구성할 수 있어야 함.

### Local Feedback 책임 분리 필요

camera shake는 hit 결과에서 발생할 수 있지만, 실제 실행 대상은 local player controller / camera에 가까움.

따라서 hit VFX / SFX / hit stop 같은 shared combat feedback과 같은 방식으로 직접 실행하지 않고, player-local feedback으로 분리할 필요가 있음.

```yaml
Shared Feedback
- hit VFX
- hit SFX
- hit stop

Local Feedback
- camera shake
```

---
## 6. 설계 방향 및 내용

### Combat Feedback 계층 구성

combat feedback은 실행 기준에 따라 다음 계층으로 분리함.

```yaml
ActionFeedback
- action montage timing 기반 feedback
- action VFX / SFX / trail 처리

ReactionFeedback
- damage result 이후 hit feedback 처리
- hit VFX / hit SFX / hit stop 처리

PlayerFeedback
- local player presentation feedback
- camera shake 실행

CombatFeedbackSubsystem
- world-level feedback request routing
- hit stop / camera shake request 전달 지원
```

### ActionFeedback Pipeline

action montage timing에 맞춰 action feedback request를 만들고, feedback component에서 trail / VFX / SFX를 실행함.

```yaml
Action Notify / NotifyState
-> ActionFeedbackRequest
-> UCActionFeedbackComponent
-> Trail / VFX / SFX
```

```yaml
UCActionFeedbackComponent
- action feedback request 처리
- action VFX / SFX 실행
- trail on / off 처리
- action feedback timing dispatch
```

### ReactionFeedback Pipeline

`TakeDamage` 이후 damage result를 기반으로 hit feedback을 실행하는 경로를 구성함.

```yaml
TakeDamage
-> Damage Result
-> ReactionFeedbackRequest
-> UCReactionFeedbackComponent
-> Hit VFX / Hit SFX / HitStop
```

```yaml
UCReactionFeedbackComponent
- TakeDamage 이후 hit feedback dispatch
- hit VFX / hit SFX / hit stop 실행
- Player-side / Enemy-side 공통 feedback 처리
```

### PlayerFeedback Pipeline

camera shake처럼 local player presentation이 필요한 feedback은 player-local 경로로 분리함.

```yaml
Damage Result
-> PlayerFeedbackRequest
-> UCPlayerFeedbackComponent
-> CameraShake
```

```yaml
UCPlayerFeedbackComponent
- local player controller 기준 feedback 소비
- camera shake 실행
- shared feedback과 player-local feedback 책임 분리
```

### Player / Enemy 공통 ActionFeedback Request

Player와 Enemy가 같은 action feedback notify를 사용하더라도 각자의 runtime context로 request를 만들 수 있도록 owner-level request 생성 흐름을 둠.

```yaml
Action Notify
-> Owner-level feedback request provider
-> Player / Enemy runtime context 기반 request 생성
-> UCActionFeedbackComponent
```

notify는 timing trigger 역할에 집중하고, 실제 request 구성은 owner runtime context 기준으로 처리함.

---
## 7. 결과

이 작업으로 combat feedback은 다음 기준으로 정리됨.

```yaml
ActionFeedback
- action montage timing 기준

ReactionFeedback
- P14 기준 hit result feedback 포함

PlayerFeedback
- local player presentation 기준

CombatFeedbackSubsystem
- world-level feedback routing 기준
```

이를 통해 action timing feedback, hit feedback, local player feedback이 서로 다른 실행 기준을 가질 수 있게 됨.

또한 Player와 Enemy가 공통 notify를 사용하더라도 각자의 runtime context로 action feedback request를 만들 수 있는 구조가 마련됨.

---
## 8. 관련 문서

이 문서는 아래 문서들과 같은 작업 시점 기준으로 함께 읽을 수 있음.

### 같은 작업 단위

- `D15`
- `P14`

---
## 9. 결론

`feature/combat-feedback`의 핵심은 combat feedback을 단일 처리 흐름으로 두지 않고, action timing, hit result, local player presentation 기준으로 분리한 것임.

이 작업을 통해 action montage timing 기반 feedback과 player-local feedback의 실행 경로가 정리됨.

또한 Player와 Enemy가 같은 action feedback notify를 사용하더라도 각자의 runtime context를 기준으로 feedback request를 만들 수 있는 구조가 마련됨.

---
