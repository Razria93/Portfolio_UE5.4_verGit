# N22. Debug Log Policy Work Plan Note

## 목적

이 문서는 `refactor/debug-log-policy-v1` 작업의 기준과 진행 순서를 정리한다.

목표는 로그를 전부 제거하는 것이 아니다. 로그의 성격, 활성화 조건, 빌드 포함 여부, 출력 책임을 분리해서 다음 상태를 만든다.

디버그 로그 / 진단 코드 작성 규칙은 `N23_Debug_Log_And_Diagnostic_Code_Policy_Note.md`를 따른다. 이 문서는 N23의 정책을 이번 브랜치에 적용하기 위한 작업 계획이다.

```text
1. Shipping / Release 성격의 배포 빌드에 debug dump가 포함되지 않는다.
2. Development / DebugGame 빌드에서도 hot path debug log는 기본 비활성이다.
3. Error / Warning / Debug dump / Profiling audit / Temporary trace가 구분된다.
4. Combat / AI / Feedback 계열 로그는 필요한 경우 명시적 gate를 통해서만 출력된다.
5. 기능 동작은 변경하지 않는다.
```

---

## 문제의식

현재 프로젝트에는 `FLog::Log`, `UE_LOG`, `Print...Summary`, `Audit` 계열 로그가 Combat / AI / Feedback / Profiling 경로에 분산되어 있다.

이 로그들은 개발 중 문제를 찾는 데 유용했지만, 다음 리스크가 있다.

```text
1. Debug dump와 error log가 같은 출력 경로에 섞여 있다.
2. AI / Combat hot path에서 조건 없는 로그가 남으면 성능 측정과 실제 플레이를 왜곡할 수 있다.
3. Shipping 빌드 포함 여부가 코드에서 명확하지 않다.
4. 로그 prefix / 메시지 형식 / 책임 위치가 일관되지 않다.
5. 임시 trace와 유지해야 할 audit log의 구분이 흐려질 수 있다.
```

따라서 이번 작업은 "로그 제거"가 아니라 "로그 정책 정리"로 진행한다.

---

## 기본 원칙

### 1. Debug log는 배포 빌드에서 제외한다

Debug dump, profiling audit, 임시 trace는 원칙적으로 Shipping 빌드에 포함하지 않는다.

권장 형태:

```cpp
#if !UE_BUILD_SHIPPING
if (CVarSomeDebugLog.GetValueOnGameThread() != 0)
{
    FLog::Log(...);
}
#endif
```

이 구조는 다음 목적을 가진다.

```text
#if !UE_BUILD_SHIPPING
-> Shipping 빌드에서 코드 자체를 제외한다.

CVar / debug flag
-> Development / DebugGame 빌드에서도 필요할 때만 출력한다.
```

### 2. Error / Warning은 debug dump와 다르게 취급한다

설정 누락, asset contract 위반, 복구 불가능한 상태는 단순 debug log가 아니다.

```text
Error
-> 잘못된 상태이며 기능을 계속 수행할 수 없는 경우
-> ensureMsgf / UE_LOG(Error) / safe return과 함께 사용

Warning
-> fallback은 가능하지만 asset / 설정 / 계약이 어긋난 경우
-> spam 가능성이 낮아야 하며, 반복 출력은 gate 또는 once 정책 필요
```

### 3. Profiling audit은 debug log와 별도 취급한다

AI Runtime LOD / CSV profiling 과정에서 추가한 audit log는 일반 debug log가 아니라 측정 보조 정보다.

정책:

```text
1. 기본 비활성
2. CVar로 명시적으로 활성화
3. Shipping 제외
4. 가능하면 frame마다 직접 출력하지 않고 summary 중심으로 출력
5. CSV counter는 tick phase 또는 안정적인 flush 지점에서 기록
```

### 4. Temporary trace는 최종 코드에 남기지 않는다

일회성 확인용 로그는 정책화하지 않는다.

```text
Temporary trace
-> 원인 확인 후 제거
-> 남겨야 한다면 Debug dump 또는 Profiling audit로 승격하고 gate를 붙인다.
```

---

## 로그 분류 기준

