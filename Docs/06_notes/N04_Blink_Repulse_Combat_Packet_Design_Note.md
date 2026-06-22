# Blink / Repulse Combat Packet Design Note

> Status update: W04에서 전투 송수신 경계를 `CombatSignal Source / Target` 기준으로 재정의했다. 이 문서의 `Combat packet`, `Request`, `Receiver`, `Resolution` 표현은 후속 작업에서 `CombatSignal`, `CombatSignalSource`, `CombatSignalTarget`, target-side evaluation 기준으로 재해석한다. 최신 기준은 `N05_Combat_Signal_Boundary_Design_Note.md`를 따른다.

## 1. 목적

본 문서는 Blink / Repulse와 cue 기반 combat packet 전달 구조를 정리한다.

현재 W03은 Guard / Parry v1을 안정화하는 브랜치이므로 Blink / Repulse를 구현하지 않는다. 다만 Parry 이후 counter 계열을 확장하려면, 공격 cue 전달, target 검색, 결과 판정, feedback / event 알림의 책임을 미리 구분해야 한다.

이 문서는 구현 지시서가 아니라 후속 Combat Resolution / Counter 설계 판단 기록이다.

---

## 2. 용어 기준

### Blink

`Blink`는 특수 공격 cue에 대한 타이밍 방어 성공 결과다.

주요 결과는 defender movement / relocation이다. 즉, 공격을 단순히 막는 것이 아니라 defender가 target-relative 위치로 이동하거나 회피 우위를 얻는 counter outcome으로 본다.

### Repulse

`Repulse`는 특수 공격 cue에 대한 타이밍 방어 성공 결과다.

주요 결과는 attacker 상태 변화다. 예를 들어 attacker가 밀려나거나, 약점 / stagger / exposed state가 열리는 counter outcome으로 본다.

`Repulse`는 사용자가 말한 리펄스 기준 명칭이며, `Impulse`와 구분한다.

### Defensive Outcome

Parry / Guard / Blink / Repulse는 모두 Combat Resolution이 판정하는 defensive outcome 후보로 본다.

```text
Incoming combat packet
-> defender 상태 / 입력 / cue 해석
-> Outcome: Hit / Guard / Parry / Blink / Repulse
```

---

## 3. Cue 기반 전달 흐름

공격자가 특수 공격 cue를 가지고 있다면, sender가 결과를 확정하지 않는다. Sender는 cue와 source 정보를 packet에 담아 target에게 전달하고, receiver가 자기 상태를 기준으로 outcome을 결정한다.

```text
Attacker Action / Hit Window / Cue Window
-> Combat packet 생성
-> CueType 포함
-> target actor를 interface로 전달
-> defender CombatResolution에서 상태 / 입력 / cue 해석
-> Outcome: Hit / Guard / Parry / Blink / Repulse
-> defender / attacker / feedback / event로 분배
```

충돌이 있는 공격과 비충돌 cue는 target discovery 방식만 다르다. packet 처리 파이프라인은 가능하면 동일하게 둔다.

```text
Collision hit
-> overlap / hit event가 target을 찾음
-> combat packet 전달

Non-collision cue
-> cached target / lock-on target / targeting result가 target을 찾음
-> combat packet 전달
```

---

## 4. 비충돌 Cue 전달 기준

비충돌 cue 전달은 cached target actor를 interface로 변환해 packet을 직접 전달하는 방식이 적합하다.

```text
Cached target actor
-> combat packet receiver interface 확인
-> cue / source / timing / direction / damage spec 포함 packet 생성
-> target Combat Resolution 또는 TakeDamage 계층으로 전달
```

핵심 기준은 다음과 같다.

- Sender는 결과를 확정하지 않는다.
- Sender는 cue / source / timing / direction / damage spec을 전달한다.
- Receiver는 자기 상태와 입력을 기준으로 outcome을 결정한다.
- Targeted packet delivery는 interface를 우선한다.
- Subsystem은 target search / destination validation에 사용한다.

---

## 5. 시스템 책임 분리 기준

