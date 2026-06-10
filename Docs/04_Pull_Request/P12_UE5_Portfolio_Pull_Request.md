# UE5 Portfolio Pull Request

## 제목

**P12: Damage Pipeline 공유 구조 구현 및 규칙 정리**

## 날짜

**2026.04.06**

## 상태

- [x] **완료**

---

## 브랜치

- `feature/combat-core-shared`

---

## 요약

### 작업 요약

본 PR은 공격자와 피격자가 공유하는 damage 처리 흐름을 **ApplyDamage -> DefaultDamageEvent -> TakeDamage -> Health -> Reaction** 기준으로 정리한 작업이다.

### 작업 배경

Player와 Enemy가 같은 전투 규칙을 사용하려면, 공격자 쪽 damage 송신과 피격자 쪽 damage 수신을 공통 pipeline으로 분리할 필요가 있었다.

또한 동일 공격 window 안에서 같은 target이 여러 번 overlap될 수 있으므로, hit window 기반 중복 타격 방지 규칙이 필요했다.

### 구현 방향

이를 다음 네 가지 축으로 정리했다.

```yaml
1. Sender-side damage pipeline 정리
- ApplyDamage 단계에서 hit context를 payload / context / result로 변환
- damage spec resolve와 duplicate hit filtering 처리

2. Receiver-side damage pipeline 정리
- TakeDamage 단계에서 damage 수신 / 계산 / health commit 처리
- Requested / Mitigated / FinalTaken / Committed 의미 분리

3. Hit window 기반 중복 타격 방지
- hit window id를 기준으로 동일 공격 window 안의 duplicate hit 차단

4. Reaction 연결 규칙 정리
- CommittedDamage와 dead-state 전후 조건을 기준으로 hit / dead reaction 연결
```

---
## 변경 범위

### Damage Pipeline

#### A. Attachment Hit Window 규칙 정리

- weapon collision window를 hit window로 식별하고, 동일 window 내 target 중복 타격을 방지하도록 구성했다.

**Flow**
```yaml
CollisionEnabled
-> HitWindowOpened
-> CurrentHitWindowId 증가
-> overlap metadata에 HitWindowId 전달
-> CollisionDisabled
-> HitWindowClosed
```

**Structure**
```yaml
HitWindow
- CurrentHitWindowId : 현재 공격 window 식별자
- DamageCauser       : damage를 발생시킨 weapon / actor
- DamagedTargetSet   : 해당 window 안에서 이미 맞은 target 목록
```

**Rule**
```yaml
Allow
- 같은 hit window 안에서 처음 맞은 target

Reject
- HitWindowId == INDEX_NONE
- 같은 hit window 안에서 이미 맞은 target
- self-hit target
```

#### B. ApplyDamage Pipeline 정리

- 공격자 쪽 damage 송신 흐름을 `Payload / Context / Result` 기준으로 정리했다.

**Flow**
```yaml
HitContext
-> ValidateRequest
-> BuildPayload
-> BuildContext
-> ValidateContext
-> CanApplyDamage
-> ResolveApplyDamageSpec
-> ComputeApplyDamage
-> CommitApplyDamage
-> BuildResult
```

**Structure**
```yaml
FApplyDamagePayload
- hit에서 전달된 원본 요청 데이터
- target / damage causer / spec key / hit window metadata

FApplyDamageContext
- ApplyDamage 처리 중 사용하는 runtime context
- resolved spec / computed damage / reject reason

FApplyDamageResult
- ApplyDamage 호출 결과
- accepted 여부 / reject reason / committed damage
```

#### C. ApplyDamage 정책 정리

- sender-side에서 처리해야 하는 최소 damage gate와 spec resolve 정책을 정리했다.

**Rule**
```yaml
Validate
- valid target
- valid damage causer
- valid hit window id

Policy
- self-hit reject
- duplicate-hit reject
- spec miss reject

Commit
- resolved spec 기준으로 UGameplayStatics::ApplyDamage 호출
```

#### D. TakeDamage Pipeline 정리

- 피격자 쪽 damage 수신 흐름을 `Payload / Context / Result` 기준으로 정리했다.

**Flow**
```yaml
DefaultDamageEvent
-> ValidateRequest
-> BuildPayload
-> BuildContext
-> ValidateContext
-> CanTakeDamage
-> ComputeTakeDamage
-> CommitTakeDamage
-> BuildResult
```

**Structure**
```yaml
FTakeDamagePayload
- TakeDamage로 들어온 원본 damage event 데이터
- event instigator / damage causer / spec / requested damage

FTakeDamageContext
- TakeDamage 처리 중 사용하는 receiver-side context
- health snapshot / dead state / computed damage / reject reason

FTakeDamageResult
- TakeDamage 처리 결과
- accepted 여부 / committed damage / dead state before-after
```

#### E. TakeDamage Damage Amount 의미 정리

- receiver-side damage 계산 값을 단계별로 구분함.

