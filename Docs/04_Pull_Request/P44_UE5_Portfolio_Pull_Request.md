# UE5 Portfolio Pull Request

## 제목

**P44: Comment / Section Cleanup Policy**

## 날짜

**2026.07.23**

## 상태

- [x] comment / section cleanup 작업 계획 문서 작성
- [x] stale / 오타 / 잘못된 주석 정리
- [x] code-local TODO 분류 및 유지 기준 확정
- [x] header / implementation section comment 양식 정리
- [x] 반복 설명 / 태그형 / 장식성 주석 제거
- [x] comment usage rule 문서화
- [x] P3 유지 / 후속 이관 판단 기록
- [x] PR final scan 완료

## 브랜치

- `refactor/comment-section-cleanup`

## 요약

이번 PR은 P42 `Debug Log Policy v1`, P43 `CVar Ownership Policy` 이후 이어지는 code quality cleanup 작업으로, 프로젝트 내부 주석과 section comment 사용 기준을 정리한다.

기존 코드에는 다음 성격의 주석이 섞여 있었다.

```text
1. 실제 정책 / 예외 이유를 설명하는 주석
2. header / cpp 구현부를 나누는 section comment
3. Debug / Profiling helper 구역을 나누는 comment
4. 코드가 그대로 말하는 반복 설명 주석
5. [NOTE] / [Policy] / [Pass] 같은 태그형 주석
6. separator / banner 형태의 장식성 주석
7. 구현 위치에 남겨야 하는 TODO
```

이번 PR에서는 주석을 전부 제거하는 것이 아니라, "코드가 직접 말하지 못하는 이유 / 정책 / 예외 / 구역"만 남기는 기준을 고정했다.

## 주요 변경

### 1. 작업 계획 문서 작성

`W05_Comment_Section_Cleanup_Work_Plan.md`를 추가하고, 이번 브랜치에서 다룰 범위와 제외 범위를 분리했다.

포함 범위:

```text
- stale / 오타 / 잘못된 주석 정리
- commented-out / temporary trace 확인
- TODO 분류
- section comment 양식 정리
- 불필요한 설명 주석 제거
- API / inline role comment 유효성 검토
```

제외 범위:

```text
- public API rename
- 구조체 나누기 / 헤더 배치 규칙
- DataAsset 분리 구현
- UPROPERTY Category 재설계
- RuntimeLOD CVar 위치 이동
- const 정합성 정리
- gameplay policy 구현
```

### 2. 주석 사용 규칙 고정

워크플랜 문서에 `Comment Usage Rules`를 추가했다.

최종 기준:

```text
Engine / Template Header
Debug Helper
Profiling Helper
Header API Section
Implementation Section
Algorithm / Step
Policy / Exception Reason
Type / Data Meaning
Sparse / One-off
```

핵심 원칙:

```text
무엇을 하는지 반복 설명하지 않는다.
왜 필요한지, 어떤 정책/예외인지 설명한다.
section comment는 짧은 명사구로 제한한다.
태그형 [NOTE] / [Policy] / [Pass] 주석은 사용하지 않는다.
separator / banner comment는 사용하지 않는다.
UPROPERTY 변수 구간은 주석보다 Category / 변수명 / struct 분리로 관리한다.
```

### 3. stale / 오타 / 잘못된 표현 정리

다음 유형을 정리했다.

```text
Acitve -> Active
Delgate -> Delegate
Flag Toogle -> Flag Toggle
Deffered -> Deferred
Seperate -> Separate
Stemina -> Stamina
BroadCast -> Broadcast
FallBack -> Fallback
```

CVar help text와 debug / profiling 설명 문구도 실제 역할과 맞게 보정했다.

### 4. TODO 분류

의미 없는 TODO와 이미 문서로 추적 가능한 TODO는 제거했다.

남긴 TODO는 다음 6개다.

```text
CPlayer.cpp
-> TODO(Gameplay): dead actor TakeDamage route 정책

CEnemy.cpp
-> TODO(Gameplay): dead actor TakeDamage route 정책

CCombatSignalTargetComponent.cpp
-> TODO(CombatPolicy): target-side defensive gates
-> TODO(CombatPolicy): mitigation policy
-> TODO(CombatPolicy): final damage policy
-> TODO(CombatPolicy): resource commit order
```

이 TODO들은 실제 구현 위치와 직접 연결되어 있어 code-local TODO로 유지한다.

### 5. section comment 정리

header / cpp 구현부 section comment를 다음 기준으로 맞췄다.

```text
Lifecycle
Component Reference
Query
Mutation
Runtime Lifecycle
Runtime State
Notify Routing
Data Build
State Transition
Gate
Counter
Diagnostic Hook
Debug Dump
```

`API`, `Current`, `Used`, `Temp`처럼 상태성 또는 넓은 표현은 제거하거나 더 구체적인 책임명으로 바꿨다.

예:

```text
Interface API -> Interface
ActionEvent API -> Action Event Routing
Request API -> Chain Combat Request
Mapping API -> Chain Intent Mapping
```

### 6. 반복 설명 / 태그형 / 장식성 주석 제거

