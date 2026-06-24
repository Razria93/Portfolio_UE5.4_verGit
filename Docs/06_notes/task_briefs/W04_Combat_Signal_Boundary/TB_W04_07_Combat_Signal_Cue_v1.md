# TB W04-07 Combat Signal Cue v1

## 작업명

Combat Signal Cue v1

## 브랜치

```text
feature/combat-signal-cue-v1
```

## 상태

```text
준비
```

## 목적

Blink / Repulse 같은 collision 없는 timing cue를 기존 damage hit 흐름과 별도 예외 파이프라인으로 만들지 않고, `CombatSignal`의 `TimingCue` 축으로 표현할 수 있게 한다.

이번 브랜치는 Blink / Repulse를 완성하는 브랜치가 아니라, cue 기반 전투 신호가 source에서 구성되고 target receive 흐름으로 들어갈 수 있는 최소 연결 지점을 만드는 브랜치다.

## 배경

W04 이전 구조에서는 combat 흐름이 weapon overlap damage를 중심으로 구성되어 있었다.

```text
Weapon overlap
-> source damage payload 구성
-> target TakeDamage
-> guard / parry / hit / dead 판정
```

하지만 Blink / Repulse는 collision hit가 아니라 timing cue에 가깝다.

```text
Action / notify / lock-on cue
-> cue signal 구성
-> target 상태 / 타이밍 기준 판정
-> Blink / Repulse outcome
```

따라서 기존 damage payload에 억지로 끼워 넣기보다, `FCombatSignal`의 `TimingCue` 타입을 사용해서 collision hit와 cue가 같은 target receive 개념을 공유하도록 한다.

## 작업 범위

### 1. Cue vocabulary 점검

현재 존재하는 타입을 기준으로 cue 표현이 충분한지 확인한다.

```text
ECombatSignalType::TimingCue
ECombatSignalOutcome::Blink
ECombatSignalOutcome::Repulse
FCombatSignal::SignalTag
FCombatSignal::CueTag
FCombatSignal::Direction
FCombatSignal::ImpactLocation
```

필요한 경우 최소 필드만 추가한다.

### 2. Source-side cue build API 후보 추가

`UCCombatSignalSourceComponent`에 collision hit와 분리된 cue signal build / send 후보를 추가한다.

예상 형태:

```text
BuildCueSignal(...)
SendCueSignal(...)
```

실제 이름은 기존 source component API 흐름을 보고 결정한다.

### 3. Target-side cue receive / evaluation hook 추가

`UCCombatSignalTargetComponent`가 `TimingCue` signal을 받을 수 있는 hook을 둔다.

이번 단계에서는 Blink / Repulse의 완성 판정 로직보다 다음 경계를 우선한다.

```text
HitEvidence
-> 기존 damage hit 흐름

TimingCue
-> cue evaluation hook
```

### 4. Target discovery 범위 제한

이번 브랜치에서는 lock-on / targeting service를 새로 만들지 않는다.

가능하면 기존 cached target actor 또는 직접 전달 가능한 target을 기준으로 cue signal을 전달한다.

## 제외 범위

```text
Blink movement 구현
Repulse force / stagger 구현
Targeting Service / Subsystem 신규 구현
CombatResultPacket 구조 변경
DamageReaction / DamageFeedback 리네임
기존 damage amount 계산 변경
Guard / Parry 기존 판정 변경
```

## 완료 조건

- `TimingCue` signal이 source에서 구성될 수 있다.
- cue signal이 target receive 흐름으로 들어갈 수 있다.
- `HitEvidence`와 `TimingCue`의 분기 위치가 명확하다.
- cue 전용 별도 최상위 파이프라인을 만들지 않는다.
- 기존 Player -> Enemy / Enemy -> Player hit 동작이 유지된다.
- `PortfolioEditor Win64 Development` 빌드가 성공한다.

## 검증 계획

```text
rg cue / TimingCue 사용처 확인
git diff --check
PortfolioEditor Win64 Development build
Player -> Enemy hit 유지 확인
Enemy -> Player hit 유지 확인
```

## 프롬프트 업데이트 체크

작업 중 다음 기준이 새로 확인되면 `PU01_Combat_Signal_Boundary_Prompt_Update_Note.md`에 반영한다.

- collision hit와 timing cue를 같은 vocabulary로 다룰 때의 naming 기준
- cue signal이 damage data와 분리되어야 하는 기준
- target discovery를 별도 service로 승격해야 하는 기준

