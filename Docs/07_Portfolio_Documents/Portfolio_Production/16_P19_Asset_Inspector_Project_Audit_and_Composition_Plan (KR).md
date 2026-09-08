# 문서상 p.22 Asset Inspector 프로젝트 감사 및 구성 계획

> 파일명은 초기 22페이지 체계의 감사 기록을 보존한다. 현행 25페이지 체계에서는 p.22에 적용한다.

## 1. 페이지 목적

`AssetReferenceInspector`는 선택 Asset의 참조 관계를 조사·기록하는 UE5.4 Editor-only project plugin이다. 이 페이지는 기능을 삭제 자동화나 runtime 분석으로 과장하지 않고, **입력 조건 → Asset Registry 관계 조회 → Tree 탐색 → Sync / CSV 기록**이라는 조사 흐름을 설명한다.

## 2. 코드·문서 감사 결과

1. 선택 Asset 기준으로 Dependencies 또는 Referencers를 Asset Registry에서 조회한다.
2. Max Depth, Path Filter, Asset Class Filter와 Engine / Plugin Content 표시 옵션을 기준으로 재귀 Tree를 구성한다.
3. Tree node에는 관계·Depth·순환 여부·에디터 프로젝트 디스크 파일 크기 추정치가 담기며, double-click은 Content Browser Sync를 수행한다.
4. `FAssetReferenceCsvExporter`는 현재 Tree 결과를 `Saved/AssetReferenceInspector`에 timestamped UTF-8 CSV로 저장한다. generated CSV는 source control 대상이 아니다.
5. `Scan Unused Candidate`는 `/Game` Asset 중 `GetReferencers() == 0` 후보를 표시할 뿐, 삭제 가능 여부를 결정하지 않는다.

## 3. 증거 및 표현 범위

| 증거 | 확인 가능한 내용 | 페이지 사용 범위 |
| --- | --- | --- |
| `asset_reference_inspector_02_nomad_panel.jpg` | Nomad Panel UI와 입력 control 존재 | `Selected Asset: None`·빈 Tree이므로 진입점 증거로만 사용. 조사 결과 실사용 화면으로는 사용하지 않음 |
| `AssetReferenceCsvExporter` 구현 | exported row의 field와 timestamped path | 가짜 CSV 화면 대신 code-derived schema |
| `에셋참조 실사례.png` | 실제 Selected Asset, Dependencies mode·depth·filter, 비어 있지 않은 Tree | p.22 중심 실사용 검증. CSV export 완료 상태는 별도 캡처가 필요하다. |

## 4. 제한 사항

- 동적 로딩, 문자열 기반 경로, Asset Manager 규칙을 완전하게 추적한다고 주장하지 않는다.
- Soft Reference와 Manage Dependency는 별도 UI 옵션으로 구분하지 않는다.
- Size는 cooked size·runtime memory·dependency-inclusive size가 아니라 에디터 프로젝트 파일의 디스크 크기 추정값이다.
- Unused Candidate는 삭제 판정이 아닌 검토 후보 목록이다.
- runtime gameplay 기능·Marketplace 범용 plugin·자동 수정 도구로 표현하지 않는다.

## 5. 페이지 구성

1. **요구조건·의도**: 선택 Asset·방향·Depth·filter를 결과와 함께 남겨 재검토 가능한 조사 단위로 만든다. Unused Candidate는 삭제 판정이 아니라 검토 후보임을 먼저 고정한다.
2. **구현 구조**: `Selected Asset + options → Asset Registry GetDependencies / GetReferencers → Recursive Tree → Sync / timestamped CSV`를 사용자 제작 다이어그램 슬롯으로 둔다. `/Game Scan → GetReferencers() = 0 → Unused Candidate`는 별도 경로로 분리한다.
3. **실사용 검증**: 실제 Selected Asset과 비어 있지 않은 Tree·CSV export 상태가 같은 Panel에서 보이는 신규 캡처 슬롯을 둔다. 기존 Panel-open 화면은 중심 증거로 사용하지 않는다.
4. **사용 범위**: 조사·기록·비판정 범위를 3칸으로 분리해, 동적 로딩/문자열 경로/Asset Manager 규칙의 완전한 추적과 cooked·runtime size 해석을 주장하지 않는다.

## 6. 독립 사전 검토 반영

- p.21의 generic Editor UI/bridge와 중복하지 않고, Asset Registry 조사라는 단일 목적에 집중한다.
- `Referencers 0 = 검토 후보, 삭제 판정 아님`을 가장 강한 경계 문구로 둔다.
- CSV는 가짜 결과 캡처가 아니라 code-derived export schema와 data lineage로 다룬다.

## 7. 검수 상태

- 코드·문서·증거 범위 감사: 완료
- HTML 구성 반영: 완료 — Tooling 공통 형식(요구조건·의도 → 구현 구조 → 실사용 검증 → 사용 범위)으로 전환
- 증거 상태: Selected Asset·Tree 실사용 캡처 및 중앙 다이어그램 반영 / CSV export 완료 상태 보완 필요
- PDF 레이아웃 검증: 보완 후 별도 수행 필요
