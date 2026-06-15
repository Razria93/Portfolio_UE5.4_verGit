# Guard Release Deferred Action Candidate Note

## 1. 목적

본 문서는 `Guard In` 실행 중 `Guard Released` 입력이 들어왔을 때 발생할 수 있는 상태 고정 문제와, 이를 해결하기 위한 deferred action candidate 구조 필요성을 기록한다.

현재 브랜치에서는 우선 Guard / Parry v1 동작을 안정화한다.
공통 Orchestrator 수준의 deferred action candidate consume 구조는 현재 브랜치에서 Guard release 문제를 기준으로 최소 도입한다.

---

## 2. 발견된 문제

`Block_In`이 끝나기 전에 Guard key가 release되면 `Block_Out` 요청이 현재 실행 중인 `Block_In`과 충돌할 수 있다.

이때 release 입력이 처리되지 못하면 다음 상태가 남을 수 있다.

```text
Pressed
-> Block_In 실행
-> Guard pose 시작
-> Parry window 시작

Released
-> Block_Out 요청 발생
-> Block_In 실행 중이라 즉시 실행되지 못함

Block_In 종료
-> Guard pose가 유지됨
-> Hold 상태에 고정될 수 있음
```

---

## 3. 구조 원인

이 문제는 단순히 `Block_Out` 호출 위치를 조정하는 문제만은 아니다.

핵심 원인은 다음 요청 유형을 처리할 공통 경로가 아직 없다는 점이다.

```text
현재 실행 중인 action 때문에 지금은 실행할 수 없지만,
사용자 입력 의도는 버리면 안 되는 요청
```

`Guard Released`는 새 combo 입력처럼 다시 들어오는 요청이 아니라, 이미 들어온 종료 의도를 안전한 시점까지 보관해야 하는 요청이다.

---

## 4. Combo 재호출과의 차이

`ComboAttack`의 다음 단계는 재입력 또는 follow-up request를 통해 다시 요청된다.

```text
ChainWindowOpened
-> Player 재입력 또는 AI follow-up request
-> RequestCombatAction 재호출
```

반면 `Guard Released during Block_In`은 이미 release 입력이 들어온 상태다.

```text
Released 입력
-> Guard Completed request 생성
-> 현재는 실행 불가
-> request 보관
-> Block_In 완료 시점에 자동 소비
```

따라서 Guard release 문제는 단순 재호출보다 resolved candidate를 보관하고, consume trigger 시점에 다시 처리하는 deferred action candidate 구조에 가깝다.

---

## 5. 단순 실행의 위험

보관된 `Block_Out` 요청을 완료 시점에 바로 실행하면 기존 action request pipeline의 검증을 우회할 수 있다.

위험한 예시는 다음과 같다.

- `Block_In` 중 피격되어 reaction takeover가 발생했는데, 이후 `Block_Out`이 reaction을 덮어쓴다.
- 사망 상태로 들어갔는데, 완료 시점에 `Block_Out` montage가 실행된다.
- dodge / cinematic / input lock 상태에서 보관된 `Block_Out`이 실행된다.
- weapon state나 action data가 바뀐 뒤 과거 request가 현재 상태와 맞지 않게 실행된다.
- owner 또는 component가 invalid 상태인데 보관된 request가 실행된다.

따라서 deferred action candidate는 소비 시점에도 현재 상태 기준의 orchestration 판단을 다시 통과해야 한다.

---

## 6. 권장 구조

장기적으로는 Orchestrator가 deferred action candidate를 관리하는 구조가 적합하다.

```text
Action Request
-> Action Candidate resolve
-> 즉시 처리 가능?
   - Yes -> 실행
   - No, 버릴 요청인가? -> Reject / Ignore
   - No, 나중에 소비할 수 있는 candidate인가? -> Deferred Candidate 저장

Action lifecycle event
-> Consume trigger 발생
-> Deferred candidate consume
-> 가능하면 실행
-> 불가능하면 Reject / Ignore / Expire
```

Guard release 기준의 예시는 다음과 같다.

```text
Guard Released
-> FCombatActionRequest(Guard, Completed)
-> FActionCandidate(Guard, index 2) resolve
-> 현재 Block_In 실행 중이라 GuardInCompleted key로 deferred 저장

Block_In Complete 또는 exit notify
-> ConsumeDeferredAction(GuardInCompleted)
-> 저장된 Guard Out candidate를 공통 ProcessActionCandidate 경로로 재처리
-> 가능하면 Block_Out 실행
```

중요한 점은 notify가 `Block_Out`을 직접 실행하지 않는 것이다.

notify 또는 action lifecycle event는 소비 가능한 시점을 알려주고, Orchestrator가 보관된 candidate를 다시 처리한다.

---

## 7. v1 대응 방향

현재 Guard / Parry v1에서는 Guard release 문제를 기준으로 deferred action candidate 저장 / 소비 구조를 최소 도입한다.

```text
Released 입력
-> Guard Out candidate resolve
-> 현재 Guard In 실행 중인가?
   - Yes -> GuardInCompleted key로 deferred candidate 저장
   - No  -> 공통 ProcessActionCandidate 경로로 즉시 처리

Guard In Complete 또는 지정 notify
-> GuardInCompleted deferred candidate가 있는가?
   - Yes -> candidate consume 후 공통 ProcessActionCandidate 경로로 재처리
   - No  -> Guard Hold 유지
```

이 방식은 Guard 동작을 먼저 안정화하면서, 이후 다른 action에도 같은 consume key 모델을 확장할 근거를 남긴다.

---

## 8. 후속 작업 후보

- deferred action candidate 만료 / 우선순위 정책 검토
- `GuardInCompleted` 또는 `GuardCanExit` consume trigger 후보 검토
- Combo reserved / Guard deferred / Dodge cancel / Parry counter를 하나의 delayed execution 모델로 통합할 수 있는지 검토
- System Architecture 재분류 이후 System Design Record로 승격 여부 판단

---

## 9. 현재 결론

`Guard Released during Block_In` 문제는 단순 입력 누락 버그가 아니라, action lifecycle 중 지연 처리해야 하는 candidate를 어떻게 보관하고 안전하게 소비할지에 대한 구조 문제다.

현재 브랜치에서는 Guard v1을 우선 안정화하고, deferred action candidate 구조는 Orchestrator의 확장 지점으로 유지한다.
