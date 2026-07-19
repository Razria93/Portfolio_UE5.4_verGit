# N23. Debug Log And Diagnostic Code Policy Note

## 목적

이 문서는 프로젝트에서 디버그 로그, 진단 코드, profiling audit, 임시 trace를 작성하고 관리하는 기준을 정리한다.

이번 기준은 `refactor/debug-log-policy-v1` 브랜치의 코드 정리 판단 근거로 사용하며, 이후 새 debug log를 추가할 때도 동일하게 적용한다.

목표는 다음과 같다.

```text
1. Shipping / Release 성격의 배포 빌드에 debug dump와 임시 trace가 포함되지 않게 한다.
2. Runtime 본문에 불필요한 debug 조건문과 출력 코드를 남기지 않는다.
3. 유지 가치가 있는 debug 지점은 명시적 API로 분리한다.
4. Error / Warning / Debug / Audit / Temporary trace를 구분한다.
5. 로그 메시지와 API naming을 일관되게 만든다.
```

---

## 용어

### Error

복구할 수 없거나, 기능을 계속 수행하면 잘못된 결과가 발생할 수 있는 상태다.

예:

```text
필수 component / asset 누락
필수 Blackboard key 누락
생성해야 하는 actor 생성 실패
runtime contract 위반
```

### Warning

fallback은 가능하지만 설정이나 데이터가 의도와 어긋난 상태다.

예:

```text
중복 key overwrite
중복 feedback match
optional data 누락
fallback route 사용
```

### Debug Dump

사람이 상태를 확인하기 위한 상세 출력이다.

예:

```text
TargetDataMap dump
CombatSignal context dump
Hit / Overlap context dump
Action / Reaction data dump
Feedback request dump
```

### Diagnostic Hook

본문 흐름에 남겨도 되는 진단 API 호출 지점이다.

예:

```cpp
RecordCombatSignalRejectedForDebug(Context, Reason);
RecordEngageAssignmentResolvedForAudit(TargetActor);
```

Diagnostic Hook은 본문에서 디버그 의도를 드러내되, 실제 출력 조건 / CVar / build guard / formatting은 내부로 숨긴다.

### Profiling Audit

성능 측정, 비용 분리, 카운트 검증을 위한 계측이다.

예:

```text
CSV counter
assignment rebuild summary
perception candidate audit
animation refresh counter
combat collision / feedback counter
```

### Temporary Trace

특정 문제를 확인하기 위해 임시로 넣은 출력이다.

예:

```text
[Index Done]
[Investigate Time out]
[Some Function Enter]
```

Temporary Trace는 커밋 전에 제거하는 것을 원칙으로 한다.

### Commented-out Debug Code

주석 처리된 로그 또는 debug 함수 호출이다.

예:

```cpp
// FLog::Log(TEXT("..."));
// PrintTargetData();
```

주석 처리된 debug code는 원칙적으로 남기지 않는다. 유지 가치가 있으면 Diagnostic Hook으로 승격하고, 가치가 없으면 제거한다.

### Visual Debug

화면 표시, debug draw, overlay, debug panel 같은 시각적 진단 도구다.

Visual Debug는 기능 또는 도구 성격이 강하므로 일반 log cleanup 브랜치에서 구현하지 않는다.

---

## 기본 원칙

### 1. Debug / Audit / Temporary trace는 Shipping에서 제외한다

Debug Dump, Profiling Audit, Temporary Trace는 원칙적으로 Shipping 빌드에 포함하지 않는다.

권장 형태:

```cpp
#if !UE_BUILD_SHIPPING
if (ShouldPrintCombatSignalDebug())
{
    FLog::Log(...);
}
#endif
```

다만 이 패턴은 본문에 직접 반복하지 않는다. 가능하면 debug API 내부에 숨긴다.

### 2. Runtime 본문은 gameplay 흐름을 우선한다

비권장:

```cpp
#if !UE_BUILD_SHIPPING
if (CVarDebug.GetValueOnGameThread() != 0)
{
    FLog::Log(...);
}
#endif

DoGameplayLogic();
```

권장:

```cpp
RecordCombatSignalRejectedForDebug(Context, Reason);

DoGameplayLogic();
```

