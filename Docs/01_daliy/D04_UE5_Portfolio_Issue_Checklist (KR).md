# UE5 Portfolio – Issue Checklist

### Date

- **Day 5**

- **Date : 2025.12.10**


---

### Objective

- 캐릭터 무기 장착 / 해제 시스템 구현

- 소켓 기반 무기 Attach/Detach 처리

- 장착 상태 플래그 및 캐릭터 상태와의 연동 기초


### Branch

- feature/character-weapon-equip


---

### TODO List

#### 1. 시스템 구성

- [ ] 무기 장착 상태 정의 (Equipped / Unequipped)

- [ ] 캐릭터 스켈레톤에 무기용 소켓 설정 (손 / 보관 위치)

- [ ] CPlayerCharacter에 무기 참조 변수 및 Equip / Unequip / Toggle 함수 추가


#### 2. 애니메이션 연동

- [ ] Equip / Unequip용 몽타주 또는 애니메이션 구간 선택

- [ ] 입력 또는 상태 변화에 따라 장착/해제 애니 재생

- [ ] AnimNotify 시점에 소켓 전환(손 ↔ 보관 위치) 동기화


#### 3. 캐릭터 연동

- [ ] Equip 입력 액션 추가 및 바인딩 (토글 방식 등)

- [ ] 장착/해제 시 캐릭터 내부 상태(플래그/Enum) 업데이트

- [ ] 향후 공격 시스템에서 “무기 미장착 시 공격 불가” 조건으로 활용 가능하도록 설계


---

### Notes
- 


---
