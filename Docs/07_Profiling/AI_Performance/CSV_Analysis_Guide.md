# AI Performance CSV Analysis Guide

## 목적

이 문서는 AI performance / Runtime LOD CSV를 해석할 때 반복해서 사용하는 기준을 정리한다.

측정 결과를 볼 때는 먼저 이 문서를 기준으로 다음 순서대로 확인한다.

```text
1. 측정 조건이 의도대로 적용됐는지 확인한다.
2. 같은 조건끼리 비교 가능한지 확인한다.
3. Frame / Game / GPU / Render p95를 본다.
4. 변경 축과 직접 연결된 세부 지표를 본다.
5. ActorCount와 Tick count가 어긋날 수 있는 항목은 보조 지표로 해석한다.
6. 평균값은 경향 확인, p95는 비교 기준, max는 outlier 확인용으로 사용한다.
```

---

## 분석 구간 기준

CSV의 `FrameTime` 단위는 ms다.

현재 Runtime LOD 측정의 기본 분석 기준:

```text
Capture Duration: about 36s
Analysis Window: first 3s / last 3s trimmed, middle 30s used
```

계산 기준:

```text
total_ms = sum(FrameTime)
start_trim = 3000ms
end_trim = total_ms - 3000ms
```

주의:

```text
Analysis Window는 UE 자동 종료 기능이 아니다.
CSV는 전체 capture duration을 기록하고, 비교값 계산 시 앞뒤 3초를 제외한다.
```

---

## 비교 전 확인 순서

### 1. 측정 조건 확인

먼저 CVar / map / actor setup이 의도대로 반영됐는지 확인한다.

예시:

```text
EnemyMeshMode 0 / 1 / 2가 의도한 상태로 적용됐는지
DisableEnemyWeaponActor 0 / 1이 의도한 상태로 적용됐는지
AIController / BT / Perception을 제거한 조건인지
WeaponActor를 유지하는 조건인지 제거하는 조건인지
fixed camera 조건인지 gameplay stress 조건인지
```

축별 확인 지표:

```text
WeaponActor 제거 측정
-> ActorCount/CWeaponActor
-> ActorCount/TotalActorCount
-> Ticks/SkeletalMeshComponent
-> RHI/DrawCalls

Mesh visibility 측정
-> RHI/DrawCalls
-> RHI/PrimitivesDrawn
-> GPUTime
-> Ticks/SkeletalMeshComponent

Animation / pose update 측정
-> Exclusive/GameThread/Animation
-> GameThreadTime
-> Ticks/SkeletalMeshComponent

AI / BT 측정
-> Ticks/BehaviorTreeComponent
-> Exclusive/GameThread/AIPerception
-> GameThread/PortfolioAI_BT_UpdateAIContext
```

### 2. 비교 가능 조건 확인

다음 항목이 같아야 직접 비교한다.

```text
Map
Enemy count
Camera / PlayerStart
PIE fullscreen 여부
Log State
Capture Duration
Analysis Window
Gameplay state
```

조건이 다르면 같은 표 안에 넣더라도 "정규 비교"가 아니라 "참고 측정"으로 분리한다.

---

## 주요 지표 해석 기준

### Frame / Thread

```text
FrameTime p95
-> 플레이 체감 기준의 1차 지표

GameThreadTime p95
-> gameplay / animation / AI / movement / actor tick 쪽 병목 확인

GPUTime p95
-> render / material / shadow / primitive 비용 확인

RenderThreadTime p95
-> render command / scene update 쪽 비용 확인
```

해석 우선순위:

```text
FrameTime p95가 내려갔는지 본다.
GameThreadTime / GPUTime 중 어느 쪽이 같이 내려갔는지 본다.
변경 축의 세부 지표도 같이 내려갔는지 확인한다.
```

### p95 / avg / max

```text
p95
-> 기본 비교 기준
-> 순간 outlier에 덜 흔들리면서 체감 비용을 반영한다.

avg
-> 전체 경향 확인용
-> p95와 방향이 같으면 결과 신뢰도가 올라간다.

max
-> outlier 확인용
-> 단독으로 결론을 내리지 않는다.
```

---

## ActorCount / Tick Count 해석 기준

UE CSV의 `ActorCount/*`는 PIE / editor world / duplicated world 상태와 섞여 보일 수 있다.

따라서 active runtime 대상 수를 판단할 때는 다음 순서로 본다.

```text
1. Ticks/CEnemy
2. Ticks/BehaviorTreeComponent
3. Ticks/CAIController
4. ActorCount/TotalActorCount
5. ActorCount/specific actor
```

예시:

```text
ActorCount/CEnemy = 81
Ticks/CEnemy = 40
Ticks/BehaviorTreeComponent = 40
Ticks/CAIController = 40
```

이 경우 실제 runtime 측정 대상은 40 Enemy로 해석한다.

`ActorCount/CEnemy`는 절대값보다 상대 변화와 참고 지표로 본다.

단, 특정 actor가 생성됐는지 제거됐는지 확인할 때는 `ActorCount/specific actor`가 유효하다.

예시:

```text
ActorCount/CWeaponActor: 41 -> 0
ActorCount/TotalActorCount: 339 -> 299
```

이 경우 WeaponActor 제거 CVar가 적용됐다고 판단할 수 있다.

---

## Missing Column 해석 기준

CSV에 특정 컬럼이 없을 수 있다.

해석 기준:

```text
ActorCount/CWeaponActor가 없거나 값이 0
-> 해당 actor가 생성되지 않았을 가능성이 높다.

Ticks/BehaviorTreeComponent가 없음
-> BT가 실행되지 않는 조건일 가능성이 높다.

PortfolioAI_* custom stat이 없음
-> 해당 scope가 실행되지 않았거나 측정 조건에서 제외된 것이다.
```

단, missing column은 항상 0이라고 단정하지 않고 측정 조건과 함께 확인한다.

---

## 이상값 판단 기준

다음 경우는 결과를 그대로 결론으로 쓰지 않고 재확인한다.

```text
CVar를 바꿨는데 target actor / tick count가 변하지 않음
FrameTime은 크게 변했는데 변경 축 세부 지표가 전혀 변하지 않음
GameThread / GPU 방향이 예상과 반대로 크게 튐
같은 조건 재측정 간 p95 차이가 너무 큼
Capture Duration이 크게 다름
Analysis Window가 다름
PIE fullscreen / camera / map 조건이 다름
outlier max가 p95 해석을 왜곡할 정도로 큼
```

---

## 해석 템플릿

CSV 결과를 정리할 때는 다음 순서를 사용한다.

```text
1. 조건 확인
2. target axis 적용 여부
3. Frame / Game / GPU / Render p95 변화
4. target axis 세부 지표 변화
5. 의외의 값 또는 주의점
6. 결론
7. 다음 측정 조건
```

예시:

```text
WeaponActor Off는 ActorCount/CWeaponActor를 41에서 0으로 낮췄다.
TotalActorCount와 SkeletalMeshComponent tick도 함께 감소했으므로 CVar는 정상 적용됐다.
Frame / GameThread p95가 낮아졌고 DrawCalls도 감소했다.
따라서 WeaponActor는 Object Management / Representation 양쪽에 비용이 있는 축으로 본다.
```