본문에는 의미 있는 진단 API 호출만 남기고, 실제 출력 조건은 API 내부로 이동한다.

### 3. Error / Warning은 제거 대상이 아니다

Error / Warning은 debug dump와 다르다. 잘못된 상태를 알려주는 방어선이므로 제거하지 않는다.

단, 다음 기준은 적용한다.

```text
1. 메시지에 domain / event / reason이 드러나야 한다.
2. 반복 출력 가능성이 있으면 once / rate limit / validation 단계 이동을 검토한다.
3. 단순 debug dump를 Warning처럼 출력하지 않는다.
```

### 4. 주석 처리된 로그는 남기지 않는다

비권장:

```cpp
// FLog::Log(TEXT("[CombatSignal] Rejected"));
// PrintCombatSignalContext(Context);
```

대체:

```cpp
RecordCombatSignalRejectedForDebug(Context, Reason);
```

또는 제거한다.

### 5. Profiling Audit은 일반 Debug와 분리한다

Profiling Audit은 사람에게 상태를 보여주는 Debug Dump가 아니라 측정값을 남기기 위한 진단 경로다.

정책:

```text
1. 기본 비활성
2. CVar로 명시적 활성화
3. Shipping 제외
4. 가능하면 summary / counter 중심
5. event 기반 값은 안정적인 tick / flush phase에서 CSV로 기록
```

---

## 처리 분류표

| 분류 | 처리 방향 | 본문 허용 | Shipping 포함 | 비고 |
| --- | --- | ---: | ---: | --- |
| Error | 유지 | Yes | 제한적 가능 | `ensureMsgf`, `UE_LOG(Error)`, safe return |
| Warning | 유지 / 제한 | Yes | 제한적 가능 | 반복 가능하면 once / validation 후보 |
| Debug Dump | Debug API로 분리 | Hook만 허용 | No | CVar / debug flag 필요 |
| Profiling Audit | Audit / Profiling API로 분리 | Hook만 허용 | No | CSV / CVar 기준 |
| Temporary Trace | 제거 | No | No | 커밋 전 제거 |
| Commented-out Debug Code | 제거 또는 Hook 승격 | No | No | 주석 로그 금지 |
| Visual Debug | 별도 feature 후보 | No | No | overlay / panel / draw debug |

---

## API 작성 패턴

### Debug Dump

권장 naming:

```text
ShouldPrint{Domain}{Feature}Debug()
Print{Domain}{Feature}Debug()
Record{Domain}{Event}ForDebug()
```

예:

```cpp
RecordCombatSignalRejectedForDebug(Context, Reason);
PrintTargetDataDebug(TargetData);
ShouldPrintPerceptionDebug();
```

### Profiling Audit

권장 naming:

```text
ShouldAudit{Domain}{Feature}()
Record{Domain}{Event}ForAudit()
Print{Domain}{Feature}AuditSummary()
Flush{Domain}{Feature}AuditToCsv()
```

예:

```cpp
RecordEngageRequestSubmittedForAudit(TargetActor);
PrintPerceptionCandidateAuditSummary();
FlushCombatFeedbackProfilingToCsv();
```

### Error / Warning Report

권장 naming:

```text
Report{Domain}{Event}Warning()
Report{Domain}{Event}Error()
Validate{Domain}{Contract}()
```

예:

```cpp
ReportDuplicateActionExecutorKeyWarning(ActionExecutorKey);
ValidateRequiredBlackboardKeys(BlackboardAsset);
```

Error / Warning은 반드시 별도 API로 빼야 하는 것은 아니다. 단발 validation 또는 초기화 실패처럼 본문에서 바로 읽히는 편이 더 명확하면 본문에 유지할 수 있다.

---

## CVar naming 규칙

기존 Runtime LOD / profiling CVar는 유지한다.

기존 예:

```text
Portfolio.AI.RuntimeLOD.PerceptionCandidateAudit
Portfolio.AI.RuntimeLOD.BlackboardEngageLatencyAudit
Portfolio.AI.RuntimeLOD.EngageAssignmentAudit
Portfolio.AI.RuntimeLOD.EngageAssignmentVerboseAudit
```

신규 debug CVar가 필요할 경우 다음 형태를 우선한다.