| 분류 | 예시 | Shipping 포함 | 기본 활성 | 제어 방식 |
| --- | --- | ---: | ---: | --- |
| Error | 필수 asset / component 누락, 복구 불가 상태 | 제한적 가능 | On | `ensureMsgf`, `UE_LOG(Error)`, safe return |
| Warning | fallback 가능한 설정 문제 | 제한적 가능 | On 또는 제한 | `UE_LOG(Warning)`, once/gate 검토 |
| Debug Dump | 상태 summary, payload dump, decision dump | No | Off | `#if !UE_BUILD_SHIPPING` + CVar/debug flag |
| Profiling Audit | 측정용 count / summary / assignment audit | No | Off | `#if !UE_BUILD_SHIPPING` + profiling CVar |
| Temporary Trace | 원인 확인용 임시 출력 | No | Off | 최종 제거 |
| Visual Debug 후보 | 화면 overlay, debug panel, draw debug | No | Off | 별도 feature 후보 |

---

## 1차 작업 범위

이번 브랜치에서 우선 다루는 범위는 다음으로 제한한다.

```text
1. Source/Portfolio 안의 로그 사용처 inventory 작성
2. Combat / AI / Feedback 쪽 unconditional debug dump 분류
3. 이미 CVar가 있는 profiling audit은 해당 CVar 뒤로 정리
4. Shipping 제외가 필요한 debug dump에 build guard 적용
5. 임시 trace성 로그 제거 또는 후속 후보로 분류
6. 문서와 PR 기록 업데이트
```

이번 작업에서 바로 전면 변경하지 않는 범위:

```text
1. FLog 시스템 전체 교체
2. Visual debug UI / panel 구현
3. 모든 로그 category 재설계
4. 모든 Warning / Error 제거
5. gameplay behavior 변경
6. 성능 측정 재수행
```

---

## 작업 단계

### Step 1. Inventory

대상:

```text
FLog::Log
UE_LOG
GEngine->AddOnScreenDebugMessage
Print*Info
Print*Summary
*Audit*
```

산출물:

```text
1. 로그 사용처 목록
2. 분류: Error / Warning / Debug Dump / Profiling Audit / Temporary Trace
3. 우선 수정 대상 목록
4. 보류 대상 목록
```

판단 기준:

```text
hot path 여부
Shipping 포함 가능성
이미 CVar / debug flag가 있는지
에러성 로그인지 단순 dump인지
반복 출력 가능성이 있는지
```

### Step 1 결과. 1차 로그 사용처 스캔

`Source/Portfolio` 기준 1차 스캔 결과, `FLog::Log`, `UE_LOG`, `Print...`, `Audit` 계열은 Combat / AI / Feedback / Reaction 경로에 넓게 분포한다.

파일별 주요 사용처는 다음과 같다.

| 파일 | 대략 사용 수 | 주요 성격 |
| --- | ---: | --- |
| `Controller/CAIController.cpp` | 149 | Perception debug / profiling audit |
| `Component/CCombatSignalSourceComponent.cpp` | 74 | Combat signal context dump |
| `System/Combat/CWorldSubsystem_CombatEngage.cpp` | 71 | Assignment audit / summary |
| `Component/CReactionComponent.cpp` | 59 | Reaction data / executor dump |
| `Component/CCombatSignalTargetComponent.cpp` | 52 | Combat signal target dump |
| `Component/CHitFeedbackComponent.cpp` | 50 | Hit feedback request dump |
| `Weapon/CWeaponActor.cpp` | 42 | overlap / hit context dump |
| `Reaction/CReaction.cpp` | 40 | reaction runtime / participant dump |
| `Component/CActionFeedbackComponent.cpp` | 36 | action feedback dump |
| `Component/CHealthComponent.cpp` | 29 | damage / heal / dead context dump |
| `Component/CReactionFeedbackComponent.cpp` | 24 | reaction feedback dump |

이미 CVar 또는 profiling gate가 있는 영역은 다음과 같다.

