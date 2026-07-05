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

## Case PA04 - 40 Enemy / BlackboardEngageLatencyAudit

측정 파일:

```text
CSV: Portfolio/Csvprofile/Profile(20260706_002628).csv
Log: Portfolio/Csvprofile/Log(20260706_002628).txt
```

측정 조건:

```text
Case: 40 Enemy / BlackboardEngageLatencyAudit
Capture Duration: 약 36초
Analysis Window: first 3s / last 3s trimmed, middle 30s used
Log State: -noailogging
PIE: F11 fullscreen
CVar: Portfolio.AI.RuntimeLOD.DisableEnemyPerception 0
CVar: Portfolio.AI.RuntimeLOD.PerceptionCandidateAudit 1
CVar: Portfolio.AI.RuntimeLOD.BlackboardEngageLatencyAudit 1
```

CSV 요약:

| Metric | Avg | P95 | Max |
| --- | ---: | ---: | ---: |
| FrameTime | 12.2371ms | 13.6816ms | 16.7157ms |
| GameThreadTime | 12.1353ms | 13.5957ms | 16.1568ms |
| GPUTime | 9.5948ms | 10.4328ms | 11.5139ms |
| AIPerception | 0.1488ms | 0.1798ms | 0.6368ms |
| BehaviorTreeTick | 0.2869ms | 0.3735ms | 0.6769ms |
| BT_UpdateAIContext | 0.1914ms | 0.2263ms | 0.5225ms |
| BT_UpdateEngageContext | 0.0019ms | 0.0024ms | 0.0332ms |
| CombatEngage_Tick | 0.0011ms | 0.0075ms | 0.0307ms |
| CombatEngage_RebuildAssignments | 0.0009ms | 0.0070ms | 0.0301ms |
| CharacterMovement | 1.0089ms | 1.4425ms | 2.3108ms |
| Animation | 1.8378ms | 2.0754ms | 2.9224ms |
| AnimationParallelEvaluation TotalTaskTime | 3.4718ms | 4.0081ms | 4.9625ms |
| DrawCalls | 794.1332 | 830 | 867 |
| PrimitivesDrawn | 3,692,117 | 4,773,552 | 4,839,698 |
| CEnemy ActorCount | 81 | 81 | 81 |
| CWeaponActor ActorCount | 42 | 42 | 42 |
| CAIController ActorCount | 40 | 40 | 40 |
| TotalActorCount | 335 | 335 | 335 |

Audit 요약:

| Metric | Avg | P95 | Min | Max |
| --- | ---: | ---: | ---: | ---: |
| RawEvents | 518.575 | 522 | 514 | 524 |
| RawActors | 41 | 41 | 41 | 41 |
| ValidProviders | 1 | 1 | 1 | 1 |
| InvalidProviders | 40 | 40 | 40 | 40 |
| MaxTargetDataMap | 41 | 41 | 41 | 41 |
| FirstRawLatency | 0.354s | 0.450s | 0.010s | 0.450s |
| FirstValidLatency | 3.713s | 3.743s | 3.679s | 3.743s |

Blackboard / Engage latency 요약:

| Metric | Count | Avg | P95 | Min | Max |
| --- | ---: | ---: | ---: | ---: | ---: |
| PerceptionContextLatency | 40 | 3.724s | 3.754s | 3.690s | 3.754s |
| BlackboardTargetLatency | 40 | 3.724s | 3.754s | 3.690s | 3.754s |
| EngageRequestLatency | 40 | 3.724s | 3.754s | 3.690s | 3.754s |
| EngageAssignmentLatency | 2 | 3.764s | 3.764s | 3.764s | 3.764s |

Frame delta 요약:

| Delta | Count | Avg | P95 | Min | Max |
| --- | ---: | ---: | ---: | ---: | ---: |
| FirstValid -> PerceptionContext | 40 | 1.05 frames | 1 frame | 1 | 3 |
| PerceptionContext -> BlackboardTarget | 40 | 0 frames | 0 frames | 0 | 0 |
| BlackboardTarget -> EngageRequest | 40 | 0 frames | 0 frames | 0 | 0 |
| EngageRequest -> EngageAssignment | 2 | 4 frames | 5 frames | 3 | 5 |

해석:

```text
40 Enemy 조건에서 RawActors p95는 41, InvalidProviders p95는 40이다.
즉 Player 1명을 찾는 과정에서 Enemy 40명이 후보로 같이 들어온다.

FirstValidLatency p95는 3.743초이고,
PerceptionContextLatency / BlackboardTargetLatency / EngageRequestLatency p95는 모두 3.754초다.
FirstValid target이 인정된 뒤 PerceptionContext / BlackboardTarget / EngageRequest는 거의 같은 프레임에서 이어진다.

Frame delta 기준으로 FirstValid -> PerceptionContext는 p95 1 frame,
PerceptionContext -> BlackboardTarget은 0 frame,
BlackboardTarget -> EngageRequest도 0 frame이다.
따라서 이번 측정에서 3.7초 지연의 주 원인은 Blackboard 반영이나 Engage request가 아니다.

EngageAssignment은 40명 중 2명만 기록됐다.
이는 현재 engage assignment 정책상 target에 배정되는 attacker 수가 제한되기 때문이다.
기록된 2명은 request 이후 3~5 frame 안에 assignment가 완료됐다.

결론적으로 병목 후보는 FirstValid target이 나오기 전 단계다.
즉 raw perception 후보는 초반에 들어오지만,
valid target provider가 후보로 인정되는 시점이 늦다.
다음 분석은 team attitude / target provider filtering / perception candidate cap 쪽을 우선 본다.
```

---

## Case PA05 - 80 Enemy / BlackboardEngageLatencyAudit

측정 파일:

```text
CSV: Portfolio/Csvprofile/Profile(20260706_012112).csv
Log: Portfolio/Csvprofile/Log(20260706_012112).txt
```

측정 조건:

```text
Case: 80 Enemy / BlackboardEngageLatencyAudit
Capture Duration: 약 36초
Analysis Window: first 3s / last 3s trimmed, middle 30s used
Log State: -noailogging
PIE: F11 fullscreen
CVar: Portfolio.AI.RuntimeLOD.DisableEnemyPerception 0
CVar: Portfolio.AI.RuntimeLOD.PerceptionCandidateAudit 1
CVar: Portfolio.AI.RuntimeLOD.BlackboardEngageLatencyAudit 1
CVar: Portfolio.AI.RuntimeLOD.DisableEnemyWeaponActor 0
CVar: Portfolio.AI.RuntimeLOD.EnemyMeshMode 0
```

CSV 요약:

| Metric | Avg | P95 | Max |
| --- | ---: | ---: | ---: |
| FrameTime | 18.1419ms | 20.5497ms | 98.8381ms |
| GameThreadTime | 18.0174ms | 20.3026ms | 93.1750ms |
| GPUTime | 10.7054ms | 11.5416ms | 17.3124ms |
| AIPerception | 0.2729ms | 0.8596ms | 1.1915ms |
| BehaviorTreeTick | 0.5611ms | 0.6971ms | 1.3450ms |
| BT_UpdateAIContext | 0.3882ms | 0.4739ms | 1.0331ms |
| CombatEngage_Tick | 0.0017ms | 0.0112ms | 0.0203ms |
| CombatEngage_RebuildAssignments | 0.0015ms | 0.0109ms | 0.0198ms |
| CharacterMovement | 1.7660ms | 2.6772ms | 5.2863ms |
| Animation | 3.2665ms | 3.5045ms | 73.0987ms |
| AnimationParallelEvaluation TotalTaskTime | 5.6968ms | 6.6040ms | 8.8496ms |
| DrawCalls | 1,309.4449 | 1,352 | 2,480 |
| PrimitivesDrawn | 7,001,508 | 8,013,446 | 13,502,118 |
| CEnemy ActorCount | 161 | 161 | 161 |
| CWeaponActor ActorCount | 82 | 82 | 82 |
| CAIController ActorCount | 80 | 80 | 80 |
| TotalActorCount | 483 | 483 | 483 |

Audit 요약:

| Metric | Avg | P95 | Min | Max |
| --- | ---: | ---: | ---: | ---: |
| RawEvents | 170.962 | 172 | 169 | 173 |
| RawActors | 81 | 81 | 81 | 81 |
| ValidProviders | 1 | 1 | 1 | 1 |
| InvalidProviders | 80 | 80 | 80 | 80 |
| MaxTargetDataMap | 81 | 81 | 81 | 81 |
| FirstRawLatency | 0.773s | 0.962s | 0.010s | 0.962s |
| FirstValidLatency | 9.294s | 9.377s | 9.197s | 9.393s |

