# Portfolio Case Evidence & Page Assignment Plan

> 상태: **Audit Complete / 25페이지 HTML·목차 전환 완료 / 제출용 런타임 캡처 대기**  
> 작성일: 2026-09-05  
> 목적: 구조·설계 개선 및 트러블슈팅 사례를 현재 코드·문서·Git 이력으로 대조하고, 하나의 사례가 하나의 포트폴리오 페이지를 대표하도록 배정한다.  
> 범위: 이 문서는 사례와 증거의 배치 계획을 다룬다. 2026-09-06 기준으로 p.15 Execution Pair 추가, HTML·목차·footer의 25페이지 순연을 동기화했다.

---

## 1. 감사 기준

### 1.1 채택 기준

대표 사례는 아래 네 조건 중 최소 세 가지를 만족해야 한다.

1. 발생 조건 또는 설계상 해결해야 할 충돌이 구체적이다.
2. 현재 `main` 코드에 해결 구조가 남아 있다.
3. Bug Report, Architecture, PR, Git commit 중 하나 이상이 변경 이유를 뒷받침한다.
4. 런타임 결과·PIE·로그·캡처 중 적어도 하나가 존재하거나, 없을 경우 설계·구현 범위로 제한해 안전하게 설명할 수 있다.

### 1.2 판정 표기

| 표기 | 의미 |
| --- | --- |
| **대표** | 해당 페이지의 핵심 결론을 문제·판단·변경으로 증명한다. 원칙적으로 한 페이지에만 배정한다. |
| **보조** | 대표 사례의 책임·안전성·검증 범위를 보완한다. 독립 중심 서사로 사용하지 않는다. |
| **구조 중심** | 실제 장애보다 책임 분리 또는 안전 계약이 핵심이다. 문제 해결처럼 과장하지 않는다. |
| **보류** | 코드 또는 문서는 있으나 런타임 결과·필수 캡처가 없어 결과 주장에 제한이 있다. |
| **면접 보조** | 기술적으로 유효하지만 해당 포트폴리오 페이지의 중심 메시지를 흐릴 가능성이 높다. |

### 1.3 현행 포트폴리오 범위

- 현행 기준은 **총 25페이지**이며 HTML·목차·footer도 동기화됐다.
- p.8 Balance / Collapse, p.9 Execution Collaboration(게임플레이), p.15 Execution Pair Coordination(구조)은 각각 별도 페이지다.
- p.15 Execution Pair 추가에 따라 기존 문서상 p.15~p.24는 p.16~p.25로 이동한다.

---

## 2. 전체 사례 원장

### 2.1 Gameplay · Combat Core

