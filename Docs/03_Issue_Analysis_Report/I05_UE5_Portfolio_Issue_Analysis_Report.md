# UE5 Portfolio Issue Analysis Report

## 제목

**I05: Trail 설정 집중화와 실행 수명**

## 날짜·상태

2026.09.17 / 근거 기반 보고서 작성. 검증 한계와 추가 확인은 아래에 명시한다.

## 관련 브랜치

- 원 작업: `staging/portfolio-v1-split`
- 최종 재구성: `promotion/portfolio-v1-final`
- 후속 보완: `fix/execution-montage-terminal-contract`
- 문서: `docs/stellar-asset-integration-reports`

---

## 1. 증상

TrailId와 컴포넌트 이름을 따로 매핑하고 Action·무기·컴포넌트 여러 곳에 설정을 분산하면 수정 위치가 늘어났다. 세션 B:5,62에 사용자가 이 문제와 무기 쪽 데이터 집중화를 직접 제안했다.

## 2. 기존 가정과 제약

단일 Trail만 가정하지 않고 Notify 식별자·Action 조건으로 선택할 수 있어야 한다. 데이터 정의 위치와 런타임 인스턴스의 소유자는 구분해야 한다.

## 3. 대안 검토와 결정

식별 조건과 Niagara 에셋을 DataAsset으로 모으고 무기가 런타임 인스턴스를 구성하도록 변경했다. 범용 컴포넌트 관리자를 새로 만든 것이 아니라 WeaponActor의 책임 아래 Trail 인스턴스를 둔 구조다.

## 4. 구현·에셋 변경

`CWeaponTrailDataAsset`은 TriggerKey, ActionFeedbackMatchKey, NiagaraSystems를 정의한다. `CWeaponTrailComponent`는 Niagara 파생 인스턴스 타입이다. 생성·캐시·활성화·해제 관리자는 `CWeaponActor`다. 정식 Action 종료/Stop에서 Weapon runtime 정리와 Action Feedback 정리 경로가 Trail 비활성화를 요청한다. 소스는 Weapon 폴더에서 Component/DataAsset으로 역할별 이동했다.

## 5. 검증 결과와 한계

당시 Trail 재생과 최종 사용자 PIE 정상 확인이 있다. 세션 C:14에는 Advance Combo 전후 Trail Window 배치에 따른 유지 범위 차이 관찰도 있다. 이것은 Notify 구간 저작의 중요성을 보여 주지만 당시 assistant의 index 원인 설명을 현재 코드 결함으로 단정하지 않는다.

## 6. 추가 확인

별도 신규 검증 요구는 없다. 공개 결과 캡처가 필요하면 Trail 시작·종료와 콤보 중단 장면을 사용한다. 데이터화가 성능을 개선했다는 주장은 측정 근거가 없어 하지 않는다.

## 7. 근거와 관련 문서

- [근거 목록 E05](../98_Evidence/Stellar_Asset_Integration/README.md): 커밋·세션 파일·구현 경로.
- [W07 작업 내역](../01_Work_List/W07_Stellar_Asset_Integration/W07_UE5_Portfolio_Work_List.md)
- [S39 현행 표현 계약](../05_System_Architecture/S39_UE5_Portfolio_Weapon_Presentation_Pivot_Architecture.md)
- [F08 종료 계약 보완](../04-02_Fix_Pull_Request/F08_UE5_Portfolio_Pull_Request_Fix.md)

---
