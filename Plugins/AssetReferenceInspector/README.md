# Asset Reference Inspector

`AssetReferenceInspector`는 Unreal Engine 5.4 Editor 전용 Project Plugin이다.

Content Browser에서 선택한 Asset을 기준으로 Asset Registry를 조회하고, Dependencies / Referencers 관계를 Slate Tree View에 표시한다. 분석 결과는 Content Browser Sync와 CSV Export로 이어진다.

이 플러그인은 현재 Portfolio 프로젝트에 편입된 Editor tooling이다. Marketplace 배포용 범용 플러그인이나 runtime gameplay 기능으로 claim하지 않는다.

## 플러그인 정보

- 유형: Editor Plugin
- 모듈: `AssetReferenceInspector`
- Loading Phase: `Default`
- Content 포함 여부: 없음
- 런타임 의존성(Runtime Dependency): 없음

## 주요 기능

- 선택 Asset 기준 Dependencies 분석
- 선택 Asset 기준 Referencers 분석
- Max Depth 기반 재귀 Tree 생성
- Path Filter
- Asset Class Filter
- Engine / Plugin Content 표시 옵션
- 순환 참조 표시
- Asset 디스크 크기 표시
- Unused Candidate 전체 스캔
- Tree 노드 더블 클릭 시 Content Browser Sync
- 현재 Tree 결과 CSV Export

## 사용법

1. Unreal Editor에서 플러그인이 포함된 프로젝트를 연다.
2. `Window > Portfolio Tools > Asset Reference Inspector`를 선택해 탭을 연다.
3. Content Browser에서 분석할 Asset을 선택한다.
4. `Pick Selected Asset`을 클릭한다.
5. Dependencies 또는 Referencers 모드를 선택한다.
6. 필요하면 Max Depth, Path Filter, Asset Class Filter, Engine / Plugin Content 표시 옵션을 조정한다.
7. `Analyze`를 클릭한다.
8. Tree View에서 참조 관계를 확인한다.

Tree 노드를 더블 클릭하면 Content Browser Sync를 수행한다.

`Scan Unused Candidate`는 선택 Asset과 무관하게 `/Game` 아래 Asset을 스캔해 `GetReferencers()` 결과가 0개인 후보를 표시한다. 이 결과는 삭제 가능 판정이 아니라 검토 후보 목록이다.

`Export CSV`는 현재 Tree 결과를 프로젝트 `Saved/AssetReferenceInspector` 아래에 저장한다.

## CSV 컬럼

```text
AssetName,PackageName,Class,Path,Depth,SizeBytes,Mode,ParentPackage,IsCircular,IsUnusedCandidate
```

CSV 저장 규칙과 escaping 기준은 현재 `Export/AssetReferenceCsvExporter` 구현을 기준으로 한다.

## 프로젝트 편입 기준

- Portfolio 프로젝트의 `Plugins/AssetReferenceInspector` 아래에 위치한다.
- Editor-only module로 동작한다.
- `Portfolio.Build.cs`에 Editor dependency를 추가하지 않는다.
- `Portfolio.uproject`에 별도 plugin entry를 추가하지 않는 것을 기본 전제로 한다.
- generated CSV는 `Saved/AssetReferenceInspector` 아래에 생성되며 소스 관리 대상이 아니다.

## 제한 사항

- 게임 runtime 기능이 아니다.
- 코드에서 동적으로 로딩하는 Asset, 문자열 기반 경로, Asset Manager 규칙을 완전하게 추적한다고 보장하지 않는다.
- Soft Reference와 Manage Dependency는 현재 별도 UI 옵션으로 분리하지 않는다.
- Asset Size는 cooked size, runtime memory size, dependency inclusive size가 아니라 에디터 프로젝트의 디스크 파일 크기 기준 추정값이다.
- Unused Candidate는 삭제 가능 판정이 아니다.
- Node Graph UI는 Future Work다.

## 소스 구조

```text
Source/AssetReferenceInspector/
  AssetReferenceInspector.Build.cs
  Private/
    AssetReferenceInspectorModule.cpp
    Analysis/
    Export/
    UI/
```

## 관련 문서

- `Docs/04_Pull_Request/P54_UE5_Portfolio_Pull_Request.md`
