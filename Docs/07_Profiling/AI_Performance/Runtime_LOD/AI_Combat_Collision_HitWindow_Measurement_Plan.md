# AI Combat Collision / Hit Window Measurement Plan

## 목적

`Combat Collision / Hit Window` 축이 40 / 80 Enemy 조건에서 실제 frame budget에 어느 정도 영향을 주는지 분리 측정한다.

이번 브랜치는 최종 Runtime LOD 구현이 아니라 비용 분리 측정이다. `WeaponActor` 생성 비용, attack montage 비용, feedback presentation 비용과 섞지 않고, 공격 판정 window가 열렸을 때 발생하는 collision / overlap / hit processing 경로를 확인한다.

## 브랜치

```text
feature/ai-combat-collision-profiling
```

## 측정 질문

```text
Enemy WeaponActor와 attack montage를 유지한 상태에서
hit processing 또는 hit collision 경로를 차단하면
Frame / Game / hit route 비용이 유의미하게 줄어드는가?
```

## 측정 범위

포함:

```text
WeaponActor 생성 유지
WeaponActor attach / socket follow 유지
attack montage 실행 유지
AnimNotify / hit window notify route 유지
Engage / Alert / Observe 정책 유지
```

1차 분리 축:

```text
hit window open / close가 정상 호출되는지
weapon overlap / hit processing route가 실제로 진입하는지
hit processing count와 combat signal commit count가 어떻게 갈라지는지
```

1차 제외:

```text
WeaponActor 생성 자체 제거
attack action / montage 제거
trail / Niagara / sound / camera shake 제거
proxy / dormant actor 전환
```

Feedback presentation은 다음 축으로 분리한다. 이번 측정에서 feedback까지 함께 끄면 hit collision 비용과 feedback 비용을 구분하기 어렵다.

## 계측 정책

### CSVProfiler 기준

이번 축은 `AnimNotify`, overlap delegate, combat signal request처럼 event 기반 호출이 많다.

직접 event 함수 안에서 `CSV_CUSTOM_STAT_GLOBAL(..._Count)` 또는 `CSV_SCOPED_TIMING_STAT_GLOBAL(...)`를 호출하는 방식은 실제 로그 호출이 확인되어도 CSV 컬럼에 안정적으로 남지 않았다. 반면 기존 BT service, subsystem tick, anim update 계측은 tick 또는 tick에 준하는 주기적 실행 지점에서 기록되기 때문에 안정적으로 잡혔다.

따라서 Combat Collision 축의 공식 계측 기준은 다음으로 둔다.

```text
event 지점: FCombatCollisionProfilingCounters::Record...()로 카운터만 누적
flush 지점: UCWorldSubsystem_CombatEngage::Tick()에서 FlushToCsv()
CSV 해석: *_FlushCount 컬럼만 공식 카운트로 사용
```

이 방식은 event 발생 자체를 직접 CSV에 쓰지 않고, stable tick phase에서 누적값을 CSV로 내보낸다. event 단위의 정밀한 duration이 필요하면 CSVProfiler보다 Unreal Insights 또는 별도 scoped trace를 사용한다.

### 공식 카운터

```text
PortfolioAI_CollisionNotify_Begin_FlushCount
PortfolioAI_CollisionNotify_End_FlushCount
PortfolioAI_ActionCollisionWindow_Begin_FlushCount
PortfolioAI_ActionCollisionWindow_End_FlushCount
PortfolioAI_WeaponComponent_OpenCollisionWindow_FlushCount
PortfolioAI_WeaponComponent_CloseCollisionWindow_FlushCount
PortfolioAI_HitWindow_Open_FlushCount
PortfolioAI_HitWindow_Close_FlushCount
PortfolioAI_HitWindow_Overlap_FlushCount
PortfolioAI_HitProcessing_FlushCount
PortfolioAI_CombatSignal_FlushCount
PortfolioAI_CombatSignalCue_Notify_FlushCount
PortfolioAI_ActionCombatSignalCue_FlushCount
PortfolioAI_AICombatSignalCue_Request_FlushCount
PortfolioAI_CombatSignalCue_Request_FlushCount
PortfolioAI_CombatSignalCue_Send_FlushCount
```

