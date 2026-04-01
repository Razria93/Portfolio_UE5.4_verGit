# 인지 - 입력 - 상태변환 - 액션 실행 구조 정리

## 1. 목적

본 문서는 `Player`와 `Enemy`가 서로 다른 제어 방식으로 동작하더라도,  
실행 파이프라인 관점에서는 동일하게 다음 흐름을 따른다는 점을 정리하기 위한 문서임.

`인지 -> 입력(Intent) -> 상태변환 -> 액션 실행`

이 문서를 통해 각 계층의 책임을 분리하고, 공용 전투 컴포넌트가 특정 엔티티 구현에 과도하게 결합되지 않도록 설계 기준을 명확히 하는 것이 목적임.


---

## 2. 핵심 개념

### 2.1 인지(Perception / Awareness)

인지 단계는 현재 캐릭터가 외부와 내부 상황을 해석하기 위한 입력 데이터를 수집하는 단계임.

예시:

- 주변 타겟 존재 여부
- 거리 / 방향 / 시야
- 현재 무기 상태
- 현재 HP / DeadState
- 현재 Reaction 여부
- 플레이어 입력 값
- AI Blackboard / Context 데이터

즉, 인지는 "무엇을 할 수 있는가"를 판단하기 위한 원재료를 제공함.


---

### 2.2 입력(Intent)

입력은 단순한 물리 입력만을 의미하지 않음.  
여기서 입력은 "행동 의도(Intent)를 발생시키는 계층"을 뜻함.

#### Player의 입력

Player는 사용자 입력 장치로부터 intent를 생성함.

예시:

- 이동 키 입력
- 점프 입력
- 공격 입력
- 무기 전환 입력

#### Enemy의 입력

Enemy는 사용자 입력 대신 AI 의사결정 계층이 intent를 생성함.

예시:

- Perception 결과 기반 타겟 추적 결정
- Blackboard 기반 AttackIndex 선택
- BT를 통한 Chase / Engage / Reaction / Dead 진입 결정

즉, Enemy에게서 BT는 사실상 Player의 입력 계층을 대체함.


---

### 2.3 상태변환(State Transition)

입력 또는 의사결정 결과는 곧바로 액션 실행으로 이어지지 않고,  
반드시 상태 전이를 통해 현재 캐릭터의 실행 가능 조건을 정리해야 함.

예시:

- `Idle -> Action`
- `Idle -> Equip`
- `Alive -> Reaction`
- `Alive -> Dying`
- `Dying -> Dead`

상태변환의 목적은 다음과 같음.

- 현재 캐릭터가 무엇을 할 수 있는지 정의함
- 중복 실행을 방지함
- 상호 배타적 동작을 정리함
- 애니메이션 및 이동 정책과 동기화함


---

### 2.4 액션 실행(Action Execution)

상태가 확정되면, 그 상태에 맞는 실제 액션을 실행하게 됨.

예시:

- 공격 몽타주 재생
- 장비 장착 / 해제
- HitReact 재생
- Dead montage 재생
- 무기 collision 활성화
- 데미지 컨텍스트 주입
- 이동 제한 / 해제

즉, 액션 실행은 "결정된 상태를 실제 동작으로 변환하는 단계"에 해당함.


---

## 3. Player 실행 구조

Player는 다음 순서로 동작함.

`인지 -> 입력 -> 상태변환 -> 액션 실행`

### 3.1 인지

Player는 다음 정보를 기반으로 현재 실행 가능 여부를 판단함.

- 현재 무기 상태
- 현재 State
- 현재 HP / DeadState
- 이동 가능 여부
- 입력 가능 여부

### 3.2 입력

사용자 입력이 intent를 만듦.

예시:

- 공격 버튼 입력
- 이동 입력
- 무기 전환 입력

### 3.3 상태변환

입력은 현재 조건 검사를 거쳐 상태를 바꾸게 됨.

예시:

- 공격 입력 시 `Idle` 상태일 때만 공격 가능함
- 피격 시 `Reaction` 상태에 진입함
- HP가 0이 되면 `Dying`에 진입함

### 3.4 액션 실행

상태 전이 후 실제 액션이 실행됨.

예시:

- `ActionComp`가 montage를 실행함
- `WeaponComp`에 ActionContext를 주입함
- `ReactionComp`가 HitReact를 실행함
- `HealthComp`가 DeadState를 갱신함


---

## 4. Enemy 실행 구조

Enemy도 본질적으로는 동일한 흐름을 따름.

`인지 -> 입력(Intent 대체) -> 상태변환 -> 액션 실행`

단, 여기서 입력 계층은 사용자 입력이 아니라 BT/Blackboard가 대체함.

### 4.1 인지

Enemy는 다음 데이터를 인지 계층에서 수집함.

- AI Perception
- 타겟 위치 / 거리
- Blackboard Context
- 공격 가능 여부
- DeadState / Reaction 상태

