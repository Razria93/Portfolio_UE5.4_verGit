# Combat Interaction Architecture Documentation Plan

> 상태: **방향 확정 / Static Code·Document Audit Complete / 신규 PIE 캡처 대기**  
> 작성일: 2026-09-06  
> 목적: Execution 전환, 단방향 Combat Signal, 양방향 Execution Pair의 관계를 현재 코드·기존 문서·제출 증거 기준으로 재정의하고, 이후 기술 문서 및 포트폴리오 개편의 기준을 고정한다.

---

## 1. 이번에 고정하는 판단

### 1.1. 페이지와 구조의 관계

현행 포트폴리오는 **25페이지**다. 사용자가 허용한 협업 구조 독립 설명을 위한 p.15를 삽입했고, HTML·목차·Page Spec의 이후 페이지 번호도 함께 순연했다.

```text
11–12p  Execution Transition / Intervention
13–14p  단방향 Combat Signal
15p     양방향 Execution Pair (신규)
```

25페이지 전환은 코드 근거와 Page Spec 갱신안을 검토한 뒤 적용했다. 기존 15–24p는 16–25p로 순연했다. 제출용 런타임 증거 E11–E15는 별도 미완료 항목으로 유지한다.

### 1.2. 세 구조의 책임 경계

| 구조 | 해결하는 문제 | 책임 | 포함하지 않는 책임 |
| --- | --- | --- | --- |
| Execution Transition / Intervention | 실행 중 새 실행이 들어왔을 때의 종료·전환·정책 판단 | Orchestrator의 전환 판단, 실행 객체의 고유 정책·동작, Lifecycle 전환 | Damage 결과 판정, Pair 거래 소유 |
| Combat Signal | 공격자가 Target을 직접 조작하지 않고 전투 사실을 전달 | Source의 Packet 구성·전달, Target의 Packet 수신·Outcome 확정 | 양측 준비 확인·예약·공동 Commit |
| Execution Pair | 처형처럼 두 Actor가 함께 성립해야 하는 실행 | 양측 조건 확인, Pair 예약, Commit, 취소·Release | 단방향 Damage Packet의 일반 소비 |

### 1.3. 핵심 정정

`Target-side Outcome Resolution`은 독립 상호작용 모델이 아니다. 이는 **단방향 Combat Signal의 Target 소비 단계**다.

```text
Combat Signal
Source: 사실 Packet 구성·전달
  → Target: Packet + Target State + Target Rule 해석
  → Target: Outcome 확정·결과 처리

Execution Pair
Actor A ↔ Pair Coordinator ↔ Actor B
  → 상호 조건 확인 → 예약 → 공동 Commit → 실행 또는 Release
```

따라서 13p와 14p는 하나의 Combat Signal 구조를 각각 Source/전달 단계와 Target/Outcome 단계로 나누어 설명한다. Execution Pair는 Combat Signal의 특수 Packet이나 하위 분기가 아니라 동등한 레벨의 별도 협업 구조로 설명한다.

---

## 2. 기준 기술 문서 계획

> 감사 결과: [Combat Interaction Code & Documentation Concordance Audit](26_Combat_Interaction_Code_Document_Concordance_Audit (KR).md)을 우선 기준으로 사용한다. B10의 재배치, 기존 C14의 별도 배치 결정, Pair의 `RequestExecutionOutcomeTarget()` 접점은 기준 기술 문서 작성 전에 반드시 반영한다.

| 문서 | 상태·조치 | 포트폴리오 연결 | 근거 출발점 |
| --- | --- | --- | --- |
| `S37_UE5_Portfolio_Execution_Transition_and_Intervention_Architecture.md` (신규) | 작성 완료 | 11–12p | S18, S20, S21, S22, S23, B09, B10, Action/Reaction Orchestrator |
| `S38_UE5_Portfolio_Combat_Signal_Architecture.md` (신규) | 작성 완료 | 13–14p | J01, N05, N06, B12, CombatSignal Source/Target Components |
| `S36_UE5_Portfolio_Execution_Collaboration_Architecture.md` (기존) | Pair 계약과 Combat Signal의 접점·표현 제한 보강 완료 | 15p, 9p 보조 근거 | `UCExecutionCollaborationComponent`, Balance lifecycle, Execution session |