Blackboard / Engage latency 요약:

| Metric | Count | Avg | P95 | Min | Max |
| --- | ---: | ---: | ---: | ---: | ---: |
| PerceptionContextLatency | 80 | 9.310s | 9.393s | 9.212s | 9.408s |
| BlackboardTargetLatency | 80 | 9.310s | 9.393s | 9.212s | 9.408s |
| EngageRequestLatency | 80 | 9.310s | 9.393s | 9.212s | 9.408s |
| EngageAssignmentLatency | 2 | 9.242s | 9.242s | 9.242s | 9.242s |

Frame delta 요약:

| Delta | Count | Avg | P95 | Min | Max |
| --- | ---: | ---: | ---: | ---: | ---: |
| FirstValid -> PerceptionContext | 80 | 1.025 frames | 1 frame | 1 | 3 |
| PerceptionContext -> BlackboardTarget | 80 | 0 frames | 0 frames | 0 | 0 |
| BlackboardTarget -> EngageRequest | 80 | 0 frames | 0 frames | 0 | 0 |
| EngageRequest -> EngageAssignment | 2 | 1 frame | 1 frame | 1 | 1 |

해석:

```text
80 Enemy 조건에서도 RawActors p95는 81, InvalidProviders p95는 80이다.
Player 1명을 찾는 동안 Enemy 80명이 perception 후보로 같이 들어온다.

FirstRawLatency p95는 0.962초이고 FirstValidLatency p95는 9.377초다.
40 Enemy의 FirstValidLatency p95 3.743초 대비 크게 증가했다.
즉 후보 누수 규모가 커질수록 valid target 인정 지연도 같이 증가한다.

PerceptionContextLatency / BlackboardTargetLatency / EngageRequestLatency p95는 모두 9.393초다.
Frame delta 기준으로 FirstValid -> PerceptionContext는 p95 1 frame,
PerceptionContext -> BlackboardTarget은 0 frame,
BlackboardTarget -> EngageRequest도 0 frame이다.

따라서 80 Enemy에서도 장기 지연은 Blackboard 반영이나 Engage request 단계가 아니라 FirstValid target 이전 단계에서 발생한다.
EngageAssignment는 80명 중 2명만 기록됐고, request 이후 1 frame 안에 assignment가 완료됐다.

결론적으로 team attitude / affiliation을 통해 Enemy끼리 perception 대상이 되지 않게 만드는 작업이 우선이다.
그 전 단계의 방어선으로 TargetDataMap에 invalid provider를 넣지 않도록 보정한다.
```

---

## 적용된 1차 보정

```text
ACAIController::OnTargetPerceptionUpdated()에서 ITargetContextProvider가 없는 Actor는 TargetDataMap에 넣지 않는다.
Raw perception callback 자체는 계속 기록하지만, provider 없는 Enemy 후보가 downstream target selection으로 전파되지 않게 막는다.
```

기대 효과:

```text
RawActors / InvalidProviders는 여전히 높게 나올 수 있다.
하지만 MaxTargetDataMap은 Player target 중심으로 낮아져야 한다.
BT_UpdateAIContext / SelectTopPriority / TargetDataMap 순회 비용 감소를 확인한다.
FirstValidLatency가 얼마나 줄어드는지는 별도 측정으로 확인한다.
```

다음 측정:

```text
Case: 40 Enemy / TargetDataMapProviderGuard
Case: 80 Enemy / TargetDataMapProviderGuard
CVar: Portfolio.AI.RuntimeLOD.DisableEnemyPerception 0
CVar: Portfolio.AI.RuntimeLOD.PerceptionCandidateAudit 1
CVar: Portfolio.AI.RuntimeLOD.BlackboardEngageLatencyAudit 1
CVar: Portfolio.AI.RuntimeLOD.DisableEnemyWeaponActor 0
CVar: Portfolio.AI.RuntimeLOD.EnemyMeshMode 0
```

---

## Case PA06 - 40 Enemy / TargetDataMapProviderGuard

측정 파일:

```text
CSV: Portfolio/Csvprofile/Profile(20260706_014017).csv
Log: Portfolio/Csvprofile/Log(20260706_014017).txt
```

