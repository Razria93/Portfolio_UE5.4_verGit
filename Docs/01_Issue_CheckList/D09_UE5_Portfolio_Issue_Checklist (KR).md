# UE5 Portfolio – Issue Checklist

## 제목

**M2-04: TakeDamage 수신부 구현 (오버라이드 + 임시 검증 피드백)**

### 날짜

- **Day 9**

- **Date : 2026.01.06**


---

### 목표

- `CEnemy::TakeDamage(...)` 오버라이드를 통해 타깃 수신 진입점을 구현하고, `CApplyDamageComponent`(M2-03) 디스패치가 정상적으로 도달하는지 검증함.

- `FDamageEvent`은 `DamageEvent.GetTypeID()` 기반의 명시적 분기 유효성을 확인함.

- `CEnemy::TakeDamage(...)`에서는 최소한의 유효성만 체크한 뒤, `UCTakeDamageComponent`에 처리를 위임함.

- `UCTakeDamageComponent::HandleTakeDamage(...)`에서 피격자 상태 기반 보정/검증을 수행함.

- 최종 결과는 `CHealthComponent`와 `CReactionComponent`에서 처리하며, 본 마일스톤에서는 최소 구현(HP 감소 + 최소 HitReaction)만 확정함.


---

### 브랜치

- feature/combat-take-damage


---

### TODO List

#### 1. CEnemy::TakeDamage 수신 진입점(오버라이드)

- [ ] `CEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)` 오버라이드

- [ ] 최소 유효성 검증만 수행(Null / Self / Amount / 필수 포인터 등) 후 즉시 위임

- [ ] `DamageEvent.GetTypeID()` 기반으로 이벤트 타입을 명시적으로 분기 처리  
  - Default 경로 처리(기본 이벤트)  
  - Custom Event Id 사용 시 해당 경로 포함

- [ ] 위임 호출: `UCTakeDamageComponent`를 찾아 `HandleTakeDamage(...)`로 전달


#### 2. UCTakeDamageComponent 구성 및 처리 파이프라인

- [ ] `UCTakeDamageComponent` 클래스 생성 및 Enemy에 부착/초기화 경로 확보

- [ ] 컴포넌트 엔트리 포인트 정의: `HandleTakeDamage(...)`
  - 입력: `DamageAmount`, `DamageEvent`, `EventInstigator`, `DamageCauser`  
  - 출력: 최종 적용 데미지(또는 처리 결과 구조)

- [ ] `HandleTakeDamage(...)`에서 수신자 상태 기반 보정/검증 단계 정의
  - 처리 예: 무적/가드/방어력/상태이상/피격 불가 (해당 이슈에서는 최소 구현만 진행)


#### 3. Health 처리(CHealthComponent) 최소 구현 연결

- [ ] `CHealthComponent` 생성 또는 기존 컴포넌트 연결

- [ ] HP 감소 API 확정(예: `TakeDamageToHealthPoint(float FinalDamage)`)

- [ ] HP 감소 및 Dead 플래그 처리(HP <= 0) 연결

- [ ] 감소 전/후 로그 출력으로 적용 결과 검증


#### 4. Reaction 처리(CReactionComponent) 최소 구현 연결

- [ ] `CReactionComponent` 생성 또는 기존 컴포넌트 연결

- [ ] 최소 HitReaction API 확정  
  - 예: `OnHitReact(const FHitReactContext&)` 또는 `PlayHitReaction(...)`

- [ ] 본 이슈 범위에서는 최소 반응만 구현  
  - 로그 출력(필수)  


#### 5. 구조화 로그 및 디버그 출력(검증 피드백)

- [ ] `TakeDamage` 수신 로그 1회/호출 규칙 고정(중복 출력 방지)

- [ ] 로그 포맷에 최소 포함 항목 정의 및 통일  
  - Target, DamageAmount  
  - EventInstigator, DamageCauser  
  - DamageEvent TypeID(+ Custom Id / SpecKey 존재 시 출력)


#### 6. M2-03 연동 검증 시나리오

- [ ] `CApplyDamageComponent::ApplyDamageToTarget(...)` → `CEnemy::TakeDamage(...)` 연결 확인

- [ ] `CEnemy::TakeDamage(...)` → `UCTakeDamageComponent::HandleTakeDamage(...)` 위임 호출 확인

- [ ] `UCTakeDamageComponent` → `CHealthComponent` / `CReactionComponent` 호출 확인

- [ ] 단일 히트 이벤트 기준 호출 빈도 점검(의도치 않은 중복 호출 여부 확인)

- [ ] 다중 타깃 시나리오 점검(테스트 레벨에 Enemy 다수 배치)


---

### Notes
- 


---
