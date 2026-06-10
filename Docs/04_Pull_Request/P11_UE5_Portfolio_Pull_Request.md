# UE5 Portfolio Pull Request

## 제목

**P11: Player Combat Receiver 구축 및 전투 수신 루프 연결**

## 날짜

**2026.04.01**

## 상태

- [x] **완료**

---

## 브랜치

- `feature/player-combat-receiver`

---

## 요약

### 작업 요약

본 PR은 Player를 공격자 전용 객체에서 combat receiver로 확장하고, AI 공격을 수신했을 때 `TakeDamage -> Health -> Reaction -> Dead` 루프가 동작하도록 연결한 작업이다.

### 작업 배경

이전 단계에서는 Player 공격 또는 Enemy 전투 흐름을 주로 확인했지만, Player가 Enemy 공격을 받아 damage, reaction, dead state까지 처리하는 수신 루프는 별도로 닫혀 있지 않았다.

따라서 Player에도 damage 수신 component와 health / reaction component를 연결하고, Enemy attack context가 Player 수신 흐름까지 전달되도록 보완할 필요가 있었다.

### 구현 방향

이를 다음 네 가지 축으로 정리했다.

```yaml
1. Player combat receiver 구성
- Player에 TakeDamage / Health / Reaction component 연결
- ACPlayer::TakeDamage 진입점 구성

2. Player damage-reaction loop 연결
- TakeDamage 이후 pending reaction 소비 / 실행 흐름 연결
- Player hit reaction 진입 확인

3. Health / Dead lifecycle 보완
- Health 기준 alive / dead 판단 API 추가
- DeadState 변경과 UCStateComponent sync 연결

4. Enemy attack context 전달 보완
- Enemy attack의 action type / index가 Player hit context까지 전달되도록 정리
```

---
## 변경 범위

### Player Combat Receiver

#### A. Player 수신 컴포넌트 구성

- Player가 damage를 수신하고 reaction / dead state를 처리할 수 있도록 핵심 component를 연결했다.

**Structure**
```yaml
ACPlayer
- UCTakeDamageComponent : engine TakeDamage 이후 damage 처리
- UCHealthComponent     : HP / DeadState 관리
- UCReactionComponent   : hit / dead reaction 실행
```

#### B. Player TakeDamage 진입점 연결

- `ACPlayer::TakeDamage()`를 override하고, damage 처리를 `UCTakeDamageComponent`로 위임함.

**Flow**
```yaml
Enemy Attack
-> ApplyDamage
-> ACPlayer::TakeDamage
-> UCTakeDamageComponent::RequestTakeDamage
-> Health / Reaction 후속 처리
```

#### C. Player 피격 / Reaction 루프 연결

- Player가 damage 결과를 받아 pending reaction을 소비하고 hit reaction을 실행할 수 있도록 연결했다.

**Flow**
```yaml
TakeDamage
-> Damage Result
-> Pending Reaction
-> ACPlayer::Tick
-> TryConsumePendingReaction
-> TryExecuteReaction
-> HitReact
```

**Rule**
```yaml
Hit Reaction
- damage 수신 후 pending reaction 존재
- Player alive 상태
- reaction 실행 가능 상태

Repeated Hit
- 기존 reaction과 incoming reaction의 replace / interrupt 흐름 확인
```

#### D. Health / Dead Lifecycle 보완

- Player damage 수신 이후 dead state와 gameplay state가 함께 갱신되도록 정리했다.

**Structure**
```yaml
UCHealthComponent
- IsAlive / IsDead
- ChangeDeadState
- OnDeadStateChanged.Broadcast

UCStateComponent
- OnDeadStateChanged
```

**Flow**
```yaml
BeginPlay
-> UCHealthComponent->OnDeadStateChanged.AddUObject(UCStateComponent, &UCStateComponent::OnDeadStateChanged)

UCHealthComponent
TakeDamage
-> Health 감소
-> ChangeDeadState(NewDeadState)
-> OnDeadStateChanged.Broadcast(PrevDeadState, DeadState)

UCStateComponent
-> OnDeadStateChanged(PrevDeadState, NewDeadState)
-> dead-state에 맞춰 state sync
```

