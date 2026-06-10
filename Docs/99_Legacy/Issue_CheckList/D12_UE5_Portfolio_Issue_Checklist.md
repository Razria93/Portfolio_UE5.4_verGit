# UE5 Portfolio – Issue Checklist

## 제목

**M03-02: Player Combat Receiver 구축**

### 날짜

- **Day 12**

- **Date : 2026.03.31**

---
### 브랜치

- feature/player-combat-receiver

---
### 목표

- 플레이어를 피격, 리액션, 사망 상태까지 처리 가능한 전투 엔티티로 확장함.

- `TakeDamage / Health / Reaction` 축을 플레이어에 연결하여 Enemy와 동일한 전투 수신 파이프라인 기반을 마련함.

- 플레이어가 AI 공격을 정상적으로 수신하고, 상태 변화가 Anim/State 기준으로 일관되게 반영되도록 정리함.

---
### TODO 리스트

#### 1. Player 수신 컴포넌트 구성

- [x] `ACPlayer`에 `TakeDamageComponent` 추가

- [x] `ACPlayer`에 `HealthComponent` 추가

- [x] `ACPlayer`에 `ReactionComponent` 추가

- [x] Player 생성자 기준 컴포넌트 초기화 순서 정리

#### 2. Player Damage 진입점 연결

- [x] `ACPlayer::TakeDamage()` 오버라이드 추가

- [x] `TakeDamageComponent` 경유 처리 흐름 연결

- [x] fallback 처리 정책 정리

- [x] Player 기준 최소 로그 출력 확인

#### 3. Health / Dead 상태 연결

- [x] 피격 시 HP 감소 반영 확인

- [x] `DeadState` 진입 규칙 정리

- [x] Dead 상태 재피격 정책 정리

- [x] Anim/State와 DeadState 동기화 방향 정리

#### 4. Reaction 상태 연결

- [x] 플레이어 피격 시 Reaction 요청 확인

- [x] HitReact 진입 조건 검증

- [x] Reaction 종료 후 Idle/기본 상태 복귀 규칙 정리

- [x] Player 기준 movement/state 제어 영향 확인

#### 5. 통합 검증

- [x] 시나리오 1: AI 공격 -> 플레이어 HP 감소

- [x] 시나리오 2: AI 공격 -> 플레이어 HitReact 진입

- [x] 시나리오 3: 누적 피격 -> DeadState 진입

- [x] 시나리오 4: Dead 상태에서 추가 피격 처리 확인

---
### 비고

- 본 이슈는 플레이어를 **전투 수신 가능한 엔티티**로 편입하는 것이 목적이며, 플레이어 공격 루프 자체는 후속 이슈에서 정리함.

- 우선순위는 고급 전투 시스템보다 `TakeDamage -> Health -> Reaction -> Dead` 기본 루프를 닫는 데 둠.

---
