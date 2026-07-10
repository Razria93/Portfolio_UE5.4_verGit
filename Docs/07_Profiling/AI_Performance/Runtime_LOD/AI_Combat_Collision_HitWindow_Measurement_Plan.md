# AI Combat Collision / Hit Window Measurement Plan

## 목적

`Combat Collision / Hit Window` 축이 40 / 80 Enemy 조건에서 실제 frame budget에 어느 정도 영향을 주는지 분리 측정한다.

이번 브랜치는 최종 Runtime LOD 구현이 아니라 비용 분리 측정이다.
`WeaponActor` 존재 비용, attack montage 비용, feedback presentation 비용과 섞지 않고, 공격 판정 window가 열렸을 때 발생하는 collision / overlap / hit processing 경로의 비용을 확인한다.

## 브랜치

```text
feature/ai-combat-collision-profiling
```

## 측정 질문

```text
Enemy WeaponActor와 attack montage를 유지한 상태에서
hit collision window만 차단하면
Frame / Game / collision / hit processing 관련 비용이 유의미하게 줄어드는가?
```

## 측정 범위

### 포함

```text
WeaponActor 생성 유지
WeaponActor attach / socket follow 유지
attack montage 실행 유지
AnimNotify / hit window notify route 유지
Engage / Alert / Observe 정책 유지
```

### 1차로 분리할 축

```text
hit window open 시 weapon collision을 실제로 켜는지 여부
weapon overlap / hit processing route에 진입하는지 여부
```

### 1차에서 제외

```text
WeaponActor 생성 자체 제거
attack action / montage 제거
trail / Niagara / sound / camera shake 제거
damage / combat signal route만 별도 차단
proxy / dormant actor 전환
```

Feedback presentation은 다음 축으로 분리한다.
이번 측정에서 feedback을 함께 끄면 hit collision 비용과 feedback 비용을 구분하기 어렵다.

## 현재 코드 스캔 대상

구현 전 아래 파일과 경로를 먼저 확인한다.

```text
Source/Portfolio/Weapon/CAttachment.cpp
Source/Portfolio/Weapon/CAttachment.h
Source/Portfolio/Component/CWeaponComponent.cpp
Source/Portfolio/Component/CApplyDamageComponent.cpp
Source/Portfolio/Component/CApplyDamageComponent.h
Source/Portfolio/Component/CombatSignalSourceComponent*
Source/Portfolio/Component/CombatSignalTargetComponent*
Source/Portfolio/Type/CWeaponStructure.h
```

확인할 질문:

```text
1. hit window open / close가 어디서 호출되는가?
2. collision enable / disable 책임은 어디에 있는가?
3. overlap event는 어디서 수신하는가?
4. hit context / damage payload는 어디서 생성되는가?
5. 한 hit window에서 동일 target 중복 hit를 어디서 막는가?
6. hit processing과 feedback presentation이 같은 함수에 섞여 있는가?
```

## CVar 계획

1차 CVar는 하나만 둔다.

```text
Portfolio.AI.Profiling.DisableEnemyWeaponHitCollision
```

의미:

```text
0: 기본 전투 판정 유지
1: Enemy WeaponActor는 유지하지만 Enemy weapon hit collision window만 차단
```

적용 원칙:

```text
Enemy만 대상
Player weapon은 유지
WeaponActor 생성은 유지
attack montage는 유지
AnimNotify 호출은 유지
collision enable 또는 overlap processing 진입만 차단
```

이 CVar는 profiling 전용이다.
실제 Runtime LOD 정책으로 바로 사용하기보다, hit collision / overlap 비용이 유효한 축인지 판단하기 위한 측정 스위치로 둔다.

## 카운터 계획

측정값 해석을 위해 최소 카운터를 추가한다.
카운터는 비용 자체보다 "해당 경로가 실제로 실행되었는지 / 차단되었는지"를 검증하는 목적이다.

권장 카운터:

```text
PortfolioAI_HitWindow_Open_Count
PortfolioAI_HitWindow_Close_Count
PortfolioAI_HitWindow_Overlap_Count
PortfolioAI_HitProcessing_Count
PortfolioAI_CombatSignal_Count
```

1차 구현에서 모든 카운터를 넣기 어렵다면 최소 기준은 다음과 같다.

```text
PortfolioAI_HitWindow_Open_Count
PortfolioAI_HitWindow_Overlap_Count
PortfolioAI_HitProcessing_Count
```

## 공통 측정 조건

P36 / P37에서 안정화한 조건을 유지한다.

```text
Capture Duration: 약 36초
Analysis Window: first 3s / last 3s trimmed, middle 30s used
Log State: -noailogging
PIE: F11 fullscreen
GC Event: none 권장
```

공통 CVar:

```text
Portfolio.AI.RuntimeLOD.EngageAssignmentWarmupTime 1.2
Portfolio.AI.RuntimeLOD.EngageAssignmentEngageCap 2
Portfolio.AI.RuntimeLOD.EngageAssignmentAlertCap 6

Portfolio.AI.RuntimeLOD.BTUpdateIntervalMode 0
Portfolio.AI.RuntimeLOD.EnemyMeshMode 0
Portfolio.AI.RuntimeLOD.EnemyAnimationMode 0
Portfolio.AI.RuntimeLOD.EnemyAnimationRefreshCounter 0
Portfolio.AI.RuntimeLOD.DisableEnemyWeaponActor 0
Portfolio.AI.RuntimeLOD.DisableEnemyPerception 0
Portfolio.AI.RuntimeLOD.PerceptionCandidateAudit 0
Portfolio.AI.RuntimeLOD.BlackboardEngageLatencyAudit 0
Portfolio.AI.RuntimeLOD.CanMoveDecoratorAudit 0
Portfolio.AI.RuntimeLOD.EnemyMovementMode 0
```

비교 CVar:

```text
Portfolio.AI.Profiling.DisableEnemyWeaponHitCollision 0
Portfolio.AI.Profiling.DisableEnemyWeaponHitCollision 1
```

## 권장 map

기준 map은 P37 이후 gameplay stress 조건이 유지되는 map을 사용한다.

추천:

```text
MAP_AIPerf_ObserveIntent_40Enemy
MAP_AIPerf_ObserveIntent_80Enemy
```

별도 map이 필요해지면 다음 이름을 사용한다.

```text
MAP_AIPerf_CombatCollision_40Enemy
MAP_AIPerf_CombatCollision_80Enemy
```

map을 새로 만들지는 구현 후 실제 측정 안정성에 따라 결정한다.
기존 ObserveIntent map에서 Engage 2 / Alert 6 / Observe 흐름이 안정적으로 유지되면 그대로 사용한다.

## 권장 측정 순서

1차 측정:

```text
1. 40 Enemy / FullCombat / DisableEnemyWeaponHitCollision 0
2. 40 Enemy / HitCollisionDisabled / DisableEnemyWeaponHitCollision 1
3. 80 Enemy / FullCombat / DisableEnemyWeaponHitCollision 0
4. 80 Enemy / HitCollisionDisabled / DisableEnemyWeaponHitCollision 1
```

40 Enemy에서 카운터가 의도대로 줄지 않으면 80 Enemy 측정으로 넘어가지 않는다.

측정 전 시각 확인:

```text
Engage 2 유지
Alert 6 유지
나머지 Observe 또는 Idle 유지
Enemy WeaponActor 생성 유지
attack montage 실행 유지
DisableEnemyWeaponHitCollision 1에서 hit overlap / hit processing count 감소 확인
```

## 분석 지표

우선 지표:

```text
Frame p95
Game p95
Exclusive/GameThread/BehaviorTreeTick
Exclusive/GameThread/CharacterMovement
PortfolioAI_HitWindow_Open_Count
PortfolioAI_HitWindow_Overlap_Count
PortfolioAI_HitProcessing_Count
PortfolioAI_CombatSignal_Count
```

보조 지표:

```text
AIContext Count
AIIntent Count
EngageContext Count
ActorCount / Ticks 계열
CSV 로그의 GC 이벤트 여부
```

## 해석 기준

### A. Frame / Game p95가 줄고 hit counter도 줄어드는 경우

```text
Hit collision / overlap / hit processing은 유효한 최적화 후보로 본다.
다음 단계에서 overlap과 hit processing을 추가 분리한다.
```

### B. Frame / Game p95는 비슷하지만 hit counter만 줄어드는 경우

```text
차단 기능은 정상이나 현재 40 / 80 Enemy 조건에서는 주요 병목이 아니다.
Collision / Hit Window는 Runtime LOD 우선순위를 낮춘다.
```

### C. CharacterMovement / BT Tick 변동이 더 큰 경우

```text
collision보다 assignment / movement 후보 수가 더 큰 변수다.
P36 AlertCap 결과와 함께 해석한다.
```

### D. counter가 줄지 않는 경우

```text
CVar 적용 위치가 hit collision route를 실제로 막지 못한 것이다.
측정값은 폐기하고 구현 위치를 다시 잡는다.
```

## 후속 분기

1차 결과가 유의미하면 다음 측정으로 쪼갠다.

```text
Case 1: weapon collision만 차단
Case 2: overlap은 받지만 hit processing 차단
Case 3: hit processing은 유지하되 feedback presentation 차단
```

1차 결과가 유의미하지 않으면 다음 축으로 넘어간다.

```text
Feedback Presentation
Component Tick Audit
Perception Active Budget / Cap
```

## 완료 기준

```text
1. hit collision 차단 CVar가 Enemy 전용으로 동작한다.
2. WeaponActor / attack montage는 유지된다.
3. 40 / 80 Enemy에서 FullCombat과 HitCollisionDisabled를 쌍으로 측정한다.
4. GC 이벤트 없는 대표값을 채택한다.
5. counter 기준으로 차단 여부를 검증한다.
6. Frame / Game / CharacterMovement / BT Tick / hit counter를 표로 정리한다.
7. Collision / Hit Window가 Runtime LOD 후보인지, 후순위 축인지 결론을 남긴다.
```
