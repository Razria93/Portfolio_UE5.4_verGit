# UE5 Portfolio Issue Checklist

## 제목

**M04-01: Combat Feedback 구현 및 정리**

### 날짜

- **Day 15**
  
- **Date : 2026.04.11**


---

### 목표

- 전투가 구조만 보이는 상태에서 벗어나, 피격 결과가 실제 체감으로 보이도록 **Combat Feedback**을 구현하고 정리함.
  
- `TakeDamage -> Reaction -> ReactionFX` 흐름을 기준으로 피격 이후 후속 연출을 일관되게 연결함.


---

### 브랜치
- `feature/combat-feedback`


---

### TODO List

#### 1. Feedback 구조 정리

- [ ] 현재 `ReactionComponent`와 `ReactionFXComponent` 책임 재점검
      
- [ ] `TakeDamageCommitted` 이후 Feedback 트리거 지점 정리
      
- [ ] Reaction 실행과 Feedback 실행 순서 정리
      
- [ ] Dead / Revive 시 Feedback 차단 또는 종료 규칙 정리


#### 2. Hit Feedback 1차 구현

- [ ] Hit Stop 적용
      
- [ ] Hit VFX 적용
      
- [ ] Hit Sound 적용
      
- [ ] Camera Shake 적용
      
- [ ] 강한 피격 / 약한 피격에 따른 Feedback 차이 여부 점검


#### 3. 체감 품질 점검

- [ ] 기본 공격 1타 피격 체감 확인
      
- [ ] 콤보 중 연속 피격 체감 확인
      
- [ ] Dead 직전 / Dead 이후 Feedback 동작 확인
      
- [ ] Reaction 종료 후 이질감 없는지 점검


#### 4. 최소 검증
- [ ] Scenario 1: Player 공격 -> Enemy HitReact + Feedback
      
- [ ] Scenario 2: 콤보 공격 -> 연속 Feedback 동작 확인
      
- [ ] Scenario 3: Dead 전이 시 Feedback 종료 규칙 확인
      
- [ ] Scenario 4: Revive 이후 Feedback 초기화 확인


---

### Notes

- 본 이슈는 새 전투 규칙 추가보다 **전투가 실제로 보이고 느껴지게 만드는 것**에 집중한다.
- 이후 `Combat Validation`, `AI Combat Loop`, `CounterAction` 작업의 체감 기준이 되는 브랜치로 본다.


---