| 책임 | 권장 위치 | 예시 |
| --- | --- | --- |
| 공격 / cue packet 전달 | Interface | `ICombatPacketReceiver::ReceiveCombatPacket()` |
| target / destination 검색 | Subsystem 또는 Targeting Service | lock-on target 검색, Blink destination 검증 |
| 결과 판정 | CombatResolution 또는 target component | Hit / Guard / Parry / Blink / Repulse 판정 |
| 여러 시스템 알림 | Event Bus | Parry success, Blink success, Repulse success broadcast |
| 전역 규칙 / debug / registry | Subsystem | difficulty modifier, combat debug draw, combat actor registry |

기준은 다음과 같이 잡는다.

```text
누구에게 보낼지 이미 안다
-> Interface

누구에게 보낼지 찾아야 한다
-> Subsystem / Targeting Service

여러 곳이 알아야 한다
-> Event Bus

결과 판정이 필요하다
-> CombatResolution / target component

월드 단위 정책이 필요하다
-> Subsystem
```

---

## 6. Interface가 적합한 경우

Interface는 송신자와 수신자가 명확한 targeted delivery에 적합하다.

예시는 다음과 같다.

- melee overlap으로 찾은 target에게 combat packet 전달
- lock-on target에게 Blink / Repulse cue packet 전달
- Parry 성공 결과를 attacker에게 전달
- interaction target에게 `Interact()` 호출

이 경우 Event Bus나 Subsystem을 거치면 routing이 과해질 수 있다.

```text
Attacker knows target
-> target interface 호출
-> target 내부 CombatResolution이 packet 해석
```

---

## 7. Subsystem이 적합한 경우

Subsystem은 world-level query / validation / registry에 적합하다.

예시는 다음과 같다.

- Blink 이동 위치 검색
- lock-on 후보 검색
- 주변 enemy / valid target 필터링
- Repulse 가능한 attacker 후보 찾기
- combat debug draw on/off
- difficulty 기반 timing modifier

Subsystem은 packet delivery 자체보다 “누구에게 보낼지 찾는 일” 또는 “월드 단위 규칙을 제공하는 일”에 둔다.

---

## 8. Event Bus가 적합한 경우

Event Bus는 여러 시스템이 같은 사건을 관찰해야 할 때 적합하다.

예시는 다음과 같다.

- Parry 성공
- Blink 성공
- Repulse 성공
- Guard break
- enemy staggered
- player dead
- hit stop 발생

```text
CombatResolution
-> CombatEventBus.Broadcast(RepulseSucceeded)
-> UI / SFX / VFX / Camera / Debug timeline이 구독
```

Event Bus는 outcome을 판정하지 않는다. 이미 결정된 사건을 여러 observer에게 알리는 역할로 둔다.

---

## 9. Combat Resolution과 결과 분배

Combat Resolution은 cue packet이 현재 상태에서 어떤 outcome인지 판정한다.

```text
CueType = Blink
-> Blink input timing valid?
-> movement destination valid?
-> Outcome = Blink

CueType = Repulse
-> Repulse input timing valid?
-> attacker가 Repulse 가능한 상태인가?
-> Outcome = Repulse
```

판정 이후 실행은 각 owner component로 분배한다.

```text
Outcome = Blink
-> defender MovementComponent가 relocation 실행
-> feedback / event 알림

Outcome = Repulse
-> attacker result receiver에 counter result 전달
-> attacker Reaction / State / Feedback 처리
-> feedback / event 알림
```

Combat Resolution은 action / reaction / movement를 직접 실행하지 않는다. outcome과 필요한 request / result를 구성하고, 각 component / interface / event 경로로 분배한다.

---

## 10. W03 이후 후속 기준

W03에서는 Guard / Parry v1만 마무리한다.

후속 작업에서는 다음 순서로 확장하는 것이 적합하다.