```text
Portfolio.Debug.<Domain>.<Feature>
Portfolio.Combat.Debug.<Feature>
Portfolio.AI.Debug.<Feature>
Portfolio.Feedback.Debug.<Feature>
```

신규 profiling CVar가 필요할 경우 기존 측정 체계와 맞춘다.

```text
Portfolio.AI.RuntimeLOD.<Feature>Audit
Portfolio.AI.RuntimeLOD.<Feature>Counter
Portfolio.Combat.Profiling.<Feature>
```

규칙:

```text
1. Debug CVar와 Runtime LOD 정책 CVar를 섞지 않는다.
2. 성능 정책을 바꾸는 CVar는 Debug namespace에 두지 않는다.
3. 단순 출력만 제어하는 CVar는 Debug namespace를 사용한다.
4. 측정 결과를 만드는 CVar는 Audit / Profiling 명칭을 사용한다.
```

---

## 로그 메시지 형식

권장 형식:

```text
[Domain|System|Event] Key=Value | Key=Value
```

예:

```text
[AI|EngageAssignment|Summary] RebuildId=13 | RequestSnapshot=80 | FinalEngage=2 | FinalAlert=6
[Combat|SignalSource|Rejected] Reason=InvalidTarget | Source=Enemy_01 | Target=Player
[Feedback|Action|DuplicateVFX] Key=Slash01 | Policy=Skip
```

비권장:

```text
[Index Done]
[Duplicate key] Overwrite Value
Invalid
```

메시지 기준:

```text
1. Domain이 드러나야 한다.
2. Event가 드러나야 한다.
3. Reason 또는 Policy가 있으면 함께 적는다.
4. Actor / Component / Asset 이름은 GetNameSafe 기준으로 출력한다.
5. 여러 줄 dump는 header / footer 형식을 통일한다.
```

---

## 주석 정책

### 금지

```cpp
// FLog::Log(...);
// PrintSomeDebugInfo();
// 필요하면 여기 로그 켜기
```

이런 주석은 죽은 코드에 가깝고, 이후 유지보수자가 실제로 유지해야 하는 진단 지점인지 판단하기 어렵다.

### 허용

```cpp
RecordCombatSignalRejectedForDebug(Context, Reason);
```

또는 코드 자체를 제거한다.

### 예외

다음 경우에는 짧은 주석을 허용한다.

```text
1. Debug hook이 왜 이 위치에 있어야 하는지 코드만으로 설명되지 않는 경우
2. Shipping 제외 guard가 특정 파일에서 선언 / 정의 단위로 필요한 이유가 있는 경우
3. Profiling audit이 측정 기준 문서와 연결되는 경우
```

단, 주석은 로그 본문을 대체하지 않는다.

---

## 이번 브랜치 적용 기준

`refactor/debug-log-policy-v1`에서는 다음 순서로 적용한다.

```text
1. Commented-out debug log 제거
2. Temporary trace 제거
3. 유지할 가치가 있는 dump 지점은 Debug API로 승격
4. Debug API 내부에 `#if !UE_BUILD_SHIPPING` + CVar / debug flag 적용
5. Profiling audit은 기존 Audit API / CVar 유지
6. Error / Warning 후보는 severity와 메시지 형식 정리
```

이번 브랜치에서 하지 않는 작업:

```text
1. Visual debug tool 구현
2. FLog 시스템 전면 교체
3. 모든 로그 category 재설계
4. 모든 Warning 제거
5. gameplay behavior 변경
```

---

## 리뷰 기준

PR 리뷰에서는 다음 질문으로 판단한다.

```text
1. 이 출력은 Error / Warning / Debug / Audit / Temporary trace 중 무엇인가?
2. Shipping에 포함되어도 되는가?
3. 본문에 직접 있어야 하는가, API 안으로 들어가야 하는가?
4. 주석 처리된 로그로 남아 있지는 않은가?
5. CVar / debug flag 없이 hot path에서 출력될 가능성이 있는가?
6. 메시지에서 domain / event / reason을 알 수 있는가?
7. profiling audit이 일반 debug log와 섞이지 않았는가?
```

이 기준을 통과하지 못하면 이번 브랜치에서 정리하거나, 후속 후보로 명시한다.
