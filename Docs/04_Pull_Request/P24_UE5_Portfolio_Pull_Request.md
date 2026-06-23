# UE5 Portfolio Pull Request

## 제목

**P24: Combat Signal Component Rename**

## 날짜

**2026.06.23**

## 상태

- [x] **완료**

---

## 브랜치

- `refactor/combat-signal-component-rename`

---

## 요약

이번 PR에서는 W04-03 / W04-04에서 정리한 source-side / target-side 책임 경계를 코드 이름에 반영했다.

핵심은 기존 combat runtime 동작을 바꾸지 않고, `ApplyDamageComponent` / `TakeDamageComponent`로 남아 있던 컴포넌트, 파일, 내부 API, 구조체, 필드, debug label을 `CombatSignalSource` / `CombatSignalTarget` 기준으로 리네임하는 것이다.

---

## 변경 배경

P22에서는 `TakeDamageComponent` 내부 흐름을 target-side 기준으로 정리했다.

P23에서는 `ApplyDamageComponent` 내부 흐름을 source-side 기준으로 정리했다.

두 브랜치 이후 실제 책임은 source / target 기준으로 읽히지만, 코드 이름은 여전히 `ApplyDamage / TakeDamage`에 남아 있었다. 이번 PR에서는 이 명칭 불일치를 줄이고, 후속 damage data 타입 정리와 cue 연결 작업의 기준 이름을 고정한다.

---

## 변경 범위

### 1. Component / File / Class Rename

```text
UCApplyDamageComponent
-> UCCombatSignalSourceComponent

UCTakeDamageComponent
-> UCCombatSignalTargetComponent
```

```text
CApplyDamageComponent.h/.cpp
-> CCombatSignalSourceComponent.h/.cpp

CTakeDamageComponent.h/.cpp
-> CCombatSignalTargetComponent.h/.cpp
```

Character / Weapon / Feedback / Reaction 참조도 새 컴포넌트명 기준으로 갱신했다.

### 2. Internal Flow API Rename

source-side flow API는 `CombatSignalSource` 또는 source 동작 기준으로 정리했다.

target-side flow API는 `CombatSignalTarget` 또는 target 동작 기준으로 정리했다.

UE engine boundary에 해당하는 `AActor::TakeDamage`, `Super::TakeDamage`, target `TakeDamage()` 호출은 유지했다.

### 3. Runtime Struct Rename

source-side payload / context / result 타입을 `CombatSignalSource` 기준으로 변경했다.

target-side payload / context / result / packet 타입을 `CombatSignalTarget` 기준으로 변경했다.

단, `FApplyDamageSpec`, `FApplyDamageSpecKey`, `FApplyDamageAmount`, `EApplyDamageRejectReason`, `ETakeDamageRejectReason`은 damage data 계층 의미가 남아 있어 이번 PR에서 변경하지 않았다.

### 4. Field / Debug Label Rename

캐싱 필드, local variable, subobject display name을 새 컴포넌트명 기준으로 정리했다.

debug output label도 다음 기준으로 갱신했다.

```text
Apply Damage
-> Combat Signal Source

Take Damage
-> Combat Signal Target
```

---

## Migration / Redirect

Unreal serialized reference 보호를 위해 다음 redirect를 추가했다.

```text
ClassRedirects:
- CApplyDamageComponent -> CCombatSignalSourceComponent
- CTakeDamageComponent -> CCombatSignalTargetComponent

PropertyRedirects:
- CEnemy.ApplyDamageComponent -> CEnemy.CombatSignalSourceComponent
- CEnemy.TakeDamageComponent -> CEnemy.CombatSignalTargetComponent
- CPlayer.ApplyDamageComponent -> CPlayer.CombatSignalSourceComponent
- CPlayer.TakeDamageComponent -> CPlayer.CombatSignalTargetComponent
```

에디터 asset load / resave 검증은 병합 전 확인 항목으로 남긴다.

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

- old component / API / struct / debug label 잔여 검색
- `git diff --check` 통과
- UE `TakeDamage()` engine boundary 유지 확인
- damage data 타입명 유지 범위 확인

### 추가 확인 필요

- `BP_CPlayer`, `BP_CEnemy`, 관련 map 에디터 로드
- renamed native component 중복 / 누락 여부 확인
- 기존 component property 값 보존 여부 확인

---

## 제외 범위

이번 PR에서는 다음 작업을 의도적으로 제외했다.

- combat runtime behavior 변경
- Guard / Parry / Defensive Outcome 로직 변경
- damage amount 계산 변경
- `FCombatSignal`을 기존 damage flow에 직접 연결
- damage data 타입명 정리
- `StructRedirects` / `EnumRedirects` 추가
- Blueprint asset resave
- Blink / Repulse timing cue 구현

---

## 후속 작업

권장 후속 작업은 다음과 같다.

```text
refactor/combat-damage-data-types
```

후속 작업 목표:

- `FApplyDamageSpec`, `FApplyDamageSpecKey`, `FApplyDamageAmount` 명칭 재검토
- `EApplyDamageRejectReason`, `ETakeDamageRejectReason` 유지 / 변경 기준 결정
- reflected struct / enum redirect 필요 여부 확인
- Blueprint / asset migration 결과 반영

---

## 관련 문서

- `Docs/01_Work_List/W04_Combat_Signal_Boundary/W04_UE5_Portfolio_Work_List.md`
- `Docs/06_notes/N05_Combat_Signal_Boundary_Design_Note.md`
- `Docs/06_notes/N06_Combat_Signal_Branch_Implementation_Plan.md`
- `Docs/06_notes/task_briefs/W04_Combat_Signal_Boundary/TB_W04_05_Combat_Signal_Component_Rename.md`
