# N05 Combat Signal Boundary Design Note

## 1. 목적

이 문서는 W03 이후 전투 처리 구조를 `CombatSignal` 기준으로 다시 정리한다.

기존 논의에서는 `Intent -> Request -> Resolution -> Routing -> Domain` 또는 공용 `GameplayIntentGateway / GameplayCoordinator`가 후보였다. 

그러나 현재 코드의 직접적인 문제는 공용 routing 계층 부재가 아니라, `ApplyDamageComponent`와 `TakeDamageComponent` 안에 전투 신호 생성 / 전달 / 수신 / 판정 / 적용 / 결과 통지가 압축되어 있다는 점이다.

따라서 이번 설계 기준은 다음과 같다.

```text
CombatSignalSource
-> CombatSignal
-> CombatSignalTarget
-> Evaluate
-> Apply
-> Notify Result
```

## 2. 핵심 판단

### 2.1 Request 용어를 핵심 이름에서 제외한다

`Request`는 보통 요청과 응답의 결합을 암시한다.

하지만 현재 전투 흐름은 source가 "이 신호를 처리해 달라"고 보내고, target이 자신의 상태를 기준으로 outcome을 판단하는 구조다. 결과가 source에게 돌아갈 수는 있지만, 모든 signal이 request-response API로 닫혀야 하는 것은 아니다.

따라서 핵심 파이프라인 이름에는 `Request`를 쓰지 않는다.

### 2.2 Attack 용어를 핵심 이름에서 제외한다

`Attack`은 공격 행위 쪽으로 의미가 좁다.

현재 파이프라인은 단순 공격만이 아니라 다음을 모두 다룬다.

- collision hit
- guard / block hit
- parry
- blink cue
- repulse cue
- attacker-side result
- defender-side reaction / feedback

이 흐름은 source와 target 사이의 전투 공방 전체에 가깝다. 따라서 `Attack`보다 `CombatSignal`이 더 적절하다.

### 2.3 Damage 용어를 핵심 이름에서 제외한다

`Damage`는 결과 적용 중 하나다.

Parry, Guard, Blink, Repulse처럼 damage commit이 없거나 조건부인 outcome도 같은 파이프라인에서 처리되어야 한다. 따라서 핵심 파이프라인 이름을 `Damage`로 두면 방어 성공, cue, result notification이 부자연스러워진다.

## 3. 용어 정의

### CombatSignal

아직 판정 전인 전투 입력 / 증거 / cue다.

예:

- weapon overlap hit evidence
- montage timing cue
- lock-on target cue
- UE `TakeDamage()`에서 변환된 legacy damage signal

### CombatSignalSource

전투 신호를 만드는 actor-side component다.

주요 책임:

- hit window 추적
- duplicate target 관리
- hit / cue signal 구성
- target에게 signal 전달
- optional result 수신

### CombatSignalTarget

전투 신호를 받는 actor-side component다.

주요 책임:

- signal 수신
- target-side context 검증
- guard / parry / hit / blink / repulse / dead 평가
- health / reaction / feedback / result out 적용 자료 구성
- 후속 domain 호출 또는 통지

### CombatSignalEvaluation

target이 signal을 해석한 평가 결과다.

예:

```text
Outcome = Parried
ShouldCommitDamage = false
DefenderReaction = Parry
AttackerResult = Parried
FeedbackCue = ParrySpark
```

### CombatSignalApplyResult

평가 결과를 실제 domain에 반영한 결과다.

예:

- committed damage
- health changed 여부
- reaction request accepted 여부
- feedback dispatched 여부

### CombatSignalResult

외부 통지 / source-side result / debug / event에 사용할 최종 결과다.

## 4. 권장 컴포넌트 구조

```text
UCCombatSignalSourceComponent
UCCombatSignalTargetComponent
```

v1에서는 `Evaluator`, `Applier`, `Notifier`를 별도 component로 나누지 않는다. 먼저 target component 내부 메서드 단계로 분리한다.

이유:

- 현재 코드의 실제 압축 위치는 `TakeDamageComponent` 내부다.
- 컴포넌트를 먼저 늘리면 routing 계층이 다시 과해질 수 있다.
- 함수 경계와 구조체 경계가 안정된 뒤에만 component 분리를 검토하는 편이 안전하다.

## 5. Source 책임

`CombatSignalSource`는 source-side 사실을 정리한다.

```text
Weapon overlap / cue
-> hit window id 확인
-> duplicate target 검사
-> signal payload 구성
-> target delivery
```

포함 책임:

- `OpenSignalWindow`
- `CloseSignalWindow`
- `BuildHitSignal`
- `BuildCueSignal`
- `SendCombatSignal`
- `NotifyCombatSignalResult`

