# UE5 Portfolio – Issue Checklist

## 제목

**M01-05: 라이트 공격 1타 시스템 구현**

### 날짜

- **Day 5**

- **Date : 2025.12.18**


---

### 목표

- 라이트 공격 1타 입력 / 트리거 로직 구현

- 라이트 공격 애니메이션 및 몽타주 재생 로직 구성

- AnimNotify 기반 히트 타이밍 및 공격 상태 처리 플로우 1차 정리


---

### 브랜치

- feature/combat-light-attack


---

### TODO List

#### 1. 공격 액션

- [x] LightAttack 입력 액션 추가 및 바인딩

- [x] CPlayerCharacter에 LightAttack 트리거 함수 구현

- [x] 무기 장착 여부에 따른 공격 가능 조건 처리 (미장착 시 공격 불가)


#### 2. 애니메이션 로직

- [x] 라이트 공격 1타용 애니메이션 또는 몽타주 구성

- [x] LightAttack 입력 → 몽타주 재생 플로우 구현 (C++ 또는 BP)

- [x] AnimNotify를 이용해 공격 판정(히트 체크) 타이밍 설정


#### 3. 전투 상태 / 입력 처리

- [x] 공격 시작 시 ECharacterState를 Attack으로 전환

- [x] Attack 상태에서 이동 / 회전 / 장착 입력 제한 정책 1차 적용

- [x] 몽타주 종료 또는 조건 충족 시 Idle/Move 상태로 복귀 플로우 정리


---

### Notes

- 


---