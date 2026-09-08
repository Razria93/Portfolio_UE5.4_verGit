# Portfolio Page Composition Application Plan

> 상태: 기존 양식 반영 기록 / **Visual Grammar Refactor 1차 구현 진행 중**  
> 범위: Analytical A4 Wireframe p.6~p.24 보완  
> 선행 기준: p.4 Targeting, p.5 Combo / Hit 파일럿 페이지

> 2026-09-05 이후의 번호·섹션 헤더·단색 UI·SVG 중심 시각화 규칙은 [Visual Grammar Refactor Plan](24_Portfolio_Visual_Grammar_Refactor_Plan%20(KR).md)을 우선한다. 이 문서의 기존 반영 기록은 감사 이력으로 보존한다.

---

## 1. 목적

p.4와 p.5에서 확인한 페이지 구성 원칙을 나머지 본문에 적용한다. 목표는 모든 페이지를 같은 박스 양식으로 복제하는 것이 아니라, 각 페이지가 **무엇을 만들었는지, 무엇으로 확인했는지, 어떤 판단 또는 범위를 가졌는지**를 한 페이지 안에서 완결하는 것이다.

각 페이지는 다음 세 역할을 기본으로 사용한다.

```text
핵심 동작·구조
→ 검증 근거·사례
→ 문제 해결·설계 경계·구현 범위
```

세 역할은 공통 원칙이며, 모든 페이지가 같은 제목·같은 시각 자료·같은 비중을 가질 필요는 없다.

### 공통 제약

1. 한 페이지에는 하나의 중심 시각 자료만 둔다.
2. p.1 Cover, p.2 Index, p.3 System Map은 안내·관계 지도 성격이므로 이 패턴의 적용 대상에서 제외한다.
3. p.4와 p.5는 게임플레이 Hero와 Runtime 데이터 검증을 중심으로 하는 파일럿이다.
4. 구조 상세는 p.11~p.14에, 개별 기능의 실제 플레이 결과와 lifecycle은 p.4~p.10에 우선 배치한다. 같은 다이어그램이나 설명을 반복하지 않는다.
5. 실제 캡처·코드·문서·측정값으로 확인하지 못한 내용은 Placeholder 또는 구현 범위로만 표기한다.

---

## 2. 페이지 유형별 구성 원칙

| 유형 | 중심 시각 자료 | 둘째 영역의 근거 | 셋째 영역의 역할 |
| --- | --- | --- | --- |
| Gameplay | Hero 프레임, 비교 프레임, 상태 전환 프레임 | Runtime Overlay·Event Log·조건/결과 비교 | 문제 해결 경험 또는 설계상 충돌 해결 |
| Shared Structure | 단순 책임 다이어그램, 정책 표, 데이터 흐름 | 요청·결과·데이터 사례 | 책임 경계 또는 확장성 판단 |
| Enemy AI | 행동 전환 흐름, 할당 타임라인 | Current AI·Recent Event·Assignment 상태 | 권위·참여·수명주기 경계 |
| Profiling / Debug | 전후 그래프, 측정 흐름, 주석 캡처 | 측정 조건·CSV 구간·Overlay 값 | 원인과 개선 또는 관찰 범위 |
| Editor Tooling | 입력→출력 도구 흐름 | 실제 패널·Tree·CSV·전후 결과 | Editor-only·안전성·운영 범위 |
| Workflow / Traceability | 작업 산출물 연결 흐름 | Brief·Issue·문서·Commit/PR·검증 기록 | 사람의 판단 책임·추적 범위 |

### 셋째 영역 제목 선택 규칙

| 조건 | 권장 제목 |
| --- | --- |
| 실제 오류 또는 처리 충돌을 해결한 경우 | 문제 해결 경험 |
| 기능 간 책임 분리가 핵심인 경우 | 설계 경계 |
| 아직 증거가 제한되거나 구현 범위를 밝혀야 하는 경우 | 구현 범위와 제한 |
| Editor 도구의 오용을 방지해야 하는 경우 | 안전성·운영 범위 |
| AI·문서화에서 사람의 검토가 핵심인 경우 | 판단 책임·추적 범위 |

---

## 3. p.6~p.24 적용 계획

