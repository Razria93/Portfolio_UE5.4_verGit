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

## 측정 실행 기준

측정은 가능하면 다음 순서로 진행한다.

```text
1. 측정 map / camera / gameplay state 고정
2. CVar 적용
3. PIE 실행
4. PIE 시작 직후 필요하면 UE console에서 gc 입력
5. 2~3초 대기
6. csvprofile start
7. 약 36초 유지
8. csvprofile stop
9. capture log에 GC 이벤트가 있었는지 확인
10. first 3s / last 3s trim 후 p95 중심으로 해석
```

주의:

```text
gc 명령은 측정 구간 중 GC 발생을 완전히 막는 통제 장치가 아니다.
csvprofile start 전에 GC 정리를 시도해서 capture 중 GC가 끼어들 가능성을 줄이는 절차로만 사용한다.
gc 입력 여부는 대표값 선정 기준으로 사용하지 않는다.
대표값 선정은 csvprofile start 이후 capture log에 CSVEvent "GC"가 기록됐는지 여부를 우선한다.
```

UI / editor 오염 변수는 다음 기준으로 줄인다.

```text
PIE는 F11 fullscreen 기준으로 유지한다.
측정 중 Output Log / Details / Content Browser를 조작하지 않는다.
측정 중 console 입력창을 열어둔 채 시간을 보내지 않는다.
csvprofile start / stop 입력 시 외에는 viewport 또는 빈 영역에 마우스를 둔다.
대량 로그가 발생하는 조건에서는 측정하지 않는다.
```

---

## 측정 요청 템플릿

다음 측정을 요청할 때는 가능한 한 아래 템플릿을 함께 제공한다.

기본 템플릿:

```text
Case:
Map:
Enemy:
State:
CVar:
Capture Duration: about 36s
Analysis Window: first 3s / last 3s trimmed, middle 30s used
Log State: -noailogging
PIE: F11 fullscreen
Camera / PlayerStart:

Pre-capture:
1. CVar 적용
2. PIE 실행
3. 선택: PIE 시작 직후 gc 입력
4. 2~3초 대기
5. csvprofile start

Capture:
1. 약 36초 유지
2. csvprofile stop

확인 항목:
- target CVar 적용 여부
- target actor / component count 변화 여부
- gameplay smoke 유지 여부
- capture 중 CSVEvent "GC" 발생 여부
- 측정 중 editor / output log / console 조작 여부
```

결과 공유 템플릿:

```text
Case:
CSV:
Capture Duration:
Analysis Window:
Log State:
PIE:
Map:
CVar:

Pre-capture log:
Cmd: <CVar>
Cmd: gc                 // 입력했다면
Cmd: csvprofile start

Capture log:
LogCsvProfiler: Display: Capture Starting
LogCsvProfiler: Display: Metadata set : starttimestamp="..."
LogCsvProfiler: Display: CSVEvent "GC" [Frame ...]   // 있으면 포함
Cmd: csvprofile stop
LogCsvProfiler: Display: Metadata set : endtimestamp="..."
LogCsvProfiler: Display: Capture Ended. Writing CSV to file : ...
LogCsvProfiler: Display: Frames : ...

Observed:
- 측정 중 특이사항
- 조작 여부
- gameplay smoke 확인 여부
```

분석 응답 기준:

```text
분석할 때는 먼저 의심 지점을 말한다.
예: CVar 미적용, GC 이벤트, Capture Duration 차이, map / camera 조건 불일치, target count 불일치.
의심 지점이 없으면 "측정 조건상 큰 이상 없음"을 먼저 명시한다.
그 다음 p95 중심으로 결과를 해석한다.
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
GC event 여부
UI / editor 조작 여부
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

### GC / outlier

GC 이벤트는 작은 ms 단위 측정에서 해석을 흔들 수 있다.

기본 기준:

```text
GC 이벤트 없음
-> 대표값 후보로 적합하다.

GC 이벤트 있음
-> p95는 조건부로 사용할 수 있다.
-> p99 / max는 보조 지표로만 본다.
-> 결과가 애매하면 같은 조건으로 재측정한다.

On / Off 중 한쪽에만 GC 이벤트가 있음
-> 직접 비교 신뢰도가 낮아진다.
-> 가능하면 GC 이벤트가 없는 쪽으로 재측정해 대표값을 교체한다.
```

GC 이벤트가 없더라도 결과가 자동으로 정확해지는 것은 아니다.
다만 측정 구간의 큰 오염 변수가 하나 줄어든 것이므로, 같은 조건 재측정에서 방향이 반복되면 결론 신뢰도가 올라간다.

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
GC 이벤트가 한쪽 측정에만 존재함
측정 중 console / output log / editor panel 조작이 있었음
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
