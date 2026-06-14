# 실행 개입 키 윈도우 모델

## 1. 목적

본 문서는 Action / Reaction execution 간 intervention window를 설정하고 소비하는 구조를 정리한다.

핵심은 montage notify에서 “누가”, “무엇을 어떻게”, “누구에게” 개입할 수 있는지를 명확하게 작성하고, runtime executor는 이를 Want / Allow 및 Cancel / Interrupt 컨테이너로 분리해서 판단하는 것이다.

---

## 2. 관련 브랜치

- `intervention-policy-refactor`

---

## 3. 이전 시스템의 형태

기존 구조는 interrupt / cancel 가능 여부를 broad window로 열고 닫는 방식에 가까웠다.

```yaml
InterruptWindow
CancelWindow
```

이 방식은 “가능하다 / 불가능하다”는 표현할 수 있지만, 다음을 명확히 표현하기 어려웠다.

```yaml
무엇에 의해 cancel 가능한가
무엇에 의해 interrupt 가능한가
내가 끊고 싶은 것인가
내가 끊겨도 되는 것인가
```

또한 Source / Target을 완성된 key 형태로 직접 노출하면, notify가 붙은 montage의 owner와 무관한 source / target을 잘못 입력할 수 있었다.

---

## 4. 이전 시스템의 문제 분석 및 한계

### 1) Source / Target 직접 입력의 혼란

Intervention 관계는 다음 요소로 구성된다.

```yaml
누가
누구에게
무엇을
어떻게
```

하지만 notify가 붙은 montage에서는 “내가”가 이미 정해져 있다.

따라서 editor에서 Source와 Target을 모두 직접 입력하게 하면, montage owner와 무관한 source를 입력할 수 있다.

### 2) Want / Allow에 따라 “내가”의 위치가 바뀜

Want window에서는 내가 Source다.

```yaml
내가 상대를 끊고 싶다
```

Allow window에서는 내가 Target이다.

```yaml
나는 상대에 의해 끊겨도 된다
```

따라서 notify authoring에서는 Source / Target을 직접 노출하기보다, Owner와 Counterpart를 나누고 WindowRole이 Source / Target 위치를 결정하는 편이 명확하다.

### 3) StopReason과 WindowRole은 window의 성격

하나의 window는 하나의 성격만 가져야 한다.

```yaml
Cancel Allow
Cancel Want
Interrupt Allow
Interrupt Want
```

따라서 StopReason과 WindowRole은 단일 값으로 두고, counterpart만 배열로 받는 것이 읽기 쉽다.

---

## 5. 리팩터링 방향 및 내용

### 1) Participant Filter 도입

Intervention 대상 표현은 `FExecutionInterventionParticipantFilter`로 통일한다.

```yaml
Domain
ActionType
ReactionType
Index
```

이 구조체는 Action / Reaction participant를 같은 방식으로 매칭하기 위한 filter다.

`ActionType::All`, `ReactionType::All`은 wildcard로 사용한다.

Action filter에서 `Index == INDEX_NONE`은 index wildcard로 사용한다.

```yaml
ActionType: ComboAttack
Index: INDEX_NONE
-> 모든 ComboAttack index와 매칭

ActionType: ComboAttack
Index: 1
-> ComboAttack 1번 index와만 매칭
```

Reaction filter는 현재 index 개념이 없으므로 `ReactionType`만 매칭한다.

코드 API와 notify authoring 순서는 동일하게 다음 흐름을 따른다.

```yaml
OwnerFilter
StopReason
WindowRole
CounterpartFilters
```

이는 “누가 / 무엇을 / 어떻게 / 대상” 순서로 읽기 위한 것이다.

### 2) Notify Authoring Model

단일 notify state는 다음 순서로 필드를 노출한다.

```yaml
Owner
-> OwnerFilter

Window
-> StopReason
-> WindowRole

Counterpart
-> CounterpartFilters
```

읽는 순서는 다음과 같다.

```yaml
누가
무엇을 어떻게 한다
대상
```

예를 들어 HitReaction montage에서 Dodge cancel을 허용하려면 다음처럼 작성한다.

```yaml
OwnerFilter
-> Domain: Reaction
-> ReactionType: Hit

StopReason
-> Cancelled

WindowRole
-> Allow

CounterpartFilters[0]
-> Domain: Action
-> ActionType: Dodge
-> Index: INDEX_NONE
```

의미는 다음과 같다.

```yaml
Reaction:Hit은 Action:Dodge에 의해 Cancelled 되는 것을 허용한다.
```

### 3) Runtime Container Model

Executor는 window 성격에 따라 4개 컨테이너를 가진다.

```yaml
WantCancelFilters
WantInterruptFilters
AllowCancelFilters
AllowInterruptFilters
```

Notify begin 시점에는 `StopReason + WindowRole`로 window 성격을 정하고, 이에 맞는 컨테이너를 선택해 `CounterpartFilters`를 추가한다.

Notify end 시점에는 같은 컨테이너에서 같은 filter를 제거한다.

### 4) Matching Model

Intervention query가 들어오면 incoming과 active가 각각 판단한다.

Want 판단:

```yaml
내가 incoming/source
상대는 active/target
StopReason에 맞는 Want 컨테이너에서 ActivePart를 매칭
```

Allow 판단:

```yaml
내가 active/target
상대는 incoming/source
StopReason에 맞는 Allow 컨테이너에서 IncomingPart를 매칭
```

최종 intervention은 incoming Want와 active Allow가 모두 true일 때 성립한다.

### 5) Notify Name / Color

Notify 이름은 설정값을 기반으로 표시한다.

```yaml
Intervention Window (Cancel Allow)
Intervention Window (Cancel Want)
Intervention Window (Interrupt Allow)
Intervention Window (Interrupt Want)
```

색상도 StopReason과 WindowRole 조합에 따라 다르게 표시한다.

```yaml
Cancel Allow
Cancel Want
Interrupt Allow
Interrupt Want
```

이는 editor에서 window 성격을 빠르게 구분하기 위한 장치다.

---

## 6. 시행착오 과정

초기에는 `FExecutionInterventionKey`처럼 Source / Target / StopReason을 모두 가진 완성 key를 notify에 노출하는 방향을 고려했다.

하지만 이 방식은 notify가 붙은 montage의 owner와 무관한 source / target을 작성할 수 있어서 authoring 실수를 유발한다.

이후 “내가”의 위치가 Want / Allow에 따라 달라진다는 점을 기준으로 구조를 다시 나누었다.

```yaml
Want
-> Owner가 Source
-> Counterpart가 Target

Allow
-> Counterpart가 Source
-> Owner가 Target
```

결과적으로 notify는 OwnerFilter와 CounterpartFilters만 노출하고, WindowRole이 source / target 위치를 결정하게 되었다.

---

## 7. 결론

현재 intervention window는 다음 모델로 정리된다.

```yaml
OwnerFilter
-> 이 notify를 소비할 execution

StopReason + WindowRole
-> window 성격

CounterpartFilters
-> 이 window가 대상으로 삼는 상대 execution들
```

이 구조는 broad interrupt / cancel window보다 구체적이고, 완성된 Source / Target key를 editor에 직접 노출하는 방식보다 안전하다.

이후 Dodge / Guard / Parry / Counter 같은 변칙 execution도 같은 window model 위에서 확장한다.