| 페이지 | 핵심 동작·구조 | 검증 근거·사례 | 판단 또는 대체 요소 |
| ---: | --- | --- | --- |
| 6 Guard / Parry | 동일 피격 신호의 Guard·Parry·일반 피격 결과 분기 | Block Hit·Parry·Player Hit 3조건 Overlay 비교 | 방어 판정과 피격 반응의 책임 분리 |
| 7 Dodge Intervention | Guard → Dodge 또는 PIE로 확인한 현재 실행과 회피 요청의 관계 | Decision·ApplyMode·RejectReason의 Runtime 사례 | 문제 해결 경험: Want / Allow 정책과 window timing 책임 분리 |
| 8 Balance / Collapse | Parry 누적부터 Collapse Loop·Reset까지의 단일 Actor 수명주기 | Stack 2·3은 Count/Threshold 보조 근거, 상태 전이·TTL·Reset은 신규 캡처 | `UCBalanceComponent`의 State·Serial·Timer 권한 |
| 9 Execution Collaboration | 두 Actor가 검증·예약·확정·해제하는 Pair Transaction | Session/Partner·Reservation·Commit/Release는 신규 캡처 | HP 1 Standard 사전 거절은 공용 Resolve 계약으로만 설명 |
| 10 Death Lifecycle | Health Dead 이후 Cleanup·Presentation·Finalize/Destroy의 분리된 수명주기 | Dead Entry·Facing Suppressed·Finalize route의 신규 runtime 캡처 | Dead 상태의 늦은 Facing 재적용 차단 |
| 11 Shared Execution | Player·AI·Reaction 요청이 공통 계약으로 합류하는 경로 | 주체별 요청 사례 | 공통 계약과 주체별 실행 책임의 경계 |
| 12 Intervention Policy | 현재 실행과 새 요청의 결정 흐름 | 공격 중 회피·피격 중 회피 등의 정책 사례 | 충돌 우선순위 설계 |
| 13 Combat Signal | Hit Context부터 결과 전달까지의 데이터 흐름 | 수용·거절 또는 결과 패킷 사례 | Source·Target·Health·Reaction 책임 분리 |
| 14 Data-Driven Resolve | Request → Key → Data → Runtime 조회 | 공통 로직과 데이터 변경점 비교 | 로직·설정 분리의 확장성 |
| 15 AI Intent | Blackboard 문맥 → Intent State → 요청·종료 대기 | Intent State·Action 상태 기록 | 판단과 실행 책임 분리 |
| 16 Combat Target | Decision Policy → SoT → Consumer, Revision lifecycle | Target Snapshot·변경 이벤트 | 단일 기준과 Blackboard 투영 경계 |
| 17 Participation | Evidence → Candidate → Assignment → Role, release lifecycle | Assignment 상태·Evidence 수명 | admission과 진행 Action 보호 정책 |
| 18 Profiling | PA05 / PA09 문서 기록 비교와 후보 누수 변화 | 동일 조건·지표·source ID, raw CSV reattach gate | 유효 전투 대상 인식 지연의 원인 추적 |
| 19 Runtime Debug | Runtime Source → Snapshot/ViewData → Canvas Overlay, CVar → World DrawDebug의 두 관찰 경로 | Current AI 중심 FinalCandidate + Focus·Event Log 보조 증거 | read-only 관찰 범위와 제출 주장 경계 |
| 20 Overlay Editor Plugin | Toolbar → Panel → CVar → Overlay 사용 흐름 | Toolbar·Nomad Panel 실제 캡처 | Editor와 Runtime 역할 분리 |
| 21 Asset Inspector | Asset 선택 → 의존성 조회 → Tree/CSV 출력 | Tree View·CSV 출력 사례 | 조사 도구 범위: 삭제 승인 도구가 아님 |
| 22 Root Motion Tool | 원본 → Apply → Revert 흐름 | 적용 전후 및 Revert 결과 | 안전한 Apply/Revert 계약 |
| 23 AI Workflow | Work Brief → 조사 → AI 초안 → 개발자 검토 → 검증 기록 | 동일 식별자의 Brief·Review·Verification 연결 | AI 제안과 사람의 채택·미검증 기록 책임 |
| 25 Change Traceability | D13 선행 기준 + B05 증상 → P12 내부 변경 기록 → historical commit → 현재 코드 순서 계약 | B05·P12·commit·현재 코드의 2×2 대조 보드 | Git·문서는 연결점이며 현행 runtime 증거를 대체하지 않음 |

