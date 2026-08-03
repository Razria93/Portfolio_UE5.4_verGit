# UE5 Portfolio Pull Request

## 제목

**P55: Debug Overlay Focus Component Migration Closure**

## 날짜

**2026.08.04**

## 상태

- [x] `UCDebugOverlayTargetComponent` -> `UCDebugOverlayFocusComponent` 이주 완료
- [x] `CDebugOverlayTargetComponent.*` -> `CDebugOverlayFocusComponent.*` rename 반영
- [x] Controller / HUD / FocusResolver 호출 경로 Focus 기준 정리
- [x] `EDebugOverlayFocusSource` 도입 및 기존 enum alias 호환 유지
- [x] Target compatibility wrapper API 유지 정책 명시
- [x] subobject name `TEXT("DebugOverlayTarget")` 유지 정책 명시
- [x] Editor Tooling UI terminology Target -> Focus 정리
- [x] 구조 검토/검증 문서에 브랜치 경계(현재/다음) 고정 메모 반영
- [x] `git diff --check` 통과
- [x] `PortfolioEditor Win64 Development` build 통과

## 브랜치

- Base: `main`
- Branch: `main`
- Draft PR: 생성 예정

## 대표 스크린샷

이번 PR은 구조/용어/문서 마감 정리 중심 작업이다. 신규 screenshot evidence는 추가하지 않는다.

## 요약

이번 PR 후보는 Debug Overlay Target terminology를 Focus terminology로 이주한 작업을 마감하고, 호환성 경계를 명시적으로 고정한다.

핵심은 기능 확장이 아니라 rename 정리와 문서 경계 고정이다. 외부 command surface와 호환 계층은 유지하고, 제거/마이그레이션 성격 작업은 다음 브랜치로 이관한다.

## 변경 배경

직전 구조 정리에서 Debug Overlay 내부 역할은 이미 Focus 기준으로 수렴했지만, Editor Tooling UI 용어와 문서 경계가 혼재되어 있었다.

이번 작업에서는 다음 원칙을 고정한다.

- 현재 브랜치: terminology/docs closure
- 다음 브랜치: 기능 확장 및 호환성 제거 패키지

## 주요 변경

### 1. Focus component migration 반영

적용 내용:

- FocusComponent 기반 상태 저장 책임 정리
- FocusResolver 결과를 Controller가 적용하는 경계 유지
- HUD는 FocusComponent 조회 기반 표시 경로 유지

관련 커밋:

```text
c799607c refactor(debug): migrate overlay target component to focus component
```

### 2. Editor Tooling terminology cleanup

대상:

```text
Plugins/PortfolioDebugOverlayEditor/Source/PortfolioDebugOverlayEditor/Private/PortfolioDebugOverlayEditorModule.cpp
```

정리 내용:

- UI section label/help를 Focus 기준으로 정리
- button label `Select Nearest Focus`, `Clear Focus` 반영
- 내부 상태명 `LastFocusCommandStatus`로 정리

유지 항목:

- `DebugOverlaySelectNearestTarget`
- `DebugOverlaySelectActorTarget`
- `DebugOverlayClearTarget`
- `ExecuteDebugOverlayTargetCommand` helper 역할

관련 커밋:

```text
efa086a4 refactor(debug): align overlay editor focus terminology
```

### 3. 문서 경계 고정(현재 브랜치 / 다음 브랜치)

대상 문서:

```text
Docs/07_Portfolio_Documents/Debug_Overlay/01_Planning/Debug_Overlay_Focus_Resolver_Terminology_Design_KR.md
Docs/07_Portfolio_Documents/Debug_Overlay/01_Planning/Debug_Overlay_FocusComponent_Rename_Preparation_KR.md
Docs/07_Portfolio_Documents/Debug_Overlay/05_Verification/Debug_Overlay_P1_Code_Clean_Structure_Review_KR.md
```

추가 내용:

- 현재 브랜치에서 마무리할 것
- 다음 브랜치에서 구현할 것
- 이번 브랜치에서 하지 않을 것
- 다음 브랜치 인계 기준

관련 커밋:

```text
59e84407 docs(debug): close overlay focus component migration
```

## 변경 파일 범위

핵심 변경 파일:

```text
Docs/04_Pull_Request/P55_UE5_Portfolio_Pull_Request.md
Docs/04_Pull_Request/00_Pull_Request_Index.md
Docs/07_Portfolio_Documents/Debug_Overlay/01_Planning/Debug_Overlay_Focus_Resolver_Terminology_Design_KR.md
Docs/07_Portfolio_Documents/Debug_Overlay/01_Planning/Debug_Overlay_FocusComponent_Rename_Preparation_KR.md
Docs/07_Portfolio_Documents/Debug_Overlay/05_Verification/Debug_Overlay_P1_Code_Clean_Structure_Review_KR.md
Plugins/PortfolioDebugOverlayEditor/Source/PortfolioDebugOverlayEditor/Private/PortfolioDebugOverlayEditorModule.cpp
Source/Portfolio/Controller/CPlayerController.*
Source/Portfolio/Core/Debug/CDebugOverlayFocusComponent.*
Source/Portfolio/Core/Debug/FDebugOverlayFocusResolver.*
Source/Portfolio/Core/Debug/CDebugOverlayHUD.*
```

변경하지 않은 항목:

```text
Portfolio.uproject
Source/Portfolio/Portfolio.Build.cs
Content/
Config/ (Core Redirect 추가 없음)
*.umap
*.uasset
```

## 검증

### Build

```text
PortfolioEditor Win64 Development
Result: Pass
```

### Static check

```text
git diff --check
Result: Pass
```

### Scope check

- public console command 문자열 유지
- Target wrapper API 유지
- `EDebugOverlayTargetSource` alias 유지
- `TEXT("DebugOverlayTarget")` subobject name 유지
- Store schema/API 변경 없음
- Core Redirect 선제 추가 없음

## 제외 / 보류 항목

이번 PR에서 하지 않는다:

- Canvas text overflow / ellipsis
- Runtime LOD actual
- BT active node tracking
- Blueprint/UMG adapter
- Blueprint/UMG override
- Target compatibility wrapper 실제 제거
- enum alias 실제 제거
- subobject name migration
- Core Redirect 선제 추가

## 후속 작업 후보

다음 브랜치에서 우선 검토:

1. Focus command result 구조체화
2. Focus mode enum 확장
3. RecentCombat 명시 command 경로
4. wrapper/alias/subobject name 제거 패키지 검토

## 관련 문서

- `Docs/07_Portfolio_Documents/Debug_Overlay/01_Planning/Debug_Overlay_Focus_Resolver_Terminology_Design_KR.md`
- `Docs/07_Portfolio_Documents/Debug_Overlay/01_Planning/Debug_Overlay_FocusComponent_Rename_Preparation_KR.md`
- `Docs/07_Portfolio_Documents/Debug_Overlay/05_Verification/Debug_Overlay_P1_Code_Clean_Structure_Review_KR.md`
- `Docs/04_Pull_Request/P53_UE5_Portfolio_Pull_Request.md`
- `Docs/04_Pull_Request/P54_UE5_Portfolio_Pull_Request.md`
