# N13. Component Lifecycle Cleanup Policy Note

## 목적

이 문서는 `Source/Portfolio`의 actor / component lifecycle cleanup 기준을 정리하기 위한 작업 노트다.

P29에서는 character-owned component reference 주입과 복구 기준을 정리했고, P30에서는 runtime lookup을 DI 대상과 구분했다. 다음 단계에서는 `BeginPlay` / `EndPlay` / delegate / timer / spawned actor / runtime cache가 어떤 기준으로 초기화되고 해제되어야 하는지 정리한다.

이번 작업은 teardown 시점의 누락 가능성을 줄이고, 코드 리뷰에서 설명 가능한 cleanup 정책을 만드는 데 초점을 둔다.

---

## 기본 원칙

```text
BeginPlay에서 bind / cache / runtime setup을 수행했다면
-> EndPlay 또는 대응 lifecycle에서 unbind / clear / teardown을 검토한다.

OnPossess에서 controller-owned runtime state를 구성했다면
-> OnUnPossess에서 controller-owned runtime state를 정리한다.

spawned actor를 owner component가 생성했다면
-> owner component lifecycle에서 runtime state 정리 후 Destroy 책임을 가진다.

timer handle을 저장하거나 actor state를 임시 변경했다면
-> timer 종료 전 world teardown / owner invalid 상황을 고려한다.
```

---

## Naming 기준

이번 작업에서는 `Initialize`를 모든 setup helper에 일괄 적용하지 않는다.

상위 lifecycle 조합 함수는 `Initialize / Uninitialize`를 사용하고, 하위 helper는 실제 동작의 반대말이 자연스럽게 보이도록 구분한다.

```text
Runtime lifecycle 조합
-> InitializeXXXRuntime
-> UninitializeXXXRuntime

map / container 구성과 비움
-> BuildXXXMap
-> ClearXXXMap

초기 상태 설정과 기본 상태 복구
-> SetInitialXXXState
-> ResetXXXState

delegate / event 연결과 해제
-> BindXXXEvents
-> UnbindXXXEvents

logic 시작과 중단
-> StartXXXRuntime
-> StopXXXRuntime

spawned actor 생성과 파괴
-> CreateXXX
-> DestroyXXX
```

이 기준을 적용하면 `Initialize`의 반대편에 `Clear`, `Reset`, `Unbind`, `Stop`, `Destroy`가 섞여 보이는 문제가 줄어든다.

예시:

```text
InitializeControllerRuntime
-> SetPossessionRuntimeState
-> BindPerceptionEvents
-> SetInitialBlackboardRuntimeValues
-> StartBehaviorTreeRuntime

UninitializeControllerRuntime
-> StopBehaviorTreeRuntime
-> ClearBlackboardRuntimeValues
-> UnbindPerceptionEvents
-> ResetPossessionRuntimeState
```

---

## Cleanup 분류

## Dead Destroy Flow와의 관계

이번 브랜치는 Dead 이후 Actor Destroy 프로세스의 선행 기반으로 lifecycle cleanup hook을 정리한다.

```text
이번 브랜치
-> BeginPlay / EndPlay 대칭성 확인
-> OnPossess / OnUnPossess 대칭성 확인
-> NativeInitializeAnimation / NativeUninitializeAnimation 대칭성 확인
-> delegate / timer / spawned actor / runtime cache cleanup hook 확인

후속 Dead Destroy 작업
-> Dead state 진입 이후 Destroy 시점 결정
-> death reaction / feedback 완료 시점 결정
-> AI stop / possession 해제 / collision 비활성화 정책 결정
-> weapon / combat signal / hit window 종료 순서 결정
-> Destroy 호출 이후 EndPlay teardown 검증
```

즉 P31은 Destroy가 발생했을 때 호출될 teardown hook과 cleanup 책임을 명확히 하는 준비 작업이다.

Dead 이후 실제 Destroy 프로세스와 실행 중 상태 전환 cleanup 순서 정책은 별도 후속 노트에서 추적한다.

관련 후속 노트:

```text
Docs/06_notes/N14_Dead_Destroy_And_Execution_Cleanup_Followup_Note.md
```

### 1. Gameplay Runtime Cleanup

전투 행동이 정상 종료되거나 중단될 때 임시 gameplay state를 정리하는 흐름이다.

예시:

```text
Action / Reaction ClearRuntime
Action / Reaction CleanupRuntimeEffects
WeaponComponent ClearWeaponRuntimeState
FeedbackComponent ClearRuntimeFeedback
DefenseComponent ClearGuardState
```

이 경로는 action / reaction interrupt 구조 안에서 이미 동작하고 있다.

overlay, feedback window, weapon collision, hit context, result-out 같은 상태가 늘어나면 다음 기준을 더 명확히 해야 한다.

