# p.14 Target Outcome Resolution 프로젝트 감사 및 구성 계획

> 상태: **코드·설계 문서 감사 완료 / Target Outcome 다이어그램 반영 / 제출용 동일 조건 비교 캡처 대기**  
> 기준: 현행 포트폴리오와 HTML은 25페이지다.

## 1. 페이지 목적

p.14는 별도 상호작용 구조가 아니다. p.13 Source Boundary에 이어지는 **단방향 Combat Signal의 Target 소비·판정 단계**를 설명한다.

핵심 주장은 `Incoming Packet + Target State + Target Rule → Outcome`이다. Target이 Damage 감쇠와 Reaction 종류를 확정하므로, Damage 흐름이 Action/Reaction Orchestrator로 역류하지 않는다.

## 2. 코드·문서 대조

| 항목 | 현행 확인 경로 | 확정 범위 |
| --- | --- | --- |
| Default Damage 진입 | `UCCombatSignalTargetComponent::HandleDefaultDamageEvent()` | Payload/Context 생성 뒤 Target 검증을 시작한다. |
| 결과 해석 | `ValidateContext` → pre-state → `CanReceiveCombatSignal` → `ComputeTargetDamage` | Packet과 Target 상태·규칙을 함께 해석한다. |
| 적용·결과 | `CommitCombatSignalTarget` → `ResolveDamageReactionOutcome` → `FCombatSignalTargetPacket` | Health commit, Defense/Reaction Outcome, dispatch를 Target에서 순서대로 처리한다. |
| Guard / Parry | `EDamageDefenseOutcome`, target packet | Guard는 감쇠 후 commit하며 생존 시 `BlockHit`가 된다. Dead 전이는 우선한다. Parry는 damage commit 없이 accepted packet/`Parry` outcome이 될 수 있다. |
| Source 상태 제한 | `UCCombatSignalSourceComponent::CommitCombatSignalSource()` | Source의 0 damage `CommitFailed`만으로 Target Parry 실패를 판단하지 않는다. |
| 문서 기준 | `S38_UE5_Portfolio_Combat_Signal_Architecture.md` §5, §8 | 구현 근거와 제출 표현 제한을 동일하게 사용한다. |

## 3. 페이지 구성

### 기본 페이지 구성 — 구조 문제 → Target Outcome Flow → 구조 해결

1. 구조 문제: Source 또는 Orchestrator에 Defense·Damage·Reaction Outcome 판단을 두었을 때 Target 상태를 모르거나 실행 판단 책임이 역류하는 이유를 callout으로 제시한다.
2. 중심 다이어그램: 사용자 제작 `Diagram/PP/TargetSource.png`를 `Assets/Diagrams/p14_target_outcome_resolution.png`로 복사해 삽입했다. Target Context snapshot → 수신·입력·방어 정책 → Normal/DamageOnly/Guard/Parry outcome → Target Packet dispatch 순서를 표시한다.
3. 구조 해결: Target Context, Outcome Resolution, Result Dispatch의 책임 3칸과 E14 표현 제한 메모로 끝낸다.

```text
Incoming Damage Request
  → Target Context (HP / Dead / External Input Policy)
  → Can Receive / Defense Policy
  → Commit 여부 + Reaction Outcome
  → TargetPacket / Reaction / Feedback / Result Receiver
```

Source 흐름과 Pair 거래는 이 페이지의 주 시각화에 넣지 않는다.

E14는 같은 계열 hit의 Target Packet·Defense Outcome·Committed Damage·HP 전후·Reaction Outcome 비교를 위한 Ledger 항목으로만 유지한다. 기존 FinalCandidate는 이 비교의 완료 증거로 사용하지 않는다.

## 4. 증거 상태와 금지 표현

| 증거 | 상태 | 사용 규칙 |
| --- | --- | --- |
| Target Outcome 코드 경로 | Code Confirmed | 주 주장 근거 |
| S38, J01/N05/N06, B12 | Documented | 설계 이유·보조 근거 |
| FinalCandidate | 후보 | 제출용 결과 검증으로 승격 금지 |
| E14 Target Packet 비교 캡처 | Needs Capture | 확보 뒤에만 Normal/Guard/Parry 결과 표를 완료로 표기 |

금지 표현:

- Source가 Target Reaction 종류를 직접 결정하거나 실행한다.
- Parry의 Source `CommitFailed`가 Target 처리 실패를 뜻한다.
- 모든 Packet 종류가 범용 Handler 객체로 이미 분리됐다.

## 5. 완료 조건

- [ ] E14에서 Target Packet, Defense Outcome, HP 전후, Reaction/Result를 같은 조건으로 비교한다.
- [x] p.14 본문에서 `런타임 검증`과 별도 문제 해결 경험 카드를 제거하고, E14를 Evidence Ledger에서만 추적한다.
- [x] p.13 Source Boundary, p.15 Execution Pair와 다이어그램·주장 중복을 렌더링 기준으로 검토했다.
