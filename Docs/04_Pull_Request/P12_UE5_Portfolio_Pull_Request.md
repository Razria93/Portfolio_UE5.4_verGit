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

이번 PR에서는 **공격자가 만든 타격 정보가 피격자의 HP 감소와 피격 반응까지 이어지도록 공통 damage 처리 흐름을 정리했다.**

이를 통해 Player와 Enemy가 같은 규칙으로 damage를 주고받고, 같은 공격 구간 안에서 중복 타격이 발생하지 않도록 안정화했다.

성격별 핵심 변경은 다음과 같다.

### Feature

- **공통 damage 처리 흐름 연결**: 공격자 쪽 damage 요청과 피격자 쪽 damage 수신을 하나의 흐름으로 연결했다.

- **공격 구간 중복 타격 방지**: 같은 공격 구간에서 같은 대상에게 damage가 반복 적용되지 않도록 타격 기록을 관리했다.

- **Damage 결과 기반 피격 반응 연결**: 실제 HP 감소량과 사망 상태 변화를 기준으로 피격 / 사망 반응이 이어지도록 정리했다.

### Refactoring

- **공격자 damage 처리 단계 정리**: 공격자 쪽 damage 요청을 요청 값, 처리 중 상태, 처리 결과 단계로 나누어 검증과 계산 흐름을 정리했다.

- **피격자 damage 처리 단계 정리**: 피격자 쪽 damage 수신, damage 계산, HP 반영, 결과 생성을 단계별로 정리했다.

- **Damage amount 의미 분리**: 요청 damage, 감소 계산 후 damage, 실제 HP 반영 직전 damage, 실제 반영 damage의 의미를 구분했다.

### Troubleshooting

- **첫 타격 reject 문제 보완(B05)**: collision이 켜지기 전에 hit window id를 먼저 준비하도록 순서를 바꿔, 첫 overlap이 invalid request로 reject되지 않게 했다.

---

## 핵심 개념

이 섹션은 이후 설명에서 반복되는 최소 용어를 먼저 정리한다.

이 PR의 핵심 흐름은 공격자 쪽 ApplyDamage가 타격 정보를 정리하고, 피격자 쪽 TakeDamage가 실제 HP 반영과 reaction 연결 기준을 확정하는 구조다.

```text
Damage Pipeline(damage 처리 흐름)
-> 타격 정보가 damage 요청, HP 반영, reaction 연결로 이어지는 공통 처리 흐름
-> 이 PR에서는 ApplyDamage와 TakeDamage를 sender-side / receiver-side 단계로 나눔
```

```text
ApplyDamage(공격자 쪽 damage 요청)
-> 공격자가 hit context를 바탕으로 target에게 전달할 damage 요청을 구성하는 단계
-> damage spec 조회, 중복 타격 방지, target 전달 요청을 담당함
```

```text
TakeDamage(피격자 쪽 damage 수신)
-> 피격자가 damage event를 받아 실제 damage 계산과 HP 반영을 수행하는 단계
-> damage 결과와 dead state 변화를 기준으로 후속 reaction 연결 기준을 제공함
```

```text
Hit Window(타격 유효 구간)
-> weapon collision이 켜져 실제 hit를 처리할 수 있는 공격 구간
-> 같은 hit window 안에서는 같은 target에게 damage를 한 번만 적용함
```

```text
Payload / Context / Result(요청 값 / 처리 중 상태 / 처리 결과)
-> Payload는 외부에서 들어온 원본 요청 값
-> Context는 처리 중 검증, 계산, reject reason을 담는 runtime 상태
-> Result는 호출자가 확인할 수 있는 최종 처리 결과
```

```text
CommittedDamage(실제 반영 damage)
-> 최종적으로 HP에 반영된 damage 값
-> hit / dead reaction 연결과 dead target 추가 damage 차단 기준으로 사용됨
```

---

## 변경 배경

이 섹션은 이번 PR이 필요했던 이유와 기존 damage 처리 흐름에서 정리가 필요했던 지점을 설명한다.

### Player / Enemy 공통 damage 규칙 필요성

Player와 Enemy가 같은 전투 규칙을 사용하려면, 공격자 쪽 damage 요청과 피격자 쪽 damage 수신이 같은 기준으로 이어져야 했다.

공격자가 target에게 damage를 전달하는 단계와 피격자가 실제 HP를 줄이는 단계를 분리해야, 이후 reaction이나 dead 처리도 같은 규칙 위에서 확장할 수 있었다.

### Hit Window 기반 중복 타격 방지 필요성

근접 공격 collision은 하나의 공격 구간 안에서 같은 target과 여러 번 overlap될 수 있었다.

따라서 같은 hit window 안에서 이미 맞은 target을 기록하고, 같은 공격 구간 안의 중복 damage 적용을 차단할 필요가 있었다.

### Damage 결과와 Reaction 연결 기준 필요성

Reaction은 단순히 hit callback이 발생했다고 실행되면 안 되고, 실제 damage가 HP에 반영됐는지와 사망 상태가 어떻게 바뀌었는지를 기준으로 결정되어야 했다.

