# AI Perception Audit Analysis Guide

## 목적

Perception Candidate Audit 측정 결과를 빠르게 해석하기 위한 기준을 정리한다.

이 문서는 다음 분석을 반복하지 않기 위한 체크리스트다.

```text
CSV 파싱 기준
PerceptionCandidateAudit 로그 해석 기준
유효한 결론과 아직 결론 내리면 안 되는 항목
다음 측정으로 확인해야 하는 항목
```

---

## 분석이 오래 걸렸던 이유

이번 40 Enemy 측정 분석이 오래 걸린 이유는 세 가지다.

```text
1. CSV 컬럼명이 중복되어 PowerShell Import-Csv가 실패했다.
2. 로컬 환경에서 python 명령이 연결되어 있지 않아 대체 파서를 바로 쓰지 못했다.
3. 기존 측정 문서 일부가 mojibake 상태라 새 결과를 붙이는 과정에서 문서 검증 시간이 추가로 들었다.
```

따라서 이후 CSV 분석은 `Import-Csv`를 기본값으로 쓰지 않는다.
Unreal CSV는 같은 컬럼명이 반복될 수 있으므로 header index 기반으로 읽는다.

PowerShell 로그 필터링에서는 `-like '*[PerceptionCandidateAudit]*'`를 쓰지 않는다.
`[]`가 wildcard character set으로 해석되어 `Cmd` / `LogCsvProfiler` 라인까지 섞일 수 있다.
Audit 로그는 `.Contains('[PerceptionCandidateAudit]')` 또는 `Select-String -SimpleMatch`로 찾는다.

---

## 입력 자료 확인 순서

분석을 시작할 때 다음 순서로 확인한다.

```text
1. CSV 파일 경로가 실제로 존재하는지 확인한다.
2. 첨부 로그에 LogCsvProfiler start / stop 구간이 있는지 확인한다.
3. 첨부 로그에 CSVEvent "GC"가 capture 중 발생했는지 확인한다.
4. PerceptionCandidateAudit 로그 라인이 Enemy 수만큼 있는지 확인한다.
5. CVar / Enemy count / map / PIE fullscreen / analysis window 조건을 확인한다.
```

문제가 있으면 먼저 문제를 말하고 분석 신뢰도를 낮춰 기록한다.

---

## CSV 분석 기준

기본 분석 구간:

```text
Capture Duration: about 36s
Analysis Window: first 3s / last 3s trimmed, middle 30s used
```

계산 방식:

```text
FrameTime 누적합으로 전체 capture time을 계산한다.
누적 시간이 3초 이상이고 total - 3초 이하인 frame만 사용한다.
p95를 주요 비교값으로 사용한다.
avg는 경향 확인, p99 / max는 outlier 확인에 사용한다.
```

주요 CSV 지표:

```text
FrameTime
GameThreadTime
GPUTime
RenderThreadTime
Exclusive/GameThread/AIPerception
Exclusive/GameThread/BehaviorTreeTick
GameThread/PortfolioAI_BT_UpdateAIContext
GameThread/PortfolioAI_CombatEngage_Tick
GameThread/PortfolioAI_CombatEngage_RebuildAssignments
Exclusive/GameThread/CharacterMovement
Exclusive/GameThread/Animation
ActorCount/CEnemy
ActorCount/CWeaponActor
ActorCount/CAIController
ActorCount/TotalActorCount
```

주의:

```text
AIPerception p95가 낮아도 후보 누수가 없다는 뜻은 아니다.
Perception 후보 수와 TargetDataMap 크기는 Audit 로그로 따로 본다.
Frame / GameThread p95는 전체 성능 상태를 보는 보조 지표다.
```

---

## Audit 로그 분석 기준

분석 대상 로그:

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

Blackboard / Engage latency 로그:

```text
[BlackboardEngageLatencyAudit]
Owner
PerceptionContextLatency
BlackboardTargetLatency
EngageRequestLatency
EngageAssignmentLatency
StartFrame
PerceptionContextFrame
BlackboardTargetFrame
EngageRequestFrame
EngageAssignmentFrame
PerceptionTarget
BlackboardTarget
EngageRequestTarget
EngageAssignmentTarget
```

각 값의 의미:

```text
RawEvents
-> perception callback에서 raw actor를 감지한 이벤트 수

RawActors
-> 해당 AI가 감지 후보로 본 unique actor 수

ValidProviders
-> ITargetContextProvider를 가진 유효 target 수

InvalidProviders
-> raw 후보였지만 ITargetContextProvider가 없는 actor 수

MaxTargetDataMap
-> 측정 중 TargetDataMap이 커진 최대 크기

FirstRawLatency
-> controller runtime 시작 후 첫 raw perception 후보가 들어오기까지 걸린 시간

FirstValidLatency
-> controller runtime 시작 후 첫 valid target provider가 들어오기까지 걸린 시간
```

