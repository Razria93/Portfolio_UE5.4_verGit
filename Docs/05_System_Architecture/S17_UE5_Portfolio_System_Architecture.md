# Damage Feedback과 Reaction Feedback 책임 재정의

## 1. 제목

M06-S17: Damage Feedback과 Reaction Feedback 책임 재정의

---

## 2. 목적

본 문서는 `feature/reaction-orchestration` 브랜치에서 `DamageFeedback`과 `ReactionFeedback`의 책임을 다시 분리한 이유와, `FDamageImpactInfo`를 통해 hit 위치 metadata를 전달하는 구조를 정리하기 위한 문서임.

핵심은 `feature/combat-feedback`에서 1차 구성된 feedback 계층 중 hit result 기반 feedback과 reaction execution timing 기반 feedback을 서로 다른 context로 재정의하는 것임.

---
## 3. 관련 브랜치

- `feature/reaction-orchestration`

---
## 4. 기존 시스템의 형태

### CombatFeedback 1차 구성 이후 상태

`feature/combat-feedback` 이후에는 action feedback, reaction feedback, player feedback 계층이 1차로 구성되어 있었음.

```yaml
ActionFeedback
- action montage timing 기반 feedback

ReactionFeedback
- TakeDamage 이후 hit result 기반 feedback 포함

PlayerFeedback
- local player presentation feedback
```

이 구조는 combat feedback을 처음 계층화하는 데는 유효했지만, `ReactionFeedback`이라는 이름 안에 hit result 기반 feedback이 포함되어 있었음.

### Hit feedback 중심의 ReactionFeedback 처리

분리 이전의 `ReactionFeedback`은 reaction montage timing feedback보다 hit result 기반 feedback에 가까운 책임을 함께 가지고 있었음.

```yaml
기존 ReactionFeedback 책임
- hit VFX
- hit SFX
- hit stop
- camera shake request
```

### Overlap 기반 hit detection

기존 melee hit detection은 weapon collision overlap 기반으로 동작함.

```yaml
Weapon collision overlap
-> OnComponentBeginOverlap
-> hit context 구성
-> ApplyDamage / TakeDamage pipeline
```

이 방식은 collision window 안에서 타격 여부를 감지하는 데 사용할 수 있음.

다만 overlap 기반 구조는 항상 정확한 impact point를 제공하지 않으므로, hit feedback 위치 metadata를 별도로 정리할 필요가 생김.

---
## 5. 기존 시스템의 문제 분석 및 한계

### Hit Feedback과 Reaction Feedback 기준 혼재

피격 상황에서는 여러 feedback이 같은 순간에 발생할 수 있음.

```yaml
피격 상황에서 함께 발생할 수 있는 feedback
- hit VFX
- hit SFX
- hit stop
- camera shake request / local camera shake
- reaction montage timing feedback
- reaction start / complete / interrupted feedback
```

이 feedback들은 모두 combat 중에 발생하지만, 실행 기준이 같지 않음.

```yaml
DamageFeedback으로 분리해야 하는 기준
- damage event accepted / committed 여부
- hit impact point
- hit impact normal / direction
- TakeDamagePacket context

ReactionFeedback으로 분리해야 하는 기준
- active reaction type
- reaction execution timing
- trigger key
- reaction lifecycle event
```

단일 component 또는 단일 request context로 처리하면 다음 책임이 섞임.

```yaml
섞이는 책임
- damage result 해석
- hit 위치 metadata 해석
- reaction lifecycle 해석
- reaction montage timing 해석
```

hit VFX / hit SFX / hit stop은 damage event 결과와 impact metadata를 기준으로 해석하는 것이 자연스러움.

reaction montage 중 발생하는 reaction VFX / SFX는 reaction execution timing을 기준으로 해석하는 것이 자연스러움.

camera shake는 hit 결과에서 request가 생성될 수 있지만, 최종 실행 대상은 local player controller / camera에 가까우므로 `PlayerFeedback` 경로로 분리하는 것이 자연스러움.

### Hit 위치 metadata 부재

Overlap 기반 collision은 타격 여부를 감지할 수 있지만, 항상 유효한 hit 위치를 제공하지는 않음.

```yaml
Overlap 기반 hit detection
- weapon collision overlap으로 타격 여부 감지
- socket / attachment / animation transform에 의해 weapon 이동
- sweep 기반 충돌이 아니면 bFromSweep == false 발생 가능
- bFromSweep == false이면 SweepResult의 impact point를 신뢰하기 어려움
```

이 상태에서 위치가 중요한 feedback을 실행하면 다음 문제가 생김.

