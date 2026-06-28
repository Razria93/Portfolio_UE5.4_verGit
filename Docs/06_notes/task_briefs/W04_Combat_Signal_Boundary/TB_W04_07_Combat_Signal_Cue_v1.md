# TB W04-07 Combat Signal Cue v1

## 작업명

Combat Signal Cue v1

## 브랜치

```text
feature/combat-signal-cue-v1
```

## 상태

```text
완료
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

## 연결 지점 분석 결과

### 현재 타입 상태

`CCombatSignalStructure`에는 cue를 표현할 최소 vocabulary가 이미 있다.

```text
ECombatSignalType::TimingCue
ECombatSignalOutcome::Blink
ECombatSignalOutcome::Repulse
FCombatSignal::SignalTag
FCombatSignal::CueTag
FCombatSignal::Direction
FCombatSignal::ImpactLocation
FCombatSignal::ImpactNormal
```

다만 현재 런타임 source / target component는 아직 `FCombatSignal`을 사용하지 않는다.

## TimingCue 표현력 확인 결과

### v1 연결에 필요한 정보

`TimingCue`를 source에서 구성하고 target receive hook으로 전달하는 데 필요한 최소 정보는 다음과 같다.

```text
signal type
source actor
target actor
instigator actor
signal causer
signal identity
cue identity
cue 기준 위치
cue 방향
```

현재 `FCombatSignal`은 위 정보를 모두 표현할 수 있다.

| 필요 정보 | 현재 필드 | 판단 |
| --- | --- | --- |
| signal type | `Header.SignalType` | `TimingCue` 지정 가능 |
| source actor | `Header.SourceActor` | 공격 / cue 발생 주체 표현 가능 |
| target actor | `Header.TargetActor` | 직접 전달 대상 표현 가능 |
| instigator actor | `Header.InstigatorActor` | 의도 주체 표현 가능 |
| signal causer | `Header.SignalCauser` | weapon / action / cue owner 표현 가능 |
| signal identity | `SignalTag` | 큰 신호 의미 표현 가능 |
| cue identity | `CueTag` | Blink / Repulse cue 종류 표현 가능 |
| cue 기준 위치 | `ImpactLocation` | cue 기준점으로 재사용 가능 |
| cue 방향 | `Direction` | attacker -> target 또는 cue 방향 표현 가능 |

### 이번 브랜치 판단

이번 브랜치에서는 `FCombatSignal` 필드를 추가하지 않는다.

이유:

- `TimingCue` v1의 목표는 cue signal을 만들고 target receive hook으로 넣는 것이다.
- 현재 필드만으로 source / target / cue identity / direction을 표현할 수 있다.
- Blink 이동 위치, Repulse 강도, input timing window는 아직 실제 판정 로직과 함께 검증되지 않았다.
- 사용처가 확정되지 않은 필드를 먼저 추가하면 cue 구조가 과설계될 수 있다.

따라서 이번 단계에서는 다음 기준으로 시작한다.

```text
SignalType = TimingCue
SignalTag = cue signal의 큰 의미
CueTag = Blink / Repulse 같은 실제 cue 종류
ImpactLocation = cue 기준 위치
Direction = cue 진행 방향 또는 source -> target 방향
```

`UCAnimNotify_CombatSignalCue`는 damage 값을 노출하지 않는다.

Notify는 cue 종류와 발생 시점만 제공하며, damage나 resource commit 후보값은 target-side policy / outcome 단계에서 별도로 판단한다.

### 후속 확장 후보

Blink / Repulse의 실제 판정과 실행을 구현할 때 다음 필드가 필요해질 수 있다.

```text
CueWindowId
CueStrength
TimingStart / TimingEnd
DestinationHint
LockOnTarget
bRequiresTargetFacing
bRequiresInputWindow
```

위 항목은 이번 브랜치에서 추가하지 않는다. 실제 cue evaluation / movement / repulse 실행 요구가 확인되는 브랜치에서 추가 여부를 판단한다.

## TimingCue tag / naming 기준

### 기준

`SignalTag`와 `CueTag`는 같은 정보를 반복해서 담지 않는다.

```text
SignalTag
= 신호의 큰 의미 / routing hint

