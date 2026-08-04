# UE5 Portfolio Pull Request

## 제목

**P56: Debug Overlay Focus Migration Package**

## 날짜

**2026.08.04**

## 상태

- [x] Focus command result 구조체 모델 도입
- [x] 문자열 요약 중심 저장에서 구조화된 결과 저장으로 전환
- [x] Focus mode enum 확장 (`NearestEnemy`, `RecentCombat`, `WorldScanFallback`, `GameplayTarget`)
- [x] `DebugOverlaySelectRecentCombatTarget` 명시 command 경로 추가
- [x] Target compatibility wrapper API 제거
- [x] `EDebugOverlayTargetSource` alias 제거
- [x] subobject name `DebugOverlayTarget` -> `DebugOverlayFocus` 변경
- [x] Core Redirect는 증거 기반 정책으로 보류
- [x] `git diff --check` 통과
- [x] `PortfolioEditor Win64 Development` build 통과

## 브랜치

- Base: `main`
- Branch: `feature/debug-overlay-focus-migration-package`

## 요약

이번 PR은 Focus migration의 후속 패키지로, debug overlay focus command 결과 저장 모델을 문자열 중심에서 구조체 중심으로 전환한다.

동시에 explicit RecentCombat command 경로를 추가하고, 호환성 레이어(wrapper/alias)를 제거해 Focus 기준 API로 정리한다.

## 주요 변경

### 1. Focus command result 구조체화

대상:

```text
Source/Portfolio/Core/Debug/CDebugOverlayFocusComponent.*
Source/Portfolio/Controller/CPlayerController.*
```

변경:

- `FDebugOverlayFocusCommandResult` 구조체 도입
- `CommandType`, `Status`, `FocusMode`, `ActorName`, `ClassName`, `Distance`, `Radius` 저장
- FocusComponent는 last command result를 구조체로 저장
- HUD 표시 텍스트는 구조체를 내부 포맷하여 생성

### 2. Focus mode 확장 + RecentCombat command

대상:

```text
Source/Portfolio/Core/Debug/FDebugOverlayFocusResolver.*
Source/Portfolio/Controller/CPlayerController.*
Plugins/PortfolioDebugOverlayEditor/Source/PortfolioDebugOverlayEditor/Private/PortfolioDebugOverlayEditorModule.cpp
```

변경:

- `EDebugOverlayFocusSource` 확장
  - `NearestEnemy`
  - `RecentCombat`
  - `WorldScanFallback`
  - `GameplayTarget`
- `ResolveRecentCombatEnemy()` 추가
- `DebugOverlaySelectRecentCombatTarget` Exec command 추가
- Editor Tooling Focus 섹션에 `Select Recent Combat Focus` 버튼 추가

### 3. 호환성 제거 패키지

대상:

```text
Source/Portfolio/Core/Debug/CDebugOverlayFocusComponent.*
Source/Portfolio/Controller/CPlayerController.cpp
```

변경:

- compatibility wrapper API 제거
  - `HasDebugOverlayTarget`
  - `GetDebugOverlayTargetActor`
  - `GetDebugOverlayTargetSummary`
  - `GetDebugOverlayTargetSource`
  - `SetDebugOverlayTarget`
  - `ClearDebugOverlayTarget`
  - selection summary wrapper API
- `EDebugOverlayTargetSource` alias 제거
- subobject name 변경
  - `TEXT("DebugOverlayTarget")` -> `TEXT("DebugOverlayFocus")`

## Core Redirect 결정

이번 작업에서는 Core Redirect를 추가하지 않았다.

판단 근거:

- 코드/설정 레벨에서 즉시 확인 가능한 missing class/reference 증거가 없었다.
- 정책대로 증거 확인 전 선제 Redirect 추가는 보류한다.

추가 검증이 필요하면 다음 단계를 수행한다.

```text
1) Editor 로드 시 warning/error 로그 확인
2) 관련 Blueprint compile/load 확인
3) 실제 missing class/reference 발생 시 최소 Redirect만 추가
```

## 검증

수행:

```text
git diff --check
"C:/Program Files/Epic Games/UE_5.4/Engine/Build/BatchFiles/Build.bat" PortfolioEditor Win64 Development -Project="C:/UE5_Portfolio/Portfolio_UE5.4_verGit/Portfolio/Portfolio.uproject" -WaitMutex -FromMsBuild
```

결과:

```text
git diff --check 통과
PortfolioEditor Win64 Development build 통과
```

## 제외 범위

이번 PR에서 의도적으로 제외:

- Runtime LOD actual
- BT active node tracking
- Blueprint/UMG adapter/override
- Store schema/API 실변경
- lock-on 시스템 성격 구현