```text
Before
-> 어떤 action / reaction / overlay / weapon context가 active였는가

Transition
-> 어떤 원인으로 종료 또는 중단됐는가
-> 어떤 상태를 먼저 snapshot하는가
-> 어떤 상태를 닫고 어떤 상태를 유지하는가
-> 외부 결과 dispatch는 cleanup 전후 어느 시점에 수행하는가

After
-> 최종 execution state는 무엇인가
-> overlay / weapon / feedback / runtime cache가 의도한 상태로 정리됐는가
```

이번 브랜치에서는 object lifecycle teardown 기준을 우선 정리한다.
gameplay runtime cleanup의 순서 정책은 ResultOut / Repulse처럼 실제 결과 전달 사례가 생긴 뒤 후속 작업에서 다룬다.

### 2. Lifecycle Teardown Cleanup

객체가 world에서 종료되거나 owner 관계가 끊기는 시점에 dangling delegate, timer, spawned actor, stale cache를 정리하는 흐름이다.

예시:

```text
Actor EndPlay
ActorComponent EndPlay
AnimInstance NativeUninitializeAnimation
AIController OnUnPossess
WorldSubsystem Deinitialize
```

이번 작업의 주 검토 대상이다.

---

## 준비 단계 조회 결과

```text
Lifecycle hook 보유 파일: 11개
Delegate / Timer 사용 파일: 8개
SpawnActor / NewObject 생성 경로: 3개
Runtime cleanup 명명 사용처: 약 20개+
```

---

## 우선 검토 대상

### ACAIController

현재 구조:

```text
OnPossess
-> SetPossessionRuntimeState
-> ClearTargetDataMap
-> BindPerceptionEvents
-> SetupBlackboardComponent
-> SetInitialBlackboardRuntimeValues
-> StartBehaviorTreeRuntime

OnUnPossess
-> StopBehaviorTreeRuntime
-> ClearBlackboardRuntimeValues
-> UnbindPerceptionEvents
-> ClearTargetDataMap
-> ResetPossessionRuntimeState
```

검토 포인트:

```text
- perception delegate unbind 필요 여부
- TargetDataMap cleanup 필요 여부
- OnPossess가 재호출될 때 중복 AddDynamic 가능성
- BeginPlay가 비어 있는 경우 유지 또는 제거 여부
```

### UCWorldSubsystem_CombatFeedback

현재 구조:

```text
ApplyHitStop
-> actor별 CustomTimeDilation 저장
-> timer 등록

RestoreHitStop
-> actor가 유효하면 dilation 복구
-> handle / cached dilation 제거
```

검토 포인트:

```text
- subsystem teardown 시 ActiveHitStopMap 정리 필요 여부
- world cleanup 중 timer callback이 호출되지 않는 경우 dilation restore 보장 여부
- invalid actor key가 남았을 때 map cleanup 기준
```

### UCActionComponent / UCReactionComponent

현재 구조:

```text
BeginPlay
-> BuildActionRuntimeMaps / BuildReactionRuntimeMaps
-> SetInitialActiveActionRuntimeState / SetInitialActiveReactionRuntimeState

EndPlay
-> ResetActiveActionRuntimeState / ResetActiveReactionRuntimeState
-> ClearActionRuntimeMaps / ClearReactionRuntimeMaps

NewObject
-> action / reaction executor 생성
```

검토 포인트:

```text
- executor object ownership은 component outer 기준으로 유지 가능한지
- EndPlay에서 active context clear가 필요한지
- gameplay runtime cleanup과 teardown cleanup을 섞지 않아야 하는 지점
```

이 영역은 gameplay execution과 연결되므로, 먼저 lifecycle 책임을 문서화하고 수정 필요 여부를 별도로 판단한다.

---

## 이번 브랜치 포함 범위

```text
- lifecycle cleanup 정책 문서화
- delegate / timer / spawned actor 사용처 전수 목록화
- ACAIController cleanup 보강 여부 판단 및 수정 필요 항목 반영
- UCWorldSubsystem_CombatFeedback teardown 보강 여부 판단 및 수정 필요 항목 반영
- Action / Reaction executor lifecycle은 검토 결과를 문서화하고, execution 영향도가 크면 후속 분리
```

---

## 제외 범위

```text
- Action / Reaction 실행 종료 정책 재설계
- montage stop 정책 변경
- Guard / Reaction runtime cleanup 재설계
- 모든 _Injected reference를 EndPlay에서 일괄 null 처리
- Blink / Repulse / ResultOut 구현
```

---

## 검증 기준

```text
rg -n "BeginPlay|EndPlay|NativeUninitializeAnimation|OnUnPossess" Source/Portfolio -g "*.cpp" -g "*.h"
rg -n "AddDynamic|AddUniqueDynamic|AddUObject|RemoveDynamic|RemoveAll|SetTimer|ClearTimer" Source/Portfolio -g "*.cpp" -g "*.h"
rg -n "SpawnActor|Destroy\\(|NewObject" Source/Portfolio -g "*.cpp" -g "*.h"
git diff --check
PortfolioEditor Win64 Development build
PIE basic combat loop smoke test
```

---
