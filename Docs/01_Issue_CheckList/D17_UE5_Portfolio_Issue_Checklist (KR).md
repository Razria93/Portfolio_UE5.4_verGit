# UE5 Portfolio Issue Checklist

## 제목

**M05-02: Reaction 흐름을 ReactionOrchestrator / ReactionComponent / CReaction 구조로 재정리함**

### 날짜

- **Day 17**
  
- **Date : 2026.04.30**


---

### 목표

- reaction 흐름을 **ReactionOrchestrator -> ReactionComponent -> CReaction** 구조로 명확히 분리함.

- reaction 요청 진입점을 `UCReactionOrchestratorComponent`로 통일함.

- `UCReactionComponent`는 reaction 실행 상태 소유와 실행 상태 적용 책임으로 축소함.

- Player Tick과 Enemy BT에 나뉘어 있는 reaction 실행 트리거를 제거함.

- `CReaction`은 montage lifecycle, notify 기반 window, reaction별 interrupt / cancel 정책을 담당하도록 유지함.


---

### 브랜치
- `feature/reaction-orchestration`


---

### TODO List

#### 1. 현재 Reaction 흐름 정리

- [ ] `TakeDamage -> Reaction request -> execute` 현재 경로 정리

- [ ] Player Tick / Enemy BT 의존 지점 정리

- [ ] `ReactionComponent`의 현재 책임과 문제 지점 정리


#### 2. ReactionOrchestrator 구조 확정

- [ ] reaction request의 상위 진입점을 `ReactionOrchestrator`로 정리

- [ ] damage result -> reaction intent 변환 책임 정리

- [ ] reaction type / priority / decision 생성 책임을 orchestrator로 정리


#### 3. ReactionComponent 책임 재정의

- [ ] `ReactionComponent`를 reaction 실행 상태 관리자 역할로 축소

- [ ] active / pending reaction context 소유 구조 정리

- [ ] reaction 시작 / 종료 시 action abort, movement lock, execution state 전환 책임 정리


#### 4. CReaction 실행 단위 책임 유지

- [ ] `CReaction`이 montage lifecycle / notify window / interruption policy를 계속 소유하도록 정리

- [ ] executor runtime flag와 orchestration decision 연결 방식 정리

- [ ] `Hit / Dead` 등 executor별 정책 차이가 공통 흐름과 충돌하지 않는지 확인


#### 5. Reaction Decision 정책 정리

- [ ] `Reject / Ignore / Start / Replace / Pending` 1차 구현 범위 확정

- [ ] active reaction 중 재피격 priority / interruption 판단 기준 정리

- [ ] pending replacement와 queue 범위 정리


#### 6. TakeDamage / Feedback 연동 정리

- [ ] `TakeDamageComponent`와 reaction request 연결 범위 정리

- [ ] zero damage / rejected damage / dead-state 조건 정리

- [ ] reaction-feedback과 reaction 실행 여부의 관계 정리


#### 7. Player / Enemy 공통 실행 경로 정리

- [ ] Player / Enemy가 동일한 reaction request 경로를 사용하도록 정리

- [ ] Player Tick 기반 pending consume 제거 방향 정리

- [ ] Enemy BT의 reaction 실행 책임 축소 방향 정리


#### 8. Dead Reaction 및 상태 전이 정리

- [ ] dead reaction과 dead-state 전이 관계 정리

- [ ] dead-state에서 reaction cleanup / 추가 request 처리 기준 정리

- [ ] `EReactionType::Dead` 포함 여부와 관련 구조 정리


#### 9. 1차 구현 범위 정리

- [ ] 1차 범위를 hit / dead reaction orchestration과 공통 실행 경로 통일에 한정

- [ ] Guard / Parry / Counter / Launch / KnockDown / queue 확장 범위 분리

- [ ] 이번 브랜치에서 유지해야 하는 기존 combat / action / feedback 안정성 기준 정리


#### 10. 최소 검증 기준 정리

- [ ] Scenario 1: Player 피격 -> `ReactionOrchestrator`를 통해 reaction 시작

- [ ] Scenario 2: Enemy 피격 -> BT start task 의존 없이 reaction 시작

- [ ] Scenario 3: active action 도중 reaction takeover 시 action abort 확인

- [ ] Scenario 4: active reaction 중 재피격 시 priority / interruption 정책 확인

- [ ] Scenario 5: dead damage result가 dead reaction / dead-state 흐름으로 정상 연결되는지 확인

- [ ] Scenario 6: reaction 종료 후 유효한 경우에만 movement / execution state가 복구되는지 확인


---

### Notes

- 이 이슈는 신규 reaction 기능 추가가 아니라 구조 리팩터링에 집중함.

- `ReactionOrchestratorComponent`는 요청 라우팅과 decision 생성을 담당하고, `ReactionComponent`는 runtime state 소유와 decision 적용을 담당함.

- `CReaction`은 montage lifecycle과 timing event를 처리하는 실행 단위로 유지함.

- queue 동작이 1차 범위에 필요하지 않다면 decision 타입만 남기고 queue 저장 / 처리 구현은 후속으로 미룸.


---
