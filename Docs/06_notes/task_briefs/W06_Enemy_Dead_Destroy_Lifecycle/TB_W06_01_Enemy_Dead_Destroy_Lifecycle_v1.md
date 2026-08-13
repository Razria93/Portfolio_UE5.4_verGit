# TB W06-01 Enemy Dead / Destroy Lifecycle v1

## 1. 작업 상태

```yaml
Branch: feature/dead-actor-destroy-flow
Base: main / 7a0463b9
Runtime C++: 완료
Life-State Migration: 완료
Editor Asset Integration: 완료
Character / Weapon Dissolve: 완료
PIE Integration: 완료
Debug Observability: 완료
Documentation Sync: 완료
Development Build: 성공
Push / PR / Merge: 미수행
```

현재 구조와 정책의 단일 기준은 [S31 Enemy Dead / Presentation / Destroy 생명주기 설계](../../../05_System_Architecture/S31_UE5_Portfolio_System_Architecture.md)다. 이 문서는 구현 범위, 자산 변경, 검증 결과와 커밋 이력을 기록한다.

---

## 2. 작업 목적

- Health의 `Alive -> Dead` 판정과 Enemy Actor 종료를 하나의 설명 가능한 흐름으로 연결한다.
- DeadIn Reaction과 DeadLoop Locomotion을 분리한다.
- Character와 SkeletalMesh Weapon에 Dissolve Presentation을 적용한다.
- 표현 자원의 자연 완료 이후에만 정상 Destroy한다.
- 표현 미구현 또는 시작 실패 시 DeadLoop 잔존 시간을 보장한 뒤 안전하게 Destroy한다.
- Destroy와 EndPlay cleanup의 책임을 분리한다.
- W05에서 이관한 Targeting Actor 수명 종료 경계를 실제 Destroy로 검증한다.
- 정상 상태와 계약 위반을 Debug Overlay에서 관찰한다.

Player Destroy, Respawn, Revive, Ragdoll, Pooling과 Execution 협업 사망은 범위에서 제외한다.

---

## 3. 최종 구조

### 3.1 정상 흐름

```text
Damage Commit
-> HealthComponent: Alive -> Dead
-> OnDeadStateChanged(Alive, Dead)

AnimInstance
-> bIsDead = true
-> AnimBP DeadLoop base pose 준비

Enemy
-> Death Lifecycle 시작
-> 신규 실행 의도 차단
-> AI movement / deferred action / weapon hit runtime 정리

Reaction System
-> DeadIn 요청과 Intervention
-> DeadIn Started
-> FullBody Montage
-> Complete Reaction Notify
-> DeadIn Completed

Enemy
-> Presentation fallback delay 선예약
-> CharacterFeedback에 Presentation 요청

Blueprint
-> 필수 Character Niagara 생성 성공
-> NotifyDeathPresentationStarted
-> Character / Weapon Dissolve
-> Character Niagara OnSystemFinished
-> NotifyDeathPresentationFinished

Enemy
-> RequestFinalizeDeath(PresentationCompleted)
-> 다음 Tick FinalizeDeath
-> gameplay cleanup
-> Destroy

Actor / Component
-> EndPlay teardown
```

### 3.2 Fallback 합류

```text
DeadIn Rejected / 미시작
-> BeginDeathPresentation(DeadInStartFailed)

DeadIn formal Interrupted / Ignored
-> BeginDeathPresentation(DeadInInterrupted)

Presentation listener 없음 / request 실패 / 필수 Niagara 생성 실패
-> fallback delay 유지
-> RequestFinalizeDeath(PresentationFallbackExpired)
```

정상 Presentation이 Started를 통지하면 fallback delay를 해제한다. 활성 Niagara에는 시간 제한을 두지 않으며, 잔여 파티클이 자연 수명을 모두 소비한 `OnSystemFinished`가 정상 종료 권한을 가진다.

---

## 4. C++ 구현 기록

### Health와 상태

```text
Type/CHealthTypes.h
- EDeadState를 Alive / Dead로 축소
- 기존 UAsset 직렬화 호환을 위해 Alive = 0, Dead = 2 유지

Component/CHealthComponent.*
- HP 0과 TryKill에서 Dead Commit
- OnDeadStateChanged(previous, current) 발행
- Dying / Reviving 및 Revive API 제거

Component/CStateComponent.*
- Dead 실행 상태 투영 제거
- DeadIn 동안 Life State == Dead, Execution State == Reaction 허용

Component/CMovementComponent.*
- HealthComponent::IsAlive() 최종 이동 gate
```

