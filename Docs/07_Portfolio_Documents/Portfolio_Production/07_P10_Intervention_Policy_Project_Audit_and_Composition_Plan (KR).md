# 현행 p.12 Intervention Policy 프로젝트 감사 및 구성 계획

> 파일명은 초기 22페이지 체계의 감사 기록을 보존한다. 현행 25페이지 체계에서는 p.12에 적용한다.

> 상태: **코드·설계 문서 감사 완료 / 정책 다이어그램·실제 적용 사례 HTML 반영 / 런타임 캡처 대기**  
> 목적: p.12은 실행 요청의 수락 여부, 실행 관계, 적용 방식이 서로 다른 책임임을 설명한다. p.11의 공통 전환 계약 자체나 p.7의 Dodge 기능 사례를 반복하지 않는다.

## 1. 확인 근거

- `Docs/05_System_Architecture/S22_UE5_Portfolio_System_Architecture.md`
  - 실행 가능 여부, 관계, 적용 방식을 `Accept / Reject / Ignore`, `Independent / Sequential / Exclusive`, `Start / Reserve / Intervene`으로 분리한 설계 근거.
- `Docs/05_System_Architecture/S23_UE5_Portfolio_System_Architecture.md`
  - cross-domain intervention은 stop 호출이 아닌 대상·사유·후속 동작을 담은 Directive여야 한다는 근거.
- `Source/Portfolio/Component/CActionOrchestratorComponent.cpp`
  - `ResolveExecutionApplyMode()`의 Independent=Start, Sequential=Reserve, Exclusive=Intervene 흐름.
  - `ResolveInterventionDirective()`의 incoming Want / active Allow 검증과 Directive 생성.
- `Source/Portfolio/Component/CReactionOrchestratorComponent.cpp`
  - Reaction의 Independent·Exclusive 처리와 incoming Want / active Allow 검증.
  - Reaction Component는 Reserve를 지원하지 않으며, DeadReaction은 permission query를 우회하는 terminal 예외.

## 2. 안전한 주장 범위

1. `Reject / Ignore`는 관계·적용 방식으로 진입하지 않는 종료 결과다.
2. `Sequential`은 Action의 Combo 연결을 위한 `Reserve → Notify Consume` 흐름이다. Reaction에는 Reserve 적용을 주장하지 않는다.
3. `InterventionDirective`는 `Exclusive` 관계에서 incoming Want와 active Allow가 성립한 경우에만 생성된다고 표현한다.
4. DeadReaction의 강제 개입은 일반 Want / Allow 정책의 예외이며, 본문의 일반 정책 흐름에 섞지 않는다.

## 3. 페이지 구성

### 중심 시각 자료 — 정책을 세 축으로 분리

최종 삽입 이미지: `Assets/Diagrams/p12_intervention_policy.png`를 HTML에 반영했다. p.12의 기본 읽기 순서는 **구조 문제 → 정책 흐름 다이어그램 → 실제 적용 사례 → 구조 해결**이며, p.11과 달리 코드·문서로 확인된 이전 구현의 Before/After는 제시하지 않는다.

```text
Snapshot + Incoming/Active
        ↓
Decision: Accept ───────────────→ Relationship → ApplyMode
          Reject / Ignore → 종료(Apply 없음)
```

### 보조 자료 1 — 관계별 실제 적용

| 관계 | 조건 | 결과 | 범위 |
| --- | --- | --- | --- |
| Independent | Idle + active 없음 | Start | Action / Reaction |
| Sequential | Action 흐름의 연속성 | Reserve → Notify Consume | Action 전용, Reaction Reserve 미지원 |
| Exclusive | active 존재 + 정책 확인 | Want + Allow → Directive → Intervene / 아니면 Reject | Action / Reaction |

### 보조 자료 2 — Directive 필드

`Exclusive + Want / Allow 수락 시 생성되는 Intervention Directive`

- TargetDomain: 무엇을 멈추는가
- StopSource: 누가 결정했는가
- StopReason: 왜 멈추는가
- AfterStopAction: 다음에 무엇을 하는가

### 구현 제한 메모

- DeadReaction은 terminal semantics를 위해 participant permission을 거치지 않는 예외다.
- Reaction Component는 Reserve 적용을 지원하지 않는다.

## 4. 검수 기준

- p.11의 Candidate·Context·Query 계약을 재도식화하지 않는다.
- p.7의 Dodge 시나리오를 결과 사례로 반복하지 않는다.
- Relationship, ApplyMode, Directive의 조건이 섞이지 않게 한다.
- 25페이지 HTML에 `p12_intervention_policy.png` 정책 다이어그램을 반영했다. E12(B09 범위) 제출 캡처 전에는 정책 코드·문서 근거로만 한정한다.

## 5. 25페이지 전환 기준 (2026-09-06)

- p.12는 S37의 Intervention 정책 상세 페이지다. `런타임 검증`과 별도 문제 해결 경험 카드는 배치하지 않으며, Evidence Ledger의 E12(B09) 항목으로만 런타임 증거를 추적한다.
- B09는 Want/Allow 및 directive 기반 중단 사례의 보조 근거다. Dead 강제 개입은 일반 Want∧Allow 규칙의 예외로 별도 표기한다.