CueTag
= 실제 cue 종류 / 판정 대상
```

v1에서는 `FName` 기반으로 유지하고, GameplayTag 또는 별도 enum으로 승격하지 않는다.

이유:

- 현재 프로젝트의 notify / feedback trigger도 `FName` 기반이다.
- cue 종류와 tag 계층이 아직 충분히 쌓이지 않았다.
- 지금 enum / GameplayTag로 고정하면 Blink / Repulse 이후 확장 방향을 먼저 제한할 수 있다.

### v1 기본 사용 예

```text
SignalType = ECombatSignalType::TimingCue
SignalTag = Combat.Signal.TimingCue
CueTag = Combat.Cue.Blink
```

```text
SignalType = ECombatSignalType::TimingCue
SignalTag = Combat.Signal.TimingCue
CueTag = Combat.Cue.Repulse
```

### 사용 규칙

- `SignalType`은 시스템 분기용 enum이다.
- `SignalTag`는 같은 `SignalType` 안에서 더 구체적인 신호 의미가 필요할 때 사용한다.
- `CueTag`는 Blink / Repulse처럼 target이 실제로 판정해야 하는 cue 종류를 담는다.
- `SignalTag`가 `NAME_None`이면 `FCombatSignal::IsValidMinimal()`을 통과하지 않는다.
- `TimingCue`에서는 `CueTag`도 필수로 보는 것이 맞다. 다만 이 검증은 `FCombatSignal` 공용 minimal validation이 아니라 target-side cue validation에서 처리한다.

### 이번 브랜치 판단

이번 브랜치에서는 tag 상수 파일을 만들지 않는다.

후속으로 cue 종류가 늘어나거나 여러 component가 같은 tag를 반복해서 사용하면 다음 중 하나로 승격한다.

```text
namespace 기반 FName 상수
GameplayTag
ECombatSignalCueType enum
```

### 현재 source 흐름

현재 source 흐름은 weapon overlap 기반 hit 전용이다.

```text
ACWeaponActor::OnComponentBeginOverlap
-> FHitContext 구성
-> UCCombatSignalSourceComponent::RequestCombatSignalSource(FHitContext)
-> source-side damage spec / amount 계산
-> FDefaultDamageEvent 구성
-> TargetActor->TakeDamage(...)
```

따라서 cue를 이 흐름에 억지로 끼우면 `FHitContext`, `FDefaultDamageEvent`, `DamageSpec` 의존이 불필요하게 따라온다.

### 현재 target 흐름

현재 target 흐름은 UE `TakeDamage()` entry 기반 damage event 전용이다.

```text
AActor::TakeDamage
-> UCCombatSignalTargetComponent::RequestCombatSignalTarget(float, FDamageEvent, Instigator, DamageCauser)
-> FDefaultDamageEvent 검증
-> FCombatSignalTargetPayload / Context 구성
-> Guard / Parry / Hit / Dead 평가
-> Health commit / Reaction / Feedback / Result dispatch
```

`TimingCue`는 UE damage event가 아니므로 이 entry에 직접 태우면 의미가 흐려진다.

### 권장 연결 지점

이번 브랜치의 최소 연결은 `FCombatSignal` 전용 entry를 Source / Target에 병렬로 추가하는 것이다.

```text
HitEvidence
-> 기존 FHitContext / FDefaultDamageEvent / TakeDamage 흐름 유지