| ID | 사례 | 분류 / 판정 | 현재 근거 | Git 근거 | 런타임 증거 | 주 페이지 | 안전한 주장 / 피할 주장 |
| --- | --- | --- | --- | --- | --- | --- | --- |
| C01 | Player 입력 기반 선택과 AI 인지 기반 대상 생산자 분리 | 구조 중심 / 대표 | `CPlayerTargetSelectionComponent`, `CCombatTargetComponent`, `CTargetLockAssistComponent`; p.4 명세 | 별도 단일 bug commit 대신 현재 책임 분리 구조 | 신규 Player Lock-on 캡처 필요 | p.4 Targeting | **안전:** Player와 AI가 다른 근거로 대상 요청을 만들고 공통 대상 상태를 소비한다. **피함:** 하나의 API가 모든 대상 결정을 해결한다고 표현. |
| C02 | 첫 overlap 이전 HitWindow 준비 | 트러블슈팅 / 대표 | `Source/Portfolio/Weapon/CWeaponActor.cpp`의 collision 후보 수집 → `CurrentHitWindowId` 준비 → collision enable; B05 | `746ea548` | B05에 Player·Enemy 반복 log 검증 기록. 제출용 전후 capture는 필요 | p.5 Combo / Hit | **안전:** collision 활성화 전에 유효 HitWindow를 준비해 첫 타 `InvalidRequest`를 방지했다. **피함:** raw evidence 없이 오류가 0회가 되었다고 단정. |
| C03 | Guard Hold의 실행·상태 분리 | 구조 중심 / 보조 | `CAction_Guard`, `UCDefenseComponent`, `UCObservableOverlayComponent`, Guard snapshot | `accae471`, `5154a6de`, `111ecddc` 계열 | B11/B12 및 FinalCandidate 보조 | p.6 Guard / Parry | **안전:** Guard In/Out은 Action, 지속 pose·판정은 Overlay/Defense 상태로 분리했다. **피함:** Defense만이 모든 Overlay 상태를 단독 소유한다고 표현. |
| C04 | Guard Out 중 Hit reaction 시작 실패 | 트러블슈팅 / **대표** | `UCDefenseComponent`의 idempotent clear, `UCObservableOverlayComponent`의 dirty registry refresh; B12 | `d5919103`, `1ec1dd73`, `2cca03d7` | B12: build 및 PIE에서 GuardOut→Hit reaction 확인 | p.6 Guard / Parry | **안전:** 이미 정리된 Guard 상태에도 `ClearGuardState`가 성공하도록 하여 Hit reaction 시작을 막지 않게 했다. **피함:** 모든 Overlay operation이 무조건 idempotent하다고 일반화. |
| C05 | Guard In 중 release 유실 | 트러블슈팅 / 보조 | `CAction_Guard.cpp`의 deferred consume key, `CActionOrchestratorComponent`의 candidate 재처리; B11 | `73aa0e87`, `92e706a2` | B11의 완료 기록은 있으나 제출용 독립 capture 없음 | p.6 Guard / Parry | **안전:** Guard In 중 Out 후보를 보관한 뒤 공통 candidate 경로로 재평가하도록 설계했다. **피함:** 모든 release UX가 최종 캡처로 검증됐다고 표현. |
| C06 | Dodge의 개입 적용과 Guard 상태 정리 | 구조 중심 / 보조 | `CAction_Dodge.cpp`의 relationship 판단·`ClearGuardState` handling·Want intervention | `56648669` 및 B09 계열 | 신규 PIE capture 필요 | p.7 Dodge Intervention | **안전:** Dodge는 active 실행과의 관계·허용 조건을 확인한 뒤 시작하며 Guard 상태를 정리할 수 있다. **피함:** 독립적인 Dodge bug를 해결했다고 표현. |
| C07 | Balance / Collapse lifecycle ownership | 구조 개선 / 대표 | `CBalanceComponent.cpp`: lifecycle state, serial, TTL, Reset, reservation; S35 | `f29f91f9`, `46a24934`, `eb4ea442`, `53c8adde` | S35의 PIE 검증 및 Stack 2·3 FinalCandidate. 전이/TTL/Reset capture 필요 | p.8 Balance / Collapse | **안전:** Count·상태·Timer·Reset을 하나의 lifecycle로 소유하고 serial로 오래된 입력을 구분한다. **피함:** 모든 stale timer 사례가 영상으로 완전 검증됐다고 표현. |
| C08 | Execution pair transaction과 HP=1 preflight | 구조 개선 / 대표(보류) | `CExecutionCollaborationComponent`, `TryResolveExecutionAppliedDamage`, S36 | `821acbea`, `74415372`, `c7580883` | 구조는 구현됨. Pair session/HP=1 제출용 capture 필요 | p.9 Execution Collaboration | **안전:** Source·Target이 Reserve→Activate→Commit/Release 계약을 공유하며 Standard 피해가 0이면 예약 전 거절하도록 구현했다. **피함:** full execution 회귀 또는 HP=1 PIE 결과가 캡처됐다고 표현. |
| C09 | AI Combo 1타 반복 | 트러블슈팅 / 대표 | `CAction_ComboAttack` action event → `ActionComponent` → `ACEnemy` 재요청 경로; B07 | `6a218760` | B07: AI 0→1→2, Player·AI 공통 path 확인 | p.11 Shared Execution | **안전:** AI가 별도 콤보 판정을 만들지 않고 Player와 같은 Action-driven chain 경로를 재사용한다. **피함:** Behavior Tree가 콤보 index를 직접 결정한다고 표현. |
| C10 | Want / Allow 책임 분리 | 트러블슈팅·구조 개선 / 대표 | Action/Reaction Orchestrator, Want·Allow rules, allow window API; B09 | `659cb37e`, `168d7cfb`, `4b2c0b97` | B09에 Action/Reaction interrupt 결과 기록 | p.12 Intervention Policy | **안전:** 일반 개입은 incoming Want와 active Allow, 필요한 경우 Allow Window timing을 함께 평가한다. **피함:** Dead force policy 같은 예외까지 항상 Want∧Allow라고 표현하거나 Notify에 정책이 전혀 없다고 단정. |
| C11 | interrupt 뒤 Trail·Collision·Hit Context 잔존 | 트러블슈팅 / 대표(결과 보류) | `CAction.cpp`, `CReaction.cpp`: payload capture → montage stop → `CleanupRuntimeEffects` → `ClearRuntime` → feedback; B10 | `29c8b5d1` | B10은 build/diff 검증이 명확하나 제출용 PIE capture는 필요 | p.13 Combat Signal | **안전:** Notify End에 의존하던 외부 runtime effect 정리를 executor terminal path로 이동했다. **피함:** 제출 증거 없이 모든 interrupt 뒤 잔존 문제가 실측상 제거됐다고 표현. |
| C12 | Combat Signal Source / Target 및 Defensive Outcome | 구조 중심 / 보조 | `UCCombatSignalSourceComponent`, `UCCombatSignalTargetComponent`, `EDamageDefenseOutcome`, target packet | `7f501417`, `13cddd37`, `cb0d4054` | Guard/Parry FinalCandidate와 B12 보조 | p.13 Combat Signal | **안전:** 명중 증거, 방어 결과, 피해 commit, reaction·feedback 소비를 Source/Target 경계로 분리했다. **피함:** 이 경계만으로 모든 전투 확장을 이미 지원한다고 표현. |
| C13 | Blueprint native component reference recovery | 트러블슈팅 / 보조·면접 보조 | Player/Enemy `PostInitializeComponents → RecoverReferences → BuildReferences → InjectReferences`; B13/B14 | `f7176318`, `35b9495c` | B14 build·PIE·recovery log 확인 | p.13 하단 또는 p.24 보조 | **안전:** rename·serialized reference 불일치에 대해 recovery와 DI 초기화 순서로 방어했다. **피함:** `FindComponentByClass`를 일반 dependency wiring으로 사용한다고 표현. |
| C14 | Data-driven Resolve boundary | 구조 중심 / 대표 | Action/Reaction DataKey·Data·Context, p.14 명세 | 관련 orchestration refactor | 별도 확장 사례 미확정 | p.14 Data-Driven Resolve | **안전:** 변화 지점을 Key·Data·Context 해석 단계로 분리했다. **피함:** 실제 트러블슈팅이나 확장 성과처럼 꾸밈. |

