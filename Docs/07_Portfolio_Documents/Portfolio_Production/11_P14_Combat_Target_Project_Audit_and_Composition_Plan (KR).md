# 문서상 p.17 Combat Target 프로젝트 감사 및 구성 계획

> 파일명은 초기 22페이지 체계의 감사 기록을 보존한다. 현행 25페이지 체계에서는 p.17에 적용한다.

> 상태: **코드·설계 문서 감사 완료 / 사용자 제작 Target State Contract 다이어그램 HTML 반영 완료**

## 확인 근거와 주장 범위

- `UCCombatTargetComponent`는 `CurrentTarget`(weak reference), `CombatTargetRevision`, `LastChangeReason`, `OnCombatTargetChanged`를 Runtime 상태로 소유한다.
- `RequestSetCombatTarget` / `RequestClearCombatTarget`은 실제 변경에만 Revision을 증가시키며, 동일 Target 요청은 No-op으로 처리한다.
- Component는 Target의 EndPlay와 stale weak reference를 정리하고, Snapshot으로 Target·Revision·변경 사유를 함께 제공한다.
- Blackboard는 Combat Target의 Source of Truth가 아니라 AI 판단을 위한 단방향 투영값이다. Lock-on, HUD, Action Facing도 상태를 직접 대체하지 않는 Consumer다.

## 페이지 구성

### 구조 문제 — 대상이 여러 곳에 분산되면 교체·소멸·늦은 Clear에서 기준이 갈라짐

선택 정책, AI, Lock-on, HUD, Facing이 각자 Target을 보관하면 동일한 교체와 소멸을 다르게 해석할 수 있다. 이전 Target 기준의 늦은 Clear가 새 Target을 지우지 않도록, 변경 세대와 수명 정리를 Target 상태 계약에 포함한다.

### 중앙 다이어그램 — Target State Contract

사용자 제작 `Diagram/PP/Combat Target.png`를 `Assets/Diagrams/p17_combat_target.png`로 복사해 반영했다.

```text
Decision Request
  → Combat Target Kernel
    (CurrentTarget / Revision / Lifecycle)
  → Snapshot / Changed Event
  → Blackboard / Lock-on·HUD / Action Facing

Expected Target + Expected Revision 일치 → Conditional Clear
불일치 → No-op
```

### 구조 해결 — 결정·상태·소비를 Target State Contract로 분리

| 책임 경계 | 확정 범위 |
| --- | --- |
| Decision Request | 선택·참여·수명 정책은 Set / Clear를 요청한다. Target 상태를 직접 보관하거나 변경하지 않는다. |
| Combat Target Kernel | CurrentTarget·Revision·변경 사유·EndPlay/stale 정리·변경 Event를 소유한다. 동일 Target 요청은 No-op이다. |
| Snapshot Consumer | Blackboard, Lock-on/HUD, Facing은 Snapshot·Event를 읽어 반영한다. 조건부 Clear는 Target·Revision이 일치할 때만 허용한다. |

HTML에는 런타임 검증 카드나 별도 문제 해결 경험을 두지 않는다. Runtime Capture 전에는 Target 교체·소멸 처리를 성공 사례로 단정하지 않는다.

## 표현 제한

- p.4의 후보 수집·점수·Lock-on 입력 정책을 이 페이지에서 다시 설명하지 않는다.
- Blackboard가 Target을 직접 확정하거나 Combat Target 상태를 소유하는 것처럼 표현하지 않는다.
- Enemy Participation의 Evidence·할당·정원 정책은 p.18에서만 상세하게 다룬다.
