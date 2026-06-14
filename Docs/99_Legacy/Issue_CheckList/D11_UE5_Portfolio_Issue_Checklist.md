# UE5 Portfolio – Issue Checklist

## 제목

**M03-01: AI BehaviorTree Core 구축 및 AIState 분기별 실행 흐름 구현**

### 날짜

- **Day 11**

- **Date : 2026.01.26**

---
### 브랜치

- feature/ai-behaviortree-core

---
### 목표

- `AIController` / `Blackboard` / `BehaviorTree`의 기본 코어를 구성하고, Enemy AI의 상태 전이 파이프라인을 확정함.

- `Perception -> AIContext -> AIState -> BT Branch` 흐름을 기준으로 `Idle / Patrol / Investigate / Chase / Alert / Engage / HitReact / Dead` 상태 전환 구조를 정리함.

- 테스트 레벨에서 **단일 Enemy 기준**으로 상태 전이와 전투 흐름을 검증하고, 로그 기반 추적성을 확보함.

---
### TODO 리스트

#### 1. AIController 코어 및 초기화 루틴 구성

- [x] `CAIController` 클래스 구성

- [x] `OnPossess`에서 Blackboard 초기화

- [x] `OnPossess`에서 BehaviorTree 실행

- [x] 소유 Pawn 캐싱 및 Enemy 유효성 검증

- [x] Blackboard Key 유효성 검증 루틴 추가

- [x] 초기 Blackboard 값 세팅 규칙 정리

- [x] 최소 디버그 로그 규칙 정의

#### 2. Blackboard 키 체계 정리

- [x] `CAIKey` namespace 기반 키 체계 정리

- [x] Targeting / State / Perception / Metric / Navigation 키 정의

- [x] Patrol / Investigate / Chase / Alert / Engage 키 정의

- [x] Reaction / Dead 관련 키 정의

- [x] Spawn / Possess 시점 초기값 세팅 규칙 확정

#### 3. AI Perception 및 Target Context 구성

- [x] `AIPerceptionComponent` 연동

- [x] Sight 설정값 구성

- [x] Perception 이벤트 수신 처리

- [x] `TargetDataMap` 기반 타겟 캐시 구성

- [x] Target priority / LOS / memory timeout 정책 반영

- [x] `BuildPerceptionContext()` 흐름 구성

#### 4. AIContext 갱신 서비스 구성

- [x] `UpdateAIContext` 서비스 구성

- [x] Perception context 갱신

- [x] Home metric 갱신

- [x] Alert range 계산

- [x] Engage assignment 요청/반영

- [x] Reaction context 갱신

- [x] Dead context 갱신

- [x] 상황별 Blackboard clear 정책 정리

#### 5. AIState 전이 서비스 구성

- [x] `UpdateAIState` 서비스 구성

- [x] `Dead > HitReact > Engage > Investigate/Chase/Alert/Idle` 우선순위 정리

- [x] Target / LOS / AlertRange / Engage 조건 기반 상태 결정

- [x] 상태 전이 시 Blackboard clean-up 규칙 정리

- [x] Engage 이탈 시 공격 관련 키 초기화 규칙 정리

#### 6. BehaviorTree 상태 브랜치 구성

- [x] Root 기준 상태 브랜치 구조 구성

	- [x] `Idle`

	- [x] `Patrol`

	- [x] `Investigate`

	- [x] `Chase`

	- [x] `Alert`

	- [x] `Engage`

	- [x] `HitReact`

	- [x] `Dead`

	- [x] 각 Branch 진입 조건용 Decorator 연결

#### 7. Patrol / Investigate / Chase / Alert 흐름 구성

- [x] Patrol path / point 기반 순찰 흐름 구성

- [x] Investigate location / index 기반 탐색 흐름 구성

- [x] Chase 거리 조건 및 이동 흐름 구성

- [x] Alert 위치 선정 및 step 이동 흐름 구성

- [x] 상태별 이동 속도 / focus 제어 노드 구성

#### 8. Engage Assignment 및 전투 흐름 구성

- [x] `UCWorldSubsystem_CombatEngage` 추가

- [x] Engage request / assignment 구조 구성

- [x] 다수 AI 대상 Engage / Alert 역할 분배 규칙 반영

- [x] `UpdateEngageContext`로 `bInEngageRange`, `bCanAttack` 계산

- [x] Engage positioning subtree 구성

- [x] Attack subtree 진입 조건 정리

#### 9. Attack Subtree 및 공격 루프 구성

- [x] `SelectAttackIndex`

- [x] `StartAttack`

- [x] `WaitAttackEnd`

- [x] `CommitAttackCooldown`

- [x] `AnimNotify_EndEnemyAttack` 연동

- [x] 공격 중 `bIsAttacking` 유지 규칙 정리

- [x] 공격 종료 후 재진입 / 쿨다운 검증

#### 10. Reaction / Dead 상태 흐름 구성

- [x] pending / active reaction 구조 반영

- [x] `TryStartReaction`, `WaitEndReaction` 구성

- [x] `HitReact` 상태 진입 및 종료 검증

- [x] `DeadState` Blackboard 동기화

- [x] `StayDead`, `WaitDeadState`, `StartRevive` 구성

- [x] `AnimNotify_EnterDeadState`, `AnimNotify_EnterAliveState` 연동

#### 11. 통합 검증 시나리오

- [x] 시나리오 1: Enemy Spawn -> Idle 유지

- [x] 시나리오 2: Patrol 설정 시 Patrol 순환

- [x] 시나리오 3: Target 감지 -> Chase 전환

- [x] 시나리오 4: Alert 거리 진입 -> Alert 전환

- [x] 시나리오 5: Engage assignment 반영 -> Engage 진입

- [x] 시나리오 6: 공격 시작 -> 종료 -> 쿨다운 -> 재공격

- [x] 시나리오 7: 피격 시 HitReact 진입 및 복귀

- [x] 시나리오 8: 사망 시 모든 행동 중단 및 Dead 상태 유지

- [x] 시나리오 9: revive 시 Alive 상태 복귀 확인

---
### 비고

- 본 이슈는 **Enemy AI의 BT 코어와 상태 전이 파이프라인**을 닫는 것이 목적이며, 플레이어 전투 루프 자체는 후속 이슈에서 확장함.

- 기존 `Combat` 중심 표현은 현재 구조상 `Engage` 기반으로 재정리하는 것이 맞으며, Blackboard 키와 서비스/태스크도 동일한 기준으로 맞춤.

- 이 이슈는 `Perception -> Context -> State -> Branch -> Action` 흐름을 코드와 Blackboard 기준으로 일관되게 정리하는 작업임.

---
