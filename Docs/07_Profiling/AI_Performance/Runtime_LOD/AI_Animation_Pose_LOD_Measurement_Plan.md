# AI Animation / Pose / Locomotion LOD 측정 계획

## 목적

이 문서는 Team Attitude / Affiliation 보정으로 AI perception 후보 누수를 해결한 뒤, P35에서 다음으로 측정할 축을 정리한다.

다음 측정축:

```text
Animation / Pose / Locomotion
```

이번 작업의 목표는 애니메이션 구조를 바로 재설계하는 것이 아니다.
현재 40 / 80 Enemy profiling 조건에서 animation update, pose update, locomotion visual detail이 의미 있는 runtime cost 축인지 측정하는 것이다.

## 현재 확인된 내용

현재까지 확정된 병목 원인:

```text
AI perception candidate leak
```

Team Attitude 보정으로 perception 후보는 `모든 Enemy + Player`에서 `Player only`로 줄었다.

보정 이후 남은 주요 비용 후보:

```text
Animation / Pose / Locomotion
Movement / Nav
BT Update Interval
Perception Active Budget
Collision / Overlap
Actor / Component / Proxy
```

다음 측정을 Animation / Pose / Locomotion부터 시작하는 이유:

- 40 / 80 Enemy 측정에서 `AnimationParallelEvaluation` 비용이 계속 남아 있다.
- mesh visibility만으로 남은 GameThread / Frame 비용을 모두 설명하지 못했다.
- `EnemyMeshMode 2`는 통제된 render coverage 조건에서 pose update skip이 animation 비용을 크게 줄일 수 있음을 보여줬다.
- 현재 combat-capable Enemy는 montage notify와 weapon socket timing을 gameplay dependency로 사용하므로 pose update를 바로 skip할 수 없다.

## 범위

포함:

- Skeletal mesh animation update 비용
- Pose update 비용
- Locomotion visual detail 비용
- Animation / pose update rate 조절 후보
- Montage notify / weapon socket follow와 관련된 combat safety 기준

제외:

- Movement / Nav decision 비용
- BT update interval 조정
- Perception active budget
- Collision / overlap
- Actor proxy / pooling
- Combat action timeline 재설계

## 측정축

측정은 다음 상태를 분리해서 본다.

```text
Mode A: Baseline
-> mesh visible
-> animation update enabled
-> pose update enabled
-> locomotion animation active
-> combat-capable path unchanged

Mode B: Reduced Animation Update
-> mesh visible
-> socket / notify timing을 유지할 수 있는 수준의 animation / pose 유지
-> update frequency 또는 detail 축소
-> 첫 gameplay-safe 후보

Mode C: Pose Skip Isolation
-> mesh hidden 또는 통제된 visibility 조건
-> pose update skip 허용
-> animation 비용 절감 상한선을 보기 위한 측정 전용 모드
-> combat-capable Enemy에는 gameplay-safe하지 않음
```

Mode C는 측정 기준점이지, 1차 구현 목표가 아니다.

## 측정 맵 구성

Animation / Pose / Locomotion 측정은 별도 맵에서 진행한다.

권장 맵:

```text
MAP_AIPerf_AnimationLOD_40Enemy
MAP_AIPerf_AnimationLOD_80Enemy
```

맵 구성 기준:

```text
Player 또는 target 역할 actor를 중앙에 배치한다.
Enemy는 fixed camera 안에 모두 보이도록 배치한다.
Enemy가 Player를 인식하고 Alert / Engage로 진입할 수 있게 한다.
Enemy가 화면 밖으로 빠져나가지 않도록 arena / placement / engage range를 조정한다.
Enemy끼리 피격하지 않도록 한다.
Enemy끼리 길막이 측정을 지배하지 않도록 한다.
AIController / BT / BB / Perception / Movement / WeaponActor는 유지한다.
EnemyMeshMode는 0으로 유지한다.
EnemyAnimationMode만 0 / 1로 비교한다.
```

고정 카메라는 사용한다.
다만 RenderCoverage 측정과 목적이 다르다.

```text
RenderCoverage fixed camera
-> 화면 안 mesh 수 / draw call / primitive 통제가 목적

AnimationLOD fixed camera
-> gameplay stress 상태, movement 흐름, locomotion 품질 관찰 통제가 목적
```

이번 측정에서는 모든 Enemy를 화면 안에 유지한다.
Enemy가 화면 밖으로 나가면 이동 끊김, 길막, Engage 이탈, animation refresh 지연을 구분하기 어렵다.
따라서 fixed camera 안에서 Alert / Engage / movement / locomotion이 관찰되는 조건을 기본값으로 둔다.

## HiddenAllowPoseSkip을 바로 쓰지 않는 이유

`EnemyMeshMode 2`는 animation / pose 비용 측정에는 유효하다.
하지만 combat Runtime LOD 모드로 바로 쓰기에는 안전하지 않다.

현재 Phase 1 combat timing:

```text
Montage playback
-> AnimNotify
-> hit window / cue / command timing
-> weapon socket follow
```