`HitWindow Overlap`과 `HitProcessing`은 후보 충돌 / 처리 진입 수를 본다. `CombatSignal`은 검증, 중복 hit window, friendly/self filtering 등을 통과해 실제 전투 신호로 commit된 수에 가깝다.

## Profiling CVar

### HitProcessingDisabled

```text
Portfolio.AI.RuntimeLOD.DisableEnemyHitProcessing
```

의미:

```text
0: 기본 hit processing 수행
1: Enemy overlap은 유지하지만 RequestCombatSignalSource 이후 hit processing을 skip
```

적용 기준:

```text
Enemy 전용
WeaponActor 생성 유지
attack montage 유지
AnimNotify 유지
hit window open / close 유지
weapon collision / overlap 유지
HitProcessing / CombatSignal 감소 확인
```

이 CVar는 profiling 전용이다. 목적은 `Overlap` 비용과 `HitProcessing / CombatSignal` 비용을 분리해서 보는 것이다.

## 계측 검증 기록

### 대표 FullCombat baseline

`20260711_104859 / 40`은 계측 검증에는 유효했지만 `HitWindow Overlap / HitProcessing`이 348로 과다하게 튀었다. 대표 baseline에서는 제외하고 참고 측정으로만 둔다.

대표 baseline은 다음 측정값을 사용한다.

| Case | CSV | Frame p95 | Game p95 | CharacterMovement p95 | BT Tick p95 | HitWindow Open / Close | HitWindow Overlap | HitProcessing | CombatSignal |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 40 Enemy / FullCombat | `Profile(20260711_111308).csv` | 12.6371ms | 12.6204ms | 0.4925ms | 0.2055ms | 24 / 24 | 42 | 42 | 38 |
| 80 Enemy / FullCombat | `Profile(20260711_110514).csv` | 17.5514ms | 17.5532ms | 0.7883ms | 0.4098ms | 24 / 24 | 34 | 34 | 30 |

해석:

```text
hit window open / close 횟수는 40 / 80 모두 안정적이다.
40 / 80 모두 overlap과 hit processing count가 비슷한 규모로 잡힌다.
104859처럼 overlap 후보가 과다하게 튄 측정은 대표값으로 사용하지 않는다.
```

### HitProcessingDisabled 결과

`Portfolio.AI.RuntimeLOD.DisableEnemyHitProcessing 1` 기준으로 40 / 80 Enemy를 측정했다.

공통 확인:

```text
Engage 2 유지
Alert 6 유지
attack montage 유지
hit feedback 유지
GC 이벤트 없음
```

| Case | CSV | Frame p95 | Game p95 | CharacterMovement p95 | BT Tick p95 | HitWindow Open / Close | HitWindow Overlap | HitProcessing | CombatSignal | CombatSignalCue |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 40 Enemy / FullCombat | `Profile(20260711_111308).csv` | 12.6371ms | 12.6204ms | 0.4925ms | 0.2055ms | 24 / 24 | 42 | 42 | 38 | 8 |
| 40 Enemy / HitProcessingDisabled | `Profile(20260711_120705).csv` | 12.5523ms | 12.5267ms | 0.5171ms | 0.2042ms | 26 / 24 | 47 | 0 | 0 | 8 |
| 80 Enemy / FullCombat | `Profile(20260711_110514).csv` | 17.5514ms | 17.5532ms | 0.7883ms | 0.4098ms | 24 / 24 | 34 | 34 | 30 | 8 |
| 80 Enemy / HitProcessingDisabled | `Profile(20260711_121022).csv` | 17.4999ms | 17.4853ms | 0.7731ms | 0.4024ms | 26 / 25 | 30 | 0 | 0 | 8 |

해석:

```text
HitProcessingDisabled는 hit window open / close, attack montage, overlap 후보 수집을 유지한 채 HitProcessing과 CombatSignal만 0으로 낮췄다.
따라서 현재 CVar는 collision window / overlap 비용과 hit processing 이후 전투 신호 경로를 분리하는 축으로 유효하다.
CombatSignalCue count는 8로 유지된다. 이는 montage / action cue 기반 presentation route가 hit processing gate와 별도 책임임을 의미한다.
Frame / Game p95 변화는 40 / 80 모두 작다. 현재 조건에서는 hit processing 이후 경로가 주요 프레임 병목이라고 보기는 어렵다.
다만 HitProcessing과 CombatSignal을 완전히 끊었으므로, 이후 collision-only / feedback-only 비교의 기준점으로 사용할 수 있다.
```

