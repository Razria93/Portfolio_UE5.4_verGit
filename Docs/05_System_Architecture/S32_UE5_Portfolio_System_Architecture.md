# S32 공통 Combat Target 상태 및 의사결정 경계 설계

## 1. 문서 상태

```text
Status: Historical Design Baseline / 구현 완료 후 참조 기록
Original Scope: Goal 1 — 현행 조사와 설계 확정
Implemented Branch: feature/combat-target-participation
```

이 문서는 Player와 Enemy가 공통으로 사용하는 전투 대상 상태의 Source of Truth와, 그 상태를 결정·투영·소비하는 계층의 경계를 고정한다.

> **현재 계약 안내**: 이 문서는 구현 전 단계에서 작성한 경계 설계와 마이그레이션 계획을 보존한다. 본문에 남아 있는 `UCBTTask_SetFocus`, `ICombatTargetProvider`, Goal 1 이후 계획은 당시의 조사·전환 기준이며 현재 runtime API가 아니다. 현재 공통 Combat Target 구현은 [S33](S33_UE5_Portfolio_System_Architecture.md), Enemy Evidence·Assignment·Investigate 정책은 [S34](S34_UE5_Portfolio_Combat_Participation_Policy.md)를 정규 기준으로 사용한다.

기존 S32의 다음 방안은 폐기한다.

```text
Player Target SoT = UCTargetingComponent
Enemy Target SoT  = Blackboard TargetActor
Controller Provider만 공통화
```

최종 결정은 다음과 같다.

```text
Player / Enemy 공통 Combat Target SoT
= 각 Character가 소유한 UCCombatTargetComponent

Player 입력과 AI/BT
= 서로 다른 타겟 결정 정책

Blackboard TargetActor
= CombatTargetComponent 상태의 단방향 투영값
```

---

## 2. 설계 배경과 해결할 문제

현재 프로젝트에는 다음 문제가 함께 존재한다.

- Player는 `UCTargetingComponent::CurrentTarget`을 전투 타겟으로 사용한다.
- Enemy는 Blackboard `TargetActor`를 인지 후보이자 전투 타겟처럼 사용한다.
- 공통 Runtime 소비자가 Player Targeting, AIController, Blackboard 저장 방식을 직접 알아야 한다.
- Enemy가 피격되었을 때 공격자를 전투 타겟으로 채택하는 명시적 Runtime 계약이 없다.
- Action의 타겟 방향 보정, AI Focus, 카메라, HUD가 앞으로 서로 다른 저장소를 참조할 위험이 있다.
- 타겟 파괴·스트리밍 제거·약한 참조 만료 시 Player와 Enemy의 수명 규칙이 달라질 수 있다.

Provider만 공통화하면 조회 문법은 같아지지만 두 Source of Truth와 서로 다른 수명 규칙은 그대로 남는다. 이는 후속 Hit Engage와 Action Facing을 구현할수록 동기화 비용과 의미 충돌을 키운다.

따라서 이번 설계는 조회 인터페이스만 통합하는 것이 아니라 **전투 타겟 상태와 수명 계약 자체를 공통화**한다.

---

## 3. 현행 코드 전수조사 결과

### 3.1 Player 경로

```text
ACPlayerController
→ UCTargetingComponent
→ TWeakObjectPtr<ACEnemy> CurrentTarget
```

현재 `UCTargetingComponent`는 다음 책임을 함께 가진다.

- 화면 중앙 우선 후보 수집과 점수 계산
- 최초 선택과 좌우 전환
- 거리·View Cone·생존 여부 기반 정책 검증
- `CurrentTarget` 저장
- `OnTargetChanged` 발행
- 선택 대상의 `OnEndPlay` 구독과 약한 참조 만료 정리

이 중 후보 탐색·평가·선택은 Player 전용 정책이지만, 상태 저장·변경 이벤트·대상 수명 관리는 Player와 Enemy가 공유해야 할 Runtime 계약이다.

### 3.2 Enemy 경로