S37·S38의 번호는 `Docs/05_System_Architecture`의 현행 번호 체계를 기준으로 한 예정 번호다. 실제 생성 직전 충돌 여부를 다시 확인한다.

### 2.1. 공통 문서 양식

세 기술 문서는 다음 목차를 공유한다.

1. 문제와 기존 방식
2. 구조 요구사항과 비목표
3. 현재 코드의 책임 경계
4. 런타임 흐름과 불변 조건
5. 코드·문서·런타임 증거 표
6. 제출 표현 제한과 현재 미확보 증거
7. 확장 조건과 의도적으로 보류한 일반화

모든 사실 주장은 아래 상태를 별도 표기한다.

| 상태 | 의미 |
| --- | --- |
| Code Confirmed | 현행 코드의 타입·호출 경로·조건으로 확인됨 |
| Documented | 설계 문서·Bug Report·작업 기록으로 확인됨 |
| Runtime Captured | 제출 가능한 PIE/Overlay/로그 캡처가 있음 |
| Needs Capture | 코드 또는 문서는 있으나 제출용 런타임 증거가 없음 |

과거 문서의 클래스명·역할이 현행 코드와 다르면 현행 코드를 우선하며, 과거 문서는 리팩터링 전 문제의 기록으로만 인용한다.

---

## 3. S37 — Execution Transition & Intervention Architecture

### 3.1. 문서의 결론

Action과 Reaction을 같은 물리 클래스라고 표현하지 않는다. 두 도메인은 각자 Orchestrator와 Execution Result를 유지하되, `Candidate → Context → Decision Query/Result → Apply/Dispatch`라는 **공통 전환 계약**으로 정규화된다.

Execution 전환 구조를 만든 목적은 Intervention이다. 기존 실행 위에 새 반응을 덧씌우는 방식은 이전 Trail, Collision, Hit Context 등 Runtime Effect의 소유·종료 경계를 흐리고, 새로운 예외가 생길 때마다 계층을 추가하게 만든다.

### 3.2. 문서 목차와 검증 항목

1. **문제 진단**
   - Action 내부, Reaction 내부, Action↔Reaction 교차 간섭 요구
   - 새 피격 반응이 기존 실행의 Runtime Effect를 알 수 없어 광범위한 정리가 필요했던 이유
   - 전환 책임을 개별 실행 객체에 두었을 때 예외가 늘어나는 이유

2. **전환 계약**
   - Candidate, Context, Decision Query/Result, ApplyMode, Directive의 역할
   - Orchestrator / Component / Executor 책임표
   - `Start → Interrupt → Cleanup → End/Release → Next Start` 순서

3. **Intervention 정책**
   - Priority, 실행 조건, Want, Allow, Notify Timing Gate의 소유자
   - Notify는 정책 판단자가 아니라 허용 시점 Gate라는 제한
   - Dead 강제 개입은 일반 Want∧Allow 규칙의 예외라는 제한

4. **검증**
   - B09: Action/Reaction 개입 조건과 실제 interrupt 결과
   - B10: terminal path에서 Runtime Effect를 정리한 구현 근거
   - 제출용 캡처에서는 Action→Reaction, Reaction→Reaction 전환을 구분해 확인

### 3.3. 포트폴리오 적용안

| 페이지 | 첫 섹션: 시각화·흐름 | 두 번째 섹션 | 세 번째 섹션 |
| --- | --- | --- | --- |
| 11p | 기존 덧씌우기 방식과 `Interrupt → Cleanup → Start` 전환 구조의 Before/After | `런타임 검증` — 이전 실행 종료와 신규 실행 시작 | `설계 판단 | 전환 책임을 개별 실행 객체에서 분리` |
| 12p | Want/Allow·Priority·Timing Gate의 판단 흐름 | `런타임 검증` — B09 범위의 개입 결과 | `문제 해결 경험 | Want·Allow와 Notify Timing의 분리` |

11p와 12p는 독립 사례가 아니라 하나의 구조 리팩터링의 구조·정책 페이지다.

---

## 4. S38 — Combat Signal Architecture

### 4.1. 문서의 결론

Combat Signal은 단방향 구조다. Source는 공격자·충돌·Damage 관련 **사실**을 Packet으로 구성해 전달하고, Target은 Source가 지정한 Reaction을 실행하지 않는다. Target은 Packet과 자신의 상태·규칙을 함께 해석해 Outcome을 확정한다.

