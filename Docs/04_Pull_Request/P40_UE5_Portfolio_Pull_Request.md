# UE5 Portfolio Pull Request

## 제목

**P40: AI Enemy Actor Tick 비용 분리 측정**

## 날짜

**2026.07.11**

## 상태

- [x] Enemy Actor Tick 측정 범위 정의
- [x] `EnemyActorTickMode` CVar 추가
- [x] `ACEnemy::Tick` 비활성화 모드 구현
- [x] Enemy Actor Tick 전용 40 / 80 Enemy 측정 맵 추가
- [x] 40 Enemy 2쌍, 80 Enemy 3쌍 반복 측정
- [x] Runtime LOD v1 우선순위 판단 정리

## 브랜치

- `feature/ai-enemy-actor-tick-profiling`

## 요약

이번 PR은 AI Runtime LOD 후보 중 `Enemy Actor Tick` 축을 분리 측정한다.

측정 대상은 Unreal Engine 전체 tick이 아니라 프로젝트 코드가 직접 가진 Enemy-side actor tick이다. 그중 이번 PR에서는 `ACEnemy::Tick`을 제어했다.

결론적으로 `EnemyActorTickMode 1`은 `Ticks/CEnemy`를 제거하는 데 성공했다. 그러나 40 / 80 Enemy 반복 측정에서 Frame / Game p95 개선은 안정적으로 재현되지 않았다. `BT Tick`은 비교적 일정했고, Frame / Game 변동은 `CharacterMovement`와 `Animation` 변동을 더 크게 따라갔다.

따라서 `ACEnemy::Tick` 제거는 제거 가능한 polling tick 검증으로는 의미가 있지만, 현재 Runtime LOD v1의 주요 병목 축으로 보기는 어렵다. 후속 분석은 `Movement Isolation`과 `Animation Isolation`으로 분리한다.

## 주요 변경

```text
1. Enemy actor tick profiling CVar 추가
   - Portfolio.AI.RuntimeLOD.EnemyActorTickMode
   - 0: Default
   - 1: EnemyActorTickDisabled

2. ACEnemy actor tick 제어 추가
   - original actor tick enabled state cache
   - Mode 1에서 ACEnemy::Tick 비활성화
   - CActionComponent / CMovementComponent / CharacterMovement / BT / SkeletalMesh tick은 유지

3. Enemy Actor Tick 측정 맵 추가
   - MAP_AIPerf_ComponentTick_40Enemy
   - MAP_AIPerf_ComponentTick_80Enemy
   - 측정 명칭과 CVar는 `EnemyActorTick`으로 정리했다. 다만 UE asset rename / redirector churn을 피하기 위해 기존 측정 맵 에셋명은 유지했다.

4. Enemy Actor Tick 측정 문서 정리
   - 측정 범위 명시
   - 40 / 80 Enemy 반복 측정 결과 기록
   - Movement / Animation 후속 분리 계획 정리
```

## CVar

```text
Portfolio.AI.RuntimeLOD.EnemyActorTickMode
```

| Mode | 이름 | 제어 내용 |
| ---: | --- | --- |
| 0 | Default | 기본 tick 유지 |
| 1 | EnemyActorTickDisabled | `ACEnemy::Tick` 비활성화 |

주의:

```text
Mode 1은 ACEnemy actor tick 자체를 끄므로 PIE 중 1 -> 0 runtime restore 검증에는 적합하지 않다.
정규 측정은 PIE 시작 전 CVar를 설정한 뒤 진행한다.
```

## 변경 파일

```text
Source/Portfolio/Character/Enemy/CEnemy.h
Source/Portfolio/Character/Enemy/CEnemy.cpp

Content/00_Profiling/00_AI_Performance/00_Map/11_ComponentTick/MAP_AIPerf_ComponentTick_40Enemy.umap
Content/00_Profiling/00_AI_Performance/00_Map/11_ComponentTick/MAP_AIPerf_ComponentTick_80Enemy.umap

Docs/04_Pull_Request/00_Pull_Request_Index.md
Docs/04_Pull_Request/P40_UE5_Portfolio_Pull_Request.md
Docs/07_Profiling/AI_Performance/Runtime_LOD/AI_Enemy_Actor_Tick_Audit_Plan.md
Docs/07_Profiling/AI_Performance/Runtime_LOD/README.md
```

## 측정 조건

```text
Capture Duration: 약 36~37초
Analysis Window: first 3s / last 3s trimmed
Log State: -noailogging
PIE: F11 fullscreen
Fixed camera
GC Event: none
```

공통 CVar:

```text
Portfolio.AI.RuntimeLOD.BTUpdateIntervalMode 0
Portfolio.AI.RuntimeLOD.EnemyMeshMode 0
Portfolio.AI.RuntimeLOD.EnemyAnimationMode 0
Portfolio.AI.RuntimeLOD.EnemyAnimationRefreshCounter 0
Portfolio.AI.RuntimeLOD.DisableEnemyWeaponActor 0
Portfolio.AI.RuntimeLOD.DisableEnemyPerception 0
Portfolio.AI.RuntimeLOD.PerceptionCandidateAudit 0
Portfolio.AI.RuntimeLOD.BlackboardEngageLatencyAudit 0
Portfolio.AI.RuntimeLOD.CanMoveDecoratorAudit 0
Portfolio.AI.RuntimeLOD.EnemyMovementMode 0
Portfolio.AI.RuntimeLOD.DisableEnemyHitProcessing 0
Portfolio.AI.RuntimeLOD.DisableEnemyCombatFeedback 0
```

