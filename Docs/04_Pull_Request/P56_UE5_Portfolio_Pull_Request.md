# UE5 Portfolio Pull Request

## 제목

**P56: Debug Overlay Focus Migration Package**

## 날짜

**2026.08.08**

## 상태

- [x] Focus command result 구조화
- [x] Focus source/mode 기반 표시 흐름 정리
- [x] RecentCombat 명시 focus command 경로 추가
- [x] Target compatibility wrapper 제거 및 Focus API 기준 정리
- [x] SnapshotStore 책임 분리 및 helper/API 섹션 정리
- [x] ViewDataBuilder / TextFormatter / CanvasRenderer 책임 정리
- [x] EventLog / Interaction panel을 EventLog / World Summary 기준으로 정리
- [x] PortfolioDebugOverlayEditor module 책임 분리
- [x] `git diff --check main..HEAD` 통과
- [x] `PortfolioEditor Win64 Development` build 통과
- [x] 수동 Editor plugin / PIE HUD / Focus command 검수 통과

## 브랜치

- Base: `main`
- Branch: `feature/debug-overlay-focus-migration-package`
- HEAD: `ab7f55af chore(debug): fix overlay focus resolver whitespace`

## 요약

이번 PR은 Debug Overlay의 focus migration과 runtime/editor 리팩터링을 하나의 패키지로 마감한다.

핵심 방향은 기존 Target 중심 문자열/헬퍼 흐름을 Focus 기준 구조로 정리하고, HUD 표시 데이터 생성, 텍스트 포맷, Canvas 렌더링, SnapshotStore 기록/조회, Editor Tooling 조작 책임을 명확히 분리하는 것이다.

기능 확장성 작업은 포함하지 않는다. Blueprint/UMG adapter, Runtime LOD actual, BT active node tracking, Store schema/API 변경은 후속 작업으로 남긴다.

## 변경 배경

P52/P53 이후 Debug Overlay는 runtime HUD, SnapshotStore, Editor Tooling이 동작 가능한 상태였지만 다음 문제가 남아 있었다.

- HUD / ViewDataBuilder / TextFormatter / CanvasRenderer의 표시 책임 경계가 흐렸다.
- TargetComponent 명명과 Focus 개념이 섞여 있었다.
- nearest/editor-selection/recent-combat focus command 흐름이 Controller, component, resolver에 분산되어 있었다.
- SnapshotStore 단일 cpp에 record, filter, ring access, lifecycle 책임이 몰려 있었다.
- Editor Tooling module cpp가 CVar access, Slate UI, PIE command bridge, module lifecycle을 모두 들고 있었다.

이번 PR은 기능 정책을 바꾸기보다 위 책임 경계를 코드 구조로 고정하는 데 초점을 둔다.

## 주요 변경

### 1. Focus command / Focus component 정리

대상:

```text
Source/Portfolio/Core/Debug/CDebugOverlayFocusComponent.*
Source/Portfolio/Core/Debug/FDebugOverlayFocusTypes.h
Source/Portfolio/Core/Debug/FDebugOverlayFocusResolver.*
Source/Portfolio/Core/Debug/FDebugOverlayFocusRuntimeHelper.*
Source/Portfolio/Core/Debug/FDebugOverlayFocusLogHelper.*
Source/Portfolio/Controller/CPlayerController.*
```

변경 내용:

- `FDebugOverlayFocusCommandResult` 기반으로 마지막 focus command 결과를 구조화했다.
- focus source / command result / actor name / distance / radius 표시 정보를 분리했다.
- `DebugOverlaySelectRecentCombatTarget` 명시 command 경로를 추가했다.
- Controller는 exec command entry와 resolver 호출, component 결과 적용만 담당하도록 정리했다.
- FocusResolver는 nearest, outliner actor name, recent combat 기반 focus resolve를 담당한다.
- FocusComponent는 focus 상태와 마지막 command result 저장소 역할로 정리했다.
- 기존 Target compatibility wrapper와 alias는 이번 패키지에서 제거했다.

유지한 것:

- console command 문자열 compatibility는 유지했다.
- 자동 RecentCombat fallback은 추가하지 않았다.
- gameplay lock-on system 구현은 하지 않았다.

### 2. HUD ViewData / TextFormatter / CanvasRenderer 정리

대상:

```text
Source/Portfolio/Core/Debug/CDebugOverlayHUD.*
Source/Portfolio/Core/Debug/FDebugOverlayViewDataTypes.h
Source/Portfolio/Core/Debug/FDebugOverlayViewDataBuilder.*
Source/Portfolio/Core/Debug/FDebugOverlayTextFormatter.cpp
Source/Portfolio/Core/Debug/FDebugOverlayTextPanelTypes.h
Source/Portfolio/Core/Debug/FDebugOverlayCanvasRenderer.cpp
```

변경 내용:

- HUD는 ViewData build와 text panel rendering 조립 중심으로 정리했다.
- ViewDataBuilder는 actor status, recent execution, AI, focus, EventLog, World Summary 데이터를 생성한다.
- TextFormatter는 ViewData를 표시 문자열 panel로 변환한다.
- CanvasRenderer는 formatted text panel을 받아 Canvas에 그리는 책임만 갖도록 정리했다.
- 기존 Interaction panel 명칭을 World Summary 표시 구조로 정리했다.
- left actor panel, right world summary panel, center event log panel의 책임을 분리했다.

표시 정책:

- `[Debug Overlay Pannel_01/02/03]` 문자열은 유지한다.
- `EnemyFocusMode`, `EnemyFocusActor`, `EnemyFocusCommand` 표시를 기준으로 정리했다.
- EventLog filter/limit 의미는 변경하지 않았다.
- Canvas text overflow / ellipsis는 이번 PR에서 구현하지 않았다.

### 3. SnapshotStore 책임 분리

대상:

```text
Source/Portfolio/Core/Debug/FDebugOverlaySnapshotStore.*
Source/Portfolio/Core/Debug/FDebugOverlaySnapshotStoreInternals.h
Source/Portfolio/Core/Debug/FDebugOverlaySnapshotStoreLifecycle.cpp
Source/Portfolio/Core/Debug/FDebugOverlaySnapshotStoreRecordBuilder.cpp
Source/Portfolio/Core/Debug/FDebugOverlaySnapshotStoreFilterPolicy.cpp
Source/Portfolio/Core/Debug/FDebugOverlaySnapshotStoreRingAccess.cpp
Source/Portfolio/Core/Debug/FDebugOverlaySnapshotTypes.h
```

변경 내용:

- `FDebugOverlaySnapshotStore.cpp`를 public orchestration API 중심으로 축소했다.
- lifecycle, record builder, filter policy, event ring access를 별도 cpp로 분리했다.
- SnapshotStore internals 선언과 구현 파일의 API 순서를 맞췄다.
- helper 이름을 record formatting 의미에 맞게 정리했다.
- per-pawn recent AI snapshot query를 추가해 HUD AI 표시 흐름과 연결했다.

유지한 것:

- Store public API/schema 의미 변경은 하지 않았다.
- EventLog filter/noise/collision 의미를 유지했다.
- `RecentCombatPair`는 Store가 focus를 선택하지 않고, FocusResolver가 필요한 경우 조회하는 구조를 유지했다.

### 4. PortfolioDebugOverlayEditor module 책임 분리

대상:

```text
Plugins/PortfolioDebugOverlayEditor/Source/PortfolioDebugOverlayEditor/Private/PortfolioDebugOverlayEditorModule.cpp
Plugins/PortfolioDebugOverlayEditor/Source/PortfolioDebugOverlayEditor/Public/PortfolioDebugOverlayEditorModule.h
Plugins/PortfolioDebugOverlayEditor/Source/PortfolioDebugOverlayEditor/Public/SPortfolioDebugOverlayEditorWidget.h
Plugins/PortfolioDebugOverlayEditor/Source/PortfolioDebugOverlayEditor/Private/SPortfolioDebugOverlayEditorWidget.cpp
Plugins/PortfolioDebugOverlayEditor/Source/PortfolioDebugOverlayEditor/Private/FPortfolioDebugOverlayEditorCVarAccess.*
Plugins/PortfolioDebugOverlayEditor/Source/PortfolioDebugOverlayEditor/Private/FPortfolioDebugOverlayEditorFocusCommandBridge.*
```

변경 내용:

- `PortfolioDebugOverlayEditorModule.cpp`는 module lifecycle, menu registration, tab spawning만 담당한다.
- `SPortfolioDebugOverlayEditorWidget.*`는 Slate UI 구성을 담당한다.
- `FPortfolioDebugOverlayEditorCVarAccess.*`는 Debug Overlay CVar name/get/set/availability를 담당한다.
- `FPortfolioDebugOverlayEditorFocusCommandBridge.*`는 PIE world 탐색, outliner actor 선택, focus command 실행을 담당한다.
- Widget은 raw console command string을 직접 조합하지 않고 bridge의 semantic API를 호출한다.

