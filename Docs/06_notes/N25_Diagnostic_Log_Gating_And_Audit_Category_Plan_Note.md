# N25. Diagnostic Log Gating And Audit Category Plan Note

## 목적

이 문서는 죽은 debug dump / 주석 trace 정리 이후, 실제 runtime에 살아 있는 diagnostic log를 어떤 기준으로 분류하고 구조화할지 정리한다.

책임 범위:

```text
이 문서는 구현 설계 문서다.
Diagnostic Hook / Debug Dump / CVar helper 구조, helper 파일 배치, CVar 카테고리, 구현 원칙을 정의한다.
정책의 원문 기준은 N23에서 관리하고, 파일별 inventory와 우선순위는 N24 / N26에서 관리한다.
```

관련 문서:

```text
N22_Debug_Log_Policy_Work_Plan_Note.md
N23_Debug_Log_And_Diagnostic_Code_Policy_Note.md
N24_Debug_Log_Cleanup_Inventory_Note.md
```

현재까지의 cleanup은 다음을 주로 처리했다.

```text
1. 호출처 없는 Print... debug dump 제거
2. 주석 처리된 FLog::Log trace 제거
3. 일부 수동 debug dump를 CVar 기반 verbose audit으로 연결
```

다음 단계는 active runtime log를 무작정 제거하는 것이 아니라, 실제 관측 가치가 있는 지점을 선별하고, 본문 호출부와 출력 정책을 분리하는 것이다.

---

## 문제 인식

프로젝트에는 combat, AI, feedback, action/reaction, movement, notify, asset validation 로그가 섞여 있다.

이 로그들은 모두 같은 성격이 아니다.

```text
1. 정상 정책 reject
2. 비정상 reject
3. asset / data configuration 오류
4. runtime invariant 위반
5. 시스템 경계 통과 기록
6. 성능 측정용 audit / counter
7. 임시 trace
```

정상 정책 reject까지 모두 출력하면 로그 노이즈가 커지고, Tick / overlap / notify / combat event 같은 고빈도 지점에서는 성능 측정도 왜곡될 수 있다.

반대로 모든 로그를 제거하면 combat result, combat signal, invalid notify, invalid asset configuration 같은 실제 문제 추적 경로가 사라진다.

따라서 다음 기준이 필요하다.

```text
1. 관측해야 하는 지점을 먼저 전수조사한다.
2. 정상 흐름과 비정상 흐름을 분리한다.
3. 호출부는 시그니처만 남긴다.
4. 출력 여부는 CVar로 제어한다.
5. Shipping 빌드에서는 debug 출력 구현이 포함되지 않게 한다.
```

---

## 기본 구조

본문 gameplay code에는 문자열 포맷팅과 `FLog::Log`를 직접 두지 않는다.

권장 호출부:

```cpp
FCombatResultDebug::PrintReceived(this, InCombatResultPacket);
FCombatResultDebug::PrintParryStack(this, InCombatResultPacket, ParryResultCount, Threshold, bStaggerReady);
FCombatResultDebug::PrintStaggerRequest(this, InCombatResultPacket, bStarted);
```

권장 구현부:

```cpp
#if !UE_BUILD_SHIPPING

TAutoConsoleVariable<int32> CVarCombatResultAudit(
	TEXT("Portfolio.Debug.CombatResultAudit"),
	0,
	TEXT("Print combat result diagnostic logs. 0: disabled, 1: enabled."),
	ECVF_Default);

bool FCombatResultDebug::ShouldPrintAudit()
{
	return CVarCombatResultAudit.GetValueOnGameThread() != 0;
}

void FCombatResultDebug::PrintReceived(const AActor* InReceiver, const FCombatResultPacket& InPacket)
{
	if (!ShouldPrintAudit()) return;

	FLog::Log(...);
}

#else

bool FCombatResultDebug::ShouldPrintAudit()
{
	return false;
}

void FCombatResultDebug::PrintReceived(const AActor* InReceiver, const FCombatResultPacket& InPacket)
{
}

#endif
```

핵심은 `#if !UE_BUILD_SHIPPING`을 gameplay 본문에 흩뿌리지 않고 debug helper 구현 파일에 모으는 것이다.

---

## 전수조사 기준

전수조사는 구현보다 먼저 한다.

각 후보는 다음 표 기준으로 기록한다.

| 항목 | 설명 |
| --- | --- |
| 파일 | 후보 로그가 있는 파일 |
| 함수 | 후보 지점이 있는 함수 |
| 후보 이벤트 | 실패 / 거절 / 분기 / 경계 / 결과 |
| 현재 상태 | active log / commented trace / no log but 관측 후보 |
| 빈도 | low / medium / high |
| 정상 흐름 여부 | 정상 정책 reject인지, 비정상 diagnostic인지 |
| 권장 카테고리 | CombatResult / CombatSignal / Feedback / ActionReaction / AIState / Movement / Notify |
| 권장 처리 | 제거 / 메시지 보강 / CVar gate / helper 분리 / CSV counter |
| Shipping 정책 | 포함 / 제외 |

