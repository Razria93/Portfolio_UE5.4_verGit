# Debug Overlay P1 Code Clean Structure Review

## 1. 목적

이 문서는 `feature/debug-overlay-evidence-plan` 브랜치에서 구성한 Debug Overlay P1 코드 구조를 W05 Code Quality Plan, 과거 W05 PR 문서 패턴, 프로젝트 내부 UCLASS/header/layout 관례 기준으로 검토한 결과를 정리한다.

이번 단계는 코드 수정이 아니라 마감 전 cleanup 후보를 분류하는 작업이다. 기능 구현, asset 변경, config 변경, Build.cs 변경은 하지 않는다.

검토 목표:

- Debug Overlay 관련 코드가 기존 프로젝트 구조와 과하게 어긋나지 않는지 확인한다.
- 마감 전 바로 처리 가능한 `LowRiskFix`와 별도 판단이 필요한 `DecisionNeeded`를 분리한다.
- P1 마감 기준에서 건드리지 않을 `NoChange`와 P2 이후로 넘길 `Later`를 명확히 한다.
- 이후 cleanup 작업이 기능 변경이나 evidence claim 변경으로 번지지 않도록 범위를 잠근다.

## 2. 기준 문서

W05 기준:

- `Docs/01_Work_List/W05_Code_Quality_Plan/W05_Naming_Rules.md`
- `Docs/01_Work_List/W05_Code_Quality_Plan/W05_API_Const_Consistency_Work_Plan.md`
- `Docs/01_Work_List/W05_Code_Quality_Plan/W05_Tuning_Constants_Rules.md`
- `Docs/01_Work_List/W05_Code_Quality_Plan/W05_Type_Header_Organization_Rules.md`
- `Docs/01_Work_List/W05_Code_Quality_Plan/W05_Comment_Section_Cleanup_Work_Plan.md`

PR 패턴 기준:

- `Docs/04_Pull_Request/P42_UE5_Portfolio_Pull_Request.md`
- `Docs/04_Pull_Request/P43_UE5_Portfolio_Pull_Request.md`
- `Docs/04_Pull_Request/P44_UE5_Portfolio_Pull_Request.md`
- `Docs/04_Pull_Request/P45_UE5_Portfolio_Pull_Request.md`
- `Docs/04_Pull_Request/P46_UE5_Portfolio_Pull_Request.md`
- `Docs/04_Pull_Request/P47_UE5_Portfolio_Pull_Request.md`
- `Docs/04_Pull_Request/P48_UE5_Portfolio_Pull_Request.md`
- `Docs/04_Pull_Request/P49_UE5_Portfolio_Pull_Request.md`
- `Docs/04_Pull_Request/P50_UE5_Portfolio_Pull_Request.md`
- `Docs/04_Pull_Request/P51_UE5_Portfolio_Pull_Request.md`
- `Docs/04_Pull_Request/P52_UE5_Portfolio_Pull_Request.md`

Debug Overlay 기준:

- `Docs/07_Portfolio_Documents/Debug_Overlay/05_Verification/Debug_Overlay_P1_Code_Quality_Review_KR.md`
- `Docs/07_Portfolio_Documents/Debug_Overlay/05_Verification/Debug_Overlay_W05_PR_Style_Gap_Review_KR.md`
- `Docs/07_Portfolio_Documents/Debug_Overlay/01_Planning/Debug_Overlay_P1_Closure_Criteria_KR.md`
- `Docs/07_Portfolio_Documents/Debug_Overlay/01_Planning/Debug_Overlay_P1_Overlay_Layout_Style_Lock_KR.md`

## 3. 검토 대상 코드