```text
ACAIController Perception
→ UCBTService_UpdateAIContext
→ Blackboard TargetActor
→ Intent / Focus / Action / Combat Signal / Debug 소비
```

현재 `TargetActor`는 다음 의미를 동시에 맡는다.

- Perception이 선택한 최상위 인지 대상
- BT가 판단에 사용하는 대상
- Focus Task가 바라보는 대상
- Combat Action의 실행 대상
- Combat Signal의 전달 대상
- Debug Overlay에 표시하는 대상

즉 **인지된 후보**와 **현재 싸우기로 확정한 전투 대상**이 분리되지 않았다.

확인된 직접 접근 지점은 다음과 같다.

| 영역 | 현재 사용 |
| --- | --- |
| `UCBTService_UpdateAIContext` | Perception 결과를 `TargetActor`에 직접 Set/Clear |
| `UCBTDecorator_HasValidTarget` | Blackboard 타겟 유효성 판단 |
| `UCBTService_UpdateAIIntentState` | Intent 산정 |
| `UCBTService_UpdateEngageContext` | Engage 문맥 구성 |
| `UCBTTask_SetFocus` | AI Focus 설정 |
| `UCBTTask_StartCombatAction` | Action 대상 전달 |
| `UCBTTask_SelectAlertPoint` | Alert 지점 선택에 사용 |
| `CAIRuntimeLODTierResolver` | Runtime LOD 판단에 사용 |
| `UCCombatSignalSourceComponent` | AIController와 Blackboard를 직접 조회 |
| Debug Overlay | 현재 AI Target 표시 |

이 목록은 모두 같은 방식으로 기계적으로 교체하지 않는다. 각 사용처가 **전투 대상**, **인지 후보**, **마지막 인지 위치** 중 무엇을 요구하는지 분류한 뒤 이동한다.

### 3.3 Character 중심 Runtime 구조

Player와 Enemy Character는 이미 Health, Movement, Action/Reaction, Combat Signal 등 전투 Runtime 컴포넌트를 소유한다. Controller는 입력 또는 AI 의사결정을 내리고 Character의 공개 요청 API를 호출한다.

따라서 전투 타겟 상태도 Controller나 Blackboard가 아니라 Character Runtime에 두는 것이 현재 프로젝트의 책임 구조와 가장 잘 맞는다.

### 3.4 기존 인터페이스와의 구분

`ITargetContextProvider`는 Perception 대상의 우선순위 `GetTargetPriority()`를 제공한다. 이는 “이 Actor를 얼마나 중요하게 인지할 것인가”에 대한 문맥이며, 현재 전투 상대를 제공하는 계약이 아니다.

`ICombatTargetProvider`와 이름·책임을 합치지 않는다.

---

## 4. 핵심 용어

### 4.1 Perceived Actor / Perception Candidate

시청각 또는 월드 정보로 존재를 인지한 Actor다. 아직 전투 상대로 확정되지 않았으며 여러 개일 수 있다.

```text
Perception
= 누구를 관찰하거나 기억하고 있는가
```

### 4.2 Combat Target

현재 Character가 전투 상대로 명시적으로 선택한 단일 Actor다.

```text
Combat Target
= 지금 누구와 싸우는가
```

### 4.3 Target Decision Policy

인지 후보 또는 입력을 근거로 어떤 Actor를 Combat Target으로 채택·교체·해제할지 결정하는 계층이다.

- Player: 입력, 화면 점수, 선택·전환 정책
- Enemy: Perception/CombatResult를 투영받은 BT 의사결정

### 4.4 Target Consumer

확정된 Combat Target을 읽고 자기 책임을 수행하는 계층이다.

- Focus
- Facing / Action Tracking
- Player Camera Lock Assist
- Target Marker / Enemy Status HUD
- Combat Signal
- Debug Overlay

### 4.5 Action Facing Target