---

## 4. 적용 순서와 검수 단위

한 번에 전체 페이지를 수정하지 않는다. 각 페이지군의 첫 페이지를 파일럿으로 검토한 뒤 같은 유형에만 확장한다.

1. **Gameplay:** p.6 Guard / Parry → p.7 Dodge Intervention → p.8 Balance / Collapse → p.9 Execution Collaboration → p.10 Death Lifecycle
2. **Shared Structure:** p.11 Shared Execution → p.12~p.14
3. **Execution Pair Structure:** p.15 Execution Pair Coordination
4. **Enemy AI:** p.16 AI Intent → p.17~p.18
5. **Profiling / Tooling:** p.19 Profiling → p.20~p.23
6. **Workflow:** p.24 AI Workflow → p.25 Traceability

각 페이지의 완료 기준은 다음과 같다.

- 페이지의 핵심 메시지가 다른 페이지와 중복되지 않는다.
- 중심 시각 자료가 하나이며, 나머지 블록은 그 주장을 보강한다.
- 실제 근거가 있는 주장만 본문에 쓴다.
- 필요한 새 캡처는 Capture Shot List와 Evidence Capture Ledger에 기록한다.
- HTML A4 출력에서 페이지 하단을 넘기지 않고, 24페이지 예산을 유지한다.

---

## 5. p.6 Guard / Parry 파일럿 기록

### 반영 근거

p.6은 Block Hit·Parry·Player Hit FinalCandidate 증거가 이미 있어, 새 기능 구현이나 신규 캡처에 의존하지 않고 페이지 양식을 검증할 수 있다. 세 화면은 각각의 결과를 증명하며, 동일 공격 조건의 직접 비교로는 주장하지 않는다.

### 적용 결과

`피격 신호는 방어 상태와 패링 허용 구간을 먼저 해석하고, 그 결과에 따라 피해 반영과 피격 반응을 각각 결정한다`는 메시지로 구성했다.

### 구성

1. **Guard / Parry 게임플레이 흐름**  
   Debug Overlay 없는 Guard·Parry·Player Hit 비교 Hero와 짧은 판정 흐름을 중심 시각 자료로 둔다.

2. **결과 데이터 검증**  
   기존 FinalCandidate 캡처 3장을 사용해 Defense Outcome, 피해 반영 여부, 최종 Reaction 결과를 비교한다. 실제 캡처에서 읽히는 항목만 표기한다.

3. **방어 판정과 피격 반응의 책임 분리**  
   `Defense Outcome → Reaction Outcome → Damage Commit`의 작은 흐름으로 서로 다른 책임을 설명한다.

### 검증 완료 범위

1. Evidence Package 기준으로 Block Hit는 Guard·BlockHit·Final/Commit 2.500, Parry는 Parry·Final/Commit 0.000, Player Hit은 Hit·Commit 10.000 가시 값을 사용한다.
2. `EDamageDefenseOutcome`은 방어 판정 사실을, `EDamageReactionOutcome`은 최종 피격 표현을 보존하는 코드·설계 계약을 확인했다.
3. p.6 HTML 및 Page Spec Index, Enrichment Backlog, Capture Shot List를 동기화했다. 결과 데이터 증거는 Ready지만, 게임플레이 Hero는 신규 캡처가 필요하다.

### p.7 Dodge Intervention 반영 기록

1. 기존의 개념 다이어그램·보조 흐름·사례 카드 반복을 제거하고, **회피 실행과 상태 전환 → 개입 결정 Runtime 검증 → 개입 정책의 책임 분리**로 재구성했다.
2. Hero는 `Guard → Dodge` 또는 PIE로 검증된 `Action → Dodge` 신규 캡처 자리로만 두었다. active Reaction 중 Dodge 수용은 실제 Capture 전까지 성공 결과로 표현하지 않는다.
3. Runtime 사례는 `Idle → Dodge`, `Active → Dodge`, `Guard → Dodge`로 나누고, Decision·ApplyMode·RejectReason 중 실제 확보한 값만 교체하도록 했다.
4. p.7은 A4 PDF 출력에서 하단 넘침 없이 확인됐다. Hero와 Runtime Capture는 계속 `Needs Capture`다.

