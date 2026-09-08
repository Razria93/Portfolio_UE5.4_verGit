# 현행 p.25 Change Traceability 감사 및 구성 계획

> 이 문서는 현행 25페이지 체계의 p.25 기준이다. 과거 기록의 클래스명과 현재 코드를 같은 구현으로 단정하지 않고, 변경 계약과 증거 상태를 분리해 관리한다.

## 코드·문서 대조

| 근거 | 날짜·역할 | 확인된 사실 | 표현 제한 |
| --- | --- | --- | --- |
| `Docs/02_Bug_Report/B05_UE5_Portfolio_Bug_Report.md` | 2026-04-06 · 문제 기록 | 과거 `CAttachment::CollisionEnabled`에서 collision enable이 `CurrentHitWindowId` 준비보다 앞서면 첫 overlap이 `INDEX_NONE(-1)`로 reject될 수 있음을 기록한다. | 내부 Bug Record이며 GitHub Issue로 표현하지 않는다. |
| `Docs/99_Legacy/Issue_CheckList/D13_UE5_Portfolio_Issue_Checklist.md` | 2026-04-01 · Pipeline 기준 | Damage Pipeline의 선행 점검 문서다. B05보다 앞선 범용 기준이다. | B05의 후속 문서나 직접 원인으로 연결하지 않는다. |
| `Docs/04_Pull_Request/P12_UE5_Portfolio_Pull_Request.md` | 2026-04-06 · 내부 변경 기록 | B05·D13을 관련 문서로 참조하며, 활성화 대상 수집 뒤 Hit Window를 준비하고 collision을 켜는 순서를 기록한다. | 외부 GitHub Pull Request로 표현하지 않는다. |
| commit `746ea548` | 2026-04-06 · 과거 patch | `CAttachment.cpp`의 first attachment overlap에 대한 historical patch다. | 과거 patch는 현재 런타임 결과의 증거가 아니다. |
| `Source/Portfolio/Weapon/CWeaponActor.cpp` `ACWeaponActor::CollisionEnabled()` | 현재 코드 · 순서 계약 | collision 후보 수집 → `CurrentHitWindowId`·열림 상태 준비 및 notify → `SetCollisionEnabled` 순서를 확인했다. | `CAttachment`와 클래스 동일성·직접 rename을 주장하지 않는다. |

## 코드·문서 차이와 처리

1. 기존 초안의 `B05 → D13 → P12` 직선 흐름은 사실과 다르다. D13은 B05보다 이른 2026-04-01의 선행 기준이고, B05·P12는 2026-04-06 기록이다.
2. B05 및 commit은 과거 `CAttachment`를 가리키지만 현재 구현은 `ACWeaponActor`다. 따라서 페이지는 클래스 계보가 아니라 **collision 활성화 전에 Hit Window를 준비하는 순서 계약**의 대조로 작성한다.
3. B05·P12에는 historical validation 기록이 있으나, 현행 빌드의 제출용 first-overlap runtime capture는 없다. 페이지는 현재 성공 결과가 아니라 문서·Git·현재 코드의 관계만 확인한다.

## 페이지 구성

1. **추적 요구조건:** 과거 기록과 현재 코드의 명칭이 달라도 변경 계약을 재확인해야 하는 이유를 짧게 제시한다.
2. **변경 추적 관계 다이어그램:** `D13 Pipeline Baseline`과 `B05 First Overlap Symptom`이 `P12 Internal Change Record`에서 합류하고, historical commit `746ea548`과 current `ACWeaponActor` 순서 계약으로 이어지도록 표현한다. 사용자 제작 다이어그램 슬롯으로 유지한다.
3. **증거 대조 2×2 보드:** B05 증상 / P12 판단 / 과거 commit / 현재 코드의 각 역할을 짧은 발췌·경로·핵심 순서로 비교한다. 원문 전체 화면은 작은 면적에서 읽히지 않으므로 사용하지 않는다.
4. **증거 상태와 경계:** historical document·Git, 현재 코드 읽기 확인, 제출용 runtime capture를 분리한다. D13의 시간 관계·과거/현재 클래스 차이·runtime capture 미확보를 명시한다.

## 증거 상태

| 항목 | 상태 | 제출 전 보완 |
| --- | --- | --- |
| B05·D13·P12·commit·현재 코드의 역할 대조 | Ready | 문서·코드 발췌를 2×2 보드용으로 정리 가능 |
| Change Traceability Relation Diagram | Needs Capture | 위 관계를 사용자 제작 다이어그램으로 교체 |
| 현행 first-overlap runtime 결과 | Needs Capture | 유효 Hit Window ID, `InvalidRequest` 미발생, 현재 빌드 식별이 읽히는 캡처 확보 |

## 검증 상태

- HTML 반영: 완료
- 페이지 수: 25페이지 유지 확인 필요
- 제출용 PDF 렌더: 다이어그램 교체 뒤 레이아웃·가독성 재검수 필요