Action 실행 중 방향 보정에 쓰는 타겟이다. 기본 원천은 Combat Target이지만 Accepted 시점에 실행 Context로 캡처할 수 있다. 실행 중 Combat Target이 바뀌더라도 이미 Commit된 공격 궤적을 무조건 흔들지 않기 위한 별도 실행 문맥이다.

### 4.6 Debug Focus

Debug Overlay가 관찰하는 Actor다. Combat Target과 동기화할 수 있지만 독립된 관찰 상태이며 수동 Focus를 Runtime Target이 무조건 덮어쓰지 않는 기존 계약을 유지한다.

---

## 5. 확정 아키텍처

### 5.1 전체 흐름

```text
Player Input / AI Perception / CombatResult
                │
                ▼
       Target Decision Policy
       - Player Targeting Policy
       - AI BT Decision Policy
                │ Request Set / Clear
                ▼
       UCCombatTargetComponent
       - 단일 Runtime Source of Truth
       - 상태 변경과 대상 수명 관리
                │
        ┌───────┴────────┐
        ▼                ▼
Runtime Consumers   Blackboard Projection
Focus/Facing/etc.   AI 의사결정용 읽기 모델
```

### 5.2 소유 위치

Player와 Enemy Character가 각각 `UCCombatTargetComponent` 인스턴스를 소유한다.

```text
ACPlayer
└─ UCCombatTargetComponent

ACEnemy
└─ UCCombatTargetComponent
```

Controller는 상태 소유자가 아니다.

- `ACPlayerController`의 `UCTargetingComponent`는 Player 선택 정책을 수행하고 Player Character의 CombatTargetComponent에 변경을 요청한다.
- `ACAIController`와 BT는 Enemy의 의사결정을 수행하고 Enemy Character의 CombatTargetComponent에 변경을 요청한다.

### 5.3 Source of Truth

```text
UCCombatTargetComponent::CurrentTarget
= 유일한 Runtime Combat Target 상태
```

다음은 더 이상 Source of Truth가 아니다.

- `UCTargetingComponent::CurrentTarget`
- Blackboard `TargetActor`
- Controller의 별도 Target 필드
- Focus Actor
- Action Context의 Target snapshot

Action Context snapshot은 특정 실행의 안정성을 위한 파생 데이터일 뿐 현재 Combat Target 상태가 아니다.

---

## 6. UCCombatTargetComponent 책임

### 6.1 소유하는 책임

공통 컴포넌트는 다음 최소 상태·수명 계약을 소유한다.

```text
- TWeakObjectPtr<AActor> CurrentTarget
- Set / Clear 요청 처리
- OnCombatTargetChanged(Previous, Current)
- 현재 대상 OnEndPlay 구독·해제
- 대상 파괴·레벨 제거 시 즉시 Clear
- stale weak pointer 정리
- 같은 대상 재설정 중복 억제
- Target Revision / Serial 증가
```

Revision은 비동기·지연 소비자가 오래된 타겟 변경 결과를 구분할 수 있게 하는 단조 증가값이다. Actor 주소 비교를 대신하는 고유 Actor ID가 아니라 **Combat Target 상태 변경 버전**이다.

### 6.2 소유하지 않는 책임

공통 컴포넌트는 다음 정책을 알지 않는다.

- 화면 중앙 점수와 좌우 전환 후보 계산
- 거리, View Cone, LOS, 팀, 생존 여부를 이용한 선택 적합성 정책
- AI Perception과 위협도 계산
- 어떤 후보를 선택할지에 대한 BT 의사결정
- AI Focus 설정
- Character 회전과 Action Tracking
- Player 카메라 보정
- HUD/Marker 렌더링
- Engage 슬롯 중재

컴포넌트는 요청받은 상태 변경을 일관되게 Commit하고 구조적 수명을 보장한다. “이 대상을 선택해도 되는가”는 요청 전에 결정 정책이 판단한다.

### 6.3 개념 API

실제 구현 시 프로젝트 네이밍과 타입 규칙에 맞춰 확정하되 계약은 다음과 같다.