### 구 p.8 Balance / Collapse / Execution 반영 기록

> 이 기록은 분리 전 감사 이력이다. 현재 제출용 구성과 증거 상태는 `20_P08_Balance_Collapse_P09_Execution_Composition_Plan (KR).md`를 우선한다.

1. 중심은 Execution 결과가 아니라 `Parry Result → Balance Count → Threshold → Collapse Loop → Recovery` 수명주기로 고정했다.
2. 기존 Stagger stack 2·3 FinalCandidate는 각각 Balance 누적과 Collapse 진입의 보조 증거로만 배치했다.
3. Execution은 `Opportunity → Pair Validation → Reservation` 구현 흐름과 별도 표현 제한 메모로 분리했다. Commit·terminal의 제출용 Runtime 증거가 없다는 사실을 같은 단계 카드로 섞지 않는다.
4. 독립 검토에서 지적된 수명주기 카드 폭과 footer 간섭을 3단계→2단계 꺾인 흐름, 별도 Boundary Note로 수정한 뒤 A4 렌더링 검수와 최종 승인을 통과했다.

### p.9 Shared Execution 반영 기록

1. 공통 실행을 단일 Orchestrator가 아니라 `Candidate·Context → Decision Query·Result`의 공통 계약으로 한정했다.
2. Action과 Reaction의 요청 출처, Candidate·Context, 도메인별 ExecutionResult를 2-lane으로 분리해 실제 인스턴스·결과 객체가 별도라는 사실을 보였다.
3. Component와 Executor의 적용·수명주기 책임을 별도 영역에 남기고, p.11의 관계 정책 및 p.12의 Combat Signal 데이터와 겹치지 않게 했다.
4. 코드·설계 문서 근거 페이지로 유지했으며, 완료되지 않은 Runtime 증거를 요구하거나 표현하지 않았다.

### p.11 Intervention Policy 반영 기록

1. `Decision → Relationship → ApplyMode`의 세 축을 중심 도식으로 분리하고, Reject·Ignore이 관계·적용에 진입하지 않는 종료 분기를 명시했다.
2. Independent·Sequential·Exclusive를 실제 적용 방식에 매핑했다. Sequential은 Action 전용 `Reserve → Notify Consume`이며 Reaction Reserve 미지원임을 같은 카드에서 제한했다.
3. Intervention Directive는 Exclusive에서 incoming Want와 active Allow가 모두 수락된 경우에만 생성된다는 조건을 제목과 내용으로 고정했다.
4. DeadReaction 강제 개입을 일반 정책처럼 섞지 않고 구현 제한 메모로 분리했다.

### p.12 Combat Signal 반영 기록

1. Hit Window / Overlap → Source Resolve → Target Resolve → TargetPacket → 병렬 소비자라는 데이터 경계를 중심 시각 자료로 고정했다.
2. `FHitContext`, Source Payload, TargetPacket 카드로 각 단계의 정보 보존을 보여주되, p.6의 방어 결과 이미지와 중복하지 않았다.
3. Resource Commit·Reaction Request·Feedback Dispatch를 TargetPacket의 독립 소비자로 표현해 직접 실행 관계라는 오해를 막았다.

### p.13 Data-Driven Resolve 반영 기록

1. Action과 Reaction이 하나의 조회 경로로 합류하지 않고, 각 Orchestrator가 자기 DataKey·Data·Context·Executor를 해석하는 2-lane 구조로 표현했다.
2. Key는 lookup 기준, Data는 설정·정책, Context는 resolved Data·Executor를 포함한 runtime 입력이라는 역할 차이를 별도 카드로 고정했다.
3. wildcard·Global match는 조회 규칙이며 runtime Context는 엄격한 입력이라는 제한을 Boundary Note로 남겼다.

### p.14 AI Intent 반영 기록

