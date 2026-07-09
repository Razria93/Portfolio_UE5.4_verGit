# UE5 Portfolio - AI Performance Bottleneck And LOD Plan Note

## 목적

이 문서는 `refactor/ai-update-interval-policy` 측정 이후 확인된 대량 Enemy 성능 이슈를 바탕으로, 다음 최적화 작업 순서와 사전 검증 방법을 정리한다.

핵심 원칙은 다음과 같다.

```text
최적화는 추측으로 시작하지 않는다.
먼저 병목 축을 분리 측정한다.
유의미한 차이가 확인된 축부터 별도 브랜치로 구현한다.
```

이번 문서는 실제 최적화 구현 문서가 아니라, **작업 전 검증 계획과 후속 브랜치 순서**를 고정하기 위한 문서다.

---

## P34의 위치

`P33: AI Update Interval Profiling 정책 정리`는 이미 측정 / 분석 / 기록을 완료한 PR이다.

그 다음 단계인 `P34: AI Profiling Test Asset 분리`는 최적화 구현 PR이 아니다.

P34의 역할은 P35~P37 최적화에 들어가기 전에 다음 기준을 고정하는 것이다.

```text
공유 gameplay asset과 profiling 전용 asset 분리
측정 Map / Enemy / BT 조건 재현 가능화
극단 비교 테스트를 실행할 수 있는 환경 준비
각 최적화 축의 작업 전 검증 절차 고정
```

P34의 asset 분리 실행 기준은 `N20_AI_Profiling_Test_Asset_Plan_Note.md`를 따른다.

따라서 P34는 다음 작업의 준비 단계다.

```text
P34
-> 측정 환경과 검증 계획 고정

P35
-> runtime LOD 구현 후보 검증 및 적용

P36
-> perception LOD 구현 후보 검증 및 적용

P37
-> update interval LOD 구현 후보 검증 및 적용
```

P34에서 하지 않는 것:

```text
runtime LOD 구현
perception LOD 구현
BT Service interval LOD 구현
대량 AI 최적화 로직 도입
```

P34 완료 후 P35~P37은 “추측 기반 최적화”가 아니라, P34에서 고정한 profiling 환경과 극단 비교 측정 결과를 근거로 시작한다.

---

## 기본 판단 기준

PIE에서 플레이 렉을 검토할 때 AI tick부터 보지 않는다.

실제 프레임 드랍은 보통 가장 비싼 시스템 하나가 아니라, 현재 프레임에서 병목이 된 thread 또는 resource가 만든다.

먼저 다음 축을 구분한다.

```text
Frame ms
GameThread ms
RenderThread ms
GPU ms
Hitch
Memory / Streaming
```

기본 확인 명령:

```text
stat unit
stat unitgraph
stat game
stat gpu
stat memory
```

판단 기준:

```text
GameThread가 높다
-> gameplay / AI / animation / physics / actor tick / collision / blueprint 후보

GPU가 높다
-> render / shadow / material / skeletal mesh / post process / resolution 후보

RenderThread가 높다
-> draw call / mesh component 수 / visibility / skeletal mesh 제출 비용 후보

간헐적 hitch가 있다
-> asset streaming / GC / spawn-destroy / shader / async load 후보

Memory warning 또는 OUT OF MEMORY가 있다
-> actor 수 / skeletal mesh / animation / texture / component / editor duplication 후보
```

---

## 이번 측정에서 확인된 상태

모든 축이 병목이었다고 단정할 수는 없다.

정확히는 여러 병목 후보가 관찰됐고, 일부는 수치로 확인됐으며, 일부는 정황상 가능성이 높은 상태다.

### 수치 또는 현상으로 확인된 축

```text
Frame / GameThread 총량 증가
BT Service / Blackboard update 비용 증가
AI Perception 부하와 인지 지연
160+ Enemy 구간의 OUT OF MEMORY / runtime stress limit
```

BT Tick은 Enemy 수 증가에 따라 커졌다.

```text
40 Enemy  -> 약 0.409ms p95
60 Enemy  -> 약 0.542ms p95
120 Enemy -> 약 0.768ms p95
180 Enemy -> 약 1.017ms p95
200 Enemy -> 약 1.113ms p95
```

