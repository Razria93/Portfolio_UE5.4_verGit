# AI Perception Runtime LOD Measurements

## 목적

이 문서는 `P35: AI Runtime LOD 정책 정리`에서 수행할 AI Perception 비용 분리 측정 계획과 결과를 누적 기록한다.

CSV 해석 기준:

```text
Docs/07_Profiling/AI_Performance/CSV_Analysis_Guide.md
```

---

## 현재 구조 요약

Perception 흐름:

```text
ACAIController
-> UAIPerceptionComponent / UAISenseConfig_Sight 생성
-> OnTargetPerceptionUpdated에서 TargetDataMap 갱신
-> UCBTService_UpdateAIContext::BuildPerceptionContext
-> ACAIController::BuildPerceptionContext
-> TargetDataMap 정리 / top target 선택
-> Blackboard TargetActor / bHasLOS / LastSeenTime / LastKnownLocation 갱신
-> Alert / Engage context 계산
```

주요 파일:

```text
Source/Portfolio/Controller/CAIController.h
Source/Portfolio/Controller/CAIController.cpp
Source/Portfolio/AI/BehaviorTree/Service/CBTService_UpdateAIContext.cpp
```

Perception 후보 누수 / 인지 지연 Audit 계획:

```text
Docs/07_Profiling/AI_Performance/Runtime_LOD/AI_Perception_Candidate_Audit_Plan.md
```

현재 측정에서 확인할 수 있는 비용 지표:

```text
Exclusive/GameThread/AIPerception
GameThread/PortfolioAI_BT_UpdateAIContext
Ticks/BehaviorTreeComponent
Ticks/CAIController
FrameTime
GameThreadTime
```

---

## 측정상 주의점

Perception은 비용축이면서 동시에 gameplay input이다.

따라서 단순히 Perception을 끄면 다음 변화가 함께 발생한다.

```text
TargetDataMap이 갱신되지 않음
Blackboard TargetActor가 비거나 갱신되지 않음
Alert / Engage context가 달라짐
Enemy가 Engage에 진입하지 않거나 전투 상태가 달라짐
```

이 경우 Frame / GameThread 차이는 Perception 비용만이 아니라 gameplay state 변화까지 포함한다.

따라서 P35에서는 Perception 측정을 두 단계로 나눈다.

```text
0. Perception Candidate Audit
-> Perception 후보 누수와 first valid target latency를 확인한다.
-> Perception Gate 측정 전에 TargetDataMap 오염 여부를 판단한다.

1. Perception Gate Impact
-> Perception을 끈 상태와 켠 상태의 engine / controller 비용 차이를 확인한다.
-> gameplay state가 달라지므로 Perception이 여는 downstream 부하 측정으로 기록한다.

2. Perception Active Budget 후보
-> 대량 Enemy에서 모든 Enemy가 perception을 켜는 것이 필요한지 확인한다.
-> 실제 active cap / distance budget 구현은 후속 PR에서 다룬다.
```

---

## 측정 스위치

구현된 CVar:

```text
Portfolio.AI.RuntimeLOD.DisableEnemyPerception
0: Enemy Perception enabled
1: Enemy Perception disabled for runtime LOD measurement

Portfolio.AI.RuntimeLOD.PerceptionCandidateAudit
0: Candidate audit disabled
1: Enemy Perception candidate audit enabled
```

적용 위치:

```text
ACAIController::InitializeControllerRuntime
-> Perception profiling 상태를 기본값으로 복구
-> BindPerceptionEvents 전에 측정 스위치 확인

ACAIController::ResetPerceptionStateForProfiling
-> bPerceptionDisabledForProfiling 초기화
-> Sight sense 활성화

ACAIController::DisableEnemyPerceptionForProfiling
-> TargetDataMap 정리
-> Sight sense 비활성화

ACAIController::BuildPerceptionContext
-> disabled 상태면 NoData 반환
```

적용 방식:

```text
Sight sense 비활성화
Perception delegate binding 생략
TargetDataMap 정리
BuildPerceptionContext에서 NoData 반환
```

P35의 1차 측정은 `Sight sense 비활성화 + delegate binding 생략` 방식으로 수행한다.
목적은 gameplay-safe 적용이 아니라 Perception 축의 최대 비용 차이를 확인하는 것이다.

---

## 측정 케이스

### Case P00 - 40 Enemy / Perception On

```text
Case: 40 Enemy / Perception Isolation / DisableEnemyPerception 0
Map: MAP_AIPerf_WeaponActor_40Enemy 또는 별도 Perception map
Enemy: 40 AIPerf Enemy
State: Gameplay Stress
CVar: Portfolio.AI.RuntimeLOD.DisableEnemyPerception 0
Capture Duration: about 36s
Analysis Window: first 3s / last 3s trimmed, middle 30s used
Log State: -noailogging
PIE: F11 fullscreen
```

### Case P01 - 40 Enemy / Perception Off

```text
Case: 40 Enemy / Perception Isolation / DisableEnemyPerception 1
Map: same as P00
Enemy: 40 AIPerf Enemy
State: Gameplay Stress
CVar: Portfolio.AI.RuntimeLOD.DisableEnemyPerception 1
Capture Duration: about 36s
Analysis Window: first 3s / last 3s trimmed, middle 30s used
Log State: -noailogging
PIE: F11 fullscreen
```

### Case P02 / P03 - 80 Enemy

```text
P02: 80 Enemy / DisableEnemyPerception 0
P03: 80 Enemy / DisableEnemyPerception 1
```

80 Enemy는 40 Enemy에서 비용 차이가 확인된 뒤 측정한다.

---

## 측정 요청 템플릿

```text
Case: 40 Enemy / Perception Isolation / DisableEnemyPerception 0
Map:
Enemy:
State:
CVar: Portfolio.AI.RuntimeLOD.DisableEnemyPerception 0
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
- capture 중 CSVEvent "GC" 발생 여부
- TargetActor 갱신 여부
- Engage 진입 여부
- AIPerception p95
- BT_UpdateAIContext p95
- Frame / GameThread p95
```

---

## 해석 기준

Perception Off에서 Engage 진입이 달라지면 다음처럼 해석한다.

```text
Perception Off 결과는 pure frame comparison이 아니다.
Perception 비용과 gameplay state 변화가 함께 반영된 비용 분리 측정이다.
```

유효한 결론:

```text
Perception 활성화 자체가 대량 Enemy에서 비용을 만드는지
Perception을 끄면 AIPerception p95가 얼마나 줄어드는지
BT_UpdateAIContext가 TargetDataMap / Blackboard 갱신 감소로 함께 줄어드는지
Perception Off가 gameplay state를 얼마나 바꾸는지
```

아직 결론 내리지 않는 항목:

```text
Perception Off를 그대로 Runtime LOD 정책으로 적용할지 여부
거리 기반 active cap 값
전투 중 Enemy의 perception 유지/해제 정책
```

---

## 후속 구현 후보

측정 결과가 유의미하면 후속 PR에서 다음 정책을 검토한다.

```text
distance-based perception active cap
combat importance 기반 perception enable / disable
time-sliced perception activation
dormant enemy perception disable
최근 target memory 유지 후 perception 비활성화
```