**Structure**
```yaml
Requested
- attacker가 요청한 damage amount

Mitigated
- 방어 / 감소 계산 이후 damage

FinalTaken
- 실제 health commit 직전 damage

Committed
- UCHealthComponent에 실제 반영된 HP 감소량
```

#### F. Dead Target 보호 및 후처리 규칙 정리

- dead / dying 상태 이후 추가 damage가 health에 다시 반영되지 않도록 처리했다.

**Rule**
```yaml
Alive Target
- damage 계산과 health commit 허용

Dead / Dying Target
- 추가 damage reject 또는 CommittedDamage = 0 처리
- 추가 HP 감소 방지
```

#### G. Reaction 연결 규칙 정리

- damage commit 결과를 기준으로 reaction request가 연결되도록 정리했다.

**Rule**
```yaml
Hit Reaction
- CommittedDamage > 0
- DeadState_After == Alive

Dead Reaction
- DeadState_Before == Alive
- DeadState_After != Alive

No Reaction
- damage rejected
- CommittedDamage <= 0
- already dead target
```

---
## 안정성 보완

### 첫 overlap HitWindowId 전달 안정화 (B05 보완)

#### A. Collision 활성화 전 hit window context 준비

- 첫 overlap callback이 발생하기 전에 `CurrentHitWindowId`와 hit window state가 먼저 준비되도록 처리 순서를 보정했다.
- 자세한 재현 조건과 원인은 `B05` Bug Report에서 분리하여 정리했다.

**Before**
```yaml
CollisionEnabled 호출
-> collision 먼저 활성화
-> 첫 overlap 발생
-> HitWindowId == INDEX_NONE
-> InvalidRequest reject
```

**After**
```yaml
CollisionEnabled 호출
-> HitWindowId 증가
-> hit window state 준비
-> collision 활성화
-> 첫 overlap에도 유효한 HitWindowId 전달
```

---
## 주요 Pipeline

### Shared Damage Pipeline

```yaml
HitContext
-> ApplyDamage
-> DefaultDamageEvent
-> TakeDamage
-> Health
-> Reaction
```

### ApplyDamage Pipeline

```yaml
HitContext
-> FApplyDamagePayload
-> FApplyDamageContext
-> FApplyDamageResult
-> UGameplayStatics::ApplyDamage
```

### TakeDamage Pipeline

```yaml
DefaultDamageEvent
-> FTakeDamagePayload
-> FTakeDamageContext
-> Health Commit
-> FTakeDamageResult
```

### Hit Window Pipeline

```yaml
CollisionEnabled
-> HitWindowOpened
-> Overlap
-> Duplicate Hit Check
-> CollisionDisabled
-> HitWindowClosed
```

---
## 테스트 방법

### Shared Damage Flow

- Player와 Enemy 양쪽에서 동일한 공격 흐름으로 `ApplyDamage -> TakeDamage`가 동작하는지 확인
- `ApplyDamageResult`와 `TakeDamageResult`의 request / final / committed 값이 로그 기준으로 일관되게 연결되는지 확인

### Hit Window

- 동일 공격 window 안에서 target 중복 타격이 방지되는지 확인
- 첫 overlap 시점에 `HitWindowId`가 유효하게 전달되어 첫 타가 `InvalidRequest`로 reject되지 않는지 확인

### Health / Dead Policy

- HP가 0에 도달했을 때 `CommittedDamage`가 남은 HP만큼만 반영되는지 확인
- dead / dying 이후 추가 hit은 `CommittedDamage = 0`으로 처리되고 추가 HP 감소가 발생하지 않는지 확인

### Reaction

- `CommittedDamage > 0`인 경우 hit reaction이 연결되는지 확인
- death 전이 시 hit reaction보다 dead reaction이 우선되는지 확인

---
## 검증 결과

- Player / Enemy 공통 `ApplyDamage -> TakeDamage` pipeline 동작 확인
- hit window 기반 duplicate hit 방지 동작 확인
- 첫 overlap 시점 `HitWindowId` 전달 문제 수정 확인
- `Requested / Mitigated / FinalTaken / Committed` 로그 기준 정합성 확인
- dead target 추가 damage 차단 확인
- damage result 기반 hit / dead reaction 연결 확인

---
## 관련 문서

- Issue Checklist: `D13_UE5_Portfolio_Issue_Checklist.md`

- Bug Report: `B05_UE5_Portfolio_Bug_Report.md`

---
## 정리

이 PR의 핵심은 damage를 단순히 health를 감소시키는 함수 호출로 두지 않고, sender-side `ApplyDamage`와 receiver-side `TakeDamage`를 분리된 pipeline으로 정리한 것이다.

변경 후 공격자는 hit context를 기반으로 damage payload / context / result를 구성하고, 피격자는 damage event를 수신한 뒤 receiver-side context에서 damage 계산과 health commit을 수행한다.

또한 hit window 기반 중복 타격 방지와 dead target 보호 규칙을 추가하여, Player와 Enemy가 같은 shared combat core 위에서 damage와 reaction을 처리할 수 있도록 정리했다.

---
