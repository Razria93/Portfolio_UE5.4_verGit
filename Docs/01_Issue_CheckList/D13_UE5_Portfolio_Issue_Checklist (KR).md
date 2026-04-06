# UE5 Portfolio Issue Checklist

## 제목

**M03-03: Combat Core Shared 규칙 정리**

### 날짜

- **Day 13**

- **Date : 2026.04.01**


---

### 목표

- 공격자와 피격자가 공유하는 전투 코어를 정리하고, 공통 데미지 처리 규칙을 확정함.

- `HitContext -> ApplyDamage -> DefaultDamageEvent -> TakeDamage -> Health -> Reaction` 흐름에서 각 단계의 책임과 최소 정책을 정리함.

- Player / Enemy 구분 없이 동일한 규칙으로 데미지가 처리되도록 구조를 정리함.


---

### 브랜치
- `feature/combat-core-shared`


---

### TODO List

#### 1. ApplyDamage 단계 책임 정리

- [x] `ApplyDamage` 단계 입력/출력 책임 정리

- [x] `FApplyDamageSpecKey` 기반 spec 조회 정책 확정

- [x] spec miss 처리 정책 정리

- [x] 최소 디버그 로그 흐름 정리


#### 2. 공격 윈도우 및 중복 타격 규칙 정리

- [x] hit window 기반 공격 윈도우 식별 규칙 추가

- [x] 동일 공격 윈도우 내 중복 타격 방지 정책 추가

- [x] self-hit 금지 규칙 유지

- [x] target 중복 overlap 처리 정책 정리

- [x] hit window open / close 후속 정책 정리


#### 3. TakeDamage 단계 책임 정리

- [x] `Requested / Mitigated / FinalTaken / Committed` 의미 확정

- [x] request reject 조건 정리

- [x] accepted / rejected 후처리 방향 정리

- [x] dead target 처리 정책 정리


#### 4. Reaction 연결 규칙 정리

- [x] reaction 진입 최소 조건 정리

- [x] damage result -> reaction request 연결 규칙 정리

- [x] death 전이와 reaction 우선순위 기준 정리


#### 5. 통합 검증
- [x] 시나리오 1: 동일 규칙으로 Player / Enemy 데미지 처리 확인

- [x] 시나리오 2: 동일 공격 흐름 내 중복 타격 방지 확인

- [x] 시나리오 3: 공격 시작부터 첫 타까지 데미지 처리 흐름 정상 동작 확인

- [x] 시나리오 4: 피격 후 reaction / dead 전이 포함 후속 처리 흐름 확인

- [x] 시나리오 5: dead target 추가 데미지 처리 정책 확인


---

### 현재 정리 결과

- `ApplyDamage`와 `TakeDamage`는 공통적으로 `Payload / Context / Result` 흐름으로 정리되었음.

- hit window 기반 중복 타격 방지, invalid request reject, dead target 방어 정책이 반영되었음.

- `Reaction`은 `CommittedDamage`와 dead-state 전후 조건을 기준으로 연결되도록 정리되었음.


---

### Notes

- 본 이슈는 고급 수치 설계보다 **전투 공통 규칙과 최소 정책 정리**를 목표로 함.

- 이후 Guard, Armor, Resistance 같은 수치 확장 요소는 현재 정리된 shared combat core 위에 확장 가능하도록 설계함.

---

### Follow-Up TODO

  - Team 식별 구조 도입 여부 결정

  - Friendly Fire 허용 여부 및 판정 정책 결정

  - Guard / Armor / Resistance 등 receiver-side 확장 정책 연결


---