1. p.14은 Patrol·Alert·Engage의 행동 목록이 아니라, `Blackboard 문맥 → Intent State → BT Task 요청 → 종료 대기`라는 판단 경계로 재구성했다.
2. `Dead · HitReact · Incapacitated`를 일반 AI 의도보다 앞서는 절대 상태로 분리해, Service가 실제로 적용하는 우선순위를 보였다.
3. BT/Blackboard의 판단·기록, BT Task의 요청·대기, 공통 Action 실행 계층의 수락·실행 책임을 별도 카드로 나눴다.
4. Combat Target의 Source of Truth와 Participation의 교전 역할 할당은 입력 문맥으로만 한정하고, 각각 p.15·p.16에서 설명한다.

### 다음 작업

### p.15 Combat Target 반영 기록

1. p.15는 대상 후보의 평가나 다수 Enemy 배정을 반복하지 않고, 상위 정책의 Set / Clear 요청과 `UCCombatTargetComponent`의 상태 소유를 분리했다.
2. `CurrentTarget · Revision · LastChangeReason · OnCombatTargetChanged`를 Kernel이 소유하고, Blackboard·Lock-on/HUD·Action Facing은 Snapshot·이벤트를 소비하는 단방향 구조로 배치했다.
3. None→A→B→None의 Revision 전이를 보조 증거로 두고, 동일 요청 No-op·EndPlay/stale 정리·늦은 결과 Revision 불일치 무시를 수명 계약으로 한정했다.
4. Player 후보 평가의 상세는 p.4, AI Intent는 p.14, Enemy 교전 역할 할당은 p.16에 남긴다.

### 다음 작업

### p.16 Participation 반영 기록

1. p.16는 개별 AI의 요청·승인 구조가 아니라 Perception·HitReactive의 live Evidence를 집계해 Participant × Target별 Assignment를 만드는 흐름으로 재구성했다.
2. `Assignment != None`만이 참여 상태의 권위임을 명시하고, None·Observe·Alert·Engage의 결과를 Component가 소비·반영하는 구조로 한정했다.
3. General Base 우선, live HitReactive Evidence가 있는 경우에만 Extra admission, Alert/Observe fallback이라는 역할 배정 순서를 별도 영역에 배치했다.
4. 마지막 Evidence 종료 뒤 정확한 Engage Action lock만 임시 보호하며 새 Extra admission은 즉시 사라지는 release 범위를 명시했다.

### 다음 작업

### p.17 Profiling 반영 기록

1. 제출 전 확정되지 않은 `10.064s → 1.070s`는 사용하지 않고, 분석 문서에 측정 계약이 함께 기록된 PA05/PA09의 `FirstValidLatency p95 9.377s → 0.724s`만 문서 기록 비교로 배치했다.
2. 80 Enemy·동일 테스트 맵·약 36초 캡처·중간 30초 분석·p95라는 비교 조건을 수치와 같은 카드에 고정했다.
3. RawActors/InvalidProviders/MaxTargetDataMap의 81/80/81→1/0/1 변화로 후보 누수와 Team Attitude / Affiliation 변경의 인과를 보였다.
4. 원본 CSV가 현재 워크트리에 없으므로 페이지 상태는 `Blocked: raw CSV reattach`를 유지한다. 수치는 AIPerception 엔진 전체 비용이 아닌 valid target recognition 지연이며, raw CSV를 다시 연결하기 전 최종 성능 사례로 승격하지 않는다.

### p.18 Runtime Debug 반영 기록

1. Canvas Overlay와 World DrawDebug를 하나의 출력으로 합치지 않고, `Runtime Source → Snapshot Store / ViewData → Canvas Overlay`와 `CVar 표시 범위 → World DrawDebug`의 두 경로로 분리했다.
2. 중앙 증거는 Enemy Current AI FinalCandidate 한 장으로 제한하고, 실제 화면의 Controller·Pawn·Target·IntentState·HasLOS·DistanceToTarget·IsCombatAction만 설명했다.
3. FocusComponent.NearestFocus와 Event Log All은 별도 보조 증거로 배치해 중앙 캡처에 없는 정보를 지목하지 않았다.
4. Gameplay UI·Shipping HUD·BT active node·Runtime LOD actual은 미구현 주장 대신 이 페이지의 제출 관찰 범위 밖으로 명시했다. p.19의 Toolbar·Panel·CVar 제어 책임은 다루지 않는다.