TimingCue
-> FCombatSignal 구성
-> target component의 FCombatSignal entry로 전달
-> TimingCue evaluation hook으로 분기
```

즉 기존 damage hit 흐름을 교체하지 않고, 같은 `CombatSignalTargetComponent` 안에 cue receive hook을 연다.

### 첫 구현 후보

```text
UCCombatSignalSourceComponent
- BuildCueSignal(...)
- SendCueSignal(...)

UCCombatSignalTargetComponent
- RequestCombatSignalTarget(const FCombatSignal& InCombatSignal)
- ProcessCombatSignalTarget(const FCombatSignal& InCombatSignal)
- HandleTimingCueSignal(...)
```

이름은 구현 시 기존 API 순서와 책임을 맞춰 다시 확정한다.

### 이번 단계 판단

- `FCombatSignal` 타입은 source / target component에 아직 연결되어 있지 않다.
- cue는 `TakeDamage()` 경로가 아니라 `FCombatSignal` 직접 전달 경로가 필요하다.
- target discovery는 이번 브랜치에서 새 service로 만들지 않고, source component가 기존 AI blackboard target을 조회하는 형태부터 시작한다.
- 기존 hit flow는 그대로 유지한다.

### 2. Source-side cue build API 후보 추가

`UCCombatSignalSourceComponent`에 collision hit와 분리된 cue signal build / send 후보를 추가한다.

예상 형태:

```text
BuildCueSignal(...)
SendCueSignal(...)
```

실제 이름은 기존 source component API 흐름을 보고 결정한다.

구현 결과:

```text
UCAction::ResolveNotifyCombatSignalCue(CueTag)
-> FActionCombatSignalCueRequest

UCActionComponent::HandleActionCombatSignalCue(CueTag)
-> active action cue request resolve
-> RequestAICombatSignalCue(ResolvedCueTag)

RequestAICombatSignalCue(CueTag)
-> source-side target resolve
-> source-side cue location / direction 구성
-> RequestCombatSignalCue(TargetActor, CueTag, ...)

RequestCombatSignalCue(...)
-> BuildCueSignal(...)
-> ValidateCueSignal(...)
-> SendCueSignal(...)
```

`UCCombatSignalSourceComponent` 내부 helper는 hit flow helper와 cue flow helper를 분리해서 배치한다.

```text
Hit Helper
-> BuildHitWindowKey
-> BuildSpecKey
-> ResolveInstigatorController
-> IsDuplicateHit
-> IsFriendlyTarget

Cue Helper
-> ResolveCueTargetActor
-> BuildCueSignal
-> ValidateCueSignal
```

현재 `BuildCueSignal`은 `FCombatSignal`의 `TimingCue` 값을 구성한다.

```text
SignalType = TimingCue
SignalTag = Combat.Signal.TimingCue
CueTag = caller provided cue tag
SourceActor = owner
TargetActor = resolved target or caller provided target
InstigatorActor = owner
SignalCauser = caller provided causer or owner
Direction = caller direction or source -> target direction
```

`SendCueSignal`은 target actor의 `UCCombatSignalTargetComponent`를 찾아 `FCombatSignal` entry로 전달한다.

```text
SendCueSignal(FCombatSignal)
-> TargetActor->FindComponentByClass<UCCombatSignalTargetComponent>()
-> RequestCombatSignalTarget(FCombatSignal)
```

### Source entry naming 후속 정리

현재 source public entry 이름은 임시 비대칭 상태다.

```text
RequestCombatSignalSource(FHitContext)
RequestCombatSignalCue(...)
```

두 함수 모두 source component에 원천 입력을 제출하고, 내부에서 build / validate / send 단계를 수행한다.

따라서 장기적으로 public entry는 `Submit...Signal` 기준으로 정렬하는 것이 더 명확하다.

후속 후보:

```text
RequestCombatSignalSource
-> SubmitHitEvidenceSignal

