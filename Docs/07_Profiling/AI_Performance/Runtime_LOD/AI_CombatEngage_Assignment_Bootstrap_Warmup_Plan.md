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

## Expected Causes

초기 후보 수집은 단일 함수 호출로 끝나는 구조가 아니라 AI Perception, Blackboard 갱신, BT Service 호출, Engage request 제출이 여러 프레임에 걸쳐 이어지는 구조다.
따라서 80 Enemy가 같은 타겟을 동시에 볼 수 있는 배치에서도 모든 AI가 같은 rebuild frame에 request를 제출한다고 가정하기 어렵다.

예상 원인은 다음과 같다.

- `AI Perception`은 모든 Enemy의 감지 결과를 한 프레임에 안정적으로 올려주지 않는다.
- 감지 결과가 들어와도 즉시 assignment request가 되는 것이 아니라 `UpdateAIContext`, `UpdateAIIntentState`, `UpdateEngageContext` 같은 BT Service 갱신 단계를 거친다.
- 초기 상태 전환은 `Perception -> Blackboard -> IntentState -> EngageRequest -> RebuildAssignments -> BT 상태 반영` 순서로 이어지므로 여러 갱신 주기를 통과해야 한다.
- Enemy 수가 많아질수록 perception / BT scheduling 편차가 커지고, request snapshot이 `6 -> 13 -> 32 -> 62 -> 80`처럼 단계적으로 채워진다.
- Assignment cap이 작을 때는 초기 request pool만으로도 자연스럽게 보일 수 있지만, Engage / Alert 총합이 커질수록 더 넓은 후보군이 들어올 시간이 필요하다.

관찰상 80 Enemy 기준에서 `1.0s`는 최소 동작 후보지만 request snapshot이 `32 ~ 80` 사이에서 흔들릴 수 있었다.
`1.2s`는 현재 테스트 조건에서 더 안정적인 warmup 후보로 본다.
이는 초기 request pool 형성 시간이 시스템 부하와 scheduling 편차에 영향을 받으며, 80 Enemy 기준에서는 1초 부근이 경계값에 가깝다는 근거다.

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

기본값:

```text
0.0s
```

기본값이 0이면 기존 동작과 동일하게 즉시 assignment를 확정한다.
측정 시에는 1.0s 또는 1.2s를 사용한다.

측정 후보:

```text
0.0s / 1.0s / 1.2s
```

Warmup 검증용 로그:

```text
Portfolio.AI.RuntimeLOD.EngageAssignmentAudit
Portfolio.AI.RuntimeLOD.EngageAssignmentVerboseAudit
```

`EngageAssignmentAudit`은 warmup 지연 중 request count와 최초 assignment summary만 출력한다.
CSV 성능 측정 시에는 로그 영향을 줄이기 위해 0으로 둔다.
`EngageAssignmentVerboseAudit`은 후보 목록과 상세 assignment 추적이 필요할 때만 사용한다.

검증 목적 기본값:

```text
EngageAssignmentAudit 1
EngageAssignmentVerboseAudit 0
```

구현 상태:

```text
UCWorldSubsystem_CombatEngage에 CVar 추가 완료
Warmup 중 RequestContainer consume 보류 완료
Warmup 종료 후 최초 RebuildAssignments에서 request snapshot consume 완료
EngageAssignmentAudit / VerboseAudit debug 출력 제어 추가 완료
```

### Runtime State

```text
float AssignmentWarmupStartTime
bool bAssignmentWarmupCompleted
```

첫 request가 들어온 시간을 `AssignmentWarmupStartTime`으로 기록하고, 현재 시간과의 차이로 warmup 경과 시간을 계산한다.

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
EngageAssignmentAudit 1
EngageAssignmentVerboseAudit 0
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

## Validation Result

### 80 Enemy / WarmupTime 1.0

측정 파일:

```text
Profile(20260709_165800).csv
Log(20260709_165800).txt
```

조건:

```text
EngageAssignmentWarmupTime 1.0
EngageAssignmentAudit 1
EngageAssignmentVerboseAudit 0
BTUpdateIntervalMode 0
Engage 2 / Alert 6
```

로그 결과:

```text
Warmup 중 RequestCount 대표 패턴: 6 -> 8/13 -> 32/44
Warmup 종료 후 RequestSnapshot 관찰값: 32 / 74 / 80
FinalEngage: 2
FinalAlert: 6
FinalTotal: 8
```

해석:

- Warmup 종료 전 assignment 확정은 지연됐다.
- 기존에 보였던 2명 / 6명이 나뉘어 출발하는 현상은 사라졌다.
- 첫 측정에서 상대적으로 먼 Enemy가 선택되는 비합리적인 선정이 1회 관찰됐지만, 이후 약 30회 PIE 초반 반복 관찰에서는 재발하지 않았다.
- `WarmupTime 1.0`은 bootstrap split 완화 효과는 확인됐지만, request snapshot이 32 / 74 / 80으로 흔들릴 수 있어 안정값으로 보기에는 경계에 가깝다.
- 이후 BT interval 재측정 baseline은 `WarmupTime 1.2`를 우선 사용한다.

### 80 Enemy / WarmupTime 1.2

반복 관찰 결과:

```text
Warmup 중 RequestCount 대표 패턴:
6 -> 11/13/17 -> 38/44/50 -> 80
6 -> 8 -> 32 -> 74 -> 80
6 -> 7 -> 26 -> 68 -> 80

Warmup 종료 후 RequestSnapshot: 80
FinalEngage: 2
FinalAlert: 6
FinalTotal: 8
```

해석:

- `WarmupTime 1.2`에서는 여러 반복 관찰에서 최초 assignment 확정 시점의 `RequestSnapshot`이 80으로 수렴했다.
- 일부 케이스는 request count가 1.0초를 넘긴 뒤에야 80에 도달했다.
- 따라서 80 Enemy 기준의 BT interval 재측정 baseline은 `WarmupTime 1.2`를 사용한다.
- `WarmupTime 1.0`은 최소 동작 후보로 남기되, 안정 측정 기준으로는 사용하지 않는다.

## Risk

```text
WarmupTime이 길면 첫 반응성이 늦어진다.
Warmup 중에도 target을 본 Enemy가 아무 행동을 하지 않는 것처럼 보일 수 있다.
실게임 적용 시에는 전투 시작 상황, 스폰 상황, 대량 AI 상황을 구분할 필요가 있다.
```

## Follow-up

Warmup 적용 후에도 초기 멤버 흔들림이 남으면 그때 `Strong Initial Reselection`을 보조 정책으로 검토한다.
다만 1차 구현에서는 Warmup만 적용해 원인에 직접 대응한다.