### DeadIn과 실행 생명주기

```text
Reaction/CReaction_Dead.*
- 기존 Reaction Orchestrator / Intervention 재사용

Notify/CAnimNotify_CompleteReaction.*
- Reaction 정상 종료를 명시적으로 요청

Action / Reaction MontageEnded
- non-interrupted: Complete Notify 누락 fallback
- interrupted: 정규 Stop을 대신하지 않고 계약 위반 Audit
- Montage / Play Serial로 stale callback 차단
```

### Enemy 조정과 Destroy

```text
Character/Enemy/CEnemy.*
- Health와 Reaction lifecycle 이벤트 구독
- BeginDeathLifecycle
- BeginDeathPresentation
- Presentation fallback delay
- 멱등 RequestFinalizeDeath / FinalizeDeath
- Destroy 직전 CleanupDeathGameplayRuntime
- 다음 Tick Destroy로 callback 재진입 회피
- EndPlay에서 timer / delegate teardown
```

### Presentation 프로토콜

```text
Type/CCharacterFeedbackTypes.h
- Reason / RuntimeState / EventType / FinalizeReason

Component/CCharacterFeedbackComponent.*
- BP OnDeathPresentationRequested
- NotifyDeathPresentationStarted
- NotifyDeathPresentationUnavailable
- NotifyDeathPresentationFinished
- 단일 native OnDeathPresentationEvent
```

BP Callable은 표현 결과를 입력하는 API이고 native event는 그 결과를 Enemy에 반환하는 출력이다. Feedback Component는 Actor Destroy 권한을 가지지 않는다.

### Weapon 참여

```text
Component/CWeaponComponent.*
- StartWeaponDissolve
- SetWeaponDissolveAmount
- FinishWeaponDissolve

Weapon/CWeaponActor.*
- Blueprint 구현 이벤트로 Material 표현 위임
```

Weapon은 Character와 같은 Timeline 값을 받지만 독립 완료 barrier 또는 Destroy 권한을 가지지 않는다.

### 제거한 레거시

```text
- EDeadState::Dying / Reviving
- EExecutionState::Dead
- EnterDeadState / EnterAliveState Health Notify
- FinalizeEnemyDeath Notify
- StartRevive / WaitDeadState BT Task
- TryRevive / TryCancelRevive / CanRevive / TryStartRevive
- Dying / Reviving AnimBP 분기
```

---

## 5. 에디터 및 자산 변경 기록

### Character

- `Dying` Animation을 `Dead_In`으로 전환했다.
- `M_Dead_In` FullBody Montage를 Dead ReactionData에 연결했다.
- DeadIn Montage에 `Complete Reaction` Notify를 배치했다.
- `Dead` Animation을 DeadLoop Locomotion으로 사용한다.
- AnimBP는 `bIsDead`로 Alive base pose와 DeadLoop base pose를 전환한다.
- 기존 Dying / Reviving 상태 머신과 Health / Finalize Notify 참조를 제거했다.

### Feedback Blueprint

`BP_CEnemy`의 CharacterFeedback 요청 이벤트에서 다음을 조정한다.

```text
OnDeathPresentationRequested
-> Character Niagara spawn

성공
-> NotifyDeathPresentationStarted
-> Character / Weapon Dissolve Timeline
-> Niagara OnSystemFinished
-> NotifyDeathPresentationFinished

실패 또는 미구현
-> NotifyDeathPresentationUnavailable
```

### Character / Weapon Dissolve

- Character와 Weapon용 Dissolve Material 및 Material Instance를 추가했다.
- Character SkeletalMesh와 Sword SkeletalMesh에 Dissolve 자산을 연결했다.
- Weapon Actor Blueprint가 Character Timeline의 Start / Amount / Finish를 반영한다.
- Character Niagara가 정상 완료 권한을 가지며 Weapon은 동기 표현 참여자로 유지한다.

### AI 자산

- Dead BT는 `ClearFocus + StayDead`만 유지한다.
- HitReact BT는 Reaction 종료만 관찰한다.
- Profiling용 AI / AnimBP에도 동일한 Alive / Dead 계약을 반영했다.

---

## 6. Debug Observability

```text
Enemy / CharacterFeedback
-> FDeathLifecycleDebug
-> Debug Overlay Snapshot / EventLog
```

Enemy Focus의 `[Death Lifecycle]` 블록은 다음을 표시한다.

