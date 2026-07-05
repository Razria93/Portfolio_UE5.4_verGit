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
ValidProviders가 1인데 InvalidProviders가 크면 Player 1명을 찾기 위해 다수 Enemy를 같이 처리하는 구조다.
```

### 지연 판단

```text
FirstRawLatency가 낮고 FirstValidLatency가 높으면 raw sight 감지 자체보다는 valid target 인정 경로를 의심한다.
FirstRawLatency와 FirstValidLatency가 함께 높으면 perception stimulus 처리 또는 activation 시점도 의심한다.
FirstValidLatency가 여러 Enemy에서 비슷한 값으로 몰리면 BT service interval 또는 batch update 가능성이 있다.
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

Interpretation:
- 후보 누수:
- valid target 지연:
- 다음 측정 필요 여부:
```
