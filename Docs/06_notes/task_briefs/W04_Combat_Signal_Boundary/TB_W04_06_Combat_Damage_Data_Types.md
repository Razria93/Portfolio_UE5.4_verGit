# TB W04-06 Combat Damage Data Types

## 작업명

```text
Combat Damage Data Types
```

## 브랜치

```text
refactor/combat-damage-data-types
```

## 목표

`CombatSignalSource / CombatSignalTarget` 리네임 이후에도 `ApplyDamage / TakeDamage` 이름으로 남아 있는 damage data 타입을 현재 책임 기준에 맞게 정리한다.

이번 작업은 gameplay 동작 변경이 아니라, damage data / source hit window / source reject / target reject의 명칭 경계를 분명히 하는 리팩터링이다.

## 배경

W04-05에서는 컴포넌트, 파일, 내부 API, payload / context / result / packet 타입을 `CombatSignalSource / CombatSignalTarget` 기준으로 정리했다.

다만 다음 타입은 damage data 계층 의미가 남아 있어 별도 브랜치로 분리했다.

```text
FApplyDamageSpecKey
FApplyDamageSpec
FApplyDamageAmount
FApplyDamageHitWindowKey
EApplyDamageRejectReason
ETakeDamageRejectReason
```

이 타입들을 모두 `CombatSignalSource`로 바꾸면 weapon / action 기반 damage 설정이라는 의미가 흐려질 수 있다. 반대로 모두 `Damage`로만 바꾸면 source-side hit window나 source / target 단계의 reject reason 차이가 사라진다.

따라서 이번 브랜치에서는 타입의 실제 책임을 기준으로 다음 세 범주로 나눈다.

```text
Damage data
-> weapon / action 기반 damage 설정과 수치

CombatSignal source-side runtime
-> hit window, duplicate target, signal send 단계

CombatSignal target-side runtime
-> receive, evaluate, commit 단계
```

## 핵심 범위

### Damage data 타입

```text
FApplyDamageSpecKey
-> FDamageSpecKey

FApplyDamageSpec
-> FDamageSpec

FApplyDamageAmount
-> FDamageAmount
```

이 타입들은 source component 전용 타입이 아니라, combat damage 설정 / 수치 데이터를 표현한다.

### Source hit window 타입

```text
FApplyDamageHitWindowKey
-> FCombatSignalHitWindowKey
```

이 타입은 damage spec이 아니라 source-side hit window와 duplicate target cache를 식별하는 key다.

### Reject reason 타입

```text
EApplyDamageRejectReason
-> ECombatSignalSourceRejectReason

ETakeDamageRejectReason
-> ECombatSignalTargetRejectReason
```

reject reason은 damage data 자체가 아니라 source / target 처리 단계의 실패 이유다.

## 유지 범위

다음 이름은 이번 브랜치에서 변경하지 않는다.

```text
AActor::TakeDamage
Super::TakeDamage
TargetActor->TakeDamage(...)
UCHealthComponent::TakeDamage
FDefaultDamageEvent
FDamageImpactInfo
FCombatResultPacket
```

유지 이유:

- `TakeDamage()`는 Unreal Engine boundary다.
- `UCHealthComponent::TakeDamage()`는 resource commit API다.
- `FDefaultDamageEvent`, `FDamageImpactInfo`는 engine damage event / impact data 의미가 남아 있다.
- `FCombatResultPacket`은 result-out / attacker-side 흐름 정리 브랜치에서 따로 검토한다.

## Damage 용어 유지 / 후속 검토 기준

이번 브랜치 이후에도 `Damage` 용어를 모두 제거하지 않는다.

유지 기준:

```text
Damage data
Damage amount
Damage event
Damage causer
Damage impact
Health TakeDamage
UE TakeDamage
```

위 항목은 실제 damage 수치, damage event, damage commit, engine boundary를 뜻하므로 유지한다.

후속 검토 기준:

```text
DamageReaction*
DamageFeedback*
FCombatResultPacket
result-out packet 내부 naming
FDamageImpactInfo가 damage 외 cue impact까지 확장되는 경우
```

위 항목은 실제 damage 수치보다 reaction / feedback / result-out / cue outcome에 가까워질 수 있다.

따라서 이번 브랜치에서 억지로 바꾸지 않고, 다음 브랜치들의 책임이 더 명확해진 뒤 검토한다.

후속 분리:

```text
Combat Signal Cue v1
-> FDamageImpactInfo가 cue impact까지 담아야 하는지 확인

Combat Signal Result Out
-> FCombatResultPacket / bDamageCommitted / CommittedDamage naming 확인

Combat Feedback Boundary
-> DamageReaction* / DamageFeedback* naming 확인
```

## 제외 범위

- combat damage 계산 로직 변경
- Guard / Parry / Defensive Outcome 로직 변경
- source-side target discovery 방식 변경
- target-side Health commit 방식 변경
- `FCombatSignal` 직접 연결
- `FCombatResultPacket` 리네임 또는 구조 변경
- `UCHealthComponent::TakeDamage` 리네임
- Blueprint asset migration 변경

