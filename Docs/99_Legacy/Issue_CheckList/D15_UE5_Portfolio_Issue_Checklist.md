# UE5 Portfolio – Issue Checklist

## 제목

**M04-01: Action / Reaction / Player Feedback 역할 분리 및 Pipeline 정리**

### 날짜

- **Day 15**

- **Date : 2026.04.11**

---
### 브랜치

- feature/combat-feedback

---
### 목표

- 전투 결과와 액션 타이밍을 기반으로 하는 **Combat Feedback** 구조를 1차적으로 정리함.

- **reaction-feedback / action-feedback / player-feedback**의 역할을 구분하고, Player-side와 Enemy-side에서 공통으로 사용할 수 있는 흐름을 검토함.

- 이후 전투 구조 확장에 사용할 수 있도록 feedback 실행 경로와 최소 검증 기준을 정리함.

---
### TODO 리스트

#### 1. Feedback 구조 및 책임 정리

- [x] `reaction-feedback / action-feedback / player-feedback` 구분 정리

- [x] shared feedback과 player local feedback 범위 정리

- [x] 전투 결과 기반 feedback과 notify 기반 feedback의 연결 지점 정리

#### 2. Reaction / Player Feedback 1차 연결

- [x] `TakeDamage` 이후 reaction-feedback 연결 경로 정리

- [x] hit VFX / hit SFX / hit stop 1차 적용 검토

- [x] player local feedback 적용 지점 검토

#### 3. Action Feedback 1차 연결

- [x] Trail 및 action SFX / VFX 적용 방식 검토

- [x] notify 기반 action-feedback 실행 타이밍 정리

- [x] 액션 시작 / 종료 시점 feedback 적용 가능 여부 검토

#### 4. Player / Enemy 공통 흐름 검토

- [x] Player-side action-feedback 실행 경로 확인

- [x] Enemy-side action-feedback 실행 경로 확인

- [x] Enemy 공격 종료 이후 cleanup 흐름 검토

#### 5. 최소 검증 기준 정리

- [x] Scenario 1: Player 공격 -> Enemy 피격 feedback

- [x] Scenario 2: Player action 실행 -> action-feedback 출력

- [x] Scenario 3: Enemy action 실행 -> action-feedback 출력

- [x] Scenario 4: Player local feedback 출력 확인

---
### 비고

- 본 이슈는 새로운 전투 규칙 추가보다, **전투 결과와 액션 타이밍이 실제 체감으로 이어지도록 feedback 구조를 연결하는 것**에 초점을 둠.

- 이후 validation 및 action 구조 확장 작업에서 공통으로 사용할 feedback 기준 브랜치로 활용함.

---
