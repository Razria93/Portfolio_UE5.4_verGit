# UE5 Portfolio Pull Request

## 제목

**P50: Section Comment Consistency**

## 날짜

**2026.07.25**

## 상태

- [x] 섹션 주석 판단 규칙 문서화
- [x] 파일군별 section policy 정리
- [x] `.h / .cpp` 책임 섹션 동기화 기준 반영
- [x] Action / Reaction / Component / Controller / System / Weapon 섹션 정리
- [x] BT Service / RuntimeLOD / Core Profiling / Type 구현 cpp 섹션 정리
- [x] 단계형 / Case / 로컬 설명형 주석 정리
- [x] 예외 파일군 판단 기준 문서화
- [x] 큰 파일 무섹션 후보 재스캔 완료
- [x] `git diff --check` 통과
- [x] `PortfolioEditor Win64 Development` build 통과
- [x] PIE smoke 확인

## 브랜치

- `refactor/section-comment-consistency`

## 요약

이번 PR은 W05 코드 품질 정리 흐름의 후속 작업으로, `Source/Portfolio`의 섹션 주석 체계와 파일군별 판단 기준을 정리한다.

핵심 기준은 모든 파일에 같은 섹션을 강제하는 것이 아니라, 같은 성격 / 책임 / 파일 패턴을 가진 파일군끼리 같은 섹션 체계를 쓰는 것이다. 짧은 UE adapter, Interface, Type helper처럼 함수 자체가 구조를 설명하는 파일은 섹션 생략을 허용하고, Component / Controller / Action / Reaction / BT Service처럼 책임이 반복되는 파일군은 공통 섹션명을 우선 사용한다.

코드 동작, 타입명, 함수명, 시그니처, UPROPERTY, UFUNCTION, Blueprint 노출 계약은 변경하지 않았다.

## 변경 배경

기존 코드에는 다음 문제가 섞여 있었다.

```text
-> 같은 책임인데 파일마다 섹션명이 다름
-> .h에는 섹션이 있지만 .cpp에는 대응 섹션이 없음
-> .cpp에는 구현 흐름 섹션이 있지만 .h 책임 구조와 연결이 약함
-> Case 1, Another Case, Result, Context 같은 로컬 설명이 섹션처럼 보임
-> 짧은 파일과 큰 파일에 같은 기준을 적용할지 판단 기준이 불명확함
```

이번 PR에서는 이를 한 번에 같은 규칙으로 정리했다.

```text
1. 같은 파일군은 같은 책임 섹션을 사용한다.
2. 파일 고유 책임은 고유 섹션으로 허용한다.
3. .h / .cpp는 함수 1:1이 아니라 책임 그룹 기준으로 동기화한다.
4. 로컬 흐름 설명은 섹션이 아니라 문장형 주석으로 작성한다.
5. 짧고 단일 책임인 UE adapter / Interface / Type cpp는 섹션을 생략할 수 있다.
```

## 변경 범위

### 1. 섹션 주석 정책 문서화

무엇

W05 comment / section cleanup 문서에 파일군별 section policy를 추가했다.

어떻게

다음 파일군을 독립 판단 대상으로 정리했다.

```text
-> Action / Reaction Base
-> Action / Reaction Derived
-> Component
-> Controller
-> Behavior Tree Service / Task / Decorator
-> AI RuntimeLOD Policy
-> Character
-> System
-> Weapon
-> Type
-> Core Debug / Profiling
-> Notify / Interface / Blackboard / Patrol / Module
```

결과

`Source/Portfolio` 상위 계열 전체가 섹션 주석 판단 규칙 안에 들어왔다.

### 2. Action / Reaction 계열 섹션 정리

무엇

Action / Reaction base와 파생 클래스의 섹션명을 계열 기준으로 맞췄다.

어떻게

공통 책임은 아래 이름을 우선 사용했다.

```text
-> Decision
-> Lifecycle
-> Notify
-> Observable Overlay
-> Intervention
-> State Transition
```

파일 고유 책임은 고유 섹션으로 유지했다.