### 2.2 AI · Target · Participation · Death

| ID | 사례 | 분류 / 판정 | 현재 근거 | Git 근거 | 런타임 증거 | 주 페이지 | 안전한 주장 / 피할 주장 |
| --- | --- | --- | --- | --- | --- | --- | --- |
| A01 | Blackboard stale target | 트러블슈팅 / 대표 | `CBTService_UpdateAIContext.cpp`의 Success / NoData / Error별 Set·Clear; B03 | AI context branch 및 P10 | B03: Investigate→Idle 반복, 재인식 확인 | p.15 AI Intent | **안전:** context 실패를 early return하지 않고 결과 종류별 Blackboard projection을 명시적으로 정리했다. **피함:** 모든 AI 오류를 자동 복구한다고 표현. |
| A02 | Combat Target Snapshot·Revision·EndPlay | 구조 개선 / 대표 | `CCombatTargetComponent.cpp`의 weak target·revision·EndPlay binding·conditional clear, Participation/Enemy 재검증 | `b799d395`, `12829330`, `0fc0b65b` | 코드·문서 강함, dedicated capture 필요 | p.16 Combat Target | **안전:** 대상 상태의 identity·revision·수명을 단일 상태로 관리하고 consumer는 snapshot/event를 재검증한다. **피함:** stale callback 결함이 전혀 없음을 실측으로 보장한다고 표현. |
| A03 | Evidence 기반 참여 배정 | 구조 중심 / 대표(보류) | `CWorldSubsystem_CombatParticipation`, `CEnemyCombatParticipationComponent`, S34 | participation authority commits | 실제 3 Enemy admission/release capture 필요 | p.17 Participation | **안전:** participant×target evidence를 집계해 role을 재평가한다. **피함:** 다수 AI 성능 또는 모든 edge case를 검증 완료했다고 표현. |
| A04 | Dead 중 Combat Target Facing 재적용 차단 | 트러블슈팅 / 대표 | `CEnemyCombatTargetFacingComponent.cpp`: Dead event에서 deferred sync 취소, Gameplay Focus clear, OrientToMovement, resolve 시 suppression 재검사; S31 | `4ed7f45c`, `4757c07e`, `814b3cac` | S31의 PIE 기록, Facing 전후 전용 capture 필요 | p.10 Death Lifecycle | **안전:** Dead여도 남아 있을 수 있는 target state를 Facing 입력으로 소비하지 않고, 늦은 deferred sync의 재적용도 차단한다. **피함:** target 자체가 항상 즉시 삭제된다고 표현. |
| A05 | Death presentation·finalize·destroy 책임 분리 | 구조 개선 / 보조 | `ACEnemy::BeginDeathPresentation`, `RequestFinalizeDeath`, `FinalizeDeath`, cleanup; S31 | `4582dbe4`, `72e45573` | S31에 PIE 기록, 제출용 lifecycle capture 필요 | p.10 Death Lifecycle | **안전:** Health Dead 전이와 Presentation 완료·fallback·next-tick finalize·destroy를 분리했다. **피함:** 모든 death path의 최종 영상 검증이 끝났다고 표현. |

