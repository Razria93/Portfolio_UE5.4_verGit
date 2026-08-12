# Enemy Dead / Presentation / Destroy 생명주기 설계

## 1. 문서 목적과 상태

이 문서는 Enemy의 사망 판정, DeadIn Reaction, DeadLoop Locomotion, Dissolve Presentation, 최종 gameplay cleanup과 Actor `Destroy()`의 런타임 계약을 정의한다.

구현과 코드 리뷰에서는 이 문서를 Enemy Dead / Destroy 구조의 기준으로 사용한다. W06 Task Brief는 구현 절차와 검증 기록을 담당하고, 본 문서는 최신 확정 정책과 책임 경계를 담당한다.

```yaml
Status: Runtime C++ Implemented / Life-State Asset Migration Completed / Presentation Integration And PIE Pending
Scope: Enemy Runtime
Player Destroy: Out of Scope
Ragdoll / Pooling / Respawn: Out of Scope
```

이 개정은 이전의 다음 계약을 폐기한다.

```text
Alive -> Dying -> Dead
Dead Montage 후반 Finalize Notify
-> 즉시 Destroy
```

최신 계약은 생명 상태와 애니메이션 구간을 분리한다.

```text
Life State
- Alive
- Dead

Death Animation / Presentation
- DeadIn: Reaction Montage
- DeadLoop: AnimBP Locomotion
- Dissolve: Feedback Presentation
- Destroy: Enemy Lifecycle Finalization
```

---

## 2. 핵심 원칙

### 2.1 생명 상태와 실행 상태를 분리한다

`Dead`는 캐릭터의 생명 상태다. `Idle`, `Action`, `Reaction`은 현재 실행 상태다. 서로 다른 축이므로 `Dead`를 Action / Reaction 실행 상태처럼 취급하지 않는다.

```text
Life State: Alive | Dead
Execution State: Idle | Action | Reaction | ...
```

DeadIn Reaction이 실행되는 동안에도 다음 두 상태는 동시에 참일 수 있어야 한다.

```text
Life State == Dead
Execution State == Reaction
```

DeadIn이 종료되면 Execution State는 정규 Reaction 계약에 따라 종료되고, AnimBP의 DeadLoop가 자연스럽게 노출된다.

### 2.2 HealthComponent가 생명 상태의 단일 원본이다

CombatSignalTargetComponent는 Damage Packet을 해석하고 Health 변경을 요청한다. 최종 HP Commit과 `Alive -> Dead` 판정은 HealthComponent가 소유한다.

```text
CombatSignalTargetComponent
-> Damage 해석
-> HealthComponent HP Commit
-> HealthComponent Alive -> Dead 판정
-> OnDeadStateChanged 발행
```

AnimBP, Enemy, Targeting, Action / Reaction gate는 HealthComponent의 생명 상태 또는 그 이벤트를 관찰한다. 별도의 경쟁하는 Dead 원본을 만들지 않는다.

### 2.3 DeadIn, DeadLoop, Dissolve의 책임을 분리한다

```text
DeadIn
- Reaction System이 실행
- 사망 진입 동작 표현

DeadLoop
- AnimBP Locomotion이 표현
- bIsDead presentation cache로 선택

Dissolve / Material Overlay / FX / Sound
- Feedback Component가 표현

Destroy
- ACEnemy Death Lifecycle이 최종 결정
```

### 2.4 정상 Destroy 시점은 Dissolve 완료 이벤트가 결정한다

정상 경로에서 고정 Destroy Timer로 Dissolve 시간을 추측하지 않는다.

```text
Dissolve Finished
-> Enemy::RequestFinalizeDeath(PresentationCompleted)
-> 다음 Tick FinalizeDeath()
-> Destroy()
```

Timer는 정상 연출 시간을 결정하는 수단이 아니라, 완료 이벤트 누락으로 Actor가 영구 잔류하는 것을 막는 Watchdog으로만 사용한다.

---

## 3. 책임 경계

### UCHealthComponent

```text
- HP 저장과 변경
- Alive / Dead 생명 상태 판정
- OnDeadStateChanged 발행
```

`Dying`, `Reviving`, `EnterDeadState`, `EnterAliveState`와 기존 Health Notify 호환층은 제거한다. 부활 기능이 필요해지면 별도 Respawn / Revive 정책에서 다시 설계한다.

### ACEnemy

