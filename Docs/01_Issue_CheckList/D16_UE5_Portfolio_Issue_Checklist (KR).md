# UE5 Portfolio Issue Checklist

## 제목

**M04-02: Player Action Orchestration 구조 정리**

### 날짜

- **Day 16**
  
- **Date : 2026.04.19**


---

### 목표

- Player 입력에서 액션 실행까지의 흐름을 **Orchestrator 중심 구조**로 정리함.

- 상태 변경과 액션 실행의 순서를 분리하여, `상태 전이 확정 -> 액션 실행` 흐름을 명확하게 구성함.

- 이후 AI 동기화, Reaction orchestration, Guard / Parry / Counter 같은 특수 액션 확장을 고려할 수 있는 기반을 마련함.


---

### 브랜치
- `feature/action-orchestration`


---

### TODO List

#### 1. Player 입력 흐름 정리

- [ ] Player 입력이 직접 `ActionComponent`를 호출하는 구조 점검

- [ ] 입력 요청을 Orchestrator로 전달하는 흐름 검토

- [ ] 공통 입력 차단 조건과 액션별 실행 조건 분리 방향 정리


#### 2. Orchestrator 1차 구조 구성

- [ ] Player action request 처리 흐름 설계

- [ ] 글로벌 규칙 체크 위치 정리

- [ ] 상태 전이 판단과 상태 전이 확정 단계 분리

- [ ] 실패 시 rollback 흐름 검토


#### 3. ActionComponent 책임 정리

- [ ] 액션 보관 / 조회 / 현재 액션 관리 책임 정리

- [ ] 액션 실행 요청 API 재검토

- [ ] `ChangeActionMode` 계열 API의 역할 재정의


#### 4. CAction 책임 정리

- [ ] `CAction`에서 직접 상태를 변경하는 구조 제거 방향 검토

- [ ] 액션 고유 실행 조건과 실행 로직 분리

- [ ] ComboAttack / LightAttack 기존 동작 유지 여부 확인


#### 5. 최소 검증 기준 정리

- [ ] Scenario 1: Player 입력 -> Orchestrator -> Action 실행

- [ ] Scenario 2: 상태 전이 이후 액션 실행 순서 확인

- [ ] Scenario 3: 액션 실행 실패 시 상태 rollback 확인

- [ ] Scenario 4: 기존 ComboAttack / LightAttack 동작 유지 확인


---

### Notes

- 본 이슈는 새 액션 추가보다 **Player action 실행 흐름의 책임 분리**에 집중함.

- AI, Reaction, Guard / Parry / Counter는 본 구조를 기반으로 이후 브랜치에서 확장함.

- Arbiter는 이번 브랜치에서 완성형으로 구현하기보다, Orchestrator 내부 판단 흐름을 정리하면서 필요 범위를 검토함.


---
