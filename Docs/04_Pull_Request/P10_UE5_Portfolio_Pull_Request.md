# UE5 Portfolio Pull Request

## 제목

**P10: AI BehaviorTree Core 구축 및 AIState 분기별 실행 흐름 구현**

## 날짜

**2026.03.31**

## 상태

- [x] **완료**

---

## 브랜치

- `feature/ai-behaviortree-core`

---

## 요약

### 작업 요약

본 PR은 Enemy AI의 `AIController / Blackboard / BehaviorTree` 기반 core를 구성하고,
AI 판단 흐름을 아래와 같은 구조로 정리한 작업이다.

```yaml
Perception
-> Update AIContext
-> Resolve AIState
-> Select BT Branch
-> Execute Task / Subtree
```

### 작업 배경

#### AI 행동 state 확장의 필요성

Enemy AI가 단순 추적 / 단일 attack 수준을 넘어, `Patrol / Investigate / Chase / Alert / Engage / HitReact / Dead` 같은 기본 행동 state를 표현할 수 있어야 했다.

이를 BehaviorTree branch 단위로 정리하여, state별 진입 조건과 실행 책임을 추적 가능한 구조로 만들고자 했다.

#### Combat / Engage / Attack 개념 분리

기존에는 전투 관련 상태를 `Combat / Alert` 정도로 나누었지만, `Combat` 안에 실제 attack 실행, attack 대기, range 조정, 전투 참여 여부 판단이 함께 섞일 수 있었다.

이를 분리하기 위해 `Combat`은 전투 관련 상위 도메인으로 두고, `Engage`는 combat domain 안에서 실제 교전에 참여하는 state로 정의했다.

실제 attack 실행은 `Engage` 내부의 `SBT_Attack` subtree로 분리했다.

### 구현 방향

이를 다음 네 가지 축으로 정리했다.

```yaml
1. AI Core 구성
- ACAIController / Blackboard / BehaviorTree 초기화
- CAIKey namespace 기반 Blackboard key 체계 구성

2. AIContext / AIState 갱신
- perception / target memory / range / engage / reaction / dead context 갱신
- AIState priority 기반 state transition

3. BT State Branch 구성
- Idle / Patrol / Investigate / Chase / Alert / Engage / HitReact / Dead branch 구성

4. Engage / Attack / Reaction / Dead 흐름 연결
- Combat Role Assignment 기반 Engage 여부 판단
- Attack subtree, Reaction branch, Dead branch 연결
```

---
## 변경 범위

### AI BehaviorTree Core

#### A. AIController / Blackboard / BehaviorTree 초기화

- Enemy possess 이후 Blackboard를 초기화하고 BehaviorTree를 실행하는 AI core 진입 구조를 구성했다.

**Flow**
```yaml
Enemy Possess
-> ACAIController::OnPossess
-> Blackboard 초기화
-> Initial Blackboard Value 설정
-> BehaviorTree 실행
```

**Structure**
```yaml
ACAIController
- Enemy pawn cache
- Blackboard asset & Blackboard key validation
- initial blackboard value setup
- BehaviorTree start

CAIKey
- State / Targeting / Perception / Metric / Navigation
- Patrol / Investigate / Chase / Alert / Engage
- Reaction / Dead
```

#### B. Perception / Target Context 구성

- AI perception 결과를 target context로 caching하고, `LOS`, `TargetPriority`, `LastKnownLocation`, `memory timeout` 기준을 Blackboard에 반영하도록 구성했다.

**Flow**
```yaml
AIPerception
-> TargetDataMap update
-> TargetPriority / LOS / memory timeout 계산
-> AIContext update
-> Blackboard update
```

**Structure**
```yaml
Target Context
- TargetActor       : 현재 target actor
- TargetPriority    : target selection priority
- bHasLOS           : line of sight 여부
- LastSeenTime      : 마지막 perception time
- LastKnownLocation : 마지막 known target location
```

#### C. AIContext 및 AIState 갱신 서비스

- BehaviorTree service에서 현재 상황을 AIContext로 update하고, 해당 context를 기준으로 AIState를 resolve하도록 구성했다.

**Flow**
```yaml
BT Service Tick
-> Update AIContext
-> Resolve AIState
-> Update AIState Blackboard key
-> Select BT Branch
```

**Rule**
```yaml
AIState Priority
- Dead
- HitReact
- Engage
- Investigate / Chase / Alert
- Patrol / Idle
```

#### D. BehaviorTree State Branch 구성

- AIState를 기준으로 BehaviorTree branch를 분기하고, 각 state가 필요한 task / service / decorator를 통해 실행되도록 구성했다.

**Flow**
```yaml
AIState
-> Selector / Decorator
-> State Branch
-> Task / Subtree 실행
```

**Structure**
```yaml
BT Branch
- Idle
- Patrol
- Investigate
- Chase
- Alert
- Engage
- HitReact
- Dead
```

#### E. Patrol / Investigate / Chase / Alert의 Movement Flow