### 2.3 Debug · Tooling · Workflow

| ID | 사례 | 분류 / 판정 | 현재 근거 | Git 근거 | 런타임 증거 | 주 페이지 | 안전한 주장 / 피할 주장 |
| --- | --- | --- | --- | --- | --- | --- | --- |
| D01 | Debug Focus·Actor scoped history | 구조 개선 / 대표 | `CDebugOverlayFocusComponent`, Resolver, SnapshotStore, ViewData/Text/Canvas Renderer, actor history ring | `9507f487`, `4ed7f45c`, `c8882040` | Current AI FinalCandidate Ready / Focused Enemy 통합 화면 Needs Capture | p.20 Runtime Debug | **안전:** gameplay target과 debug focus를 구분하고, 읽기 전용 snapshot·actor history로 원인 흐름을 관찰한다. **피함:** Overlay가 game UI·shipping HUD이거나 AI/LOD 성능을 보장한다고 표현. |
| D02 | Editor-only CVar·PIE command bridge | 구조 중심 / 대표 | `PortfolioDebugOverlayEditor` module, Slate widget, CVar access, PIE focus command bridge | `a3a89af8`, `54a49bae` | Panel UI Ready / 현행 p.21 실사용 화면 Needs Capture | p.21 Overlay Editor Plugin | **안전:** Editor-only panel이 CVar와 기존 PIE command를 연결하되 runtime state는 소유하지 않는다. **피함:** 현재 CVar pointer cache 또는 호출 수 성능 개선을 주장. 현재 `FPortfolioDebugOverlayEditorCVarAccess.cpp`는 access마다 lookup한다. |
| D03 | Asset Registry readiness와 Unused Candidate 제한 | 구조 개선 / 대표 | Asset Inspector widget의 `SearchAllAssets(true)`와 `WaitForCompletion`, tree/CSV | `f9d97d42` | Panel UI Ready / Selected Asset·Tree·CSV 실사용 화면 Needs Capture | p.22 Asset Inspector | **안전:** registry 준비 후 참조 관계를 조사하고 zero referencer를 삭제 결정이 아닌 검토 후보로 남긴다. **피함:** dynamic/string/Asset Manager 참조까지 완전 추적하거나 삭제 안전성을 보장한다고 표현. |
| D04 | Root Motion Apply/Revert contract | 구조 중심 / 대표(보류) | `CAnimModifier_TransferBodyMotionToRoot`: validation·backup·apply·direct child compensation·revert | `6ecf78cb` | Evidence Ledger: Needs Capture | p.23 Root Motion Tool | **안전:** raw track 수정 전 backup과 Revert 경로를 분리한 editor safety contract을 구현했다. **피함:** visual preservation·transform 결과가 검증됐다고 표현. |
| D05 | AI assisted workflow | 구조 중심 / 대표 | Work Brief·prompt·review·verification 문서 체계 | Docs/08_AI_Workflow 및 Git/Docs 연결 | 동일 Work ID Brief·Review / Decision·Verification Record Needs Capture | p.24 AI Workflow | **안전:** AI 초안은 Work Brief·개발자 판단·검증 기록을 거쳐 반영한다. **피함:** AI가 구현을 완성하거나 검증을 대체했다고 표현. |
| D06 | B05 change traceability | 구조 중심 / 대표 | D13 Pipeline baseline + B05 symptom → P12 internal change record → historical commit `746ea548` → current `CWeaponActor` sequence contract | `746ea548`, P12 내부 변경 기록 | 문서·코드 관계 Ready, first-overlap 제출 capture 및 다이어그램 필요 | p.25 Change Traceability | **안전:** 기록의 시점·역할과 현재 Hit Window 순서 계약을 대조한다. **피함:** D13을 B05 후속으로, 과거 `CAttachment`를 현재 `CWeaponActor`와 동일 구현으로, historical record를 현재 runtime 성공으로 표현. |

