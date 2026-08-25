# AI Runtime LOD Debugging Obstacle Note

> 2026.08.24 closure note: any historical statement below that describes Movement mode `0 / 1 / 2` is a profiling record. The current movement policy retains only `0: Default` and `1: BlockMovementIntent`; state-refresh tick disable was not adopted.

## 목적

P35 AI Runtime LOD 작업 중 측정 방향과 설계 포인트를 바꾸게 만든 장애요소를 정리한다.

이 문서는 단순 시행착오 기록이 아니라, 성능 최적화 과정에서 병목을 잘못 해석할 수 있었던 지점과 이를 어떻게 분리 / 검증 / 보정했는지를 남기기 위한 문서다.
리팩터링, 디버깅, 계측 환경 개선의 근거로 사용한다.

## 요약

```text
초기 목표:
BT Service tick / update interval 최적화 경험을 확보한다.

실제 진행:
AI tick만 문제가 아니라 render, animation, weapon actor, perception, movement, BT service scheduling, assignment policy가 서로 영향을 주는 것을 확인했다.

최종 방향:
단일 interval 값을 낮추는 방식이 아니라,
Engage / Alert / Idle 계층을 먼저 안정화하고,
그 계층에 따라 AIContext / AIIntentState update precision을 다르게 적용하는 Runtime LOD 정책으로 전환했다.
```

## 장애요소와 해결 과정

### 1. 측정 환경 자체가 병목처럼 보이는 문제

문제:

```text
60 Enemy 이상에서 frame drop, OUT OF MEMORY 경고, perception 지연이 관찰됐다.
처음에는 BT tick 비용이 직접 원인인지, 렌더링 / 충돌 / crowd / weapon / perception이 섞인 결과인지 분리되지 않았다.
```

원인 판단:

```text
Enemy끼리 충돌하거나 피격되는 조건, 밀집된 patrol layout, 같은 지점으로 몰리는 movement가 측정값을 오염했다.
따라서 BT update 비용만 보고 있다고 보기 어려웠다.
```

해결:

```text
AI performance profiling 전용 asset / map을 분리했다.
Enemy 간 피격을 차단하고, 충돌체 / patrol area / MoveTo 허용반경 / PlayerStart 위치를 측정 목적에 맞게 조정했다.
40 / 80 Enemy를 반복 측정 가능한 기준 scale로 고정했다.
```

어필 포인트:

```text
최적화 전에 측정 환경의 오염 변수를 먼저 제거했다.
프레임 드랍 원인을 바로 코드 최적화로 몰지 않고, 실험 조건을 정리해서 재현성과 비교 가능성을 확보했다.
```

### 2. CSV 해석 기준이 불안정했던 문제

문제:

```text
같은 조건에서도 CSV p95가 흔들렸고, GC 이벤트나 측정 시간 차이 때문에 해석이 매번 늦어졌다.
Frame / Game p95만 보고 결론을 내리면 작은 단위 최적화가 오차에 묻혔다.
```

원인 판단:

```text
CSV capture 중 GC 이벤트가 들어오면 p95가 튈 수 있다.
또한 30초 전후의 측정 구간에서 초반 안정화와 종료 입력 구간이 섞이면 값이 흔들린다.
```

해결:

```text
CSV_Analysis_Guide를 만들고 first 3s / last 3s trim, middle 30s 중심 해석 기준을 고정했다.
측정 로그와 CSV 파일을 함께 보관하고, GC 이벤트가 있는 측정은 대표값에서 제외하거나 보조 측정으로 분리했다.
Frame / Game p95뿐 아니라 직접 counter와 BT Tick p95를 함께 보게 했다.
```

어필 포인트:

```text
성능 측정을 수치 하나로 단정하지 않고, 측정 절차와 해석 기준을 문서화했다.
작은 단위 최적화가 GC / capture noise에 묻히지 않도록 분석 기준을 표준화했다.
```

### 3. Mesh hidden 측정이 animation 비용 측정과 섞인 문제

문제:

```text
Enemy mesh를 숨겼을 때 render 비용만 줄어드는 것이 아니라 pose update / socket follow / montage timing에도 영향이 생겼다.
런타임 중 Mode 2로 바꾼 경우와 PIE 시작 전 Mode 2로 시작한 경우의 동작도 달랐다.
```

원인 판단:

```text
Skeletal mesh visibility, pose update, animation tick, socket transform 갱신은 서로 분리된 축이다.
mesh hidden은 render 측정에 유용하지만, pose update skip까지 함께 들어가면 gameplay timing이 깨질 수 있다.
현재 combat timing은 montage notify와 강하게 연결돼 있다.
```

해결:

```text
EnemyMeshMode를 VisibleDefault / HiddenKeepPose / HiddenAllowPoseSkip으로 분리했다.
combat-capable 조건에서는 notify / socket timing 때문에 pose update skip을 금지한다는 정책을 세웠다.
RenderCoverage 전용 map을 만들고 render 비용 측정과 gameplay stress 측정을 분리했다.
```

어필 포인트:

```text
눈에 보이는 mesh visibility와 실제 animation / combat timing dependency를 분리했다.
LOD 정책을 render만이 아니라 gameplay dependency 기준으로 설계했다.
```

### 4. WeaponActor 비용 축이 action / feedback 비용과 섞인 문제

문제:

```text
WeaponActor를 빼면 actor 수는 줄지만, 전투 action / hit processing / feedback까지 같이 줄어드는지 구분이 필요했다.
초기에는 weapon actor presence cost와 combat bundle cost가 섞일 수 있었다.
```

원인 판단:

```text
WeaponActor는 생성 비용, mesh / shadow, collision / hit context, trail / Niagara feedback과 각각 다른 책임을 가진다.
따라서 weapon actor를 단순히 최하위 LOD 계층으로 고정하면 정책이 흐려진다.
```

해결:

```text
DisableEnemyWeaponActor CVar를 추가해 presence cost를 먼저 분리했다.
생성 스킵 조건, 타입 보존, 후속 처리 API를 분리해 profiling 예외 흐름이 정규 equip 흐름과 섞이지 않도록 정리했다.
WeaponActor는 Simulation / Representation / Combat Processing / Feedback 축에 걸쳐 판단하는 것으로 정책을 정리했다.
```

어필 포인트:

```text
단순히 actor를 끄는 것이 아니라, 어떤 비용 축을 줄이는지 구분했다.
프로파일링 예외 코드를 정규 코드 흐름에서 읽히도록 API 경계를 정리했다.
```

### 5. Perception 후보 누수와 target 식별 지연 문제

문제:

```text
80 Enemy 조건에서 player를 perception range 안에 두어도 Blackboard TargetActor가 늦게 잡혔다.
debug perception을 보면 Enemy끼리도 candidate로 잡히는 정황이 있었다.
```

원인 판단:

```text
팀 식별 / affiliation 정책이 명확하지 않아 같은 팀 Enemy도 perception candidate로 들어왔다.
OnTargetPerceptionUpdated 내부의 cast 비용 자체보다, 후보 수와 engine perception update scheduling이 더 큰 변수였다.
```

해결:

```text
PerceptionCandidateAudit과 BlackboardEngageLatencyAudit을 추가했다.
Raw candidate, valid provider, invalid provider, first raw latency, first valid latency를 분리했다.
Team attitude / affiliation 설정을 보완해 불필요한 후보 유입을 줄였다.
```

어필 포인트:

```text
감으로 perception이 느리다고 판단하지 않고 raw candidate와 valid target provider 사이의 지연을 계측했다.
팀 식별 정책 누수라는 설계 문제를 성능 이슈와 함께 추적했다.
```

### 6. Movement/Nav LOD가 representation을 깨뜨린 문제

문제:

```text
MovementComponent tick을 끄면 이동 비용은 줄어들 수 있지만, Alert Spread 중 locomotion representation이 깨졌다.
Intent block은 움직임 자체를 막아 gameplay 상태를 바꾸는 효과가 있었다.
```

