# UE5 Portfolio Issue Analysis Report

## 제목

**I02: MoveTo 도착 판정 반경과 서비스 거리 계산 기준 불일치 분석**

## 날짜

**2026.03.06**

## 상태

- [x] **완료**

---

## 브랜치

- `feature/ai-behaviortree-core`

---

## 요약

- `MoveTo`는 성공했지만 Patrol 서비스의 도착 판정이 실패하여 `PatrolIndex` 갱신이 멈추는 문제가 발생했다.
  
- 갱신이 멈추는 것 자체는 `CBTService_UpdatePatrolContext`에서 `FVector::Dist`로 측정한 거리 값이 임계값인 `ReachThreshold`를 넘지 못하여 `earlyreturn` 되어 발생하는 현상임을 인지하고 있었다.
  
- 그러나 `MoveTo`로 도착이 완료된 캐릭터의 위치에서 도착 지점까지 거리가 `100` 이상 나온다는 것이 이해되지 않아 그 원인을 밝히고자 했다.
  
- 결과적으로 원인은 다음과 같았다.
	1. `MoveTo` 내부의 도착 완료 판정에 캐릭터의 충돌체 캡슐의 반경이 고려된다는 것
	   
	2. `MoveTo` 내부적으로 0 ~ 10의 오차보정을 가지고 있다는 것
	   
	3. `CBTService_UpdatePatrolContext`에서 `FVector::Dist2D` 가 아닌 `FVector::Dist`를 사용했다는 것
 

---

## 재현 절차

1. Patrol BT에서 `MoveTo(Blackboard: PatrolLocation)`를 구성한다.
   
2. `UCBTService_UpdatePatrolContext`를 구성하고 `FVector::Dist(ownerLocation, patrolLocation)`으로 `Enemy`와 `PatrolPoint` 사이의 거리를 구한다.
   
3. `UCBTService_UpdatePatrolContext`에서 `Dist`와 `ReachThreshold`의 값 비교로 판정한다.
   
4. 로그에서 `MoveTo Success`인데 서비스는 `bReached = false`로 유지되는 것을 확인한다.
   
5. 첫 포인트 이후 다음 인덱스 갱신이 지연/중단되는 현상을 재현한다.


---

## 기대 동작 vs 실제 동작

**기대 동작**
- `MoveTo` 성공 시 서비스도 동일하게 도착 처리하고 다음 포인트로 전환되어야 한다.
  
**실제 동작**
- `MoveTo` 성공 이후에도 서비스가 도착 실패로 남아 포인트 갱신이 멈췄다.


---

## 이슈 코드

```cpp
const float dist = FVector::Dist(currentOwnerLocation, currentPatrolLocation);
bool bReached = dist <= ReachThreshold; // ReachThreshold == 10

if (!bReached) return; // Error Point
```

```txt
MoveTo 설정:
- Acceptable Radius: 0
- Blackboard Key: PatrolLocation
```


---

## 실행 결과

```cpp
Custom_FLog: Display: [PatrolDist] dist: 109.38 (<= 10.00: false) | Reached: false
```


---

## 원인 분석

1. `MoveTo`는 캐릭터의 충돌체 중심에서 목표 지점까지의 거리를 기반으로 판단하는 것이 아닌,
   캐릭터의 충돌체와 목표 지점의 충돌체가 `overlap`되는지 여부를 가지고 판단한다.
   
2. `MoveTo`의 성공판정에는 캐릭터 충돌체의 높이, 반경, `MoveTo` Task의 허용 범위, `MoveTo` 내부의 오차 범위 등이 고려된다.
   
3. 따라서 서비스에서 `FVector::Dist`를 가지고 충돌 여부를 체크하게 될 경우, `MoveTo`와 임계값을 동일하게 설정하더라도 결과와 충돌이 발생할 여지가 높다.

4. 또한 캐릭터의 충돌체 중심의 Z값과 목표 지점의 Z값에서 차이가 있을 경우 `FVector::Dist` 방식은 의도치 않은 에러를 유발할 수 있다.


---

## 해결 방법

```cpp
const float dist2D = FVector::Dist2D(ownerLocation, patrolLocation);
const float diff_Z = FMath::Abs(ownerLocation.Z - patrolLocation.Z);

bool bReached_XY = dist2D <= ReachThreshold_XY;
bool bReached_Z = diff_Z <= Tolerance_Z;
bool bReached = bReached_XY && bReached_Z;

if (!bReached) return;
```

1. 도착 판정을 `Dist2D` 중심으로 변경했다.
   
2. 높이 차는 `ZTolerance`로 분리했다(`Dist2D + ZTolerance` 정책).
   
3. `ReachThreshold`를 `bReached_XY`와 `bReached_Z`로 나누어 판단했다.

4. `MoveTo` Task의 허용 범위는 기본값인 `0.f`를 사용했다.


---

## 해결 결과

1. 포인트별 반복 순찰 테스트(Loop/Reverse/Random)를 수행했다.
   
2. `MoveTo Success` 직후 서비스에서 `bReached=true` 전환되는 것을 확인했다.
   
3. `PatrolIndex`, `PatrolLocation`이 정상 갱신되는 것을 확인했다.
   
4. 캡슐 반경/Acceptable Radius 변경 시 로그값과 전환 시점이 의도대로 변하는지 검증했다.


---

## 결론

- `MoveTo` 도착 판정은 `collision overlap` 기반의 방식이다.

- `MoveTo`의 성공판정에는 캐릭터 충돌체의 높이, 반경, `MoveTo` Task의 허용 범위, `MoveTo` 내부의 오차 범위 등이 고려된다.

- 지상 이동 AI에서는 `Dist2D + ZTolerance` 방식이 보편적이며 안정적이다.
  
- `Service` 성공 기준과 `Task`의 성공 기준이 다를 수 있음을 인지해야 한다.


---