### 4.2 입력(Intent 대체)

Enemy에게는 플레이어 입력이 없으므로,  
BT와 Blackboard가 행동 의도를 생성함.

예시:

- Chase 진입 결정
- Engage 진입 결정
- AttackIndex 선택
- HitReact / Dead 전환 판단

즉, Enemy에서 BT는 Player의 입력 계층을 대체함.

### 4.3 상태변환

BT는 현재 맥락에 따라 적절한 상태를 선택함.

예시:

- `Patrol -> Chase`
- `Chase -> Engage`
- `Engage -> HitReact`
- `Alive -> Dead`

### 4.4 액션 실행

선택된 상태를 실제 실행으로 바꾸는 책임은 BT Task가 가짐.

예시:

- 공격 몽타주 재생
- 이동 정지
- AttackContext 주입
- 리액션 실행
- Dead 처리

즉, Enemy에서는 `BT + Blackboard + Task`가 Player의 `StateComp + ActionComp` 역할을 사실상 대체함.


---

## 5. Player와 Enemy의 구조적 대응

### Player

- 인지: 로컬 캐릭터 상태 / 입력 가능 조건 확인
- 입력: 사용자 입력
- 상태변환: StateComp 중심
- 액션 실행: ActionComp 중심

### Enemy

- 인지: Perception / Blackboard Context
- 입력 대체: BT Decision
- 상태변환: BT / Blackboard 중심
- 액션 실행: BT Task 중심

정리하면 다음과 같음.

- Player는 `Input-driven execution` 구조를 가짐
- Enemy는 `Decision-driven execution` 구조를 가짐

그러나 두 경우 모두 최종 파이프라인은 동일함.

`Intent 생성 -> 상태 전이 -> 액션 실행`


---

## 6. 현재 구조의 문제점

현재 공용 컴포넌트 일부는 Player 중심 구조를 전제로 작성되어 있음.

예시:

- `ReactionComponent`가 `StateComp`를 직접 기대함
- `WeaponComponent`의 Context 주입 흐름이 `CAction` 중심으로 설계되어 있음

그 결과 다음 문제가 발생함.

- Enemy는 실제로 `StateComp`를 사용하지 않더라도 보유해야 함
- Enemy 공격도 Player식 실행 경로를 부분적으로 맞춰야 함
- 공용 컴포넌트와 Player 구현 사이에 불필요한 결합이 발생함

즉, 현재 Enemy의 일부 구성은 설계상 본질적 요구사항이 아니라  
Player 중심 공용 컴포넌트와의 호환을 위한 임시 결합층에 해당함.


---

## 7. 설계 원칙

향후 리팩터링 시 다음 원칙을 지향해야 함.

### 7.1 공용 컴포넌트는 Player 전용 상태 모델을 강제하지 않아야 함

공용 전투 컴포넌트는 다음 정도만 알아야 함.

- 현재 죽었는가
- 현재 리액션 중인가
- 이동 가능한가
- 현재 공격 컨텍스트는 무엇인가

`Equip`, `Action`, `Idle` 같은 풍부한 Player 상태는 공용 규칙이 아니라 Player 전용 orchestration 계층에 가까움.

### 7.2 Intent 공급원은 엔티티마다 달라도 됨

- Player: 입력 장치
- Enemy: BT / Blackboard

그러나 intent 이후의 공통 전투 처리 흐름은 최대한 통일할 수 있음.

### 7.3 액션 실행 책임과 컨텍스트 전달 책임은 분리해야 함

- 실행 주체
  - Player: `CAction`
  - Enemy: `BT Task`
- 컨텍스트 반영 주체
  - 공통: `WeaponComponent`

즉, 누가 실행하든 최종 컨텍스트 전달은 공용 계층에서 처리되어야 함.


---

## 8. 결론

Player와 Enemy는 제어 방식은 다르지만, 실행 구조는 동일한 추상 흐름으로 설명할 수 있음.

`인지 -> 입력(Intent) -> 상태변환 -> 액션 실행`

차이는 입력 계층에 있음.

- Player는 실제 사용자 입력이 intent를 만듦
- Enemy는 BT와 Blackboard가 intent를 만듦

따라서 Enemy에서 BT는 구조적으로 Player의 `StateComp + ActionComp` 역할을 대체하는 AI 전용 orchestration 계층으로 볼 수 있음.

다만 현재 구현에서는 공용 컴포넌트가 Player 기준으로 작성된 부분이 존재하며,  
이로 인해 Enemy가 본래 필요하지 않은 컴포넌트를 임시적으로 보유하는 결합이 발생하고 있음.

향후 리팩터링 목표는 다음과 같음.

- 공용 전투 컴포넌트의 Player 편향 제거
- Player / Enemy의 orchestration 계층 분리
- 공용 파이프라인은 전투 실행 규칙만 담당하도록 정리


---