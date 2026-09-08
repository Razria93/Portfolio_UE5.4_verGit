# Portfolio Page Spec Index

> 목적: 이 문서는 제출 전 25페이지 전환안에서 각 페이지가 증명할 한 가지 메시지와 필요한 근거를 고정한 계획 기록이다.
> 제출본 상태(2026-09-07): 현재 제출 HTML은 [23페이지 문서](염동섭_UE5_Portfolio_Project_Stellar.html)이며, p.24 AI Workflow와 p.25 Traceability는 소스에 보존하되 제출 화면에서는 제외했다. E11–E15의 런타임 증거 상태는 Evidence Ledger와 유지보수 TODO에서 관리한다.

상태 표기: `Planned` / `Evidence Ready` / `Drafted` / `Reviewed` / `Final`

| p. | 페이지 | 한 줄 핵심 메시지 | 중심 시각 자료 | 핵심 근거 | 상태 |
| ---: | --- | --- | --- | --- | --- |
| 1 | Cover | 전투·AI·성능·툴링을 구조 설계·측정·검증·문서·기록의 기준으로 설명하는 UE5 C++ 프로젝트 | 대표 전투 화면 + Focus 3축 + 대표 근거 | 프로젝트 요약, GitHub·Docs 링크, 대표 게임플레이 URL | Planned |
| 2 | Index | Combat Core / AI & Performance / Tooling & Workflow로 탐색 가능 | 6개 읽기 그룹 Index | 본 문서 | Drafted |
| 3 | System Map | 입력·AI 판단·피해 사건은 요청과 전투 데이터로 해석되고, 결과는 자원·실행·효과 영역에서 분리해 반영된다 | 통합 전투 시스템 맵 + 전투 대상 관리 흐름 | Action/Reaction·Combat Signal·Health·Feedback·`CCombatTargetComponent`·Participation, S13·S33·S34 | Drafted |
| 4 | Targeting | 후보 평가는 대상 변경을 요청하고, 카메라·이동·표시는 공통 대상 변경을 구독해 적용한다 | Lock 게임플레이 Hero + 상황별 Runtime 데이터 검증 | `CPlayerTargetSelectionComponent`, `CCombatTargetComponent`, `CTargetLockAssistComponent`, `CTargetHUDPresenterComponent`, W05·P58 | Drafted |
| 5 | Combo / Hit | 다음 콤보는 연계 가능 구간에서만 예약하며, collision 전에 유효 Hit Window를 준비해 첫 overlap의 invalid reject를 방지한다 | 콤보 Hero + 런타임 데이터 3사례 + B05 순서 보정 | `CAction_ComboAttack`, `CWeaponActor`, B05, Combat Signal Source | Drafted |
| 6 | Guard / Parry | 피격 신호는 방어 상태와 패링 허용 구간을 먼저 해석하며, Guard Out 중에도 Hit reaction 시작을 막지 않도록 cleanup·registry를 정리한다 | 게임플레이 비교 Hero + 결과 필드 검증 목업 + B12 해결 기록 | C04a 결과 필드 출처, B12, `EDamageDefenseOutcome`, `EDamageReactionOutcome` | Drafted |
| 7 | Dodge Intervention | 회피 요청은 현재 실행을 무조건 덮어쓰지 않고, 실행 관계와 Allow 정책을 통과할 때만 시작하거나 개입한다 | 회피 Hero + 개입 결정 Runtime 사례 | `CAction_Dodge`, Action Orchestrator, B09 | Drafted |
| 8 | Balance / Collapse | Collapse는 Balance 누적·상태 전이·Loop TTL·Reset을 함께 관리하는 전투 수명주기다 | Debug Overlay 없는 Balance → Collapse gameplay Hero + 상태 전이 증거 목업 | `CBalanceComponent`, S35, C06a Stack 2·3 보조 근거, C06b Hero 필요 | Drafted: state transition captures needed |
| 9 | Execution Collaboration | Execution은 두 Actor가 대상 관계·기회·결과를 검증·예약·확정·해제하는 Pair Transaction이다 | Debug Overlay 없는 Execution Pair gameplay Hero + Session evidence 목업 | `CExecutionCollaborationComponent`, `TryResolveExecutionAppliedDamage`, S36, C06c~e 필요 | Drafted: runtime captures needed |
| 10 | Death Lifecycle | Health의 Dead 전이 이후에도 Presentation·Facing 억제·Finalize·Destroy는 별도 수명주기로 관리한다 | Death lifecycle flow + Dead Facing suppression + runtime evidence 목업 | `CHealthComponent`, `ACEnemy`, `CEnemyCombatTargetFacingComponent`, S31 | Drafted: dedicated runtime captures needed |
| 11 | Execution Transition | Action과 Reaction은 별도 도메인 실행을 유지하며, 허용된 전환 Directive 뒤 기존 executor가 Runtime Effect를 terminal cleanup한 다음 새 executor를 시작한다 | 구조 문제 → Before/After 전환 비교 → Orchestrator·기존 Executor·다음 Executor 책임 분리 | S37, Action/Reaction Orchestrator, B10 | HTML Drafted: 구조 설명 전용 / E11-A·B는 Ledger에서 capture needed |
| 12 | Intervention Policy | 일반 개입은 incoming Want·active Allow·필요한 timing gate를 함께 평가해 Directive와 Intervene을 결정한다 | 구조 문제 → 정책 흐름 다이어그램 → Decision·Relationship·Apply/Directive 정책 계층 | S37, B09, Action/Reaction Orchestrator | 정책 다이어그램 반영 완료: `Assets/Diagrams/p12_intervention_policy.png` / E12는 Ledger에서 capture needed |
| 13 | Combat Signal Source Boundary | Source는 hit/cue의 사실을 구성·전달하고 Target의 Defense·Reaction Outcome을 직접 결정하지 않는다 | 구조 문제 → Source→Target Boundary 다이어그램 → Source Normalize·Delivery Entry·Target Boundary | S38, `CCombatSignalSourceComponent`, `FHitContext` | Damage·Cue 병렬 다이어그램 반영 완료: `p13_combat_signal_damage.png`, `p13_combat_signal_cue.png` / E13은 Ledger에서 capture needed |
| 14 | Target Outcome Resolution | Target은 incoming packet과 자신의 상태·방어 규칙을 해석해 Normal/Guard/Parry Outcome을 확정한 뒤 결과를 dispatch한다 | 구조 문제 → Target Outcome Flow 다이어그램 → Target Context·Outcome Resolution·Result Dispatch | S38, `CCombatSignalTargetComponent`, `FCombatSignalTargetPacket` | Target Outcome 다이어그램 반영 완료: `p14_target_outcome_resolution.png` / E14는 Ledger에서 capture needed |
| 15 | Execution Pair Coordination | 처형 Pair는 양측 조건을 같은 Session으로 확인하고 reservation·Active·Commit·Release를 소유하는 양방향 협업 거래다 | 구조 문제 → Pair Transaction 다이어그램 → Source·Pair Contract·Target Boundary | S36, `CExecutionCollaborationComponent`, `CBalanceComponent` | Pair Transaction 다이어그램 반영 완료: `p15_execution_pair_coordination.png` / E15-A/B/C는 Ledger에서 capture needed |
| 16 | AI Intent | BT는 판단 문맥을 Intent로 정리하고, Task는 요청·종료 대기만 맡겨 공통 Action 실행 계약에 위임한다 | 구조 문제 → Intent Flow 다이어그램 → Blackboard Context·Intent Service·BT Task/Execution | `CBTService_UpdateAIIntentState`, `CBTTask_StartCombatAction`, `CBTTask_WaitEndCombatAction` | Intent Flow 다이어그램 반영 완료: `p16_ai_intent.png` / Runtime 성공 주장은 없음 |
| 17 | Combat Target | 결정 정책은 요청하고 `UCCombatTargetComponent`가 상태·Revision·수명을 단일 기준으로 소유하며, Consumer는 Snapshot·Event를 읽는다 | 구조 문제 → Target State Contract 다이어그램 → Decision Request·Target Kernel·Snapshot Consumer | `CCombatTargetComponent`, `FCombatTargetSnapshot`, S33 | Target State Contract 다이어그램 반영 완료: `p17_combat_target.png` / Runtime 성공 주장은 없음 |
| 18 | Participation | World Subsystem은 Participant × Target의 live Evidence를 집계해 None·Observe·Alert·Engage 역할을 할당하고, 근거 종료 시 참여를 재평가한다 | 구조 문제 → Participation Contract 다이어그램 → Evidence Ingress·Assignment Kernel·Applied Snapshot/Release Guard | `CWorldSubsystem_CombatParticipation`, `CEnemyCombatParticipationComponent`, S34 | Participation Contract 다이어그램 반영 완료: `p18_participation.png` / Runtime 성공 주장은 없음 |
| 19 | Profiling | 80 Enemy 조건에서 후보 구성을 분리 측정해 유효 전투 대상 확정 지연의 원인을 추적했다 | 문제 제기 → 측정 환경 → Before=100 정규화 p95 막대그래프 + 후보 필터 적용 패널 → 측정 계약 3카드 | `98_Evidence/Profiling/AI_Perception_Candidate_Audit_20260803`의 Before/After CSV·로그·분석 스크립트 | Ready: 1회 비교 / 3회 반복 보완 예정 |
| 20 | Runtime Debug | Focus된 Enemy의 현재 상태와 Target Facing 문맥을 같은 Runtime 패널에서 대조하되, 도구는 게임플레이 상태·판정을 소유하지 않는다 | 요구조건·의도 → 관찰 경계 다이어그램 → Focused Enemy 현재 상태·Facing 실사용 검증 → 사용 범위 | `CDebugOverlayHUD`, SnapshotStore·ViewData·Renderer, Focused Enemy Runtime Capture | Ready: Focused Enemy Runtime 패널 |
| 21 | Overlay Editor Plugin | Editor-only Nomad Panel은 p.20 관찰에 필요한 표시 조건과 Focus 요청을 기존 CVar·PIE command로 연결하되, runtime 상태 소유권은 가져가지 않는다 | 요구조건·의도 → Editor Control Bridge 다이어그램 → Nomad Panel 실사용 검증 → 사용 범위 | `Plugins/PortfolioDebugOverlayEditor`, EditorTooling FinalCandidate | Ready: Nomad Panel session-only CVar 설정 화면 |
| 22 | Asset Inspector | 선택 Asset의 관계를 Asset Registry로 조회해 Tree·Content Browser Sync·CSV로 조사·기록하며, 출력값은 삭제 판정이 아닌 검토 입력이다 | 요구조건·의도 → Asset Reference Investigation 다이어그램 → Selected Asset·Tree 실사용 검증 → 사용 범위 | `Plugins/AssetReferenceInspector`, README, CSV exporter | Partial: Selected Asset·Tree 캡처 Ready / CSV export 완료 상태 Needs Capture |
| 23 | Root Motion Tool | in-place Animation의 Body Motion을 Root로 옮기기 전에 입력을 수용하고 Root·direct root child track을 backup해 Apply·Revert 범위를 제한한다 | 요구조건·의도 → Root Motion Safety Contract 다이어그램 슬롯 → 동일 Asset Before·Apply·Revert 실사용 검증 슬롯 → 사용 범위 | `CAnimModifier_TransferBodyMotionToRoot`, Evidence Ledger | Needs Capture: 동일 Asset 3분할 Before·Apply·Revert 캡처 및 다이어그램 사용자 제작 예정 |
| 24 | AI Workflow | AI 제안·초안은 Work Brief와 개발자 수용 판단·검증 기록을 거쳐서만 반영하며, 미수행 검증은 성공으로 바꾸지 않고 남긴다 | 요구조건·의도 → AI-Assisted Work Control Flow 다이어그램 슬롯 → 동일 Work ID Brief·Review·Verification 실사용 검증 슬롯 → 사용 범위 | `Docs/08_AI_Workflow`, 동일 작업 식별자의 Brief·Review·Verification evidence slot | Needs Capture: 동일 Work ID 문서 보드 및 다이어그램 사용자 제작 예정 |
| 25 | Change Traceability | B05 문제 기록·D13 선행 기준·P12 변경 기록·과거 commit·현재 코드의 역할을 시점별로 대조해 Hit Window 순서 계약을 재확인한다 | 추적 요구조건 → Change Traceability Relation 다이어그램 슬롯 → B05/P12/commit/current code 2×2 증거 대조 → 추적 범위 | B05, `Docs/99_Legacy/Issue_CheckList/D13`, P12, commit `746ea548`, `ACWeaponActor::CollisionEnabled` | 문서·코드 관계 Ready / 사용자 제작 다이어그램 및 현행 first-overlap runtime capture Needs Capture |