- Engage 전후의 investigate, chase, alert의 movement flow를 BehaviorTree에서 branch와 service, decorator, task 단위로 구성했다.

**Flow**
```yaml
Patrol
-> PatrolPath / PatrolPoint
-> SelectPatrolPoint
-> MoveTo

Investigate
-> LastKnownLocation
-> StartInvestigate
-> Investigate index advance
-> EndInvestigate

Chase / Alert
-> target distance / alert range
-> chase movement 또는 alert step movement
```

**Structure**
```yaml
Movement Context
- PatrolPath / PatrolPoint
- InvestigateLocation / InvestigateIndex
- Chase range / hysteresis
- AlertStepLocation
```

#### F. Combat Role Assignment 및 Engage Context 구성

- 여러 Enemy의 perception target이 같을 때, Combat Role Assignment를 통해 각 AI의 combat role을 나누도록 구성했다.

- Engage role을 받은 AI만 Engage state에서 attack 가능 여부를 계산하고, 실제 attack은 `SBT_Attack` subtree에서 실행하도록 분리했다.

**Flow**
```yaml
AIContext
-> Engage Request
-> UCWorldSubsystem_CombatEngage
-> Combat Role Assignment
-> UpdateEngageContext
-> bInEngageRange / bCanCombatAction 계산
```

**Structure**
```yaml
UCWorldSubsystem_CombatEngage
- Engage request container
- Combat Role Assignment container
- target별 Combat Role Assignment

Engage Blackboard
- bShouldEngage
- bInEngageRange
- bCanCombatAction
- bIsCombatAction
- NextCombatActionTime
```

#### G. Attack Subtree 및 Attack Loop 구성

- Attack을 상위 AIState가 아니라 `Engage` 내부의 하위 execution flow로 구성했다.

**Flow**
```yaml
Engage Branch
-> SBT_Attack 실행

SBT_Attack
-> SelectAttackIndex
-> StartAttack
-> WaitAttackEnd
-> CommitAttackCooldown
```

**Structure**
```yaml
Attack Flow Element
- SelectAttackIndex
- StartAttack
- WaitAttackEnd
- CommitAttackCooldown
- AnimNotify_EndEnemyAttack
```

#### H. HitReact State Flow 구성

- 피격 이후 AI가 HitReact branch로 진입하고, reaction 종료까지 BehaviorTree가 active reaction state를 관찰할 수 있도록 구성했다.

**Flow**
```yaml
TakeDamage / Reaction Request
-> Pending Reaction
-> HitReact Branch
-> StartReaction
-> WaitEndReaction
-> AIState 복귀
```

**Structure**
```yaml
Reaction Context
- pending reaction
- active reaction
- reaction priority
- bIsActiveReaction
```

#### I. Dead State Flow 구성

- AI의 dead state flow를 Blackboard와 AnimInstance에 sync하도록 구성했다.

**Flow**
```yaml
Health / DeadState 변경
-> Blackboard DeadState update
-> Dead Branch 진입
-> StayDead / WaitDeadState
```

**Structure**
```yaml
DeadState
- Alive
- Dying
- Dead

Dead Task
- StayDead
- WaitDeadState
- AnimNotify_EnterDeadState
```

#### J. Animation / Montage / Weapon / Asset 정리

- AI state branch에서 사용할 animation, montage, weapon asset을 정리하고 테스트 가능한 상태로 연결했다.

**Changes**
```yaml
Animation / Montage
- Dead / Dying / HitReact montage 정리
- AI sword attack / draw / sheath montage 추가
- sword / unarmed animation naming 정리

Weapon / Asset
- sword weapon asset 교체
- Blackboard / BehaviorTree / Subtree asset 갱신
- test level 검증 환경 갱신
```

---
## 안정성 보완

### AIContext Blackboard 정리 안정화 (B03 보완)

#### A. Service Early-return 이후 Blackboard 잔여값 제거

- AIContext update 중 target 정보가 없거나 계산이 실패하는 경우에도 Blackboard가 이전 값을 유지하지 않도록 정리했다.
- 자세한 재현 조건과 원인은 `B03` Bug Report에서 분리하여 정리했다.

**Flow**
```yaml
UCBTService_UpdateAIContext
-> AIContext update result 확인
-> Success / NoData / Error 분기
-> TargetActor / bHasLOS / DistanceToTarget / bInRange 명시적 갱신
```

**Rule**
```yaml
Success
- 현재 AIContext 기준 Blackboard 값 Set

NoData / Error
- target 관련 Blackboard 값 Clear
- 상태 전이 판단에 stale value가 남지 않도록 처리
```

### AttackIndex 초기화 안정화 (B04 보완)

#### A. AIState 전이 시 AttackIndex 명시 초기화

- AIState 전이 후 이전 attack context가 남아 ComboAttack이 같은 단계로 반복되는 문제를 정리했다.
- 자세한 재현 조건과 원인은 `B04` Bug Report에서 분리하여 정리했다.