Gameplay smoke:

```text
Engage 2 유지
Alert 6 유지
Observe / Idle 계층 유지
attack montage 정상
hit / guard / parry result 정상
feedback presentation 정상
```

## 측정 결과

### 40 Enemy

| Set | Mode | Frame p95 | Game p95 | TickActors p95 | CharacterMovement p95 | Animation p95 | BT Tick p95 | CEnemy Tick |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | --- |
| 40 R1 | 0 | 11.9250ms | 10.8108ms | 1.4724ms | 0.5084ms | 1.9996ms | 0.2198ms | 40 |
| 40 R1 | 1 | 12.0640ms | 11.0778ms | 1.4959ms | 0.5112ms | 2.0839ms | 0.2195ms | missing |
| 40 R2 | 0 | 11.9344ms | 11.0786ms | 1.5314ms | 0.5191ms | 2.1083ms | 0.2191ms | 40 |
| 40 R2 | 1 | 11.9257ms | 11.1269ms | 1.4990ms | 0.5449ms | 2.1380ms | 0.2232ms | missing |

### 80 Enemy

| Set | Mode | Frame p95 | Game p95 | TickActors p95 | CharacterMovement p95 | Animation p95 | BT Tick p95 | CEnemy Tick |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | --- |
| 80 R1 | 0 | 17.8866ms | 17.8446ms | 2.9108ms | 0.9470ms | 4.0210ms | 0.4111ms | 80 |
| 80 R1 | 1 | 17.1558ms | 17.1629ms | 2.7634ms | 0.9095ms | 3.8622ms | 0.4075ms | missing |
| 80 R2 | 0 | 16.7917ms | 16.7457ms | 2.7682ms | 0.8442ms | 3.7991ms | 0.4129ms | 80 |
| 80 R2 | 1 | 17.2328ms | 17.1904ms | 2.7561ms | 0.9052ms | 3.9476ms | 0.4171ms | missing |
| 80 R3 | 0 | 19.1129ms | 19.1179ms | 3.0502ms | 0.9456ms | 4.3873ms | 0.4273ms | 80 |
| 80 R3 | 1 | 17.6868ms | 17.7054ms | 2.9069ms | 0.9464ms | 4.1182ms | 0.4171ms | missing |

## 해석

`EnemyActorTickMode 1`은 기능적으로 성공했다.

```text
Mode 0: Ticks/CEnemy = 40 또는 80
Mode 1: Ticks/CEnemy = missing
```

하지만 Frame / Game p95 개선은 반복 측정에서 안정적으로 재현되지 않았다.

```text
40 Enemy:
ACEnemy tick 제거 후에도 Frame / Game / TickActors 개선이 거의 없다.

80 Enemy:
일부 측정에서는 Mode 1이 좋아 보이지만, 다른 측정에서는 반대로 튄다.
R3에서는 Mode 0의 Animation / GameThread가 크게 튀면서 Mode 1 개선처럼 보이는 부분이 있다.
```

`BT Tick`은 반복 측정에서 비교적 안정적이다. 반면 Frame / GameThread 변동은 `CharacterMovement`와 `Animation` 변동을 더 크게 따라간다.

따라서 이번 PR의 결론은 다음과 같다.

```text
ACEnemy actor tick 제거는 제거 가능한 polling tick 검증으로는 의미가 있다.
하지만 현재 Runtime LOD 성능 병목의 본체는 아니다.
후속 분석은 Movement와 Animation을 독립적으로 통제하는 환경에서 진행해야 한다.
```

## 검증

```text
1. PortfolioEditor Development build
2. 40 Enemy Mode 0 / 1 반복 측정
3. 80 Enemy Mode 0 / 1 반복 측정
4. GC 이벤트 없음 확인
5. Ticks/CEnemy 제거 여부 확인
6. CActionComponent / CMovementComponent / CharacterMovement / BT / SkeletalMesh tick 유지 확인
7. Frame / Game / TickActors / CharacterMovement / Animation / BT Tick p95 비교
```

## 제외 범위

```text
1. CharacterMovementComponent 직접 off
   - Movement / Nav 축에서 별도 해석한다.

2. SkeletalMeshComponent / pose update 직접 off
   - Animation / Mesh 축에서 별도 해석한다.

3. BehaviorTreeComponent 직접 off
   - BT update interval 축에서 별도 해석한다.

4. CMovementComponent tick off
   - 이전 Movement / Nav 측정에서 representation 손상이 확인된 축이다.
   - 이번 PR에서는 ACEnemy actor tick만 1차로 분리한다.

5. CActionComponent tick off
   - combat action lifecycle을 깨뜨릴 가능성이 있어 이번 PR 범위에서 제외한다.
```

## 후속 작업

```text
1. Movement Isolation
   - Observe Static vs Alert Movement 비교
   - active movement 후보 수 / movement intent / path following 비용 분리
   - CharacterMovement p95, TickActors p95, Frame / Game p95 중심 해석

2. Animation Isolation
   - movement를 고정하거나 최소화한 조건에서 animation / pose / locomotion detail 분리
   - Animation p95, SkeletalMeshComponent tick, Frame / Game p95 중심 해석

3. Combined Stress
   - Movement / Animation 분리 결과를 실제 Engage / Alert / Observe gameplay stress 조건에서 재확인
```
