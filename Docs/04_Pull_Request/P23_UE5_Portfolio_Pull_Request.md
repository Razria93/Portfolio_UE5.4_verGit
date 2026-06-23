# UE5 Portfolio Pull Request

## 제목

**P23: Combat Signal Source Boundary v1 정리**

## 날짜

**2026.06.23**

## 상태

- [x] **완료**

---

## 브랜치

- `refactor/combat-signal-source-v1`

---

## 요약

이번 PR에서는 `UCApplyDamageComponent`를 rename하거나 `FCombatSignal`에 연결하지 않고, 현재 weapon overlap damage 송신 흐름을 source-side 처리 단계 기준으로 정리했다.

핵심은 기존 weapon overlap damage 동작과 target `TakeDamage()` 전달 방식을 유지하면서, `ApplyDamageComponent`가 실제로 수행하는 hit window 관리, hit context 정규화, damage spec 해석, target 전달, duplicate cache 책임을 코드에서 더 명확히 읽히게 만드는 것이다.

---

## 변경 배경

P22에서 `TakeDamageComponent` 내부 흐름을 target-side 기준으로 정리했다.

다음 단계에서 바로 component rename이나 packet 교체를 진행하면 source / target 양쪽 책임 정리가 끝나기 전에 이름만 먼저 바뀔 수 있다. 따라서 이번 PR에서는 `ApplyDamageComponent` 내부 책임을 먼저 source-side 단계로 정렬하고, 실제 `CombatSignalSource` 전환은 후속 브랜치로 남겼다.

---

## 변경 범위

### 1. ApplyDamage source-side 단계 정리

`UCApplyDamageComponent` private API를 다음 단계 기준으로 재배치했다.

```text
HitWindow
-> Entry
-> Receive
-> Resolve
-> Send
-> Cache
-> Helper
-> Debug
```

### 2. Header / Source 정의 순서 정렬

`CApplyDamageComponent.h`의 선언 순서와 `CApplyDamageComponent.cpp`의 정의 순서를 맞췄다.

큰 처리 흐름은 위에서 아래로 읽히게 두고, `BuildHitWindowKey`, `BuildSpecKey`, `ResolveInstigatorController`, duplicate / friendly target helper는 별도 `Helper` 단계로 분리했다.

### 3. ProcessApplyDamage 흐름 라벨 정리

`ProcessApplyDamage()` 내부에 다음 단계 라벨을 추가했다.

```text
Receive
Resolve
Send
Debug
```

라벨은 흐름 가독성을 위한 주석이며, 기존 실행 순서와 조건 분기는 변경하지 않았다.

---

## 구현 범위

이번 PR의 코드 변경은 `UCApplyDamageComponent` 내부 정렬로 제한했다.

- 기존 public API 유지
- 기존 `RequestApplyDamage` 흐름 유지
- 기존 weapon overlap damage 흐름 유지
- 기존 target `TakeDamage()` 전달 방식 유지
- 기존 `FHitContext`, `FApplyDamagePayload`, `FApplyDamageContext`, `FApplyDamageResult` 유지
- `FCombatSignal` 직접 연결 없음

---

## 검증

### 빌드

```text
PortfolioEditor Win64 Development
```

결과:

```text
성공
Target is up to date
```

### 정적 확인

- `CApplyDamageComponent.h` private method group 확인
- `CApplyDamageComponent.cpp` 정의 순서 확인
- `ProcessApplyDamage()` 단계 라벨 확인
- `git diff --check` 통과

---

## 제외 범위

이번 PR에서는 다음 작업을 의도적으로 제외했다.

- `UCApplyDamageComponent` rename
- `UCCombatSignalSourceComponent` 신설
- `FCombatSignal`을 기존 damage flow에 연결
- `RequestApplyDamage` API rename
- `UCTakeDamageComponent` 수정
- `FCombatSignalResult` 변환 함수 구현
- Blink / Repulse / GuardBreak cue flow 정리

---

## 후속 작업

권장 후속 브랜치는 다음과 같다.

```text
refactor/combat-signal-component-rename
```

후속 작업 목표:

- `UCApplyDamageComponent`와 `UCTakeDamageComponent` 명칭을 Source / Target 책임 기준으로 교체
- Character / Weapon 참조 갱신
- Blueprint 영향 확인
- 기존 weapon overlap damage와 Guard / Parry / Hit / Dead 회귀 확인

---

## 관련 문서

- `Docs/01_Work_List/W04_Combat_Signal_Boundary/W04_UE5_Portfolio_Work_List.md`
- `Docs/06_notes/N05_Combat_Signal_Boundary_Design_Note.md`
- `Docs/06_notes/N06_Combat_Signal_Branch_Implementation_Plan.md`
- `Docs/06_notes/task_briefs/W04_Combat_Signal_Boundary/TB_W04_04_Combat_Signal_Source_Boundary_v1.md`
