# UE5 Portfolio Issue Checklist

## 제목

**M04-02: Action Orchestration 구조 정리 및 AI Combo / Reaction 연동 보강**

### 날짜

- **Day 16**
  
- **Date : 2026.04.19**


---

### 목표

- Player 입력에서 액션 실행까지의 흐름을 **Orchestrator 중심 구조**로 정리함.

- 상태 변경과 액션 실행의 순서를 분리하여, `상태 전이 확정 -> 액션 실행` 흐름을 명확하게 구성함.

- Player와 AI가 같은 combat request 경로와 combo chain 실행 경로를 재사용하도록 정리함.

- combo action 도중 피격 시 Reaction takeover 이후에도 전투 흐름이 유지되도록 최소 안전 구조를 추가함.


---

### 브랜치
- `feature/action-orchestration`


---

### TODO List

#### 1. Player 입력 흐름 정리

- [x] Player 입력이 직접 `ActionComponent`를 호출하는 구조 점검

- [x] 입력 요청을 Orchestrator로 전달하는 흐름 검토

- [x] 공통 입력 차단 조건과 액션별 실행 조건 분리 방향 정리


#### 2. Orchestrator 1차 구조 구성

- [x] Player action request 처리 흐름 설계

- [x] 글로벌 규칙 체크 위치 정리

- [x] 상태 전이 판단과 상태 전이 확정 단계 분리

- [x] 일반적인 실패 시 rollback 정책 확정


#### 3. ActionComponent 책임 정리

- [x] 액션 보관 / 조회 / 현재 액션 관리 책임 정리

- [x] 액션 실행 요청 API 재검토

- [x] `ChangeActionMode` 계열 API의 역할 재정의


#### 4. CAction 책임 정리

- [x] `CAction`에서 직접 상태를 변경하는 구조 제거 방향 검토

- [x] 액션 고유 실행 조건과 실행 로직 분리

- [x] ComboAttack 기존 동작 유지 여부 확인


#### 5. AI Combo 연동 정리

- [x] AI combat blackboard key 정리 (`bCanCombatAction`, `bIsCombatAction`, `NextCombatActionTime`)

- [x] `StartCombatAction` / `WaitEndCombatAction` 구조 반영

- [x] action event callback 기반 AI combo chain follow-up 연결

- [x] Player / AI combo chain 실행 경로 통일 확인


#### 6. Reaction Takeover 안전성 보강

- [x] reaction 진입 시 active action abort 구조 추가

- [x] reaction 상태를 combat availability 계산에 반영

- [x] combo action 도중 피격 후 reaction 이후 combat flow 복구 확인


#### 7. 최소 검증 기준 정리

- [x] Scenario 1: Player 입력 -> Orchestrator -> Action 실행

- [x] Scenario 2: 상태 전이 이후 액션 실행 순서 확인

- [x] Scenario 3: 일반적인 액션 실행 실패 시 rollback 정책 확정

- [x] Scenario 4: Player ComboAttack 동작 유지 확인

- [x] Scenario 5: AI ComboAttack이 다음 단계로 정상 chain되는지 확인

- [x] Scenario 6: combo action 도중 피격 후 reaction 이후 combat flow 복구 확인


---

### Notes

- 본 이슈는 새 액션 추가보다 **공유 action 실행 흐름의 책임 분리**에 집중함.

- AI combo chain 연동과 reaction takeover 최소 안전 구조까지 이번 브랜치 범위에 포함함.

- Guard / Parry / Counter, 고도화된 Reaction orchestration, 상위 coordination 계층은 이후 브랜치 확장 대상으로 남겨둠.


---