---

## 관측 후보 판단 기준

### 실패

관측 후보가 될 수 있다.

예:

```text
required reference 없음
asset / data key 없음
montage play 실패
delegate bind 실패
subsystem / component invalid
blackboard key contract 위반
```

단, 고빈도 fallback에서 매번 발생할 수 있는 실패는 기본 출력 금지다.

### 거절

모든 reject를 출력하지 않는다.

```text
정상 정책 reject: 기본 로그 금지
비정상 reject: diagnostic 후보
반복 가능성이 높은 reject: CVar audit 후보
```

예:

```text
CanReserveChain false: 정상 정책 reject일 가능성이 높음
Invalid action data key: diagnostic 후보
Required component invalid: diagnostic 후보
```

### 분기

gameplay outcome을 바꾸는 분기만 후보로 둔다.

예:

```text
CombatSignal accepted / rejected
DefenseOutcome 결정
Parry stack threshold 도달
Engage / Alert assignment 확정
```

단순 `if invalid return`은 보통 로그 후보가 아니다.

### 경계

시스템 간 데이터가 넘어가는 경계는 audit 후보가 될 수 있다.

예:

```text
Notify -> Action handler
Action -> CombatSignal
CombatSignal -> Reaction
AI Perception -> Blackboard
Blackboard -> EngageSubsystem
Feedback request -> VFX/SFX/Trail
```

---

## CVar 카테고리

처음부터 로그마다 CVar를 만들지 않는다.

기능 경계 기준으로 카테고리 단위를 둔다.

### CombatResult

```text
Portfolio.Debug.CombatResultAudit
```

대상:

```text
CEnemy
CPlayer
Combat result packet 수신
Parry stack
Stagger request
```

용도:

```text
전투 결과 이벤트 흐름 관측
parry / stagger 정책 검증
```

### CombatSignal

```text
Portfolio.Debug.CombatSignalAudit
```

대상:

```text
CCombatSignalSourceComponent
CCombatSignalTargetComponent
CombatSignal cue
hit window
damage signal route
signal accept / reject
```

용도:

```text
전투 판정 경계와 거절 이유 관측
```

### Feedback

```text
Portfolio.Debug.FeedbackAudit
```

대상:

```text
CActionFeedbackComponent
CReactionFeedbackComponent
CHitFeedbackComponent
CWorldSubsystem_CombatFeedback
```

용도:

```text
VFX / SFX / Trail / HitStop / CameraShake 요청과 실패 관측
```

### Execution / Action / Reaction

```text
Portfolio.Debug.ExecutionOrchestratorDump
Portfolio.Debug.ActionRequestAudit
Portfolio.Debug.ReactionRequestAudit
Portfolio.Debug.ActionComponentAudit
Portfolio.Debug.ActionComponentDump
Portfolio.Debug.ReactionComponentAudit
Portfolio.Debug.ReactionComponentDump
```

대상:

```text
CActionComponent
CReactionComponent
CAction
CReaction
CActionOrchestratorComponent
CReactionOrchestratorComponent
```

용도:

```text
action / reaction data lookup
montage interruption
execution state 불일치
```

### AIState

```text
Portfolio.Debug.AIStateAudit
```

대상:

```text
CAIController
BT Service / Task
Blackboard
Perception
Engage context
Intent state
```

주의:

```text
기존 RuntimeLOD / Profiling CVar와 중복되지 않게 정리해야 한다.
PerceptionCandidateAudit, BlackboardEngageLatencyAudit은 profiling audit 성격이다.
```

### Movement

```text
Portfolio.Debug.MovementAudit
```

대상:

```text
CMovementComponent
movement gait
movement intent
rotation policy
invalid gait map
```

용도:

```text
movement 설정 오류와 상태 전환 경계 관측
```

### Notify

```text
Portfolio.Debug.NotifyAudit
```

대상:

```text
AnimNotify
AnimNotifyState
invalid trigger type
notify cue dispatch 실패
```

용도:

```text
animation asset / notify 설정 오류 관측
```

---

## Helper 파일 배치

권장 위치:

```text
Source/Portfolio/Core/Debug/
```

초기 후보:

```text
FCombatResultDebug.h/.cpp
FCombatSignalDebug.h/.cpp
FFeedbackDebug.h/.cpp
FExecutionOrchestratorDebug.h/.cpp
FAIStateDebug.h/.cpp
FMovementDebug.h/.cpp
FNotifyDebug.h/.cpp
```

파일이 너무 잘게 나뉘면 관리 비용이 생기므로, 초기에는 실제 적용 카테고리부터 만든다.