원인 판단:

```text
movement/nav/update는 Simulation LOD에 속하고, locomotion animation은 Representation LOD에 속한다.
MovementComponent tick off는 단순 비용 절감이 아니라 gameplay와 visual state를 동시에 흔든다.
```

해결:

```text
MovementMode를 Default / MovementStateRefreshDisabled / MovementIntentBlocked로 분리했다.
측정 후 MovementComponent tick off는 성능 이득 대비 representation 손상이 커서 우선 후보에서 제외했다.
movement 제어는 별도 제한 조건에서만 검토하기로 했다.
```

어필 포인트:

```text
수치상 개선 가능성보다 실제 gameplay / visual 품질 손상을 우선 판단했다.
LOD 후보에서 제외하는 판단도 측정 결과와 관찰 근거로 남겼다.
```

### 7. BT Service interval이 asset 기본값 때문에 제어되지 않은 문제

문제:

```text
C++ 생성자에서 BT service interval 기본값을 바꿔도 이미 저장된 BT asset의 직렬화된 interval 값이 유지됐다.
OnBecomeRelevant에서 interval을 덮어쓰는 방식도 실제 scheduling에 기대만큼 반영되지 않았다.
```

원인 판단:

```text
BT Service node는 asset에 저장된 interval property를 가지고 있고, runtime tick scheduling은 단순 property 변경만으로 제어되지 않을 수 있다.
```

해결:

```text
ScheduleNextTick / SetNextTickTime 기반으로 다음 service tick 시간을 명시적으로 예약하는 구조로 변경했다.
BT service interval helper를 만들고 AIContext / AIIntentState / EngageContext interval 선택 책임을 모았다.
```

어필 포인트:

```text
에디터 asset 값과 runtime scheduling의 차이를 확인하고, 실제 runtime 제어 지점으로 리팩터링했다.
```

### 8. Active count를 실제 호출수로 오해한 문제

문제:

```text
stat / CSV에서 보이는 active count만으로 service 호출수 감소를 판단하려 했다.
Mode 1 / 2 차이가 명확하지 않아 interval 정책이 적용됐는지 판단이 흔들렸다.
```

원인 판단:

```text
active count는 실제 service TickNode 호출수와 동일한 지표가 아니다.
호출수와 interval preset 선택 분포를 직접 기록해야 했다.
```

해결:

```text
UpdateAIContext / UpdateAIIntentState / UpdateEngageContext 직접 counter를 추가했다.
Default / Reduced / Aggressive interval preset 선택 counter도 추가했다.
이후 Mode 0 / 1 / 2가 의도한 preset을 선택하는지 검증했다.
```

어필 포인트:

```text
엔진 stat을 그대로 믿지 않고, 프로젝트 의도에 맞는 계측 지표를 직접 추가했다.
```

### 9. Interval policy 구현 오류

문제:

```text
Mode 1 / 2가 CSV에서 비슷하게 나왔고, Low precision에서 Reduced / Aggressive 차이가 발생하지 않았다.
문법 오류나 컴파일 오류가 아니라 정책 구현 오류였다.
```

원인 판단:

```text
Mode와 precision을 함께 고려해야 하는데, Low precision에서 Mode 1 / 2의 차이가 명확히 드러나지 않는 구조였다.
```

해결:

```text
Mode x Precision matrix로 읽히도록 interval preset 선택 함수를 정리했다.
Mode 0: High / Reduced / Low 모두 Default
Mode 1: High Default, Reduced / Low Reduced
Mode 2: High Default, Reduced Reduced, Low Aggressive
```

어필 포인트:

```text
성능 최적화 코드에서도 정책 matrix가 읽히도록 API와 함수 배치를 정리했다.
단순 동작보다 유지보수 가능한 정책 표현을 우선했다.
```

### 10. Assignment gate 없이 interval만 줄이면 AI가 Idle로 빠지는 문제

문제:

```text
BTUpdateIntervalMode 1 / 2에서 Enemy가 Engage / Alert로 안정적으로 들어가지 못하고 Idle로 빠졌다.
interval 감소 자체는 먹혔지만 assignment 형성 전에 상태가 Idle로 눌리는 흐름이 생겼다.
```

원인 판단:

```text
RequestContainer가 비는 구간과 AssignmentContainer 갱신 타이밍 때문에 CombatRole None 구간이 생겼다.
AIIntentState가 이 구간을 읽으면 Idle로 빠졌다.
즉 문제는 interval 값이 아니라 assignment lifetime / bootstrap 구조였다.
```

해결:

```text
Engage / Alert / Idle assignment gate를 명확히 했다.
Assignment lease / preserve 정책을 추가해 이미 확정된 역할이 짧은 갱신 틈에 사라지지 않도록 했다.
Engage / Alert cap 안에 들어오지 못한 Enemy는 Chase / Investigate로 몰리지 않고 Idle wait 쪽으로 빠지도록 정리했다.
```

어필 포인트:

```text
최적화 값을 튜닝하기 전에 gameplay state ownership 문제를 먼저 해결했다.
성능 정책과 AI 상태 정책의 결합 지점을 정리했다.
```

### 11. 초기 assignment 선점 문제

문제:

```text
80 Enemy 시작 직후 request가 한 번에 80개 들어오지 않고 6 -> 13 -> 32 -> 62 -> 80처럼 단계적으로 채워졌다.
초기 6개 request가 먼저 Engage / Alert를 선점하면, 더 가까운 Enemy가 뒤늦게 request를 내도 lease / preserve 때문에 교체되지 않았다.
```

원인 판단:

```text
AI Perception, Blackboard, BT Service, EngageRequest 제출이 여러 프레임에 걸쳐 분산된다.
문제는 sort 기준이 아니라 불완전한 후보군을 너무 빨리 확정하는 데 있었다.
```

해결:

```text
EngageAssignmentWarmupTime CVar를 추가했다.
초기 warmup 동안 assignment 확정을 지연하고 request snapshot이 충분히 채워진 뒤 최초 assignment를 확정했다.
80 Enemy 기준 1.0s는 경계값이고, 1.2s가 현재 테스트 조건에서 더 안정적인 후보로 확인됐다.
```

어필 포인트:

```text
정렬 알고리즘을 무작정 바꾸지 않고, 후보군 수집 타이밍 문제를 먼저 확인했다.
로그를 통해 request snapshot 증가 패턴을 관찰하고, warmup이라는 정책적 해결로 연결했다.
```

## 최종 설계 포인트

```text
1. Runtime LOD는 단일 CVar로 전체 AI를 느리게 만드는 구조가 아니다.
2. Engage / Alert / Idle assignment 계층이 먼저 안정화되어야 한다.
3. EngageContext는 combat timing에 직접 관여하므로 기본 interval을 유지한다.
4. AIContext / AIIntentState는 assignment precision에 따라 interval을 줄일 수 있다.
5. Mode 1은 combat-capable 조건의 보수 후보로 본다.
6. Mode 2는 far / offscreen / NonCombat / Dormant 후보로 우선 분류한다.
7. Proxy / Dormant actor 최적화는 이후 별도 feature로 분리한다.
```

## PR 어필 포인트

```text
측정 환경 정리:
오염 변수 제거 후 40 / 80 Enemy 기준을 고정했다.

계측 도구 보강:
CSV 분석 기준, GC 확인, service counter, preset counter, perception audit, assignment audit을 추가했다.

정책 리팩터링:
BT interval을 asset 값 수정이 아니라 runtime scheduling helper로 제어했다.

디버깅:
perception 후보 누수, assignment 선점, CombatRole None 구간, interval policy 구현 오류를 로그와 CSV로 좁혀갔다.

설계 결론:
단순 tick 감소가 아니라 assignment 기반 Runtime LOD precision 정책으로 방향을 전환했다.
```