1. Parry 성공 결과를 attacker에게 전달하는 interface 경계 정리
2. Combat packet에 cue type / source / direction / timing 정보를 담는 구조 검토
3. Blink destination 검색을 Targeting Service 또는 Subsystem 후보로 분리
4. Repulse 결과를 attacker state / weak point / stagger exposure로 연결
5. 성공 outcome을 Event Bus로 broadcast해 UI / SFX / VFX / Camera / Debug가 구독하게 구성

---

## 11. 관련 문서

- `Docs/01_Work_List/W03_Parry/W03_UE5_Portfolio_Work_List.md`
- `Docs/06_notes/N03_Guard_Hold_Overlay_Layer_Design_Note.md`
- `Docs/06_notes/archive/NA01_Combat_Intent_Request_Resolution_Routing_Design_Note.md`
- `Docs/05_System_Architecture/S27_UE5_Portfolio_System_Architecture.md`

---

## 12. Combat Request / Resolution / Consequence Pipeline

Blink / Repulse / Parry / Guard가 확장되면 기존 `ApplyDamageComponent`와 `TakeDamageComponent`만으로는 책임 경계가 흐려진다.

장기 구조에서는 전투 요청 생성, 수신, 판정, 후속 처리 분배를 다음과 같이 분리한다.

```text
Intent / Event
(Input, AI, System, Hit, Cue, TakeDamage adapter)
-> CombatRequester
-> CombatReceiver
-> CombatResolution
-> CombatConsequenceCoordinator
-> Action / Reaction Orchestrator
-> State / Resource
-> Feedback
-> CombatResult / Event
```

### 12.1 CombatRequester

`CombatRequester`는 전투 판정이 필요한 intent나 event를 `CombatRequestPacket`으로 만든다.

모든 intent가 `CombatRequester`를 타야 하는 것은 아니다.
단순 action 실행, UI, editor debug, local-only feedback처럼 전투 판정이 필요 없는 요청은 기존 action / system 경로를 직접 사용할 수 있다.

`CombatRequester`가 담당하는 요청은 다음과 같다.

- collision hit로 발생한 공격 요청
- lock-on target에게 전달하는 cue 요청
- Blink / Repulse / Parry counter 계열 요청
- 기존 `ApplyDamageComponent`가 target에게 전달하던 damage request

장기적으로 현재 `ApplyDamageComponent`는 `CombatRequester` 또는 `CombatRequestSource` 성격으로 축소된다.

### 12.2 CombatReceiver

`CombatReceiver`는 본인, 적, 타인 등 target actor가 combat request를 받는 경계다.

수신 방식은 상황에 따라 다를 수 있다.

```text
collision hit
-> target actor receiver

non-collision cue
-> cached target / lock-on target receiver

UE TakeDamage adapter
-> CombatReceiver로 변환
```

`CombatReceiver`는 packet을 받은 뒤 직접 결과를 확정하지 않고 `CombatResolution`으로 넘긴다.

### 12.3 CombatResolution

`CombatResolution`은 packet과 현재 상태를 해석해 전투 결과를 결정한다.

주요 책임은 다음과 같다.

- request 유효성 판단
- defender 상태 확인
- guard / parry / invincible / armor / cue timing 판단
- `Hit / Guard / BlockHit / Parry / Blink / Repulse / Dead / Ignore` outcome 결정
- damage commit 필요 여부 결정
- 필요한 reaction intent / feedback cue / attacker result 후보 구성

`CombatResolution`은 action, reaction, movement, feedback을 직접 실행하지 않는다.

예시는 다음과 같다.

```text
Outcome = Parry
-> DamageCommit = false
-> DefenderReactionIntent = Parry
-> AttackerResult = Parried
-> FeedbackCue = ParrySpark
```

이 단계에서 “Parry reaction을 지금 실행할 수 있는가”를 판단하지 않는다.
그 판단은 `ReactionOrchestrator`의 실행 충돌 / 개입 정책에 속한다.

### 12.4 CombatConsequenceCoordinator

`CombatConsequenceCoordinator`는 `CombatResolution` 결과를 받아 후속 책임으로 분배한다.