| 파일 | 현재 역할 | 판단 |
| --- | --- | --- |
| `Source/Portfolio/Core/Debug/CDebugOverlayHUD.h/.cpp` | Canvas 기반 3-panel HUD, actor state, recent summary, EventLog panel, target resolve, Current AI 표시 | 가장 큰 cleanup 후보 |
| `Source/Portfolio/Core/Debug/CDebugOverlayFocusComponent.h/.cpp` | debug-only focus actor/source/command result 저장, public Target compatibility wrapper 유지 | 현재 구조 유지 |
| `Source/Portfolio/Core/Debug/FDebugOverlaySnapshotTypes.h` | snapshot/event/recent summary data-only type | 현재 구조 유지 |
| `Source/Portfolio/Core/Debug/FDebugOverlaySnapshotStore.h/.cpp` | world별 snapshot store, event ring, EventLog filter, recent summary 기록/query | 섹션 정리 후보 |
| `Source/Portfolio/Core/Debug/CDebugOverlayGameMode.h/.cpp` | TestRoom debug HUD 연결 | 현재 구조 유지 |
| `Source/Portfolio/Core/Debug/FExecutionOrchestratorDebug.cpp` | execution decision을 SnapshotStore에 기록하는 debug hook | 현재 구조 유지 |
| `Source/Portfolio/Core/Debug/FCombatSignalDebug.cpp` | collision window, target packet, combat result를 SnapshotStore에 기록하는 debug hook | 현재 구조 유지 |
| `Source/Portfolio/Core/Debug/FCombatResultDebug.cpp` | combat result receive를 SnapshotStore에 기록하는 debug hook | 현재 구조 유지 |
| `Source/Portfolio/Core/Debug/FAICombatBTDebug.cpp` | AI combat task event를 SnapshotStore에 기록하는 debug hook | 현재 구조 유지 |
| `Source/Portfolio/Controller/CPlayerController.h/.cpp` | debug target Exec command entry, FocusResolver 호출, FocusComponent focus result apply | 현재 구조 유지 |

## 4. W05 / PR 축별 판단

| 기준 | 현재 상태 | 판단 |
| --- | --- | --- |
| P42 Debug Log Policy | gameplay hook은 Store 기록 호출 중심이고, 화면 출력은 HUD가 담당한다. | 유지 |
| P43 CVar Ownership | `Portfolio.DebugOverlay.*` CVar는 Store/HUD/Controller debug 경로에 한정된다. | 유지 |
| P44 Comment / Section Cleanup | Store/HUD helper가 길어졌고 HUD anonymous namespace의 섹션 구분이 약하다. | LowRiskFix |
| P45 Naming / API Cleanup | `Try`, `Append`, `Format`, `Resolve`, `Record` 계열 이름은 대체로 맞다. 일부 역할 이름 보강 후보가 있다. | LowRiskFix |
| P46 Type Header Organization | SnapshotTypes는 data-only, Store는 API-only static utility, TargetComponent는 UActorComponent로 역할이 분리되어 있다. | 유지 |
| P47 Meaning Cleanup | `Pannel_01/02/03` 오탈자처럼 보이는 표현은 현재 style lock에 의해 표시값으로 고정되어 있다. | NoChange |
| P48 Include Order Cleanup | 대부분 matching header -> project/internal -> engine 흐름을 따른다. 일부 재확인 후보만 있다. | LowRiskFix |
| P49 API Const Consistency | getter/query const는 대체로 지켜진다. HUD draw/cache/diagnostic record는 const 대상이 아니다. | 유지 |
| P50 Section Comment Consistency | Store API section은 유지 가능하다. HUD helper section은 더 명확히 나눌 수 있다. | LowRiskFix |
| P51 Tuning Constants Cleanup | HUD layout/style 값은 style lock 기준으로 유지한다. radius/stale/event limit은 정책 상수 또는 CVar contract로 본다. | 유지 |

## 5. 구조 판단

### 5.1 HUD

`CDebugOverlayHUD.cpp`는 현재 Debug Overlay P1에서 가장 많은 책임을 가진 파일이다.

포함 책임:

- Player / Enemy actor current state formatting
- actor-local Recent Execution formatting
- Enemy Current AI / Recent AI Event formatting
- Interaction panel formatting
- EventLog panel formatting
- panel layout / background / header draw
- TargetComponent Focus API 기반 Enemy resolve

현재 구조는 P1 마감 기준으로 허용 가능하다. Debug overlay는 개발 전용 tooling이고, 현재 파일 분리는 기능 안정화보다 리스크가 크다.

다만 다음 cleanup에서는 동작 변경 없이 helper 섹션과 일부 helper 이름을 정리할 수 있다.

### 5.2 Store

`FDebugOverlaySnapshotStore.cpp`는 record, query, filter, subject match를 모두 포함한다.

현재 유지해야 할 점:

- Store ring buffer record path는 모든 event를 보존한다.
- display filter는 조회/display 단계에서 적용한다.
- EventLog schema에는 actor pointer를 넣지 않는다.
- `Recent Combat`은 collision lifecycle이 아니라 combat 판정 evidence summary로 유지한다.

정리 후보:

- display filter helper, subject match helper, collect helper 섹션을 더 명확히 나눌 수 있다.
- `ExtractSummaryFieldValue`처럼 legacy `=`와 현재 `: ` 표기를 함께 처리하는 helper는 위치와 이름으로 역할을 더 명확히 할 수 있다.

### 5.3 FocusComponent / PlayerController

`UCDebugOverlayFocusComponent`는 debug-only focus actor/source/command result만 저장하고 public Target API는 compatibility wrapper로 유지하므로 현재 책임이 적절하다.

`ACPlayerController`는 debug Exec command entry, FocusResolver 호출, focus result apply, Output Log, command result recording만 담당한다. gameplay input/action flow를 바꾸지 않고 console command로만 동작하므로 P1에서는 유지한다.

다만 `CPlayerController.h`에서 Exec command가 field보다 앞에 배치된 구조는 프로젝트 일반 UCLASS field-first 스타일과 다를 수 있다. console command 가시성 목적이 있으므로 이번 cleanup에서 임의로 이동하지 않는다.

### 5.4 Debug hook files

`FExecutionOrchestratorDebug.cpp`, `FCombatSignalDebug.cpp`, `FCombatResultDebug.cpp`, `FAICombatBTDebug.cpp`는 각 도메인 hook에서 SnapshotStore record API를 호출하는 얇은 연결부로 남아 있다.

이 구조는 유지한다. Store로 도메인 hook 책임을 끌어오면 오히려 Store의 도메인 의존성이 커진다.

## 6. Findings

### 6.1 LowRiskFix

| 항목 | 대상 | 제안 |
| --- | --- | --- |
| HUD helper section 정리 | `CDebugOverlayHUD.cpp` | anonymous namespace를 `Layout Constants`, `Text Formatting`, `Actor State`, `Recent Summary`, `EventLog`, `Panel Drawing`, `Target Resolve` 정도로 나눈다. |
| HUD helper naming 정리 | `CDebugOverlayHUD.cpp` | `AppendSnapshotLines`는 실제 역할이 Interaction panel 구성에 가까우므로 `AppendInteractionPanelLines` 같은 이름을 검토한다. |
| panel helper naming 보강 | `CDebugOverlayHUD.cpp` | title/header 판정 helper가 문자열 비교 기반이라면 `IsPanelTitleLine`, `IsSectionHeaderLine`처럼 역할을 드러내는 이름을 검토한다. |
| HUD layout/style 상수 섹션화 | `CDebugOverlayHUD.cpp` | width, gap, margin, padding, color, stale timeout을 섹션별로 묶되 값은 style lock 기준으로 유지한다. |
| Store helper section 정리 | `FDebugOverlaySnapshotStore.cpp` | event filter, subject match, event collect, snapshot copy helper를 섹션으로 분리한다. |
| include order 재확인 | HUD / Store / PlayerController `.cpp` | W05 P48 기준으로 matching header, project headers, type headers, engine headers 순서를 재확인한다. |
| P52 최신화 | `Docs/04_Pull_Request/P52_UE5_Portfolio_Pull_Request.md` | 코드 클린 리뷰 문서 링크와 보류 항목을 PR 후보 문서에서 추적 가능하게 한다. |

LowRiskFix 기준:

- 표시값, CVar 이름, schema, Store public API, gameplay flow를 바꾸지 않는다.
- 빌드 결과와 PIE 동작이 동일해야 한다.
- `.umap`, `.uasset`, config, `Build.cs`를 건드리지 않는다.

### 6.2 DecisionNeeded

| 항목 | 대상 | 결정 필요 이유 |
| --- | --- | --- |
| HUD 파일 분리 | `CDebugOverlayHUD.cpp` | formatting, panel layout, actor resolve, AI blackboard read가 한 파일에 모여 있다. 분리하면 읽기는 좋아지지만 파일/헤더 증가와 마감 전 리스크가 있다. |
| AI Current source 이동 | `CDebugOverlayHUD.cpp` -> Store 또는 별도 provider | 현재 HUD가 Blackboard를 직접 읽는다. HUD를 display-only로 둘지, Current AI snapshot을 Store에 기록할지 결정이 필요하다. |
| Store role matcher 분리 | `FDebugOverlaySnapshotStore.cpp` | subject/event role match 정책이 Store helper에 있다. 별도 matcher로 분리하면 명확하지만 schema/정책 변경과 함께 커질 수 있다. |
| `FDebugOverlayRecentCombatPair` 위치 | `FDebugOverlaySnapshotStore.h` 또는 `FDebugOverlaySnapshotTypes.h` | public query type이라 header 노출이 필요하다. type header로 옮길지 유지할지 결정이 필요하다. |
| `CPlayerController.h` Exec API 위치 | `CPlayerController.h` | field-first 스타일과 console command 가시성 사이의 선택이다. 이번 cleanup에서는 임의 이동하지 않는다. |
| debug log category 교체 | `CPlayerController.cpp` | `LogTemp`를 debug overlay 전용 log category로 바꾸려면 category 선언/소유 위치가 필요하다. 단순 cleanup보다 정책 결정 성격이 있으므로 마감 전 LowRiskFix에서 제외한다. |
| diagnostic fallback helper 제거 | `CDebugOverlayHUD.h/.cpp` | RecentCombatTarget / WorldScanFallback helper는 Runtime Display Data Cleanup에서 제거한다. |

