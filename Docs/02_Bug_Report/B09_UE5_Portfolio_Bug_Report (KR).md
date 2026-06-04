# UE5 Portfolio Bug Report (KR)

## 제목

**M05-B09: Action / Reaction intervention window 설정 누락 및 notify 구간 문제로 active execution이 중단되지 않음**

### Date

- **2026.05.22**

### Type

- Bug

### Status

- [ ] In Progress

### Branch

- `feature/orchestration-refactor`

---

## 요약

- Action / Reaction intervention 구조에서 active side의 `Allow` window가 명시적으로 설정되지 않으면 incoming execution이 active execution을 중단하지 못하는 문제가 발생함.
- 대표 증상은 공격 Action 중 피격되어도 Action montage가 끊기지 않고 계속 진행되는 문제와, active `HitReaction` 중 새 `HitReaction`이 들어와도 기존 reaction이 interrupt되지 않고 request가 reject되는 문제였음.
- 원인은 active execution 쪽 `Allow` filter 누락, override 내부 `Super` 호출로 인한 판단 경로 혼재, montage 0 frame부터 마지막 frame까지 배치한 intervention notify state의 begin/end 타이밍 문제였음.

---

## 환경

- Engine: Unreal Engine 5.4
- Branch:
  - `feature/orchestration-refactor`

### Related Code

- `Source/Portfolio/Action/CAction.cpp`
- `Source/Portfolio/Reaction/CReaction.cpp`
- `Source/Portfolio/Reaction/CReaction_Hit.cpp`
- `Source/Portfolio/Reaction/CReaction_Dead.cpp`
- `Source/Portfolio/Component/CActionOrchestratorComponent.cpp`
- `Source/Portfolio/Component/CReactionOrchestratorComponent.cpp`
- `Source/Portfolio/Notify/CAnimNotifyState_ExecutionInterventionWindow.cpp`

### Related Assets

- `Content/04_Montage/Sword/M_Attack_Sword_0.uasset`
- `Content/04_Montage/Sword/M_Attack_Sword_1.uasset`
- `Content/04_Montage/Sword/M_Attack_Sword_2.uasset`
- `Content/04_Montage/Damaged/M_HitReact.uasset`

---

## 재현 방법

1. Enemy가 `ComboAttack` 0 / 1 / 2타를 순차적으로 실행하도록 함.
2. Player가 피격되어 `HitReaction`이 발생하도록 함.
3. `HitReaction` montage에 intervention allow window를 설정함.
4. notify state 구간을 0 frame부터 마지막 frame까지 설정한 경우와 1 frame부터 마지막 - 1 frame까지 설정한 경우로 비교함.
5. 2타 피격 시 `HitReaction -> HitReaction` interrupt가 정상 처리되는지 확인함.
6. 공격 Action 중 피격되었을 때 Action montage가 `HitReaction`에 의해 interrupt되는지 확인함.

---

## 기대 결과

- Action 실행 중 Hit reaction이 들어오면 active Action이 interrupt되어야 함.
- active `HitReaction` 중 새 `HitReaction`이 들어오면 현재 reaction이 interrupt 가능 상태일 때 새 `HitReaction`으로 교체되어야 함.
- intervention 판단은 incoming execution의 Want, active execution의 Allow, active side allow filter cache가 모두 성립할 때 성공해야 함.

---

## 실제 결과

### 1. Action이 HitReaction에 의해 중단되지 않음

- 공격 Action에 `Allow Interrupt by HitReaction` window를 설정하지 않은 상태에서는 피격되어도 Action montage가 슈퍼아머처럼 계속 진행됨.
- Action 쪽에 interrupt allow window를 `HitReaction` 대상으로 설정한 뒤에야 Action이 정상적으로 중단됨.

### 2. HitReaction이 다시 들어온 HitReaction에 의해 중단되지 않음

- active `HitReaction` 중 `ComboAttack` 2타에 의해 새 `HitReaction`이 들어왔을 때 incoming side의 want 판정은 true였지만 active side의 allow 판정이 false가 되어 request가 reject됨.

```text
[ResolveInterventionDirective]
bIncomingWants = true
bActiveAllows = false

[RequestDamageReaction]
ResultType = Rejected
RejectReason = ActiveCannotAcceptIntervention
```