```text
CAIController
-> Portfolio.AI.RuntimeLOD.PerceptionCandidateAudit
-> Portfolio.AI.RuntimeLOD.BlackboardEngageLatencyAudit

CWorldSubsystem_CombatEngage
-> Portfolio.AI.RuntimeLOD.EngageAssignmentAudit
-> Portfolio.AI.RuntimeLOD.EngageAssignmentVerboseAudit

CBTDecorator_CanMove
-> Portfolio.AI.RuntimeLOD.CanMoveDecoratorAudit

Runtime LOD / CSV counter 계열
-> CCombatCollisionProfilingCounters
-> CCombatFeedbackProfiling
-> CAIStateRuntimeLODPolicy
-> CAIAnimationRuntimeLODPolicy
```

이 영역은 이미 명시적 측정 / audit 제어가 있으므로, 1차 수정에서는 동작을 크게 바꾸지 않는다. 필요하면 Shipping 제외 guard와 naming / section 정리만 적용한다.

---

## 1차 수정 방향

### 1. 명백한 debug trace부터 정리한다

작고 안전한 우선 후보:

```text
AI/BehaviorTree/Task/CBTTask_AdvanceInvestigateIndex.cpp
-> [Index Done]

AI/BehaviorTree/Service/CBTService_UpdateInvestigateContext.cpp
-> [Investigate Time out]
```

이 로그들은 정상 상태 전환 확인용에 가깝다. Error / Warning이 아니므로 기본 출력 대상이 아니다.

정책:

```text
1. 제거하거나
2. debug CVar / build guard 뒤로 이동한다.
```

### 2. Action / Reaction runtime log는 성격을 나눈다

후보:

```text
Action/CAction.cpp
-> Unexpected montage interruption

Reaction/CReaction.cpp
-> Unexpected montage interruption
-> Stopped
-> Ignored
```

정책:

```text
Unexpected montage interruption
-> 비정상 가능성이 있으므로 Warning 성격인지 Debug 성격인지 판단한다.

Stopped / Ignored
-> 상태 흐름 dump에 가까우므로 기본 출력하지 않는다.
```

### 3. Data build / duplicate key log는 Warning 후보로 본다

후보:

```text
Component/CActionComponent.cpp
-> Duplicate key overwrite
-> BuildActionExecutorMap failed

Component/CReactionComponent.cpp
-> Duplicate key overwrite
-> BuildReactionExecutorMap failed

Component/CActionFeedbackComponent.cpp
-> Duplicate trail / VFX / SFX match

Component/CReactionFeedbackComponent.cpp
-> Duplicate VFX / SFX execution key
```

정책:

```text
1. asset / data contract 위반이면 Warning 또는 Error 성격으로 유지한다.
2. 정상 fallback 가능한 중복 skip이면 Debug dump로 낮추고 gate를 둔다.
3. 반복 출력 가능성이 있으면 once / build-time validation / future data validation 후보로 분리한다.
```

### 4. Print... 함수는 Shipping 제외 후보로 본다

대상:

```text
PrintCombatSignalSourceSummaryInfo
PrintCombatSignalTargetSummaryInfo
PrintActionFeedbackRequestInfo
PrintReactionFeedbackRequestInfo
PrintHitStopRequestInfo
PrintReactionInfoSummary
PrintTargetData
PrintEngageContext
PrintBeginOverlapContextInfo
PrintEndOverlapContextInfo
```

현재 대부분 호출은 주석 처리되어 있지만, 함수 정의 자체는 Shipping 빌드에도 포함될 수 있다.

정책:

```text
1. Debug dump 함수는 `#if !UE_BUILD_SHIPPING` 대상으로 본다.
2. 선언 / 정의 / 호출부 guard 중 어떤 방식이 가장 코드 오염이 적은지 파일별로 판단한다.
3. 호출이 풀렸을 때 hot path에서 반복 출력될 수 있는 함수는 CVar / debug flag도 함께 고려한다.
```

### 5. Profiling audit은 기존 CVar를 유지한다

대상:

```text
PerceptionCandidateAudit
BlackboardEngageLatencyAudit
EngageAssignmentAudit
EngageAssignmentVerboseAudit
CanMoveDecoratorAudit
StateRuntimeLODTierAudit
AnimationRefreshCounter
```

정책:

```text
1. 기존 CVar semantics를 바꾸지 않는다.
2. Shipping 제외 guard는 적용 후보로 본다.
3. 로그가 너무 많은 verbose audit은 summary 중심 유지 여부를 검토한다.
```

---

## 1차 적용 순서

```text
1. 가장 작은 AI debug trace 정리
   -> CBTTask_AdvanceInvestigateIndex
   -> CBTService_UpdateInvestigateContext

