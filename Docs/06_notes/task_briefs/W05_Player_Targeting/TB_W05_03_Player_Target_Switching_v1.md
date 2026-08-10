# TB W05-03 Player Target Switching v1

## 작업명

Player Target Switching v1

## 브랜치

```text
feat/player-targeting-component
```

## 상태

```text
계획 확정
```

## 목적

현재 락온 타겟을 기준으로 화면상 왼쪽 또는 오른쪽에 인접한 유효 Enemy로 전환할 수 있게 한다.

W05-03은 타겟 선택 상태만 변경한다. 카메라 추적, 캐릭터 이동·회전 보정, 타겟 마커와 Enemy Status HUD는 각각 W05-04와 W05-05에서 처리한다.

## 결정 사항

- 좌우 판정은 월드 좌표나 플레이어의 로컬 좌우가 아니라 카메라의 화면 공간을 기준으로 한다.
- 현재 타겟과 후보 위치는 `ProjectWorldLocationToScreen()`으로 투영한다.
- 후보는 살아 있는 `ACEnemy`, 최대 거리 이내, View Cone 내부 조건을 모두 만족해야 한다.
- 후보 선택은 기존 `BuildTargetEvaluation()`과 `IsTargetEvaluationValid()`의 거리·각도·생존 판정을 재사용한다.
- 현재 타겟은 후보에서 제외한다.
- 후보는 화면 투영에 성공하고 Player Viewport 범위 안에 있어야 한다.
- Line Of Sight는 W05-03 후보 조건에 추가하지 않는다.
- 선택 방향에 후보가 없으면 현재 타겟을 유지한다.
- 반대편 끝으로 자동 순환하는 Wrap Around는 v1에서 사용하지 않는다.
- 현재 타겟이 없을 때 좌우 전환 입력이 들어오면 기존 `AcquireBestTarget()`을 실행한다.
- 실제 상태 변경은 기존 `SetCurrentTarget()`을 통해 처리해 `OnTargetChanged`와 Destroy Bind/Unbind 계약을 유지한다.

## 입력 정책

현재 타겟 선택·해제는 마우스 가운데 버튼을 유지한다.

```text
MiddleMouseButton -> TargetLock
MouseScrollUp     -> TargetSwitchLeft
MouseScrollDown   -> TargetSwitchRight
```

마우스 휠을 이후 무기 교체 등에 사용해야 한다면 입력 매핑만 별도 변경하며, `SwitchTarget(Direction)`의 런타임 계약은 유지한다.

## 공개 타입과 API

방향 타입은 타게팅 타입 헤더에 둔다.

```cpp
UENUM(BlueprintType)
enum class ETargetSwitchDirection : uint8
{
	Left,
	Right,

	Max,
};
```

타게팅 컴포넌트는 다음 명령 API를 제공한다.

```cpp
bool SwitchTarget(ETargetSwitchDirection InDirection);
```

반환 계약:

```text
true  -> 새로운 타겟으로 실제 변경됨
false -> 유효한 방향 후보가 없어 기존 상태 유지
```

현재 타겟이 없어서 `AcquireBestTarget()`이 성공한 경우도 `true`를 반환한다.

## 후보 수집

후보 탐색은 기존 최초 획득과 동일하게 입력 시점에만 `TActorIterator<ACEnemy>`를 사용한다. Tick마다 전체 Enemy를 수집하거나 별도 후보 배열을 상시 유지하지 않는다.

후보 조건:

```text
Candidate != CurrentTarget
Candidate is valid ACEnemy
HealthComponent::IsAlive() == true
Distance <= MaxTargetDistance
Dot >= MinDot
ProjectWorldLocationToScreen() == true
CandidateScreenPosition is inside Player Viewport
```

현재 타겟 역시 화면 공간 기준점이 필요하므로 화면 투영에 실패하면 전환하지 않고 현재 타겟을 유지한다.

## 좌우 판정

```text
CandidateScreenX < CurrentTargetScreenX -> Left 후보
CandidateScreenX > CurrentTargetScreenX -> Right 후보
```

화면 X가 동일하거나 부동소수점 허용 오차 안에 있으면 어느 방향 후보에도 포함하지 않는다.

```text
abs(CandidateScreenX - CurrentTargetScreenX) <= KINDA_SMALL_NUMBER
-> 방향 후보 제외
```

## 후보 우선순위

