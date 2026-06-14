# UE5 Portfolio – Issue Checklist

## 제목

**M05-02: CReactionOrchestratorComponent 추가 및 Reaction 공용 실행 구조 구축**

### 날짜

- **Day 17**

- **Date : 2026.04.30**

---
### 브랜치

- feature/reaction-orchestration

---
### 목표

- reaction 흐름을 **ReactionOrchestrator -> ReactionComponent -> CReaction** 구조로 명확히 분리함.

- reaction 요청 진입점을 `UCReactionOrchestratorComponent`로 통일함.

- `UCReactionComponent`는 reaction 실행 상태 소유와 실행 상태 적용 책임으로 축소함.

- Player Tick과 Enemy BT에 나뉘어 있는 reaction 실행 트리거를 제거함.

- `CReaction`은 montage lifecycle, notify 기반 window, reaction별 interrupt / cancel 정책을 담당하도록 유지함.

---
### TODO 리스트

#### 1. 현재 Reaction 흐름 정리

- [x] `TakeDamage -> ReactionOrchestrator -> ReactionComponent -> CReaction` 실행 경로를 정리함

- [x] Player Tick / Enemy BT 기반 pending consume 의존 지점을 정리함

- [x] `ReactionComponent`의 pending 저장 / 실행 판단 / 실행 적용 책임이 섞여 있던 문제를 정리함

#### 2. ReactionOrchestrator 구조 확정

- [x] reaction request의 상위 진입점을 `UCReactionOrchestratorComponent::RequestReaction()`으로 정리함

- [x] damage result를 기반으로 `Hit / Dead` reaction type을 resolve하는 책임을 orchestrator로 정리함

- [x] reaction type / data / executor / policy / decision 생성 책임을 orchestrator로 정리함

#### 3. ReactionComponent 책임 재정의

- [x] `ReactionComponent`를 active reaction state 관리자와 decision 적용 객체로 축소함

- [x] pending reaction context를 제거하고 active reaction context 중심으로 정리함

- [x] reaction 시작 / 종료 시 action abort, movement lock, execution state 전환 책임을 component lifecycle에 맞춰 정리함

#### 4. CReaction 실행 단위 책임 유지

- [x] `CReaction`이 montage lifecycle / control window / feedback notify / local policy hook을 소유하도록 정리함

- [x] executor runtime flag와 `FReactionExecutionPolicy` / executor hook의 역할 차이를 정리함

- [x] `Hit / Dead` reaction type이 공통 orchestration 흐름에서 처리되도록 정리함

#### 5. Reaction Decision 정책 정리

- [x] `Reject / Ignore / Start / Interrupt / Cancel` 1차 decision 범위를 확정함

- [x] active reaction 중 재피격 priority / interruption 판단 기준을 `CanInterruptActiveReaction()` 중심으로 정리함

- [x] pending replacement와 queue는 1차 범위에서 제거하고 후속 확장 대상으로 분리함

#### 6. TakeDamage / Feedback 연동 정리

- [x] `TakeDamageComponent`가 accepted damage 이후 `ReactionOrchestrator`로 reaction request를 전달하도록 정리함

- [x] zero damage / rejected damage / dead-state 조건을 take damage result와 reaction type resolve 기준으로 정리함

- [x] `ReactionFeedback`은 reaction execution timing 기준, `DamageFeedback`은 damage event / impact metadata 기준으로 분리함

#### 7. Player / Enemy 공통 실행 경로 정리

- [x] Player / Enemy가 동일한 reaction request 경로를 사용하도록 컴포넌트 구성을 정리함

- [x] Player Tick 기반 pending consume 흐름을 제거함

- [x] Enemy BT를 active reaction state observer 역할로 정리함

#### 8. Dead Reaction 및 상태 전이 정리

- [x] dead-state 전이 결과를 `EReactionType::Dead`로 resolve하도록 정리함

- [x] dead-state에서 추가 action / reaction request가 gate 또는 priority 정책에 따라 정리되도록 기준을 세움

- [x] `EReactionType::Dead`를 reaction type으로 연결하고 최상위 priority / force interrupt 정책을 부여함

#### 9. 1차 구현 범위 정리

- [x] 1차 범위를 hit / dead reaction orchestration과 공통 실행 경로 통일에 한정함

- [x] Guard / Parry / Counter / Launch / KnockDown / queue 확장 범위를 후속 작업으로 분리함

- [x] 기존 combat / action / feedback 안정성 기준을 유지하면서 reaction feedback과 damage feedback을 분리함

#### 10. 최소 검증 기준 정리

- [x] Scenario 1: Player 피격 -> `ReactionOrchestrator`를 통해 reaction 시작 확인함

- [x] Scenario 2: Enemy 피격 -> BT start task 의존 없이 reaction 시작 확인함

- [x] Scenario 3: active action 도중 reaction takeover 시 action abort 확인함

- [x] Scenario 4: active reaction 중 재피격 시 priority / interruption 정책 확인함

- [x] Scenario 5: dead damage result가 dead reaction / dead-state 흐름으로 정상 연결되는지 확인함

- [x] Scenario 6: reaction 종료 후 유효한 경우에만 movement / execution state가 복구되는지 확인함

---
### 비고

- 이 이슈는 reaction 실행 구조 리팩터링에 집중하는 작업임.

- `ReactionOrchestratorComponent`는 요청 라우팅과 decision 생성을 담당하고, `ReactionComponent`는 runtime state 소유와 decision 적용을 담당함.

- `CReaction`은 montage lifecycle과 timing event를 처리하는 실행 단위로 유지함.

- pending / queue 동작은 1차 범위에서 제거하고, 필요 시 후속 action / reaction 확장 작업에서 별도 모델로 재검토함.

- hit feedback은 `DamageFeedback`, reaction execution feedback은 `ReactionFeedback`으로 분리함.

- AI BT는 active reaction state observer 역할로 정리함.

---
