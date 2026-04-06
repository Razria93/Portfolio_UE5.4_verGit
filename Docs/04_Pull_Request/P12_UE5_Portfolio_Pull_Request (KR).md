# Combat Core Shared 규칙 정리 및 데미지 파이프라인 공통화

## 제목

`♻️ refactor: combat-core-shared 규칙 정리 및 데미지 파이프라인 공통화`

## 요약

- 본 PR은 공격자와 피격자가 공유하는 전투 코어를 정리하고, `ApplyDamage -> DefaultDamageEvent -> TakeDamage -> Health -> Reaction` 흐름을 공통 규칙 기준으로 재구성한 작업을 포함함.

- `ApplyDamage`와 `TakeDamage`를 각각 `Payload / Context / Result` 흐름으로 정리하여, sender-side / receiver-side 책임을 명확히 분리함.

- attachment hit window 개념을 도입하여 동일 공격 윈도우 내 중복 타격 방지 규칙을 추가하고, 첫 overlap 시점 `HitWindowId`가 `INDEX_NONE(-1)`로 전달되던 버그를 함께 수정함.

- 또한 dead target 보호, reaction 연결 규칙, 관련 이슈 체크리스트 및 버그 리포트 문서를 함께 정리함.


---

## 완료된 항목

### 1. Attachment Hit Window 규칙 정리

- `ACAttachment`에 hit window lifecycle 추가

- `CurrentHitWindowId` 기반 overlap metadata 전달 구조 정리

- `CollisionEnabled()` / `CollisionDisabled()` 기준 hit window open / close 흐름 정리

- 첫 overlap 이전에 유효한 `HitWindowId`가 준비되도록 collision enable 순서 보정

### 2. ApplyDamage 파이프라인 공통화

- `UCApplyDamageComponent`를 `ValidateRequest -> BuildPayload -> BuildContext -> ValidateContext -> CanApplyDamage -> ResolveApplyDamageSpec -> ComputeApplyDamage -> CommitApplyDamage -> BuildResult` 흐름으로 정리

- `FApplyDamagePayload`, `FApplyDamageContext`, `FApplyDamageAmount`, `FApplyDamageResult` 구조 도입

- `FApplyDamageSpecKey` 기반 spec 조회 및 spec miss 처리 정책 정리

- self-hit 방지, duplicate-hit 방지, invalid request reject 경로 정리

### 3. TakeDamage 파이프라인 공통화

- `UCTakeDamageComponent`를 `ValidateRequest -> BuildPayload -> BuildContext -> ValidateContext -> CanTakeDamage -> ComputeTakeDamage -> CommitTakeDamage -> BuildResult` 흐름으로 정리

- `Requested / Mitigated / FinalTaken / Committed` 의미를 receiver-side 기준으로 정리

- dead target reject 및 `CommittedDamage` 기준 후처리 흐름 정리

- accepted / rejected dispatch 흐름 정리

### 4. Reaction 연결 규칙 정리

- `CommittedDamage > 0` 조건 기반 reaction 연결 정리

- dead-state 전후 조건을 함께 반영하여 death 전이와 reaction 우선순위 흐름 정리

- 연속된 hit 상황에서 reaction replace / interrupt 흐름 검증

### 5. 문서 정리

- `D13_UE5_Portfolio_Issue_Checklist (KR)` 업데이트

- `D13_UE5_Portfolio_Issue_Checklist (EN)` 업데이트

- `B05_UE5_Portfolio_Bug_Report (KR)` 추가

- `B05_UE5_Portfolio_Bug_Report (EN)` 추가


---

## 테스트 방법

1. Player와 Enemy 양쪽에서 동일한 공격 흐름으로 `ApplyDamage -> TakeDamage`가 동작하는지 확인

2. 동일 공격 윈도우 내에서 target 중복 타격이 방지되는지 확인

3. 첫 overlap 시점에 `HitWindowId`가 유효하게 전달되어 첫 타가 `InvalidRequest`로 리젝트되지 않는지 확인

4.  ApplyDamageResult와 TakeDamageResult의 Request / FinalTaken / Committed 값이 로그 기준으로 일관되게 연결되는지 확인

5. HP가 0에 도달했을 때 `CommittedDamage`가 남은 HP만큼만 반영되는지 확인

6. dead / dying 이후 추가 타격이 `CommittedDamage = 0`으로 처리되며 추가 HP 감소가 발생하지 않는지 확인


---

## 관련 이슈 / 브랜치

- 브랜치: `feature/combat-core-shared`

- 관련 작업:

  - `M03-03: Combat Core Shared 규칙 정리 (#34)`

  - `M3-B05: CurrentHitWindowId 증가 타이밍 문제로 인한 첫 타격 InvalidRequest 리젝트 버그 수정 (#35)`


---

## 노트

- 본 PR의 초점은 수치 설계보다 **전투 공통 규칙, 책임 분리, 파이프라인 정리**에 있음.

- Team / Friendly Fire, Guard / Armor / Resistance 같은 확장 정책은 현재 shared combat core 위에서 후속 작업으로 확장 예정임.


---
