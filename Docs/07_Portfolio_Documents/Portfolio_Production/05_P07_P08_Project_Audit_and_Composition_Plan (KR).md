# p.7 Dodge Intervention / 구 p.8 Balance · Collapse · Execution 프로젝트 감사 및 구성 계획

> 상태: **p.7 HTML Drafted / 구 p.8 감사 기록은 23페이지 재구성으로 대체됨**  
> 목적: p.7 감사 기록과, 분리 전 구 p.8의 사실·증거 경계를 보존한다. 현재 p.8 Balance / Collapse와 p.9 Execution Collaboration의 구성은 `20_P08_Balance_Collapse_P09_Execution_Composition_Plan`을 우선한다.

---

## 1. 감사 방법과 공통 결론

다음 세 층을 교차 확인했다.

1. **현재 C++ 구현**: `CAction_Dodge`, Action Orchestrator, `CBalanceComponent`, `CExecutionCollaborationComponent`
2. **설계·변경 기록**: S35 Balance / Collapse, S36 Execution Collaboration, B09 Intervention 정책 버그 보고서
3. **제출용 증거 상태**: Evidence Capture Ledger, Capture Shot List, FinalCandidate 파일 목록

결론은 다음과 같다.

- p.7은 회피 연출 자체보다 **현재 실행과 새 회피 요청의 관계를 정책으로 결정하는 과정**이 핵심이다. 다만 어떤 피격 반응이 실제로 회피에 의해 중단되는지는 데이터 규칙과 PIE 연속 캡처로 별도 확인해야 한다.
- p.8은 **Balance 누적과 Collapse 수명주기**에는 코드·문서·FinalCandidate 증거가 있다. 반면 Execution은 현재 구현 계약은 확인되지만, 제출용 런타임 결과 증거가 없다. 따라서 p.8의 중앙 시각 자료를 Execution 성공 장면으로 만들면 안 된다.
- 두 페이지 모두 p.10 Intervention Policy, p.11 Combat Signal과 역할이 겹치지 않아야 한다. p.7은 회피 기능의 실제 동작·검증, p.8은 상태 수명주기와 Execution 연결 범위만 다룬다.

---

## 2. p.7 Dodge Intervention 감사

### 2.1 코드로 확인된 사실

| 확인 항목 | 현재 구현 사실 | 근거 |
| --- | --- | --- |
| 회피 요청 | `ECombatActionIntent::Dodge`는 `EActionType::Dodge` 후보로 해석된다. | `CActionOrchestratorComponent::ResolveCombatActionCandidate()` |
| 기본 실행 관계 | `UCAction_Dodge`는 현재 실행 상태와의 관계를 `Independent` 또는 `Exclusive`로 해석해 수락 여부를 결정한다. | `CAction_Dodge::ResolveExecutionDecision()` |
| 개입 성립 조건 | Exclusive 관계일 때 incoming Dodge의 `WantIntervention()`과 active Action/Reaction의 `AllowIntervention()`이 모두 성립해야 `Intervene`으로 적용된다. | `CActionOrchestratorComponent::ResolveInterventionDirective()` |
| 거절 가능성 | active 실행이 Allow하지 않으면 `ActiveCannotAcceptIntervention`, incoming이 Want하지 않으면 `IncomingCannotIntervene`으로 거절된다. | 같은 Orchestrator 구현 |
| Guard 전환 | Dodge가 시작될 때 Guard runtime state가 있으면 `ClearGuardState`를 요청한다. | `CAction_Dodge::ResolveObservableOverlayCondition()` |
| 관찰 가능성 | Orchestrator audit에는 Decision, Relationship, ApplyMode, RejectReason, Overlay handling이 남는다. | `FExecutionOrchestratorDebug` |

### 2.2 문서로 확인된 설계 기준

`B09`는 incoming의 **Want**와 active의 **Allow**를 분리하고, Notify는 정책이 아니라 Allow window의 timing만 제공하도록 수정한 사례다. 다만 B09의 대표 재현은 `HitReaction → active Action` 중심이다. 따라서 이를 “Dodge가 피격 반응을 중단한 완료 증거”로 쓰지 않는다.

### 2.3 현재 증거 범위와 금지 주장

| 구분 | 상태 | 포트폴리오에서의 사용 |
| --- | --- | --- |
| `CAction_Dodge` 및 Orchestrator 코드 | 확인됨 | 개입 정책과 Guard 해제 계약 설명 가능 |
| B09 변경 기록 | 확인됨 | Want / Allow 분리의 설계 근거로 사용 가능 |
| Dodge 연속 게임플레이 | 없음 | 새 캡처 전에는 실제 Hero 결과로 표현 불가 |
| Action 중 Dodge 수용 | 미확정 | PIE에서 Decision / ApplyMode를 확인한 뒤에만 사례로 사용 |
| Hit Reaction 중 Dodge 수용 | 미확정 | 수용을 전제한 문구·화면 금지. 수용 또는 거절 중 실제 결과만 기록 |

