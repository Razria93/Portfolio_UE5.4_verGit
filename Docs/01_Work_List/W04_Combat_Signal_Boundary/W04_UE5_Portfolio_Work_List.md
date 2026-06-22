# W04 Combat Signal Boundary Work List

## 1. 목표

이번 작업은 W03 이후 논의된 `Combat Resolution / Request / Routing` 구조를 다시 검토하고, 현재 프로젝트에 필요한 최소 전투 파이프라인 경계를 `CombatSignal Source / Target` 기준으로 재정의한다.

핵심은 공용 `Intent Gateway / Coordinator`를 먼저 만드는 것이 아니라, 실제로 책임이 압축되어 있는 `ApplyDamageComponent`와 `TakeDamageComponent`의 장기 대체 축을 명확히 정하는 것이다.

```text
기존 압축 구조
-> ApplyDamageComponent
-> TakeDamageComponent

새 기준
-> CombatSignalSource
-> CombatSignal
-> CombatSignalTarget
-> Evaluate
-> Apply
-> Notify Result
```

## 2. 방향 전환 결정

이전 후보였던 `GameplayIntentGateway / GameplayCoordinator` 구조는 이번 W04의 주도 구조로 사용하지 않는다.

보류 이유는 다음과 같다.

- 현재 문제는 공용 intent routing 부재보다 전투 송수신 경계의 이름과 책임 혼재가 더 직접적이다.
- Gateway / Coordinator를 먼저 만들면 God Object가 될 가능성이 크다.
- `Request`는 request-response 흐름을 암시하지만, 현재 전투 흐름은 source가 signal을 전달하고 target이 자체 상태로 outcome을 판단하는 구조에 가깝다.
- Blink / Repulse 같은 cue 기반 defensive outcome은 `Damage`나 `Attack`보다 넓은 전투 신호 개념이 필요하다.

따라서 이번 W04는 `CombatSignal` vocabulary와 Source / Target 책임 경계를 먼저 확정한다.

## 3. 권장 이름

### 3.1 컴포넌트

```text
UCCombatSignalSourceComponent
UCCombatSignalTargetComponent
```

`Source`는 전투 신호를 만드는 쪽이다. 무기 overlap, montage notify, timing cue, lock-on cue 등에서 target에게 보낼 전투 신호를 구성하고 전달한다.

`Target`은 전투 신호를 받는 쪽이다. 자신의 guard / parry / dead / invincible / cue timing 상태를 바탕으로 outcome을 평가하고 결과 적용과 후속 통지를 수행한다.

### 3.2 구조체

```text
FCombatSignalHeader
FCombatSignal
FCombatSignalContext
FCombatSignalEvaluation
FCombatSignalApplyResult
FCombatSignalResult
```

### 3.3 열거형

```text
ECombatSignalType
- None
- HitEvidence
- TimingCue
- DirectDamage
- System

ECombatSignalOutcome
- None
- Hit
- Blocked
- Parried
- Blink
- Repulse
- Staggered
- Dead

ECombatSignalResultType
- None
- Handled
- Ignored
- Rejected
```

## 4. 책임 경계

### 4.1 CombatSignalSource

책임:

- hit window open / close 상태를 추적한다.
- 동일 hit window 안에서 duplicate target을 거른다.
- overlap / cue / timing signal을 `FCombatSignal`로 구성한다.
- target actor의 `CombatSignalTarget` 경계를 찾아 signal을 전달한다.
- 필요 시 target에서 돌아온 result를 받아 ParryStack, debug, result out에 연결한다.

하지 않는 일:

- defender의 Guard / Parry / Blink / Repulse 성공 여부를 결정하지 않는다.
- HP를 직접 감소시키지 않는다.
- defender reaction / feedback을 직접 실행하지 않는다.

### 4.2 CombatSignalTarget

책임:

- `FCombatSignal`을 수신한다.
- signal 유효성과 target-side context를 검증한다.
- Guard / Parry / Hit / Blink / Repulse / Dead outcome을 평가한다.
- Health / Reaction / Feedback / ResultOut으로 넘길 apply/result 자료를 만든다.
- 필요한 후속 domain 호출 순서를 관리한다.
- UE `TakeDamage()`는 장기적으로 이쪽으로 들어오는 legacy adapter로 축소한다.

