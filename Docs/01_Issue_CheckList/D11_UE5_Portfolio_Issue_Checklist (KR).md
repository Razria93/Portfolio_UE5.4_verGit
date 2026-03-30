# UE5 Portfolio – Issue Checklist

## 제목

**M03-01: AIController + Blackboard + BehaviorTree 기본 파이프라인 구성**

### 날짜

- **Day 11**

- **Date : 2026.01.26**


---

### 목표

- `AIController` / `Blackboard` / `BehaviorTree`의 기본 구성을 확정하고, Enemy AI의 최소 동작 파이프라인을 구축함.

- 전투/추적/대기 상태 전환을 **Blackboard 키와 BehaviorTree 흐름**으로 정리하여 이후 확장(전술/패턴/스킬) 기반을 마련함.

- 테스트 레벨에서 **단일 Enemy 기준으로 동작 검증**을 완료하고, 로그 기반 추적성을 확보함.


---

### 브랜치

- feature/ai-behaviortree-core


---

### TODO List

#### 1. AIController 기본 클래스 및 소유 관계 정리

- [ ] `CAIController`(가칭) 클래스 생성

- [ ] `OnPossess`에서 Blackboard/BehaviorTree 초기화 루틴 작성

- [ ] 소유 Pawn 캐싱(Enemy 캐스팅 및 유효성 검증)

- [ ] 디버그 출력(Controller, Pawn, BT Asset, BB Asset) 최소 로그 규칙 정의


---

#### 2. Blackboard 구성(키 규격화)

- [ ] Blackboard Asset 생성 및 기본 키 정의

  - [ ] `TargetActor` (Object)

  - [ ] `HomeLocation` (Vector)

  - [ ] `PatrolLocation` (Vector, 옵션)

  - [ ] `IsInCombat` (Bool)

  - [ ] `IsDead` (Bool)

  - [ ] `LastKnownTargetLocation` (Vector)

- [ ] 키 네이밍 규칙 확정(접두사/타입 포함 여부)

- [ ] 키 초기값 설정 규칙 정의(Spawn/OnPossess 시점)


---

#### 3. BehaviorTree 골격 구성(최소 동작)

- [ ] BT Root → Selector 구조 구성

  - [ ] Dead 상태 Branch (우선순위 최상)

  - [ ] Combat 상태 Branch (추적/공격)

  - [ ] Idle/Patrol Branch (대기/순찰)

- [ ] 각 Branch에 필요한 Decorator 조건 정의

- [ ] 최소 실행 가능한 노드 구성(Wait, MoveTo, Simple Sequence)


---

#### 4. BT Task 노드(최소 2종) 구현

- [ ] Task: `SetTargetFromSense` 또는 `UpdateTarget` (감지 결과 반영)

- [ ] Task: `MoveToTarget` 또는 `MoveToLocation`

- [ ] Task: `ClearTarget` (타겟 상실 시 초기화)

- [ ] Task 수행 결과 로그 출력 규칙 추가(성공/실패 사유)


---

#### 5. Service/Decorator 최소 설계(상태 갱신)

- [ ] Service: `UpdateCombatState` (거리/라인오브사이트 기준)

- [ ] Decorator: `IsValidTarget` (TargetActor 유효성)

- [ ] Decorator: `IsInCombat` / `IsDead` (Blackboard Bool 기반)

- [ ] 조건 실패 시 흐름 전환 정책 정의(Idle 복귀, Search 등)


---

#### 6. AI Perception 연동(옵션/후속 확장)

- [ ] `AIPerceptionComponent` 추가 여부 결정

- [ ] 시야/청각 기본 설정값 확정

- [ ] 감지 이벤트 → Blackboard 갱신 규칙 확정


---

#### 7. 통합 검증 시나리오

- [ ] 시나리오 1: Enemy Spawn → Idle 유지 (Target 없음)

- [ ] 시나리오 2: Target 감지 → Combat 전환 → MoveTo 실행

- [ ] 시나리오 3: Target 상실 → Combat 해제 → Idle 복귀

- [ ] 시나리오 4: Dead 상태 전환 시 모든 행동 중단 확인


---

### Notes

- 초기 BT 구성은 **최소 동작(Idle/Combat/Dead)** 흐름만 확보하고, 공격/패턴/스킬은 후속 이슈에서 확장함.

- Blackboard 키는 **전투 시스템과 공유 가능한 규격**으로 설계하여 다른 컴포넌트와의 연계를 고려함.


---