## HTML A4 와이어프레임

- [UE5 Portfolio A4 Wireframe](UE5_Portfolio_A4_Wireframe.html)은 초기 22페이지 문서 흐름과 Word 이관 구조를 검토하기 위한 레이아웃 시안이다.
- [Submitted 23-page Portfolio HTML](염동섭_UE5_Portfolio_Project_Stellar.html)은 현재 제출 기준 문서다. 25페이지 전환안과 SVG 다이어그램 계획은 이 인덱스 및 관련 Audit Plan에서 역사적 제작 기록으로 보존한다.
- [Page Enrichment Backlog](01_Page_Enrichment_Backlog (KR).md)는 페이지별 하단 보조 증거 모듈과 필요한 증거를 관리한다.
- [Page Composition Application Plan](04_Portfolio_Page_Composition_Application_Plan (KR).md)은 p.4·p.5 파일럿 이후 p.6~p.25에 적용할 페이지 유형별 구성 원칙과 순서를 관리한다.
- [구 p.7·p.8 Project Audit and Composition Plan](05_P07_P08_Project_Audit_and_Composition_Plan (KR).md)은 분리 전 Dodge와 Balance / Collapse / Execution의 감사 이력을 보존한다. 현재 p.8·p.9 기준은 문서 20을 우선한다.
- [현행 p.10 Death Lifecycle Project Audit and Composition Plan](22_P10_Death_Lifecycle_Project_Audit_and_Composition_Plan (KR).md)은 Dead 전이, Presentation, Facing 억제와 Finalize/Destroy 경계를 관리한다.
- [현행 p.11 Execution Transition Project Audit and Composition Plan](06_P09_Shared_Execution_Project_Audit_and_Composition_Plan (KR).md)은 공통 전환 계약, terminal cleanup과 표현 제한을 관리한다.
- [현행 p.12 Intervention Policy Project Audit and Composition Plan](07_P10_Intervention_Policy_Project_Audit_and_Composition_Plan (KR).md)은 관계별 적용 규칙, Directive 조건과 구현 예외를 관리한다.
- [현행 p.13 Combat Signal Source Boundary Audit](08_P11_Combat_Signal_Project_Audit_and_Composition_Plan (KR).md)은 Source 사실 전달·packet trace의 책임과 표현 제한을 관리한다.
- [기존 C14 Data-driven Resolve Audit](09_P12_Data_Driven_Resolve_Project_Audit_and_Composition_Plan (KR).md)은 Key·Data·Context 경계의 역사·보조 근거를 보존하며, p.14 대표 Audit으로 사용하지 않는다.
- [현행 p.14 Target Outcome Resolution Audit](27_P14_Target_Outcome_Project_Audit_and_Composition_Plan (KR).md)은 Target Packet·Normal/Guard/Parry 결과 판정을 관리한다.
- [현행 p.15 Execution Pair Coordination Audit](28_P15_Execution_Pair_Project_Audit_and_Composition_Plan (KR).md)은 reservation·Active·Commit·Release 구조를 관리한다.
- [현행 p.16 AI Intent Project Audit and Composition Plan](10_P13_AI_Intent_Project_Audit_and_Composition_Plan (KR).md)은 Blackboard 판단 문맥, Intent State, 요청·종료 대기 경계를 관리한다.
- [현행 p.17 Combat Target Project Audit and Composition Plan](11_P14_Combat_Target_Project_Audit_and_Composition_Plan (KR).md)은 Combat Target 상태 권위, Revision·수명 계약, Consumer 투영 경계를 관리한다.
- [현행 p.18 Participation Project Audit and Composition Plan](12_P15_Participation_Project_Audit_and_Composition_Plan (KR).md)은 Evidence 기반 assignment, admission·release 범위를 관리한다.
- [현행 p.19 Profiling Project Audit and Composition Plan](13_P16_Profiling_Project_Audit_and_Composition_Plan (KR).md)은 80 Enemy Candidate Audit 원본·분석 계약, 1회 비교의 안전한 표현 범위를 관리한다.
- [현행 p.20 Runtime Debug Project Audit and Composition Plan](14_P17_Runtime_Debug_Project_Audit_and_Composition_Plan (KR).md)은 Canvas Overlay·World DrawDebug의 관찰 경계, FinalCandidate 증거 범위, 제출 표현 제한을 관리한다.
- [현행 p.21 Overlay Editor Plugin Project Audit and Composition Plan](15_P18_Overlay_Editor_Plugin_Project_Audit_and_Composition_Plan (KR).md)은 Editor-only bridge, CVar·command 위임, session-only·runtime 미소유 경계를 관리한다.
- [현행 p.22 Asset Inspector Project Audit and Composition Plan](16_P19_Asset_Inspector_Project_Audit_and_Composition_Plan (KR).md)은 Asset Registry 조사 흐름, CSV data lineage, 삭제 비판정과 참조·크기 해석 제한을 관리한다.
- [현행 p.23 Root Motion Tool Project Audit and Composition Plan](17_P20_Root_Motion_Tool_Project_Audit_and_Composition_Plan (KR).md)은 입력 수용 조건, track backup·Revert 계약, 필수 전후 evidence gate를 관리한다.
- [현행 p.24 AI Workflow Project Audit and Composition Plan](18_P21_AI_Workflow_Project_Audit_and_Composition_Plan (KR).md)은 AI 제안과 개발자 검토·미검증 기록의 책임 경계를 관리한다.
- [현행 p.25 Traceability Project Audit and Composition Plan](19_P22_Traceability_Project_Audit_and_Composition_Plan (KR).md)은 B05 변경 사례의 문서·현재 코드·검증 기록 대조와 Git 이력의 제한을 관리한다.
- [p.8 Balance / Collapse · p.9 Execution Collaboration Project Audit and Composition Plan](20_P08_Balance_Collapse_P09_Execution_Composition_Plan (KR).md)은 p.9 Gameplay 페이지의 사실·대표 사례·신규 캡처 경계를 관리한다.
- [Combat Interaction Architecture Documentation Plan](25_Combat_Interaction_Architecture_Documentation_Plan (KR).md)은 25페이지 전환과 구조 문서 기준을 관리한다.
- [Portfolio Capture Shot List](02_Portfolio_Capture_Shot_List (KR).md)와 [Portfolio Claim Freeze Log](03_Portfolio_Claim_Freeze_Log (KR).md)는 캡처와 제출 주장의 확정 조건을 관리한다.
- [Portfolio Production Session Handoff](23_Portfolio_Production_Session_Handoff (KR).md)는 다음 작업 세션이 읽어야 할 문서 순서, 25페이지 기준선, 증거 우선순위와 동기화 규칙을 관리한다.
- [Word Layout Handoff](UE5_Portfolio_Word_Layout_Handoff (KR).md)는 페이지 유형별 표 구조와 캡처 비율을 정의한다.
- 와이어프레임의 Placeholder는 증거 준비 영역을 표시할 뿐이며, 이 표의 Evidence Status와 성능 수치 확정 상태를 변경하지 않는다.

## 운영 메모

- p. 19는 수치와 측정 조건을 확정하기 전까지 본문 작성에 들어가지 않는다.
- p. 8은 Balance / Collapse 수명주기만 다룬다. p. 9 Execution은 제출용 영상·캡처가 준비되기 전까지 Pair 계약과 구현 범위 중심으로 표현하며, Stagger 2·3은 Execution 결과 증거로 대체하지 않는다.
- p. 4, p. 5, p. 7, p. 20은 신규 영상 또는 스크린샷 확보가 필요하다.
- p. 9의 기존 `Unified Execution Pipeline.png`은 상세 설계 자료이며, 포트폴리오용으로 단순화한 새 다이어그램을 제작한다.
