# Debug Overlay Evidence 작업 공간

## 목적

이 폴더는 이력서, 포트폴리오, 기술문서, 제출 영상에서 사용할 debug overlay evidence 작업을 관리한다.

이 overlay는 완성형 게임 HUD가 아니다. Action / Reaction, CombatSignal / Damage, Enemy AI, Runtime LOD 흐름을 화면에서 확인하기 위한 개발 전용 시각 자료다.

## 폴더 구조

- `01_Planning`
  - 작업 범위, 구현 위치 검토, 위험 요소 정리
- `02_Operation`
  - 세션 운영 정책, 브랜치 정책, 검증 흐름
- `03_Evidence_Map`
  - 표시 항목별 코드 근거, 상태 분류, 신뢰도
- `04_Capture_Presets`
  - 제출 영상별 overlay preset 및 화면 구성
- `05_Verification`
  - PIE 확인 체크리스트, 실패 분기, 캡처 전 검증 기준
- `06_Evidence_Package`
  - 1차/최종 evidence 패키지 구성, 파일 목록, 사용 가능 범위

## 주요 문서

- `01_Planning/Debug_Overlay_Plan_KR.md`
- `01_Planning/Debug_Overlay_Implementation_Position_Review_KR.md`
- `01_Planning/Debug_Overlay_P0_Final_Decision_KR.md`
- `01_Planning/Debug_Overlay_P0_5_Player_Enemy_Extension_Design_KR.md`
- `01_Planning/Debug_Overlay_P0_5_HUD_Panel_Implementation_Plan_KR.md`
- `01_Planning/Debug_Overlay_P0_5_Compact_Display_Subject_Review_KR.md`
- `01_Planning/Debug_Overlay_P0_5_Final_Decision_KR.md`
- `01_Planning/Debug_Overlay_P0_5_Remaining_Refactor_Plan_KR.md`
- `01_Planning/Debug_Overlay_P1_Work_Order_KR.md`
- `01_Planning/Debug_Overlay_P1_Scope_KR.md`
- `01_Planning/Debug_Overlay_P1_Target_Selection_Design_KR.md`
- `01_Planning/Debug_Overlay_P1_Target_Selection_Decision_KR.md`
- `01_Planning/Debug_Overlay_P1_Target_Component_Implementation_Plan_KR.md`
- `01_Planning/Debug_Overlay_P1_Target_Set_Path_Design_KR.md`
- `01_Planning/Debug_Overlay_P1_NearestTarget_Diagnostic_Plan_KR.md`
- `01_Planning/Debug_Overlay_P1_EventLog_Filter_Design_KR.md`
- `01_Planning/Debug_Overlay_P1_Store_Subject_Separation_Design_KR.md`
- `01_Planning/Debug_Overlay_P1_Player_Enemy_EventLog_Separation_Implementation_Plan_KR.md`
- `02_Operation/Debug_Overlay_Operation_Guide_KR.md`
- `03_Evidence_Map/Debug_Overlay_Evidence_Map_KR.md`
- `04_Capture_Presets/Debug_Overlay_Capture_Presets_KR.md`
- `05_Verification/Debug_Overlay_P0_PIE_Checklist_KR.md`
- `05_Verification/Debug_Overlay_P1_TargetComponent_PIE_Checklist_KR.md`
- `05_Verification/Debug_Overlay_P1_TargetComponent_PIE_Result_KR.md`
- `05_Verification/Debug_Overlay_P1_EventLog_Filter_PIE_Checklist_KR.md`
- `05_Verification/Debug_Overlay_P1_EventLog_Filter_PIE_Result_KR.md`
- `06_Evidence_Package/Debug_Overlay_P0_5_Evidence_Package_Round1_KR.md`
- `06_Evidence_Package/Debug_Overlay_P0_5_Final_Capture_Candidate_Plan_KR.md`

## 현재 상태

P0.5 기준 overlay는 TestRoom PIE에서 다음 화면 구조로 확인한다.

```text
[Debug Overlay P0.5]

[Player]
State:
Action:
Reaction:
Guard:
Movement:
HP:
Runtime LOD:
AI:

[Enemy]
State:
Action:
Reaction:
Guard:
Movement:
HP:
Runtime LOD:
AI:

[Recent Execution]
[Recent Combat]
[Recent AI]
[Event Log]
```

P0.5의 핵심은 Player/Enemy 상태를 같은 순서로 비교하고, 최근 execution/combat/AI/event evidence를 공통 recent block으로 확인하는 것이다.

현재 P0.5에서 확정한 표시 정책:

- enum prefix는 overlay 표시에서 제거한다.
- multi-field 상태값은 `|`로 구분한다.
- Guard action은 `Guard In`, `Guard Out`처럼 index 없이 표시한다.
- Execution summary는 `Action(ComboAttack[1])`, `Reaction(Hit)`처럼 subject를 포함한다.
- Enemy는 P0.5에서 `WorldScanFallback` 기반으로 표시하며, Target Component 기반 선택은 P1 후보로 둔다.
- EventLog 추가 축약, category filter, Player/Enemy별 EventLog 분리는 P0.5에서 제외한다.

현재 캡처 파일은 1차 패키지로만 정리한다. 반복 촬영/패키징은 P1 완료 전까지 중단하고, 최종 제출 후보는 P1 설계/구현/검증 이후 별도 패키지로 분리한다.

## 현재 브랜치

- `feature/debug-overlay-evidence-plan`

## PR / 품질 리뷰 문서

- `05_Verification/Debug_Overlay_P1_Code_Quality_Review_KR.md`
- `05_Verification/Debug_Overlay_W05_PR_Style_Gap_Review_KR.md`
- `../../04_Pull_Request/P52_UE5_Portfolio_Pull_Request.md`