측정 조건:

```text
Case: 40 Enemy / TargetDataMapProviderGuard
Capture Duration: 약 36초
Analysis Window: first 3s / last 3s trimmed, middle 30s used
Log State: -noailogging
PIE: F11 fullscreen
CVar: Portfolio.AI.RuntimeLOD.DisableEnemyPerception 0
CVar: Portfolio.AI.RuntimeLOD.PerceptionCandidateAudit 1
CVar: Portfolio.AI.RuntimeLOD.BlackboardEngageLatencyAudit 1
CVar: Portfolio.AI.RuntimeLOD.DisableEnemyWeaponActor 0
CVar: Portfolio.AI.RuntimeLOD.EnemyMeshMode 0
```

CSV 요약:

| Metric | Avg | P95 | Max |
| --- | ---: | ---: | ---: |
| FrameTime | 13.2622ms | 14.4562ms | 16.0033ms |
| GameThreadTime | 13.2574ms | 14.5096ms | 15.9328ms |
| GPUTime | 9.7800ms | 10.5924ms | 11.5603ms |
| AIPerception | 0.1385ms | 0.1745ms | 0.6465ms |
| BehaviorTreeTick | 0.2303ms | 0.3216ms | 0.5170ms |
| BT_UpdateAIContext | 0.1344ms | 0.1658ms | 0.3129ms |
| CombatEngage_Tick | 0.0011ms | 0.0074ms | 0.0171ms |
| CombatEngage_RebuildAssignments | 0.0009ms | 0.0071ms | 0.0164ms |
| CharacterMovement | 1.0302ms | 1.3464ms | 2.2288ms |
| Animation | 1.8577ms | 2.0374ms | 2.6015ms |
| AnimationParallelEvaluation TotalTaskTime | 3.3673ms | 3.7533ms | 4.6682ms |
| DrawCalls | 797.9361 | 836 | 874 |
| PrimitivesDrawn | 3,914,389 | 5,252,994 | 5,278,282 |
| CEnemy ActorCount | 81 | 81 | 81 |
| CWeaponActor ActorCount | 42 | 42 | 42 |
| CAIController ActorCount | 40 | 40 | 40 |
| TotalActorCount | 323 | 323 | 323 |

Audit 요약:

| Metric | Avg | P95 | Min | Max |
| --- | ---: | ---: | ---: | ---: |
| RawEvents | 476.275 | 479 | 472 | 483 |
| RawActors | 41 | 41 | 41 | 41 |
| ValidProviders | 1 | 1 | 1 | 1 |
| InvalidProviders | 40 | 40 | 40 | 40 |
| MaxTargetDataMap | 1 | 1 | 1 | 1 |
| FirstRawLatency | 0.346s | 0.442s | 0.010s | 0.442s |
| FirstValidLatency | 3.702s | 3.734s | 3.667s | 3.734s |

Blackboard / Engage latency 요약:

| Metric | Count | Avg | P95 | Min | Max |
| --- | ---: | ---: | ---: | ---: | ---: |
| PerceptionContextLatency | 40 | 3.716s | 3.746s | 3.678s | 3.780s |
| BlackboardTargetLatency | 40 | 3.716s | 3.746s | 3.678s | 3.780s |
| EngageRequestLatency | 40 | 3.716s | 3.746s | 3.678s | 3.780s |
| EngageAssignmentLatency | 2 | 3.734s | 3.734s | 3.734s | 3.734s |

Frame delta 요약:

| Delta | Count | Avg | P95 | Min | Max |
| --- | ---: | ---: | ---: | ---: | ---: |
| FirstValid -> PerceptionContext | 40 | 1.175 frames | 1 frame | 1 | 5 |
| PerceptionContext -> BlackboardTarget | 40 | 0 frames | 0 frames | 0 | 0 |
| BlackboardTarget -> EngageRequest | 40 | 0 frames | 0 frames | 0 | 0 |
| EngageRequest -> EngageAssignment | 2 | 4.5 frames | 5 frames | 4 | 5 |

이전 40 Enemy 기준과 비교:

| Metric | PA04 Before | PA06 ProviderGuard |
| --- | ---: | ---: |
| MaxTargetDataMap p95 | 41 | 1 |
| FirstValidLatency p95 | 3.743s | 3.734s |
| PerceptionContextLatency p95 | 3.754s | 3.746s |
| BT_UpdateAIContext p95 | 0.2263ms | 0.1658ms |
| BehaviorTreeTick p95 | 0.3735ms | 0.3216ms |
| AIPerception p95 | 0.1798ms | 0.1745ms |

해석:

```text
TargetDataMap provider guard는 의도대로 동작했다.
RawActors / InvalidProviders는 여전히 41 / 40이지만 MaxTargetDataMap p95는 41에서 1로 감소했다.
즉 perception callback 후보 누수는 남아 있지만 downstream target map 전파는 차단됐다.

BT_UpdateAIContext p95는 0.2263ms에서 0.1658ms로 줄었다.
따라서 TargetDataMap 순회 / target selection 비용에는 개선 효과가 있다.

FirstValidLatency p95는 3.743초에서 3.734초로 거의 변하지 않았다.
따라서 장기 지연은 TargetDataMap 삽입 이후가 아니라 perception callback에서 valid provider가 들어오기 전 단계에 남아 있다.

다음 개선은 team attitude / affiliation으로 Enemy끼리 perception target이 되지 않게 하는 방향이 우선이다.
```

---

## Case PA07 - 80 Enemy / TargetDataMapProviderGuard

측정 파일:

```text
CSV: Portfolio/Csvprofile/Profile(20260706_014454).csv
Log: Portfolio/Csvprofile/Log(20260706_014454).txt
```

측정 조건:

```text
Case: 80 Enemy / TargetDataMapProviderGuard
Capture Duration: 약 36초
Analysis Window: first 3s / last 3s trimmed, middle 30s used
Log State: -noailogging
PIE: F11 fullscreen
CVar: Portfolio.AI.RuntimeLOD.DisableEnemyPerception 0
CVar: Portfolio.AI.RuntimeLOD.PerceptionCandidateAudit 1
CVar: Portfolio.AI.RuntimeLOD.BlackboardEngageLatencyAudit 1
CVar: Portfolio.AI.RuntimeLOD.DisableEnemyWeaponActor 0
CVar: Portfolio.AI.RuntimeLOD.EnemyMeshMode 0
```

CSV 요약:

| Metric | Avg | P95 | Max |
| --- | ---: | ---: | ---: |
| FrameTime | 19.7909ms | 23.0862ms | 25.2496ms |
| GameThreadTime | 19.7852ms | 23.1067ms | 25.2857ms |
| GPUTime | 10.7434ms | 11.5051ms | 12.2351ms |
| AIPerception | 0.2756ms | 0.8280ms | 1.0464ms |
| BehaviorTreeTick | 0.4180ms | 0.5377ms | 0.9374ms |
| BT_UpdateAIContext | 0.2368ms | 0.2903ms | 0.4663ms |
| CombatEngage_Tick | 0.0019ms | 0.0119ms | 0.0239ms |
| CombatEngage_RebuildAssignments | 0.0017ms | 0.0115ms | 0.0236ms |
| CharacterMovement | 1.8786ms | 2.9221ms | 4.0249ms |
| Animation | 3.3905ms | 3.7309ms | 5.0358ms |
| AnimationParallelEvaluation TotalTaskTime | 5.7087ms | 6.5796ms | 7.6793ms |
| DrawCalls | 1,301.0576 | 1,351 | 1,406 |
| PrimitivesDrawn | 6,633,567 | 7,773,164 | 7,957,652 |
| CEnemy ActorCount | 161 | 161 | 161 |
| CWeaponActor ActorCount | 82 | 82 | 82 |
| CAIController ActorCount | 80 | 80 | 80 |
| TotalActorCount | 483 | 483 | 483 |

Audit 요약:

| Metric | Avg | P95 | Min | Max |
| --- | ---: | ---: | ---: | ---: |
| RawEvents | 147.450 | 149 | 146 | 149 |
| RawActors | 81 | 81 | 81 | 81 |
| ValidProviders | 1 | 1 | 1 | 1 |
| InvalidProviders | 80 | 80 | 80 | 80 |
| MaxTargetDataMap | 1 | 1 | 1 | 1 |
| FirstRawLatency | 0.522s | 0.684s | 0.010s | 0.684s |
| FirstValidLatency | 9.785s | 9.877s | 9.678s | 9.894s |