---

## 3. 페이지별 최종 배치안 — 현행 25페이지

> 원칙: 대표 사례가 없는 페이지는 부족한 페이지가 아니다. 구조 자체가 메시지인 페이지에 억지로 트러블슈팅을 넣지 않는다.

| p. | 페이지 | 최종 역할 | 대표 사례 | 중심 시각 자료 | 하단 보조 증거 / 제한 |
| ---: | --- | --- | --- | --- | --- |
| 1 | Cover | 프로젝트를 읽는 기준 제시 | 사례를 배정하지 않음 | gameplay hero + 구조·측정·기록 3축 | GitHub / Docs / Video 링크 |
| 2 | Index | 문서 탐색 | 사례를 배정하지 않음 | 섹션 인포그래픽 | 별도 하단 사례 불필요 |
| 3 | System Map | 전체 책임 관계 지도 | 사례를 배정하지 않음 | 의도 생성 → 해석 → 결과 반영 → 관찰 | Combat Target 관리 흐름을 보조로만 사용 |
| 4 | Targeting | Player 대상 선택·적용 | C01 | Lock-on hero + 요청→후보→적용 흐름 | 2~4개 runtime state slot; AI producer와 혼동하지 않음 |
| 5 | Combo / Hit | Hit Window의 순서 계약 | **C02 / B05** | gameplay hero + HitWindow 준비→collision enable→first overlap | Event/Overlay 전후 capture 필요 |
| 6 | Guard / Parry | 방어 결과·상태 정리 | **C04 / B12** | Guard·Parry·Player Hit 비교 | C03/C05를 상태·deferred 설계 카드로 보조 |
| 7 | Dodge Intervention | 공통 개입 정책의 적용 | C06 | Action/Guard→Dodge hero + decision 결과 | B09의 Want/Allow 원리는 반복하지 않고 결과만 표시 |
| 8 | Balance / Collapse | lifecycle 권한 | **C07** | accumulate→collapse→loop→reset lifecycle | serial·TTL·reset capture 필요 |
| 9 | Execution Collaboration | pair transaction 계약 | **C08** | Reserve→Activate→Commit/Release | HP=1 preflight은 capture 전 설계·구현 범위로 제한 |
| 10 | Death Lifecycle | Dead 이후 lifecycle 경계 | **A04** / A05 보조 | Dead→Cleanup→Presentation→Finalize/Destroy + Facing suppression | Dead-facing / finalize runtime capture 필요 |
| 11 | Shared Execution | Player/AI 공통 실행 계약 | **C09 / B07** | Player·AI·Reaction 2-lane callback map | AI Combo 0→1→2 capture 권장 |
| 12 | Intervention Policy | 개입 판단의 책임 분리 | **C10 / B09** | Want ∧ Allow (+ Window) policy diagram | Dead force policy 및 전용 override 예외 표기 |
| 13 | Combat Signal | signal·cleanup·consumer 경계 | **C11 / B10** | Source→Resolve→Packet→Consumer + terminal cleanup sequence | C12, C13을 작은 보조 카드로만 사용; interrupt PIE capture 필요 |
| 14 | Data-Driven Resolve | Key·Data·Context 해석 구조 | C14 | Action/Reaction resolve map | 구조 페이지이며 사례 강제 금지 |
| 15 | AI Intent | 판단·요청·대기 분리 | **A01 / B03** | Blackboard→Intent→Task→Execute/Wait | Success/NoData/Error별 Set/Clear |
| 16 | Combat Target | SoT·Revision·lifetime | **A02** | Policy→Target SoT→Consumer + revision lifecycle | p.4 후보 평가와 중복 금지; lifecycle capture 필요 |
| 17 | Participation | evidence 기반 role assignment | A03 | Evidence→Candidate→Assignment→Role | 3 Enemy admission/release capture 필요 |
| 19 | Profiling | 동일 조건의 후보 구성 원인 분석 | 80 Enemy Candidate Audit | First Valid Target p95와 Raw/Invalid/Map Before·After 비교 | 1회 비교만 사용하며 전체 FrameTime 개선·반복 재현성 주장 금지 |
| 20 | Runtime Debug | read-only 관찰 구조 | **D01** | 요구조건 → 관찰 경계 → Focused Enemy 통합 실사용 검증 | capture state·focus 의미·actor history를 설명 |
| 21 | Overlay Editor Plugin | Editor↔PIE bridge | **D02** | 요구조건 → Editor Control Bridge → 실사용 검증 | editor-only, session-only, CVar cache 성능 주장 금지 |
| 22 | Asset Inspector | 조사 도구의 안전한 해석 | **D03** | 요구조건 → Selected Asset·Options→Registry→Tree→CSV → 실사용 검증 | `Unused Candidate ≠ 삭제 판정`; 실제 Tree·CSV 화면 확보 전 사용성 주장 금지 |
| 23 | Root Motion Tool | Apply/Revert safety contract | **D04** | Source→Apply→Revert | 전후 capture 전에는 구현 범위로 제한 |
| 24 | AI Workflow | AI 활용의 검증 책임 | **D05** | 요구조건 → Work Control Flow → 동일 Work ID 실사용 검증 | 실제 동일 Work ID Brief·Review·Verification evidence 연결 필요 |
| 25 | Change Traceability | 변경 추적성 | **D06 / B05 재사용** | D13 기준 + B05 증상 → P12 → historical commit → current code contract → runtime evidence 상태 | p.5의 기술 해결을 반복하지 않고, 시점·역할·현재 코드 대조와 증거 경계만 설명 |