### 2.4 p.7 권장 메시지와 페이지 역할

**한 줄 결론**

> 회피 요청은 현재 실행을 무조건 덮어쓰지 않고, 실행 관계와 Allow 정책을 통과할 때만 시작하거나 개입합니다.

이 페이지는 p.10의 일반 정책표를 미리 설명하는 장이 아니다. **Dodge라는 플레이 가능한 기능이 어떤 조건에서 현재 실행과 공존·교체·거절되는지**를 보여주는 Gameplay 페이지다.

#### 권장 세션 구성

1. **회피 실행과 상태 전환**  
   - 중심 자료: Debug Overlay 없는 `Guard → Dodge` 또는 검증된 `Action → Dodge` 16:9 Hero.
   - 우측 짧은 흐름: 입력 → 현재 실행 관계 판단 → Guard 상태 정리(해당 시) → Dodge 적용.
   - `피격 중 회피 성공`을 Hero로 정하지 않는다. PIE 결과를 확보한 뒤 선택한다.

2. **개입 결정 Runtime 검증**  
   - 중심 자료: 2~3개의 연속 Overlay / Event Log Crop.
   - 각 사례에는 `Incoming=Dodge`, `Active`, `Decision`, `ApplyMode`, `RejectReason` 중 실제로 보이는 값만 표시한다.
   - 권장 사례: Idle에서 `Started`, 검증된 active 실행에서 `Intervened` 또는 `Rejected`, Guard 상태에서 `ClearGuardState` 확인.

3. **문제 해결 경험 — 개입 정책의 책임 분리**  
   - B09를 바탕으로 “notify가 정책과 timing을 함께 들면 판단 경로가 흔들린다”는 문제를 간결하게 제시한다.
   - 해결: incoming Want / active Allow는 Action·Reaction Data가, Notify는 window timing만 담당.
   - 이 영역은 Dodge의 런타임 성공을 대신 증명하지 않는다. 정책을 왜 데이터·timing으로 분리했는지 설명한다.

### 2.5 신규 캡처 게이트

| ID | 우선순위 | 캡처 내용 | 판정 기준 |
| --- | --- | --- | --- |
| C05a | 필수 | Overlay 없는 `Guard → Dodge` 또는 검증된 `Action → Dodge` Hero | 회피 입력과 전환 결과가 한 프레임에서도 읽힘 |
| C05b | 필수 | `Dodge` 요청이 처리된 Overlay / Event Log | Active, Decision, ApplyMode 중 확인 가능한 값을 캡션으로 기록 |
| C05c | 권장 | active Reaction 중 Dodge 요청 결과 | `Intervened`와 `Rejected` 중 실제 결과를 그대로 기록. 성공을 전제하지 않음 |

---

## 3. p.8 Balance · Collapse · Execution 감사

### 3.1 Balance / Collapse에서 확인된 사실

| 흐름 | 현재 구현 사실 | 근거 |
| --- | --- | --- |
| 누적 | Parry 결과 packet만 수용하고, 같은 target의 오래되거나 중복된 result serial은 무시한다. | `UCBalanceComponent::AdvanceBalanceFromParry()` |
| 임계 전이 | threshold 최초 도달 시 lifecycle serial을 올리고 `CollapseInPending`으로 전이한다. | 같은 구현 |
| Collapse | `CollapseIn` 완료 뒤에만 `CollapseLoopActive`와 Loop TTL을 시작한다. | `CBalanceComponent`, S35 |
| 회복 | TTL 만료는 `CollapseOutPending`과 `CollapseOut` 요청으로 이어지고, Reset Notify 뒤 Accumulating으로 돌아간다. | `CBalanceComponent`, S35 |
| 전투 차단 | Balance lifecycle이 Accumulating이 아니면 일반 전투 요청을 차단한다. | `IsBalanceLifecycleBlocking()`, Orchestrator 호출부 |
| 관찰 | Count, lifecycle state / serial, Loop remaining, pose, blocking, facing suppression을 Debug snapshot으로 투영한다. | S35 §13 |

### 3.2 Execution에서 확인된 사실과 증거 한계

S36과 현재 C++은 Execution을 Source / Target의 pair collaboration으로 구현한다.

```text
CollapseLoopActive
→ Target이 opportunity / reservation 검증
→ Source / Target primary pair 시작
→ Source Commit
→ Standard: Execution Down / Recovery
   또는 Lethal: Health Dead / execution death presentation
```

현재 코드에는 Target Snapshot revision, 실제 거리·각도, reservation, commit, 양측 terminal 관찰, Standard / Lethal 분기가 있다. 그러나 Evidence Ledger에는 **제출용 Execution 런타임 캡처가 없다.** `enemy_recent_execution` 파일명은 AI 이벤트 용도이며 Execution pair의 완료 근거로 사용하지 않는다.

### 3.3 p.8의 안전한 주장 범위