```yaml
Hit 위치 metadata가 없을 때
- hit VFX / hit SFX 위치를 actor location에 의존함
- 실제 충돌 위치와 feedback 위치가 어긋날 수 있음
- damage pipeline 안에서 collision 위치를 다시 계산하려는 압력이 생김
```

따라서 actor location보다 나은 hit position을 제공하면서도, overlap 기반 구조를 유지할 수 있는 fallback 기준이 필요함.

### Damage Pipeline의 Collision 계산 책임 증가

hit 위치 metadata가 없으면 `ApplyDamage / TakeDamage` 계층이 collision 정보를 다시 계산하는 방향으로 책임이 밀릴 수 있음.

```yaml
Damage pipeline의 본래 책임
- damage request 전달
- damage context 구성 / 해석
- damage result 산출
- health commit 이후 흐름 연결

Damage pipeline에 넣으면 안 되는 책임
- collision 위치 재계산
- weapon contact point 추정
- hit normal / impact point 산출
```

damage pipeline이 collision 위치 계산까지 담당하면 hit detection 책임과 damage 처리 책임이 섞임.

---
## 6. 리팩터링 방향 및 내용

### Feedback 책임 재정의

리팩터링 방향은 feedback을 “피격 표현”이라는 하나의 범주로 묶지 않고, feedback이 소비하는 context 기준으로 분리하는 것임.

```yaml
DamageFeedback
- damage event / damage impact metadata 기준
- hit VFX / hit SFX / hit stop 처리
- camera shake request 생성

ReactionFeedback
- reaction execution context / montage timing 기준
- reaction VFX / reaction SFX / reaction lifecycle feedback 처리

PlayerFeedback
- camera shake처럼 local player presentation이 필요한 feedback

CombatFeedbackSubsystem
- world-level feedback execution / local feedback routing 지원
```

이 문서의 중심은 `DamageFeedback`과 `ReactionFeedback`의 분리임.

`DamageFeedback`은 damage가 발생한 순간의 표현을 담당하고, `ReactionFeedback`은 reaction executor가 실행 중일 때 montage timing에 맞춘 표현을 담당함.

camera shake처럼 damage result에서 request가 생성되지만 local player controller / camera에서 소비되어야 하는 feedback은 `CombatFeedbackSubsystem`과 `PlayerFeedback` 경로로 분리함.

### Component 책임 변경

기존 hit feedback 성격의 `UCReactionFeedbackComponent`는 `UCDamageFeedbackComponent`로 재정의함.

이후 새로운 `UCReactionFeedbackComponent`를 추가하여 reaction execution timing 기반 feedback을 담당하도록 정리함.

```yaml
Before
UCReactionFeedbackComponent
- hit VFX
- hit SFX
- hit stop
- camera shake request

After
UCDamageFeedbackComponent
- TakeDamagePacket / DamageImpactInfo 기반 hit feedback
- hit VFX / hit SFX / hit stop
- camera shake request 구성

UCReactionFeedbackComponent
- active reaction context 기반 feedback
- reaction timing / trigger key 기반 VFX / SFX
```

### DamageImpactInfo 전달 구조

`FDamageImpactInfo`는 damage result가 아니라 damage event에 동반되는 impact metadata임.

이 구조의 목적은 `DamageFeedback`이 actor location이 아니라 hit impact point에 가까운 위치에서 VFX / SFX를 실행할 수 있게 하는 것임.

책임은 다음처럼 분리함.

```yaml
ACWeaponActor
- overlap / sweep 정보를 기반으로 FDamageImpactInfo 생성

ApplyDamage / TakeDamage pipeline
- FDamageImpactInfo를 계산하지 않고 payload / context / packet으로 전달

UCDamageFeedbackComponent
- FTakeDamagePacket.Context.DamageImpactInfo 소비
- feedback 위치와 방향 결정
- hit stop / camera shake request 구성
```

이렇게 하면 hit detection 계층이 impact metadata를 만들고, damage pipeline은 전달만 담당함.

`UCDamageFeedbackComponent`는 hit feedback 위치와 방향을 결정하고, local feedback이 필요한 camera shake는 `CombatFeedbackSubsystem`을 통해 player-local 경로로 전달함.

### Impact Point 계산 기준

현재 overlap 기반 hit detection에서는 다음 순서로 impact point를 결정함.

```yaml
Impact Point Resolve
1. bFromSweep == true이면 SweepResult 우선 사용
2. SweepResult가 유효하지 않으면 GetClosestPointOnCollision fallback 사용
```