RequestCombatSignalCue
-> SubmitTimingCueSignal
```

`Signal` suffix는 public entry가 단순 cue 처리 함수가 아니라 combat signal 처리 흐름으로 제출하는 진입점임을 드러내기 위해 유지한다.

내부 단계는 현재처럼 역할 기준 이름을 유지한다.

```text
BuildCueSignal
ValidateCueSignal
SendCueSignal
```

이번 단계에서는 기존 hit 호출부를 유지한 채 cue target hook만 연결한다. public entry rename은 별도 후속 작업으로 미룬다.

### Target entry naming 후속 정리

target public entry도 현재는 임시 overload 상태다.

```text
RequestCombatSignalTarget(float, FDamageEvent, ...)
RequestCombatSignalTarget(FCombatSignal)
```

두 함수는 각각 UE damage event adapter entry와 `FCombatSignal` direct entry로 성격이 다르다.

후속 후보:

```text
RequestCombatSignalTarget(float, FDamageEvent, ...)
-> ReceiveDamageEvent

RequestCombatSignalTarget(FCombatSignal)
-> ReceiveCombatSignal

ProcessCombatSignalTarget(float, FDamageEvent, ...)
-> ProcessDamageEvent

ProcessCombatSignalTarget(FCombatSignal)
-> ProcessCombatSignal
```

이번 단계에서는 cue receive hook 연결을 우선하고, Source / Target entry naming 정렬은 별도 후속 작업으로 미룬다.

### 3. Target-side cue receive / evaluation hook 추가

`UCCombatSignalTargetComponent`가 `TimingCue` signal을 받을 수 있는 hook을 둔다.

이번 단계에서는 Blink / Repulse의 완성 판정 로직보다 다음 경계를 우선한다.

```text
HitEvidence
-> 기존 damage hit 흐름

TimingCue
-> cue evaluation hook
```

구현 결과:

```text
RequestCombatSignalTarget(FCombatSignal)
-> ProcessCombatSignalTarget(FCombatSignal)
-> HandleTimingCueSignal(FCombatSignal)
-> ValidateSignalRequest(FCombatSignal)
```

현재 `HandleTimingCueSignal`은 receive / validation hook만 제공한다.

```text
TimingCue signal 수신 성공
-> true

invalid signal / unsupported signal
-> false
```

Blink / Repulse 판정, movement, reaction, feedback, result-out 분배는 아직 수행하지 않는다.

### 4. Target discovery 범위 제한

이번 브랜치에서는 lock-on / targeting service를 새로 만들지 않는다.

가능하면 기존 cached target actor 또는 직접 전달 가능한 target을 기준으로 cue signal을 전달한다.

구현 결과:

```text
UCAnimNotify_CombatSignalCue
-> owner action 상태 검증
-> UCActionComponent::HandleActionCombatSignalCue(CueTag)
-> active UCAction::ResolveNotifyCombatSignalCue(CueTag)
-> UCActionComponent routes resolved request to source component

UCCombatSignalSourceComponent
-> AI Blackboard TargetActor 조회
-> cue location / direction 구성
-> FCombatSignal TimingCue 구성
-> SendCueSignal(...)
```

`UCAnimNotify_CombatSignalCue`는 TimingCue 발생 시점을 알려주는 notify다. Enemy attack montage에 배치해 `Combat.Cue.Blink` 또는 `Combat.Cue.Repulse`를 action notify routing으로 전달한다.

전달 규약:

```text
Notify
-> ActionComponent
-> Active Action policy resolve
-> ActionComponent
-> CombatSignalSourceComponent
```

기존 action notify와의 차이:

```text
기존 action command notify
-> ActionComponent
-> Active Action command handling
-> Action 내부 상태 변경

Action feedback notify
-> ActionComponent
-> Active Action feedback handling
-> Action 생애주기 기반 feedback 출력