| 구분 | 포트폴리오에서 말할 수 있는 것 | 말하면 안 되는 것 |
| --- | --- | --- |
| Balance / Collapse | Parry 누적, threshold, Collapse lifecycle, Loop TTL, 회복의 구현·관찰 구조 | 같은 장면만으로 모든 lifecycle 분기를 회귀 검증했다고 표현 |
| Execution 구조 | opportunity reservation, Source / Target pair, Commit 이전 취소와 outcome contract가 코드·설계 문서에 존재 | Standard / Lethal이 실제 플레이에서 끝까지 검증·완료됐다고 표현 |
| 기존 캡처 | Stagger stack 2/3이 Count 누적과 Collapse 진입의 보조 증거 | 이를 Execution 성공·처형 결과 증거로 전용 |

### 3.4 p.8 권장 메시지와 페이지 역할

**한 줄 결론**

> Parry 결과로 누적된 Balance는 Collapse 수명주기를 열고, Execution은 그 기회를 예약해 별도의 Source·Target 협업 계약으로 처리합니다.

#### 권장 세션 구성

1. **Balance에서 Collapse까지의 수명주기**  
   - 중심 자료: `Parry → Count → Threshold → CollapseIn → CollapseLoopActive → TTL → CollapseOut → Reset` 단일 lifecycle 다이어그램.
   - p.8의 중심은 Execution 성공이 아니라 이 lifecycle이다.

2. **누적·전이 Runtime 검증**  
   - `Stagger stack 2`와 `Stagger stack 3`을 2개 사례로 사용한다.
   - Caption은 Count·threshold·lifecycle state처럼 화면에서 읽히는 값만 기록한다.
   - 새 캡처가 가능하면 CollapseIn 또는 Loop state 1장을 더해 2~3개 사례로 구성한다.

3. **Execution 연결 범위와 제한**  
   - 작은 계약 흐름: `CollapseLoopActive + no reservation → Source/Target 검증 → reservation → pair / Commit`.
   - Standard / Lethal의 결과를 그림으로 과장하지 않고, “런타임 제출 증거 확보 전에는 구조와 구현 범위로만 설명”이라고 명시한다.
   - 이 영역에는 ‘문제 해결 경험’ 대신 **구현 범위와 제한**이라는 제목이 적합하다.

### 3.5 신규 캡처 게이트

| ID | 우선순위 | 캡처 내용 | 판정 기준 |
| --- | --- | --- | --- |
| C06a | 기존 사용 | Stagger stack 2 / 3 | Balance Count 누적과 Collapse 진입의 보조 증거로만 사용 |
| C06b | 필수 | Overlay 없는 Parry 누적 → Collapse Hero | 실제 시스템 결과가 보이고, Debug Overlay는 없음 |
| C06c | 선택·강화 | Execution session Overlay / Event Log | reservation, commit, terminal 중 실제로 수집 가능한 값만 사용 |
| C06d | 선택·강화 | Execution clean gameplay | Standard 또는 Lethal 중 실제 완료한 한 경로만 보여줌 |

---

## 4. 다음 HTML 보완 작업의 순서

1. C05a~c, C06b~d의 **현재 확보 가능 여부를 먼저 확인**한다. 없는 경우 Placeholder의 문구에 그 제한을 남긴다.
2. p.7을 **회피 실행과 상태 전환 → 개입 결정 Runtime 검증 → 문제 해결 경험** 순서로 보완한다.
3. p.8을 **Balance / Collapse lifecycle → Stack 2/3 Runtime 검증 → Execution 연결 범위와 제한** 순서로 보완한다.
4. HTML 반영 직후 Page Spec Index, Backlog, Capture Shot List, Evidence Ledger를 실제 사용한 캡처 범위와 동기화한다.
5. A4 렌더링에서 p.7·p.8의 하단 넘침과 중복 설명을 확인한다.

## 5. 관련 근거

- `Source/Portfolio/Action/CAction_Dodge.cpp`
- `Source/Portfolio/Component/CActionOrchestratorComponent.cpp`
- `Source/Portfolio/Component/CBalanceComponent.cpp`
- `Source/Portfolio/Component/CExecutionCollaborationComponent.cpp`
- `Source/Portfolio/Core/Debug/FExecutionOrchestratorDebug.cpp`
- [B09 Action / Reaction intervention bug report](../../../02_Bug_Report/B09_UE5_Portfolio_Bug_Report.md)
- [S35 Balance / Collapse Architecture](../../../05_System_Architecture/S35_UE5_Portfolio_Enemy_Balance_Collapse_Architecture.md)
- [S36 Execution Collaboration Architecture](../../../05_System_Architecture/S36_UE5_Portfolio_Execution_Collaboration_Architecture.md)
- [Evidence Capture Ledger](../../../98_Evidence/Portfolio_Evidence_Capture_Ledger (KR).md)
- [Capture Shot List](02_Portfolio_Capture_Shot_List (KR).md)
