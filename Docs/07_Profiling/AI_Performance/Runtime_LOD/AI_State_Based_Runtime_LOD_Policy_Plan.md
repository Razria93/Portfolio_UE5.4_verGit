# AI State-Based Runtime LOD Policy Plan

## 목적

지금까지의 Runtime LOD 작업은 축별 전역 CVar로 비용을 분리하는 방식이었다.

```text
EnemyMovementMode
EnemyAnimationMode
BTUpdateIntervalMode
DisableEnemyWeaponActor
DisableEnemyHitProcessing
DisableEnemyCombatFeedback
```

이 문서는 축별 실험 결과를 바탕으로, 실제 적용 가능한 상태 기반 Runtime LOD 정책 v1을 정의한다.

핵심 전환:

```text
축별 전역 On / Off 실험
-> Enemy 상태와 전투 relevance에 따른 tier 기반 정책
```

## 측정 근거

| 축 | 관찰 결과 | 정책 판단 |
| --- | --- | --- |
| AlertCap / Assignment | Alert 후보 수 제한이 CharacterMovement p95에 직접 영향을 줬다. | 핵심 정책으로 유지 |
| BT Update Interval | service 호출 수 감소는 유효하지만 Frame / Game p95 개선은 제한적이었다. | tier별 update precision으로 사용 |
| Movement / Nav | movement intent block은 비용 감소 가능성이 있으나 전역 적용은 gameplay를 깨뜨린다. | 상태 기반으로만 적용 |
| Animation Refresh | parameter refresh 감소 gate는 동작했지만 단독 frame 개선은 약했다. | Observe / Idle 이하 보조 정책 |
| Enemy Actor Tick | `CEnemy Tick` 제거는 가능하지만 주요 병목은 아니었다. | 낮은 우선순위 |
| WeaponActor | 비용 축으로 유효했다. | Observe 이하 후보, v1 직접 적용은 보수적 판단 |
| Combat Hit Pipeline | hit window / hit processing 차단 효과는 현재 조건에서 작았다. | v1 핵심 제어에서 제외 |
| Feedback Presentation | Enemy feedback skip은 정상 작동했지만 frame 개선은 작았다. | 최하위 representation 후보 |
| Perception | 비활성화는 입력 자체를 흔들 수 있다. | v1에서는 끄지 않고 후속 active budget으로 분리 |

## 기준

### Tier 기준

| Tier | 기준 |
| --- | --- |
| Engage | CombatEngage subsystem에서 Engage role을 받은 직접 전투 참여자 |
| Alert | CombatEngage subsystem에서 Alert role을 받은 전투 후보 / 경계 참여자 |
| Observe | target awareness는 있지만 combat role이 없는 관찰 상태 |
| Idle | target awareness가 없고 전투 / 조사 중이 아닌 일반 상태 |
| Dormant | player와 멀고 시야에도 들어오지 않는 비활성 후보 |

### Dormant 기준

`Dormant`는 움직이지 않는 비활성 객체로 한정한다.

멀리 있더라도 patrol, return home, scripted idle move처럼 목적지 이동이 필요한 객체는 `Dormant`로 내리지 않는다.
이 경우는 `Idle` tier의 `Idle Movement Low` 정책으로 처리한다.

```text
이동이 필요함
-> Idle Movement Low

이동이 필요 없음 + 멀리 있음 + 시야 밖
-> Dormant 후보
```

### Combat Hit Pipeline 기준

이 문서의 `Combat Hit Pipeline`은 전투 공격 판정 흐름을 뜻한다.

```text
Combat Hit Pipeline:
-> AnimNotify Collision Begin / End
-> ActionCollisionWindow Begin / End
-> WeaponComponent Open / Close
-> HitWindow Open / Close
-> HitWindow Overlap
-> HitProcessing
-> CombatSignal
-> CombatSignalCue route
```

### Feedback 기준

Feedback Presentation은 combat result와 분리한다.

```text
Action feedback:
-> trail / attack VFX / attack SFX

Reaction feedback:
-> hit / guard / parry reaction presentation

Hit feedback:
-> hit VFX / SFX / decal / camera shake
```