유지한 것:

- Editor plugin의 CVar 이름, command 이름, 버튼 label/status 문구는 유지했다.
- `Build.cs`와 `.uplugin`은 변경하지 않았다.

## 변경 파일 범위

주요 변경 범위:

```text
Docs/04_Pull_Request/
Docs/06_notes/
Plugins/PortfolioDebugOverlayEditor/Source/PortfolioDebugOverlayEditor/
Source/Portfolio/Controller/CPlayerController.*
Source/Portfolio/Core/Debug/
```

변경하지 않은 범위:

```text
Portfolio.uproject
Source/Portfolio/Portfolio.Build.cs
Plugins/PortfolioDebugOverlayEditor/PortfolioDebugOverlayEditor.uplugin
Config/
Content/
*.umap
*.uasset
```

## 검증

### Static check

```text
git diff --check main..HEAD
Result: Pass
```

### Build

```text
PortfolioEditor Win64 Development
Result: Pass
```

실행 명령:

```text
"C:/Program Files/Epic Games/UE_5.4/Engine/Build/BatchFiles/Build.bat" PortfolioEditor Win64 Development -Project="C:/UE5_Portfolio/Portfolio_UE5.4_verGit/Portfolio/Portfolio.uproject" -WaitMutex -FromMsBuild
```

### 수동 검수

사용자 수동 검수 완료:

```text
- Debug Overlay editor tab 열림
- Enabled / Collect checkbox 동작 유지
- EventLog Filter combo 동작 유지
- EventLog Limit spinbox 동작 유지
- Hide Noise / Hide Collision checkbox 동작 유지
- Nearest Target Radius spinbox 동작 유지
- Select Nearest Focus 버튼 동작 유지
- Select Outliner Focus 버튼 동작 유지
- Select Recent Combat Focus 버튼 동작 유지
- Clear Focus 버튼 동작 유지
- Last Command status text 표시 유지
- PIE HUD / focus 표시 회귀 없음
```

## Scope Guard

이번 PR에서 하지 않은 것:

- `Build.cs` 변경 없음
- `.uplugin` 변경 없음
- config 변경 없음
- `.umap` / `.uasset` 변경 없음
- `Portfolio.uproject` 변경 없음
- Blueprint/UMG adapter 구현 없음
- Blueprint/UMG override 구현 없음
- Canvas text overflow / ellipsis 구현 없음
- Runtime LOD actual 구현 없음
- BT active node tracking 구현 없음
- gameplay lock-on system 구현 없음
- Shipping HUD claim 없음
- Store schema/API behavior 변경 없음

## 리스크 / 확인 포인트

- 표시 문자열 일부는 evidence 기준에 영향을 줄 수 있으므로 HUD 수동 검수 결과를 PR에 명시한다.
- FocusComponent class/file rename과 Target wrapper 제거가 포함되어 있으므로 editor load, BP compile, PIE 진입 검수 결과를 확인한다.
- Editor Tooling 파일 분리는 plugin module 내부에서만 수행되었고, runtime module dependency를 오염시키지 않았는지 확인한다.

## 후속 작업 후보

이번 PR 이후 별도 브랜치에서 다룰 작업:

1. Canvas text overflow / ellipsis
2. Blueprint/UMG adapter 설계
3. Blueprint/UMG override 구현
4. Runtime LOD actual 표시
5. BT active node tracking
6. Store schema/API 변경 필요성 판단

## 관련 문서

- `Docs/06_notes/N28_DebugOverlay_SnapshotStore_Work_Guideline_Note.md`
- `Docs/07_Portfolio_Documents/Debug_Overlay/01_Planning/Debug_Overlay_Code_Cleanup_Investigation_KR.md`
- `Docs/07_Portfolio_Documents/Debug_Overlay/01_Planning/Debug_Overlay_Focus_Resolver_Terminology_Design_KR.md`
- `Docs/07_Portfolio_Documents/Debug_Overlay/01_Planning/Debug_Overlay_FocusComponent_Rename_Preparation_KR.md`
- `Docs/07_Portfolio_Documents/Debug_Overlay/05_Verification/Debug_Overlay_P1_Code_Clean_Structure_Review_KR.md`
