# UE5 Portfolio Pull Request

## 제목

**P57: Debug Overlay Focus Naming Finalization**

## 날짜

**2026.08.09**

## 상태

- [x] Debug Overlay focus 선택/표시/에디터 조작 경로의 Target 계열 stale naming 정리
- [x] `DebugOverlaySelectNearestFocus` / `DebugOverlaySelectOutlinerFocus` / `DebugOverlaySelectRecentCombatFocus` / `DebugOverlayClearFocus` 기준 command surface 고정
- [x] legacy Target command wrapper 제거
- [x] `DebugOverlayFocus` subobject name 반영
- [x] `Pannel_01/02/03` -> `Panel_01/02/03` 표시 문자열 정리
- [x] `FocusTarget:` -> `FocusActor:` 표시 정리
- [x] Editor Tooling CVar / button / status text Focus 기준 정리
- [x] Editor CVar lookup cache 적용으로 `FindConsoleObject()` 반복 경고 대응
- [x] Debug Overlay 관련 문서와 P56 PR 문서의 현재 기대 명칭 갱신
- [x] `git diff --check` 통과
- [x] `PortfolioEditor Win64 Development` build 통과
- [x] 수동 PIE HUD / Focus command / Editor panel 검수 통과

## 브랜치

- Base: `main`
- Branch: `feature/debug-overlay-focus-naming-finalization`
- HEAD: `3bee4521 docs(debug): preserve overlay focus migration history`

## 대상 스크린샷

이번 PR은 Debug Overlay focus naming / 문서 / Editor CVar access 정리 작업이다.
새 screenshot evidence는 추가하지 않고, 사용자가 PIE에서 HUD와 Editor panel을 수동 검수했다.

확인된 화면 기준:

```text
[Debug Overlay Panel_01]
[Debug Overlay Panel_02]
[Debug Overlay Panel_03]
RuntimeFocusSource: FocusComponent.OutlinerFocus
FocusActor: BP_CEnemy_C_1
```

AI Blackboard 표시 예외는 기존 문구를 유지한다.

```text
Target: BP_CPlayer_C_0
DistanceToTarget: ...
NoTarget
```

## 요약

이번 PR은 P56 이후 남아 있던 Debug Overlay focus 경로의 stale `Target` / `Pannel` 명칭을 현재 구조에 맞춰 최종 정리한다.

핵심 방향은 Debug Overlay가 gameplay combat target system이 아니라 “현재 HUD가 집중해서 보여주는 focus actor”를 다룬다는 점을 코드, Editor Tooling, HUD 표시, 문서에 일관되게 반영하는 것이다.

단, AI Blackboard의 `Target` / `DistanceToTarget` / `NoTarget`과 combat/snapshot schema의 `TargetActor` / `TargetName`은 의미가 다른 영역이므로 변경하지 않는다.

## 변경 배경

P56에서 FocusComponent / FocusResolver / ViewData / TextFormatter / CanvasRenderer 구조가 정리되었지만, 리뷰와 수동 검수 과정에서 다음 stale naming이 남아 있었다.

- console command에 `Target` 계열 이름이 남아 있음
- HUD 표시 문자열에 `FocusTarget` 또는 `Pannel` 오탈자가 남아 있음
- Editor Tooling CVar와 UI label에 `NearestTargetRadius` 계열 이름이 남아 있음
- 관련 Debug Overlay 문서 파일명과 본문이 TargetComponent / Target Selection 기준으로 남아 있음
- Editor panel Slate lambda가 CVar를 매 프레임 이름 lookup하면서 `FindConsoleObject()` 성능 경고를 발생시킴

이번 PR은 위 항목을 한 브랜치에서 닫는다.

## 주요 변경

### 1. Runtime focus command surface 정리

대상:

```text
Source/Portfolio/Controller/CPlayerController.*
Source/Portfolio/Core/Debug/FDebugOverlayFocusRuntimeHelper.*
Source/Portfolio/Core/Debug/FDebugOverlayFocusResolver.*
Source/Portfolio/Core/Debug/FDebugOverlayFocusLogHelper.*
Source/Portfolio/Core/Debug/FDebugOverlayFocusTypes.h
Source/Portfolio/Core/Debug/CDebugOverlayFocusComponent.cpp
```

변경 내용:

- `DebugOverlaySelectNearestTarget` -> `DebugOverlaySelectNearestFocus`
- `DebugOverlaySelectOutlinerTarget` -> `DebugOverlaySelectOutlinerFocus`
- `DebugOverlaySelectRecentCombatTarget` -> `DebugOverlaySelectRecentCombatFocus`
- `DebugOverlayClearTarget` -> `DebugOverlayClearFocus`
- `DebugOverlaySelectActorTarget` wrapper 제거
- `NearestTarget` / `OutlinerTarget` / `RecentCombatTarget` / `GameplayTarget` source naming을 Focus 기준으로 정리
- log label을 `Target:`에서 `Focus:`로 정리
- `InvalidTargetComponent`, `TargetIsNotEnemy`, `NoTargetFound` 같은 focus command result naming을 Focus 기준으로 정리

유지한 것:

- `EDebugOverlayFocusResolveOutcome::NoTarget`은 focus resolve 내부의 “대상 없음” outcome 의미로 유지
- RecentCombatPair의 `SourceActor` / `TargetActor` schema 유지
- combat/snapshot `SourceName` / `TargetName` schema 유지

### 2. HUD / TextFormatter 표시 문자열 정리

대상:

```text
Source/Portfolio/Core/Debug/CDebugOverlayHUD.cpp
Source/Portfolio/Core/Debug/FDebugOverlayTextFormatter.cpp
Source/Portfolio/Core/Debug/FDebugOverlayViewDataBuilder.cpp
```

변경 내용:

- `[Debug Overlay Pannel_01/02/03]` -> `[Debug Overlay Panel_01/02/03]`
- `FocusTarget:` -> `FocusActor:`
- HUD 내부 helper/local naming을 Focus 기준으로 정리
- TextFormatter의 panel role mapping도 `Panel_` prefix 기준으로 정리

유지한 것:

- AI Current AI / Recent AI Event의 `Target:`, `DistanceToTarget:`, `NoTarget` 표시 유지
- EventLog / World Summary / Actor panel 표시 정책 유지
- Store schema/API 변경 없음

### 3. Editor Tooling Focus naming 정리

대상:

```text
Plugins/PortfolioDebugOverlayEditor/Source/PortfolioDebugOverlayEditor/Private/FPortfolioDebugOverlayEditorCVarAccess.*
Plugins/PortfolioDebugOverlayEditor/Source/PortfolioDebugOverlayEditor/Private/FPortfolioDebugOverlayEditorFocusCommandBridge.cpp
Plugins/PortfolioDebugOverlayEditor/Source/PortfolioDebugOverlayEditor/Private/SPortfolioDebugOverlayEditorWidget.cpp
Plugins/PortfolioDebugOverlayEditor/Source/PortfolioDebugOverlayEditor/Public/SPortfolioDebugOverlayEditorWidget.h
```

변경 내용:

- `Portfolio.DebugOverlay.NearestTargetRadius` -> `Portfolio.DebugOverlay.NearestFocusRadius`
- `Nearest Target Radius` -> `Nearest Focus Radius`
- Editor command bridge가 새 Focus command만 실행하도록 정리
- Last Command status text를 Focus command 기준으로 정리
- Editor CVar access에서 `IConsoleVariable*` cache를 적용해 반복 `FindConsoleObject()` 경고를 줄임

유지한 것:

- Editor plugin module / Build.cs / uplugin 변경 없음
- Button workflow 유지
- PIE world 탐색과 Outliner actor 선택 정책 유지

### 4. 문서와 PR 문서 현재 기대값 갱신

대상:

```text
Docs/04_Pull_Request/P56_UE5_Portfolio_Pull_Request.md
Docs/07_Portfolio_Documents/Debug_Overlay/**
```

변경 내용:

- 현재 기대 command를 Focus 기준으로 갱신
- `EnemySource` / `EnemyTarget` / `EnemySelect` 계열 문서를 `EnemyFocusMode` / `EnemyFocusActor` / `EnemyFocusCommand` 기준으로 정리
- `Pannel` 문서 표기를 `Panel`로 정리
- FocusComponent / Focus Selection / NearestFocus Diagnostic 문서 파일명과 링크 정리
- migration record 문서에서는 pre-migration old identifier로 일부 Target 명칭을 의도적으로 보존

유지한 것:

- 과거 evidence 파일명 자체의 `target` 명칭은 asset filename 성격이므로 보존
- AI Blackboard, combat target, snapshot target schema 문맥은 보존

## 변경 파일 범위

주요 변경 범위:

```text
Source/Portfolio/Controller/CPlayerController.*
Source/Portfolio/Core/Debug/CDebugOverlayFocusComponent.cpp
Source/Portfolio/Core/Debug/CDebugOverlayHUD.cpp
Source/Portfolio/Core/Debug/FDebugOverlayFocus*.h/.cpp
Source/Portfolio/Core/Debug/FDebugOverlayTextFormatter.cpp
Source/Portfolio/Core/Debug/FDebugOverlayViewDataBuilder.cpp
Plugins/PortfolioDebugOverlayEditor/Source/PortfolioDebugOverlayEditor/
Docs/04_Pull_Request/P56_UE5_Portfolio_Pull_Request.md
Docs/07_Portfolio_Documents/Debug_Overlay/
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
FDebugOverlaySnapshotStore public schema/API
FDebugOverlaySnapshotTypes combat/snapshot target fields
CombatSignalTargetComponent
AI Blackboard Target/DistanceToTarget/NoTarget naming
```

## 검증

### Static check

```text
git diff --check
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

### 수동 PIE 검수

사용자 수동 검수 완료:

```text
- Editor loading: OK
- BP compile: OK
- PIE entry: OK
- DebugOverlaySelectNearestFocus: OK
- DebugOverlaySelectOutlinerFocus BP_CEnemy_C_1: OK
- DebugOverlaySelectRecentCombatFocus: OK
- DebugOverlayClearFocus: OK
- Legacy Target command 제거 확인: OK
- HUD Panel_01/02/03 표시 확인: OK
- RuntimeFocusSource / FocusActor 표시 확인: OK
- AI Target / DistanceToTarget 표시 유지 확인: OK
- Editor Debug Overlay panel controls: OK
- CVar FindConsoleObject 반복 경고 대응 확인: OK
```

확인된 Output Log 예시:

```text
DebugOverlaySelectNearestFocus Result: Selected | Focus: BP_CEnemy_C_1 | Distance: 105 | Radius: 3000
DebugOverlaySelectRecentCombatFocus Result: Selected | Focus: BP_CEnemy_C_1
DebugOverlaySelectOutlinerFocus Result: Selected | Focus: BP_CEnemy_C_1
```

### Stale naming gate

다음 Debug Overlay focus-path stale token은 runtime/editor current 경로에서 발견되지 않음을 확인했다.
Migration record 문서에는 pre-migration old identifier로 일부 Target 명칭을 의도적으로 보존한다.

```text
DebugOverlaySelectNearestTarget
DebugOverlaySelectOutlinerTarget
DebugOverlaySelectActorTarget
DebugOverlaySelectRecentCombatTarget
DebugOverlayClearTarget
DebugOverlayTarget
NearestTarget
OutlinerTarget
RecentCombatTarget
GameplayTarget
TargetComponentLive
NoTargetFound
TargetIsNotEnemy
InvalidTargetComponent
LogInvalidTargetComponent
FocusTarget
Pannel_
```

## 제외 / 보류 항목

이번 PR에서 하지 않는 것:

- Blueprint/UMG adapter 설계 또는 구현
- Canvas text overflow / ellipsis
- Runtime LOD actual 표시
- BT active node tracking
- Store schema/API 변경
- combat/snapshot target field rename
- AI Blackboard target field/display rename
- gameplay lock-on / targeting system 구현
- Core Redirect 추가
- `.umap`, `.uasset`, config, Build.cs, uproject, uplugin 변경

## 후속 작업 후보

다음 작업 후보:

1. Debug Overlay PR 리뷰 대응
2. 필요 시 Focus command 문서의 과거 evidence 표현 정밀 정리
3. Blueprint/UMG adapter 설계는 별도 feature 브랜치에서 진행
4. Canvas text overflow / ellipsis는 표시 정책 변경 PR로 분리

## 관련 문서

- `Docs/07_Portfolio_Documents/Debug_Overlay/README.md`
- `Docs/07_Portfolio_Documents/Debug_Overlay/01_Planning/Debug_Overlay_Focus_Resolver_Terminology_Design_KR.md`
- `Docs/07_Portfolio_Documents/Debug_Overlay/01_Planning/Debug_Overlay_FocusComponent_Rename_Preparation_KR.md`
- `Docs/07_Portfolio_Documents/Debug_Overlay/01_Planning/Debug_Overlay_P1_Focus_Selection_Decision_KR.md`
- `Docs/07_Portfolio_Documents/Debug_Overlay/05_Verification/Debug_Overlay_P1_FocusComponent_PIE_Checklist_KR.md`
- `Docs/04_Pull_Request/P56_UE5_Portfolio_Pull_Request.md`