```cpp
bool HasCombatTarget() const;
AActor* GetCombatTargetActor() const;
int32 GetCombatTargetRevision() const;

bool RequestSetCombatTarget(AActor* InTarget, ECombatTargetChangeReason InReason);
bool RequestClearCombatTarget(ECombatTargetChangeReason InReason);

FOnCombatTargetChanged OnCombatTargetChanged;
```

변경 사유의 초기 범주는 다음을 권장한다.

```text
PlayerSelection
AIDecision
HitReceived
PolicyInvalidated
TargetEndPlay
OwnerLifecycle
ManualClear
```

### 6.4 불변 조건

```text
1. 현재 타겟 변경은 컴포넌트 API를 통해서만 Commit한다.
2. 같은 Actor 재설정은 변경 이벤트를 중복 발행하지 않는다.
3. Target EndPlay는 정확히 한 번 Clear 전이를 만든다.
4. 교체된 이전 Target의 늦은 EndPlay callback은 새 Target을 해제하지 않는다.
5. stale weak pointer도 명시적인 Clear 상태와 이벤트로 정리한다.
6. 변경 이벤트의 Previous/Current는 실제 Commit 결과와 일치한다.
7. Target Revision은 실제 상태 전이 때만 증가한다.
```

### 6.5 변경 권한

초기 구현에서는 범용 Arbitration/Priority 시스템을 만들지 않는다.

```text
Player Character의 정책 Writer
= UCTargetingComponent

Enemy Character의 정책 Writer
= AI/BT Target Decision 경로

공통 Lifecycle Clear
= UCCombatTargetComponent 자체
```

동일 Character에 여러 정책 Writer가 필요해지는 실제 사례가 생기면 요청 우선순위와 소유권 토큰을 후속 설계한다. 현재부터 추상적인 중재 시스템을 넣는 것은 과설계다.

---

## 7. Player 정책 경계

`UCTargetingComponent`는 제거하지 않는다. 다음과 같이 **선택 정책 컴포넌트**로 책임을 좁힌다.

### 유지할 책임

- 후보 수집
- 화면 중심·거리 점수
- 최초 타겟 선택
- 좌우 전환
- Player 락온 적합성 평가
- 주기적 정책 유효성 검사

### 이동할 책임

| 현재 `UCTargetingComponent` 책임 | 이동 위치 |
| --- | --- |
| `CurrentTarget` 저장 | `UCCombatTargetComponent` |
| `OnTargetChanged` | `UCCombatTargetComponent::OnCombatTargetChanged` |
| Target `OnEndPlay` 구독 | `UCCombatTargetComponent` |
| stale weak 정리 | `UCCombatTargetComponent` |
| 중복 Set/Clear 억제 | `UCCombatTargetComponent` |

정책 검증에서 타겟이 거리·View Cone·Health 조건을 벗어나면 TargetingComponent가 공통 컴포넌트에 Clear를 요청한다. 공통 컴포넌트가 Player 전용 적합성 규칙을 직접 실행하지 않는다.

---

## 8. AI와 Behavior Tree 경계

### 8.1 기본 원칙

```text
Character + Runtime Components
= 기능적으로 완결된 Domain Object

AIController + Behavior Tree
= 얇은 의사결정 계층
```

BT는 Runtime 상태 저장소가 아니다.

### 8.2 Service 책임

Service는 다음 정보를 Blackboard에 투영한다.

- Character와 Runtime Component의 상태
- Perception의 인지 후보·마지막 인지 정보
- 월드와 전투 문맥
- 현재 Combat Target의 읽기 전용 Projection

Service가 Component의 내부 상태를 직접 임의 변경하거나, Perception 최상위 후보를 곧바로 Runtime Combat Target으로 덮어쓰지 않는다.

### 8.3 BT 책임

BT는 투영된 정보로 다음 의사결정을 수행한다.

```text
인지 후보 중 누구를 Combat Target으로 채택할 것인가
현재 Target을 유지·교체·해제할 것인가
어떤 Intent와 Action을 요청할 것인가
```

