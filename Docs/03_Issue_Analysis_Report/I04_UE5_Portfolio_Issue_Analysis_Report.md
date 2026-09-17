# UE5 Portfolio Issue Analysis Report

## 제목

**I04: 무기 규격 차이를 흡수하는 Socket·Pivot 계층**

## 날짜·상태

2026.09.17 / 근거 기반 보고서 작성. 검증 한계와 추가 확인은 아래에 명시한다.

## 관련 브랜치

- 원 작업: `staging/portfolio-v1-split`
- 최종 재구성: `promotion/portfolio-v1-final`
- 후속 보완: `fix/execution-montage-terminal-contract`
- 문서: `docs/stellar-asset-integration-reports`

---

## 1. 증상

새 검 도입 후 장탈착 위치 이격, 콤보 양손 전환, 처형에서 손잡이가 아닌 메시 원점 부근을 중심으로 도는 문제가 있었다. 사용자 진술과 세션 A:472,621에 요구가 남아 있다.

## 2. 기존 가정과 제약

Animation Editor의 Preview Asset은 Weapon Blueprint의 보정을 그대로 재현하지 않는다. BP에만 위치·크기를 보정하면 애니메이션 프리뷰에서 정밀하게 맞추기 어렵다. Mesh만 돌리면 Collision·FX와 표현이 갈라진다.

## 3. 대안 검토와 결정

캐릭터 소켓을 저작 기준으로 두고, 소켓 간 전환과 무기 내부 Pivot 회전을 분리했다. RootScene → PresentationPivot → PresentationContentRoot → Mesh/Collision/FX 계층은 회전 기준을 옮기면서 자식이 함께 움직이도록 한다. 손잡이 좌표계로 변환 후 회전하고 무기 좌표계로 복귀한다. 상세 수학은 S39를 단일 기준으로 사용한다.

## 4. 구현·에셋 변경

`CWeaponActor::InitializePivotHierarchy`가 HandGrip을 기준으로 Pivot과 역보정 ContentRoot를 구성한다. `UCWeaponComponent`는 Action Pose Scope 안에서 Socket Transition을 허용한다. 장탈착은 물리 전환 완료 후 논리 commit과 기준선 갱신을 수행하고, 콤보의 임시 손 전환은 종료 때 시작 상태로 복구한다. 정식 종료 흐름이 복구를 소유하며 MontageEnd 자체는 관측용이다.

## 5. 검증 결과와 한계

현재 구현과 사용자 PIE 확인을 대조했다. 회전 수학 테스트와 Presentation 감사 도구가 존재하지만 이번 문서 작업에서 테스트를 새로 실행한 것은 아니다. 당시 Before 영상 없이 시각 개선량을 수치화하지 않는다.

## 6. 추가 확인

문서 보강용으로 Equip/Unequip, 양손 전환, HandGrip 처형 회전 장면을 사용할 수 있다. 이미 정상 확인한 동작을 다시 전체 검사할 필요는 없다.

## 7. 근거와 관련 문서

- [근거 목록 E02, E03, E04, E08](../98_Evidence/Stellar_Asset_Integration/README.md): 커밋·세션 파일·구현 경로.
- [W07 작업 내역](../01_Work_List/W07_Stellar_Asset_Integration/W07_UE5_Portfolio_Work_List.md)
- [S39 현행 표현 계약](../05_System_Architecture/S39_UE5_Portfolio_Weapon_Presentation_Pivot_Architecture.md)
- [F08 종료 계약 보완](../04-02_Fix_Pull_Request/F08_UE5_Portfolio_Pull_Request_Fix.md)

---
