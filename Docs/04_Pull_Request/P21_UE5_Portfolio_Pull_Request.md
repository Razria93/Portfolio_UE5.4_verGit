# UE5 Portfolio Pull Request

## 제목

**P21: Combat Signal Boundary v1 정리**

## 날짜

**2026.06.22**

## 상태

- [x] **완료**

---

## 브랜치

- `refactor/combat-signal-boundary`

---

## 요약

이번 PR에서는 이후 combat 송수신 책임을 정리하기 위한 기준으로 **Combat Signal Boundary v1**을 문서화하고, 다음 리팩터링에서 공유할 최소 타입 vocabulary를 추가했다.

기존 gameplay 흐름은 변경하지 않았다. `ApplyDamageComponent`, `TakeDamageComponent`, Guard / Parry / Hit / Dead 흐름은 그대로 두고, 후속 브랜치에서 Target / Source 책임을 나눌 수 있도록 설계 기준과 타입만 마련했다.

---

## 변경 배경

Guard / Parry v1 이후 damage 처리 흐름에는 hit 전달, 수신 측 방어 판정, damage commit, reaction / feedback, attacker result 전달 책임이 함께 모여 있었다.

초기에는 이를 일반화된 `Intent / Request / Receiver / Resolution / Coordinator` 구조로 분리하는 방향을 검토했다. 하지만 입력, damage, timing cue, system event는 발생 원인과 해석 기준이 달라 하나의 공용 `Request` 파이프라인으로 먼저 묶기에는 범위가 넓었다.

따라서 현재 브랜치에서는 공용 상태 변경 파이프라인 일반화를 보류하고, 후속 리팩터링의 기준으로 사용할 `CombatSignal Source / Target` 경계만 정리했다.

---

## 변경 범위

### 1. Combat Signal 설계 기준 정리

- `N05_Combat_Signal_Boundary_Design_Note.md` 추가 및 정리
- `N06_Combat_Signal_Branch_Implementation_Plan.md` 추가 및 정리
- branch 분할 기준을 feature 단위가 아니라 behavior / risk 축 기준으로 정리

### 2. 기존 Request Routing 설계 문서 archive

- 기존 Request / Receiver / Resolution / Coordinator 중심 문서를 archive로 이동
- active note와 충돌하지 않도록 관련 참조 갱신

```text
Docs/06_notes/archive/NA01_Combat_Intent_Request_Resolution_Routing_Design_Note.md
```

### 3. Combat Signal 타입 vocabulary 추가

새 타입 파일을 추가했다.

```text
Source/Portfolio/Type/CCombatSignalStructure.h
Source/Portfolio/Type/CCombatSignalStructure.cpp
```

추가된 주요 타입:

```text
ECombatSignalType
ECombatSignalOutcome
ECombatSignalResultType
FCombatSignalHeader
FCombatSignal
FCombatSignalContext
FCombatSignalEvaluation
FCombatSignalApplyResult
FCombatSignalResult
```

각 struct에는 최소 유효성 검사용 `IsValidMinimal()`을 두었다. 이번 단계에서는 actor 유효성이나 domain별 rule을 강제하지 않고, signal/result 타입의 기본 상태만 확인한다.

### 4. 작업 기록 체계 정리

- W04 작업 리스트 갱신
- task brief 추가
- work journal 정리
- prompt update note 정리

---

## 구현 범위

이번 PR에서 실제 runtime 연결은 하지 않았다.

- `ApplyDamageComponent` 변경 없음
- `TakeDamageComponent` 변경 없음
- Guard / Parry / Hit / Dead 동작 변경 없음
- CombatSignal 타입은 아직 기존 damage packet 흐름에 연결하지 않음

이번 PR의 구현 범위는 다음으로 제한했다.

- Combat Signal 타입 파일 추가
- 타입별 최소 유효성 검사 추가
- 관련 문서와 작업 기록 갱신

---

## 검증

### 빌드

```text
PortfolioEditor Win64 Development
```

- UnrealHeaderTool 통과
- `CCombatSignalStructure.cpp` 컴파일 통과
- Editor target 빌드 성공

### 정적 확인

- 기존 gameplay component 변경 없음
- 기존 damage / guard / parry runtime 연결 없음
- 신규 타입 파일은 `CoreMinimal.h`와 generated header만 포함

---

## 제외 범위

이번 PR에서는 다음 작업을 의도적으로 제외했다.

- `TakeDamageComponent`의 Target 내부 경계 정리
- `ApplyDamageComponent`의 Source 내부 경계 정리
- 컴포넌트 rename
- CombatSignal 타입을 기존 damage 흐름에 연결
- Blink / Repulse cue 구현
- 별도 Coordinator / Gateway 계층 구현

---

## 후속 작업

권장 후속 브랜치는 다음과 같다.

```text
refactor/combat-signal-target-v1
```

후속 작업 목표:

- `TakeDamageComponent` 내부를 Target 관점으로 정리
- Receive / Evaluate / Apply / Notify 단계 명시
- 기존 Guard / Parry / Hit / Dead 동작 유지
- CombatSignal 타입 연결 여부는 필요한 지점에서만 검토

그 다음 후보:

```text
refactor/combat-signal-source-v1
refactor/combat-signal-component-rename
feature/combat-signal-cue-v1
```

---

## 관련 문서

- `Docs/06_notes/N05_Combat_Signal_Boundary_Design_Note.md`
- `Docs/06_notes/N06_Combat_Signal_Branch_Implementation_Plan.md`
- `Docs/06_notes/task_briefs/W04_Combat_Signal_Boundary/TB_W04_01_Combat_Signal_Boundary_Rescope.md`
- `Docs/06_notes/task_briefs/W04_Combat_Signal_Boundary/TB_W04_02_Combat_Signal_Types_v1.md`
- `Docs/06_notes/work_journal/J01_Combat_Signal_Boundary_Work_Journal.md`
- `Docs/06_notes/prompt_updates/PU01_Combat_Signal_Boundary_Prompt_Update_Note.md`