```text
-> Chain Reservation
-> Chain Window
-> Chain Query
-> Weapon
-> Guard State Cleanup
```

결과

`CAction_ComboAttack`, `CAction_Dodge`, `CAction_Guard`, `CAction_Equip`, `CAction_Unequip`, `CReaction_Hit`, `CReaction_Dead`, `CReaction_Stagger`, `CReaction_Parry`, `CReaction_BlockHit`의 책임 구분이 base class와 같은 기준으로 정리됐다.

### 3. Component / Controller / Character / System / Weapon 섹션 정리

무엇

큰 gameplay 파일과 runtime service 파일의 `.h / .cpp` 책임 섹션을 동기화했다.

어떻게

Component 계열은 다음 공통 섹션을 우선 사용했다.

```text
-> Component Reference
-> Lifecycle
-> Runtime Lifecycle
-> Query
-> Mutation
-> Runtime State
-> State Transition
-> Request / Entry
-> Receive
-> Resolve
-> Apply
-> Packet
-> Notify
-> Helper
```

도메인 고유 섹션은 유지했다.

```text
-> Movement Arbitration
-> Movement Input
-> Movement Policy
-> Hit Window
-> AI Entry
-> Camera Shake
-> Assignment Build
-> Assignment Warmup
-> Engine Delegate Events
```

결과

Component / Controller / Character / System / Weapon 계열의 공통 책임 섹션이 같은 이름으로 정리됐고, 파일 고유 책임은 필요한 곳에만 남았다.

### 4. BT Service / RuntimeLOD / Core Profiling / Type cpp 정리

무엇

BT Service, RuntimeLOD resolver, Core Profiling, Type implementation cpp의 섹션 기준을 보강했다.

어떻게

BT Service는 context / blackboard 흐름 기준으로 정리했다.

```text
-> Lifecycle
-> Context Build
-> Context Compute
-> Blackboard Update
-> Blackboard Clear
-> Intent Decision
-> Intent Transition
```

Core Profiling은 profiling 책임 기준으로 정리했다.

```text
-> Gate
-> Counter
-> Service Tick Counter
-> Interval Preset Counter
```

Type cpp는 헤더 taxonomy와 맞는 최소 섹션만 추가했다.

```text
-> Helper API
-> Data / Config
-> Runtime State
```

결과

BT Service / RuntimeLOD / Core Profiling / Type 구현 파일의 책임 구분이 문서 규칙과 코드 양쪽에서 맞춰졌다.

### 5. 단계형 / Case / 로컬 설명 주석 정리

무엇

섹션처럼 보이던 로컬 설명 주석을 문장형 설명으로 바꿨다.

어떻게

다음 패턴을 정리했다.

```text
-> Case 1 / Case 2
-> GuardState Case / Another Case
-> Default Action Case / Default Reaction Case
-> Idle && No ActivePart / No Idle && Has ActivePart
-> Based OwnerPawn / Based Perception / Based TargetActor
-> Absolute States / Context / Result
-> Resolve Executor / Candidate SpecKey
-> Delay for Warmup / Flag Toggle
-> Slow InActor / Restore InActor
-> Early-Return / Invalid / Legacy delegate
-> V1 hook only
```

결과

섹션 주석과 로컬 흐름 설명의 경계가 분리됐다.

### 6. 예외 파일군 기준 정리

무엇

무조건 섹션을 넣지 않아도 되는 파일군을 명시했다.

어떻게

다음 파일군은 짧거나, framework adapter 성격이 강하거나, 자체 구조가 명확하면 섹션 생략을 허용한다.

```text
-> BT Task / Decorator
-> Notify
-> Interface
-> AI Blackboard
-> AI Patrol
-> 짧은 Type cpp
-> Module / Global
```

결과

일관성과 파일 고유성 사이의 판단 기준이 문서화됐다.

## 주요 처리 흐름