### 다음 작업

### p.19 Overlay Editor Plugin 반영 기록

1. Toolbar / Menu는 Nomad Panel을 여는 진입점으로만 표현하고, 직접 overlay toggle이나 command 실행 기능으로 과장하지 않았다.
2. Nomad Panel의 CVar read/write와 Focus command bridge를 2-lane으로 분리했다. 후자는 기존 PIE PlayerController command에 위임하며 FocusComponent·HUD·Store를 직접 조작하지 않는다.
3. Nomad Panel, Toolbar, Outliner 선택 FinalCandidate를 역할별로 나눠 배치하고, PIE world/PlayerController 부재 시 command를 실행하지 않는 안전 실패를 명시했다.
4. Editor module 격리, session-only 설정, runtime 상태 미소유를 마지막 책임 경계로 고정했다.

### 다음 작업

### p.20 Asset Inspector 반영 기록

1. 선택 Asset·조건 입력에서 Asset Registry의 Dependencies/Referencers query, Tree View, Content Browser Sync/CSV Export로 이어지는 조사 흐름을 중심에 배치했다.
2. Nomad panel은 실제 input·mode·filter·Tree/Export UI의 증거로 사용하고, CSV는 가짜 결과 캡처 대신 `AssetReferenceCsvExporter`의 code-derived schema로 제시했다.
3. `GetReferencers() == 0`의 Unused Candidate는 삭제 판정이 아닌 검토 후보라는 제한을 첫 안전 카드로 고정했다.
4. 동적·문자열·Asset Manager 참조 범위, Soft/Manage UI 부재, editor 디스크 크기 추정의 한계를 함께 명시했다.

### 다음 작업

### p.21 Root Motion Tool 반영 기록

1. Root Motion 변환 결과가 아니라 `입력 검증 → Track Backup → Transfer + Compensate`와 별도 Revert 계약을 중심에 뒀다.
2. 사본 Asset에서 작업하는 원칙은 사용자 작업 절차로, Root existing motion/yaw tolerance는 도구의 입력 수용 조건으로 분리했다.
3. Source·Apply·Revert 3프레임은 완료 결과가 아닌 `Needs Capture` evidence slot으로만 배치했다.
4. Revert는 backup이 있는 Root/direct child track의 복원 또는 원래 없던 track 제거로 한정했고, Editor-only raw track 수정과 거절 조건을 명시했다.

### 다음 작업

p.22 AI Workflow는 프롬프트 자체의 양이 아니라 Work Brief·코드/문서 조사·구현·검증·기록에서 사람이 판단을 책임지는 확인 가능한 작업 흐름으로 구성한다.

### p.22 AI Workflow 반영 기록

1. AI를 구현 결과의 작성자가 아니라 조사·계획·초안의 보조로 한정하고, Work Brief와 개발자 검토 뒤에만 반영되도록 표현했다.
2. 동일 작업 식별자의 Brief·Review·Verification Record를 중심 증거로 요구해 템플릿 나열과 구분했다.
3. 실행하지 못한 Diff·Build·PIE 검증은 완료 주장으로 바꾸지 않고 미검증 기록으로 남긴다.

### p.25 Change Traceability 반영 기록

1. 일반적인 Issue/PR 흐름 대신 실제 B05 첫 overlap reject 사례를 D13의 선행 Pipeline 기준, P12의 내부 변경 기록, historical commit, 현재 코드 순서 계약으로 분리해 대조한다.
2. B05는 내부 Bug Record이고 P12는 내부 변경 기록이다. 둘을 GitHub Issue·외부 PR로 표현하지 않는다.
3. D13은 B05보다 앞선 baseline이며, 과거 `CAttachment`와 현재 `ACWeaponActor`는 같은 클래스라고 단정하지 않는다. collision enable 전 Hit Window 준비라는 순서 계약만 대조한다.
4. first-overlap 제출용 런타임 캡처는 아직 없으므로, 페이지는 현행 실행 성공이 아니라 변경 근거·현재 코드·증거 상태의 관계만 제시한다.
