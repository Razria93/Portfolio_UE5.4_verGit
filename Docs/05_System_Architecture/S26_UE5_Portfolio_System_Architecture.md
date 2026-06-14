# 실행 몽타주 생명주기 결정

## 1. 목적

본 문서는 action / reaction executor의 montage lifecycle에서 `Complete`, `Stop`, `MontageEnd`의 책임을 어떻게 나누어야 하는지 정리하기 위한 문서임.

핵심은 자연 종료, 외부 중단, 엔진 콜백을 구분하고, component의 active state 정리와 executor의 montage lifecycle 처리를 명확히 분리하는 것임.

---

## 2. 관련 브랜치

- `orchestration-refactor`

---

## 3. 이전 시스템의 형태

### Montage 기반 execution Flow

Montage 기반 execution은 다음 흐름을 가짐.

```yaml
Start
-> Montage_Play
-> Notify 또는 MontageEnd
-> Complete / Stop / Finish
```

기존 구조에서는 complete notify, stop 요청, montage end delegate가 모두 종료 흐름에 관여할 수 있었음.

### 종료 흐름 종류

```yaml
MontageEnd
-> 엔진 콜백

Complete
-> 자연 종료

Stop
-> 외부 요청에 의한 중단
```

---

## 4. 이전 시스템의 문제 분석 및 한계

### Stop 이후 MontageEnd 중복 가능성

Stop 이후에도 montage end delegate가 호출될 수 있음.

따라서 Stop과 MontageEnd 양쪽에서 finish를 처리하면 중복 종료가 발생할 수 있음.

반대로 montage delegate만 기다리면 명시적 stop 요청의 결과가 불명확해질 수 있음.

또한 executor는 montage lifecycle을 가장 잘 알고 있지만, active context와 execution state는 component가 소유함.

### Executor / Component 책임 혼재

이 때문에 다음 책임이 섞이기 쉬웠음.

```yaml
Executor
-> montage stop / complete 처리

Component
-> active context clear
-> state transition
```

---

## 5. 리팩터링 방안 제안

### Executor / Component 책임 분리

권장 책임은 다음과 같음.

```yaml
Executor
-> montage play / stop / complete
-> notify command 처리
-> stop reason 기록
-> component에 finish 알림

Component
-> active context 소유
-> execution state 갱신
-> active context clear
-> fallback end 처리
```

Stop, Complete, MontageEnd는 다음 기준으로 구분함.

### 종료 유형 구분

```yaml
Complete
-> 자연 종료
-> 정상 execution completion

Stop
-> 외부 요청에 의한 중단
-> interrupted / cancelled / ignored reason 포함

MontageEnd
-> engine callback
-> 이미 처리된 stop의 중복 정리를 막아야 함
```

Component는 executor가 finish를 알리면 active state를 정리함.

Executor가 invalid하거나 stop 이후에도 active state가 남아 있으면 component가 fallback으로 정리할 수 있음.

---

## 6. 시행착오 과정

처음에는 `Stop()`이 montage stop만 해야 하는지, finish까지 동기적으로 처리해야 하는지 애매했음.

Montage_Stop이 blend-out 이후 delegate를 호출할 수 있기 때문에, Stop 내부에서 finish를 처리하면 이후 MontageEnd가 다시 들어올 수 있음.

반대로 Stop이 delegate만 기다리면 중단 요청이 즉시 반영되지 않아 active state가 어색하게 남을 수 있음.

따라서 stop 요청은 명시적으로 처리하되, MontageEnd에서는 이미 처리된 중단을 중복 처리하지 않도록 guard와 log 기준이 필요함.

---

## 7. 결론

Montage lifecycle은 다음 원칙을 기준으로 정리하는 것이 적절함.

```yaml
Complete
-> 자연 종료

Stop
-> 외부 요청에 의한 중단

MontageEnd
-> engine callback이며 중복 종료를 방지해야 함
```

Executor는 montage lifecycle을 담당하고, component는 active runtime state를 담당함.

Fallback 처리와 unexpected interruption log는 유지하되, 정상 stop 흐름과 비정상 montage callback 흐름을 구분해야 함.








