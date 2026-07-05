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
A01: 40 Enemy / AnimationReduced
A02: 80 Enemy / AnimationBaseline
A03: 80 Enemy / AnimationReduced
```

선택 측정쌍:

```text
A04: 40 Enemy / PoseSkipIsolation
A05: 80 Enemy / PoseSkipIsolation
```

## 측정 지표

주요 지표:

```text
FrameTime p95
GameThreadTime p95
Animation p95
AnimationParallelEvaluation TotalTaskTime p95
Ticks/SkeletalMeshComponent
Ticks/CharacterMovementComponent
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
```

값 후보:

```text
0: Default
1: ReducedUpdate
2: PoseSkipIsolation
```

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