combat-capable Enemy에서 pose update를 skip하면 montage notify timing과 socket 기반 weapon 동작이 불안정해질 수 있다.

따라서 기준은 다음과 같다.

```text
Combat-capable Enemy
-> pose update skip 금지

NonCombat / Dormant Enemy
-> pose update skip 측정 및 후보 검토 가능
```

## 제안 측정 케이스

P35와 같은 scale 정책을 사용한다.

```text
40 Enemy
-> sanity / 1차 비교

80 Enemy
-> 주 stress 비교

120 Enemy
-> 40 / 80만으로 판단이 부족할 때만 사용하는 optional extension
```

권장 1차 측정쌍:

```text
A00: 40 Enemy / AnimationBaseline
A01: 40 Enemy / AnimationReduced / Interval 0.1
A02: 80 Enemy / AnimationBaseline
A03: 80 Enemy / AnimationReduced / Interval 0.1
```

선택 측정쌍:

```text
A04: 40 Enemy / AnimationReduced / Interval 0.2
A05: 80 Enemy / AnimationReduced / Interval 0.2
A06: 40 Enemy / PoseSkipIsolation
A07: 80 Enemy / PoseSkipIsolation
```

`0.1`은 AIContext / EngageContext / CombatEngage rebuild 주기와 맞춘 1차 reduced 기준이다.
`0.2`는 AIIntentState 주기와 맞춘 aggressive reduced 후보이며, 0.1 측정으로 판단이 부족할 때만 추가한다.

## 측정 지표

주요 지표:

```text
FrameTime p95
GameThreadTime p95
Animation p95
AnimationParallelEvaluation TotalTaskTime p95
Ticks/SkeletalMeshComponent
Ticks/CharacterMovementComponent
PortfolioAI_AnimRefresh_Attempt
PortfolioAI_AnimRefresh_Executed
PortfolioAI_AnimRefresh_Skipped
```

보조 지표:

```text
GPUTime p95
DrawCalls p95
PrimitivesDrawn p95
BehaviorTreeTick p95
AIPerception p95
```

해석 기준:

```text
AnimationParallelEvaluation은 줄었지만 Frame / GameThread가 거의 움직이지 않으면,
animation은 유효한 local cost 축이지만 현재 최상위 frame 병목은 아니라고 본다.

AnimationParallelEvaluation과 GameThread가 함께 줄면,
animation update는 강한 Runtime LOD 후보로 본다.

animation 비용은 줄었지만 combat smoke test가 깨지면,
해당 모드는 측정 전용으로 분류하고 combat-capable Enemy에는 사용하지 않는다.
```

## 구현 후보

가장 침습도가 낮은 제어부터 시작한다.

```text
1. animation profiling CVar 추가
2. profiling 조건의 AIPerf Enemy 또는 ACEnemy에만 적용
3. combat-safe mode와 pose-skip isolation mode 분리
4. 측정 문서에 적용 mode 기록
```

CVar 후보:

```text
Portfolio.AI.RuntimeLOD.EnemyAnimationMode
Portfolio.AI.RuntimeLOD.EnemyAnimationReducedRefreshInterval
Portfolio.AI.RuntimeLOD.AnimationRefreshAudit
```

기본 reduced interval:

```text
0.1
```

값 후보:

```text
0: Default
1: ReducedParameterRefresh
```

`PoseSkipIsolation`은 후속 이관 대상이다.
첫 구현에서는 parameter refresh 주기 축소만 측정한다.

정규 측정에서는 `AnimationRefreshAudit`을 켠다.
이 카운터는 refresh gate의 attempt / executed / skipped 횟수를 CSV에 누적해, ReducedParameterRefresh가 실제로 호출 빈도를 줄였는지 확인하기 위한 측정 전용 값이다.

```text
Portfolio.AI.RuntimeLOD.AnimationRefreshAudit 1
```

카운터가 없는 기존 40 Enemy baseline / reduced 측정은 preliminary 측정으로만 보고 정규 결과표에는 사용하지 않는다.

## 측정 결과

### 40 Enemy / Interval 0.1

| Case | Mode | 시간     | Frame p95 |  Game p95 | Animation p95 | AnimParallel p95 | Attempt/s | Executed/s | Skipped/s | 판정    | 원본 CSV                    |
| ---- | ---: | ------ | --------: | --------: | ------------: | ---------------: | --------: | ---------: | --------: | ----- | ------------------------- |
| A00  |    0 | 37.28s | 12.7319ms | 12.7569ms |      2.0340ms |         3.7941ms |   3,459.1 |    3,459.1 |         - | 기준    | Profile(20260706_172159).csv |
| A01  |    1 | 37.10s | 12.7914ms | 12.7950ms |      2.0606ms |         3.7730ms |   3,446.9 |      378.8 |   3,068.1 | 제한 효과 | Profile(20260706_172439).csv |

### 80 Enemy / Interval 0.1

