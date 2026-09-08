# p.15 Execution Pair Coordination 프로젝트 감사 및 구성 계획

> 상태: **코드·설계 문서 감사 완료 / 사용자 제작 Pair Transaction 다이어그램 HTML 반영 완료 / 성공·거절·Release 캡처 대기**  
> 기준: 현행 포트폴리오와 HTML은 25페이지다.

## 1. 페이지 목적

p.15는 처형처럼 두 Actor가 함께 성립해야 하는 **양방향 Execution Pair 거래**를 설명한다. p.9는 게임플레이 결과만 보여 주고, p.15만 예약·Commit·Release의 구조를 맡는다.

Combat Signal의 단방향 사실 전달과 달리 Pair는 양측 상태·기회·기하 조건을 확인하고, Commit 전 취소 시 reservation을 해제해야 한다.

## 2. 코드·문서 대조

| 항목 | 현행 확인 경로 | 확정 범위 |
| --- | --- | --- |
| Pair 시작 | `UCExecutionCollaborationComponent::RequestCombatExecution()` | Source 상태, Target Snapshot/Revision·기하, Target 조건을 검증한다. |
| 예약·활성화 | Target `AcceptExecutionReservation`, Balance opportunity reservation, pair activate | 양측 실행 전 Pair session과 reservation을 만든다. |
| Commit | Source Action event → `HandleActionEvent()` → `HandleSourceExecutionCommit()` | session/snapshot/opportunity 검증 뒤 Target으로 Outcome을 전달한다. |
| Target 접점 | `UCCombatSignalTargetComponent::RequestExecutionOutcomeTarget(FExecutionOutcomePacket)` | 일반 `HandleDefaultDamageEvent()`와 다른 Pair 전용 Outcome 경로다. |
| 적용·통지 | `TryResolveExecutionAppliedDamage` → Collaboration `CommitExecutionOutcome` → `Health::TakeDamage` | Target 적용 피해 경계는 재사용하되 Pair 거래 소유권은 Collaboration Component에 남는다. |
| 취소 | Commit 전 cancellation / Balance reservation release / Loop TTL resume | Commit 전에는 해제·재개, Commit 후에는 이전 Loop로 rollback하지 않는다. |
| 문서 기준 | `S36_UE5_Portfolio_Execution_Collaboration_Architecture.md` §13–14 | Combat Signal과의 경계·표현 제한을 우선한다. |

## 3. 페이지 구성

### 구조 문제 — 단방향 전달만으로는 양측 준비·예약·취소의 소유자를 정할 수 없음

일반 Combat Signal은 Source가 사실을 전달하고 Target이 독립 소비한다. 그러나 처형은 Source Action, Target Reaction, Target Opportunity와 Commit 전 취소가 같은 Session으로 함께 성립해야 한다. 단일 Packet 처리로 넣으면 reservation 소유자와 Release 시점이 분산되고, 한쪽 terminal만으로 Pair를 완료할 수 없다.

### 중앙 다이어그램 — Pair Transaction

사용자 제작 다이어그램을 `Diagram/PP/ExecutionPairCoordination.png`로 준비해 `Assets/Diagrams`로 복사해 반영한다. Source/Target Actor와 중앙 `Pair Contract (양측 Collaboration)` lane을 사용한다. 중앙은 singleton이 아니라 각 Actor의 `UCExecutionCollaborationComponent`가 같은 Session/Context를 대조하는 계약 경계다.

```text
Source Request ─────→ Pair Contract ─────→ Target validation
                      ← reservation accepted
Reserved → Active → Source Action / Target Reaction start
          Commit Notify → Target Pair Commit 확인 → Health apply
          terminal correlation → Complete

Commit 전 실패/중단 → reservation Release → Collapse Loop TTL resume
```

`동시에 실행된다`는 프레임 단위 표현 대신, `Reserved → Active → Commit → terminal 또는 Release` 계약을 표시한다.

### 구조 해결 — Pair 계약이 reservation·commit·release의 책임을 소유

| 책임 경계 | 확정 내용 |
| --- | --- |
| Source Actor | 처형 요청과 자신의 Action Commit을 전달한다. Target HP나 reservation을 직접 변경하지 않는다. |
| Pair Contract | 양측 Collaboration Component가 Session·Target Snapshot·Opportunity Reservation·Active/Committed 상태와 취소 Release를 대조한다. |
| Target Boundary | Target은 Reaction과 Outcome 적용 가능 여부를 확인한다. `UCCombatSignalTargetComponent`는 수락된 Outcome의 HP 적용 경계만 제공한다. |

E15a/b/c는 Evidence Ledger에서 계속 `Needs Capture`로 유지한다. HTML에는 런타임 검증 카드나 반복 문제 해결 경험을 두지 않으며, 확보 전 성공·거절·Release 결과를 단정하지 않는다.

## 4. 증거 상태와 금지 표현

| 증거 | 상태 | 사용 규칙 |
| --- | --- | --- |
| Pair session/reservation/commit/release 코드 | Code Confirmed | 주 주장 근거 |
| S36, C08 | Documented | 구조·한계 설명 근거 |
| p.9 gameplay Hero | 보조 | p.15 구조 성공 증거로 대체 금지 |
| E15a/b/c | Needs Capture | 성공·거절·Release 결과는 확보 후에만 단정 |

금지 표현:

- Execution Pair가 Combat Signal의 특수 Packet 또는 하위 분기다.
- 모든 협업 사례를 지원한다.
- 프레임 단위로 완전히 동시 실행된다.

## 5. 완료 조건

- [ ] E15a/b/c로 성공·거절·Commit 전 Release를 각각 확인한다.
- [x] p.9 Hero와 p.15 구조 설명의 역할이 중복되지 않도록 분리했다.
- [x] 사용자 제작 `Execution Pair Coordination.png`를 `p15_execution_pair_coordination.png`로 반영했다.
- [ ] 25페이지 PDF에서 p.15 다이어그램 가독성을 검토한다.
- [x] 목차·footer·후속 p.16–p.25 번호를 25페이지 기준으로 검증했다.
