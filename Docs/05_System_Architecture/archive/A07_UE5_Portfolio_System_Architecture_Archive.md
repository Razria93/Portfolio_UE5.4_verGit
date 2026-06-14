# Reaction Lifecycle 모델

## 1. 목적

본 문서는 reaction 실행 생명주기에서 `ReactionOrchestrator`, `ReactionComponent`, `CReaction`이 각각 어떤 책임을 갖는지 정리하기 위한 문서임.

이번 reaction orchestration 작업의 핵심은 단순히 reaction 실행 요청 경로를 추가하는 것이 아님.

실제 핵심은 reaction 실행 판단, active state 관리, montage 실행, stop / finish 처리, feedback 요청의 책임을 분리하는 것임.

따라서 reaction lifecycle을 명확히 정의하지 않으면 `Start`, `Interrupt`, `Cancel`, `Stop`, `Finish`, `MontageEnd`의 의미가 섞이기 쉬움.

본 문서는 해당 용어와 책임을 고정하여 이후 action lifecycle 개선 작업에서도 기준으로 사용할 수 있게 하는 것을 목적으로 함.

---

## 2. 전체 흐름

현재 reaction 실행 흐름은 다음과 같음.

```text
TakeDamage
-> ReactionOrchestrator
-> ReactionComponent
-> CReaction
-> Montage / Notify / Feedback
-> CReaction finish
-> ReactionComponent active state cleanup
```

각 계층의 기본 책임은 다음과 같음.

```text
ReactionOrchestrator
-> reaction request를 해석함
-> 실행 context / policy를 resolve함
-> Start / Interrupt / Cancel / Reject decision을 결정함

ReactionComponent
-> orchestration decision을 적용함
-> active reaction state를 관리함
-> reaction executor의 Start / Stop을 호출함
-> executor finish callback을 받아 active state를 정리함

CReaction
-> 실제 montage를 실행함
-> reaction control window를 처리함
-> reaction feedback request를 생성함
-> Stop 요청을 finish reason으로 확정함
-> Completed / Interrupted / Cancelled / Aborted finish를 수행함
```

즉 orchestrator는 판단 계층이고, component는 실행 상태 관리 계층이며, reaction executor는 실제 실행 계층임.

---

## 3. Orchestrator의 책임

`ReactionOrchestrator`는 reaction 요청이 현재 body/runtime state에서 어떤 방식으로 처리되어야 하는지 결정함.

주요 책임은 다음과 같음.

- `TakeDamage` 기반 request를 reaction context로 변환함
- damage result를 기반으로 reaction type을 결정함
- reaction data와 executor를 resolve함
- 현재 active reaction과 incoming reaction의 경쟁 상태를 판단함
- resolved policy와 executor hook을 조합해 decision을 생성함
- decision을 `ReactionComponent`에 dispatch함

Orchestrator는 reaction을 직접 실행하지 않음.

또한 montage를 재생하거나 active state를 직접 지우지 않음.

이는 실행 상태와 executor 제어 책임을 `ReactionComponent`에 유지하기 위함임.

---

## 4. Component의 책임

`ReactionComponent`는 orchestrator가 내린 decision을 실제 실행 상태 변화로 적용함.

주요 책임은 다음과 같음.

- active reaction context를 저장함
- active reaction executor를 조회함
- Start decision을 active reaction 시작으로 변환함
- Interrupt decision을 active reaction stop 후 incoming reaction 시작으로 변환함
- Cancel decision을 active reaction stop 후 다음 처리로 변환함
- stale active state를 정리함
- executor가 finish를 알리면 active reaction state를 정리함

`ReactionComponent`는 reaction 실행 가능 여부의 최종 판단자가 아님.

실행 가능 여부와 경쟁 상태 판단은 orchestrator가 담당함.

Component는 decision을 적용하고 active state를 일관되게 유지하는 역할을 담당함.

---

## 5. CReaction의 책임

`CReaction`은 reaction executor임.

따라서 실제 실행과 실행 내부 상태를 담당함.

주요 책임은 다음과 같음.

- reaction montage를 재생함
- reaction 시작 feedback을 요청함
- reaction control window를 열고 닫음
- reaction feedback notify를 처리함
- interruptible / cancelable 상태를 보유함
- local interruption / cancel rule을 제공함
- Stop 요청을 받은 경우 finish reason을 확정함
- finish reason에 맞는 feedback을 요청함
- finish 이후 component에 종료를 알림

`CReaction`은 active reaction slot을 직접 관리하지 않음.

Active reaction slot은 `ReactionComponent`가 관리함.

따라서 executor는 자신이 종료되었다는 사실과 종료 사유만 component에 알려야 함.

---

## 6. Start / Interrupt / Cancel의 의미

`Start`, `Interrupt`, `Cancel`은 외부에서 reaction execution state를 제어하기 위한 command 성격의 entry임.