이를 위해 `CommittedDamage`와 dead state before / after 기준을 정리할 필요가 있었다.

### 첫 overlap HitWindowId 보정 필요성

collision이 활성화된 직후 첫 overlap이 들어올 때 hit window id가 아직 준비되지 않으면 첫 타격이 invalid request로 reject될 수 있었다.

hit window id는 collision을 실제로 켜기 전에 먼저 준비되어야 했다.

---

## 변경 범위

이 섹션은 damage 처리 흐름을 어떤 단계로 나눴고, 각 단계의 책임이 어떻게 정리됐는지 설명한다.

### 1. Hit Window 기반 타격 구간 관리

- **왜**:
  같은 공격 구간에서 같은 target이 여러 번 overlap되면 damage가 반복 적용될 수 있었다.

- **어떻게**:
  weapon collision이 켜질 때 hit window id를 열고, 해당 window 안에서 damage를 받은 target 목록을 관리했다.
  collision이 닫히면 hit window를 닫고 해당 window의 타격 기록을 제거했다.

- **결과**:
  같은 hit window 안에서는 target당 한 번만 damage가 적용되며, 다음 공격 구간에서는 새 기록으로 다시 판정된다.

### 2. ApplyDamage 단계 정리

- **왜**:
  공격자 쪽 damage 요청은 hit 정보 검증, damage spec 조회, 중복 타격 방지, target 전달까지 여러 책임을 가지고 있었다.

- **어떻게**:
  hit context를 `FApplyDamagePayload -> FApplyDamageContext -> FApplyDamageResult` 흐름으로 변환했다.
  이 과정에서 요청 유효성, hit window, self-hit, duplicate hit, spec 조회를 sender-side gate로 처리했다.

- **결과**:
  공격자 쪽 damage 요청은 검증, 계산, commit 결과를 분리해서 확인할 수 있고, target에게 전달되는 damage 조건이 명확해졌다.

### 3. TakeDamage 단계 정리

- **왜**:
  피격자 쪽에서는 damage event 수신, 실제 damage 계산, HP 반영, 결과 생성을 한 흐름으로 정리해야 했다.

- **어떻게**:
  damage event를 `FTakeDamagePayload -> FTakeDamageContext -> FTakeDamageResult` 흐름으로 변환했다.
  이 과정에서 target 상태, dead / dying 여부, damage 계산, health commit 결과를 receiver-side 기준으로 확정했다.

- **결과**:
  피격자 쪽 damage 수신은 계산 전 값과 실제 HP 반영 결과를 구분해 기록하며, 후속 reaction 판단에 사용할 결과를 제공한다.

### 4. Damage amount 의미 분리

- **왜**:
  damage 처리 과정에서 요청된 값과 실제 HP에 반영된 값이 다를 수 있으므로, 각 단계의 의미를 구분해야 했다.

- **어떻게**:
  damage amount를 `Requested`, `Mitigated`, `FinalTaken`, `Committed`로 나누어 로그와 result에 남겼다.

- **결과**:
  damage 계산 과정에서 어떤 값이 요청됐고, 어떤 값이 실제 HP에 반영됐는지 추적할 수 있게 됐다.

### 5. Dead Target 보호와 Reaction 연결

- **왜**:
  이미 죽었거나 죽는 중인 target에게 추가 damage가 다시 반영되면 HP와 reaction 상태가 어긋날 수 있었다.
  또한 hit reaction과 dead reaction은 실제 damage 결과를 기준으로 구분되어야 했다.

- **어떻게**:
  dead / dying 상태의 추가 damage는 reject 또는 `CommittedDamage = 0`으로 처리했다.
  `CommittedDamage`와 dead state before / after를 기준으로 hit reaction, dead reaction, no reaction 조건을 정리했다.

- **결과**:
  dead target에게 추가 HP 감소가 발생하지 않고, reaction은 실제 damage 적용 결과를 기준으로 연결된다.

### 6. 첫 overlap HitWindowId 전달 순서 보정

- **왜**:
  collision이 먼저 켜지고 hit window id가 나중에 준비되면 첫 overlap이 `HitWindowId = INDEX_NONE` 상태로 들어와 invalid request로 reject될 수 있었다.

- **어떻게**:
  먼저 활성화할 collision 목록을 수집하고, 유효한 collision이 있을 때 hit window id와 state를 준비한 뒤 실제 collision을 켜도록 순서를 바꿨다.

- **결과**:
  첫 overlap도 유효한 hit window id를 가진 hit context로 전달되며, 첫 타격이 invalid hit window 때문에 reject되지 않는다.

---

## 주요 처리 흐름

이 섹션은 damage 처리의 대표 흐름과 핵심 분기 기준을 정리한다.

### Shared Damage Pipeline 흐름

```text
weapon overlap
-> hit context 생성
-> ApplyDamage 요청
-> damage event 생성
-> target TakeDamage 호출
-> HP 반영
-> damage result 생성
-> hit / dead reaction 연결 기준 확인
```