| Case | Mode | 시간     | Frame p95 |  Game p95 | Animation p95 | AnimParallel p95 | Attempt/s | Executed/s | Skipped/s | 판정    | 원본 CSV                    |
| ---- | ---: | ------ | --------: | --------: | ------------: | ---------------: | --------: | ---------: | --------: | ----- | ------------------------- |
| A02  |    0 | 37.38s | 20.4072ms | 20.3507ms |      3.4579ms |         6.3936ms |   4,217.1 |    4,217.1 |         - | 기준    | Profile(20260706_173457).csv |
| A03  |    1 | 37.42s | 20.3399ms | 20.3328ms |      3.5562ms |         6.5276ms |   4,234.7 |      711.3 |   3,523.4 | 제한 효과 | Profile(20260706_174039).csv |

측정 조건:

```text
Capture Duration: 약 36초
Analysis Window: first 3s / last 3s trimmed, middle 30s used
Log State: -noailogging
PIE: F11 fullscreen
Map: MAP_AIPerf_AnimationLOD_40Enemy
Camera: fixed camera
DisableEnemyPerception 0
PerceptionCandidateAudit 0
BlackboardEngageLatencyAudit 0
DisableEnemyWeaponActor 0
EnemyMeshMode 0
AnimationRefreshAudit 1
EnemyAnimationReducedRefreshInterval 0.1
```

해석:

```text
EnemyAnimationMode 1에서는 refresh gate가 정상 동작했다.
Attempt는 baseline과 동일하게 40/frame 수준을 유지했고, Executed는 약 3,459/s에서 약 379/s로 감소했다.
Skipped는 약 3,068/s로 증가했다.

다만 Frame p95, GameThread p95, Animation p95는 baseline과 거의 차이가 없다.
따라서 40 Enemy 조건에서 parameter refresh 주기 축소는 동작 검증은 됐지만, frame budget을 회복하는 주요 병목 해소책으로 보기는 어렵다.

80 Enemy에서도 같은 패턴이 반복됐다.
EnemyAnimationMode 1은 Executed를 약 4,217/s에서 약 711/s로 줄였지만, Frame / GameThread p95 개선은 오차 수준이고 Animation / AnimParallel p95는 소폭 증가했다.

따라서 현재 구현 형태의 parameter refresh gate는 호출 빈도 제어 기능으로는 유효하지만, 40 / 80 Enemy 조건의 성능 병목을 해결하는 Runtime LOD 축으로는 우선순위가 낮다.
애니메이션 비용 축은 parameter refresh보다 pose update, skeletal mesh tick option, locomotion detail, montage / notify dependency 분리 쪽을 후속 후보로 둔다.
```

## EnemyMeshMode / EnemyAnimationMode 분리

현재 `EnemyMeshMode 2`는 mesh visibility와 pose update skip을 함께 제어한다.
이 구조는 render 비용과 animation / pose 비용을 분리해서 해석하기 어렵다.

따라서 다음 구현에서는 책임을 분리한다.

```text
EnemyMeshMode
0: Visible
1: Hidden

EnemyAnimationMode
0: Default
1: ReducedParameterRefresh
```

기존 `EnemyMeshMode 2`에 해당하던 조건은 다음 조합으로 이관한다.

```text
EnemyMeshMode 1
EnemyAnimationMode 2
```

`EnemyMeshMode`는 mesh 표시 여부만 담당한다.
`EnemyAnimationMode`는 animation parameter refresh, pose update, visibility based anim tick option 같은 animation / pose 정책을 담당한다.

`PoseSkipIsolation`은 측정용 극단 조건이다.
Combat-capable Enemy에서는 montage notify / socket timing 때문에 pose update skip을 허용하지 않는다.
따라서 첫 구현에서는 `EnemyAnimationMode 1`만 실동작으로 추가하고, `EnemyAnimationMode 2`는 후속 pose skip isolation 작업에서 다룬다.

정확한 구현 위치는 현재 `USkeletalMeshComponent` / AnimInstance 설정을 확인한 뒤 결정한다.

## 작업 가이드

코드 수정 전에 확인할 내용:

```text
1. 현재 animation 관련 코드와 asset 흐름을 스캔한다.
2. AIPerf Enemy에서 CAnimInstance가 어디서 지정되는지 확인한다.
3. visibility 기반 animation tick option이 이미 사용 중인지 확인한다.
4. 현재 EnemyMeshMode CVar와 animation mode가 겹치는지 확인한다.
5. 새 제어가 ACEnemy, mesh component setup, helper 중 어디에 들어가는지 결정한다.
```

구현과 측정 asset 변경은 커밋을 분리한다.

권장 커밋 분리:

```text
feat(ai): add animation runtime lod profiling control
docs(ai): record animation lod measurement setup
chore(ai): prepare animation lod profiling map
docs(ai): record animation lod profiling results
```

## 종료 조건

이 측정축은 다음 조건을 만족하면 닫을 수 있다.

```text
40 / 80 Enemy baseline과 reduced animation 측정이 기록된다.
Animation / pose 비용 차이가 확인되거나 명시적으로 배제된다.
Gameplay-safe mode와 measurement-only mode가 분리된다.
Combat-capable pose update 규칙이 문서화된다.
다음 병목 축을 6개 측정축 목록에서 선택한다.
```