선택한 방향에서 현재 타겟과 화면상 가장 인접한 후보를 선택한다. 비교는 다음 순서의 사전식 우선순위를 사용한다.

```text
1. abs(CandidateScreenX - CurrentTargetScreenX)가 작은 후보
2. abs(CandidateScreenY - CurrentTargetScreenY)가 작은 후보
3. Target Evaluation FinalScore가 높은 후보
```

이 규칙은 단순히 화면에서 가장 왼쪽 또는 가장 오른쪽 Enemy로 건너뛰는 것을 방지한다. 별도 SwitchScore 가중치나 정규화 설정은 v1에 추가하지 않는다.

모든 비교값까지 동일하면 기존 월드 탐색 순서에서 먼저 확인된 후보를 유지한다.

## 처리 흐름

```text
TargetSwitch 입력
-> 현재 타겟 확인

현재 타겟 없음
-> AcquireBestTarget()

현재 타겟 있음
-> 현재 타겟 Screen Position 계산
-> World의 ACEnemy 후보 탐색
-> 기존 Target Evaluation 수행
-> 화면 투영 / Viewport / 방향 조건 검사
-> 방향 내 인접 후보 선택

후보 있음
-> SetCurrentTarget(BestSwitchTarget)
-> Previous OnDestroyed Unbind
-> New OnDestroyed Bind
-> OnTargetChanged(Previous, New) 1회

후보 없음
-> 현재 타겟 유지
-> OnTargetChanged 발행 없음
```

## Debug Observability

타게팅 컴포넌트는 World Draw를 직접 수행하지 않는다.

Non-Shipping Debug Snapshot 또는 `FTargetingDebug` 확장을 통해 필요 시 다음 값을 관찰할 수 있게 한다.

```text
Switch Direction
Current Target Screen Position
Candidate Screen Position
Horizontal Delta
Vertical Delta
Candidate FinalScore
Selected Switch Target
Switch Failure Reason
```

초기 구현에서는 런타임 선택 정확성 검증에 필요한 최소 정보만 추가하고, Editor 패널의 새 옵션은 실제 표시 항목이 추가될 때만 확장한다.

## 변경 예정 파일

```text
Config/DefaultInput.ini
Source/Portfolio/Type/CTargetingTypes.h
Source/Portfolio/Component/CTargetingComponent.h
Source/Portfolio/Component/CTargetingComponent.cpp
Source/Portfolio/Controller/CPlayerController.h
Source/Portfolio/Controller/CPlayerController.cpp
```

Debug 정보가 추가되는 경우:

```text
Source/Portfolio/Core/Debug/FTargetingDebug.h
Source/Portfolio/Core/Debug/FTargetingDebug.cpp
Source/Portfolio/Core/Debug/FDebugOverlayViewDataTypes.h
Source/Portfolio/Core/Debug/FDebugOverlayTextFormatter.cpp
```

## 제외 범위

- 카메라 자동 추적
- 플레이어 이동 방향 보정
- 플레이어 또는 카메라 회전 보정
- 공격 대상을 CurrentTarget으로 강제
- 타겟 마커와 Enemy Status HUD
- LOS 기반 후보 제외
- 화면 끝에서 반대편으로 자동 순환
- Tick 기반 상시 후보 캐시

## 완료 조건

- 현재 타겟이 없을 때 좌우 전환 입력으로 기존 최적 후보를 획득할 수 있다.
- 현재 타겟이 있을 때 마우스 휠로 화면상 왼쪽·오른쪽 인접 Enemy를 선택할 수 있다.
- 살아 있지 않거나 거리·View Cone·Viewport 조건을 벗어난 Enemy는 전환 후보에서 제외된다.
- 선택 방향에 후보가 없으면 현재 타겟과 구독 상태를 유지한다.
- 후보가 없을 때 `OnTargetChanged`가 발행되지 않는다.
- 전환 성공 시 `OnTargetChanged(PreviousTarget, NewTarget)`가 정확히 한 번 발행된다.
- 전환 후 이전 타겟 Destroy가 새 타겟을 해제하지 않는다.
- 전환 후 새 타겟의 사망·거리 초과·Destroy 자동 해제가 기존 계약대로 동작한다.
- Debug 표시를 추가한 경우 실제 선택 후보와 표시 결과가 일치한다.
- Editor Win64 Development 빌드가 성공한다.

## 후속 작업

```text
W05-04: 카메라 / 이동 락온 보정
W05-05: 타겟 마커와 Enemy Status HUD
```