다음 유형을 제거하거나 문장형으로 바꿨다.

```text
[NOTE]
[Policy]
[Pass]
/* === ... === */
// -----------------------------------------------------------------------------
// Exact
// Tier 0
// Cached
// Remove Invalid Entry
// Set Cooldown
// Update blackboard
// MODE 1 / MODE 2 / MODE 0
```

정책 의미가 있는 주석은 태그를 제거하고 이유 중심 문장으로 바꿨다.

예:

```cpp
// Dead reaction is terminal and cannot be interrupted.
```

### 7. Type / Data 주석 최종 판단

`CAIStructure.h`, `CWeaponStructure.h`에 남은 Type / Data 주석은 이번 브랜치에서 무리하게 제거하지 않는다.

유지:

```text
enum None / All / Max sentinel 설명
FDamageAmount damage pipeline 의미 설명
GetTypeHash ADL 관련 짧은 설명
```

후속 이관:

```text
FAIContext field group 주석
FOverlapContext actor/component alias 주석
```

이 항목은 `구조체 나누기 / 헤더 배치 규칙`, `네이밍 / 매개변수 작명 규칙`, `UPROPERTY Category 정리`에서 다시 판단한다.

## 변경 파일 범위

```text
Docs/01_Work_List/W05_Code_Quality_Plan/W05_Comment_Section_Cleanup_Work_Plan.md
Docs/01_Work_List/W05_Code_Quality_Plan/W05_UE5_Portfolio_Work_List.md
Docs/04_Pull_Request/00_Pull_Request_Index.md
Docs/04_Pull_Request/P44_UE5_Portfolio_Pull_Request.md

Source/Portfolio/AI/*
Source/Portfolio/Action/*
Source/Portfolio/Character/*
Source/Portfolio/Component/*
Source/Portfolio/Controller/*
Source/Portfolio/Core/Debug/*
Source/Portfolio/Core/Profiling/*
Source/Portfolio/Reaction/*
Source/Portfolio/System/Combat/*
Source/Portfolio/Type/*
Source/Portfolio/Weapon/*
```

## 검증

### Static check

```text
git status --short
Result: clean

git diff --check
Result: Pass
```

### Comment rule scan

다음 패턴을 재검색했다.

```text
/***
***/
^[space]*// [Tag]
[NOTE]
[Policy]
[Pass]
override this API
Interface API
ActionEvent API
// -----------------------------------------------------------------------------
Who: request source
What: requested intent
How: intent event
[Case_
```

결과:

```text
Result: 0
```

### TODO scan

```text
TODO(Gameplay): 2
TODO(CombatPolicy): 4
```

판단:

```text
남은 TODO 6개는 실제 구현 위치와 직접 연결된 정책 TODO이므로 유지한다.
```

### Build

```text
Not run
```

사유:

```text
이번 PR은 주석 / 문서 / section label 정리 중심이다.
CReactionFeedbackComponent.cpp는 빈 wildcard branch를 제거하면서 동등 조건으로 정리했지만, 분기 의미는 기존과 동일하다.
빌드가 필요한 header macro / reflection / asset reference 변경은 없다.
```

## 후속 작업

이번 PR에서 처리하지 않는 항목:

```text
네이밍 / 매개변수 작명 규칙
-> public API rename
-> helper/API suffix 통일
-> 매개변수명 정합성 정리

구조체 나누기 / 헤더 배치 규칙
-> CWeaponStructure.h 대형 type 분리
-> FAIContext field group 정리
-> FOverlapContext alias 정리
-> EBTServiceIntervalPreset 위치 재검토

UPROPERTY / Editor 노출 정리
-> UPROPERTY Category naming 통일
-> Category 변경에 따른 asset 영향 확인

데이터 에셋 분리
-> Action / Reaction data map build 이동
-> DamageSpecContainer DataAsset migration
-> AI perception config DataAsset migration
-> feedback data type 이동

상수 / CVar / RuntimeLOD 구조 정리
-> CEnemy RuntimeLOD CVar 위치 이동
-> RuntimeLOD policy/config 재구성

const 정합성
-> read-only API const 정리

Gameplay / CombatPolicy TODO 구현
-> dead actor TakeDamage route 정책
-> target-side defensive gates
-> mitigation / final damage / resource commit order
```

## PR 설명 초안

```md
## Summary

- comment / section cleanup 작업 계획과 주석 사용 규칙을 문서화
- stale / 오타 / 잘못된 주석, 태그형 주석, 장식성 banner, 코드 반복 설명 주석 정리
- header / cpp section comment를 짧은 책임명 중심으로 정리
- code-local TODO 6개를 Gameplay / CombatPolicy 후속 작업으로 유지 판단
- Type / Data 주석 중 구조체/헤더 배치 작업으로 넘길 항목을 P3 final decision으로 기록

## Validation

- git status --short clean
- git diff --check 통과
- comment rule violation scan 결과 0
- TODO scan 결과: 의도적으로 유지한 6개만 남음
- Build not run: 주석 / 문서 / section label 정리 중심, reflection / asset reference 변경 없음
```
