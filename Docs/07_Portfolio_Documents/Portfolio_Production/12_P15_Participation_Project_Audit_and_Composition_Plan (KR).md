# 문서상 p.18 Participation 프로젝트 감사 및 구성 계획

> 파일명은 초기 22페이지 체계의 감사 기록을 보존한다. 현행 25페이지 체계에서는 p.18에 적용한다.

> 상태: **코드·설계 문서 감사 완료 / 사용자 제작 Participation Contract 다이어그램 HTML 반영 완료**

## 확인 근거와 주장 범위

- `UCWorldSubsystem_CombatParticipation`은 Perception 및 HitReactive Evidence를 Participant × Target 단위로 보관하고, active Evidence로 Candidate를 구성해 Assignment를 갱신한다.
- `Assignment != None`만이 전투 참여 상태의 권위이며, `None`은 Combat Target을 clear하는 비전투 상태다.
- 모든 Candidate는 General Base Engage를 우선 시도하고, Base가 가득 찼을 때만 live HitReactive Evidence가 있는 Candidate가 Extra Engage admission을 사용할 수 있다.
- 마지막 live Evidence가 종료하면 Candidate와 Assignment를 제거한다. 다만 정확히 일치하는 진행 Engage Action lock은 Action 종료 또는 timeout까지 기존 Assignment만 임시 보존한다.

## 페이지 구성

### 구조 문제 — Enemy가 인지 결과로 개별 참여를 판단하면 역할 배분이 불안정해짐

Perception과 HitReactive는 참여 판단의 근거일 뿐이다. 역할·정원·해제를 각 Enemy가 독립 처리하면 동일 Target의 교전 인원과 Evidence 종료 시점이 일관되지 않는다. Evidence 종료만으로 진행 중인 정확한 Action을 즉시 해제하면 실행 수명과 참여 수명도 충돌한다.

### 중앙 다이어그램 — Participation Contract | Evidence → Assignment → Snapshot

사용자 제작 `Diagram/PP/Participation.png`를 `Assets/Diagrams/p18_participation.png`로 복사해 반영했다.

```text
Perception / HitReactive Evidence
  → Participant × Target Candidate
  → Assignment Rebuild
  → None / Observe / Alert / Engage
  → Applied Snapshot

Evidence exhausted → exact Assignment Lock → Action terminal 또는 timeout → release
```

General Base Engage 뒤에 live HitReactive Evidence가 있는 Candidate만 Extra admission을 받을 수 있음을 표시한다. Action lock은 새 참여를 만드는 상태가 아니라, 정확히 일치한 기존 Assignment의 임시 보존으로만 표현한다.

### 구조 해결 — Evidence·Assignment·Applied Snapshot을 분리해 참여 수명 조율

| 책임 경계 | 확정 범위 |
| --- | --- |
| Evidence Ingress | Perception·HitReactive를 Participant × Target 단위로 기록·갱신한다. Evidence는 Engage 권한을 직접 만들지 않는다. |
| Assignment Kernel | World Subsystem이 live Evidence와 정원으로 None·Observe·Alert·Engage를 할당한다. Engage는 General Base 뒤에 HitReactive Extra 예외를 적용한다. |
| Applied Snapshot / Release Guard | Participant Component는 Assignment Snapshot을 소비해 필요한 투영을 반영한다. Evidence 종료 뒤에는 정확히 일치한 Action lock만 terminal 또는 timeout까지 임시 보호한다. |

HTML에는 런타임 검증 카드나 별도 문제 해결 경험을 두지 않는다. Runtime Capture 전에는 Extra admission 또는 release 결과를 성공 사례로 단정하지 않는다.

## 표현 제한

- Combat Target의 Source of Truth와 Revision·수명 계약은 p.17에서만 상세하게 설명한다.
- AIIntentState의 판단 규칙과 BT Task 동작은 p.16의 책임으로 남긴다.
- Action lock은 참여 상태를 새로 생성하는 독립 수명주기가 아니라, 이미 진행 중인 정확한 Assignment를 한시적으로 보호하는 범위로만 표현한다.