하지만 120 Enemy까지는 BT Tick보다 Frame / GameThread 총량이 먼저 한계에 가까워졌다.

따라서 BT interval 최적화는 필요하지만, 대량 Enemy에서 가장 먼저 볼 축은 Character runtime 총량과 perception / movement / render / animation 비용이다.

### 강한 후보지만 아직 분리 측정이 필요한 축

```text
Actor / Component 수 자체
SkeletalMesh / Animation / Render
Movement / Collision / 군집 이동
WeaponActor / Combat collision / HitStop
```

### 이번 측정에서 주 병목으로 보기 어려운 축

```text
Logging
CombatEngage rebuild
Blackboard repeated write
```

`Blackboard dirty write guard`는 120 Enemy 비교에서 소폭 개선을 보였지만, 계산 / perception / render 부하는 그대로이므로 미세 최적화로 분류한다.

---

## 최적화 후보 설명

### 1. Actor / Component 수

Enemy 하나는 단일 Actor가 아니라 여러 runtime object의 묶음이다.

```text
ACharacter
AIController
BehaviorTreeComponent
BlackboardComponent
AIPerceptionComponent
SkeletalMeshComponent
AnimInstance
CharacterMovementComponent
CapsuleComponent
WeaponComponent
WeaponActor
CombatSignalSource / Target
Action / Reaction / Feedback components
```

Enemy 수가 늘어나면 이 묶음이 그대로 증가한다.

최적화 후보:

```text
runtime LOD
dormant actor
proxy actor
weapon actor lazy spawn 또는 disable
far enemy component disable
```

우선은 실제 Enemy를 proxy로 교체하지 않고, Real Enemy를 유지한 채 비용을 낮추는 `runtime LOD`부터 검토한다.

### 2. Skeletal Mesh / Animation

SkeletalMesh는 단순 표시 mesh가 아니라 bone transform, AnimBP, montage, socket, attachment 비용을 가진다.

최적화 후보:

```text
Animation Update Rate Optimization
Visibility Based Anim Tick Option
Mesh LOD
Shadow disable by distance
Far enemy animation disable
```

전투 중 Enemy는 montage notify, socket, weapon, hit collision과 animation이 연결되어 있으므로 무조건 tick을 끄면 안 된다.

거리 / 전투 참여 여부 / 화면 표시 여부를 기준으로 단계적으로 줄여야 한다.

### 3. Render / GPU / Shadow

대량 Enemy는 CPU뿐 아니라 render / GPU 비용도 만든다.

최적화 후보:

```text
shadow disable by distance
material simplification
draw call reduction
visibility culling
mesh LOD
```

Proxy Geometry Tool / HLOD는 주로 static mesh를 저비용 proxy mesh로 만드는 도구다.

Enemy Character처럼 AI / animation / combat을 가진 동적 객체를 그대로 대체하는 도구는 아니다.

원거리 표시용 깡통 proxy로 사용할 수는 있지만, 가까워질 때 Real Enemy로 복원해야 하므로 별도 구조 작업이 필요하다.

### 4. Movement / Collision / Physics

CharacterMovement와 collision은 대량 Enemy에서 비용과 불안정성을 동시에 만든다.

최적화 후보:

```text
movement update interval
far enemy movement mode 축소
enemy끼리 collision channel 정리
combat collision distance gate
MoveTo target 분산
crowd density control
```

이번 측정에서는 dense setup에서 길막, Enemy끼리 피격, hit stop 변수가 있었다.

분산 배치 / collision 축소 / friendly hit 차단 후 결과가 개선됐으므로, movement / collision / 군집은 실제 영향이 있었던 축으로 본다.

### 5. AI Perception

AI Perception은 단순 변수 체크가 아니라 sight trace, listener 관리, target 감지 갱신 비용을 가진다.

최적화 후보:

```text
perception LOD
active listener cap
distance based sight activation
perception interval scaling
engage candidate만 정밀 감지
```

160개 중 1개만 perception 활성화했을 때 즉시 인식된 실험은, perception 활성 대상 수가 많을 때 감지 지연이 발생할 수 있음을 보여준다.

### 6. BehaviorTree / Blackboard Update

이번 브랜치의 직접 주제였던 축이다.

