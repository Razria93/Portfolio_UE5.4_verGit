# Portfolio Evidence Capture Ledger

> 목적: 포트폴리오에 사용되는 증거의 파일 경로, 사용 목적, 검수 상태와 신규 캡처 필요 여부를 관리한다.

상태 표기: `Ready` / `Needs Capture` / `Needs Review` / `Do Not Use`

| 사용 페이지 | 증거 | 현재 파일 또는 출처 | 목적 | 상태 | 주의 사항 |
| --- | --- | --- | --- | --- | --- |
| 1 | 대표 전투 영상 | 신규 제작 | 프로젝트의 실제 플레이 결과 | Needs Capture | 20~30초, HUD·디버그 오버레이 없는 대표 장면 우선 |
| 4 | Targeting 기존 후보 | `DebugOverlay/FinalCandidate/debug_overlay_p1_final_target_nearest.png` | Targeting 증거 후보 | Do Not Use | Player Target이 `N/A`이고 Enemy Focus 선택 정보가 표시된 화면이므로 Player Targeting 근거로 사용하지 않음 |
| 4 | Targeting | 신규 제작 | Player Target Lock과 대상 획득·전환·해제 Runtime 데이터 | Needs Capture | Hero는 Debug Overlay 없이 촬영하고, 획득·전환·해제의 Overlay Crop은 별도 검증 근거로 사용 |
| 5 | Combo / Hit | 신규 제작 | 콤보·Hit Window·피격 흐름 및 연계 예약·판정·결과 수용 Runtime 데이터 | Needs Capture | Hero는 Debug Overlay 없이 2~3타 콤보와 Enemy Hit React를 촬영하고, 연계 예약·HitWindowId·피격 결과는 Overlay 또는 Event Log Crop으로 별도 확보 |
| 6 | Guard / Parry 게임플레이 비교 | 신규 제작 | Guard 자세·Parry 성공·Player Hit의 실제 플레이 차이 | Needs Capture | Debug Overlay 없는 16:9 Hero 또는 3분할 프레임. 기존 FinalCandidate는 결과 데이터 검증에만 사용 |
| 6 | Block Hit | `DebugOverlay/FinalCandidate/debug_overlay_p1_final_block_hit.png` | 결과 필드 목업의 Guard·BlockHit·Final/Commit 값 출처 | Ready | p.6에는 이미지를 직접 배치하지 않고 텍스트 목업만 사용 |
| 6 | Parry | `DebugOverlay/FinalCandidate/debug_overlay_p1_final_parry.png` | 결과 필드 목업의 Parry·Final/Commit 값 출처 | Ready | p.6에는 이미지를 직접 배치하지 않고 텍스트 목업만 사용 |
| 6 | Player Hit | `DebugOverlay/FinalCandidate/debug_overlay_p1_final_player_hit.png` | 결과 필드 목업의 Hit·HP 감소·Commit 값 출처 | Ready | p.6에는 이미지를 직접 배치하지 않고 텍스트 목업만 사용 |
| 7 | Dodge Hero | 신규 제작 | Guard → Dodge 또는 PIE로 검증된 Action → Dodge 전환 | Needs Capture | Hero는 Debug Overlay 없이 촬영. 피격 반응 중 개입 성공을 전제하지 않음 |
| 7 | Dodge decision runtime | 신규 제작 | Incoming Dodge와 현재 실행의 Decision / ApplyMode / RejectReason 확인 | Needs Capture | Overlay 또는 Event Log에서 실제로 읽히는 값만 캡션으로 사용 |
| 7 | Dodge during active Reaction | 신규 제작 | active Reaction 중 Dodge 요청 결과 | Needs Capture | Intervened 또는 Rejected 중 실제 결과를 그대로 기록 |
| 8 | Stagger stack 2 | `DebugOverlay/FinalCandidate/debug_overlay_p1_final_stagger_stack_2.png` | Balance 누적 | Ready | 3단계와 쌍으로 사용. Execution 결과 증거로 전용하지 않음 |
| 8 | Stagger stack 3 | `DebugOverlay/FinalCandidate/debug_overlay_p1_final_stagger_stack_3.png` | Collapse 진입 | Ready | Execution 결과 캡처로 전용하지 않음 |
| 8 | Balance → Collapse Hero | 신규 제작 | 실제 Balance 누적 또는 Collapse 상태 전환 | Needs Capture | Hero는 Debug Overlay 없이 촬영 |
| 9 | Execution session / gameplay | 신규 제작 | Standard 또는 Lethal Pair Session의 reservation·commit·terminal과 동기 gameplay | Needs Capture | 확보 전에는 Pair 계약·사전 거절 규칙만 코드·문서 범위로 표현 |
| 9 | HP 1 Standard pre-check | 신규 제작 | AppliedDamage 0일 때 Pair 예약 전 거절되는 회귀 방지 계약 | Needs Capture | 실제 Pair 시작·Commit 결과 증거와 혼동하지 않음 |
| 11 | Execution transition trace | 신규 제작 | Action→Reaction·Reaction→Reaction의 Decision·Relationship·ApplyMode·Directive 대조 | Needs Capture | B07 AI combo는 Action chain 근거일 뿐 공통 전환 계약의 runtime 증거로 전용하지 않음 |
| 11 | B10 terminal cleanup | `B10_UE5_Portfolio_Bug_Report.md`, 신규 PIE capture | Interrupt 뒤 Trail·Collision·Hit Context cleanup | Needs Capture | B10의 기존 runtime 기록은 유지하되, Combat Signal Packet 사례로 배치하지 않음 |
| 12 | B09 Intervention policy | `B09_UE5_Portfolio_Bug_Report.md`, 신규 PIE capture | Want·Allow·timing gate와 interrupt 결과 | Needs Capture | Dead 강제 개입은 일반 Want∧Allow 사례와 분리 |
| 13 | Combat Signal Source→Target trace | 신규 제작 | 같은 사건의 Source Context와 Target Packet 대조 | Needs Capture | Source 0 damage/CommitFailed만으로 Parry 실패를 판단하지 않음 |
| 14 | Target Outcome comparison | 신규 제작 | 동일 계열 hit의 Normal·Guard·Parry Target Packet 비교 | Needs Capture | Accepted·DefenseOutcome·ReactionOutcome·HP 전후를 함께 표시 |
| 15 | Execution Pair coordination | 신규 제작 | Pair Reservation→Active→Commit→terminal, 사전 거절, Commit 전 Release | Needs Capture | p.9 gameplay와 구조 설명을 중복하지 않고, 완전 동시 실행으로 표현하지 않음 |
| 10 | Dead Presentation Hero | 신규 제작 | Dead 전이와 사망 표현의 실제 게임플레이 | Needs Capture | Debug Overlay 없이 Dead 진입과 Presentation이 읽히는 16:9 프레임 |
| 10 | Dead Entry lifecycle | 신규 제작 | Health Dead, death event, 실행 정리 순서 | Needs Capture | Debug Overlay 또는 Event History crop으로 실제 값을 확인 |
| 10 | Facing Suppressed(Dead) | 신규 제작 | Gameplay Focus 해제·Rotation Mode 전환·deferred sync 억제 | Needs Capture | Target 보관과 Facing 억제를 혼동하지 않게 상태 값을 함께 촬영 |
| 10 | Finalize / Destroy route | 신규 제작 | Presentation complete 또는 fallback 이후 종료 경로 | Needs Capture | 완료·fallback·finalize request가 하나의 흐름으로 읽히게 확보 |
| 16 | Current AI | `DebugOverlay/FinalCandidate/debug_overlay_p1_final_enemy_current_ai.png` | AI 현재 상태 | Ready | active node tracking으로 표현하지 않음 |
| 16 | Recent AI Event | `DebugOverlay/FinalCandidate/debug_overlay_p1_final_enemy_recent_ai_event.png` | 의도·요청 결과 | Ready | Behavior Tree의 모든 내부 상태를 보여준다고 주장하지 않음 |
| 20 | Enemy Current AI | `DebugOverlay/FinalCandidate/debug_overlay_p1_final_enemy_current_ai.png` | Focus된 Enemy의 Current AI 필드 표시 근거 | Ready | Current AI 블록의 Target·Intent·HasLOS·Distance·Action 표시만 확인한다. Event Log와의 같은 화면 대조 근거로 전용하지 않음 |
| 20 | Focused Enemy Runtime Debug Usage | 신규 제작 | Focus된 Enemy의 Character Details와 `Event Log: Focused Enemy` 실제 이벤트 행을 같은 Runtime 화면에서 대조 | Needs Capture | p.20 중심 실사용 검증. p.21 Editor Plugin UI는 화면에 섞지 않음 |
| 21 | Toolbar / Nomad / Focus | `DebugOverlay/EditorTooling` 폴더 | Editor-only 진입점, session CVar UI, 기존 PIE Focus command 위임 | Ready | 기존 후보: `debug_overlay_editor_tooling_02_nomad_panel_target_select.jpg`. 현행 p.21 중심 실사용 슬롯은 사용자 캡처 대기이며, Runtime Overlay 화면·상태 소유 근거로 전용하지 않음 |
| 22 | Asset Inspector Panel | `AssetReferenceInspector/asset_reference_inspector_01_toolbar_button.jpg`, `_02_nomad_panel.jpg` | Toolbar 진입점과 Panel UI 존재 | Ready | `_02_nomad_panel.jpg`는 `Selected Asset: None`·빈 Tree 상태이므로 관계 조사·CSV 실사용 검증으로 전용하지 않음 |
| 22 | Asset Inspector Actual Usage | 신규 제작 | 실제 Selected Asset·Mode·Depth·비어 있지 않은 Tree 결과와 CSV export 상태 | Needs Capture | Panel을 크게 확장해 조사 조건과 결과가 함께 읽히도록 캡처. Unused Candidate는 삭제 판정으로 표시하지 않음 |
| 23 | Root Motion Before / Apply / Revert | 신규 제작 | 동일 복제 Asset의 in-place Source·Root Track Before, Apply 뒤 Root Track 기록, Revert 뒤 Apply 전 track 복구 | Needs Capture | 3분할을 한 장의 실사용 검증 이미지로 구성. 같은 Asset 이름·경로와 Root / Source bone 정보를 각 분할에서 확인할 수 있어야 하며, 시각 보존 성공은 이 증거 확보 뒤에만 표현 |
| 24 | AI Workflow Work ID Board | 신규 제작 | 동일 Work ID의 Work Brief(목표·범위·수용 기준), Review / Decision(AI 제안 수용·보류·수정 근거), Verification Record(변경 파일·수행 검증·미검증 사유) | Needs Capture | 일반 Prompt 템플릿이 아닌 실제 작업 기록을 사용. 세 문서에 같은 Work ID 또는 작업명이 읽혀야 함 |
| 19 | 80 Enemy Candidate Audit CSV·로그 | `Docs/98_Evidence/Profiling/AI_Perception_Candidate_Audit_20260803/` | 후보 수·무효 후보·First Valid Target p95의 Before/After 비교 | Ready | 1회 Before/After 실행: 10.064s→1.070s, 81/80/81→1/0/1. 전체 FrameTime 개선·반복 재현성으로 일반화하지 않으며 3회 반복 측정은 보완 예정 |
| 25 | B05 Change Traceability 문서·코드 대조 | `B05_UE5_Portfolio_Bug_Report.md`, `Docs/99_Legacy/Issue_CheckList/D13_UE5_Portfolio_Issue_Checklist.md`, `P12_UE5_Portfolio_Pull_Request.md`, commit `746ea548`, `ACWeaponActor::CollisionEnabled` | D13 선행 기준·B05 증상·P12 판단·과거 patch·현재 Hit Window 순서 계약의 역할과 시점 대조 | Ready | D13은 B05보다 앞선 baseline이며, historical `CAttachment`와 current `ACWeaponActor`의 클래스 동일성은 주장하지 않음 |
| 25 | B05 Change Traceability 관계 다이어그램 | 신규 제작 | D13 baseline과 B05 symptom이 P12에서 합류하고, historical commit과 current sequence contract·runtime evidence 상태로 이어지는 관계 | Needs Capture | 사용자 제작 다이어그램. 문서의 시간 순서와 역할이 직선 인과로 오독되지 않아야 함 |
| 25 | B05 현행 first-overlap runtime 결과 | 신규 제작 | 현재 빌드에서 유효 Hit Window ID, `InvalidRequest` 미발생, 실행 대상/빌드 식별이 함께 읽히는 runtime capture | Needs Capture | B05·P12의 historical validation 기록을 현행 런타임 성공 증거로 대체하지 않음 |

## 캡처 원칙

1. 게임 결과를 보여주는 캡처와 검증을 보여주는 Debug Overlay 캡처를 혼동하지 않는다.
2. 임시 Round1 자료는 FinalCandidate와 같은 증거 등급으로 사용하지 않는다.
3. 하나의 페이지에는 최대 3장의 캡처만 사용한다.
4. 캡처 파일을 교체하면 이 대장과 Page Spec Index를 함께 갱신한다.

## 관련 제작 문서

- [Page Enrichment Backlog](../07_Portfolio_Documents/Portfolio_Production/01_Page_Enrichment_Backlog (KR).md)
- [Portfolio Capture Shot List](../07_Portfolio_Documents/Portfolio_Production/02_Portfolio_Capture_Shot_List (KR).md)
- [Portfolio Claim Freeze Log](../07_Portfolio_Documents/Portfolio_Production/03_Portfolio_Claim_Freeze_Log (KR).md)