### 8.4 Task 책임

Task는 결정을 실행하는 얇은 Adapter다.

```text
BT Decision
→ Task
→ ACEnemy 또는 Component 공개 Request API
→ Runtime 상태 Commit
```

Task가 Blackboard 값만 직접 바꾸고 그것을 Runtime 상태 변경으로 간주하면 안 된다.

### 8.5 Blackboard TargetActor 정책

기존 UAsset 호환을 위해 `TargetActor` 키는 초기 마이그레이션에서 유지할 수 있다. 단 의미는 다음으로 바뀐다.

```text
Before
Perception이 선택하고 BT/Runtime이 함께 사용하는 Source of Truth

After
UCCombatTargetComponent::CurrentTarget의 단방향 Projection
```

흐름은 반드시 한 방향이다.

```text
CombatTargetComponent 변경
→ Service 또는 전용 Projection Adapter
→ Blackboard TargetActor 갱신
```

Blackboard `TargetActor`를 직접 Set한 뒤 Component로 역동기화하지 않는다.

### 8.6 Blackboard에 별도로 남길 Working Memory

다음 정보는 Combat Target과 별개로 Blackboard에 유지한다.

- Perceived Candidate
- Last Known Hostile Actor/Location
- Threat Score
- Target Priority
- LOS와 LastSeenTime
- Alert/Investigate 지점
- Engage Role과 전술 문맥

기존 `TargetActor`를 인지 후보 용도로 사용하던 소비자는 의미에 따라 별도 키로 이동해야 한다.

---

## 9. Provider와 소비 계층

### 9.1 ICombatTargetProvider

공통 Provider는 유지할 가치가 있다. 다만 분리된 저장소를 감추는 임시 Adapter가 아니라 공통 Component의 읽기 계약을 노출한다.

권장 구현 주체는 Character다.

```text
ACPlayer / ACEnemy
→ ICombatTargetProvider
→ 자신의 UCCombatTargetComponent에 위임
```

Controller가 Provider를 구현하면 소비자가 다시 Controller 소유 구조에 묶인다. Character가 완결된 Runtime Domain Object라는 원칙에도 맞지 않으므로 기본안으로 채택하지 않는다.

### 9.2 소비자별 규칙

| 소비 영역 | 참조할 정보 | 책임 |
| --- | --- | --- |
| Player Target Lock Assist | Combat Target | 카메라·캐릭터 락온 보정 |
| Target HUD Presenter | Combat Target 변경 이벤트 | Marker 표시와 위치 투영 |
| AI Focus Task/Adapter | Combat Target | Controller Focus 설정 |
| Combat Action Task | Combat Target | Action 요청 Target 구성 |
| Action Facing | 실행 Context Target snapshot | Tracking/CommitLock |
| Combat Signal Source | Combat Target Provider | 신호 대상 해석 |
| Debug Overlay | Combat Target + Debug Focus | Runtime 상태 관찰 |
| Perception/Alert/LOD | Perception Working Memory | 인지·최적화 판단 |

핵심 구분은 다음과 같다.

```text
Perception = 누구를 발견했는가
Decision   = 누구를 선택할 것인가
Target     = 지금 누구와 싸우는가
Consumer   = 그 Target으로 무엇을 할 것인가
```

---

## 10. 현행 소비자 마이그레이션 지도