---

## 4. Death Lifecycle 독립 페이지 — 적용 완료

### 4.1 권장 결론

> **Health의 Dead 전이 이후에도 Presentation·Facing 억제·Finalize·Destroy는 별도 수명주기로 관리한다.**

### 4.2 대표 사례

```text
Health Dead 전이
→ Deferred Facing Sync 취소
→ AI Gameplay Focus 해제
→ Rotation = OrientToMovement
→ Facing = Suppressed(Dead)
→ Presentation 완료 또는 fallback
→ RequestFinalizeDeath
→ 다음 Tick Finalize / Cleanup / Destroy
```

- Combat Target 데이터가 남아 있을 수 있어도, Dead 상태에서는 Facing 입력으로 소비하지 않게 한다.
- deferred reaction callback이 다음 Tick에 Focus/Facing을 되살리지 않도록 취소와 resolve 시점 재검사를 모두 둔다.
- Death target 정리, participation 해제, presentation 종료, final destroy는 같은 책임으로 뭉개지지 않는다.

### 4.3 적용 결과

**독립 페이지로 적용했다.** A04/A05는 p.9 Execution, p.16 Combat Target, p.19 Runtime Debug에 넣으면 해당 페이지의 중심 메시지를 흐리므로 p.10에서 다룬다.

```text
p.10 Death Lifecycle 추가
기존 p.10~p.23 → p.11~p.24 이동
총 24페이지
```

Production Master Plan, Page Spec Index, Evidence Capture Ledger, Analytical A4 Wireframe과 HTML Index/Footer를 24페이지 기준으로 갱신했다. Capture Shot List와 Claim Freeze Log는 Death 전용 증거 확보 단계에서 함께 갱신한다.

### 4.4 필수 증거

- Dead 전: Combat Target 존재, Gameplay Focus, `ControllerDesired` 회전
- Dead 후: Focus clear, `OrientToMovement`, `Suppressed(Dead)`
- deferred sync가 queued 되었더라도 Dead 후 재적용되지 않는 event history 또는 debug record
- Presentation 완료 또는 fallback → Finalize → Destroy 중 한 정상 경로

전용 capture 전에는 “회전하지 않음을 영상으로 검증했다”가 아니라, **Dead-facing 억제 구조를 구현했고 S31의 PIE 기록을 보유한다**로 제한한다.

---

## 5. 페이지 유형별 기재 형식