최적화 후보:

```text
service interval LOD
dirty write guard
dirty flag
event-driven update
task polling 제거
CombatEngage rebuild 분산
```

`dirty write guard`는 반복 Blackboard write를 줄이지만 계산 자체를 줄이지 않는다.

더 큰 개선은 dirty flag, event-driven update, LOD 기반 interval 조정에서 나온다.

### 7. CombatEngage

현재 측정에서는 병목이 아니었다.

```text
CombatEngage_Rebuild p95는 120 Enemy에서도 약 0.006ms 수준
```

따라서 지금 당장 우선 최적화 대상은 아니다.

다만 engage slot, group, formation 정책이 복잡해지면 다음 후보가 될 수 있다.

```text
rebuild interval scaling
only dirty target rebuild
assignment cache
target group partitioning
```

### 8. Spawn / Destroy / GC

꾸준히 낮은 FPS가 아니라 순간적으로 끊기는 경우 확인할 축이다.

최적화 후보:

```text
object pooling
pre-spawn
lazy spawn 제거
destroy batching
GC pressure reduction
```

이번 P33 측정의 주 대상은 아니지만, 후속 runtime LOD / proxy / weapon actor 정책을 설계할 때 함께 고려해야 한다.

### 9. Debug / Logging

이번 측정에서는 주 병목으로 보기 어렵다.

로그 제거 전후 비교에서 큰 차이는 없었다.

다만 hot path log는 측정 오염과 Shipping 품질 리스크가 있으므로 별도 debug log policy 브랜치에서 정리한다.

---

## 작업 전 극단 비교 측정 계획

각 최적화 축은 구현 전에 극단 조건으로 먼저 측정한다.

의도는 실제 최적화 구현이 유의미할 가능성이 있는지 확인하는 것이다.

측정 축은 다음 우선순위로 나눈다.

```text
1차 측정
-> 코드 / 에디터 설정으로 바로 끌 수 있는 축
-> Mesh visibility, WeaponActor, AnimInstance, Movement, Collision, Tick, Shadow, Perception, BT Tick

2차 측정
-> 실제 Runtime LOD로 적용 가능한 주기 / 거리 기반 조정 축
-> component tick interval, BT service interval, perception activation range / cap

3차 측정
-> 에셋 제작 또는 대체가 필요한 축
-> material 단순화, low-poly mesh, proxy representation
```

1차 측정은 병목 후보를 빠르게 걸러내기 위한 극단 비교다.
2차 측정은 실제 적용 가능한 Runtime LOD 정책으로 이어질 수 있는지 확인한다.
3차 측정은 render 비용이 큰 축으로 확인될 때 후속 PR에서 다룬다.

### 1. Actor / Component 수

검증 질문:

```text
Enemy 수 자체가 GameThread / memory / stress limit에 얼마나 영향을 주는가?
```

극단 비교:

```text
120 Enemy normal
120 Enemy with WeaponActor disabled
120 Enemy with combat collision disabled
120 Enemy with non-essential component tick disabled
```

기대 관찰:

```text
Frame / GameThread p95 감소
OUT OF MEMORY 또는 crash 빈도 감소
```

유효하면:

```text
refactor/ai-runtime-lod-policy에서 component / weapon / collision gate 설계
```

### 2. Skeletal Mesh / Animation

검증 질문:

```text
AnimInstance / SkeletalMesh update가 대량 Enemy frame cost에 얼마나 영향을 주는가?
```

극단 비교:

```text
120 Enemy normal
120 Enemy AnimBP 미사용 또는 AnimInstance update 최소화
120 Enemy SkeletalMesh visibility off
120 Enemy mesh shadow off
120 Enemy visibility based anim tick option 변경
```

기대 관찰:

```text
Frame / GameThread / RenderThread / GPU p95 변화
stat anim / stat skeletalmesh 변화
```

유효하면:

```text
refactor/ai-runtime-lod-policy 또는 refactor/ai-animation-lod-policy로 분리
```

### 3. Movement / Collision

검증 질문:

```text
CharacterMovement / collision / 군집 이동이 frame cost와 불안정성에 얼마나 영향을 주는가?
```

극단 비교:

```text
120 Enemy normal
120 Enemy movement stop
120 Enemy capsule collision reduced
120 Enemy enemy-to-enemy collision ignore
120 Enemy combat collision off
120 Enemy MoveTo disabled
```

기대 관찰:

```text
Frame / GameThread p95 감소
collision / physics 관련 stat 감소
길막 / hit stop / enemy friendly hit 변수 제거
```

유효하면:

```text
runtime LOD에 movement / collision 단계 연결
```

### 4. AI Perception

검증 질문:

```text
동시에 활성화된 perception listener 수가 감지 지연과 AI cost에 얼마나 영향을 주는가?
```

이미 확인한 극단 비교:

```text
160 Enemy all perception active
160 Enemy 중 1개만 perception active
```

추가 비교:

```text
120 Enemy perception active cap 10
120 Enemy perception active cap 20
120 Enemy perception active cap 40
120 Enemy distance-based activation
```

기대 관찰:

```text
AIPerception p95 감소
TargetActor Blackboard set latency 감소
인지 지연 감소
```

유효하면:

```text
refactor/ai-perception-lod-policy에서 active listener cap / distance based activation 설계
```

### 5. BehaviorTree / Blackboard Update

검증 질문:

```text
BT Service interval과 Blackboard update가 전체 frame cost에 얼마나 영향을 주는가?
```

이미 확인한 비교:

```text
120 Enemy baseline
120 Enemy dirty write guard
```

추가 비교:

```text
120 Enemy service interval 0.1s
120 Enemy service interval 0.2s
120 Enemy service interval 0.5s
120 Enemy UpdateAIContext disabled
120 Enemy UpdateEngageContext disabled
```

기대 관찰:

```text
BT Tick p95 감소
Frame / GameThread p95 변화
gameplay 반응성 저하 여부
```

유효하면:

```text
refactor/ai-update-lod-policy에서 LOD별 interval 설계
```

### 6. Render / GPU

검증 질문:

```text
대량 Enemy의 렌더링 / shadow / material 비용이 frame cost에 얼마나 영향을 주는가?
```

극단 비교:

```text
120 Enemy normal
120 Enemy all shadow off
120 Enemy mesh hidden
120 Enemy material simplified
120 Enemy lower mesh LOD forced
```

기대 관찰:

```text
GPU ms 감소
RenderThread ms 감소
Frame p95 감소
```

유효하면:

```text
runtime LOD에 mesh / shadow / render policy 연결
```

---

## 추천 작업 순서

P33 이후 작업 순서는 다음으로 고정한다.

```text
0. chore/ai-profiling-test-assets
   -> 공유 gameplay asset을 오염시키지 않는 profiling 전용 Map / Enemy / BT 구성
   -> P35~P37에 들어가기 전 측정 환경과 극단 비교 테스트 기준 고정

1. refactor/ai-runtime-lod-policy
   -> Character / mesh / weapon / movement / collision runtime cost를 줄이는 축

2. refactor/ai-perception-lod-policy
   -> AI Perception 활성 대상 수와 감지 주기를 거리 / 중요도 기준으로 제한하는 축

3. refactor/ai-update-lod-policy
   -> BT Service / Blackboard update / CombatEngage rebuild 주기를 LOD와 연결하는 축

4. refactor/type-header-helper-boundary
   -> 최적화 과정에서 드러난 Type header include boundary와 helper 분리 기준 정리

5. refactor/tuning-constants-cleanup
   -> 위 작업에서 드러난 threshold / interval / radius 값을 constants 또는 config 후보로 정리
```

P34는 P35~P37의 선행 조건이다.

각 최적화 브랜치의 첫 커밋은 구현이 아니라 측정 환경 또는 극단 비교 기록이어야 한다.

---

## 완료 조건

```text
병목 후보별 극단 비교 측정 계획이 문서화되어 있다.
profiling 전용 asset 분리 필요성이 명시되어 있다.
P34가 P35~P37 선행 준비 작업임이 명시되어 있다.
runtime LOD / perception LOD / update LOD 작업 순서가 정리되어 있다.
각 최적화 작업은 측정 결과를 근거로 시작한다.
공유 gameplay asset에 profiling 전용 설정을 유지하지 않는다.
```
