# AI CombatEngage Assignment Bootstrap Warmup Plan

## Purpose

BT update interval LOD 측정 중 확인된 CombatEngage assignment 초기 선점 문제를 정리한다.

현재 문제는 assignment sort 자체가 잘못된 것이 아니라, 모든 AI가 request를 제출하기 전에 일부 AI만 먼저 assignment를 선점하는 데 있다.
따라서 최초 assignment 확정 시점을 안정화하는 warmup 정책을 우선 검토한다.

## Observed Issue

80 Enemy 조건에서 Engage request snapshot은 한 번에 80개가 들어오지 않고 단계적으로 증가했다.

```text
Rebuild 1 : RequestCount = 0
Rebuild 2 : RequestCount = 6
Rebuild 14: RequestCount = 13
Rebuild 15: RequestCount = 32
Rebuild 16: RequestCount = 62
Rebuild 17: RequestCount = 80
```

초기 request 6개가 먼저 assignment를 얻으면, 이후 더 가까운 Enemy가 request를 제출해도 기존 assignment lease / preserve 정책 때문에 즉시 교체되지 않는다.
그 결과 배치상 더 가까운 Enemy가 있어도 최초로 인지된 일부 Enemy가 Engage / Alert 권한을 선점하는 현상이 발생했다.

## Current Flow

```text
AI Perception / BT service update
-> UCBTService_UpdateAIContext
-> BuildPerceptionContext
-> ComputeEngageAssignmentContext
-> UCWorldSubsystem_CombatEngage::SubmitRequest
-> UCWorldSubsystem_CombatEngage::RebuildAssignments
-> SortRequestContexts
-> Engage / Alert assignment 확정
```

문제 지점은 `SortRequestContexts` 이후가 아니라 그 이전이다.
정렬은 현재 request snapshot에 들어온 후보만 대상으로 한다.
따라서 아직 request를 제출하지 못한 AI는 거리상 더 가까워도 비교 대상이 될 수 없다.

## Candidate Policies

### 1. Assignment Warmup

최초 N초 또는 request count 안정화 전에는 assignment 확정을 미룬다.

```text
BeginPlay / subsystem active
-> request 수집
-> warmup 동안 assignment 적용 보류
-> warmup 종료 시점에 request bucket 정렬
-> 최초 assignment 확정
-> 이후 기존 lease / preserve 정책 사용
```

장점:

- 원인에 직접 대응한다.
- 후보가 충분히 모인 뒤 최초 assignment를 결정한다.
- 기존 lease / preserve 정책과 충돌이 적다.
- 측정 조건을 안정화하기 쉽다.

주의점:

- warmup이 너무 길면 첫 반응이 늦어진다.
- 실게임 적용 시 전역 고정값이 아니라 상황별 설정값이어야 한다.

### 2. Strong Initial Reselection

초기 구간에서 기존 assignment 유지보다 거리 / 우선순위 재선정을 강하게 적용한다.

장점:

- 늦게 들어온 가까운 후보가 기존 선점자를 밀어낼 수 있다.
- 첫 반응을 완전히 지연하지 않는다.

주의점:

- 초기 몇 초 동안 Engage / Alert 멤버가 흔들릴 수 있다.
- movement가 시작된 뒤 role이 바뀌는 시각적 불안정이 생길 수 있다.
- lease / preserve 정책과 충돌하기 쉽다.
- 아직 request에 들어오지 않은 후보는 여전히 비교할 수 없다.

## Decision

현재 프로젝트에서는 `AssignmentWarmup`을 우선 적용한다.

판단 근거:

- 실제 로그에서 request 후보가 `6 -> 13 -> 32 -> 62 -> 80`으로 단계적으로 채워지는 것이 확인됐다.
- 문제의 본질은 잘못된 정렬이 아니라 불완전한 후보군을 너무 빨리 확정하는 것이다.
- 초기 재선정 강화는 보정책일 뿐, 아직 들어오지 않은 후보를 비교할 수 없다는 근본 문제를 해결하지 못한다.

## Implementation Plan

### CVar

```text
Portfolio.AI.RuntimeLOD.EngageAssignmentWarmupTime
```

권장 초기값:

```text
0.5s 또는 1.0s
```

측정용으로는 0.0 / 0.5 / 1.0을 비교한다.

### Runtime State

```text
float AssignmentWarmupElapsedTime
bool bAssignmentWarmupCompleted
```

또는 subsystem 시작 시간을 기준으로 현재 시간이 warmup 종료 시점을 지났는지 판단한다.

### RebuildAssignments Policy

Warmup 중:

```text
SubmitRequest는 계속 받는다.
RequestContainer는 controller key 기준 최신 request로 유지한다.
RebuildAssignments는 assignment 적용을 보류한다.
RequestContainer를 consume/reset하지 않는다.
```

Warmup 종료 시:

```text
RequestContainer snapshot 생성
target별 bucket 생성
bucket별 SortRequestContexts 수행
최초 Engage / Alert assignment 확정
이후 기존 lease / preserve 정책 사용
RequestContainer reset
```

Warmup 이후:

```text
기존 AssignmentLease / PreserveExistingEngage / PromoteExistingAlert / PreserveExistingAlert / ApplyFreshRequestAssignments 흐름을 유지한다.
```

## Validation Plan

측정 조건:

```text
40 Enemy / 80 Enemy
BTUpdateIntervalMode 0 / 1 / 2
EnemyMeshMode 0
EnemyAnimationMode 0
DisableEnemyWeaponActor 0
DisableEnemyPerception 0
PerceptionCandidateAudit 0
BlackboardEngageLatencyAudit 0
CanMoveDecoratorAudit 0
EnemyMovementMode 0
```

확인 항목:

```text
초기 RequestCount 증가 패턴
Warmup 종료 전 assignment 미확정 여부
Warmup 종료 후 최초 assignment가 거리 / 우선순위 기준으로 형성되는지
Engage / Alert cap이 정상 적용되는지
Warmup 이후 lease / preserve 정책이 기존처럼 안정적으로 동작하는지
BTUpdateIntervalMode 1 / 2에서도 Engage / Alert / Idle 계층이 유지되는지
```

## Expected Result

```text
초기 6개 request 선점 현상이 줄어든다.
최초 assignment가 더 완성된 후보군 기준으로 결정된다.
BT interval LOD 측정에서 시작 직후 assignment bootstrap 변수가 줄어든다.
```

## Risk

```text
WarmupTime이 길면 첫 반응성이 늦어진다.
Warmup 중에도 target을 본 Enemy가 아무 행동을 하지 않는 것처럼 보일 수 있다.
실게임 적용 시에는 전투 시작 상황, 스폰 상황, 대량 AI 상황을 구분할 필요가 있다.
```

## Follow-up

Warmup 적용 후에도 초기 멤버 흔들림이 남으면 그때 `Strong Initial Reselection`을 보조 정책으로 검토한다.
다만 1차 구현에서는 Warmup만 적용해 원인에 직접 대응한다.