### 3. notify state 구간을 조정하면 filter matching이 성공함

- 같은 상황에서 notify state를 1 frame부터 마지막 - 1 frame까지 배치하면 `CounterpartFilters`가 정상적으로 유지되고 matching이 성공함.

```text
[UCReaction::MatchesAnyInterventionFilter] Match Complete.
```

---

## 원인 분석

### 1. Allow window는 active execution 쪽의 책임임

- `Want`는 incoming execution이 무엇을 끊고 싶은지를 표현함.
- `Allow`는 active execution이 무엇에 의해 끊겨도 되는지를 표현함.
- 따라서 Hit reaction이 action을 interrupt하려면 incoming `HitReaction` 쪽 want interrupt와 active `Action` 쪽 allow interrupt by `HitReaction`이 모두 필요함.

### 2. HitReaction 재진입도 active HitReaction의 Allow가 필요함

- active `HitReaction` 중 새 `HitReaction`이 들어오는 경우에도 active reaction 쪽 allow filter가 필요함.
- incoming executor가 interrupt를 원하더라도 active executor가 중단을 허용하지 않으면 intervention은 실패함.

### 3. override 내부에서 Super 호출이 판단 경로를 흐리게 함

- 특정 reaction이 intervention 판단을 override했더라도 내부에서 `Super`를 호출하면 base filter matching 경로를 함께 거침.
- 이 방식은 공통 filter와 class 고정 정책을 결합할 수 있지만, 디버깅 시 어떤 경로로 통과했는지 구분하기 어려움.

### 4. montage 0 frame ~ last frame notify state는 begin/end 타이밍 문제가 생길 수 있음

- notify state를 montage 전체 구간에 꽉 채우면 montage 전환이나 chain consume 과정에서 notify begin/end 호출 타이밍이 기대와 다르게 동작할 수 있음.
- 관찰된 현상은 0타와 1타에서는 filter가 유지되지만 2타에서는 filter가 캐싱되지 않아 allow matching이 실패하는 형태였음.

---

## 수정 방향

### 1. Action / Reaction 모두 필요한 Allow window를 명시적으로 설정

- 공격 Action이 Hit reaction에 의해 끊겨야 한다면 Action montage에 Hit reaction 대상 allow window를 설정해야 함.
- Hit reaction이 다시 Hit reaction에 의해 interrupt되어야 한다면 HitReaction montage에도 대응되는 allow window를 설정해야 함.

### 2. override 정책에서는 Super 호출 여부를 명확히 결정

- 특정 executor가 자기 정책을 명확히 소유해야 한다면 `Super` 호출을 제거해야 함.
- notify filter 기반 정책과 class 고정 정책을 함께 쓰려면 로그와 주석으로 두 경로를 명확히 분리해야 함.

### 3. intervention notify state는 0 frame과 마지막 frame을 피해서 배치

- montage 전체 구간을 덮어야 하는 경우에도 notify state를 0 frame부터 마지막 frame까지 꽉 채우지 않음.
- 권장 배치는 1 frame 이후부터 마지막 frame 이전까지임.

---

## 검증 기준

- 공격 Action 중 `HitReaction`이 들어오면 active Action이 interrupt되는지 확인함.
- active `HitReaction` 중 새 `HitReaction`이 들어왔을 때 `bIncomingWants = true`, `bActiveAllows = true`가 되는지 확인함.
- `MatchesAnyInterventionFilter`에서 expected counterpart filter가 출력되고 match complete가 찍히는지 확인함.
- `0 frame ~ last frame` 배치와 `1 frame ~ last - 1 frame` 배치를 비교하여 notify state 구간에 따른 cache 차이를 확인함.

---

## Notes

- 이번 문제의 핵심은 Hit reaction이 interrupt를 원한다는 사실만으로는 충분하지 않다는 점임.
- intervention은 incoming Want와 active Allow가 동시에 성립해야 함.
- Action이 끊기지 않았던 문제와 HitReaction이 다시 끊기지 않았던 문제는 모두 active side allow 설정 누락 또는 allow filter cache 실패로 설명됨.
- 따라서 이 버그는 단순 montage 문제가 아니라 intervention window model의 책임 분리를 검증한 사례에 가까움.

---