| 현행 위치 | 현행 의미 | 권장 이동 |
| --- | --- | --- |
| `UCTargetingComponent::CurrentTarget` | Player Combat Target + 선택 정책 | 상태는 공통 Component, 정책은 유지 |
| `UCTargetLockAssistComponent` | Player Target 소비 | 공통 Provider/Component 참조 |
| `UCTargetHUDPresenterComponent` | Player Target 변경 소비 | 공통 변경 이벤트 구독 |
| `UCBTService_UpdateAIContext` Target Set/Clear | Perception과 Combat Target 혼합 | Perception Working Memory 투영과 Combat Target Projection 분리 |
| `UCBTDecorator_HasValidTarget` | Combat Target 존재 판정 | Projection 또는 Character 조회 |
| `UCBTService_UpdateAIIntentState` | Intent 산정 | Combat Target Projection 읽기 |
| `UCBTService_UpdateEngageContext` | Engage 요청 문맥 | Combat Target Projection 읽기 |
| `UCBTTask_SetFocus` | 전투 방향 Focus | Character Combat Target 조회 |
| `UCBTTask_StartCombatAction` | Action 대상 | Character Combat Target 조회 후 요청 Context에 전달 |
| `UCCombatSignalSourceComponent` | 신호 대상 | `ICombatTargetProvider` 사용 |
| `UCBTTask_SelectAlertPoint` | 인지/Alert 문맥 가능성 | Combat Target과 Last Known 정보 중 의미 재분류 |
| `CAIRuntimeLODTierResolver` | 인지/거리 문맥 가능성 | Perception Working Memory 사용 여부 재분류 |
| Debug Overlay | Enemy BB Target 표시 | 공통 Target 상태를 직접 관찰 |

---

## 11. 후속 전투 정책과의 결합

### 11.1 피격 기반 Enemy Engage

```text
Enemy가 유효한 Hit CombatResult 수신
→ Target-side에서 공격자 후보와 교체 정책 평가
→ BT/AI 의사결정 또는 명시적 Hit Reactive Request
→ CombatTargetComponent에 공격자 Set 요청
→ Target Projection 갱신
→ 기존 Engage Intent/Subsystem 경로 진입
```

Hit가 Blackboard를 직접 쓰거나 Engage Role을 강제하지 않는다. Combat Target 채택과 Engage 슬롯 중재는 별도 계약이다.

### 11.2 Action Facing

```text
Action Accepted
→ 현재 Combat Target snapshot

Startup Tracking Window
→ 제한 속도로 Target Yaw 추적

Commit / Hit Window
→ Facing 고정

다음 Combo Startup
→ 최신 Combat Target 재획득
```

공통 Combat Target은 “누구를 향할지”를 제공한다. 실제 회전 속도, 최대 보정각, Tracking Window와 CommitLock은 Action 실행 계층 책임이다.

### 11.3 카메라·Locomotion·Focus

- Player Camera는 Combat Target을 읽되 상태를 변경하지 않는다.
- Enemy Focus는 Combat Target을 읽되 Source of Truth가 아니다.
- Locomotion Facing은 공통 Target 존재 여부를 정책 입력으로 쓸 수 있다.
- Action Facing이 활성일 때 Locomotion Facing과의 우선순위는 Action 실행 계약에서 해결한다.

---

## 12. 단계별 구현 계획

### Goal 1 — 조사와 설계 확정

현재 문서 작업 범위다.

- Player/Enemy 타겟 저장·변경·소비 경로 전수조사
- Perception Candidate와 Combat Target 의미 분리
- 공통 Component 책임과 비책임 확정
- BT/Service/Task/Blackboard 경계 확정
- 마이그레이션 지도와 검증 기준 기록
- Roadmap과 Architecture Index 동기화

### Goal 2 — 공통 Target Kernel

- `UCCombatTargetComponent` 추가
- 공통 상태, Set/Clear, 변경 이벤트 구현
- EndPlay/stale weak/중복 callback/Revision 계약 구현
- Character 양쪽에 컴포넌트 구성
- 단위·자동화 수명 테스트 추가

### Goal 3 — Player 마이그레이션

- `UCTargetingComponent`에서 상태 저장과 수명 책임 제거
- 후보 탐색·평가·선택 정책만 유지
- Lock Assist와 HUD를 공통 상태/이벤트로 전환
- 기존 선택·전환·파괴·화면 밖 회귀 검증

### Goal 4 — Enemy와 BT 마이그레이션