```text
- OnDeadStateChanged 구독
- Death Lifecycle 시작
- 신규 AI / Action / Movement 의도 차단 연결
- DeadIn Reaction lifecycle 관찰
- Death Presentation 시작 요청
- DeadIn / Presentation Watchdog 소유
- 멱등적인 Finalize 요청과 최종 gameplay cleanup
- Actor Destroy
```

Enemy는 Material Parameter, Timeline, Niagara, Sound의 구체적인 표현을 알지 않는다.

### UCReactionOrchestratorComponent / UCReactionComponent / UCReaction_Dead

```text
- DeadIn Reaction 후보와 Context 구성
- 기존 실행과 Intervention 해결
- DeadIn Started / Completed / Interrupted / Ignored 발행
- 명시적인 Complete / Stop 계약 유지
```

엔진 측 `OnMontageEnd(bInterrupted == true)`는 명시적 Stop을 대신하지 않는다. 기존 S26 계약대로 Audit만 기록한다.

### UCAnimInstance / AnimBP

```text
- HealthComponent의 Life State 이벤트 구독
- 초기화 시 현재 Life State snapshot 적용
- bIsDead presentation cache 유지
- bIsDead == true일 때 DeadLoop Locomotion 선택
```

`bIsDead`는 애니메이션 표현을 위한 캐시이며 생명 상태의 새로운 권한이 아니다.

### Feedback Component

```text
- Dissolve
- Material Overlay
- Death FX / Sound
- Death Presentation 완료 이벤트
- 예상 Presentation 시간 제공
```

기존 Action / Reaction Feedback 구조를 우선 재사용한다. 정확한 클래스 배치는 기존 S09 / S17 책임과 실제 코드를 감사한 뒤 결정하되, Feedback Component가 Actor `Destroy()`를 직접 호출하지 않는 원칙은 고정한다.

### EndPlay

```text
- delegate 구독 해제
- timer 해제
- component별 runtime teardown
- controller / spawned actor teardown
```

EndPlay은 gameplay 연출을 시작하지 않는다. 월드 종료, 레벨 제거, 외부 Destroy에서는 Dissolve를 새로 시작하지 않고 teardown만 수행한다.

---

## 4. 정상 생명주기

```text
Damage Commit
-> Health Alive -> Dead
-> OnDeadStateChanged(Alive, Dead)

ACEnemy
-> Death Lifecycle 시작
-> 신규 실행 의도 차단
-> AI movement / deferred action / weapon hit runtime 즉시 정리

UCAnimInstance
-> bIsDead = true
-> AnimBP base pose가 DeadLoop로 전환

Reaction System
-> DeadIn Reaction 요청
-> 기존 Action / Reaction과 Intervention
-> DeadIn Started
-> DeadIn Montage 재생
-> DeadIn Completed

ACEnemy
-> BeginDeathPresentation(Normal)
-> Feedback Component에 Dissolve 요청
-> Presentation Watchdog 예약

Feedback Component
-> Dissolve / Overlay / FX / Sound 재생
-> OnDeathPresentationFinished

ACEnemy
-> Presentation Watchdog 해제
-> RequestFinalizeDeath(PresentationCompleted)
-> 다음 Tick FinalizeDeath()
-> 최종 gameplay cleanup
-> Destroy()

EndPlay
-> teardown
```

Health가 먼저 Dead가 되므로 DeadIn Montage가 시작되기 전부터 DeadLoop가 기반 Pose로 준비된다. FullBody DeadIn Montage가 그 위를 덮고, Montage가 완료되면 별도의 상태 강제 전환 없이 DeadLoop가 노출된다.

---

## 5. Death Presentation 계약

### 5.1 시작 결과

Feedback 진입점은 최소한 다음 정보를 Enemy에 반환해야 한다.

```cpp
struct FDeathPresentationStartResult
{
    bool bStarted = false;
    float ExpectedDuration = 0.f;
};
```

정확한 타입과 API 이름은 구현 시 기존 Feedback 패턴에 맞추되 의미는 유지한다.

```text
bStarted == true
-> ExpectedDuration + SafetyMargin으로 Watchdog 예약

bStarted == false
-> Audit
-> RequestFinalizeDeath(PresentationStartFailed)
```

### 5.2 정상 완료

Timeline, Niagara 또는 Feedback 실행기가 실제 완료 시점을 알고 완료 이벤트를 발행한다.

```text
Timeline Finished
또는 Niagara Finished
또는 Feedback Sequence Completed
-> OnDeathPresentationFinished
-> Watchdog 해제
-> RequestFinalizeDeath(PresentationCompleted)
```

### 5.3 Presentation Watchdog

