# UE5 Portfolio Pull Request

## 제목

**P25: Combat Signal Damage Data Type 정리** 

## 날짜

**2026.06.24**

## 상태

- [x] **완료**

---

## 브랜치

- `refactor/combat-damage-data-types`

---

## 요약

이번 PR에서는 `CombatSignalSource / CombatSignalTarget` 리네임 이후에도 `ApplyDamage / TakeDamage` 이름으로 남아 있던 damage data 타입과 사용처 이름을 정리했다.

핵심은 gameplay 동작을 바꾸지 않고, shared damage data / source-side hit window key / source-target reject reason의 명명 기준을 분리하는 것이다.

---

## 변경 배경

P24에서 컴포넌트와 source / target runtime 타입은 `CombatSignal` 기준으로 정리했다.

다만 `FApplyDamageSpecKey`, `FApplyDamageSpec`, `FApplyDamageAmount`는 weapon / action 기반 damage data 의미가 남아 있어 P24에서 의도적으로 제외했다.

이번 PR에서는 이 잔여 이름을 정리하되, 모든 타입을 무조건 `CombatSignalSource` 접두로 밀지 않고 실제 책임 기준으로 나누었다.

---

## 변경 범위

### 1. Reject Reason Rename

```text
EApplyDamageRejectReason
-> ECombatSignalSourceRejectReason

ETakeDamageRejectReason
-> ECombatSignalTargetRejectReason
```

reject reason은 damage data가 아니라 source / target 처리 단계의 실패 이유이므로 CombatSignal 처리 단계 기준으로 정리했다.

### 2. Damage Data Type Rename

```text
FApplyDamageSpecKey
-> FDamageSpecKey

FApplyDamageSpec
-> FDamageSpec

FApplyDamageAmount
-> FDamageAmount
```

damage spec / amount는 source component 전용 타입이 아니라 weapon / action 기반 combat damage data이므로 `Damage` 기준 이름으로 축소했다.

### 3. Source Hit Window Key Rename

```text
FApplyDamageHitWindowKey
-> FCombatSignalHitWindowKey
```

hit window key는 damage spec이 아니라 source-side hit window와 duplicate hit cache를 식별하므로 CombatSignal source runtime 이름으로 정리했다.

### 4. Usage Name / Debug Label Cleanup

다음 사용처 이름을 damage data 기준으로 정리했다.

```text
ApplyDamageSpecKey
-> DamageSpecKey

ApplyDamageSpec
-> DamageSpec

ApplyDamageAmount
-> DamageAmount
```

target-side commit 주석도 실제 resource commit 의미에 맞게 정리했다.

```text
Apply Damage To Health
-> Commit Damage To Health
```

---

## 유지 범위

다음 이름은 이번 PR에서 유지했다.

```text
AActor::TakeDamage
Super::TakeDamage
TargetActor->TakeDamage(...)
UCHealthComponent::TakeDamage
FDefaultDamageEvent
FDamageImpactInfo
FCombatResultPacket
```

`TakeDamage()` 계열은 Unreal Engine boundary 또는 resource commit API다.

`FCombatResultPacket`은 result-out / attacker-side 흐름 정리 브랜치에서 별도로 검토한다.

---

## Damage 용어 유지 / 후속 검토

이번 PR 이후에도 다음 `Damage` 용어는 유지한다.

```text
Damage data
Damage amount
Damage event
Damage causer
Damage impact
Health TakeDamage
UE TakeDamage
```

반대로 다음 항목은 후속 브랜치에서 다시 검토한다.

```text
DamageReaction*
DamageFeedback*
FCombatResultPacket
result-out packet 내부 naming
FDamageImpactInfo가 damage 외 cue impact까지 확장되는 경우
```

검토 기준:

- 실제 damage 수치 / commit / engine boundary이면 유지한다.
- reaction / feedback / result-out / cue outcome을 대표하는 이름이면 후속 브랜치에서 CombatSignal 또는 CombatOutcome 기준으로 재검토한다.

---

## 검증

### 빌드

```text
PortfolioEditor Win64 Development
```

결과:

```text
성공
```

### 정적 확인

- old enum / struct / field / debug label 잔여 검색
- `git diff --check` 통과
- UE `TakeDamage()` engine boundary 유지 확인
- `UCHealthComponent::TakeDamage()` resource boundary 유지 확인

---

## 제외 범위

이번 PR에서는 다음 작업을 의도적으로 제외했다.

- combat runtime behavior 변경
- damage amount 계산 로직 변경
- Guard / Parry / Defensive Outcome 로직 변경
- `FCombatSignal` 직접 연결
- `FCombatResultPacket` 리네임 또는 구조 변경
- Blueprint asset migration 변경
- Blink / Repulse timing cue 구현

---

## 후속 작업

권장 후속 작업은 다음과 같다.

```text
feature/combat-signal-cue-v1
```

후속 작업 목표:

- Blink / Repulse 같은 collision 없는 timing cue를 CombatSignal 흐름에 연결
- collision hit와 timing cue가 같은 target receive 흐름을 공유하는지 검증
- cue 전용 예외 파이프라인을 만들지 않는 방향 확인
- cue impact가 기존 `FDamageImpactInfo`와 같은 구조를 공유할 수 있는지 확인

---

## 관련 문서

- `Docs/01_Work_List/W04_Combat_Signal_Boundary/W04_UE5_Portfolio_Work_List.md`
- `Docs/06_notes/N06_Combat_Signal_Branch_Implementation_Plan.md`
- `Docs/06_notes/task_briefs/W04_Combat_Signal_Boundary/TB_W04_06_Combat_Damage_Data_Types.md`