`HitStop`은 단순 presentation이 아니라 timing에 영향을 주므로 Feedback Presentation off 대상에 포함하지 않는다.

## 정책표

| Tier | Engage | Alert | Observe | Idle | Dormant |
| --- | --- | --- | --- | --- | --- |
| Cap | 2 | 6 | 12 후보 | else | far / invisible |
| Perception | High | Reduced | Low | Low or Budgeted | Off + Wake-up |
| Movement | High | Reduced | None | None / Idle Movement Low | None |
| BT Update | High | Reduced | Low | Low | Off / VeryLow |
| Animation | Full | Full | Reduced | Reduced | Off |
| Mesh | On | On | On | On | Hidden / Proxy |
| Weapon Actor | On | On | Optional Visible | Off | Off |
| Combat Hit Pipeline | On | Off until Engage | Off | Off | Off |
| Hit Receive / Reaction | On | On | On | On / Minimal | Wake-up Hit only |
| Feedback Presentation | Action + Reaction + Hit | Reaction + Hit | Reaction + Hit | Minimal Hit / Off | Off |

## 적용 예시

### Engage

직접 전투 참여자다.

```text
Movement: High
BT Update: High
Animation: Full
WeaponActor: On
Combat Hit Pipeline: On
Feedback: Action + Reaction + Hit
```

Engage는 montage notify / socket timing / hit window / combat signal route가 모두 유지되어야 한다.
따라서 pose update skip, WeaponActor 제거, Combat Hit Pipeline 제거는 금지한다.

### Alert

전투 후보이자 경계 참여자다.

```text
Movement: Reduced
BT Update: Reduced
Animation: Full
WeaponActor: On
Combat Hit Pipeline: Off until Engage
Feedback: Reaction + Hit
```

Alert는 언제든 Engage로 승격될 수 있으므로 movement, perception, weapon, full animation을 보수적으로 유지한다.
다만 직접 공격자는 아니므로 outgoing hit 처리와 action feedback은 Engage 전까지 의미가 없다.

### Observe

target awareness는 있지만 combat assignment가 없는 상태다.

```text
Movement: None
BT Update: Low
Animation: Reduced
WeaponActor: Optional Visible
Combat Hit Pipeline: Off
Feedback: Reaction + Hit
```

Observe는 전투 권한이 없으므로 Chase / Alert Spread / Attack에 참여하지 않는다.
v1에서는 Observe에서 movement intent를 제한하는 것이 핵심 적용 후보가 된다.

### Idle

target awareness가 없고 전투 / 조사 중이 아닌 상태다.

```text
Movement: None / Idle Movement Low
BT Update: Low
Animation: Reduced
WeaponActor: Off
Combat Hit Pipeline: Off
Feedback: Minimal Hit / Off
```

`Idle Movement`는 target awareness / combat assignment와 무관한 비전투 이동을 뜻한다.

```text
Patrol
Return Home
Scripted idle move
멀리 있지만 목적지로 이동해야 하는 상태
```

### Dormant

움직이지 않는 비활성 후보 상태다.

```text
Movement: None
BT Update: Off / VeryLow
Animation: Off
Mesh: Hidden / Proxy
WeaponActor: Off
Combat Hit Pipeline: Off
Feedback: Off
```

Dormant는 자기 자신이 아닌 외부 wake-up 주체가 깨워야 한다.
Dormant에서 BT MoveTo 기반 이동은 허용하지 않는다.
BT MoveTo는 PathFollowing / CharacterMovement / MovementComponent 흐름을 사용하므로 `Movement None` 정책과 충돌한다.

## Dormant Wake-up

Dormant는 Perception / BT / Movement가 꺼지거나 매우 낮은 빈도로 줄어든 상태다.
따라서 Perception Off를 적용하려면 별도 wake-up 정책이 필요하다.

후보:

```text
1. Player-centered sphere query
2. Player-centered sphere component overlap
3. Player distance + camera forward cone
4. damage / noise / scripted event
```

v1 이후 추천 구조:

```text
Dormant registry + 주기적 distance / sphere check
```

이유:

```text
매 프레임 SphereTrace보다 비용을 통제하기 쉽다.
collision channel 설정에 덜 의존한다.
80~200 Enemy 수준에서는 0.25~0.5초 간격 거리 제곱 비교가 충분히 현실적이다.
```

Wake-up manager의 책임:

```text
Dormant 해제
Perception / BT / minimal representation 복구
이후 CombatEngage assignment 흐름에 맡김
```

Wake-up manager는 combat role을 직접 부여하지 않는다.

## v1 계획

v1은 정책 전체를 한 번에 구현하지 않는다.
측정상 가장 의미가 있었고 gameplay risk가 낮은 축부터 적용한다.

### v1 적용 후보

1. Tier resolver 추가
   - `CombatRole`
   - `AIIntentState`
   - target awareness
   - distance / visibility 후보

2. Movement
   - Observe 이하 movement intent 제한
   - Idle은 필요할 때만 `Idle Movement Low`
   - Dormant는 movement 없음

3. BT Update
   - tier별 interval / precision 연결
   - EngageContext는 기본 주기 유지

4. Animation
   - Observe / Idle reduced refresh
   - combat-capable tier는 Full 유지

### v1에서 하지 않는 것

```text
Perception Off / Active Budget
Mesh Hidden / Proxy
WeaponActor 생성 제거
Combat Hit Pipeline 제거
Feedback Presentation 제거
Dormant full implementation
```

이 항목들은 정책표에는 남기되, v1 직접 적용 대상에서 제외한다.

## 측정 계획

1차 측정:

```text
40 Enemy
Policy Off
Policy On
```

2차 측정:

```text
80 Enemy
Policy Off
Policy On
```

고정 조건:

```text
Engage 2 / Alert 6
fixed camera
-noailogging
first 3s / last 3s trim, middle 30s used
GC event 없는 측정 우선 사용
```

확인 항목:

```text
Engage 2 / Alert 6 유지
Observe가 움직이지 않는지
Idle / Idle Movement 정책이 의도대로 동작하는지
Attack / HitReact / Investigate 깨짐 없는지
CharacterMovement p95
Animation p95
BT Tick p95
AIContext / AIIntent / EngageContext count
Frame / Game p95
```

## 성공 기준

```text
Combat-capable 흐름이 깨지지 않는다.
Observe / Idle 계층에서 불필요한 movement work가 줄어든다.
Animation reduced가 visual break 없이 보조 효과를 낸다.
BT update precision이 기존 Assignment / AlertCap 정책과 충돌하지 않는다.
```

Frame / Game p95 개선이 작더라도, tier별 work reduction이 명확하고 gameplay smoke가 안정적이면 Runtime LOD v1 정책 기반으로 유지한다.

## 최적화 이슈 마감 기준

이 브랜치가 완료되면 현재 AI Runtime LOD 최적화 이슈는 1차 결론으로 마감한다.

마감 기준:

```text
State-based Runtime LOD Policy v1 적용
40 / 80 Enemy 측정
gameplay smoke 확인
측정 결과와 정책 결론 문서화
```

이 시점의 결론은 다음과 같이 정리한다.

```text
대량 Enemy를 모두 같은 비용으로 업데이트하지 않는다.
Engage / Alert / Observe / Idle / Dormant tier로 나눈다.
Combat-capable 객체는 보수적으로 유지한다.
전투 권한이 없는 객체부터 movement / BT / animation 비용을 줄인다.
Perception / proxy / mesh hidden / wake-up manager는 후속 고도화로 남긴다.
```

따라서 이 브랜치의 완료는 “최적화 전체 종료”가 아니라 `AI Runtime LOD v1 마감`으로 본다.
이후 작업은 새로운 최적화 이슈가 아니라 v1 정책 위에 얹는 후속 고도화로 분리한다.

후속 고도화 후보:

```text
1. Perception Active Budget / Wake-up
2. Dormant / Proxy Actor
3. Animation Budget / Pose Skip
4. WeaponActor spawn delay / pooling
5. Distance / camera visibility 기반 LOD policy tuning
6. 120+ Enemy stress scene
```