Watchdog은 정상 Destroy Timer가 아니다. 완료 이벤트 누락에 대한 최종 안전장치다.

```text
WatchdogDuration = ExpectedDuration + SafetyMargin
```

Watchdog 값이 예상 Dissolve 시간보다 짧아 연출 도중 Actor를 제거하지 않도록 다음 하한을 보장한다.

```cpp
WatchdogDuration = FMath::Max(
    ConfiguredWatchdogDuration,
    ExpectedDuration + SafetyMargin);
```

```text
Presentation Finished
-> Watchdog 취소

Presentation Finished 누락
-> Watchdog 만료
-> Audit: DeathPresentationTimedOut
-> RequestFinalizeDeath(PresentationTimedOut)
```

`RequestFinalizeDeath()`와 `FinalizeDeath()`는 경합 또는 중복 callback에도 한 번만 파괴하도록 멱등성을 유지한다. 다만 멱등성은 정상적으로 두 번 호출하기 위한 구조가 아니라 마지막 방어 장치다.

---

## 6. DeadIn Fallback 계약

### 6.1 DeadIn 요청 실패 또는 미시작

```text
DeadIn Rejected / 요청 누락 / 시작 검증 실패
-> Audit
-> BeginDeathPresentation(DeadInStartFailed)
```

Life State는 이미 Dead이고 DeadLoop가 표시되고 있으므로, DeadIn이 실패해도 가능한 경우 Dissolve Presentation은 유지한다.

### 6.2 정규 Interrupted / Ignored

```text
DeadIn formal Interrupted / Ignored
-> Audit
-> BeginDeathPresentation(DeadInInterrupted)
```

여기서 `Interrupted`는 Orchestrator / Component의 명시적 Stop 경로가 발행한 정규 lifecycle 결과만 의미한다. 예상하지 못한 `OnMontageEnd(bInterrupted)`를 자동 변환하지 않는다.

### 6.3 Started 후 종결 이벤트 계약

DeadIn이 Started를 발행한 이후의 종료는 Reaction 시스템의 명시적 `Completed`,
formal `Interrupted`, `Ignored`가 소유한다. Enemy는 몽타주 예상 길이로 정상 종료를
추측하거나 별도 DeadIn Timer로 Reaction 생명주기를 우회하지 않는다.

```text
DeadIn Started
-> Reaction 정규 lifecycle을 신뢰

Completed
-> BeginDeathPresentation(DeadInCompleted)

formal Interrupted / Ignored
-> BeginDeathPresentation(DeadInInterrupted)
```

Started 후 어떠한 종결 이벤트도 오지 않는다면 이는 Dead 전용 fallback 대상이 아니라
Reaction lifecycle 계약 위반이다. 해당 문제는 Reaction 실행 상태까지 함께 복구할 수 있는
공통 실행 계층에서 Audit하고 수정한다.

### 6.4 Fallback 합류

```text
DeadIn 정상 Completed
DeadIn 시작 실패
DeadIn formal Interrupted / Ignored
-> BeginDeathPresentation(Reason)

Presentation 정상 완료
Presentation 시작 실패
Presentation 완료 누락 Watchdog
-> RequestFinalizeDeath(Reason)
-> 다음 Tick FinalizeDeath()
```

`BeginDeathPresentation()`도 여러 fallback이 경합해 중복 Dissolve를 시작하지 않도록 멱등이어야 한다.

---

## 7. Finalize와 Destroy 계약

### 7.1 공개 진입점

Feedback 또는 Blueprint는 Enemy의 공개 API만 호출한다.

```text
NotifyDeathPresentationFinished()
또는 RequestFinalizeDeath(Reason)
```

Feedback / Blueprint가 개별 gameplay Component를 정리하거나 `Destroy()`를 직접 호출하지 않는다.

### 7.2 다음 Tick Finalize

Presentation 완료 Delegate, Timeline 또는 Reaction callback stack 안에서 즉시 Destroy하지 않는다.

```text
RequestFinalizeDeath()
-> bDeathFinalizationRequested 설정
-> 다음 Tick FinalizeDeath() 예약
```

```text
FinalizeDeath()
-> 생명 상태와 lifecycle 검증
-> bDeathFinalized를 cleanup보다 먼저 설정
-> 모든 Death Watchdog 해제
-> CleanupDeathGameplayRuntime()
-> Destroy()
```

### 7.3 Destroy 직전 Gameplay Cleanup

```text
- AI path movement 정지
- deferred action 제거
- Weapon collision / trail / hit context 정리
- 신규 gameplay request가 재개되지 않도록 death gate 유지
```