- Perception Working Memory와 Combat Target 분리
- AI Target 결정 Request 경로 추가
- Blackboard `TargetActor`를 단방향 Projection으로 변경
- Focus/Intent/Engage/Action Task 소비 경로 전환
- 기존 BT UAsset 호환과 점진적 Key 정리

### Goal 5 — Provider와 공통 소비자 정리

- Character 기반 `ICombatTargetProvider` 확정
- CombatSignalSource의 AIController/Blackboard 직접 의존 제거
- Debug Overlay를 공통 Target 상태로 전환
- Alert/LOD 등 각 소비자의 의미를 재분류

### Goal 6 — 최종 감사와 문서 확정

- 직접 `CurrentTarget`/Blackboard 쓰기 잔여 검색
- Player/Enemy Target 수명 회귀 검증
- BT가 Runtime 상태를 직접 소유·변경하지 않는지 감사
- API 배치, 섹션, 가시성, 책임 경계 검토
- S32와 Roadmap을 구현 결과에 맞춰 확정

---

## 13. Goal 1 이후 검증 기준

### 구조 검증

```text
- Combat Target의 Source of Truth가 Character의 공통 Component 하나인가
- Player Targeting은 정책만 소유하는가
- AI Perception과 Combat Target이 의미상 분리됐는가
- Blackboard가 Runtime 상태의 단방향 Projection인가
- BT Task가 Character/Component 공개 Request API만 호출하는가
- 소비자가 PlayerController나 AI Blackboard 구현을 직접 알지 않는가
```

### 수명 검증

```text
- Target 직접 Destroy / EndPlay → 정확히 한 번 Clear
- A → B 교체 후 A EndPlay → B 유지
- stale weak target → 명시적인 Clear 전이
- 같은 Target 재설정 → 중복 이벤트 없음
- Owner EndPlay → Delegate 안전 해제
```

### 회귀 검증

```text
- Player 선택·전환·해제 체감 유지
- Enemy Perception과 Intent 전이 유지
- Focus와 Action 요청 대상 일치
- Combat Signal 대상 일치
- Debug Focus의 Live/Frozen/Manual 계약 유지
```

---

## 14. 비목표

이번 구조가 다음 기능까지 직접 구현하는 것은 아니다.

- 범용 Threat/Target Arbitration 시스템
- Player와 Enemy가 동일한 후보 탐색 알고리즘 사용
- Perception Component 통합
- Engage 슬롯 정책 변경
- Action Tracking/Turn-in-place 구현
- Enemy Status HUD 구현
- Debug Focus와 Combat Target 상태 통합
- 기존 Blackboard Key의 즉시 삭제 또는 UAsset 일괄 변경

공통화하는 것은 **타겟 상태와 수명 계약**이다. 판단 입력과 선택 정책, 그리고 타겟을 활용하는 표현·실행은 각 책임 계층에 남긴다.

---

## 15. 최종 결정 기록

```text
1. Player와 Enemy의 Combat Target 상태 저장소를 공통화한다.
2. UCCombatTargetComponent는 Character가 소유한다.
3. UCTargetingComponent는 Player 후보 탐색·선택 정책으로 축소한다.
4. AI Perception은 후보 정보이며 Combat Target을 직접 의미하지 않는다.
5. Blackboard TargetActor는 공통 상태의 단방향 Projection으로 전환한다.
6. Service는 Runtime/World 정보를 투영하고 BT는 의사결정한다.
7. Task는 Character/Component의 공개 Request API를 호출한다.
8. ICombatTargetProvider는 Character가 공통 Component에 위임하는 읽기 계약이다.
9. Focus, Facing, Camera, HUD, Combat Signal은 Target 소비 계층이다.
10. 실제 중복이 확인되기 전 후보 평가 수학이나 범용 Arbitration은 추상화하지 않는다.
```

이 결정은 Character와 Component가 Runtime 기능의 완결성을 가지고 Controller와 BT가 얇은 의사결정 계층으로 남도록 하는 프로젝트의 장기 구조 기준이다.