```text
Health State
Lifecycle
DeadIn
Presentation
Fallback Timer
Finalization
```

정상 전이는 `Death` EventLog에 기록한다. 계약 위반은 Overlay에 항상 남기며 다음 CVar가 활성화된 경우 Output Log에도 복제한다.

```text
Portfolio.Debug.DeathLifecycleAudit
- Default: 1
- Debug Overlay Editor > Diagnostic Logging에서 제어
```

---

## 7. Targeting 통합 결과

Targeting은 Enemy 사망 정책을 직접 참조하지 않는다.

```text
Enemy Destroy
-> Actor OnEndPlay
-> TargetingComponent HandleCurrentTargetEndPlay
-> OnTargetChanged(PreviousTarget, nullptr)
```

현재 Target은 Actor instance의 weak index / serial identity로 검증해, 전환 전에 구독했던 이전 Target의 늦은 callback이 새 Target을 해제하지 않도록 한다.

---

## 8. 검증 결과

### Build

```text
PortfolioEditor Win64 Development: 성공
git diff --check: 성공
```

### PIE

```text
PASS - Health Alive -> Dead 단일 전이
PASS - DeadIn FullBody Reaction과 DeadLoop 자연 연결
PASS - Character / Weapon Dissolve
PASS - Character Niagara 완료 후 다음 Tick Destroy
PASS - Presentation 미구현 / 생성 실패 fallback
PASS - 신규 AI / Action / Movement 의도 차단
PASS - Weapon collision / hit runtime cleanup
PASS - 현재 Target Destroy 단일 해제
PASS - 사망 선행 해제 후 Destroy 중복 event 없음
PASS - 이전 Target Destroy가 새 Target을 해제하지 않음
PASS - Marker / Lock Assist / Debug Focus 정리
PASS - Death Overlay 상태와 EventLog Filter
```

---

## 9. 커밋 기록

```text
c74b7c33 docs(dead): add enemy destroy lifecycle plan
4582dbe4 feat(dead): add enemy death finalization lifecycle
eafcf2d8 docs(dead): record enemy finalization runtime contract
0bd640e2 feat(reaction): add explicit reaction completion notify
72e45573 refactor(dead): finalize enemy death lifecycle
7e707f9d assets(dead): configure dead-in reaction and locomotion
c88f8ea4 docs(dead): align enemy death lifecycle contract
63951d08 feat(dead): coordinate dissolve presentation finalization
de6b043d assets(dead): configure character and weapon dissolve presentation
814b3cac feat(debug): expose death lifecycle diagnostics
6a05a7c5 docs(dead): update presentation and diagnostics contract
```

본 P0 문서 동기화 변경은 아직 커밋하지 않는다.

---

## 10. 남은 머지 절차

1. 본 문서 동기화 diff 검토 및 커밋
2. 원격 `main` 최신화 여부 확인
3. 필요 시 최종 Development Build / 핵심 PIE 회귀
4. Branch Push와 Draft PR
5. 리뷰 대응과 CI 확인
6. 사용자 일반 Merge

---

## 11. 제외 범위와 후속

```text
- Player Destroy / Respawn / Game Over
- Revive
- Ragdoll / Actor Pooling / 영구 시체 정책
- Execution 협업 사망
- Active Niagara 무한 재생을 Timer로 강제 종료하는 정책
- 범용 Actor Death Lifecycle Framework
```

Action / Reaction 전환의 범용 cleanup 순서는 [N14](../../N14_Dead_Destroy_And_Execution_Cleanup_Followup_Note.md)의 남은 후속 안건에서 다룬다.

---

## 12. 관련 문서

- [S02 인지 / 입력 / 상태변환 / 액션 실행](../../../05_System_Architecture/S02_UE5_Portfolio_System_Architecture.md)
- [S09 Combat Feedback 계층](../../../05_System_Architecture/S09_UE5_Portfolio_System_Architecture.md)
- [S26 실행 몽타주 생명주기](../../../05_System_Architecture/S26_UE5_Portfolio_System_Architecture.md)
- [S31 Enemy Dead / Presentation / Destroy](../../../05_System_Architecture/S31_UE5_Portfolio_System_Architecture.md)
- [N14 Dead Destroy / Execution Cleanup Follow-up](../../N14_Dead_Destroy_And_Execution_Cleanup_Followup_Note.md)
- [W05 Player Targeting](../W05_Player_Targeting/README.md)