이 구조는 다음 과거 결합을 줄이기 위한 것이다.

- 공격자가 Target의 Reaction Montage를 직접 실행하던 방식
- 공격자가 Target이 사용할 Reaction 정보를 직접 지정하던 방식
- 실행·캐릭터·방어 상태가 늘어날수록 Source가 Target의 처리 규칙까지 알아야 하는 방식

### 4.2. 문서 목차와 검증 항목

1. **Source/Target 책임 경계**
   - Source: Hit Context, Payload, Packet 전달에 필요한 사실 구성
   - Target: 유효성·수신 가능 여부·Damage/Defense/Reaction Outcome 해석
   - Source가 Target을 직접 조작하지 않는 경계

2. **요청 경로**
   - Collision 기반 Damage와 Timing 기반 Damage처럼 단방향 Signal에 맞는 진입 경로
   - 요청별 진입·특화 판단은 분리하되, 공통 결과 계약만 재사용한다는 원칙
   - 모든 요청을 하나의 범용 Damage Pipeline으로 처리했다고 표현하지 않는 제한

3. **Target-side Outcome Resolution**
   - `Incoming Packet + Target State + Target Rule → Outcome`
   - Guard/Parry 추가 시 단순 TakeDamage 처리만으로는 결과를 결정할 수 없었던 이유
   - Damage 감쇠·Reaction 종류 판단을 Execution Orchestrator에 넘겨 흐름이 역류하지 않도록 Target에서 확정한 경계

4. **검증**
   - Source Packet → Target 수신 → Accepted/Rejected → Target Packet/Dispatch의 코드 경로
   - Normal / Guard / Parry의 결과 데이터 비교
   - Target Packet, Defense Outcome, HP 전후, Reaction 결과가 함께 읽히는 런타임 캡처

### 4.3. 포트폴리오 적용안

| 페이지 | 첫 섹션: 시각화·흐름 | 두 번째 섹션 | 세 번째 섹션 |
| --- | --- | --- | --- |
| 13p | Source가 사실 Packet을 만들고 Target에 전달하는 단방향 흐름 | `런타임 검증` — Source→Target→Packet/Dispatch | `설계 판단 | 상대 객체 조작 대신 사실 전달` |
| 14p | 동일 계열 Packet을 Target 상태와 함께 해석해 Outcome을 확정하는 흐름 | `런타임 검증` — Normal / Guard / Parry 비교 | `문제 해결 경험 | Damage 처리와 실행 전환 판단의 역류 차단` |

14p는 13p의 Target 소비·판정 단계를 상세화한다. `Data-driven Resolve`라는 기존의 넓은 제목은 Target Outcome Resolution의 실제 설계 이유가 확인된 뒤 대체한다.

---

## 5. S36 — Execution Pair Coordination Architecture

### 5.1. 문서의 결론

Execution Pair는 처형처럼 Source와 Target이 함께 준비되고 함께 실행되어야 하는 양방향 협업 구조다. 일반 Combat Signal처럼 한쪽이 Packet을 전달하고 다른 쪽이 독립 소비하는 방식으로는 성립 여부·예약·중단 해제를 안전하게 관리하기 어렵다.

S36에 기록된 Pair 계약을 유지한다.

```text
Opportunity 확인
  → 양측 조건 검증
  → Pair 예약
  → 공동 Commit
  → 양측 실행
  → 완료·실패·중단 Release
```

### 5.2. 보강할 문서 항목

1. **Combat Signal과의 차이**
   - Combat Signal: Source 전달 / Target 독립 소비
   - Execution Pair: 상호 조건 확인 / Pair 예약 / Commit / 양측 Release

2. **책임 경계**
   - `UCExecutionCollaborationComponent`는 Pair transaction을 소유
   - Action/Reaction Orchestrator는 각 Actor 내부의 arbitration을 담당하며 Pair transaction을 소유하지 않음
   - Combat Signal Component는 일반 단방향 Signal 처리를 담당하며 Pair 성립을 담당하지 않음

3. **검증 조건**
   - 양측 조건 만족 시 Pair 성립·실행
   - 한쪽 조건 미충족 시 Commit 이전 거절
   - Commit 이전 취소·중단 시 reservation과 상태가 Release되는지 확인
   - Commit 이후 Standard/Lethal 결과와 terminal 경로를 구분

### 5.3. 포트폴리오 적용안