2. Action / Reaction runtime log 분류
   -> Unexpected interruption
   -> stop / ignored 흐름 로그

3. Action / Reaction component data build log 분류
   -> duplicate key
   -> executor map build failure

4. Feedback / Hit / Weapon / CombatSignal Print... dump 함수 guard 검토
   -> Shipping 제외
   -> 필요 시 CVar 후보 기록

5. AI / CombatEngage profiling audit guard 검토
   -> 기존 CVar 유지
   -> Shipping 제외
```

이 순서를 따르는 이유는 다음과 같다.

```text
1. 정상 동작 로그와 오류 로그를 먼저 분리해야 한다.
2. 작은 파일에서 정책을 검증한 뒤 넓은 Combat / AI dump 함수로 확장한다.
3. 이미 측정용 CVar가 있는 Runtime LOD / profiling audit은 마지막에 건드려야 측정 workflow 회귀를 줄일 수 있다.
```

### Step 2. Policy Helper / Guard 방식 결정

우선 과도한 새 시스템을 만들지 않는다.

1차 권장 방식:

```text
1. 이미 profiling CVar가 있는 audit log는 기존 CVar를 사용한다.
2. 새 CVar가 필요한 경우 도메인 단위로 좁게 추가한다.
3. Shipping 제외는 `#if !UE_BUILD_SHIPPING`를 사용한다.
4. 단순 함수 호출부가 읽기 어려워지면 local helper로 감싼다.
```

새 공용 로그 매니저는 이번 브랜치의 1차 목표가 아니다. 필요성이 확인되면 후속 브랜치로 분리한다.

### Step 3. Minimal Code Change

우선순위:

```text
1. AI profiling audit summary
2. Combat / Feedback debug dump
3. Reaction / Action summary dump
4. 임시 trace성 로그
```

적용 기준:

```text
Error / Warning
-> 의미가 명확하면 유지
-> 반복 출력 가능성이 있으면 once/gate 검토

Debug dump
-> Shipping 제외
-> 기본 비활성
-> CVar/debug flag 뒤로 이동

Profiling audit
-> Shipping 제외
-> profiling CVar 뒤로 이동
-> summary 중심 유지

Temporary trace
-> 제거
```

### Step 4. Verification

정적 확인:

```text
rg "FLog::Log|UE_LOG|AddOnScreenDebugMessage|Print.*Summary|Print.*Info|Audit" Source/Portfolio
git diff --check
```

빌드:

```text
PortfolioEditor Win64 Development
```

PIE smoke:

```text
1. 기본 전투 루프
2. AI Engage / Alert / Observe 상태
3. Combat Feedback 기본 출력
4. Debug CVar Off 상태에서 불필요한 로그가 출력되지 않는지
5. 필요한 CVar On 상태에서 audit summary가 출력되는지
```

---

## 완료 기준

```text
1. 로그 사용처가 정책 분류에 따라 정리되어 있다.
2. Debug dump / Profiling audit는 Shipping 빌드에서 제외된다.
3. Development / DebugGame에서도 debug dump는 기본 비활성이다.
4. Error / Warning 로그는 제거하지 않고 의미 기준으로 유지된다.
5. Combat / AI / Feedback의 대표 debug dump는 gate를 가진다.
6. Temporary trace는 제거되거나 후속 후보로 명시된다.
7. 기능 동작 변경이 없다.
8. git diff --check 통과.
9. Development 빌드와 기본 PIE smoke가 가능하다.
```

---

## 후속 후보

이번 브랜치에서 발견되더라도 바로 구현하지 않을 후보:

```text
1. Visual debug overlay / panel
2. Debug draw 정책
3. FLog category / verbosity 전면 재설계
4. Log once / rate limit helper
5. CSV profiling helper 통합
6. Editor-only debug commandlet 또는 debug menu
```

이 항목들은 기능 또는 도구 성격이 강하므로 별도 브랜치로 분리한다.
