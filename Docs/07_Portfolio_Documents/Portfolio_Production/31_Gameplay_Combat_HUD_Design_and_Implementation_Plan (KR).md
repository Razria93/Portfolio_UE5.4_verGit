# Gameplay Combat HUD Design and Implementation Plan

> 이 문서는 최초 승인·계획 기록이다. 2026-09-20 구현 및 사용자 PIE 확인을 반영한 현재 상태는 [33 구현·검증](33_Gameplay_Combat_HUD_Implementation_and_Validation.md), 브랜치 전체 결과는 [34 마감 기록](34_Gameplay_Combat_HUD_Branch_Closure.md)을 따른다. 아래 TODO 표현·미실시 상태는 당시 계획이며 최종 상태가 아니다.

## 목적

Stellar Blade 전투 HUD 전체를 구성하고 현재 구현된 전투 데이터만 연결함. 미구현 리소스·스킬·아이템은 비활성 TODO 슬롯으로 유지함.

## 관련 브랜치

- `feat/stellar-combat-hud`
- 기준: `48eaae82`
- 상태: 조사·시각 초안 작성 중. Runtime 구현·PIE 미실시.

---

## 1. 승인 범위

2026-09-17 사용자 승인으로 기존 영상용 최소 HP HUD 계획을 확대함. 과거 문서의 미커밋 Stellar 에셋 정리 조건은 지난 기준선이며 반복하지 않음.

| 영역 | 실제 연결 | TODO |
| --- | --- | --- |
| Player | HP | BU·BE·SH |
| 적 | 표시명·HP·Balance·Target | 보조 자원 |
| 스킬 | 슬롯·아이콘·비활성 표현 | 발동·소모·사용 가능 판정 |
| 아이템 | 슬롯·미연결 표현 | 인벤토리·수량·사용 |
| Target Marker | 기존 기능 유지 | 신규 추적 기능 |
| 리소스 | 폰트·아이콘·게이지·배치 | 미확정 원작 동작 |

UI는 전투 판정·자원 계산·Action/Reaction 종료를 소유하지 않음. Debug Overlay를 보존하며 원작 캡처의 FPS/GPU/CPU 표시는 gameplay HUD에서 제외함.

---

## 2. 단계와 사용자 확인

1. 현재 코드·리소스 조사, [32 시각 명세](32_Gameplay_Combat_HUD_Visual_Spec.md) 작성.
2. 폰트 후보·아이콘 샘플·전체 시안 준비.
3. **첫 사용자 확인:** 배치·폰트·아이콘·TODO 표현. 승인 전 runtime 구현하지 않음.
4. UMG와 C++ Presenter 구현, HP·Balance·Target 연결.
5. 빌드·자동검사·해상도·표시 수명 검증.
6. **두 번째 사용자 확인:** 실제 PIE 외형·데이터 갱신.
7. 보완·문서화·논리 단위 로컬 커밋. Push·PR·merge는 하지 않음.

---

## 3. 코드 조사와 최소 변경안

| 근거 | 확인 | 보완안 |
| --- | --- | --- |
| `Source/Portfolio/Component/CHealthComponent.h/.cpp` | HP getter와 사망 delegate 존재, 일반 HP delegate 없음 | 초기화·피해·회복·TryKill·최대값 변경 성공 경로의 값 변경 알림 |
| `Source/Portfolio/Component/CBalanceComponent.h/.cpp` | 패리마다 Count 증가, lifecycle/presentation delegate 존재 | 중간 패리·reset·shutdown을 포함한 수치 snapshot 알림 |
| `Source/Portfolio/Component/CCombatTargetComponent.h` | 변경 이벤트·snapshot·revision 제공 | 콜백 후 현재 snapshot 조회 |
| `Source/Portfolio/Controller/CPlayerController.cpp` | Possess/UnPossess에서 Target consumer 참조 관리 | 같은 수명 경로에 새 HUD consumer 연결 |
| `Source/Portfolio/Component/CTargetHUDPresenterComponent.h` | Marker 생성·투영·수명 담당 | 그대로 보존 |

Local PlayerController 소유 Presenter → 표시 snapshot → UMG 구조를 제안함. 구독 후 초기 snapshot을 읽으며, Target 교체 시 이전 구독·보간을 제거함. 사망 callback 중 Target 해제가 가능하므로 이전 Actor를 무조건 사용하지 않음. UnPossess/EndPlay에서 정리함. 사망 이벤트 순서와 전투 계약은 보존함.

Balance는 화면에서 남은 값 `Clamp(Threshold - CurrentCount, 0, Threshold)`으로 변환하는 안임. 실제 누적·reset 계약은 변경하지 않음.

---

## 4. 표시 정책

- 상단은 현재 Combat Target용이며 별도 보스 encounter 추적을 만들지 않음.
- Target 없음/해제 시 데이터와 패널 제거. 사망 적 HP를 임의 유지하지 않음.
- 화면 밖 Target의 상단 패널은 유지하되 Marker는 기존 투영 정책대로 숨김.
- 표시명은 설정 가능한 FText와 TARGET fallback. Editor Actor label을 사용하지 않음.
- 미구현·실제 0·데이터 없음은 구분함. TODO 슬롯에 가짜 수량·쿨다운·입력을 연결하지 않음.
- Guard/Collapse/Dead 텍스트 배지는 필수 범위가 아님.

---

## 5. 리소스와 에디터

격자·기본 도형은 UMG/UI 머티리얼, 복합 아이콘은 생성 리소스를 후보로 함. 생성 이미지의 글자를 실제 폰트로 사용하지 않음.

현재 LFS 정책은 uasset/umap만 지정함. PNG/TTF도 자동 LFS 대상이라고 가정하지 않음. Runtime import는 첫 승인 후 수행하며 과거 이력은 변경하지 않음.

초기 검색에서 UMG 생성 자동화 스크립트를 발견하지 못했음. 구현에서는 플러그인 활성화나 기존 Blueprint 변경 없이 Native UUserWidget의 WidgetTree로 실제 UMG를 구성함. 별도 WBP 생성은 필요하지 않으며, 현재 레이아웃은 C++에서 관리함. 폰트·아이콘은 create-only Editor Commandlet으로 생성함. 적용·검증 기록은 [33 구현 기록](33_Gameplay_Combat_HUD_Implementation_and_Validation.md)을 참조함.

---

## 6. 검증과 마감

- HP 초기값·피해·회복·최대값 변경·0 최대값·사망.
- Balance 중간 패리·threshold·reset·Target 교체.
- 획득·전환·해제·Destroy·UnPossess·PIE 재시작에서 구독/이전 값/보간 잔류 없음.
- 1080p·1440p·21:9, 밝고 어두운 배경, 긴 이름·한글·수량 폭.
- Marker·Debug Overlay 보존. TODO와 실제 0 구분.
- 빌드/자동검사와 사용자 PIE를 별도 기록함.

커밋은 명세 / 데이터 기반 / UMG·리소스 / 검증·마감으로 구분함. 미확인 항목을 완료로 기록하지 않음.

촬영은 [30 계획](30_Portfolio_Gameplay_Video_Evidence_Plan%20%28KR%29.md), Debug Overlay는 [29 Runbook](29_Portfolio_Runtime_Capture_and_Video_Runbook%20%28KR%29.md)을 유지함.