| 페이지 | 역할 | 구성 |
| --- | --- | --- |
| 9p | 게임플레이 결과 | Execution Pair가 게임에서 어떻게 보이고 완료되는지 보여 주는 Hero와 런타임 데이터. 구조 다이어그램은 사용하지 않음. |
| 15p (신규) | 구조 설명 | 양측 레인 시퀀스: 조건 확인 → 예약 → Commit → 실행/Release. 두 번째 섹션은 `런타임 검증`, 세 번째는 `설계 판단 | 단방향 Signal과 양방향 Collaboration의 분리`. |

15p에서는 "완전히 동시에 실행된다"보다 코드로 확인되는 `Pair 예약·공동 Commit·Release` 계약을 사용한다.

---

## 6. 증거와 표현 게이트

| 페이지 | 현재 확인 가능한 근거 | 제출 전 추가로 필요한 근거 | 금지 표현 |
| --- | --- | --- | --- |
| 11p | 공통 Decision Query/Result, Action/Reaction Orchestrator, S18/S20/S21 | Action·Reaction 각각의 전환 Overlay/로그 | 하나의 Orchestrator·하나의 Execution 객체를 공유한다고 표현 |
| 12p | B09, Want/Allow·Directive 코드 | B09 범위를 읽을 수 있는 제출용 캡처 | 모든 개입이 예외 없이 Want∧Allow라고 단정 |
| 13p | CombatSignal Source/Target, Target Packet·Debug 구조, J01/N05/N06 | Source→Target packet trace 캡처 | Source가 Target Reaction을 결정·실행한다고 표현 |
| 14p | Target Outcome 코드, Guard/Parry 문서·기존 후보 | Normal/Guard/Parry 동일 조건 비교 캡처 | 모든 Packet 확장을 이미 지원한다고 표현 |
| 15p | S36, `UCExecutionCollaborationComponent`, Pair session/reservation 계약 | 성공·거절·Release의 제출용 세 사례 | 프레임 단위 완전 동시성 또는 모든 협업 사례 지원을 단정 |

Super Armor, 피격 중 회피 탈출, 추가 Packet Handler 객체화는 현행 코드와 런타임 증거를 재확인하기 전까지 완료 기능이 아니라 확장 조건 또는 설계 한계로만 기록한다.

---

## 7. 반영 순서

1. **코드 감사**
   - S37/S38/S36에 적을 클래스·함수·호출 순서·실패/Release 경로를 현행 코드로 확정한다.
2. **기준 기술 문서 작성·보강**
   - S37, S38을 작성하고 S36에는 Combat Signal과의 경계·포트폴리오 표현 제한을 보강한다.
3. **증거 계획 동기화**
   - Page Spec Index, Audit Plan, Evidence Ledger, Capture Shot List, Claim Freeze Log에 25p 전환안과 `Needs Capture` 항목을 반영한다.
4. **시각화 설계**
   - 11p Before/After 전환, 12p 정책 흐름, 13p Source→Target, 14p Outcome Resolution, 15p Pair 시퀀스를 SVG/Mermaid 등 범위 고정 다이어그램으로 작성한다.
5. **HTML·목차·페이지 번호 반영**
   - 25페이지 HTML로 전환하고, 기존 15–24p의 순연과 SVG 다이어그램 반영을 완료했다.
6. **렌더링 검증**
   - PDF 출력의 페이지 객체 25개, 목차, Footer denominator, 섹션 제목을 확인했다. E11–E15는 목업 상태로 유지한다.

---

## 8. 완료 조건

- [x] S37과 S38이 현행 코드 기준으로 작성됨
- [x] S36에 Combat Signal과 Execution Pair의 경계가 명시됨
- [x] p.11–p.15의 주장·증거·금지 표현을 Page Spec, Audit Plan, Ledger, Claim Freeze Log에 동기화함
- [x] 신규 15p의 문서 역할을 Pair 구조 설명으로 고정하고 p.9를 게임플레이 결과로 분리함
- [x] 25페이지 HTML·목차·footer와 SVG 다이어그램을 반영하고 PDF 페이지 객체 25개를 확인함
- [ ] E11–E15 제출용 런타임 캡처를 확보한 뒤 결과 주장을 Runtime Captured로 승격함

E11–E15 캡처 검증은 다음 구현 단계의 미완료 항목이다.
