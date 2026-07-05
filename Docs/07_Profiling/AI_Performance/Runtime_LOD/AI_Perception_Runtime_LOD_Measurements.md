# AI Perception Runtime LOD Measurements

## 목적

`P35: AI Runtime LOD 정책 정리`에서 AI Perception 비용과 후보 누수, target 인식 지연을 측정한 결과를 기록한다.

공통 분석 기준:

```text
Docs/07_Profiling/AI_Performance/Runtime_LOD/AI_Perception_Audit_Analysis_Guide.md
```

---

## 현재 구조 요약

Perception 흐름:

```text
ACAIController
-> UAIPerceptionComponent / UAISenseConfig_Sight 생성
-> OnTargetPerceptionUpdated에서 raw perception actor 수신
-> TargetDataMap 갱신
-> UCBTService_UpdateAIContext에서 BuildPerceptionContext 호출
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
Docs/07_Profiling/AI_Performance/Runtime_LOD/AI_Perception_Candidate_Audit_Plan.md
```

---

## 측정 스위치

```text
Portfolio.AI.RuntimeLOD.DisableEnemyPerception
0: Enemy Perception enabled
1: Enemy Perception disabled for runtime LOD measurement

Portfolio.AI.RuntimeLOD.PerceptionCandidateAudit
0: Candidate audit disabled
1: Enemy Perception candidate audit enabled
```

Perception Candidate Audit 출력:

```text
[PerceptionCandidateAudit]
Owner
RawEvents
RawActors
ValidProviders
InvalidProviders
MaxTargetDataMap
FirstRawLatency
FirstValidLatency
StartFrame
FirstRawFrame
FirstValidFrame
```

---

## 측정 기준

기본 측정 조건:

```text
Capture Duration: about 36s
Analysis Window: first 3s / last 3s trimmed, middle 30s used
Log State: -noailogging
PIE: F11 fullscreen
```

CSV 분석 기준:

```text
FrameTime 누적합 기준으로 first 3s / last 3s를 제외한다.
p95를 주요 비교값으로 사용한다.
avg는 경향 확인, p99 / max는 outlier 확인에 사용한다.
Unreal CSV는 중복 컬럼명이 있을 수 있으므로 header index 기반으로 파싱한다.
```

---

## Case PA01 - 40 Enemy / PerceptionCandidateAudit

측정 조건:

```text
Case: 40 Enemy / PerceptionCandidateAudit
Capture Duration: 약 36초
Analysis Window: first 3s / last 3s trimmed, middle 30s used
Log State: -noailogging
PIE: F11 fullscreen
CVar: Portfolio.AI.RuntimeLOD.PerceptionCandidateAudit 1
CSV: Csvprofile/Profile(20260705_190543).csv
Log: pasted-text 4ce5ed38-e802-43bf-a612-50d7f6d2e396
```

CSV 요약:

| Metric | Avg | p95 | p99 | Max |
|---|---:|---:|---:|---:|
| FrameTime | 11.2675ms | 12.3806ms | 13.0387ms | 13.7578ms |
| GameThreadTime | 11.1050ms | 12.2368ms | 12.7574ms | 13.9457ms |
| GPUTime | 9.7269ms | 10.5926ms | 10.9432ms | 11.4339ms |
| BT_UpdateAIContext | 0.1721ms | 0.2087ms | 0.2559ms | 0.3377ms |
| AIPerception | 0.1373ms | 0.1675ms | 0.4291ms | 0.5595ms |
| BehaviorTreeTick | 0.2616ms | 0.3414ms | 0.3910ms | 0.5280ms |
| CharacterMovement | 0.8353ms | 1.1782ms | 1.3080ms | 1.8102ms |
| Animation | 1.6377ms | 1.9318ms | 2.1640ms | 2.8431ms |

Audit 로그 요약:

| Metric | Min | Avg | p95 | Max |
|---|---:|---:|---:|---:|
| RawEvents | 506 | 511.425 | 516 | 517 |
| RawActors | 41 | 41 | 41 | 41 |
| ValidProviders | 1 | 1 | 1 | 1 |
| InvalidProviders | 40 | 40 | 40 | 40 |
| MaxTargetDataMap | 41 | 41 | 41 | 41 |
| FirstRawLatency | 0.009s | 0.341s | 0.432s | 0.432s |
| FirstValidLatency | 3.492s | 3.526s | 3.556s | 3.556s |

해석:

```text
40 Enemy 조건에서 각 Enemy는 평균적으로 41개의 raw perception actor를 보유한다.
그중 valid target provider는 1개이고 invalid provider는 40개다.
즉 Player 1개를 찾기 위해 같은 Enemy 40개가 perception 후보와 TargetDataMap에 함께 들어온다.

Raw perception은 대부분 0.5초 이내에 들어온다.
하지만 first valid target provider 인정은 약 3.5초 뒤에 발생한다.
따라서 현재 지연은 raw sight 감지 자체보다 provider filtering / target data update / BT service 갱신 구간에 있을 가능성이 높다.

AIPerception p95는 0.1675ms로 높지 않다.
하지만 후보 누수로 인해 TargetDataMap이 41개까지 커지는 구조가 확인됐다.
```

아직 확정하지 않는 항목:

```text
FirstValidLatency 약 3.5초의 직접 원인이 BT service interval인지, perception batch인지, target provider 이벤트 순서인지는 아직 확정하지 않는다.
80 Enemy 측정과 Blackboard / Engage latency audit 후 판단한다.
```

---

## Case PA02 - 40 Enemy / DisableEnemyPerception 비교

측정 조건:

```text
Case: 40 Enemy / DisableEnemyPerception comparison
Capture Duration: 약 36초
Analysis Window: first 3s / last 3s trimmed, middle 30s used
Log State: -noailogging
PIE: F11 fullscreen
CSV A: Csvprofile/Profile(20260705_232414).csv
Log A: Csvprofile/Log(20260705_232414).txt
CVar A: Portfolio.AI.RuntimeLOD.DisableEnemyPerception 0
CVar A: Portfolio.AI.RuntimeLOD.PerceptionCandidateAudit 1
CSV B: Csvprofile/Profile(20260705_232703).csv
Log B: Csvprofile/Log(20260705_232703).txt
CVar B: Portfolio.AI.RuntimeLOD.DisableEnemyPerception 1
CVar B: Portfolio.AI.RuntimeLOD.PerceptionCandidateAudit 1
Capture 중 CSVEvent "GC" 없음
```

CSV 비교:

| Metric | Disable 0 p95 | Disable 1 p95 | Delta |
|---|---:|---:|---:|
| FrameTime | 12.3510ms | 11.7484ms | -0.6026ms |
| GameThreadTime | 12.2010ms | 10.0197ms | -2.1813ms |
| GPUTime | 10.4527ms | 10.0674ms | -0.3853ms |
| AIPerception | 0.1742ms | 0.1538ms | -0.0204ms |
| BehaviorTreeTick | 0.3748ms | 0.1934ms | -0.1814ms |
| BT_UpdateAIContext | 0.2397ms | 0.0991ms | -0.1406ms |
| CharacterMovement | 1.3312ms | 0.3924ms | -0.9388ms |
| Animation | 1.9679ms | 2.0538ms | +0.0859ms |
| RHI DrawCalls | 834 | 747 | -87 |

Audit 로그 비교:

| Metric | Disable 0 p95 | Disable 1 p95 |
|---|---:|---:|
| RawEvents | 526 | 0 |
| RawActors | 41 | 0 |
| ValidProviders | 1 | 0 |
| InvalidProviders | 40 | 0 |
| MaxTargetDataMap | 41 | 0 |
| FirstRawLatency | 0.458s | -1.000s |
| FirstValidLatency | 3.741s | -1.000s |

해석:

```text
Perception을 끄면 GameThread p95가 약 2.18ms 낮아진다.
하지만 AIPerception p95 차이는 약 0.02ms 수준이다.
따라서 이번 비교에서 큰 차이는 perception engine 자체보다,
perception이 켜졌을 때 이어지는 BT update / target context / movement state 변화까지 포함한 총합 비용으로 해석한다.

Disable 0에서는 40 Enemy 각각이 Player 1명과 Enemy 40명을 후보로 본다.
Raw perception은 약 0.46초 안에 들어오지만 valid target provider 인정은 약 3.7초 뒤에 발생한다.
이전 PA01과 같은 패턴이 반복되므로 후보 누수와 first valid target 지연은 재현된 상태다.
```

주의:

```text
DisableEnemyPerception 1에서도 Audit CVar는 켜져 있다.
다만 perception delegate bind 경로가 차단되므로 기록할 RawEvents / RawActors / TargetDataMap이 0이 된다.
Disable 1 / Disable 0 비교는 순수 AIPerception 비용 비교가 아니라 perception 활성화로 파생되는 AI runtime 총합 비교다.
```

---

## Case PA03 - 80 Enemy / PerceptionCandidateAudit + DisableEnemyPerception 비교

측정 조건:

```text
Case: 80 Enemy / PerceptionCandidateAudit + DisableEnemyPerception comparison
Capture Duration: 약 36초
Analysis Window: first 3s / last 3s trimmed, middle 30s used
Log State: -noailogging
PIE: F11 fullscreen
CSV A: Csvprofile/Profile(20260705_205831).csv
Log A: Csvprofile/Log(20260705_205831).txt
CVar A: Portfolio.AI.RuntimeLOD.DisableEnemyPerception 0
CVar A: Portfolio.AI.RuntimeLOD.PerceptionCandidateAudit 1
CSV B: Csvprofile/Profile(20260705_210038).csv
Log B: Csvprofile/Log(20260705_210038).txt
CVar B: Portfolio.AI.RuntimeLOD.DisableEnemyPerception 1
CVar B: Portfolio.AI.RuntimeLOD.PerceptionCandidateAudit 1
Capture 중 CSVEvent "GC" 없음
```

CSV 비교:

| Metric | Disable 0 p95 | Disable 1 p95 | Delta |
|---|---:|---:|---:|
| FrameTime | 21.5917ms | 17.2850ms | -4.3067ms |
| GameThreadTime | 21.5991ms | 17.2840ms | -4.3151ms |
| GPUTime | 11.2417ms | 10.8227ms | -0.4190ms |
| AIPerception | 0.8591ms | 0.8079ms | -0.0512ms |
| BehaviorTreeTick | 0.7057ms | 0.3795ms | -0.3262ms |
| BT_UpdateAIContext | 0.4688ms | 0.1964ms | -0.2724ms |
| CharacterMovement | 2.9091ms | 0.9195ms | -1.9896ms |
| Animation | 3.8701ms | 4.9692ms | +1.0991ms |
| RHI DrawCalls | 1348 | 1272 | -76 |

Audit 로그 비교:

| Metric | Disable 0 p95 | Disable 1 p95 |
|---|---:|---:|
| RawEvents | 157 | 0 |
| RawActors | 81 | 0 |
| ValidProviders | 1 | 0 |
| InvalidProviders | 80 | 0 |
| MaxTargetDataMap | 81 | 0 |
| FirstRawLatency | 0.695s | -1.000s |
| FirstValidLatency | 9.591s | -1.000s |

해석:

```text
80 Enemy에서 perception을 끄면 GameThread p95가 약 4.3ms 낮아진다.
하지만 AIPerception p95 차이는 약 0.05ms 수준이다.
따라서 이번 결과도 perception engine 자체보다,
perception 활성화 이후 열리는 BT update / target context / movement / combat state 흐름의 총합 비용으로 해석한다.

Disable 0에서는 각 Enemy가 Player 1명과 Enemy 80명을 후보로 본다.
RawActors / InvalidProviders / MaxTargetDataMap이 40 Enemy 대비 거의 2배로 증가했다.
FirstValidLatency p95도 40 Enemy 약 3.7초에서 80 Enemy 약 9.6초로 증가했다.
즉 후보 누수는 Enemy 수에 비례하고, valid target 인정 지연도 Enemy 수 증가에 따라 커진다.

Disable 1에서는 RawEvents / RawActors / TargetDataMap이 모두 0이다.
Perception profiling gate가 후보 수집과 downstream target update를 차단하는 것은 확인됐다.
```

주의:

```text
Disable 1에서도 AIPerception p95가 0에 가까워지지는 않는다.
따라서 해당 CSV stat은 현재 map / engine level에서 남는 AIPerception 항목을 포함한다고 본다.
Gate 효과 판단은 Audit 로그의 RawEvents / RawActors / MaxTargetDataMap 0 여부와 downstream 비용 감소를 함께 본다.
```

---

## 다음 측정

Blackboard / Engage latency audit를 추가해 first valid target 이후 실제 Blackboard TargetActor 반영과 Engage 진입까지의 지연을 분리한다.

확인할 항목:

```text
FirstValidLatency 이후 Blackboard TargetActor 반영까지의 지연
Blackboard TargetActor 반영 이후 Engage request / assignment까지의 지연
BT service interval이 first valid target 지연에 관여하는지
```

측정 템플릿:

```text
Case: 80 Enemy / BlackboardEngageLatencyAudit
Capture Duration: 약 36초
Analysis Window: first 3s / last 3s trimmed, middle 30s used
Log State: -noailogging
PIE: F11 fullscreen
CVar: Portfolio.AI.RuntimeLOD.DisableEnemyPerception 0
CVar: Portfolio.AI.RuntimeLOD.PerceptionCandidateAudit 1
```

---

## 후속 개선 후보

```text
provider 없는 Actor를 TargetDataMap에 넣기 전에 필터링
team attitude 기반으로 Enemy 후보를 sight 단계에서 제외
distance / combat importance 기반 active perception cap
BT service interval / first valid target update timing 분리 측정
Blackboard / Engage latency audit 추가
```
