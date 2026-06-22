# J01 Combat Signal Boundary Work Journal

## 1. Context

W03 Guard / Parry v1 이후 다음 브랜치 후보는 `Combat Resolution 분리`, `Combat Consequence Coordinator`, `ApplyDamageComponent 책임 재정리`였다.

초기 논의에서는 공용 intent flow를 만들기 위해 다음 구조가 검토되었다.

```text
Intent
-> Gateway
-> Coordinator
-> Resolution
-> Domain
```

그러나 설계가 진행될수록 입력, damage, timing cue, system event를 모두 하나의 `Request` 파이프라인으로 묶는 것이 현재 문제보다 넓은 일반화라는 점이 드러났다.

각 event source는 발생 원인과 해석 기준이 다르다. 이를 하나의 Gateway / Coordinator가 판정하고 분배하면 해당 객체가 각 domain rule을 과도하게 알게 되고, 반대로 모든 축을 세밀하게 분리하면 현재 규모에 비해 adapter와 계층이 과도하게 늘어난다.

## 2. Options

### Option A: GameplayIntentGateway / GameplayCoordinator 유지

장점:

- 입력, 이벤트, 결과를 한 공용 흐름으로 묶을 수 있다.
- 장기적으로 다양한 domain intent를 통제할 수 있다.

단점:

- 현재 전투 파이프라인 문제보다 추상화가 앞선다.
- Coordinator가 domain rule을 흡수하면 God Object가 되기 쉽다.
- Submit / Coordinate / Result 용어가 실제 책임과 어긋나기 시작했다.

### Option B: CombatRequest Source / Receiver 구조

장점:

- 기존 ApplyDamage / TakeDamage 구조와 가까워 migration이 쉽다.
- 송신 / 수신 책임을 나누기 쉽다.

단점:

- `Request`가 request-response 흐름을 암시한다.
- Hit / Cue / Parry / Blink / Repulse가 모두 요청이라는 이름 아래 들어가면서 의미가 흐려진다.

### Option C: CombatSignal Source / Target 구조

장점:

- collision hit와 timing cue를 함께 담을 수 있다.
- source는 신호를 만들고 target은 상태 기반 outcome을 판단한다는 흐름이 명확하다.
- `Attack`보다 넓고 `Damage`보다 앞선 개념이다.
- 기존 ApplyDamage / TakeDamage의 장기 대체 이름으로 자연스럽다.

단점:

- `Signal`이 다소 추상적이므로 문서에서 의미를 명확히 고정해야 한다.
- target component가 커질 수 있어 내부 단계 경계를 엄격히 유지해야 한다.

## 3. Discussion

핵심 쟁점은 이름이었다.

`Attack`은 공격 행위 쪽으로 기울어 source와 target의 공방 전체를 담기에는 작았다. `Hit`는 현재 collision damage 흐름에는 직관적이지만, Blink / Repulse 같은 timing cue 기반 defensive outcome까지 담기에는 좁았다. `Damage`는 HP commit 이후의 결과에 가까워 Parry / Guard / Blink / Repulse를 중심 개념으로 다루기 어렵다.

반면 `CombatSignal`은 아직 판정 전인 전투 입력 / 증거 / cue라는 의미를 줄 수 있다.

```text
CombatSignal
= target이 아직 해석하지 않은 전투 신호
```

이 정의라면 collision hit와 non-collision cue를 모두 같은 target receive 흐름으로 다룰 수 있다.

## 4. Decision

W04 기준 구조는 다음으로 확정한다.

```text
UCCombatSignalSourceComponent
UCCombatSignalTargetComponent
```

핵심 데이터 이름은 다음 후보로 둔다.

```text
FCombatSignal
FCombatSignalEvaluation
FCombatSignalApplyResult
FCombatSignalResult
```

기존 `GameplayIntentGateway / GameplayCoordinator` 구조는 이번 W04 주도 구조에서 제외한다. 이는 일반화 자체를 부정하는 결정이 아니라, 입력 처리 축 / combat 처리 축 / timing cue 처리 축이 안정된 뒤 다시 검토하기 위한 보류다.

## 5. Reason

이 결정은 현재 코드의 실제 문제와 가장 가깝다.

현재 `UCApplyDamageComponent`는 damage applier가 아니라 source-side signal builder / delivery 역할에 가깝고, `UCTakeDamageComponent`는 target-side receive / evaluate / apply / notify 역할을 모두 가진다.

따라서 먼저 두 컴포넌트의 책임을 `CombatSignalSource / Target`으로 재정의하고, 이후 타입 추가와 내부 단계 분리로 들어가는 것이 가장 작고 안전하다. 공용 상태 변경 파이프라인은 이 축의 반복 패턴이 안정된 뒤 다시 판단한다.

## 6. Follow-up

다음 작업은 같은 브랜치에서 진행한다.

```text
Combat Signal Types v1
```

작업 목표:

```text
CombatSignal Source / Target이 공유할 최소 타입 vocabulary를 추가한다.
```

주의:

- 기존 ApplyDamage / TakeDamage 흐름에 바로 연결하지 않는다.
- component rename은 책임 정리가 끝난 뒤 별도 브랜치에서 수행한다.