| 사례 성격 | 본문 중심 시각 자료 | 하단 보조 증거 |
| --- | --- | --- |
| 순서·입력 문제 (B05, B11) | 3~4단계 이벤트 흐름 또는 game frame | before/after event trace 한 줄 |
| 정책 충돌 (B09, B12) | 책임 분리·조건 매트릭스 | 실제 상태/결과 1개 또는 제한 사항 |
| lifecycle·비동기 (Balance, Execution, Death, CombatTarget) | 상태 타임라인 또는 session diagram | stale input 차단 지점, capture status |
| 데이터 계약 (Combat Signal, Data Resolve) | Source→Resolve→Result map | packet의 핵심 필드·consumer 분리 카드 |
| 도구 | 입력→처리→출력 흐름 | 안전 한계 또는 사용 장면 |
| Workflow | Issue/Brief→설계→구현→검증→PR | 실제 문서/commit 연결 1개 |

---

## 6. 용어·주장 동결 규칙

| 항목 | 현재 기준 | 사용 금지 또는 주의 |
| --- | --- | --- |
| 실행 중단 결과 | `Interrupted` / `Ignored` | 과거 문서의 `Cancelled`, `Intervened`를 현재 코드 표현으로 쓰지 않는다. |
| Guard deferred consume key | `AfterGuardInAction`, `AfterGuardBlockReaction` | B11의 과거 `GuardInCompleted` 표현을 코드명처럼 사용하지 않는다. |
| Intervention | 일반적으로 Want + Allow + 필요 시 Window timing | Dead force policy, Guard/Dodge/Parry 등 전용 override 예외를 감추지 않는다. |
| B10 검증 | terminal cleanup 구조 구현은 확인 | 제출용 PIE capture 전 “잔존 문제 완전 제거”라는 결과 표현 금지. |
| Execution | pair transaction과 HP=1 preflight 구조 확인 | 제출용 runtime capture 전 full flow 완전 회귀·commit 성공 주장 금지. |
| Death Facing | suppression 구조·PIE 기록 존재 | 전용 before/after capture 전 “항상 회전하지 않는다”는 영상 결과 표현 금지. |
| Overlay Editor | Editor-only CVar/PIE bridge | 현재 구현에 없는 CVar pointer cache·호출 수 성능 개선 주장 금지. |
| Asset Inspector | registry-ready inspection, review candidate | 자동 삭제 판정 또는 dynamic reference 완전 추적 주장 금지. |
| Root Motion | Apply/Revert safety contract 구현 | capture 전 visual preservation·transform 성공 주장 금지. |

---

## 7. 증거 확보 우선순위

| 우선순위 | 페이지 | 확보할 증거 | 이유 |
| ---: | --- | --- | --- |
| P1 | p.19 Profiling | 80 Enemy 동일 조건 3회 반복 CSV·로그 | 원본 1회 비교는 Ready이며, 재현성 주장을 위한 반복 측정이 필요하다. |
| P0 | p.1 / p.4~p.7 | HUD·Debug Overlay 없는 gameplay hero | 프로젝트가 실제 게임으로 읽히는 최소 증거다. |
| P0 | p.8~p.9 | Balance lifecycle / Execution session / HP=1 preflight | 구조 설명을 결과 과장 없이 뒷받침한다. |
| P1 | p.5 / p.7 / p.13 | event/overlay crop: HitWindow, Dodge decision, cleanup state | 대표 사례의 실제 runtime 결과를 읽게 한다. |
| P1 | p.16~p.17 | target revision·lifecycle, 참여 배정/해제 | AI 구조의 추상도를 낮춘다. |
| P1 | p.10 Death Lifecycle | Dead facing before/after, deferred sync event | lifecycle·facing 억제 주장의 런타임 근거를 완성한다. |
| P2 | p.22~p.24 | Root Motion before/apply/revert, workflow·traceability 발췌 | Tooling·workflow 주장 범위를 완성한다. |

---

## 8. 적용 후 결정 및 남은 확인

1. Death Lifecycle은 p.10 독립 페이지로 확정했다.
2. p.6의 대표 사례는 **B12**이며 B11은 deferred 구조 보조 근거로 둔다.
3. p.13 B10의 runtime cleanup 결과는 캡처 전까지 구현 구조로만 표현한다.
4. p.20은 CVar cache 성능 개선이 아니라 Editor↔PIE bridge 중심으로 유지한다.
5. Death, Profiling, Gameplay Hero 등 Evidence Ledger의 P0 항목을 확보한 뒤 캡처 상태를 갱신한다.

---

## 9. 감사에 사용한 핵심 출처

