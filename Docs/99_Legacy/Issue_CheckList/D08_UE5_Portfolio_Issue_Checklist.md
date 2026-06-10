# UE5 Portfolio – Issue Checklist

## 제목

**M02-03: ApplyDamageComponent 및 ApplyDamage Pipeline 구현**

### 날짜

- **Day 8**

- **Date : 2026.01.01**

---
### 브랜치

- feature/combat-apply-damage

---
### 목표

- ApplyDamage 처리는 `CApplyDamageComponent` 한 곳에서만 수행하도록 정리함.

- ApplyDamage 요청은 `RequestApplyDamage` 한 함수로만 받고, `CalculateDamage`에서 명확한 SpecKey/Result 구조를 사용해 데미지를 계산함.

- 최종 결과는 `ApplyDamageToTarget` 한 지점에서만 타깃에게 전달하며, 이 함수에서 엔진 `TakeDamage(...)`를 호출함(수신부 구현은 M02-04에서 처리).

---
### TODO 리스트

#### 1. ApplyDamage 컴포넌트 베이스

- [x] `CApplyDamageComponent` 생성

- [x] 단일 엔트리 포인트 API 정의 (`RequestApplyDamage(...)`)

- [x] 요청 유효성 검증(Attacker / DamageCauser / Target / HitComponent 등) 및 Invalid 입력 조기 반환(Early-return) 규칙 적용

#### 2. 데미지 연산 (CalculateDamage)

- [x] 연산 입력 구조 최소 정의(예: `DamageContext`, `SpecKey`)

- [x] 최소 연산 경로 구현(`BaseDamage → FinalDamage`) 및 명시적 출력 구조(`DamageResult`) 구성

- [x] 연산 단계에서 타깃 변형 금지(연산은 Side-effect-free 유지)

#### 3. 디스패치 경계 (엔진 `TakeDamage` 연동)

- [x] `Target->TakeDamage(...)` 호출을 단일 지점(`ApplyDamageToTarget(...)`)으로 고정

- [x] 호출 경로는 구조화된 로그로만 검증함(`TakeDamage` 수신 구현은 M02-04, HP/UI는 M02-05에서 처리)

---
### 비고

- 

---