---

## 해석 규칙

### 후보 누수 판단

```text
InvalidProviders가 Enemy 수에 비례해서 증가하면 후보 누수가 있다.
MaxTargetDataMap이 RawActors와 같거나 비슷하면 raw 후보가 TargetDataMap까지 들어간다.
provider guard 적용 후에도 RawActors / InvalidProviders는 높을 수 있다.
이때 MaxTargetDataMap만 낮아지면 callback 후보 누수는 남아 있지만 downstream target map 전파는 차단된 것으로 본다.
ValidProviders가 1인데 InvalidProviders가 크면 Player 1명을 찾기 위해 다수 Enemy를 같이 처리하는 구조다.
```

### 지연 판단

```text
FirstRawLatency가 낮고 FirstValidLatency가 높으면 raw sight 감지 자체보다는 valid target 인정 경로를 의심한다.
FirstRawLatency와 FirstValidLatency가 함께 높으면 perception stimulus 처리 또는 activation 시점도 의심한다.
FirstValidLatency가 여러 Enemy에서 비슷한 값으로 몰리면 BT service interval 또는 batch update 가능성이 있다.
```

### Blackboard / Engage 지연 판단

```text
FirstValidLatency와 PerceptionContextLatency가 거의 같고 frame delta가 1 frame 수준이면,
valid target 인정 이후 BuildPerceptionContext까지의 추가 지연은 작다고 본다.

PerceptionContextLatency, BlackboardTargetLatency, EngageRequestLatency가 같고 frame delta가 0이면,
Blackboard 반영과 Engage request는 같은 BT service tick 안에서 이어진 것으로 본다.

EngageAssignmentLatency는 assignment를 받은 AI만 따로 valid-only 기준으로 해석한다.
전체 Enemy 중 일부만 assignment를 받는 구조라면 -1.000 값을 p95에 섞지 않는다.

EngageRequest -> EngageAssignment frame delta가 한 자리 frame이면,
CombatEngage rebuild interval은 장기 지연 원인이 아니라고 본다.

따라서 FirstValidLatency는 높은데 이후 단계의 frame delta가 작으면,
Blackboard / Engage subsystem이 아니라 first valid target 이전 단계를 우선 의심한다.
```

### 성능 판단

```text
AIPerception p95가 낮으면 perception engine 자체 비용은 낮다고 볼 수 있다.
하지만 TargetDataMap이 커지면 BT_UpdateAIContext / target selection / provider filtering 비용으로 전파될 수 있다.
따라서 AIPerception p95와 Audit 후보 수를 따로 해석한다.
```

---

## 이번 40 Enemy 결과 기준 해석

측정 결과:

```text
RawActors = 41
ValidProviders = 1
InvalidProviders = 40
MaxTargetDataMap = 41
FirstRawLatency p95 = 0.432s
FirstValidLatency p95 = 3.556s
AIPerception p95 = 0.1675ms
BT_UpdateAIContext p95 = 0.2087ms
```

해석:

```text
40 Enemy 조건에서도 각 Enemy가 Player 1명과 Enemy 40명을 raw 후보로 같이 본다.
raw sight 감지는 빠르지만 valid target provider 인정은 약 3.5초 뒤에 발생한다.
AIPerception 비용 자체는 크지 않지만 후보 누수가 TargetDataMap을 키우는 구조가 확인됐다.
```

아직 확정하지 않는 항목:

```text
FirstValidLatency 3.5초의 직접 원인이 BT service interval인지, perception batch인지, target provider 이벤트 순서인지는 아직 확정하지 않는다.
80 Enemy 측정과 Blackboard / Engage latency audit 후 판단한다.
```

---

## Blackboard / Engage Audit 기준 해석

40 Enemy 측정 결과:

```text
Case = 40 Enemy / BlackboardEngageLatencyAudit
RawActors p95 = 41
InvalidProviders p95 = 40
FirstRawLatency p95 = 0.450s
FirstValidLatency p95 = 3.743s

PerceptionContextLatency p95 = 3.754s
BlackboardTargetLatency p95 = 3.754s
EngageRequestLatency p95 = 3.754s

FirstValid -> PerceptionContext p95 = 1 frame
PerceptionContext -> BlackboardTarget p95 = 0 frame
BlackboardTarget -> EngageRequest p95 = 0 frame
EngageRequest -> EngageAssignment p95 = 5 frames
```

해석:

```text
이번 측정에서는 valid target 인정 이후 Blackboard TargetActor 반영과 Engage request까지의 추가 지연이 거의 없다.
따라서 3.7초 지연은 Blackboard 반영이나 Engage subsystem에서 만들어진 지연으로 보지 않는다.

Raw 후보는 초반에 들어오지만 valid target provider가 늦게 인정된다.
다음 원인 후보는 team attitude 미분리, target provider filtering 위치, perception candidate cap 부재다.
```

80 Enemy 측정 결과:

```text
Case = 80 Enemy / BlackboardEngageLatencyAudit
RawActors p95 = 81
InvalidProviders p95 = 80
FirstRawLatency p95 = 0.962s
FirstValidLatency p95 = 9.377s

PerceptionContextLatency p95 = 9.393s
BlackboardTargetLatency p95 = 9.393s
EngageRequestLatency p95 = 9.393s

FirstValid -> PerceptionContext p95 = 1 frame
PerceptionContext -> BlackboardTarget p95 = 0 frame
BlackboardTarget -> EngageRequest p95 = 0 frame
EngageRequest -> EngageAssignment p95 = 1 frame
```

80 Enemy 해석:

```text
80 Enemy에서도 valid target 이후 Blackboard / Engage 단계의 추가 지연은 거의 없다.
FirstValidLatency가 40 Enemy 약 3.7초에서 80 Enemy 약 9.4초로 증가했다.
따라서 후보 수 증가가 valid target 인정 지연을 확대하는 패턴으로 본다.
```

Provider guard 적용 후 40 Enemy 측정:

```text
Case = 40 Enemy / TargetDataMapProviderGuard
RawActors p95 = 41
InvalidProviders p95 = 40
MaxTargetDataMap p95 = 1
FirstValidLatency p95 = 3.734s
BT_UpdateAIContext p95 = 0.1658ms
```

Provider guard 해석:

```text
MaxTargetDataMap이 41에서 1로 줄었으므로 downstream target map 전파는 차단됐다.
BT_UpdateAIContext p95도 0.2263ms에서 0.1658ms로 감소했다.
따라서 TargetDataMap 순회 / target selection 비용에는 개선 효과가 있다.

FirstValidLatency p95는 거의 변하지 않았다.
따라서 장기 지연은 TargetDataMap 삽입 이후가 아니라 perception callback에서 valid target provider가 들어오기 전 단계에 남아 있다.
```

Provider guard 적용 후 80 Enemy 측정:

```text
Case = 80 Enemy / TargetDataMapProviderGuard
RawActors p95 = 81
InvalidProviders p95 = 80
MaxTargetDataMap p95 = 1
FirstValidLatency p95 = 9.877s
BT_UpdateAIContext p95 = 0.2903ms
```

80 Enemy Provider guard 해석:

```text
MaxTargetDataMap이 81에서 1로 줄었으므로 80 Enemy에서도 downstream target map 전파는 차단됐다.
BT_UpdateAIContext p95는 0.4739ms에서 0.2903ms로 감소했다.
하지만 FirstValidLatency는 9초대 후반으로 남았다.
따라서 provider guard는 target map / BT context 비용을 낮추는 보정이고, perception 후보 생성 / dispatch 지연의 근본 해결은 아니다.
```

Team attitude / affiliation 적용 후 판단:

```text
ACAIController 기준에서 ACPlayer는 Hostile, ACEnemy는 Friendly, 그 외 Actor는 Neutral로 취급한다.
SightConfig가 DetectEnemies=true, DetectFriendlies=false, DetectNeutrals=false라면 Enemy끼리는 perception target에서 제외되어야 한다.

적용 후 RawActors가 1에 가까워지고 InvalidProviders가 0에 가까워지면 affiliation filter가 동작한 것으로 본다.
RawActors / InvalidProviders가 그대로라면 GetTeamAttitudeTowards override만으로는 현재 perception target filtering에 연결되지 않은 것으로 보고,
target actor team id 제공 방식 또는 controller / pawn team interface 구성을 추가 검토한다.
```

---

## 다음 분석 템플릿

```text
Case:
CSV:
Log:
Enemy:
CVar:
Analysis Window:
GC Event:

CSV p95:
- FrameTime:
- GameThreadTime:
- AIPerception:
- BT_UpdateAIContext:
- BehaviorTreeTick:

Audit summary:
- RawActors:
- ValidProviders:
- InvalidProviders:
- MaxTargetDataMap:
- FirstRawLatency p95:
- FirstValidLatency p95:

Blackboard / Engage latency summary:
- PerceptionContextLatency p95:
- BlackboardTargetLatency p95:
- EngageRequestLatency p95:
- EngageAssignmentLatency p95:

Interpretation:
- 후보 누수:
- valid target 지연:
- Blackboard 반영 지연:
- Engage request / assignment 지연:
- 다음 측정 필요 여부:
```