**Flow**
```yaml
AIState 전이
-> Attack context cleanup
-> AttackIndex를 INDEX_NONE으로 갱신
-> 다음 attack 선택 시 새 context 기준으로 SelectAttackIndex 실행
```

**Rule**
```yaml
AttackIndex
- active attack context가 없으면 INDEX_NONE
- 새 attack 선택 시 SelectAttackIndex에서 갱신
- Blackboard ClearValue만으로 초기화하지 않음
```

---
## 주요 Pipeline

### AI Decision Pipeline

```yaml
Perception
-> Update AIContext
-> Resolve AIState
-> Select BehaviorTree Branch
-> Execute Task / Subtree
-> Movement / Attack / Reaction
```

### Engage / Attack Pipeline

```yaml
Target Context
-> Combat Role Assignment
-> Engage Branch
-> SBT_Attack
-> StartAttack
-> WaitAttackEnd
-> Cooldown Commit
```

### Reaction Pipeline

```yaml
TakeDamage Result
-> Pending Reaction
-> Blackboard Reaction Context
-> HitReact Branch
-> StartReaction
-> WaitEndReaction
-> AIState 복귀
```

### Dead Pipeline

```yaml
DeadState 변경
-> Blackboard DeadState
-> Dead Branch
-> StayDead / WaitDeadState
```

---
## 테스트 방법

### AI Core

- Enemy AI가 포함된 test level에서 `ACAIController` possess가 정상 동작하는지 확인
- `BB_Default`, `BT_Default`가 초기화되고 실행되는지 확인
- Blackboard key validation과 초기값 설정이 정상 동작하는지 확인

### AIState 전이

- `Idle / Patrol / Investigate / Chase / Alert / Engage / HitReact / Dead` branch 진입 여부 확인
- target perception, LOS lost, alert range, Combat Role Assignment에 따라 state가 전이되는지 확인
- 상태 전이 시 관련 Blackboard key가 정리되는지 확인

### Patrol / Investigate / Chase / Alert

- patrol path / point 기반 movement가 정상 동작하는지 확인
- target 상실 후 `LastKnownLocation` 기반 investigate가 실행되는지 확인
- chase distance 조건과 alert step movement가 정상 동작하는지 확인

### Engage / Attack

- `UCWorldSubsystem_CombatEngage`의 Combat Role Assignment 결과가 `bShouldEngage`에 반영되는지 확인
- `bInEngageRange`, `bCanCombatAction` 계산이 정상 동작하는지 확인
- `SBT_Attack` 진입 후 `StartAttack -> WaitAttackEnd -> CommitAttackCooldown` 흐름이 정상 종료되는지 확인

### Reaction / Dead

- 피격 시 `HitReact` branch에 진입하고 reaction 종료 후 상태가 복귀되는지 확인
- 사망 시 `DeadState`가 Blackboard / AnimInstance에 동기화되는지 확인

### Movement Branch Control

- `CanMove` decorator가 이동 sequence 진입을 제어하는지 확인
- 공격, reaction, dead 상태에서 이동 branch가 의도대로 차단되는지 확인

---
## 검증 결과

- `ACAIController` possess 이후 Blackboard / BehaviorTree 초기화 확인
- `Perception -> Update AIContext -> Resolve AIState -> Select BT Branch` state transition 확인
- Patrol / Investigate / Chase / Alert movement branch 동작 확인
- Combat Role Assignment와 Attack subtree 동작 확인
- HitReact / Dead 상태 흐름 확인
- `CanMove` 기반 movement branch gating 확인

---
## 관련 문서

- Issue Checklist: `D11_UE5_Portfolio_Issue_Checklist.md`

- Issue Analysis Report:
	- `I01_UE5_Portfolio_Issue_Analysis_Report.md`
	- `I02_UE5_Portfolio_Issue_Analysis_Report.md`

- Bug Report:
	- `B03_UE5_Portfolio_Bug_Report.md`
	- `B04_UE5_Portfolio_Bug_Report.md`

---
## 정리

이 PR의 핵심은 Enemy AI를 단순 추적 / 단일 attack 실행 구조에서, `perception -> AIContext -> AIState -> BT Branch` 기반 실행 구조로 확장한 것이다.

변경 후 책임은 다음과 같이 정리됐다.

```yaml
ACAIController
- Blackboard / BehaviorTree 초기화

BT Service
- AIContext update
- AIState resolve

BT Branch
- state별 task / subtree 실행
```

또한 기존 `Combat` state가 담당하던 전투 참여 판단을 `Engage` 기준으로 재정리했다.

그 결과 `Alert / Engage`를 포함한 combat 영역 안에서도 실제 attack 실행 가능 여부를 구분할 수 있게 되었고, Attack은 상위 state가 아니라 `Engage` 내부 `SBT_Attack`에서 실행되도록 분리됐다.

이를 통해 movement / attack / reaction / dead flow를 하나의 BehaviorTree 안에서 상태별 branch로 관리할 수 있게 됐다.

---
