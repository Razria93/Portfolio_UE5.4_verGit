# 문서상 p.23 Root Motion Tool 프로젝트 감사 및 구성 계획

> 파일명은 초기 22페이지 체계의 감사 기록을 보존한다. 현행 25페이지 체계에서는 p.23에 적용한다.

## 감사 결과

- `UCAnimModifier_TransferBodyMotionToRoot`는 Editor-only bake utility이며 raw bone transform track을 수정한다.
- Source body bone의 component/world motion을 Root로 옮기고, direct root child를 역보정하는 구현이다.
- Apply 전 Root/direct child track의 존재 여부와 key를 backup한다. Revert는 backup track을 복원하거나 Apply 전 없던 track을 제거한다.
- invalid sequence/model, 잘못된 bone 계층·frame, direct child 부재, tolerance 이상의 기존 Root translation/yaw, transferable delta 부재는 Apply 거절 조건이다.
- Header의 작업 원칙은 duplicated Animation Asset에 적용하는 것이다. 도구가 Asset을 자동 복제한다고 주장하지 않는다.
- Evidence Ledger 상태는 `Needs Capture`이며 source/apply/revert 결과 성공을 현재 주장하지 않는다.

## 구성

1. **요구조건·의도**: Root Motion 변환은 raw transform track 변경이므로, 수용 조건과 복구 범위를 먼저 확보해야 함을 제시한다. 기존 Root Motion 누적을 막는 tolerance gate와 automatic Asset duplication 부재를 명시한다.
2. **구현 구조**: `Duplicated Animation Asset → Preflight → Reject: raw track unchanged / Accept → Root·direct child backup → Root write·child compensation → Revert`를 사용자 제작 다이어그램 슬롯으로 둔다.
3. **실사용 검증**: Source·Apply·Revert를 분리된 성공 카드로 두지 않는다. 동일 복제 Asset을 기준으로 Root/Source 정보와 track 상태가 읽히는 Before·Apply·Revert 3분할 한 장을 필수 evidence slot으로 둔다.
4. **사용 범위**: 수용 입력, backup·restore 범위, 비보장 범위를 3칸으로 분리한다. visual preservation·일반 Undo·모든 Animation 수용은 캡처 전후와 관계없이 주장하지 않는다.

## 독립 검토 반영

- 사본 Asset 작업 원칙과 in-place/tolerance 입력 수용 조건을 분리한다. 도구가 Asset을 자동 복제하지 않음을 고정한다.
- Revert는 backup Root/direct child track의 복원 또는 원래 없던 track 제거로 한정한다.
- 변환 성공·시각 보존은 동일 Asset 3분할 evidence 확보 뒤에만 주장한다.

## 검수 상태

- 코드·증거 감사 및 HTML 구성: 완료 — Tooling 공통 형식(요구조건·의도 → 구현 구조 → 실사용 검증 → 사용 범위)으로 전환
- 증거 상태: Needs Capture — 동일 복제 Asset Before·Apply·Revert 3분할 및 중앙 다이어그램 사용자 제작 필요
- PDF 레이아웃 검증: 보완 후 별도 수행 필요
