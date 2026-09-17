# UE5 Portfolio Issue Analysis Report

## 제목

**I08: Content 참조 검토·호환성·LFS 이주**

## 날짜·상태

2026.09.17 / 근거 기반 보고서 작성. 검증 한계와 추가 확인은 아래에 명시한다.

## 관련 브랜치

- 원 작업: `staging/portfolio-v1-split`
- 최종 재구성: `promotion/portfolio-v1-final`
- 후속 보완: `fix/execution-montage-terminal-contract`
- 문서: `docs/stellar-asset-integration-reports`

---

## 1. 증상

대규모 경로 정리에서 Demo/Placeholder 이름만 보고 삭제하면 실제 머티리얼에 필요한 리소스를 잃을 수 있다. 이주 중 기존 BP 필드 참조와 저장 정책도 함께 유지해야 했다.

## 2. 기존 가정과 제약

Asset Registry 조회는 삭제 안전성의 완전한 증명이 아니다. 동적 문자열 참조·Asset Manager 등은 별도로 고려해야 한다. 원래 작업 이력과 최종 재구성 이력을 중복 합산하지 않는다.

## 3. 대안 검토와 결정

세션 C:27에서 Placeholder_Normal 삭제 가능 여부를 확인했다. 당시 도구의 M_CELESTE_HEAD export/graph 조사 결과를 바탕으로 삭제보다 보존·이주를 선택했고, 사용자 C:182가 이동 위치를 요청했다. 현재 T_CELESTE_FlatNormal이 존재한다. 이 사례는 이름보다 실제 사용을 확인한 보존 판단으로 기록한다.

## 4. 구현·에셋 변경

Content는 역할별 경로로 이동하고 legacy Notify를 제거했다. Weapon 필드 rename에는 PropertyRedirect를 적용하면서 Native Component 내부 이름은 유지했다. enum 숫자는 실제 이동했으므로 이 조치가 enum까지 보호한다고 설명하지 않는다. `ace28d4c`는 현재 528개 에셋을 LFS 포인터로 정규화한 커밋이며 과거 전체 이력 재작성은 아니다.

## 5. 검증 결과와 한계

사용자는 캐릭터 정리에 AssetReferenceInspector를 사용했다고 확인했다. 다만 Placeholder 당시 화면이 자체 플러그인인지 표준 Reference Viewer인지 아직 구분되지 않는다. 기존 에셋참조 실사례 이미지는 Lightning BlockHit FX Dependencies이므로 캐릭터 정리 증거로 쓰지 않는다.

## 6. 추가 확인

당시 조회 화면/CSV가 있으면 도구와 조회 조건을 확인한다. 없으면 현재 캐릭터 참조 조회를 재현해 현재 검증으로 기록한다. Unused Candidate는 삭제 허가가 아니며, 로컬 LFS 검사도 원격 객체 존재를 보장하지 않는다.

## 7. 근거와 관련 문서

- [근거 목록 E07, E08, E09](../98_Evidence/Stellar_Asset_Integration/README.md): 커밋·세션 파일·구현 경로.
- [W07 작업 내역](../01_Work_List/W07_Stellar_Asset_Integration/W07_UE5_Portfolio_Work_List.md)
- [S39 현행 표현 계약](../05_System_Architecture/S39_UE5_Portfolio_Weapon_Presentation_Pivot_Architecture.md)
- [F08 종료 계약 보완](../04-02_Fix_Pull_Request/F08_UE5_Portfolio_Pull_Request_Fix.md)

---