- `Docs/02_Bug_Report/B03`, `B05`, `B07`, `B09`, `B10`, `B11`, `B12`, `B13`, `B14`
- `Docs/05_System_Architecture/S31_UE5_Portfolio_System_Architecture.md`
- `Docs/05_System_Architecture/S35_UE5_Portfolio_Enemy_Balance_Collapse_Architecture.md`
- `Docs/05_System_Architecture/S36_UE5_Portfolio_Execution_Collaboration_Architecture.md`
- `Source/Portfolio/Action`, `Reaction`, `Component`, `Character`, `Core/Debug`, `Plugins`
- Git history: `746ea548`, `6a218760`, `659cb37e`, `168d7cfb`, `4b2c0b97`, `29c8b5d1`, `73aa0e87`, `92e706a2`, `d5919103`, `2cca03d7`, `821acbea`, `74415372`, `c7580883`, `4582dbe4`, `72e45573`, `4ed7f45c`, `4757c07e`, `6ecf78cb`, `f9d97d42` 등

> Git commit은 변경 시점의 보조 근거이며, 제출 문구는 항상 현재 `main` 코드와 Evidence Ledger의 검증 상태를 우선한다.

---

## 10. 25페이지 전환에 따른 사례 재배정 (2026-09-06)

이 절은 위 초기 24페이지 배정표에서 p.11 이후 항목을 대체한다. HTML이 아직 24페이지이므로, 이 번호는 **문서·목차·증거 계획 기준**이다.

| 사례 | 이전 배정 | 문서상 새 배정 | 판정·이유 |
| --- | --- | --- | --- |
| C09 / B07 AI Combo | p.11 Shared Execution | 대표 배정 해제, 면접 보조 | Action chain 재사용은 사실이나 Execution Transition의 Intervention 문제를 대표하지 않는다. p.16 AI Intent의 보조 설명 또는 면접 근거로만 사용한다. |
| C11 / B10 terminal cleanup | p.13 Combat Signal | p.11 Execution Transition 보조 | executor terminal path의 cleanup 근거이며 Combat Signal 책임이 아니다. |
| C10 / B09 Want·Allow | p.12 Intervention Policy | p.12 유지 | Intervention 정책의 대표 사례다. |
| C12 Combat Signal | p.13 Combat Signal | p.13 Source Boundary + p.14 Outcome 보조 | 하나의 단방향 구조를 Source/Target 단계로 분리한다. |
| C14 Data-driven Resolve | p.14 Data-Driven Resolve | p.11 Context 구성 보조 | 독립 결과 페이지를 만들지 않는다. Key/Data/Context는 공통 전환 계약의 입력 구성 근거다. |
| C08 Execution Pair | p.9 Execution Collaboration | p.9 게임플레이 + p.15 구조 | p.9는 결과, p.15는 reservation·commit·release 거래를 맡는다. |
| C13 native reference recovery | p.13 하단 또는 p.24 | 대표 배정 해제, 면접 보조 | 전환·Signal·Pair의 중심 서사를 방해하므로 관련 검증 문서/면접 보조로 제한한다. |
| A01–D06 기존 p.15–p.24 | p.15–p.24 | p.16–p.25 | 신규 p.15 삽입에 따른 단순 순연이다. |

### 10.1 문서상 p.11–p.25 최종 역할

| p. | 페이지 | 대표 사례·근거 | 증거 상태 |
| ---: | --- | --- | --- |
| 11 | Execution Transition | 공통 Decision 계약, C11/B10 cleanup | Code Confirmed / E11a·E11b Needs Capture |
| 12 | Intervention Policy | C10/B09 | Code Confirmed·Documented / E12 Needs Capture |
| 13 | Combat Signal Source Boundary | C12 Source 전달 | Code Confirmed / E13 Needs Capture |
| 14 | Target Outcome Resolution | C12 Target Outcome | Code Confirmed / E14 Needs Capture |
| 15 | Execution Pair Coordination | C08 구조 | Code Confirmed / E15a·b·c Needs Capture |
| 16–25 | 기존 AI Intent–Traceability | 기존 A01–D06 | 기존 상태를 한 페이지씩 순연 |

이 재배정은 `S37_UE5_Portfolio_Execution_Transition_and_Intervention_Architecture.md`, `S38_UE5_Portfolio_Combat_Signal_Architecture.md`, `S36_UE5_Portfolio_Execution_Collaboration_Architecture.md` 및 Evidence Ledger를 우선 기준으로 한다.
