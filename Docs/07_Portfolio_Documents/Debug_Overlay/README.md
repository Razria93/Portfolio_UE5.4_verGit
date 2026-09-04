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
- `01_Planning/Debug_Overlay_P1_Focus_Selection_Design_KR.md`
- `01_Planning/Debug_Overlay_P1_Focus_Selection_Decision_KR.md`
- `01_Planning/Debug_Overlay_P1_Focus_Component_Implementation_Plan_KR.md`
- `01_Planning/Debug_Overlay_P1_Focus_Set_Path_Design_KR.md`
- `01_Planning/Debug_Overlay_P1_NearestFocus_Diagnostic_Plan_KR.md`
- `01_Planning/Debug_Overlay_P1_EventLog_Filter_Design_KR.md`
- `01_Planning/Debug_Overlay_P1_EventLog_Noise_Filter_Design_KR.md`
- `01_Planning/Debug_Overlay_P1_Store_Subject_Separation_Design_KR.md`
- `01_Planning/Debug_Overlay_P1_Player_Enemy_EventLog_Separation_Implementation_Plan_KR.md`
- `01_Planning/Debug_Overlay_P1_Subject_EventLog_Role_Filter_Design_KR.md`
- `01_Planning/Debug_Overlay_P1_EventLog_Subject_Role_Label_Design_KR.md`
- `01_Planning/Debug_Overlay_P1_Subject_EventLog_Semantics_Cleanup_Plan_KR.md`
- `01_Planning/Debug_Overlay_P1_Interaction_Event_Flow_Redesign_Plan_KR.md`
- `01_Planning/Debug_Overlay_P1_Actor_Recent_AI_Display_Plan_KR.md`
- `01_Planning/Debug_Overlay_P1_Enemy_Current_AI_Recent_AI_Event_Plan_KR.md`
- `01_Planning/Debug_Overlay_P1_EventLog_Separate_Panel_Design_KR.md`
- `01_Planning/Debug_Overlay_P1_Overlay_Layout_Style_Lock_KR.md`
- `01_Planning/Debug_Overlay_P1_Closure_Criteria_KR.md`
- `02_Operation/Debug_Overlay_Operation_Guide_KR.md`
- `03_Evidence_Map/Debug_Overlay_Evidence_Map_KR.md`
- `04_Capture_Presets/Debug_Overlay_Capture_Presets_KR.md`
- `05_Verification/Debug_Overlay_P0_PIE_Checklist_KR.md`
- `05_Verification/Debug_Overlay_P1_FocusComponent_PIE_Checklist_KR.md`
- `05_Verification/Debug_Overlay_P1_FocusComponent_PIE_Result_KR.md`
- `05_Verification/Debug_Overlay_P1_EventLog_Filter_PIE_Checklist_KR.md`
- `05_Verification/Debug_Overlay_P1_EventLog_Filter_PIE_Result_KR.md`
- `05_Verification/Debug_Overlay_P1_Subject_Role_Label_PIE_Checklist_KR.md`
- `05_Verification/Debug_Overlay_P1_Overlay_Layout_PIE_Result_KR.md`
- `05_Verification/Debug_Overlay_P1_Integrated_PIE_Result_KR.md`
- `05_Verification/Debug_Overlay_P1_Closure_Review_KR.md`
- `05_Verification/Debug_Overlay_P1_Code_Clean_Structure_Review_KR.md`
- `05_Verification/Debug_Overlay_Editor_Tooling_PIE_Result_KR.md`
- `05_Verification/Debug_Overlay_Execution_Collaboration_Branch_Verification_KR.md`
- `06_Evidence_Package/Debug_Overlay_P0_5_Evidence_Package_Round1_KR.md`
- `06_Evidence_Package/Debug_Overlay_P0_5_Final_Capture_Candidate_Plan_KR.md`
- `06_Evidence_Package/Debug_Overlay_P1_Final_Candidate_Capture_Checklist_KR.md`
- `06_Evidence_Package/Debug_Overlay_P1_Final_Candidate_Evidence_Package_KR.md`

## 현재 runtime 운영 기준

현재 runtime의 Character Details, Event Log, World Summary, CVar gate, Combat Participation Evidence
수명·anchor 표시는 [운영 가이드](02_Operation/Debug_Overlay_Operation_Guide_KR.md)를 정규 기준으로 사용한다.

현재 화면 계약의 최소 단위는 다음과 같다.

```text
[Player] / [Enemy]                 : Character Details
[Event Log: Category | Scope: ...] : World 또는 Focused Enemy 이력
[World Summary]                    : 최신 전역 Action/Reaction·Combat·AI 요약
```

- Event Log category는 `Action / Reaction`, `Execution Session`, `Combat`, `AI`, `Balance`, `Death`, `Facing`으로 구분한다.
- Event Log scope는 `World`와 `Focused Enemy`를 지원한다.
- Character Details의 `[Recent Action / Reaction]`은 actor별 최신 실행 판단이며, `[Execution Session]`은 활성 처형 pair-session의 실시간 상태다.
- enum prefix는 표시에서 제거하고, multi-field 상태값은 `|`로 구분한다.

`01_Planning`, 과거 P0/P0.5/P1 검증 결과, 이전 evidence package는 작성 당시의 설계·증적 기록이다. 이들은
현재 runtime 계약을 대체하지 않으며, 당시의 panel 명칭·CVar·화면 제목을 소급 수정하지 않는다.

현재 제출 장면의 선택 기준은 `04_Capture_Presets/Debug_Overlay_Capture_Presets_KR.md`를 따른다. 과거
FinalCandidate package는 당시 capture record로만 참조한다.

## 기록된 브랜치 정보

- 개별 계획·캡처 문서에 적힌 브랜치명은 당시 기록의 메타데이터다. 현재 운영 기준은 특정 브랜치에
  고정하지 않고 위 운영 가이드를 따른다.

## PR / 품질 리뷰 문서

- `05_Verification/Debug_Overlay_P1_Code_Quality_Review_KR.md`
- `05_Verification/Debug_Overlay_W05_PR_Style_Gap_Review_KR.md`
- `05_Verification/Debug_Overlay_P1_Code_Clean_Structure_Review_KR.md`
- `05_Verification/Debug_Overlay_P1_Closure_Review_KR.md`
- `../../04_Pull_Request/P52_UE5_Portfolio_Pull_Request.md`