Enemy는 각 Component의 내부 EndPlay teardown이나 Weapon Actor 파괴를 중복 소유하지 않는다.

---

## 8. Targeting 통합

TargetingComponent는 Enemy의 사망 정책을 직접 알지 않는다.

```text
Enemy FinalizeDeath
-> Destroy()
-> Actor OnEndPlay
-> UCTargetingComponent Target EndPlay callback
-> OnTargetChanged(PreviousTarget, nullptr)
-> Lock Assist / Marker / Debug Focus 정리
```

Enemy가 TargetingComponent를 직접 호출하지 않는다. 사망 판정에 의한 조기 Target 해제 정책은 Targeting의 기존 Health 유효성 검사로 유지하고, 실제 Actor 수명 종료는 OnEndPlay 계약으로 처리한다.

---

## 9. 제거 및 전환 대상

최신 설계를 구현할 때 다음 기존 구조를 전수조사 후 제거 또는 교체한다.

```text
- EDeadState의 Dying / Reviving 단계
- EnterDeadState / EnterAliveState 기반 상태 확정
- CAnimNotify_EnterDeadState / CAnimNotify_EnterAliveState
- AnimBP의 Dying / Reviving 상태 머신과 enum blend
- StateComponent의 Dead 실행 상태 투영
- Dead Montage 후반 Finalize Notify가 직접 Destroy 시점을 소유하는 구조
```

다음 구조는 유지한다.

```text
- Health의 Alive / Dead 권한
- 기존 Action / Reaction Orchestrator와 Intervention
- Reaction Started / Completed / formal Interrupted / Ignored
- S26 Complete / Stop / MontageEnd 계약
- Enemy의 멱등 Finalize 경로
- Targeting의 OnEndPlay 계약
```

---

## 10. 에디터 및 자산 작업

```text
DeadIn
- Dead ReactionData에 DeadIn Montage 연결
- 기존 FullBody Reaction Slot 사용
- 명시적 Complete 계약 유지

DeadLoop
- AnimBP에서 bIsDead 기반 Locomotion 분기
- DeadIn Montage 아래에서 항상 준비된 Pose로 사용

Dissolve
- Feedback Blueprint / Component에서 Timeline 또는 정규 Feedback 실행기 구성
- 완료 시 Enemy 공개 완료 API 호출
- ExpectedDuration을 C++ Watchdog 계산에 제공
```

기존 `Enter Dead State`, `Enter Alive State`, `Finalize Enemy Death` Notify는 최신 정상 흐름에 사용하지 않는다. 실제 제거는 참조 전수조사와 자산 마이그레이션을 마친 뒤 수행한다.

---

## 11. 검증 기준

### 정상 경로

```text
- HP 0에서 Alive -> Dead가 한 번 발생
- AnimBP bIsDead가 즉시 true
- DeadLoop가 기반 Pose로 준비됨
- DeadIn Reaction이 재생됨
- DeadIn Completed 후 Dissolve가 시작됨
- Dissolve Finished에서 Watchdog이 해제됨
- 다음 Tick Finalize 후 Destroy됨
- 정상 경로에서 Destroy가 한 번만 호출됨
```

### DeadIn Fallback

```text
- DeadIn Reject / 미시작에서도 DeadLoop가 유지됨
- 가능한 경우 Dissolve fallback이 실행됨
- formal Interrupted / Ignored가 같은 Presentation 경로로 합류함
- Started 후 종결 이벤트가 누락되면 Reaction lifecycle 계약 위반으로 추적됨
- unexpected MontageEnd interrupt는 lifecycle 종료로 자동 변환되지 않음
```

### Presentation Fallback

```text
- Presentation 시작 실패 시 Audit 후 Finalize됨
- Dissolve 완료 이벤트 누락 시 ExpectedDuration + Margin 이후 Finalize됨
- Watchdog이 정상 Dissolve보다 먼저 Actor를 제거하지 않음
- 정상 Finished와 Watchdog 경합에도 Destroy가 한 번만 실행됨
```

### Runtime / Targeting

```text
- Dead 이후 AI / Action / Movement 신규 의도가 실행되지 않음
- Weapon collision / hit window가 남지 않음
- EndPlay에서 delegate / timer가 안전하게 정리됨
- 현재 Target Enemy Destroy 시 Target이 정확히 한 번 해제됨
- 이전 Target Destroy가 새 Target을 해제하지 않음
```

---

## 12. 제외 범위와 후속 확장

