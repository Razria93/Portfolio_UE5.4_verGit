# UE5 Portfolio Issue Analysis Report

> PR #122 후속 검증: Section 경로 감사를 추가하고 Parry 두 에셋의 없는 `Start` 연결을 `None`으로 정리했다. HitReact는 연속 순방향 경로와 Complete 순서가 정상이라 수정하지 않았다. 최종 재감사 0/0 및 테스트 결과는 [F08](../04-02_Fix_Pull_Request/F08_UE5_Portfolio_Pull_Request_Fix.md)의 Section 검토 후 마감 결과를 참조한다. 일반 time-zero Notify의 재생 호출 내 동기 전달은 엔진 기반 테스트에서 재현되지 않아 런타임 시작 구조는 변경하지 않았다.

## 제목

**I06: Montage 자동 완료에 가려진 종료 설정과 검증**

## 날짜·상태

2026.09.17 / 근거 기반 보고서 작성. 검증 한계와 추가 확인은 아래에 명시한다.

## 관련 브랜치

- 원 작업: `staging/portfolio-v1-split`
- 최종 재구성: `promotion/portfolio-v1-final`
- 후속 보완: `fix/execution-montage-terminal-contract`
- 문서: `docs/stellar-asset-integration-reports`

---

## 1. 증상

Action의 자연 MontageEnd 자동 Complete 제거 후 Stellar Equip의 Complete Trigger가 Unequip으로 설정된 문제가 드러났다. 사용자가 수정하고 명시적 Complete가 있어야 완료됨을 확인했다.

## 2. 기존 가정과 제약

Montage가 끝났다는 엔진 관측과 게임 실행이 완료됐다는 상태 변경은 다르다. Source Execution은 Action, Target Execution은 Reaction이며 Notify 클래스와 Trigger가 해당 실행과 일치해야 한다.

## 3. 대안 검토와 결정

누락을 fallback으로 덮지 않고 에셋을 수정했다. Action에 이어 Reaction의 자연 종료도 관측만 하도록 통일했다. Stop/Interrupt는 종료·복구를 계속 담당하며 time-zero Notify 방어도 유지했다.

## 4. 구현·에셋 변경

`051a5884`에서 Reaction 자연 종료를 `NaturalMontageEndObserved` 기록으로 변경했다. 감사 도구는 Complete 클래스·Type·ActionIndex·배치 시점을 검사한다. 스캔 데이터에서 참조를 찾지 못한 Montage는 미사용이라고 단정하지 않는다.

## 5. 검증 결과와 한계

기존 실행 로그에서 1,399 packages 로드, 실패 0, 93 data rows, 감사 오류/경고 0을 재확인했다. 49 Montage에 직접 배치된 이벤트는 151개다. 사용자가 최종 PIE 정상 확인했다. Sequence Notify·level override·동적 참조·실제 전달은 정적 감사의 보장 범위가 아니다.

## 6. 추가 확인

이미 수행한 전체 PIE를 다시 요청하지 않는다. 향후 조건부 Complete가 여러 개인 Montage/section을 추가하면 감사 정책이 보수적으로 오류 또는 경고로 보고할 수 있으므로 개별 실행 경로를 확인한다. 기존 SK_Mannequin material import 경고는 별도 사항이다.

## 7. 근거와 관련 문서

- [근거 목록 E10, E11](../98_Evidence/Stellar_Asset_Integration/README.md): 커밋·세션 파일·구현 경로.
- [W07 작업 내역](../01_Work_List/W07_Stellar_Asset_Integration/W07_UE5_Portfolio_Work_List.md)
- [S39 현행 표현 계약](../05_System_Architecture/S39_UE5_Portfolio_Weapon_Presentation_Pivot_Architecture.md)
- [F08 종료 계약 보완](../04-02_Fix_Pull_Request/F08_UE5_Portfolio_Pull_Request_Fix.md)

---