#### E. Player 입력 차단 정책 연결

- Player가 reaction 또는 dead 상태일 때 action input이 들어가지 않도록 최소 입력 차단 정책을 연결했다.

**Rule**
```yaml
Allow
- alive 상태
- action input 가능 state

Reject
- Reaction state
- Dead state
- not alive

Exception
- StopJump는 release 입력이므로 별도 차단 없이 유지
```

#### F. Enemy 공격 Context 보완

- Enemy attack이 Player에게 전달될 때 action type과 index가 포함되도록 hit context 전달을 보완했다.

**Flow**
```yaml
Enemy Attack Start
-> AttackActionType / AttackIndex 결정
-> UCWeaponComponent::PushContextToAttachment
-> Attachment HitContext
-> Player TakeDamage
```

**Structure**
```yaml
Hit Context
- AttachmentType
- EquipmentType
- ActionType
- ActionIndex
```

---
## 주요 Pipeline

### Player Damage Receive Pipeline

```yaml
Enemy Attack
-> ApplyDamage
-> ACPlayer::TakeDamage
-> UCTakeDamageComponent
-> UCHealthComponent
```

### Player Reaction Pipeline

```yaml
TakeDamage Result
-> Pending Reaction
-> Player Tick
-> Consume Pending Reaction
-> UCReactionComponent
-> HitReact
```

### Player Dead State Pipeline

```yaml
Health <= 0
-> DeadState 변경
-> OnDeadStateChanged
-> UCStateComponent sync
-> Dead gameplay state
```

### Enemy Hit Context Pipeline

```yaml
Enemy Attack
-> AttackActionType / AttackIndex
-> UCWeaponComponent
-> Attachment HitContext
-> Player TakeDamage
```

---
## 테스트 방법

### Damage Receive

- Enemy가 Player를 인지하고 공격하도록 유도
- 첫 타격 시 Player HP가 감소하는지 확인
- `ACPlayer::TakeDamage -> UCTakeDamageComponent` 흐름으로 damage가 처리되는지 확인

### Reaction

- Player 피격 시 `Reaction` 상태에 진입하는지 확인
- Player `HitReact`가 정상 재생되는지 확인
- 연속 피격 시 reaction replace / interrupt 흐름이 동작하는지 확인

### Dead Lifecycle

- 누적 피격으로 HP가 0이 되면 `DeadState`가 `Alive -> Dying -> Dead`로 전이되는지 확인
- Dead 상태 진입 이후 추가 피격이 무효 처리되는지 확인

### Input Blocking

- `Reaction` 중 공격 / 장비 전환 입력이 차단되는지 확인
- `Dead` 상태에서 이동 / 액션 입력이 차단되는지 확인
- `StopJump`는 release 입력으로 정상 처리되는지 확인

### Hit Context

- Enemy 공격 로그에서 hit context가 Player 수신 흐름까지 전달되는지 확인
- `AttachmentType`, `EquipmentType`, `ActionType`, `ActionIndex`가 유효하게 기록되는지 확인

---
## 검증 결과

- Player damage receive flow 정상 동작 확인
- Player hit reaction 진입 확인
- 연속 피격 시 reaction replace / interrupt 흐름 확인
- DeadState lifecycle 및 UCStateComponent sync 확인
- Reaction / Dead 상태 입력 차단 확인
- Enemy attack context 전달 확인

---
## 관련 문서

- Issue Checklist: `D12_UE5_Portfolio_Issue_Checklist.md`

- 후속 문서: `P12_UE5_Portfolio_Pull_Request.md`

---
## 정리

이 PR의 핵심은 Player를 combat receiver로 편입하여, Enemy 공격을 받았을 때 damage, health, reaction, dead state가 하나의 수신 루프로 연결되도록 만든 것이다.

변경 후 Player는 `UCTakeDamageComponent`, `UCHealthComponent`, `UCReactionComponent`를 통해 damage 수신과 hit reaction을 처리할 수 있게 되었고, dead state 변화도 UCStateComponent와 동기화됐다.

또한 Enemy attack context를 Player 수신 흐름까지 전달하도록 보완하여, 이후 Player / Enemy가 같은 combat core를 공유하기 위한 기반을 마련했다.

---