Blackboard / Engage latency 요약:

| Metric | Count | Avg | P95 | Min | Max |
| --- | ---: | ---: | ---: | ---: | ---: |
| PerceptionContextLatency | 80 | 9.804s | 9.911s | 9.694s | 9.911s |
| BlackboardTargetLatency | 80 | 9.804s | 9.911s | 9.694s | 9.911s |
| EngageRequestLatency | 80 | 9.804s | 9.911s | 9.694s | 9.911s |
| EngageAssignmentLatency | 2 | 9.742s | 9.742s | 9.742s | 9.742s |

Frame delta 요약:

| Delta | Count | Avg | P95 | Min | Max |
| --- | ---: | ---: | ---: | ---: | ---: |
| FirstValid -> PerceptionContext | 80 | 1.175 frames | 1 frame | 1 | 6 |
| PerceptionContext -> BlackboardTarget | 80 | 0 frames | 0 frames | 0 | 0 |
| BlackboardTarget -> EngageRequest | 80 | 0 frames | 0 frames | 0 | 0 |
| EngageRequest -> EngageAssignment | 2 | 1 frame | 1 frame | 1 | 1 |

이전 80 Enemy 기준과 비교:

| Metric | PA05 Before | PA07 ProviderGuard |
| --- | ---: | ---: |
| MaxTargetDataMap p95 | 81 | 1 |
| FirstValidLatency p95 | 9.377s | 9.877s |
| PerceptionContextLatency p95 | 9.393s | 9.911s |
| BT_UpdateAIContext p95 | 0.4739ms | 0.2903ms |
| BehaviorTreeTick p95 | 0.6971ms | 0.5377ms |
| AIPerception p95 | 0.8596ms | 0.8280ms |

해석:

```text
80 Enemy에서도 TargetDataMap provider guard는 의도대로 동작했다.
MaxTargetDataMap p95는 81에서 1로 감소했다.
BT_UpdateAIContext p95도 0.4739ms에서 0.2903ms로 줄었다.

하지만 RawActors / InvalidProviders는 81 / 80으로 그대로다.
FirstValidLatency p95도 9초대 후반으로 남아 있다.
따라서 provider guard는 downstream 비용을 줄이는 보정이고, perception 후보 생성 / dispatch 지연의 근본 해결은 아니다.

40 / 80 Enemy 모두 같은 패턴이 반복됐으므로 다음 작업은 team attitude / affiliation 기반으로 Enemy끼리 perception 대상이 되지 않게 하는 것이다.
```

---

## 적용된 2차 보정

```text
ACAIController::GetTeamAttitudeTowards()를 override해 perception affiliation 기준을 명시했다.
ACPlayer는 Hostile로 취급한다.
ACEnemy는 Friendly로 취급한다.
그 외 Actor는 Neutral로 취급한다.
```

의도:

```text
SightConfig는 DetectEnemies=true, DetectFriendlies=false, DetectNeutrals=false로 설정되어 있다.
따라서 Enemy AIController 기준에서 Player만 perception target으로 남기고,
Enemy끼리는 sight 대상에서 제외하는 것이 목표다.
```

기대 효과:

```text
RawActors는 1에 가까워져야 한다.
InvalidProviders는 0에 가까워져야 한다.
MaxTargetDataMap은 provider guard 적용 상태와 동일하게 1에 가까워야 한다.
FirstValidLatency가 줄어드는지 확인한다.
AIPerception / BT_UpdateAIContext / CharacterMovement p95 변화도 함께 확인한다.
```

다음 측정:

```text
Case: 40 Enemy / TeamAttitudeAffiliation
Case: 80 Enemy / TeamAttitudeAffiliation
CVar: Portfolio.AI.RuntimeLOD.DisableEnemyPerception 0
CVar: Portfolio.AI.RuntimeLOD.PerceptionCandidateAudit 1
CVar: Portfolio.AI.RuntimeLOD.BlackboardEngageLatencyAudit 1
CVar: Portfolio.AI.RuntimeLOD.DisableEnemyWeaponActor 0
CVar: Portfolio.AI.RuntimeLOD.EnemyMeshMode 0
```

---

## 후속 개선 후보

```text
distance / combat importance 기반 active perception cap
BT service interval / first valid target update timing 추가 분리
```
