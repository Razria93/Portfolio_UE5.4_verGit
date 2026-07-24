# UE5 Portfolio Pull Request

## 제목

**P46: Type Header Organization**

## 날짜

**2026.07.24**

## 상태

- [x] Type 헤더 분류 / 배치 규칙 문서화
- [x] 대형 umbrella Type 헤더 분해
- [x] Action / Reaction / Execution / Feedback / Combat / AI Type 책임 분리
- [x] Action / Reaction KeyTypes 분리
- [x] debug / audit state를 Core/Debug 소유 Type으로 이동
- [x] BT service interval preset Type 위치 정리
- [x] `PortfolioEditor Win64 Development` build 통과
- [x] PIE smoke 확인

## 브랜치

- `refactor/type-header-organization`

## 요약

이번 PR은 W05 code quality cleanup의 구조체 나누기 / Type 헤더 배치 규칙 범위를 처리한다.

기존 `CWeaponStructure.h` 같은 umbrella Type 헤더는 여러 도메인의 enum / struct를 한 파일에 묶고 있어, 작은 타입 하나를 쓰는 파일도 무기, action, reaction, damage, state, feedback 계층에 함께 의존하게 만들었다.

이번 PR에서는 타입 이름, `USTRUCT` / `UENUM` 이름, `UPROPERTY` 이름, enum entry는 유지하면서 파일 책임과 include 경계를 재배치했다. 목적은 동작 변경이 아니라, 공용 Type 헤더의 소유권과 include 파급 범위를 명확히 하는 것이다.

## 핵심 개념

이번 PR은 Type 헤더를 생명주기와 책임 단위로 나누고, lookup key와 editor data, runtime context가 같은 파일에 섞이지 않게 정리한다.

```text
Type 헤더
-> Source/Portfolio/Type 아래 공용 enum / struct 정의 파일

umbrella Type 헤더
-> 여러 도메인 타입을 한 번에 include하게 만들던 대형 Type 헤더

KeyTypes
-> lookup / match identity 전용 Type 헤더

DataTypes
-> editor 입력 data와 resolved execution context를 담는 Type 헤더

DebugTypes
-> gameplay shared Type이 아니라 Core/Debug helper가 소유하는 진단 상태 Type
```

## 변경 배경

W05 code quality cleanup에서 네이밍, 주석, debug / profiling 출력 정책을 정리한 뒤에도 공용 Type 헤더의 책임 경계는 여전히 넓게 남아 있었다.

특히 `CWeaponStructure.h`는 전투, action, reaction, feedback, state, health, damage 관련 타입을 한 파일에 묶고 있었다. 이 구조에서는 한 타입을 쓰기 위해 필요 이상의 도메인 헤더를 include하게 되고, Type 위치만 봐서는 타입의 생명주기와 소유 책임을 판단하기 어려웠다.

이번 PR은 구조체 rename이나 gameplay policy 변경을 하지 않고, 먼저 타입 배치 기준과 파일 책임을 고정하는 데 집중했다.

## 변경 범위

### 1. Type 헤더 규칙과 작업 계획 고정

왜:

타입을 책임 단위로 나누려면 먼저 `Data`, `Context`, `State`, `Request`, `Result`, `Packet`, `Key` 같은 이름의 의미가 고정되어야 했다.

어떻게:

`W05_Type_Header_Organization_Rules.md`에 판단 규칙을 두고, `W05_Type_Header_Organization_Work_Plan.md`에는 이번 브랜치의 실제 스캔 결과와 처리 / 보류 항목을 기록했다.

결과:

반복 적용 가능한 규칙과 이번 브랜치의 처리표가 분리되었다. 이후 rename / feedback 구조 / RuntimeLOD config 작업은 별도 pass로 이어갈 수 있게 됐다.

### 2. 대형 umbrella Type 헤더 분해

왜:

`CWeaponStructure.h`, `CWorldSubsystemStructure.h`, `CCombatSignalStructure.h` 같은 파일은 여러 도메인 타입을 한 번에 노출했다.

어떻게:

도메인별 Type 헤더로 분리했다.

```text
CWeaponTypes
CActionTypes / CActionDataTypes / CActionOrchestrationTypes
CReactionTypes / CReactionDataTypes / CReactionOrchestrationTypes
CExecutionTypes / CExecutionRuleTypes
CCombatHitTypes / CCombatDamageTypes / CCombatResultTypes
CCombatSignalTypes / CCombatSignalSourceTypes / CCombatSignalTargetTypes
CActionFeedbackTypes / CReactionFeedbackTypes / CCombatFeedbackTypes
CAITypes / CEngageAssignmentTypes
```

결과:

사용처가 필요한 Type 헤더를 직접 include하게 되었고, `Source/Portfolio` 코드에서 이전 umbrella Type 파일 참조가 제거되었다.

### 3. Action / Reaction key 책임 분리

왜:

`FActionDataKey`는 `CActionTypes.h`에 있고, `FReactionDataKey`는 `CReactionDataTypes.h`에 있어 같은 DataKey 성격의 타입이 서로 다른 층에 배치되어 있었다.

어떻게:

Key 전용 헤더를 추가했다.

```text
CActionKeyTypes.h / .cpp
-> FActionDataKey
-> GetTypeHash(FActionDataKey)
-> GetGuardActionPhaseIndex
-> ResolveGuardActionPhase

CReactionKeyTypes.h / .cpp
-> FReactionDataKey
-> GetTypeHash(FReactionDataKey)
```

결과:

Action / Reaction 모두 `Types = enum`, `KeyTypes = lookup key`, `DataTypes = data + execution context` 구조로 맞춰졌다.

`FActionContext`는 hit / combat signal에 실리는 action identity snapshot 성격이지만, `USTRUCT` / `UPROPERTY` 변경 리스크가 있어 이번 PR에서는 제거하지 않고 후속 pass로 남겼다.