이 계층은 결과를 실행하지 않는다.
대신 결과를 각 도메인이 이해할 수 있는 request / packet으로 변환하고, 처리 순서를 조율한다.

```text
CombatResolutionResult
-> DamageApplyRequest
-> ReactionRequest
-> FeedbackRequest
-> CombatResultPacket
-> Event
```

이 계층은 `CombatResultRouting`보다 `CombatConsequenceCoordinator`라는 이름이 더 적절하다.

이유는 단순 전달만 하는 router가 아니라, resolved outcome의 후속 처리를 어떤 도메인으로 보낼지 조율하는 단계이기 때문이다.
다만 coordinator가 montage 재생, VFX spawn, HP commit 같은 실제 실행을 직접 처리해서는 안 된다.

### 12.5 Orchestrator

`ActionOrchestrator`와 `ReactionOrchestrator`는 intent의 의미를 판정하는 계층이 아니라 실행 충돌과 실행 가능성을 조율하는 계층이다.

예를 들어 `CombatResolution`이 `DefenderReactionIntent = Parry`를 만들면, `ReactionOrchestrator`는 다음을 판단한다.

- 현재 active action을 끊을 수 있는가
- 현재 active reaction과 충돌하는가
- intervention이 가능한가
- reserve / defer / ignore가 필요한가
- reaction을 실제로 시작할 수 있는가

즉 outcome의 의미는 `CombatResolution`이 결정하고, 실행 가능성과 충돌 조율은 `Orchestrator`가 결정한다.

### 12.6 State / Resource

상태와 자원 계층은 coordinator가 전달한 결과를 받아 실제 게임 상태를 변경한다.

예시는 다음과 같다.

- Health
- GuardGauge
- Stamina
- Posture
- WeakPoint
- DeadState

damage commit은 이 계층에서 처리한다.
`CombatResolution`은 damage commit 여부와 최종 적용 후보값을 만들 수 있지만, HP를 직접 깎지는 않는다.

### 12.7 Feedback

Feedback 계층은 combat result에 따른 감각적 결과를 재생한다.

예시는 다음과 같다.

- VFX
- SFX
- hit stop
- camera shake
- controller rumble
- UI indicator

Feedback 계층은 “Parry가 성공했는가”를 판정하지 않는다.
그 결과는 이미 `CombatResolution`에서 확정되어야 한다.

### 12.8 CombatResult / Event

어딘가로 결과를 다시 보내는 작업은 `CombatResultPacket` 또는 event로 처리한다.

예시는 다음과 같다.

```text
Parry success
-> attacker receiver에게 Parried result 전달
-> attacker가 필요하면 자기 쪽 ReactionRequest 생성
-> EventBus가 UI / tutorial / debug / analytics에 broadcast
```

외부로 전달된 결과가 다시 새로운 실행을 만들 경우, 그것은 새로운 intent 또는 event로 본다.

### 12.9 현재 컴포넌트와의 관계

현재 `ApplyDamageComponent`와 `TakeDamageComponent`는 위 책임이 압축된 형태다.

```text
ApplyDamageComponent
-> hit context 수집
-> damage spec 조회
-> target에게 damage request 전달

TakeDamageComponent
-> packet 수신
-> defensive outcome 판단
-> damage commit
-> reaction / feedback dispatch
-> attacker result dispatch
```

장기적으로는 다음처럼 분리한다.

```text
ApplyDamageComponent
-> CombatRequester / CombatRequestSource 후보

TakeDamageComponent
-> CombatReceiver / UE TakeDamage adapter 후보

TakeDamage 내부 defensive 판단
-> CombatResolution 후보

TakeDamage 내부 reaction / feedback / attacker result dispatch
-> CombatConsequenceCoordinator 후보

Health commit
-> Health / DamageApply 계층 유지
```

따라서 W03 이후 리팩터링은 `TakeDamageComponent`를 한 번에 해체하기보다, 먼저 `CombatResolutionResult`와 `CombatConsequenceCoordinator` 경계를 만들고 기존 reaction / feedback / result dispatch를 그 경계로 옮기는 방향이 적절하다.
