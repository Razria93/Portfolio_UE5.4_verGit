# AI Perception Candidate Audit Plan

## 목적

이 문서는 `P35: AI Runtime LOD 정책 정리`에서 Perception Gate 측정 전에 확인할 Perception 후보 누수와 인지 지연 측정 계획을 정리한다.

Perception을 끄고 켜는 비교만으로는 다음 항목을 분리하기 어렵다.

```text
Perception system이 Actor를 감지한 시점
감지된 Actor 중 유효 target으로 인정된 시점
Blackboard TargetActor로 반영된 시점
Engage request / assignment가 발생한 시점
```

따라서 Perception Gate 측정 전에 감지 후보의 품질과 target 인식 지연을 먼저 계측한다.

---

## 관찰된 현상

대량 Enemy stress 조건에서 다음 현상이 관찰됐다.

```text
시야 범위에 Player가 들어와도 TargetActor 반영이 늦게 발생한다.
딜레이가 발생하면 여러 Enemy가 비슷한 시점에 동시에 움직이기 시작한다.
Perception debug에서 같은 Enemy 계열 Actor도 감지 후보로 잡히는 것으로 보인다.
Neutral 감지 설정을 끄면 Player까지 감지되지 않는 문제가 있다.
```

이 현상은 단순히 특정 Enemy 하나가 target 선택 순서에서 밀리는 문제로 보기 어렵다.
개별 Enemy마다 차등적으로 늦어지는 패턴보다, Perception / BT / Engage 갱신이 일정 시점에 같이 열리는 패턴에 가깝다.

---

## 현재 구조 해석

현재 코드 기준으로 최종 target 후보는 `ITargetContextProvider`를 통해 제한된다.

```text
ACPlayer
-> ITargetContextProvider 구현
-> TargetPriority 제공

ACEnemy
-> ITargetContextProvider 미구현
-> 최종 target priority 산출 불가
```

따라서 최종 target 선택 단계에는 피아식별에 가까운 필터가 존재한다.

다만 `OnTargetPerceptionUpdated()` 단계에서는 감지된 Actor가 먼저 `TargetDataMap`에 들어간다.

```text
Perception에 Actor 감지
-> TargetDataMap에 추가
-> UpdateTargetDataMap에서 ITargetContextProvider 확인
-> provider가 없으면 최종 target 후보로 쓰지 않음
```

이 구조에서는 최종 target이 아닌 Actor도 다음 비용을 만들 수 있다.

```text
Perception delegate 호출
TargetDataMap 삽입 / 유지
TargetDataMap 순회
provider 없는 Actor 확인 후 continue
```

따라서 측정해야 하는 문제는 두 가지다.

```text
1. 감지 후보 누수
2. 유효 target 인식 지연
```

---

## 가능한 원인

### 1. Perception 후보 누수

같은 Enemy가 최종 target은 아니더라도 Perception 후보로 들어오면 후보 처리 비용이 증가한다.

확인할 항목:

```text
RawPerceptionActorCount
ValidTargetProviderCount
InvalidTargetProviderCount
TargetDataMapSize
```

### 2. Perception system batch 처리

대량 AI가 동시에 Sight stimulus를 처리하면 감지 이벤트가 특정 프레임대에 몰릴 수 있다.

확인할 항목:

```text
FirstRawPerceptionTime
FirstRawPerceptionFrame
```

### 3. BT Service interval 동기화

Perception은 빨리 들어왔지만 `UCBTService_UpdateAIContext`가 같은 interval에서 Blackboard를 갱신하면 다수 AI가 동시에 반응할 수 있다.

확인할 항목:

```text
FirstValidTargetTime
FirstBlackboardTargetTime
FirstBlackboardTargetFrame
```

### 4. Engage subsystem rebuild interval

Engage request는 먼저 들어왔지만 `UCWorldSubsystem_CombatEngage::RebuildAssignments()`가 interval 기반으로 동작하므로 역할 배정이 같은 시점에 묶일 수 있다.

확인할 항목:

```text
FirstEngageRequestTime
FirstEngageAssignmentTime
FirstEngageAssignmentFrame
```

---

## 측정 단계

### Step 1 - Controller Local Audit

목표:

```text
Perception 후보 누수와 첫 유효 target 인식 지연을 확인한다.
```

측정 요소:

```text
RuntimeStartTime
FirstRawPerceptionTime
FirstValidTargetTime
RawPerceptionActorCount
ValidTargetProviderCount
InvalidTargetProviderCount
TargetDataMapSize
MaxTargetDataMapSize
```

측정 위치:

```text
ACAIController::InitializeControllerRuntime
ACAIController::OnTargetPerceptionUpdated
ACAIController::UpdateTargetDataMap
ACAIController::SelectTopPriority
```

확인 가능한 내용:

```text
Perception 후보에 target이 아닌 Actor가 얼마나 섞이는지
Raw perception은 빠른데 valid target 인정이 늦는지
TargetDataMap 순회 비용이 후보 누수로 커지는지
```

---

### Step 2 - Blackboard / Engage Latency Audit

목표:

```text
Perception 이후 Blackboard 반영, Engage request, Engage assignment 지연을 분리한다.
```

측정 요소:

```text
FirstBlackboardTargetTime
FirstEngageRequestTime
FirstEngageAssignmentTime
```

측정 위치:

```text
ACAIController::RecordPerceptionContextBuiltForAudit
ACAIController::RecordBlackboardTargetSetForAudit
ACAIController::RecordEngageRequestSubmittedForAudit
ACAIController::RecordEngageAssignmentResolvedForAudit
UCBTService_UpdateAIContext::UpdatePerceptionContext
UCBTService_UpdateAIContext::ComputeEngageAssignmentContext
```

확인 가능한 내용:

```text
Valid target은 빠른데 Blackboard 반영이 늦는지
Blackboard 반영은 빠른데 Engage request가 늦는지
Engage request는 빠른데 assignment가 interval에 묶이는지
```

---

### Step 3 - Batch / Synchronization Audit

목표:

```text
다수 Enemy가 동시에 움직이는 현상이 같은 frame / service tick / rebuild interval에 의해 발생하는지 확인한다.
```

측정 요소:

```text
RuntimeStartFrame
FirstRawPerceptionFrame
FirstValidTargetFrame
FirstBlackboardTargetFrame
FirstEngageRequestFrame
FirstEngageAssignmentFrame
```

측정 위치:

```text
각 Time 측정 위치와 동일
GFrameCounter를 함께 기록
```

확인 가능한 내용:

```text
raw perception 이벤트가 같은 frame에 몰리는지
Blackboard target 반영이 BT service interval에 몰리는지
Engage assignment가 CombatEngage rebuild interval에 몰리는지
```

---

### Step 4 - Aggregate Summary

목표:

```text
개별 AI 로그 대신 전체 결과를 요약해서 성능 측정 오염을 줄인다.
```

요약 요소:

```text
TotalAI
AIWithRawPerception
AIWithValidTarget
AIWithBlackboardTarget
AIWithEngageRequest
AIWithEngageAssignment

Avg / P50 / P95 / Max FirstRawPerceptionLatency
Avg / P50 / P95 / Max FirstValidTargetLatency
Avg / P50 / P95 / Max FirstBlackboardTargetLatency
Avg / P50 / P95 / Max FirstEngageRequestLatency
Avg / P50 / P95 / Max FirstEngageAssignmentLatency

Avg / Max RawPerceptionActorCount
Avg / Max InvalidTargetProviderCount
Avg / Max TargetDataMapSize
```

구현 후보:

```text
1차: ACAIController에 개별 audit record 저장
2차: 필요 시 별도 WorldSubsystem으로 aggregate summary 승격
```

---

## 해결 방향 후보

### 후보 1 - Perception 후보 early reject

`OnTargetPerceptionUpdated()`에서 `ITargetContextProvider`가 없는 Actor를 `TargetDataMap`에 넣지 않는다.

효과:

```text
TargetDataMap 후보 누수 감소
provider 없는 Actor 순회 비용 감소
```

주의점:

```text
현재 target system이 ITargetContextProvider에 강하게 의존한다.
나중에 non-player target이 필요해지면 별도 target policy가 필요하다.
```

### 후보 2 - Team Affiliation baseline 도입

Player / Enemy team을 명시하고 Sight affiliation 설정이 의도대로 동작하게 만든다.

효과:

```text
Perception 후보 자체에서 friendly / non-target Actor를 줄인다.
UE Perception 설정과 gameplay target policy가 더 명확해진다.
```

주의점:

```text
GenericTeamId 또는 별도 team policy 설계가 필요하다.
Player / Enemy / neutral object / summon / ally 확장성을 함께 검토해야 한다.
```

### 후보 3 - Perception active budget

거리 / 중요도 / combat relevance 기준으로 Perception 활성 Enemy 수를 제한한다.

효과:

```text
대량 Enemy stress에서 Sight stimulus 처리량을 줄인다.
Runtime LOD 정책으로 연결하기 쉽다.
```

주의점:

```text
감지 지연과 gameplay 반응성이 직접 바뀐다.
전투 중 Enemy와 dormant Enemy의 기준을 나눠야 한다.
```

---

## 구현 순서 제안

```text
1. Perception Candidate Audit 계측 추가
2. 40 Enemy에서 후보 누수 / first valid target latency 확인
3. 필요 시 80 Enemy로 확장
4. early reject 또는 team affiliation baseline 중 우선 해결책 선택
5. Perception Gate Impact 측정으로 이동
6. 측정 결과에 따라 active budget / distance budget 후속 PR 여부 결정
```

---

## 종료 조건

이 Audit 작업은 다음 조건을 만족하면 종료한다.

```text
Raw perception 후보 수를 확인했다.
provider 없는 후보 수를 확인했다.
first valid target latency를 확인했다.
Blackboard / Engage로 이어지는 지연 위치를 분리할 수 있는 계측 지점을 정했다.
Perception Gate 측정 전에 해결해야 할 후보 누수 문제가 있는지 판단했다.
```