### 4. Debug / audit state 소유권 이동

왜:

AI perception audit state와 engage assignment rebuild debug state가 gameplay shared Type 헤더에 있으면 일반 gameplay include가 진단 전용 타입까지 알게 된다.

어떻게:

진단 전용 state를 Core/Debug 소유 Type으로 이동했다.

```text
FAIPerceptionDebugTypes.h
-> FPerceptionCandidateAuditState
-> FBlackboardEngageLatencyAuditState

FCombatEngageDebugTypes.h
-> FEngageAssignmentRebuildDebugState
```

결과:

`CAITypes.h`, `CEngageAssignmentTypes.h`는 gameplay runtime context / assignment state 중심으로 유지되고, debug helper가 사용하는 진단 상태는 Core/Debug 쪽으로 분리되었다.

### 5. BT interval preset Type 경계 정리

왜:

`Core/Profiling`이 BT service helper에 있는 `EBTServiceIntervalPreset`을 참조하면서 profiling 계층이 BT service 구현 헤더에 의존했다.

어떻게:

공용 Type 헤더를 추가했다.

```text
CAIBehaviorTreeTypes.h
-> EBTServiceIntervalPreset
```

`CAIBehaviorTreeProfiling`은 더 이상 `CBTServiceIntervalHelper.h`를 include하지 않고, preset Type만 참조한다.

결과:

BT service interval 선택 로직과 profiling counter가 같은 enum을 공유하되, 구현 helper 의존은 끊어졌다.

## 주요 처리 흐름

```text
Type 이름 / UPROPERTY 이름 / enum entry 유지
-> Type 헤더 책임 규칙 고정
-> umbrella Type 헤더를 도메인별 Type 헤더로 분해
-> 사용처 include를 필요한 Type 헤더로 교체
-> Key / Data / Execution / Debug state 소유권을 분리
-> UHT / build / PIE로 compile 및 editor load 경로 확인
```

## 구현 결과

```text
Source/Portfolio/Type
-> 대형 structure 헤더 중심 구조에서 도메인별 Types / KeyTypes / DataTypes 구조로 전환

Source/Portfolio/Core/Debug
-> debug / audit state 소유 Type 추가

Source/Portfolio/Core/Profiling
-> BT service helper include 없이 interval preset counter 기록

Source/Portfolio 사용처
-> 이전 umbrella Type 헤더 직접 참조 제거
```

## 테스트 방법

```text
1. main 대비 diff와 변경 파일 범위 확인
2. 이전 umbrella Type 파일 참조 잔여 검색
3. 제거 대상 enum 잔여 검색
4. KeyTypes / DebugTypes / BT interval preset 위치 확인
5. git diff --check 실행
6. PortfolioEditor Win64 Development build 실행
7. PIE smoke 실행
8. PIE 로그에서 Error / Fatal / Ensure / Blueprint compile 실패 여부 확인
```

## 검증 결과

### Branch scan

```text
Branch:
refactor/type-header-organization

main 대비 변경:
155 files changed, 4546 insertions(+), 2818 deletions(-)
```

### Static check

```text
Source/Portfolio 내 이전 umbrella Type 파일 참조
Result: 0

EActionStopSource / EReactionStopSource / EAIUpdatePrecision 소스 잔여
Result: 0

FActionDataKey 정의 위치
Result: CActionKeyTypes.h

FReactionDataKey 정의 위치
Result: CReactionKeyTypes.h

EBTServiceIntervalPreset 정의 위치
Result: CAIBehaviorTreeTypes.h

git diff --check main...HEAD
Result: Pass
```

### Build

```text
PortfolioEditor Win64 Development
Result: Pass
```

### PIE

```text
PIE smoke
Map: /Game/00_UnitTest/TestRoom
Result: Pass
```

로그 확인:

```text
MapCheck
-> 오류 0 / 경고 0

Blueprint compile
-> No blueprints needed recompiling

PIE
-> PIE world 생성 / 시작 / 종료 정상

변경 관련 Error / Fatal / Ensure
-> 없음
```

## 비범위 / 후속 작업

이번 PR에서는 파일 책임과 include 경계를 정리했으며, 다음 항목은 의도적으로 후속 범위로 남겼다.

```text
FActionContext 제거 또는 rename
-> hit / combat signal action identity snapshot 성격이지만 USTRUCT / UPROPERTY 변경 검증이 필요함

Feedback MatchKey / PlaybackKey 구조 정리
-> playback dedupe 기준 변경 가능성이 있어 별도 동작 검증 필요

구조체 rename pass
-> FTargetData, FDamageImpactInfo, FDamageAmount, FPatrolPointData, FAIContext 등은 asset / Blueprint serialization 리스크가 있음

CEnemy RuntimeLOD CVar 이동
-> RuntimeLOD config / DataAsset 분리 작업에서 별도 처리
```

## 관련 문서

- Work List: `W05_UE5_Portfolio_Work_List.md`
- Type Header Rules: `W05_Type_Header_Organization_Rules.md`
- Type Header Work Plan: `W05_Type_Header_Organization_Work_Plan.md`
- Naming Rules: `W05_Naming_Rules.md`
- Comment Cleanup Work Plan: `W05_Comment_Section_Cleanup_Work_Plan.md`

## 정리

이번 PR은 W05 code quality cleanup 중 Type 헤더 책임 분리 범위를 닫는다.

동작 정책과 serialized 타입 이름은 유지하면서, 공용 Type 정의의 소유권과 include 경계를 정리했다. 이후 작업은 rename, feedback dedupe 정책, RuntimeLOD config처럼 동작 / 직렬화 리스크가 있는 범위로 분리해서 진행한다.