```text
Start
-> active reaction이 없을 때 incoming reaction을 시작함

Interrupt
-> active reaction을 외부 요인으로 중단하고 incoming reaction으로 교체함

Cancel
-> active reaction을 사용자의 의도나 상위 상태 전환으로 중단함
```

따라서 `Start`와 `Interrupt`는 같은 함수로 합치지 않는 것이 적절함.

둘 다 결과적으로 executor를 시작할 수 있지만, 의미와 선행 조건이 다름.

`Start`는 active reaction이 없어야 하는 entry임.

`Interrupt`는 active reaction이 존재하는 상태에서 기존 reaction을 중단하고 새로운 reaction으로 전환하는 entry임.

`Cancel`은 incoming reaction 시작 여부와 무관하게 현재 active reaction을 중단하는 entry로 확장될 수 있음.

---

## 7. Stop과 Finish의 차이

`Stop`은 실행 중인 reaction executor에게 중단을 요청하는 API임.

`Finish`는 executor가 자신의 종료 사유를 확정하고 종료 절차를 수행하는 단계임.

즉 둘은 같은 개념이 아님.

```text
Stop
-> 외부에서 executor에게 중단을 요청함
-> Interrupted / Cancelled / Aborted 같은 stop reason을 전달함

Finish
-> executor가 종료 사유를 확정함
-> feedback을 요청함
-> runtime state를 정리함
-> component에 종료를 알림
```

이 구조에서는 `ReactionComponent`가 `Stop`을 호출하고, `CReaction`이 `FinishInterrupted`, `FinishCancelled`, `FinishAborted` 중 하나로 종료를 확정함.

따라서 stop 요청은 component의 책임이고, finish 확정은 executor의 책임임.

---

## 8. MontageEnd의 역할

`MontageEnd`는 montage가 정상적으로 끝났는지 감지하는 callback임.

현재 구조에서 시스템에 의한 stop은 `UCReaction::Stop()`에서 즉시 finish reason으로 확정됨.

따라서 stop 이후 발생하는 montage interrupted callback은 reaction 종료를 다시 확정하지 않음.

권장 의미는 다음과 같음.

```text
MontageEnd with bInterrupted == false
-> normal completed finish로 처리함

MontageEnd with bInterrupted == true
-> 이미 Stop에서 처리된 중단 흐름으로 간주함
-> 별도 finish를 재수행하지 않음
```

이는 montage blend-out 타이밍이나 engine callback 순서에 reaction state cleanup이 의존하지 않게 하기 위함임.

Reaction state는 gameplay state이므로 animation callback보다 명시적인 system command를 우선함.

---

## 9. Finish Reason

현재 reaction finish reason은 다음과 같이 구분함.

```text
Completed
-> montage가 정상 완료됨

Interrupted
-> 외부 reaction 또는 damage response에 의해 중단됨

Cancelled
-> cancel command 또는 상위 상태 전환에 의해 중단됨

Aborted
-> invalid runtime state 또는 시스템 오류성 중단으로 종료됨
```

`Interrupted`와 `Cancelled`는 의도적으로 분리함.

Interrupted는 외부 요인에 의해 현재 reaction이 밀려난 경우에 가까움.

Cancelled는 사용자 입력, dodge, counter, body state transition 등 현재 실행을 의도적으로 접는 경우에 가까움.

Aborted는 정상 gameplay decision이라기보다 실행을 유지할 수 없는 상태를 정리하는 의미에 가까움.

---

## 10. Feedback 책임

Reaction lifecycle에서 feedback 요청은 executor가 수행함.

이는 feedback timing이 reaction 실행 내부의 event와 밀접하게 연결되어 있기 때문임.

예시는 다음과 같음.

```text
ReactionStart
-> CReaction::Start()

ReactionCompleted
-> CReaction::FinishCompleted()

ReactionInterrupted
-> CReaction::FinishInterrupted()

ReactionCancelled
-> CReaction::FinishCancelled()

WindowBegin / WindowEnd
-> reaction feedback notify state

Notify
-> reaction feedback point notify
```

`ReactionComponent`는 feedback을 직접 조합하지 않음.

Component는 active executor를 찾아 notify event를 전달하는 bridge 역할만 수행함.

실제 feedback request는 `CReaction`이 자신의 active context를 기반으로 생성함.

---

## 11. 결론

Reaction lifecycle의 핵심은 판단, 상태 관리, 실행을 분리하는 것임.

```text
ReactionOrchestrator
-> 판단함

ReactionComponent
-> 실행 상태를 관리함

CReaction
-> 실제 실행하고 종료를 확정함
```

이 구조를 유지하면 reaction 간 경쟁 상태는 orchestrator에서 처리하고, active reaction slot은 component가 관리하며, montage / notify / feedback은 executor가 처리할 수 있음.

따라서 reaction orchestration은 단순한 request forwarding이 아니라, runtime execution lifecycle을 안정적으로 분리하기 위한 구조임.