## 작업 순서

1. enum 타입명 리네임

```text
EApplyDamageRejectReason
-> ECombatSignalSourceRejectReason

ETakeDamageRejectReason
-> ECombatSignalTargetRejectReason
```

2. damage spec / amount 타입명 리네임

```text
FApplyDamageSpecKey
-> FDamageSpecKey

FApplyDamageSpec
-> FDamageSpec

FApplyDamageAmount
-> FDamageAmount
```

3. source hit window key 리네임

```text
FApplyDamageHitWindowKey
-> FCombatSignalHitWindowKey
```

4. damage spec / amount 사용처 이름 정리

```text
ApplyDamageSpecKey
-> DamageSpecKey

ApplyDamageSpec
-> DamageSpec

ApplyDamageAmount
-> DamageAmount
```

이 단계는 타입명이 아니라, struct field / function parameter / local variable처럼 해당 타입을 담는 사용처 이름을 정리하는 작업이다.

5. debug label 정리

```text
ApplyDamageSpecKey
-> DamageSpecKey

ApplyDamageSpec
-> DamageSpec

ApplyDamageAmount
-> DamageAmount
```

단, UE `TakeDamage` entry와 resource `TakeDamage` API 주변 이름은 유지한다.

6. 문서 / PR 반영

```text
W04 Work List
N06 branch implementation plan
P25 PR 문서
prompt update 필요 여부
```

## 완료조건

- `FApplyDamageSpecKey / Spec / Amount` 명칭이 damage data 기준 이름으로 정리되어 있다.
- `FApplyDamageHitWindowKey` 명칭이 source-side hit window 책임을 드러낸다.
- source / target reject reason 명칭이 CombatSignal 처리 단계와 일치한다.
- UE engine boundary와 resource boundary의 `TakeDamage` 이름은 유지되어 있다.
- 기존 combat runtime 동작이 바뀌지 않는다.
- `git diff --check`가 통과한다.
- `PortfolioEditor Win64 Development` 빌드가 성공한다.

## 결정 사항

작업 시작 기준 결정:

- damage 설정 / 수치 타입은 `Damage` 기준 이름으로 축소한다.
- source-side hit window key는 `CombatSignal` 기준 이름으로 정리한다.
- reject reason은 source / target 처리 단계 기준으로 분리한다.
- `FCombatResultPacket`은 이번 브랜치에서 건드리지 않는다.
- `TakeDamage()` 계열 engine / resource API는 유지한다.

작업 완료 기준 결정:

- `FApplyDamageSpecKey / FApplyDamageSpec / FApplyDamageAmount`는 `FDamageSpecKey / FDamageSpec / FDamageAmount`로 변경했다.
- `FApplyDamageHitWindowKey`는 source-side hit window / duplicate hit cache 책임을 드러내도록 `FCombatSignalHitWindowKey`로 변경했다.
- `EApplyDamageRejectReason / ETakeDamageRejectReason`은 처리 단계 기준을 드러내도록 `ECombatSignalSourceRejectReason / ECombatSignalTargetRejectReason`으로 변경했다.
- 기존 Blueprint / asset 직렬화 데이터를 보존하기 위해 변경된 struct / enum에 CoreRedirect를 추가했다.
- `ApplyDamageSpecKey / ApplyDamageSpec / ApplyDamageAmount` 사용처 이름은 `DamageSpecKey / DamageSpec / DamageAmount`로 정리했다.
- `Apply Damage To Health` 주석은 실제 resource commit 의미에 맞게 `Commit Damage To Health`로 정리했다.
- UE `TakeDamage()` boundary와 `UCHealthComponent::TakeDamage()` resource API는 유지했다.
- `DamageReaction*`, `DamageFeedback*`, `FCombatResultPacket`, `FDamageImpactInfo` 확장 여부는 후속 브랜치로 분리했다.
- runtime behavior, damage 계산, Guard / Parry 판정은 변경하지 않았다.

## 검증 계획

```text
rg old type name scan
CoreRedirect scan
git diff --check
PortfolioEditor Win64 Development build
Player -> Enemy hit 확인
Enemy -> Player hit 확인
Guard / Parry 기존 flow 확인
```

## 검증 결과

```text
rg old type / field / label scan 통과
CoreRedirect 추가 확인
git diff --check 통과
PortfolioEditor Win64 Development 빌드 성공
```

## 커밋 분할 후보

```text
docs(combat): add combat damage data type task brief
refactor(combat): rename combat signal reject reasons
refactor(combat): rename damage spec data types
refactor(combat): rename combat signal hit window key
refactor(combat): align damage data field names
docs(combat): document combat damage data type rename
```

## 프롬프트 업데이트 확인

이번 작업에서 확인한 후보:

```text
컴포넌트 책임명과 data 타입명을 무조건 같은 접두어로 맞추지 않는다.
data 타입은 실제 데이터의 소유 계층과 재사용 범위를 기준으로 이름을 정한다.
engine boundary / resource boundary 이름은 프로젝트 책임명과 다르게 유지할 수 있다.
```

위 기준은 PU01에 반영한다.
