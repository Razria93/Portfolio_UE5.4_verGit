# UE5 Portfolio – Issue Checklist

## 제목

**M03-03: Combat Core Shared 규약 정리**

### 날짜

- **Day 13**

- **Date : 2026.04.01**


---

### 목표

- 공격자와 피격자가 공유하는 전투 코어를 정리하고, 공통 데미지 처리 규약을 확정함.

- `HitContext -> ApplyDamage -> DefaultDamageEvent -> TakeDamage -> Health -> Reaction` 흐름에서 각 단계의 책임과 최소 정책을 정리함.

- 플레이어/Enemy 구분 없이 동일한 규칙으로 데미지가 처리되도록 구조를 정리함.


---

### 브랜치

- `feature/combat-core-shared`


---

### TODO List

#### 1. ApplyDamage 단계 책임 정리

- [ ] `ApplyDamage` 단계 입력/출력 책임 정리

- [ ] `FApplyDamageSpecKey` 기준 spec 조회 정책 확정

- [ ] spec miss 처리 정책 정리

- [ ] 최소 디버그 로그 흐름 정리


#### 2. 중복 타격 및 공격 규칙 정리

- [ ] 동일 공격 윈도우 내 중복 타격 방지 정책 추가

- [ ] self-hit 금지 규칙 유지

- [ ] target 중복 overlap 처리 정책 정리

- [ ] stop damage / overlap end 후속 정책 정리


#### 3. Team / Friendly Fire 정책 정리

- [ ] 팀 판정 구조 도입 여부 결정

- [ ] friendly fire 허용 여부 결정

- [ ] 미구현 시 stub 정책 반영


#### 4. TakeDamage 단계 책임 정리

- [ ] `Requested / Mitigated / FinalTaken / FinalApplied` 의미 고정

- [ ] request reject 조건 정리

- [ ] accepted / rejected 후처리 방향 정리

- [ ] dead 대상 처리 정책 정리


#### 5. Reaction 연결 규칙 정리

- [ ] reaction 진입 최소 조건 정리

- [ ] damage result -> reaction request 연결 규칙 정리

- [ ] dead 전이와 reaction 우선순위 기준 정리


#### 6. 통합 검증

- [ ] 시나리오 1: 동일 규칙으로 Player/Enemy 데미지 처리 확인

- [ ] 시나리오 2: 동일 공격 윈도우 중복 히트 방지 확인

- [ ] 시나리오 3: invalid request reject 확인

- [ ] 시나리오 4: dead 대상 추가 데미지 처리 정책 확인


---

### Notes

- 본 이슈는 고급 수치 설계보다 **전투 공통 규약과 최소 정책 정리**를 목표로 함.

- 이후 Guard, Armor, Resistance, Team, Friendly Fire 확장은 이 규약 위에서 확장 가능하도록 설계함.


---
