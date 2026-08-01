# UE5 Portfolio Pull Request

## 제목

**P54: Asset Reference Inspector Editor Plugin**

## 날짜

**2026.08.02**

## 상태

- [x] `AssetReferenceInspector` Editor-only project plugin 정식 편입
- [x] `창 > Portfolio Tools > Asset Reference Inspector` menu entry 정리
- [x] Asset Registry 기반 dependency / referencer 분석 UI 유지
- [x] Content Browser selected asset pick / sync 동작 유지
- [x] Unused Candidate scan 동작 유지
- [x] CSV export 동작 유지
- [x] plugin README 링크 / 표현 정리
- [x] plugin 기본 icon 추가
- [x] `PortfolioEditor Win64 Development` build 통과
- [x] `git diff --check` 통과

## 브랜치

- Base: `main`
- Branch: `feature/asset-reference-inspector-plugin`
- Draft PR: 생성 예정

## 요약

이번 PR 후보는 루트 `Plugins/AssetReferenceInspector`를 Portfolio 프로젝트의 Editor-only project plugin으로 정식 편입한다.

핵심은 runtime gameplay 기능을 추가하는 것이 아니라, Unreal Editor 안에서 asset dependency / referencer 관계를 빠르게 확인할 수 있는 편집 도구를 프로젝트에 포함하는 것이다.

Plugin 코드는 `Plugins/AssetReferenceInspector` 아래에 격리한다. Runtime `Portfolio.Build.cs`에 `UnrealEd`, `ToolMenus`, `Slate`, `AssetRegistry` 같은 Editor tooling 의존성을 추가하지 않는다.

## 변경 배경

P53에서 Debug Overlay 조작용 Editor Tooling을 정리하면서, 프로젝트 전용 Editor-only plugin을 관리하는 방향이 확정되었다.

`AssetReferenceInspector`는 Content Browser 선택 asset을 기준으로 참조 관계를 확인하고, unused 후보를 검토하며, 현재 tree 결과를 CSV로 내보내는 Editor tooling이다.

이번 작업에서는 기능을 확장하지 않고, 프로젝트에 정식 편입 가능한 형태로 최소 보정한다.

## 주요 변경

### 1. Editor-only plugin 정식 편입

추가 plugin:

```text
Plugins/AssetReferenceInspector
```

구성:

```text
AssetReferenceInspector.uplugin
README.md
Resources/Icon128.png
Source/AssetReferenceInspector/
```

정책:

- `.uplugin` module type은 `Editor`로 유지한다.
- `CanContainContent: false`로 코드 기반 Editor Tooling만 제공한다.
- `Portfolio.uproject`에는 plugin entry를 추가하지 않는다.
- Runtime module `Portfolio.Build.cs`에는 Editor dependency를 추가하지 않는다.

### 2. Window menu 위치 정리

기존 Window menu entry를 Portfolio 프로젝트 전용 도구 묶음 아래로 정리한다.

```text
창 > Portfolio Tools > Asset Reference Inspector
```

기존 Nomad tab / Slate UI / Asset Registry 분석 기능은 유지한다.

`PortfolioDebugOverlayEditor`와 같은 `Portfolio Tools` 메뉴 그룹을 공유하되, command / tab / menu id는 `AssetReferenceInspector` 기준으로 분리한다.

### 3. Asset Reference Inspector 기능 유지

Nomad panel에서 제공하는 기능:

```text
Pick Selected Asset
Analyze Dependencies
Analyze Referencers
Scan Unused Candidate
Export CSV
Content Browser Sync
```

분석 흐름:

- Content Browser selected asset을 기준으로 시작 asset을 선택한다.
- Asset Registry에서 dependency / referencer 관계를 조회한다.
- Max Depth, Path Filter, Asset Class Filter, Engine / Plugin Content 표시 옵션을 적용한다.
- 결과를 Slate Tree View에 표시한다.
- Tree node double-click 시 Content Browser Sync를 수행한다.
- 현재 Tree 결과는 CSV로 export할 수 있다.

### 4. 기본 icon 추가

Plugin Browser 식별용 기본 icon을 추가한다.

```text
Plugins/AssetReferenceInspector/Resources/Icon128.png
```

이번 단계에서는 descriptor icon 중심으로 제한하고, Slate toolbar / custom style set icon 확장은 하지 않는다.

### 5. README 정리

`Plugins/AssetReferenceInspector/README.md`를 현재 repo 기준으로 정리한다.

- Window menu 경로를 `창 > Portfolio Tools > Asset Reference Inspector` 기준으로 갱신한다.
- 존재하지 않는 문서 링크를 제거한다.
- Editor-only project plugin 기준을 명시한다.
- Marketplace-ready reusable plugin / runtime gameplay feature / packaged build feature처럼 보이는 표현을 피한다.

## 변경 파일 범위

주요 추가 / 변경:

```text
Docs/04_Pull_Request/P54_UE5_Portfolio_Pull_Request.md
Docs/04_Pull_Request/00_Pull_Request_Index.md
Plugins/AssetReferenceInspector/
```

커밋 제외:

```text
Plugins/AssetReferenceInspector/Binaries/
Plugins/AssetReferenceInspector/Intermediate/
Plugins/AssetReferenceInspector/Saved/
Saved/AssetReferenceInspector/
generated CSV
```

변경하지 않은 항목:

```text
Portfolio.uproject
Source/Portfolio/Portfolio.Build.cs
Content/
Config/
*.umap
*.uasset
Plugins/PortfolioDebugOverlayEditor/
```

## 검증

### Build

```text
PortfolioEditor Win64 Development
Result: Pass
Date: 2026.08.02
```

### Static check

```text
git diff --check
Result: Pass
```

### Scope check

- `AssetReferenceInspector` plugin은 Editor-only module로 유지된다.
- Runtime gameplay code는 변경하지 않는다.
- `Portfolio.Build.cs`, `Portfolio.uproject`, asset/config 파일은 변경하지 않는다.
- Generated binary/intermediate/saved output과 CSV export 결과는 커밋하지 않는다.

## Evidence 연결

이번 PR 문서에는 별도 screenshot evidence를 포함하지 않는다.

Editor menu / Nomad panel / CSV export 동작을 PR 본문에서 시각적으로 보강하려면 후속 작업으로 `P54 evidence screenshot capture`를 진행한다.

## 제외 / 보류 항목

- Marketplace-ready reusable plugin claim
- Runtime gameplay feature claim
- Packaged build feature claim
- Plugin preset 저장
- Plugin config 저장
- Custom Slate style set icon 확장
- Node Graph UI
- Soft Reference / Manage Dependency 전용 UI 분리
- Delete-safe unused asset 판정 claim
- Screenshot evidence 촬영 / 패키징

## PR 제출 전 확인

- [x] Editor-only plugin 경계 확인
- [x] Portfolio Tools menu grouping 확인
- [x] README broken link 정리
- [x] 기본 icon 추가
- [x] Build 통과
- [x] `git diff --check` 통과
- [x] Generated output 제외