이 흐름은 공격자 쪽 hit 정보가 피격자 쪽 HP 반영과 reaction 연결 기준으로 이어지는 전체 damage 처리 흐름을 의미한다.

### Hit Window 흐름

```text
collision enable 요청
-> 활성화할 collision 목록 확인
-> hit window id 준비
-> damaged target 기록 초기화
-> collision 활성화
-> overlap 발생
-> hit window id 검증
-> duplicate hit 확인
-> collision disable
-> hit window 기록 제거
```

이 흐름은 collision을 켜기 전에 유효한 hit window를 준비하고, 같은 공격 구간 안의 중복 타격을 막는 과정을 의미한다.

### ApplyDamage 흐름

```text
hit context 수신
-> payload 생성
-> context 생성
-> 요청 유효성 확인
-> sender-side gate 확인
   - invalid hit window
   - self-hit
   - duplicate hit
   - spec miss
-> damage amount 계산
-> target TakeDamage 호출
-> result 생성
```

이 흐름은 공격자 쪽에서 target에게 damage를 전달하기 전에 필요한 최소 검증과 계산을 수행하는 과정을 의미한다.

### TakeDamage 흐름

```text
damage event 수신
-> payload 생성
-> context 생성
-> receiver-side gate 확인
   - invalid target
   - dead / dying target
-> damage amount 계산
-> health commit
-> result 생성
-> committed damage와 dead state 변화 기록
```

이 흐름은 피격자 쪽에서 damage를 실제 HP에 반영하고 후속 reaction 판단에 필요한 결과를 만드는 과정을 의미한다.

---

## 구현 결과

- Player와 Enemy는 같은 `ApplyDamage -> TakeDamage` 흐름으로 damage를 주고받는다.

- 공격자 쪽 ApplyDamage는 hit context를 payload / context / result 단계로 정리하고, hit window와 duplicate hit 조건을 검증한다.

- 피격자 쪽 TakeDamage는 damage event를 payload / context / result 단계로 정리하고, HP 반영 결과와 dead state 변화를 기록한다.

- 같은 hit window 안에서는 같은 target에게 damage가 한 번만 적용된다.

- 첫 overlap 이전에 hit window id가 준비되어 첫 타격이 invalid request로 reject되지 않는다.

- hit / dead reaction은 `CommittedDamage`와 dead state before / after 기준으로 연결된다.

---

## 테스트 방법

### Shared Damage Flow

- Player와 Enemy 양쪽에서 동일한 공격 흐름으로 `ApplyDamage -> TakeDamage`가 동작하는지 확인한다.

- ApplyDamage result와 TakeDamage result의 request / final / committed 값이 로그 기준으로 일관되게 연결되는지 확인한다.

### Hit Window

- 동일 hit window 안에서 target 중복 타격이 방지되는지 확인한다.

- 첫 overlap 시점에 `HitWindowId`가 유효하게 전달되어 첫 타가 invalid request로 reject되지 않는지 확인한다.

### Health / Dead Policy

- HP가 0에 도달했을 때 `CommittedDamage`가 남은 HP만큼만 반영되는지 확인한다.

- dead / dying 이후 추가 hit은 `CommittedDamage = 0`으로 처리되고 추가 HP 감소가 발생하지 않는지 확인한다.

### Reaction

- `CommittedDamage > 0`인 경우 hit reaction이 연결되는지 확인한다.

- death 전이 시 hit reaction보다 dead reaction이 우선되는지 확인한다.

---

## 검증 결과

- Player / Enemy 공통 `ApplyDamage -> TakeDamage` pipeline 동작을 확인했다.

- hit window 기반 duplicate hit 방지 동작을 확인했다.

- 첫 overlap 시점 `HitWindowId` 전달 문제 수정이 반영된 것을 확인했다.

- `Requested / Mitigated / FinalTaken / Committed` 로그 기준 정합성을 확인했다.

- dead target 추가 damage 차단 동작을 확인했다.

- damage result 기반 hit / dead reaction 연결을 확인했다.

---

## 비범위

- Team 식별 구조와 Friendly Fire 판정 정책은 이번 범위에 포함하지 않는다.

- Guard / Armor / Resistance 같은 receiver-side 확장 정책은 후속 범위로 둔다.

- 후속 PR에서 도입된 상위 실행 조율 구조는 이 PR 설명에 소급하지 않는다.

---

## 관련 문서

- Issue Checklist: `D13_UE5_Portfolio_Issue_Checklist.md`

- Bug Report: `B05_UE5_Portfolio_Bug_Report.md`

---

## 정리

P12는 damage를 단순 HP 감소 호출로 두지 않고, 공격자 쪽 ApplyDamage와 피격자 쪽 TakeDamage가 각자의 책임을 가지고 이어지는 공통 damage 처리 흐름으로 정리한 PR이다.

이 PR 이후 Player와 Enemy는 hit window, duplicate hit, damage amount, dead target, reaction 연결 기준을 공유하는 combat core 위에서 damage를 처리한다.
