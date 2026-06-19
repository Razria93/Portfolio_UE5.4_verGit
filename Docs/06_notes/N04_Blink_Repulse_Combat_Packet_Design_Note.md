# Blink / Repulse Combat Packet Design Note

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
- `Docs/05_System_Architecture/S27_UE5_Portfolio_System_Architecture.md`
