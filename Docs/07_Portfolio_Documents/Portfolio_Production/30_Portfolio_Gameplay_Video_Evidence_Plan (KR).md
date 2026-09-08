# Portfolio Gameplay Video Evidence Plan

> 상태: `Planned`
> 목적: 제출 문서가 정적으로 설명한 구조를, 실제 입력 → 상태 전이 → 관측값 → 결과의 영상 증거로 보완한다.
> 범위: 카테고리 영상 3개, 통합 대표 영상 1개, 재사용 가능한 Clean / Proof 원본 클립 카탈로그.
> 비범위: 영상 촬영·업로드, Issue #119 갱신, HTML 본문 변경, 새 기능 구현 및 성능 수치 주장.

## 1. 이 문서가 고정하는 것

이 문서는 **무엇을 어떤 근거로 보여줄지**를 소유한다. 실제 PIE 조작, Overlay preset, 재촬영 기준, 촬영 직후 기록은 [Runtime Capture and Video Runbook](29_Portfolio_Runtime_Capture_and_Video_Runbook%20%28KR%29.md)을 단일 기준으로 사용한다.

| 문서 / 위치 | 소유 책임 | 이 문서와의 관계 |
| --- | --- | --- |
| [제출 기준 23페이지 HTML](염동섭_UE5_Portfolio_Project_Stellar.html) | 정적 설계·캡처·검증 카드 | 영상이 보완할 동적 전이와 페이지 anchor를 제공 |
| [Runtime Capture and Video Runbook](29_Portfolio_Runtime_Capture_and_Video_Runbook%20%28KR%29.md) | PIE 환경, CVar preset, PASS·재촬영, take 기록 | Shot별 실제 촬영 절차는 반복하지 않고 참조 |
| [Issue #119](https://github.com/Razria93/Portfolio_UE5.4_verGit/issues/119) | 공개 영상·타임스탬프 안내 허브 | 촬영이 끝난 뒤에만 영상과 timestamp를 기록 |
| 이 문서 | 카테고리, Shot ID, 편집 순서, 자막, 페이지/근거 추적 | 촬영·편집 의사결정의 단일 기준 |

## 2. 영상 패키지 결정

초기 후보인 Movement, Targeting, Guard, AI, Debug, Tooling을 각각 독립 영상으로 분리하지 않는다. 짧은 면접 검토에서 기능 나열이 되는 위험이 더 크므로, 공개 영상은 아래 **3개 카테고리**로 압축한다. 세부 상태는 독립 Shot으로 촬영해 편집 재료와 심화 증거로 보존한다.

| 공개 산출물 | 목표 길이 | 질문 | 포함 범위 | 공개 조건 |
| --- | ---: | --- | --- | --- |
| `01_Combat_Execution` | 45–75초 | 새 요청은 기존 실행과 어떤 관계로 처리되는가? | Targeting, Combo, Guard / Block, Dodge. Parry·Execution은 재현 PASS 뒤 선택 삽입 | Clean 결과와 Proof 전이가 모두 PASS |
| `02_AI_Runtime_Observation` | 45–75초 | 같은 Enemy의 인지·의도·참여·행동을 어떻게 관찰하는가? | Perception → Chase / Engage, Focus Overlay, Participation. Patrol·Investigate·Return Home·Runtime LOD는 PASS 뒤 확장 | 각 상태가 실제 Overlay / World Debug에 관측됨 |
| `03_Editor_Tooling` | 45–75초 | 관찰·조사·복구를 반복 가능한 Editor 경로로 어떻게 만들었는가? | Overlay Editor, Asset Inspector. Root Motion은 Apply·Revert 동등성 PASS 뒤 포함 | 결과가 실제 UI / 파일 상태로 확인됨 |
| `00_Project_Stellar_Overview` | 2:00–2:30 | 구현 결과와 검증·도구화 흐름을 한 번에 이해할 수 있는가? | 위 3개 카테고리의 대표 Clean / Proof 컷 | 개별 영상의 PASS Clip만 조합 |

### 2.1 편집 원칙: Clean → Proof

동일 시나리오는 두 take로 분리한다.

| Take | 표시 | 역할 | 편집 규칙 |
| --- | --- | --- | --- |
| `Clean` | Overlay, Console, World Debug, Editor 창 OFF | 조작 결과·애니메이션·공간적 변화를 먼저 보인다 | 카테고리 도입과 Hero에 사용 |
| `Proof` | 장면 판정에 필요한 Overlay block / Event Log / World Debug만 ON | 같은 Actor·상태·이벤트가 실제로 관측됐음을 보인다 | Clean 뒤 3–8초만 삽입 |

모든 block을 켠 coverage baseline, 흐린 Event Log, `N/A` / `None` / `NotCaptured`, 다른 Actor가 Focus된 화면은 최종 편집에서 사용하지 않는다. terminal 상태나 Actor Destroy 뒤의 최근 이력은 다음 Shot의 근거로 재사용하지 않는다.

## 3. 편집 대상과 우선순위

### 3.1 Category 01 — Combat Execution

**한 줄 메시지**: `Action / Reaction 요청은 현재 실행과의 관계를 판정한 뒤 Start·Reserve·Intervene으로 적용된다.`

| Shot ID | 장면 / 시작 조건 | Clean / Proof | 최종 화면에서 확인할 것 | 자막 초안 | 문서·코드 근거 | 상태 |
| --- | --- | --- | --- | --- | --- | --- |
| C01 | 유효 Combat Target을 향한 Target Lock, 인접 Target 전환 | Clean 3–4초 + Proof 2초 | 카메라·이동 회전·Target 표시가 같은 Target을 소비 | `Target Lock | 공통 Target을 카메라·이동·표시에 반영` | [p.4 Targeting](염동섭_UE5_Portfolio_Project_Stellar.html#targeting) · [Target Lock 코드](../../../Source/Portfolio/Component/CTargetLockAssistComponent.cpp) | Capture candidate |
| C02 | Idle에서 Combo 시작, chain window 입력으로 다음 Combo 예약, Notify 후 hit | Clean 6–8초 + Proof 4–6초 | `Independent → Start`, `Sequential → Reserve`, Hit Window / Enemy Hit 결과 | `Combo | 다음 입력은 Chain Window에서 Reserve하고 Notify에서 소비` | [p.5 Combo / Hit](염동섭_UE5_Portfolio_Project_Stellar.html#combo-hit) · [Combo 코드](../../../Source/Portfolio/Action/CAction_ComboAttack.cpp) | Capture candidate |
| C03 | Guard In 상태에서 Enemy hit를 Block으로 수용 | Clean 4–6초 + Proof 4초 | Guard 상태, BlockHit Reaction, 실제 HP / damage 결과 | `Guard | 같은 hit도 Target의 방어 상태에 따라 Block으로 확정` | [p.6 Guard / Parry](염동섭_UE5_Portfolio_Project_Stellar.html#guard-parry) · [Guard 코드](../../../Source/Portfolio/Action/CAction_Guard.cpp) | Capture candidate |
| C04 | Idle 또는 Guard 중 Dodge 요청 | Clean 3–5초 + Proof 4초 | Idle Start 또는 Guard cleanup 뒤 Dodge Intervene | `Dodge | 현재 실행 관계를 통과할 때만 시작하거나 개입` | [p.7 Dodge Intervention](염동섭_UE5_Portfolio_Project_Stellar.html#dodge-intervention) · [Action Orchestrator](../../../Source/Portfolio/Component/CActionOrchestratorComponent.cpp) | Capture candidate |
| C05 | Guard In 중 incoming Parry Reaction | Proof 우선, Clean 2–3초 보조 | Guard 종료 뒤 Parry Reaction이 수락·개입한 순서 | `Parry | Guard의 활성 상태와 incoming Reaction을 함께 판정` | [p.6 Guard / Parry](염동섭_UE5_Portfolio_Project_Stellar.html#guard-parry) · [Parry 코드](../../../Source/Portfolio/Reaction/CReaction_Parry.cpp) | Optional — timing PASS 필요 |
| C06 | 유효 opportunity의 Execution Pair | Clean 4–6초 + Proof 5초 | Source / Target 예약, Active, Commit 또는 Release | `Execution | 두 Actor가 같은 Session 계약을 예약·확정` | [p.9 Execution](염동섭_UE5_Portfolio_Project_Stellar.html#execution-collaboration) · [Pair 코드](../../../Source/Portfolio/Component/CExecutionCollaborationComponent.cpp) | Optional — opportunity PASS 필요 |

`C02`와 `C04`가 Category 01의 최소 PASS Shot이다. `C05`, `C06`은 입력만으로 항상 재현된다는 인상을 주지 않으며, 데이터·타이밍·opportunity 조건을 실제로 통과한 take만 사용한다.

### 3.2 Category 02 — AI Runtime Observation

**한 줄 메시지**: `AI는 Perception·Participation·거리·Reaction 문맥을 Intent로 정리하고, 같은 실행 계약으로 요청하며, Focus된 Actor 기준으로 그 과정을 관찰한다.`

| Shot ID | 장면 / 시작 조건 | Clean / Proof | 최종 화면에서 확인할 것 | 자막 초안 | 문서·코드 근거 | 상태 |
| --- | --- | --- | --- | --- | --- | --- |
| A01 | Player가 sight / LOS 범위에 들어간 뒤 Enemy가 Target을 인지하고 Chase | Proof 5–8초 + Clean 3–5초 | 같은 Focused Enemy의 Target, `HasLOS`, Intent, 거리, 최근 AI Event | `Perception → Intent | 인지는 Target 후보가 되고, Intent가 다음 행동을 고른다` | [p.16 AI Intent](염동섭_UE5_Portfolio_Project_Stellar.html#ai-intent) · [AI Context Service](../../../Source/Portfolio/AI/BehaviorTree/Service/CBTService_UpdateAIContext.cpp) | Runtime PASS 필요 |
| A02 | Engage assignment와 action 가능 조건을 만족한 Enemy의 Combat Action | Proof 5초 + Clean 3–5초 | Assignment / Target / `IsCombatAction`, Action 시작 | `Engage | 참여 역할과 실행 가능 조건이 충족될 때 Action을 요청` | [p.18 Participation](염동섭_UE5_Portfolio_Project_Stellar.html#participation) · [Combat Action Task](../../../Source/Portfolio/AI/BehaviorTree/Task/CBTTask_StartCombatAction.cpp) | Runtime PASS 필요 |
| A03 | 2–3 Enemy가 같은 Target에 Perception 또는 HitReactive evidence를 보고 | Proof 8–10초 | World Summary, World Text / Ring, Focused Enemy role·evidence | `Participation | 인지는 곧 공격 권한이 아니며, 역할은 World Subsystem이 할당` | [p.18 Participation](염동섭_UE5_Portfolio_Project_Stellar.html#participation) · [Participation Subsystem](../../../Source/Portfolio/System/Combat/CWorldSubsystem_CombatParticipation.cpp) | Optional — multi-Enemy PASS 필요 |
| A04 | Patrol → Investigate → Return Home | Clean 5–8초 + Proof 5초 | `UsePatrol`, `IntentState`, `ReturnHome`, 마지막 관측 / timeout 뒤 전이 | `Patrol / Investigate / Return Home | 관찰 근거와 Home 거리로 복귀 문맥을 갱신` | [Current AI Overlay](../../../Source/Portfolio/Core/Debug/FDebugOverlayViewDataBuilder.cpp) · [Investigate Service](../../../Source/Portfolio/AI/BehaviorTree/Service/CBTService_UpdateInvestigateContext.cpp) | Optional — TestRoom BT·Nav PASS 필요 |
| A05 | Background / Awareness / Combat tier 전이 | Proof 전용 5초 | `Runtime LOD`와 동일 Actor의 Intent / role | `Runtime LOD | 상태 기반 갱신 정책을 관찰한다` | [Runtime LOD Resolver](../../../Source/Portfolio/AI/RuntimeLOD/CAIRuntimeLODTierResolver.cpp) | Optional — 성능 수치·개선 주장 금지 |

Category 02의 공개 최소 기준은 `A01`과 `A02`다. `A03`은 다수 Enemy에서만, `A04`는 실제 TestRoom의 Patrol Path·BT branch·Nav가 재현될 때만, `A05`는 tier 표시가 실제로 관측될 때만 편입한다. PASS가 없으면 해당 Shot을 영상에서 제외하고, 코드를 근거로 한 기대 동작을 관측 결과처럼 자막화하지 않는다.

### 3.3 Category 03 — Editor Tooling

**한 줄 메시지**: `반복되는 관찰·관계 조사·변환 작업은 결과를 바꾸는 Runtime 책임과 분리된 Editor 경로로 준비·기록·복구한다.`

| Shot ID | 장면 / 시작 조건 | Clean / Proof | 최종 화면에서 확인할 것 | 자막 초안 | 문서·코드 근거 | 상태 |
| --- | --- | --- | --- | --- | --- | --- |
| T01 | Nomad Panel에서 HUD / Event Log / Focus 조건을 준비한 뒤 PIE 반영 | Editor UI 6–8초 + PIE Proof 3초 | session-only CVar와 기존 PIE Focus command가 반영됨 | `Overlay Editor | session-only 표시 설정과 PIE Focus 요청을 준비` | [p.21 Overlay Plugin](염동섭_UE5_Portfolio_Project_Stellar.html#overlay-editor-plugin) · [Editor Module](../../../Plugins/PortfolioDebugOverlayEditor/Source/PortfolioDebugOverlayEditor/Private/PortfolioDebugOverlayEditorModule.cpp) | Capture candidate |
| T02 | Selected Asset 선택 → Dependencies / Referencers Tree → Browser Sync 또는 CSV Export | Editor UI 8–12초 | 조사 조건, Tree 결과, 기록 또는 Sync 결과 | `Asset Inspector | 관계를 조회하고 현재 결과를 기록한다` | [p.22 Asset Inspector](염동섭_UE5_Portfolio_Project_Stellar.html#asset-inspector) · [Inspector Widget](../../../Plugins/AssetReferenceInspector/Source/AssetReferenceInspector/Private/UI/SAssetReferenceInspectorWidget.cpp) | Capture candidate |
| T03 | Animation asset preflight → Apply → Revert | Editor UI 10–15초 | 입력 검증, Track backup, 같은 Asset의 Revert 복구 | `Root Motion Tool | Apply 전 Track을 백업하고 Revert에서 복구` | [p.23 Root Motion Tool](염동섭_UE5_Portfolio_Project_Stellar.html#root-motion-tool) · [Modifier 코드](../../../Source/PortfolioEditor/Private/Animation/CAnimModifier_TransferBodyMotionToRoot.cpp) | Optional — Revert PASS 필요 |

Asset Inspector의 unused candidate는 **삭제 대상이 아닌 후속 검토 후보**로만 표현한다. Root Motion은 현재 before/apply 캡처만으로 Revert 동등성을 증명하지 않으므로, `T03`은 같은 asset의 Apply와 Revert를 실제로 확인한 후에만 공개 영상에 사용한다.

## 4. 통합 대표 영상 Storyboard

통합본은 모든 기능의 목록이 아니라, **결과 → 실행 판단 → AI 관찰 → 도구화 → 근거 링크** 순서의 면접관용 진입점이다. 각 시간은 편집 목표이며 실제 PASS clip 길이에 맞춰 조정한다.

| 구간 | 목표 시간 | 사용 Shot | 화면 / 자막 |
| --- | ---: | --- | --- |
| Hook | 0:00–0:08 | `C02` 또는 Hero Clean | `Project Stellar | UE5.4 C++ Gameplay & Tooling` / 전투 결과를 먼저 표시 |
| Shared Combat | 0:08–0:42 | `C02`, `C03` 또는 `C04` | Clean 8–12초 후 동일 시나리오의 Proof. `Decision → Relationship → Apply` |
| AI & Runtime | 0:42–1:14 | `A01`, `A02`, 선택 `A03` | 같은 Focused Enemy의 Intent·Target·행동. AI PASS가 없으면 Combat Proof 연장으로 대체 |
| Editor Tooling | 1:14–1:48 | `T01`, `T02`, 선택 `T03` | Panel / Inspector의 before → action → result. Runtime 상태를 Editor가 소유하지 않음을 유지 |
| Evidence recap | 1:48–2:05 | 가장 읽기 좋은 `Proof` 1개 | 실제로 관측된 Focus / Action / Event 또는 Tool result 3개 이내 |
| End card | 2:05–2:15 | 정적 title | `Gameplay · Runtime Proof · Editor Tooling` / Pages URL / GitHub |

AI runtime PASS가 확보되지 않으면 통합본은 1:45–2:00으로 축소하고, `A01` / `A02` 영역을 정적 다이어그램이나 기대 동작으로 대체하지 않는다.

## 5. Shot 제작 계약

### 5.1 파일·편집 단위

```text
<ShotID>_CLEAN_T01.ext    예: C02_CLEAN_T01
<ShotID>_PROOF_T01.ext    예: C02_PROOF_T01
<ShotID>_NOTES.md         관측값 / PASS / 재촬영 사유
```

- 입력 전과 결과 후에 각각 약 2초의 handle을 남긴다.
- `C05`, `C06`, `A01`–`A05`, `T03`은 실패 take를 삭제 기준으로 삼지 않고 `Needs Runtime PASS` 상태로 기록한다.
- Destroy, terminal, Focus 교체 뒤에는 PIE 또는 대상 상태를 리셋한다.
- Shot 하나는 주장 하나만 가진다. 다른 상태의 Event Log를 보조 근거로 재사용하지 않는다.

### 5.2 Capture profile

| Profile | 사용 Shot | 필요한 표시 | 금지 |
| --- | --- | --- | --- |
| `Clean Gameplay` | `C01`–`C06`, `A01`, `A02`, `A04` | 실제 게임플레이 UI만 | Overlay, World Debug, Console, Editor 창 |
| `Combat Proof` | `C02`–`C06` | Player / Enemy의 관련 Details, 1개 Event Log filter | 모든 Details block, 무관한 Event Log |
| `AI Proof` | `A01`–`A05` | Enemy Focus, Current AI, Recent AI Event; Participation은 World Summary / Ring | 다른 Actor Focus, 관측되지 않은 Revision / Reason 자막 |
| `Editor Proof` | `T01`–`T03` | 선택 UI와 그 결과만 | config 영속 저장처럼 보이는 연출, unrelated Content Browser |

정확한 CVar, Event Log filter, Character Details, World Debug 설정은 Runbook의 §4–§6을 사용한다. 특히 Targeting / Participation은 Event Log가 아닌 전용 Details / World Debug를 근거로 한다.

## 6. 자막 규칙

1. 첫 줄은 기능 이름이 아니라 **판단 또는 상태 전이**를 말한다.
2. 둘째 줄에는 이 take에서 관측된 state identifier를 그대로 쓴다. 예: `Decision: Accept | Apply: Reserve`.
3. `Observed`가 없는 값, 성능 수치, "항상", "완성", "자동 최적화", "모든 AI" 같은 단정은 쓰지 않는다.
4. 한국어 설명을 기본으로 하고 `FocusActor`, `IntentState`, `HasLOS`, `Start`, `Reserve`, `Intervene` 등 코드·Overlay 식별자는 영어 표기를 유지한다.
5. 한 컷당 최대 두 줄, 두 번째 줄은 실제 화면에서 읽히는 값만 넣는다.

| 용도 | 확정 전 자막 템플릿 |
| --- | --- |
| Combat Proof | `Decision → Relationship → Apply`<br>`[Observed Decision] · [Observed Relationship] · [Observed Apply]` |
| AI Proof | `Perception → Intent → Common Execution`<br>`FocusActor: [Observed Actor] · Intent: [Observed State]` |
| Participation Proof | `Evidence는 공격 권한이 아니다`<br>`Assignment: [Observed Role] · Target: [Observed Target]` |
| Overlay Editor | `session-only 표시 설정과 PIE Focus 요청`<br>`CVar / Command 결과는 현재 session에서만 사용` |
| Asset Inspector | `관계를 조회하고 현재 결과를 기록`<br>`Dependencies / Referencers · CSV Export` |
| Root Motion | `Apply 전 검증·백업, Revert에서 복구`<br>`[Observed Track / Revert result]` |

## 7. 문서·영상 증거 매핑

| 페이지 / 주장 | 영상 Shot | 정적 자산 재사용 | 영상에서만 확인할 동적 사실 | 제한 |
| --- | --- | --- | --- | --- |
| p.4 Targeting | `C01` | `p04_targeting.png` | acquire / switch / release와 소비자 반영 | Revision / Change Reason은 직접 표시 전까지 주장하지 않음 |
| p.5 Combo / Hit | `C02` | `p05_combo_hit.png` | chain reserve, Notify, Hit Window, first accepted overlap | Collision filter는 Hit Window take에서만 |
| p.6 Guard / Parry | `C03`, `C05` | `p06_guard_parry.png` | 같은 hit의 Block / Parry / PlayerHit 분기 | Parry는 별도 버튼이 아님 |
| p.7 Dodge | `C04` | `p07_dodge.png` | Idle / active relationship에 따른 Start / Intervene | 무적 frame 수치 주장 금지 |
| p.8–10 Lifecycle | `C06` 선택 | `p08_*`, `p09_execution.png`, `p10_*` | Collapse·Execution·Death의 연속 전이 | 각 terminal 뒤 state reset |
| p.16–18 AI | `A01`–`A05` | `p16_ai_intent.png`, `p18_participation.png` | intent, action request, assignment, role transition | Runtime PASS 전에는 성공 검증으로 표현 금지 |
| p.19 Profiling | 없음 | HTML chart | 없음 | Hero / 일반 gameplay 영상에 수치 삽입 금지 |
| p.20–21 Debug / Overlay | `A01`, `T01` | `p20_*`, `p21_*` | Focus / ViewData 관찰과 Editor→PIE bridge | Overlay는 read-only, setting은 session-only |
| p.22–23 Tools | `T02`, `T03` | `p22_*`, `p23_*` | 관계 조사 / CSV, Apply / Revert | unused ≠ delete, Revert PASS 전 공개 금지 |

## 8. 촬영·편집 전 최종 체크리스트

### Preflight

- [ ] TestRoom PIE, 대상 Actor, Nav / Patrol Path, 필요 Enemy 수를 장면별로 확인했다.
- [ ] FocusActor가 의도한 Enemy와 일치한다.
- [ ] Shot별 Clean / Proof preset을 Runbook 기준으로 전환했다.
- [ ] Console, tooltip, Editor 창, 겹친 Overlay가 최종 화면을 가리지 않는다.
- [ ] terminal 장면 뒤 리셋 경로를 준비했다.

### Proof validity

- [ ] Shot 하나가 주장 하나만 증명한다.
- [ ] 화면에 `N/A`, `None`, `NotCaptured`가 남지 않는다.
- [ ] Event Log filter / scope와 World Debug가 Shot의 주장에 맞는다.
- [ ] 실제 관측값 3–5개, 파일명, PASS 여부, timestamp를 Runbook 기록표에 남겼다.
- [ ] AI / Participation / Runtime LOD는 코드 기대값이 아니라 실제 PASS 화면을 근거로 한다.

### Edit and publish

- [ ] 통합 영상 첫 10초 안에 Clean gameplay 결과가 보인다.
- [ ] Proof 컷은 결과 설명에 필요한 기간만 사용한다.
- [ ] 각 자막은 코드·문서·관측 중 하나 이상의 근거를 가진다.
- [ ] Issue #119에는 실제 업로드 파일, 길이, timestamp, 해당 Portfolio 페이지 링크만 추가한다.
- [ ] README / Pages / Issue #119 링크가 동일한 영상 안내 경로를 가리킨다.

## 9. 완료 기준

- 공개 카테고리 3개와 통합 영상은 이 문서의 PASS Shot만으로 편집 가능하다.
- 모든 최종 Shot은 Page / claim / code or document / capture preset / subtitle / timestamp를 추적할 수 있다.
- 문서로 정적 표현이 어려웠던 AI 상태 전이, Action 관계, Focus 기반 관찰은 실제 관측 take가 있을 때만 영상 증거가 된다.
- 기존 Runbook의 운영 절차와 이 문서의 편집·추적 책임이 중복되지 않는다.