제외 책임:

- target의 guard / parry 성공 판단
- damage commit
- defender reaction 실행
- defender feedback 실행

## 6. Target 책임

`CombatSignalTarget`은 target-side 판단과 후속 적용의 중심이다.

```text
ReceiveCombatSignal
-> Validate target context
-> EvaluateCombatSignal
-> ApplyCombatSignalOutcome
-> NotifyCombatSignalResult
```

포함 책임:

- signal 수신
- target-side validation
- defensive outcome 평가
- damage commit 필요 여부 결정
- health apply 호출
- reaction request 구성
- feedback request 구성
- source result 구성

제외 책임:

- source-side hit window 관리
- target discovery
- source actor의 공격 선택

## 7. Interface 기준

v1에서 반드시 필요한 interface는 target 수신 경계다.

```cpp
class ICCombatSignalTarget
{
    ReceiveCombatSignal(...);
};
```

source result notify interface는 후속으로 둔다.

```cpp
class ICCombatSignalSource
{
    NotifyCombatSignalResult(...);
};
```

source notify가 후순위인 이유는 result가 항상 source에게 돌아가야 하는 것이 아니기 때문이다. ParryStack / Stagger / network / debug 요구가 정리된 뒤 추가한다.

## 8. Blink / Repulse와의 관계

N04 기준으로 Blink / Repulse는 collision hit가 아니라 cue 기반 defensive success outcome이다.

따라서 별도 `EventSource / EventTarget` 축을 만들기보다 `CombatSignal` 안에 signal type을 둔다.

```text
ECombatSignalType::HitEvidence
ECombatSignalType::TimingCue
```

처리 차이는 target discovery와 payload 해석에서만 둔다.

```text
Collision hit
-> overlap으로 target 발견
-> HitEvidence signal 전달

Timing cue
-> cached target / lock-on target 발견
-> TimingCue signal 전달
```

target 이후 흐름은 가능하면 공유한다.

```text
ReceiveCombatSignal
-> EvaluateCombatSignal
-> ApplyCombatSignalOutcome
-> NotifyCombatSignalResult
```

## 9. 기존 컴포넌트 migration

### ApplyDamageComponent

장기적으로 `CombatSignalSource`로 이동한다.

```text
NotifyHitWindowOpened / Closed
-> OpenSignalWindow / CloseSignalWindow

RequestApplyDamage
-> BuildHitSignal
-> SendCombatSignal
```

### TakeDamageComponent

장기적으로 `CombatSignalTarget`으로 이동한다.

```text
RequestTakeDamage
-> ReceiveCombatSignal

CanTakeDamage / defensive outcome
-> EvaluateCombatSignal

CommitTakeDamage
-> ApplyCombatSignalOutcome

Reaction / Feedback / AttackerResult dispatch
-> NotifyCombatSignalResult
```

### UE TakeDamage

UE `AActor::TakeDamage()`는 legacy adapter로 유지한다.

```text
AActor::TakeDamage
-> BuildSignalFromDamageEvent
-> CombatSignalTarget.ReceiveCombatSignal
```

## 10. God Object 방지 규칙

`CombatSignalTarget`이 커지는 위험은 인정한다. v1에서는 책임을 다음 규칙으로 제한한다.

- source-side hit window는 source component에 둔다.
- target-side 평가만 target component가 소유한다.
- health 변경은 Health / resource component에 위임한다.
- reaction 실행 가능성은 ReactionOrchestrator에 위임한다.
- feedback 실행은 Feedback component에 위임한다.
- result 전달은 packet / interface / event 경계로 분리한다.
- 별도 component 분리는 함수 경계와 구조체 경계가 안정된 뒤 진행한다.

## 11. 이번 브랜치의 결론

이번 브랜치에서는 CombatSignal 경계 재정의와 최소 타입 vocabulary 추가까지 포함한다. 기존 gameplay 흐름에 연결되는 리팩터링은 다음 브랜치부터 진행한다.

확정:

- 핵심 이름은 `CombatSignal`로 둔다.
- source-side component 후보는 `UCCombatSignalSourceComponent`다.
- target-side component 후보는 `UCCombatSignalTargetComponent`다.
- `Request`, `Attack`, `Damage`는 핵심 파이프라인 이름에서 제외한다.
- 기존 `GameplayIntentGateway / GameplayCoordinator` 계획은 W04 주도 구조로 사용하지 않는다.

후속:

- 다음 작업은 같은 브랜치에서 `Combat Signal Types v1`로 진행한다.
- 이후 target boundary, source boundary, component rename은 별도 브랜치 순서로 진행한다.