### 6.3 Later

| 항목 | 이유 |
| --- | --- |
| Runtime LOD actual 표시 | P1 closure 기준 보류 항목이다. cleanup이 아니라 기능 구현이다. |
| Behavior Tree active node 추적 | Current AI / Recent AI Event보다 깊은 AI evidence 기능이다. 별도 설계가 필요하다. |
| EventLog line wrapping / compact | UI polish 성격이며 현재 separate panel로 P1 가독성은 확보했다. |
| 범용 target component 전환 | 현재는 debug-only target component로 마감하고, P1 이후 별도 리팩터링으로 진행한다. |
| Store event schema 확장 | `Role`, `ReceiverName`, `SubjectHint`, actor pointer 등은 P1 cleanup 범위가 아니다. |
| UMG/Slate 전환 | Canvas HUD tooling을 유지한다. |
| FinalCandidate 촬영/패키징 | P1 기능 마감 후 별도 단계다. |

### 6.4 NoChange

| 항목 | 이유 |
| --- | --- |
| `FDebugOverlaySnapshotTypes.h` data-only 구조 | 현재 type header 역할이 명확하다. |
| `FDebugOverlaySnapshotStore.h` static utility API 구조 | field가 없는 API-only class라 UCLASS field/API 배치 기준을 그대로 적용하지 않는다. |
| `UCDebugOverlayFocusComponent` 책임 | debug-only focus actor/source/command result 저장소와 Target compatibility wrapper로 충분히 좁다. |
| debug hook 파일 분산 | 도메인별 hook에서 Store record를 호출하는 현재 구조가 의존성을 낮춘다. |
| `Pannel_01/02/03` 표시 | 현재 layout style lock 문서 기준으로 유지한다. |
| EventLog CVar 이름 | console command contract이므로 임의 변경하지 않는다. |
| HUD layout 수치 | 사용자가 수동 조정한 style lock 값이므로 기능 작업 중 임의 변경하지 않는다. |

## 7. 권장 마감 전 cleanup 범위

P1 마감 전 cleanup은 다음 하나의 목표모드로 묶는 것이 적절하다.

권장 목표:

```text
Debug Overlay P1 마감 전 LowRiskFix cleanup으로 HUD/Store helper section과 일부 helper naming만 정리하고, schema/API/layout/style/gameplay flow는 변경하지 않는다.
```

후속 cleanup을 진행한다면 포함 가능한 범위:

- `CDebugOverlayHUD.cpp` anonymous namespace section 정리
- `AppendSnapshotLines` 같은 역할 불일치 helper 이름 정리
- HUD layout/style 상수 섹션화
- `FDebugOverlaySnapshotStore.cpp` helper section 정리
- include order 재확인
- `git diff --check`
- UE `PortfolioEditor Win64 Development` 빌드 검증

제외:

- HUD 파일 분리
- Store schema/API 변경
- EventLog policy 변경
- Current AI source 이동
- Runtime LOD actual 표시
- FinalCandidate 촬영/패키징

## 8. 완료 판단

이 리뷰 문서 기준의 완료 조건:

- 현재 구조가 P1 마감 기준으로 허용 가능한지 기록했다.
- 마감 전 바로 처리할 `LowRiskFix`를 분리했다.
- 사용자 판단이 필요한 `DecisionNeeded`를 코드 수정과 분리했다.
- P1 이후로 넘길 `Later`를 명확히 했다.
- 유지해야 할 `NoChange` 항목을 고정했다.

다음 작업 후보는 이 문서의 `LowRiskFix`만 반영하는 작은 cleanup 구현이다. 실제 진행 여부와 범위는 별도 작업 시작 시 다시 확인한다.