첫 적용 helper는 `FCombatSignalDebug`다.

`FCombatSignalDebug`는 파일을 Source / Target으로 나누지 않고, 하나의 helper 안에서 다음 섹션으로 구분한다.

```text
1. Gate
2. Source Diagnostic Hook
3. Source Debug Dump
4. Target Diagnostic Hook
5. Target Debug Dump
6. Shared Dispatch Diagnostic Hook
```

---

## 구현 원칙

### 본문 호출부

본문은 debug helper API만 호출한다.

```cpp
FCombatSignalDebug::RecordSourceRejectedForAudit(Context);
```

본문에 다음을 직접 두지 않는다.

```text
FLog::Log(...)
FString::Printf(...)
#if !UE_BUILD_SHIPPING
TAutoConsoleVariable
```

### Debug helper

Debug helper는 다음을 담당한다.

```text
1. CVar 선언
2. ShouldPrint... 조건
3. 메시지 포맷팅
4. FLog::Log 호출
5. Shipping 빌드 제외
```

### CombatSignal helper

현재 적용된 CombatSignal helper:

```text
Source/Portfolio/Core/Debug/FCombatSignalDebug.h
Source/Portfolio/Core/Debug/FCombatSignalDebug.cpp
```

CVar:

```text
Portfolio.Debug.CombatSignalAudit
-> source/target reject, accepted result, timing cue, combat result dispatch 요약 출력

Portfolio.Debug.CombatSignalDump
-> source context, target packet 상세 dump 출력
```

### Shipping 빌드

Shipping에서는 debug CVar를 등록하지 않는다.

Shipping 구현은 no-op으로 둔다.

```cpp
#if UE_BUILD_SHIPPING
void FCombatResultDebug::PrintReceived(...) {}
#endif
```

### CSV와 구분

CVar debug log는 Output Log 관측용이다.

CSV 계측은 별도다.

```text
FLog::Log: Output Log
TAutoConsoleVariable: 런타임 스위치
CSV_CUSTOM_STAT_GLOBAL: CSV counter
CSV_SCOPED_TIMING_STAT_GLOBAL: CSV timing
```

---

## 적용 순서

### 1단계. 전수조사

프로젝트 전체를 파일군별로 스캔한다.

```text
Combat
Action / Reaction
Feedback
AI / BT / Blackboard
Movement
Notify
Character
Input
Asset validation
```

결과는 `N24_Debug_Log_Cleanup_Inventory_Note.md` 또는 별도 inventory 표에 기록한다.

### 2단계. CombatSignal helper 적용

hit overlap부터 target result dispatch까지 조용히 drop될 수 있는 경계가 많아 첫 적용 대상으로 조정했다.

대상:

```text
CCombatSignalSourceComponent.cpp
CCombatSignalTargetComponent.cpp
```

처리:

```text
FCombatSignalDebug helper 생성
Portfolio.Debug.CombatSignalAudit CVar 추가
Portfolio.Debug.CombatSignalDump CVar 추가
본문 FLog::Log / FString::Printf를 helper 호출로 대체
Shipping no-op 구현
```

### 3단계. CombatResult helper 적용

CombatSignal 이후 source/receiver가 결과를 받는 경계다.

대상:

```text
CEnemy::ReceiveCombatResultPacket
CEnemy::HandleParryCombatResult
CEnemy::TryRequestParryStaggerReaction
CPlayer::ReceiveCombatResultPacket
CPlayer::HandleParryCombatResult
CPlayer::TryRequestParryStaggerReaction
```

처리:

```text
FCombatResultDebug helper 생성 후보
Portfolio.Debug.CombatResultAudit CVar 추가 후보
본문 FLog::Log를 helper 호출로 대체
```

### 4단계. Invalid diagnostic 보강

대상:

```text
Invalid HitVFX / HitSFX
invalid execution state
invalid gait map
invalid notify trigger action/reaction type
```

처리:

```text
삭제보다 메시지 context 보강 우선
필요하면 Debug helper 또는 category CVar에 연결
```

### 5단계. 문서 갱신

```text
N23: 정책 확정 내용 반영
N24: 전수조사 / 처리 결과 반영
N25: 적용 결과와 후속 계획 갱신
```

---

## 결론

권장 방향은 다음과 같다.

```text
1. 전수조사로 관측 후보를 찾는다.
2. 후보를 기능 카테고리 CVar에 매핑한다.
3. 구현은 카테고리 단위로 점진 적용한다.
4. 본문은 debug helper 시그니처만 호출한다.
5. debug helper 구현부에서 CVar + !UE_BUILD_SHIPPING으로 출력 정책을 통제한다.
```

이 방식은 본문 코드 품질, runtime log 제어, shipping 빌드 안전성, 이후 디버깅 확장성을 동시에 만족한다.
