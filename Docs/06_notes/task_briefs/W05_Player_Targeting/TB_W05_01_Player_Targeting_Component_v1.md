# TB W05-01 Player Targeting Component v1

## 작업명

Player Targeting Component v1

## 브랜치

```text
feat/player-targeting-component
```

## 상태

```text
진행
```

## 목적

플레이어가 카메라 전방의 유효한 적 하나를 락온 대상으로 선택, 유지, 해제할 수 있는 최소 타게팅 기반을 만든다.

이 결과는 후속 적 상태 HUD(적 이름 / HP / 밸런스), 카메라 락온, 타겟 전환이 공유할 단일 타겟 소스가 된다.

## 결정 사항

- `UCTargetingComponent`는 `ACPlayerController`가 소유한다.
- v1 후보는 `ACEnemy`로 한정한다.
- 기존 `ITargetContextProvider`는 AI의 적대 대상 인식 전용이므로 재사용하지 않는다.
- 현재 타겟은 `TWeakObjectPtr<ACEnemy>`로 보관한다.
- v1은 타겟 선택 / 유효성 검사 / 해제 / 개발용 디버그 표시까지만 구현한다.
- v1은 카메라 강제 회전, 캐릭터 회전 보정, 타겟 전환, HUD 생성 및 UI를 포함하지 않는다.
- 타겟 선택은 플레이어 위치가 아니라 `GetPlayerViewPoint()`의 카메라 위치와 방향을 기준으로 한다.

## 작업 범위

### 1. 타게팅 타입

`FTargetingTuning`을 추가해 다음 값을 데이터로 조절할 수 있게 한다.

```text
MaxTargetDistance
MaxTargetAngleDegrees
DistanceScoreWeight
AngleScoreWeight
ValidationInterval
bEnableDebugDraw
```

### 2. 타게팅 컴포넌트

다음 공개 API를 제공한다.

```cpp
void ToggleTargetLock();
bool AcquireBestTarget();
void ClearTarget();
bool HasTarget() const;
ACEnemy* GetCurrentTarget() const;
```

타겟 변경은 `OnTargetChanged(PreviousTarget, NewTarget)` delegate로 발행한다.

### 3. 후보 선별과 점수

후보는 다음 조건을 모두 만족해야 한다.

- 유효한 `ACEnemy`
- `UCHealthComponent::IsAlive()`가 true
- 카메라 기준 최대 거리 이내
- 카메라 전방 최대 각도 이내

후보 점수는 전방 중앙에 가까운 정도와 거리가 가까운 정도를 조합한다. 가장 높은 점수의 후보를 선택한다.

### 4. 유지와 해제

컴포넌트는 일정 주기로 현재 타겟만 검증한다. 다음 조건이면 자동 해제한다.

- 타겟 액터 또는 HealthComponent가 유효하지 않음
- 타겟이 사망함
- 최대 거리 초과

화면 뒤 이동이나 일시적 Line Of Sight 상실은 v1 자동 해제 조건에 넣지 않는다.

### 5. 입력과 검증

`TargetLock` Action Mapping을 `Tab`에 연결한다.

```text
타겟 없음 + 입력 -> 최적 후보 획득
타겟 있음 + 입력 -> 수동 해제
```

Development 빌드에서는 현재 타겟, 탐색 반경, 선택 점수를 디버그 드로우로 확인할 수 있게 한다.

## 변경 예정 파일

```text
Config/DefaultInput.ini
Source/Portfolio/Type/CTargetingTypes.h
Source/Portfolio/Type/CTargetingTypes.cpp
Source/Portfolio/Component/CTargetingComponent.h
Source/Portfolio/Component/CTargetingComponent.cpp
Source/Portfolio/Controller/CPlayerController.h
Source/Portfolio/Controller/CPlayerController.cpp
```

## 완료 조건

- `Tab`으로 카메라 전방의 살아 있는 적을 획득하거나 해제할 수 있다.
- 정면에 여러 적이 있을 때 중앙에 더 가까운 적이 우선 선택된다.
- 적 사망, 파괴, 거리 초과 시 현재 타겟이 안전하게 해제된다.
- 타겟 변경 때마다 delegate가 정확히 한 번 발행된다.
- Development 빌드에서 선택 대상과 해제 상태를 확인할 수 있다.
- Editor Win64 Development 빌드가 성공한다.

## 후속 작업

```text
W05-02: 좌우 타겟 전환
W05-03: 카메라 / 이동 락온 보정
W05-04: 타겟 마커와 Enemy Status HUD 연결
```
