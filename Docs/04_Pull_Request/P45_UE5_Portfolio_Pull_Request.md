# UE5 Portfolio Pull Request

## 제목

**P45: Naming / Typo / API Cleanup**

## 날짜

**2026.07.23**

## 상태

- [x] 네이밍 규칙 문서 분리
- [x] local naming / typo 불일치 정리
- [x] bool query public API를 `Should...` 기준으로 정리
- [x] profiling helper counter API suffix 정리
- [x] `CWorldSubsystemStructure` 파일명 / include 표기 정리
- [x] 네이밍 잔여 패턴 재확인
- [x] `PortfolioEditor Win64 Development` build 통과
- [x] PIE 확인

## 브랜치

- `refactor/naming-typo-api-cleanup`

## 요약

이번 PR은 W05 code quality cleanup의 네이밍 / 오타 / API 표기 정리 범위를 처리한다.

주석 / CVar / debug helper처럼 별도 정책이 필요한 구조 변경은 이전 PR에서 분리했고, 이번 PR에서는 기능 동작을 바꾸지 않는 이름 정합성에 집중했다.

정리 기준은 다음 문서로 고정했다.

```text
Docs/01_Work_List/W05_Code_Quality_Plan/W05_Naming_Rules.md
Docs/01_Work_List/W05_Code_Quality_Plan/W05_Naming_Typo_API_Cleanup_Work_Plan.md
```

## 주요 변경

### 1. 네이밍 규칙 문서화

다음 기준을 W05 문서로 분리했다.

```text
Public type / API
-> PascalCase

Input / output parameter
-> In / Out / InOut prefix

Reference style
-> FType& Value
-> const FType& Value

Local variable
-> lowerCamelCase

bool variable
-> bPascalCase

bool query function
-> Is / Has / Can / Should

DI / runtime cache suffix
-> _Injected / _Cached 유지
```

`Comp` / `Component` 전면 통일과 책임명이 애매한 public API rename은 구조 판단이 필요하므로 후속 범위로 남겼다.

### 2. local naming / typo 정리

단발 오타와 local-only naming 흔들림을 정리했다.

```text
CPlayerController.cpp
-> inAxisValue -> InAxisValue

CActionComponent.cpp
-> executorkey -> executorKey

CReactionComponent.cpp
-> executorkey -> executorKey

CBTService_UpdateAIContext.cpp
-> dist_target -> distanceToTarget
-> dist_home -> distanceToHome

CBTService_UpdateEngageContext.cpp
-> blackBoardComp -> blackboardComp
-> dist_target -> distanceToTarget

CBTService_UpdateEngageContext.h
-> FEngageContext & -> FEngageContext&

CWorldSubsystem_CombatEngage.h / .cpp
-> FEngageRequestContext & -> FEngageRequestContext&
```

### 3. bool query public API 정리

`Getb...` 형태의 bool getter를 query API 기준에 맞췄다.

```text
CEnemy.h
-> GetbUsePatrol() -> ShouldUsePatrol()
-> GetbUseInvestigate() -> ShouldUseInvestigate()
-> GetbUseAlertStep() -> ShouldUseAlertStep()

호출부
-> CAIController.cpp
```

### 4. profiling helper API suffix 정리

`Core/Profiling` helper class는 class 이름이 profiling 책임을 이미 드러내므로 `ForProfiling` suffix를 제거했다.

```text
CAIAnimationProfiling
-> RecordAnimationRefreshAttempt()
-> RecordAnimationRefreshExecuted()
-> RecordAnimationRefreshSkipped()

CAIBehaviorTreeProfiling
-> RecordUpdateAIContextTick()
-> RecordUpdateAIIntentStateTick()
-> RecordUpdateEngageContextTick()
-> RecordAIIntentIntervalPreset()

CAIStateRuntimeLODProfiling
-> RecordResolvedTier()
```

`UCAnimInstance`의 owner-side profiling wrapper도 같은 기준으로 suffix 없는 `Record...` 형태로 맞췄다.

### 5. World subsystem structure 표기 정리

파일명과 include 경로에 남아 있던 `SubSystem` 표기를 `Subsystem`으로 정리했다.

```text
Source/Portfolio/Type/CWorldSubSystemStructure.h
-> Source/Portfolio/Type/CWorldSubsystemStructure.h

Source/Portfolio/Type/CWorldSubSystemStructure.cpp
-> Source/Portfolio/Type/CWorldSubsystemStructure.cpp

#include "Type/CWorldSubSystemStructure.h"
-> #include "Type/CWorldSubsystemStructure.h"

#include "CWorldSubSystemStructure.generated.h"
-> #include "CWorldSubsystemStructure.generated.h"
```

내부 `USTRUCT` / `UENUM` 타입명은 변경하지 않았다.

## 변경 파일 범위

```text
Docs/01_Work_List/W05_Code_Quality_Plan/*
Docs/04_Pull_Request/P45_UE5_Portfolio_Pull_Request.md
Docs/04_Pull_Request/00_Pull_Request_Index.md

Source/Portfolio/AI/BehaviorTree/Service/*
Source/Portfolio/AI/Blackboard/*
Source/Portfolio/AI/RuntimeLOD/*
Source/Portfolio/Character/*
Source/Portfolio/Component/*
Source/Portfolio/Controller/*
Source/Portfolio/Core/Debug/*
Source/Portfolio/Core/Profiling/*
Source/Portfolio/System/Combat/*
Source/Portfolio/Type/*
```

## 검증

### Branch scan

```text
Branch:
refactor/naming-typo-api-cleanup

main 대비 변경:
36 files changed, 483 insertions(+), 92 deletions(-)
```

### Static check

다음 잔여 패턴을 확인했다.

```powershell
rg -n "inAxisValue|executorkey|dist_target|dist_home|blackBoardComp|FEngageContext &|FEngageRequestContext &" ..\Source\Portfolio --glob "*.h" --glob "*.cpp"
Result: 0

rg -n "GetbUse|Record[A-Za-z0-9]+ForProfiling|CWorldSubSystemStructure" ..\Source\Portfolio --glob "*.h" --glob "*.cpp"
Result: 0

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
PIE
Result: Pass
```

## 후속 작업

이번 PR에서 처리하지 않는 항목:

```text
Comp vs Component 전면 통일
-> public getter, injected/cache member, local variable까지 영향
-> Unreal engine / parent API collision 회피 이름도 같이 봐야 하므로 별도 브랜치에서 처리

책임명 자체가 애매한 public API rename
-> 실제 책임 재분류가 필요할 수 있음
-> 단순 네이밍 정리 범위에서 제외
```