```text
섹션 주석 정책 문서화
-> 파일군별 판단 기준 정리
-> Type header taxonomy 재확인
-> Action / Reaction 계열 섹션 정리
-> Component / Controller / Character / System / Weapon 섹션 정리
-> BT Service / RuntimeLOD / Core Profiling / Type cpp 섹션 정리
-> 단계형 / Case / 로컬 설명 주석 정리
-> 예외 파일군 기준 정리
-> 큰 파일 무섹션 후보 재스캔
-> git diff --check
-> PortfolioEditor Win64 Development build
-> PIE smoke
```

## 구현 결과

```text
main...HEAD
-> 107 files changed, 1626 insertions(+), 195 deletions(-)

commit range
-> 5fd828bf docs(style): define section comment consistency rules
-> d7c6e9a8b docs(style): define section comment workflow rules
-> d0cc14ce style(type): add section taxonomy comments
-> 2903361a style(combat): sync source section comments
-> 85e4f83f style(feedback): sync header and source sections
-> 24d7b073 style(combat): normalize signal step comments
-> d82750d3 style(core): sync remaining header and source sections
-> 5480b14f style(component): normalize remaining step comments
-> 06fce7c3 style(comments): align controller blackboard section label
-> c20b2107 style(comments): standardize section comment layout
-> ee6f1b86 docs(comments): clarify section synchronization rules
-> ca6c7cf8 refactor(style): align section comment policy
-> 10c68dc3 refactor(style): align section comment policy
```

변경함:

```text
-> 섹션 주석 정책 문서화
-> 파일군별 section policy 추가
-> .h / .cpp 책임 섹션 동기화
-> Action / Reaction / Component / Controller / Character / System / Weapon 섹션 정리
-> BT Service / RuntimeLOD / Core Profiling / Type cpp 섹션 보강
-> 단계형 / Case / 로컬 설명 주석 정리
-> AI Blackboard helper 섹션 보강
-> CHealthComponent.h 중복 InitializeHealth 선언 제거
```

변경하지 않음:

```text
-> 타입명
-> 함수명
-> enum entry
-> UPROPERTY
-> UFUNCTION / Blueprint 노출 signature
-> delegate signature
-> override signature
-> gameplay logic
-> CombatSignal reserved scaffold
-> Feedback key model
```

## 테스트 방법

```text
1. 파일군별 section policy 문서 확인
2. 단계형 / Case / 흔들리는 로컬 라벨 패턴 재스캔
3. 큰 파일 무섹션 후보 재스캔
4. git diff --check
5. PortfolioEditor Win64 Development build
6. PIE smoke 실행
```

## 검증 결과

### Static check

```text
git diff --check
Result: Pass
```

### Pattern scan

```text
단계형 / Case / 흔들리는 로컬 라벨 후보
Result: 0건

큰 파일 무섹션 후보
Result: 0건

남은 Condition / Record
Result: CAnimInstance 유지 결정 범위

남은 Result
Result: Type taxonomy 섹션
```

### Build

```text
PortfolioEditor Win64 Development
Result: Pass
```

### PIE

```text
PIE smoke
Result: Pass
```

## 비고 / 후속 작업

- 이번 PR은 주석 / 섹션 / 문서 정책 정리이며 gameplay behavior 변경 PR이 아니다.
- Notify / Interface / BT Task / Decorator / 짧은 Type cpp는 파일이 커지거나 책임이 나뉘는 시점에만 섹션을 추가한다.
- 향후 AI 작업 시스템은 `File Family Section Policy`를 기준으로 같은 파일군의 섹션명을 우선 맞춘다.

## 관련 문서

- Work List: `W05_UE5_Portfolio_Work_List.md`
- Section Cleanup Work Plan: `W05_Comment_Section_Cleanup_Work_Plan.md`
- AI Workflow Rule: `02_Project_Stella_Working_Rule_Prompt (KR).md`
- Previous PR: `P49_UE5_Portfolio_Pull_Request.md`

## 정리

이번 PR은 파일군별 섹션 주석 판단 기준을 문서화하고, 주요 코드 파일의 섹션명을 같은 책임 단위로 정리했다.

결과적으로 `Source/Portfolio`의 모든 상위 파일군은 section policy 안에서 판단 가능해졌고, 섹션 주석과 로컬 흐름 설명의 경계도 분리됐다.