## 공통 측정 조건

```text
Capture Duration: 약 36초
Analysis Window: first 3s / last 3s trimmed, middle 30s used
Log State: -noailogging
PIE: F11 fullscreen
GC Event: none 권장
```

공통 CVar:

```text
Portfolio.AI.RuntimeLOD.EngageAssignmentWarmupTime 1.2
Portfolio.AI.RuntimeLOD.EngageAssignmentEngageCap 2
Portfolio.AI.RuntimeLOD.EngageAssignmentAlertCap 6

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
```

## 권장 맵

전용 map:

```text
MAP_AIPerf_CombatCollision_40Enemy
MAP_AIPerf_CombatCollision_80Enemy
```

전용 map이 아직 완전히 분리되지 않았거나 비교 안정성이 부족하면, P37 이후 안정화된 gameplay stress map을 기준으로 사용한다. 단, 문서에는 실제 사용한 map 이름을 반드시 남긴다.

## 권장 측정 순서

1차 비교:

```text
1. 40 Enemy / FullCombat / DisableEnemyHitProcessing 0
2. 40 Enemy / HitProcessingDisabled / DisableEnemyHitProcessing 1
3. 80 Enemy / FullCombat / DisableEnemyHitProcessing 0
4. 80 Enemy / HitProcessingDisabled / DisableEnemyHitProcessing 1
```

측정 전 시각 확인:

```text
Engage 2 유지
Alert 6 유지
나머지 Observe 또는 Idle 유지
Enemy WeaponActor 생성 유지
attack montage 실행 유지
hit / guard / parry 피드백 정상
```

## 분석 지표

우선 지표:

```text
Frame p95
Game p95
Exclusive/GameThread/BehaviorTreeTick
Exclusive/GameThread/CharacterMovement
HitWindow Open / Close FlushCount
HitWindow Overlap FlushCount
HitProcessing FlushCount
CombatSignal FlushCount
```

보조 지표:

```text
AIContext Count
AIIntent Count
EngageContext Count
ActorCount / Ticks 계열
CSV 로그의 GC 이벤트 여부
```

## 해석 기준

### A. HitProcessingDisabled에서 Overlap은 유지되고 HitProcessing / CombatSignal이 줄어드는 경우

```text
게이트 위치가 의도대로 작동한다.
Frame / Game p95 변화가 있으면 hit processing은 유효한 최적화 후보로 본다.
Frame / Game p95 변화가 작으면 현재 조건에서는 hit processing이 주요 병목은 아니다.
```

### B. HitProcessingDisabled에서 Overlap도 같이 줄어드는 경우

```text
hit processing이 아니라 collision / overlap 진입 자체가 달라진 것이다.
측정 조건 또는 구현 위치를 다시 확인한다.
```

### C. HitProcessingDisabled에서 HitProcessing이 줄지 않는 경우

```text
CVar 또는 게이트 위치가 실제 hit processing route를 막지 못한 것이다.
측정값은 폐기하고 구현 위치를 다시 확인한다.
```

## 후속 분기

HitProcessingDisabled 결과가 유의미하면 다음처럼 세분화한다.

```text
Case 1: weapon collision만 차단
Case 2: overlap은 받지만 hit processing 차단
Case 3: hit processing은 유지하되 feedback presentation 차단
```

HitProcessingDisabled 결과가 유의미하지 않으면 다음 축으로 넘어간다.

```text
Feedback Presentation
Component Tick Audit
Perception Active Budget / Cap
```

## 완료 기준

```text
1. event 기반 계측은 FlushCount 기준으로만 해석한다.
2. WeaponActor / attack montage는 유지한다.
3. 40 / 80 Enemy에서 FullCombat과 HitProcessingDisabled를 쌍으로 측정한다.
4. GC 이벤트 없는 대표값을 채택한다.
5. HitWindow Overlap은 유지되고 HitProcessing / CombatSignal이 줄어드는지 확인한다.
6. Frame / Game / CharacterMovement / BT Tick / FlushCount를 함께 정리한다.
7. Collision / Hit Window가 Runtime LOD 후보인지, 후순위 축인지 결론을 남긴다.
```