하지 않는 일:

- source-side hit window를 관리하지 않는다.
- target discovery를 수행하지 않는다.
- source actor의 공격 판정을 대신 결정하지 않는다.

## 5. 내부 흐름

```text
1. Source 준비
Weapon overlap / montage notify / cue window 발생

2. Signal 생성
UCCombatSignalSourceComponent
-> FCombatSignal 구성

3. Signal 전달
SourceComponent
-> TargetActor의 UCCombatSignalTargetComponent 탐색
-> ReceiveCombatSignal 호출

4. Target 평가
TargetComponent
-> EvaluateCombatSignal
-> Guard / Parry / Hit / Blink / Repulse / Dead 판단

5. 결과 적용
TargetComponent
-> ApplyCombatSignalOutcome
-> Health / Reaction / Feedback / ResultOut으로 분배

6. 결과 통지
TargetComponent
-> 필요 시 SourceComponent 또는 attacker-side result receiver에 CombatSignalResult 전달
```

## 6. API 후보

### Source

```cpp
void OpenSignalWindow(...);
void CloseSignalWindow(...);
FCombatSignal BuildHitSignal(...);
FCombatSignal BuildCueSignal(...);
FCombatSignalResult SendCombatSignal(const FCombatSignal& InSignal);
void NotifyCombatSignalResult(const FCombatSignalResult& InResult);
```

### Target

```cpp
FCombatSignalResult ReceiveCombatSignal(const FCombatSignal& InSignal);
FCombatSignalEvaluation EvaluateCombatSignal(const FCombatSignal& InSignal);
FCombatSignalApplyResult ApplyCombatSignalOutcome(const FCombatSignalEvaluation& InEvaluation);
void NotifyCombatSignalResult(const FCombatSignalResult& InResult);
```

### Legacy Adapter

```cpp
FCombatSignal BuildSignalFromDamageEvent(...);
FCombatSignalResult ReceiveEngineDamage(...);
```

## 7. 인터페이스 구성

v1에서는 interface를 최소화한다.

우선 후보:

```cpp
class ICCombatSignalTarget
{
    ReceiveCombatSignal(...);
};
```

후속 후보:

```cpp
class ICCombatSignalSource
{
    NotifyCombatSignalResult(...);
};
```

Target interface가 먼저 필요한 이유는 수신 경계가 반드시 필요하기 때문이다. 반면 source result notify는 ParryStack / Stagger / debug / network 요구가 정리된 후 추가해도 된다.

## 8. 현재 컴포넌트와의 관계

### 8.1 ApplyDamageComponent

현재 `UCApplyDamageComponent`는 이름과 달리 HP를 직접 적용하지 않는다.

현재 책임:

```text
hit window 관리
duplicate target 관리
hit context validate
damage spec 조회
requested damage 계산
target TakeDamage 호출
```

장기 위치:

```text
UCApplyDamageComponent
-> UCCombatSignalSourceComponent
```

### 8.2 TakeDamageComponent

현재 `UCTakeDamageComponent`는 수신, 평가, 적용, 후속 분배를 모두 가진다.

현재 책임:

```text
UE TakeDamage 수신
payload / context 구성
defensive outcome 판단
damage 계산
Health commit
reaction / feedback dispatch
attacker result packet dispatch
```

장기 위치:

```text
UCTakeDamageComponent
-> UCCombatSignalTargetComponent
```

## 9. 작업 분할 계획

### W04-01 Combat Signal Boundary Re-scope

브랜치:

```text
refactor/combat-signal-boundary
```

목표:

```text
Intent Gateway / Coordinator 중심 계획을 보류하고 CombatSignal Source / Target 기준을 문서로 확정한다.
```

핵심 범위:

- W04 work list 작성
- N05 Combat Signal Boundary 설계 노트 작성
- N06 branch implementation plan 작성
- task brief / work journal / prompt update note 작성
- 코드 변경 없음

완료조건:

- 같은 브랜치에서 Combat Signal 타입 추가 작업으로 바로 착수 가능하다.
- 문서에서 `Attack`, `Request`, `Damage`가 핵심 파이프라인 이름으로 남지 않는다.
- Source / Target 책임 경계가 문서만 보고 판단 가능하다.

### W04-02 Combat Signal Types

브랜치:

```text
refactor/combat-signal-boundary
```

커밋 단위:

```text
feat(combat): add combat signal type vocabulary
```

목표:

```text
CombatSignal 파이프라인의 최소 타입을 추가한다.
```

핵심 범위:

- `FCombatSignalHeader`
- `FCombatSignal`
- `FCombatSignalContext`
- `FCombatSignalEvaluation`
- `FCombatSignalApplyResult`
- `FCombatSignalResult`
- `ECombatSignalType`
- `ECombatSignalOutcome`
- `ECombatSignalResultType`

완료조건:

- 기존 `ApplyDamageComponent` / `TakeDamageComponent` 동작 변화 없음
- 타입만 추가
- Unreal build 성공

### W04-03 Combat Signal Target Boundary v1

브랜치:

```text
refactor/combat-signal-target-v1
```

목표:

```text
UCTakeDamageComponent 내부 흐름을 CombatSignalTarget 책임 기준으로 정리한다.
```

핵심 범위:

- Receive / Evaluate / Apply / Notify 단계 메서드 분리
- 기존 `RequestTakeDamage` 흐름 유지
- UE `TakeDamage()` adapter 유지
- 클래스명 rename은 아직 하지 않음

완료조건:

- 기존 Guard / Parry / Hit / Dead 동작 유지
- target-side 책임 단계가 코드에서 명확히 보임
- Unreal build 성공

### W04-04 Combat Signal Source Boundary v1

브랜치:

```text
refactor/combat-signal-source-v1
```

목표:

```text
UCApplyDamageComponent 내부 흐름을 CombatSignalSource 책임 기준으로 정리한다.
```

핵심 범위:

- hit window tracking
- duplicate target tracking
- signal build 후보 경계
- target delivery 경계
- `RequestApplyDamage` API rename 후보 준비

완료조건:

- 기존 weapon overlap damage 동작 유지
- source-side 책임 단계가 코드에서 명확히 보임
- Unreal build 성공

### W04-05 Combat Signal Component Rename

브랜치:

```text
refactor/combat-signal-component-rename
```

목표:

```text
책임 정리가 끝난 뒤 ApplyDamage / TakeDamage 명칭을 CombatSignalSource / Target으로 교체한다.
```

핵심 범위:

- `UCApplyDamageComponent` -> `UCCombatSignalSourceComponent`
- `UCTakeDamageComponent` -> `UCCombatSignalTargetComponent`
- Character / Weapon 참조 갱신
- Blueprint 영향 확인

완료조건:

- 이름만 바꾼 것이 아니라 이전 브랜치의 책임 정리가 선행되어 있다.
- 기존 전투 회귀 통과
- Unreal build 성공

### W04-06 Combat Signal Cue v1

브랜치:

```text
feature/combat-signal-cue-v1
```

목표:

```text
Blink / Repulse 같은 collision 없는 timing cue를 CombatSignal 흐름에 연결한다.
```

핵심 범위:

- `ECombatSignalType::TimingCue` 사용
- target discovery 방식 정리
- Blink / Repulse evaluation hook 추가

완료조건:

- collision hit와 timing cue가 같은 target receive 흐름을 공유한다.
- cue 전용 예외 파이프라인을 만들지 않는다.

## 10. 관련 문서

- `Docs/06_notes/N03_Guard_Hold_Overlay_Layer_Design_Note.md`
- `Docs/06_notes/N04_Blink_Repulse_Combat_Packet_Design_Note.md`
- `Docs/06_notes/archive/NA01_Combat_Intent_Request_Resolution_Routing_Design_Note.md`
- `Docs/06_notes/N05_Combat_Signal_Boundary_Design_Note.md`
- `Docs/06_notes/N06_Combat_Signal_Branch_Implementation_Plan.md`