`SweepResult`가 유효하면 엔진이 제공한 hit point / impact normal을 우선 사용함.

`SweepResult`가 유효하지 않으면 target collision 기준의 closest point를 fallback으로 사용함.

```yaml
Closest Point Fallback
Weapon collision center
-> target collision surface closest point
```

이 fallback은 정확한 blade contact point를 보장하지 않음.

다만 actor location에 VFX를 실행하는 것보다 자연스러운 hit position을 제공하는 현재 overlap 구조 안의 근사값임.

---
## 7. 이후 작업의 방향성

### Trail Trace 기반 Hit Detection 검토

Weapon Trail Trace는 현재 구현 범위에 포함하지 않음.

Trail trace는 단순히 VFX 위치를 보정하는 작업이 아니라, melee hit detection model과 hit metadata 생성 방식을 바꾸는 작업에 가까움.

```yaml
Trail Trace Model
- weapon sample point 구성
- previous position / current position 저장
- sample point 사이 trace
- trace result에서 impact point / normal / bone / surface data 획득
```

따라서 현재 단계에서는 overlap 기반 hit detection을 유지하고, `FDamageImpactInfo`와 closest point fallback을 1차 기준으로 사용함.

Trail trace는 다음 조건이 필요해질 때 별도 hit detection model로 검토함.

```yaml
Trail Trace 검토 조건
- guard / parry / weapon clash가 정확한 weapon contact direction에 의존함
- hit normal과 weapon swing direction이 gameplay 판정에 사용됨
- bone / physical material / surface 기반 feedback이 필요함
- fast swing에서 overlap보다 안정적인 contact point가 필요함
```

### ReactionFeedback Impact Context 확장 검토

현재 기준에서 `FDamageImpactInfo`는 `DamageFeedback`의 입력임.

`ReactionFeedback`은 `FDamageImpactInfo`를 직접 소비하지 않음.

따라서 reaction montage timing feedback에서 hit 위치가 필요해지는 경우에만 reaction context에 impact metadata를 포함할지 별도로 검토함.

### Feedback Data / Authoring 정리

feedback 구조가 분리되면 data 작성 기준도 함께 정리할 필요가 있음.

```yaml
Feedback authoring 정리 대상
- DamageFeedback data 구조 분리 검토
- ReactionFeedback / DamageFeedback authoring workflow 정리
- hit normal 방향 정책 정의
```

### Character Death Presentation 경계

현재 Enemy 사망의 Dissolve와 Destroy 대기는 DamageFeedback이나 ReactionFeedback의 책임이 아니다. DeadIn Reaction은 사망 진입 연출만 실행하고, DeadIn 완료 뒤 캐릭터·장착 무기의 Dissolve를 시작하고 완료를 통지하는 책임은 `UCCharacterFeedbackComponent`에 둔다.

```yaml
ReactionFeedback
- DeadIn 실행 중 montage timing feedback

CharacterFeedback
- DeadIn 완료 뒤 Death Presentation 요청
- Character / Skeletal Weapon Dissolve 표현
- Started / Unavailable / Finished 결과 통지

ACEnemy
- Death Presentation 결과에 따른 fallback 또는 Destroy 결정
```

이 경계의 최신 생명주기 계약과 Blueprint 연결 순서는 [S31 Enemy Dead / DeadIn / DeadLoop / Death Presentation / Destroy 생명주기](S31_UE5_Portfolio_System_Architecture.md)를 기준으로 한다.

---
## 8. 관련 문서

이 문서는 아래 문서들과 같은 작업 시점 또는 선행 구조 기준으로 함께 읽을 수 있음.

### 같은 작업 단위

- `D17`
- `P16`

### 선행 구조

- `S09`

---

## 9. 결론

Damage feedback과 reaction feedback은 모두 피격 상황에서 발생할 수 있지만, 같은 책임이 아님.

`DamageFeedback`은 damage event와 impact metadata를 기준으로 hit feedback과 local feedback request를 구성하고, `ReactionFeedback`은 reaction executor의 lifecycle과 montage timing을 기준으로 실행됨.

이 분리를 통해 hit VFX 위치 계산, hit stop, camera shake request 같은 damage result 기반 feedback과 reaction montage timing feedback을 서로 다른 기준으로 관리할 수 있음.

또한 `FDamageImpactInfo`를 hit detection 계층에서 생성하고 damage pipeline을 통해 전달함으로써, `ApplyDamage / TakeDamage` 계층이 collision 계산 책임을 갖지 않도록 유지할 수 있음.

---