```text
- Player Destroy / Respawn / Game Over
- Revive
- Ragdoll
- Actor Pooling
- Corpse persistence 정책
- Execution 협업 사망
- 범용 Actor Death Lifecycle Framework
```

Pooling이나 영구 시체를 도입할 경우 `FinalizeDeath()`의 최종 결과를 즉시 `Destroy()`가 아닌 제거 정책으로 확장한다. 그 전까지 Enemy의 명시적 Destroy가 확정 정책이다.

---

## 13. 관련 문서

```text
S09 - Combat Feedback 구조
S17 - Damage Feedback과 Reaction Feedback 책임
S26 - 실행 Montage 생명주기 계약
W06-01 - Enemy Dead / Destroy 작업 및 검증 기록
N14 - Dead Destroy와 Execution Cleanup 후속 노트
```

---

## 14. 2026-08-12 런타임 구현 기록

### 구현된 책임

```text
HealthComponent
- 런타임에서 Alive / Dead만 발생
- HP 0 또는 TryKill에서 즉시 Dead 커밋
- 직렬화 호환을 위해 Alive = 0, Dead = 2 값을 명시적으로 보존

StateComponent
- Life State와 Execution State 동기화 제거
- DeadIn 동안 ExecutionState == Reaction 허용
- Action / Reaction 종료 시 ExecutionState를 Idle로 정상 정리

MovementComponent
- HealthComponent::IsAlive()를 최종 이동 입력 gate로 사용

AnimInstance
- Health OnDeadStateChanged 구독
- bIsDead를 즉시 갱신
- AnimBP에는 bIsDead만 노출하며 DeadState enum 캐시는 두지 않음

Enemy
- Completed / formal Interrupted / Ignored / 시작 실패를
  멱등 BeginDeathPresentation으로 합류
- Feedback 완료 또는 Presentation Watchdog에서 다음 Tick FinalizeDeath
- 최종 gameplay cleanup 후 Destroy

CharacterFeedbackComponent
- Death Presentation 시작 요청을 Blueprint에 전달
- ExpectedDuration 제공
- Blueprint 완료 통지를 native delegate로 Enemy에 반환
- Actor Destroy 권한은 소유하지 않음
```

### Watchdog 계산

```cpp
PresentationWatchdog = Max(
    DeathPresentationWatchdogMinimumDuration,
    ExpectedDuration + DeathPresentationWatchdogSafetyMargin);
```

DeadIn에는 Watchdog을 두지 않는다. Reaction의 명시적 Complete / Stop 계약이 실행
생명주기를 소유한다. Presentation Timer만 외부 Blueprint/Material 연출의 완료 통지
누락을 복구하며, 정상 경로에서는 `NotifyDeathPresentationFinished()`가 이를 해제한다.

### 자산 마이그레이션 완료 상태

```text
EDeadState::Dying / Reviving
- enum에서 제거
- 기존 Dead 직렬화 값 2는 명시적으로 보존

EExecutionState::Dead
- enum에서 제거
- 사망 판정은 FExecutionSnapshot.bIsDead와 HealthComponent가 소유
- Idle / Action / Reaction의 기존 직렬화 값과 Max = 4를 보존

EnterDeadState / EnterAliveState 및 기존 Health Notify 클래스
- UAsset 참조 0건 확인 후 제거

CAnimNotify_FinalizeEnemyDeath
- UAsset 참조 0건 확인 후 제거
- DeadIn Completed가 Death Presentation 시작 권한을 소유

Revive API / BT Task
- TryRevive / TryCancelRevive / CanRevive / TryStartRevive 제거
- CBTTask_StartRevive / CBTTask_WaitDeadState 제거

Enemy Death Lifecycle abort
- Alive 복귀에 의한 취소 경로는 제거
- 불변조건 위반 시 내부 Timer와 Feedback을 정리하는 AbortDeathLifecycle만 유지
```

### 완료된 에디터 마이그레이션

```text
1. AnimBP는 bIsDead로 Alive / DeadLoop base pose를 선택
2. Dead ReactionData는 DeadIn FullBody Montage 사용
3. BT Dead 분기는 ClearFocus + StayDead만 유지
4. HitReact BT는 기존 Reaction 종료만 관찰
5. 기존 Dying / Reviving AnimBP 분기와 레거시 Notify 참조 제거
```

실제 Dissolve Timeline과 `NotifyDeathPresentationFinished()` 연결은 Presentation 자산 작업으로 남으며, 제거된 생명 상태 호환층과는 별개다.
