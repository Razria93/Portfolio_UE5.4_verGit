# TB W05-02 Player Targeting Debug Observability v1

## 작업명

Player Targeting Debug Observability v1

## 브랜치

```text
feat/player-targeting-component
```

## 상태

```text
진행
```

## 목적

타게팅 런타임의 선택·유지 책임과 디버그 표현 책임을 분리한다. 또한 Debug Overlay가 PlayerTarget을 Focus 출처로 선택하고, Live Sync 또는 Freeze 상태로 관찰할 수 있게 한다.

## 결정 사항

- `UCTargetingComponent`는 월드 디버그를 그리지 않는다.
- 타게팅 컴포넌트는 `FTargetingDebugSnapshot`을 읽기 전용으로 제공한다.
- `FTargetingDebug`가 World Debug Draw와 Targeting CVar를 소유한다.
- Player CurrentTarget과 Debug Overlay FocusTarget은 별도 상태다.
- `PlayerTargetLive`는 CurrentTarget 변경을 Overlay Focus에 반영한다.
- `PlayerTargetFrozen`은 마지막 동기화 FocusTarget을 유지한다.
- Frozen FocusTarget이 파괴되면 FocusActor만 안전하게 비운다.
- 수동 Selected / RecentCombat Focus는 PlayerTarget Driver가 아닐 때 기존 동작을 유지한다.

## 작업 범위

### 1. 타게팅 Debug Snapshot

다음 값을 타게팅 컴포넌트에서 제공한다.

```text
ViewLocation / ViewForward
TargetActor / TargetLocation
Distance / MaxTargetDistance
Dot / MinDot
AngleScore / DistanceScore / FinalScore
InRange / InViewCone
```

### 2. World Debug Draw

다음 표시를 독립 CVar로 제어한다.

```text
Range Sphere
Selected Target Sphere
Viewpoint to Target Line
World Debug Text
```

### 3. PlayerTarget Focus

Focus Source와 Driver를 아래처럼 추가한다.

```text
Source: PlayerTargetFocus
Driver: PlayerTargetLive
Driver: PlayerTargetFrozen
```

`LiveSyncPlayerTarget` CVar가 켜지면 Live Driver가 CurrentTarget을 갱신한다. 끄면 Frozen Driver로 전환하여 마지막 FocusActor를 유지한다.

### 4. Overlay와 Editor Panel

Debug Overlay에는 Runtime PlayerTarget의 거리, Dot, 점수, 범위/시야 판정을 표시한다.

Editor Plugin에는 Targeting 섹션과 `Select Player Target Focus` 명령을 제공한다.

## 완료 조건

- 타게팅 컴포넌트가 DrawDebugHelpers를 직접 포함하지 않는다.
- Targeting Debug를 켜면 4개 표시를 개별적으로 제어할 수 있다.
- PlayerTarget Focus를 선택하면 Overlay Focus Source가 PlayerTargetFocus로 표시된다.
- Live Sync ON에서 타겟 변경은 FocusActor를 갱신한다.
- Live Sync OFF에서 타겟 변경은 FocusActor를 갱신하지 않는다.
- Frozen FocusActor가 파괴되면 Overlay FocusActor가 비워진다.
- Editor Win64 Development 빌드가 성공한다.