CombatSignalCue notify
-> ActionComponent
-> Active Action cue policy resolve
-> ActionComponent routing
-> CombatSignalSourceComponent send
```

기존 `Complete`, `Equip`, `Combo`, `HitContext`, `Guard` 계열 notify는 대부분 현재 Action 내부 상태를 변경하는 명령이다. 따라서 `EActionNotifyCommand`로 일방 라우팅해도 의미가 명확하다.

`CombatSignalCue`는 notify 발생 자체가 Action 내부 명령이 아니라 외부 target으로 전달되는 combat signal 송신의 시작점이다. Action은 이 cue를 허용할지, 어떤 cue tag로 해석할지만 결정하고, 실제 target resolve / signal build / send는 source component가 수행한다. 그래서 `Handle`보다 `Resolve -> Route -> Send` 흐름으로 분리한다.

현재 source-side target discovery는 AI blackboard의 `CAIKey::Targeting::TargetActor`만 사용한다. Lock-on, targeting service, subsystem 승격은 Blink / Repulse 실제 구현 단계에서 다시 판단한다.

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
- Enemy attack notify에서 ActionComponent / active Action policy resolve를 거쳐 AI blackboard target으로 cue signal을 보낼 수 있다.
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

검증 결과:

```text
git diff --check: 통과
PortfolioEditor Win64 Development build: 성공
Enemy -> Player TimingCue delivery: 확인
```

## 런타임 확인

Enemy attack montage의 `UCAnimNotify_CombatSignalCue`에서 `Combat.Cue.Blink`를 발생시켜 Player target까지 전달되는 것을 확인했다.

```text
[CombatSignalTimingCue] Blink cue received
[CombatSignalCueNotify] Sent | Source=BP_CEnemy_C_1 | Target=BP_CPlayer_C_0 | CueTag=Combat.Cue.Blink
```

확인된 범위:

```text
Enemy AnimNotify
-> Enemy UCActionComponent
-> active UCAction cue policy resolve
-> Enemy UCActionComponent route
-> Enemy UCCombatSignalSourceComponent
-> FCombatSignal TimingCue 구성
-> Player UCCombatSignalTargetComponent
-> HandleTimingCueSignal
```

이 단계는 Blink 실행 구현이 아니라 cue delivery hook 검증이다.

`M_Attack_Sword_2`의 `CAnimNotifyState_ExecutionInterventionWindow(WindowKey=None)`는 이전 interruption rule 구조에서 사용하던 legacy notify state였으므로 제거했다. 현재 attack 2 interrupt 동작은 `Always` rule 기준으로 검증했으며, 해당 `None` window에 의존하지 않는다.

## 후속 ResultOut 결정

`ResultOut`은 이번 cue delivery 브랜치에서 선행 일반화하지 않는다.

Repulse는 성공 결과가 attacker-side reaction으로 되돌아가야 하는 기능이므로, Repulse v1에서 필요한 최소 ResultOut을 함께 구현한다. 이후 Repulse result 사례와 기존 ParryStack / Stagger 흐름을 비교해 `refactor/combat-result-out-v1`에서 공통화한다.

후속 순서:

```text
1. feature/combat-blink-cue-v1
2. feature/combat-repulse-cue-v1
   - minimum ResultOut 포함
3. refactor/combat-result-out-v1
   - Repulse / ParryStack / Stagger result 흐름 통합
```

## Unknown Cue 처리

지원하지 않는 `CueTag`가 target에 도착하면 `ECombatSignalTargetRejectReason::UnknownCueTag` 기준으로 reject 로그를 남긴다.

```text
[CombatSignalTimingCue] Rejected | Reason=ECombatSignalTargetRejectReason::UnknownCueTag | CueTag=...
```

## 프롬프트 업데이트 체크

작업 중 다음 기준이 새로 확인되면 `PU01_Combat_Signal_Boundary_Prompt_Update_Note.md`에 반영한다.

- collision hit와 timing cue를 같은 vocabulary로 다룰 때의 naming 기준
- cue signal이 damage data와 분리되어야 하는 기준
- target discovery를 별도 service로 승격해야 하는 기준
