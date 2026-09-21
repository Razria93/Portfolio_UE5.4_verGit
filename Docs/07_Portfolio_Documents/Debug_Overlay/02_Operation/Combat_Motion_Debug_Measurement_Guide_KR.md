# 넉백·공격 시작 페이싱 디버그 측정 가이드

기준일: 2026-09-21. 현재 구현의 사용법과 판독 기준을 정리한다. 개발용 PIE/Non-Shipping 대상이며 Shipping에서는 진단 수집·저장·출력을 제외한다.

구현 구조와 변경 파일은 [Combat Motion Diagnostics](../../Portfolio_Production/38_Gameplay_Combat_Motion_Diagnostics.md)를 참고한다.

## 1. 먼저 알아둘 구분

| 기능 | 확인하는 내용 | 확인하지 않는 내용 |
| --- | --- | --- |
| Knockback | 피격 설정 해석, 실제 Hit 시작, 이동 소스 적용·종료, 이론 거리와 실제 변위 | 충돌 원인 자동 판정, 이동 거리 보장 |
| AttackFacing | 공격 시작 시 타깃 방향 보정의 판정·결과 | 액션 내내 연속 추적, 카메라 회전 |
| 기존 CombatTargetFacing / Facing | Enemy AI Focus·회전 정책 | 공격 시작 페이싱의 적용 결과 |

화면의 `Current`는 현재 상태, `Last`는 마지막 시도 결과다. 현재 비활성이면서 마지막 결과가 정상 적용·종료인 것은 모순이 아니다. `NotCaptured`는 기록이 없다는 뜻이지 기능 실패라는 뜻이 아니다.

## 2. 빠른 시작

### 표시 설정

PIE에서 공격하기 **전에** 콘솔 명령을 한 줄씩 입력한다. Editor 디버그 설정 도구의 해당 항목으로도 제어할 수 있다.

```text
Portfolio.DebugOverlay.HUDVisible 1
Portfolio.DebugOverlay.Knockback.Enabled 1
Portfolio.DebugOverlay.Knockback.DrawWorld 1
Portfolio.DebugOverlay.AttackFacing.Enabled 1
Portfolio.DebugOverlay.AttackFacing.DrawWorld 1
```

월드 드로우 대상은 플레이어와 **디버그 Focus Enemy**다. 모든 피격 Enemy를 자동으로 그리는 방식이 아니다. 플레이어가 적을 타게팅한 뒤 다음 명령으로 디버그 Focus를 맞출 수 있다.

```text
DebugOverlaySelectPlayerTargetFocus
```

패널이 꺼져 있다면 다음 항목도 켠다.

```text
Portfolio.DebugOverlay.Player.Enabled 1
Portfolio.DebugOverlay.Enemy.Enabled 1
Portfolio.DebugOverlay.Player.Knockback.Enabled 1
Portfolio.DebugOverlay.Enemy.Knockback.Enabled 1
Portfolio.DebugOverlay.Player.AttackFacing.Enabled 1
Portfolio.DebugOverlay.Enemy.AttackFacing.Enabled 1
```

### 이벤트 확인이 필요할 때

```text
Portfolio.DebugOverlay.CaptureEnabled 1
Portfolio.DebugOverlay.EventLogScope FocusedEnemy
Portfolio.DebugOverlay.EventLogFilter Knockback
```

페이싱을 볼 때 필터를 `AttackFacing`, 전체를 볼 때 `All`로 바꾼다. Focus가 의심되면 Scope를 `World`로 바꿔 비교한다. Output Log가 필요하면 별도로 켠다.

```text
Portfolio.Debug.KnockbackAudit 1
Portfolio.Debug.AttackFacingAudit 1
```

Audit는 월드 드로우의 필수 조건이 아니다. 측정이 끝나면 필요 없는 수집·출력은 다시 `0`으로 끈다.

## 3. 넉백 측정 조건

### 설정할 곳과 적용 조건

- **공격자**의 실제 공격에 대응하는 `DamageSpecContainer` 항목을 확인한다. 키는 WeaponType / ActionType / ActionIndex다.
- 해당 `FDamageSpec.Knockback.Speed`, `Duration`이 유한한 양수여야 한다. 기본값 `0 / 0`은 비활성이며 반복 진단 이벤트도 만들지 않는다.
- 예: `300 cm/s`, `0.2 s`라면 이론상 거리는 `60 cm`다. 이는 측정용 예시이지 현재 모든 공격의 설정값이라는 뜻은 아니다.
- 피격자가 살아 있고 지상에 있어야 하며, 일반 Hit 리액션이 실제 시작되어야 한다.
- Guard, Parry, CollapseHit, 처형, 사망, 공중 피격은 적용 대상이 아니다. 외부 입력 정책·Balance 생명주기·Movement 상태 등 실행 조건도 통과해야 한다.
- **피격자의 Hit 몽타주**에 Root Motion이 있거나 피격자가 애니메이션 Root Motion을 재생 중이면 제외될 수 있다. 공격자의 공격 몽타주에 Root Motion이 있다는 것과 구분한다.

플레이어가 Enemy를 때리는 시험이라면 ‘플레이어 공격 DamageSpec’과 ‘Enemy Hit 리액션/몽타주’를 확인한다. 반대로 플레이어 피격을 시험할 때는 공격자와 피격자의 역할을 바꾼다.

### 권장 측정 순서

1. 평평하고 장애물 없는 곳에서 한 명을 Focus로 지정한다.
2. 진단을 켠 후 이동 입력을 최소화하고 단발 공격을 한다.
3. Enemy 패널에서 `Movement / Started`와 소스 시간, 속도·지속 시간을 확인한다.
4. 종료 후 `Stopped` 사유와 `Nominal / Actual XY`를 확인한다.
5. 단발 기준이 잡힌 뒤 연속 피격, 벽 근처, 경사면, HitStop 조건을 각각 따로 비교한다.

처음부터 콤보로 측정하면 새 Hit가 기존 넉백을 교체해 마지막 결과가 빠르게 바뀐다. 단발로 조건을 분리한 뒤 콤보를 보는 것이 원인 파악에 좋다.

## 4. 넉백 시각 요소

| 요소 | 의미 |
| --- | --- |
| 흰 작은 원 | 실제 넉백이 시작된 위치의 발밑 표시 |
| 노란 점선 | 시작 방향으로 `속도 × 설정 지속 시간`만큼 뻗은 이론상 이동 선 |
| 노란 큰 원 | 이론상 도착 위치. 보장 도착점이 아님 |
| 청록 실선 | 시작 위치에서 현재 위치 또는 종료 위치까지의 실제 변위 |
| 청록 작은 원 | 활성 중에는 현재 위치, 종료 후에는 기록된 종료 위치 |

이론 위치와 실제 위치가 일치하면 노란 큰 원 안에 청록 작은 원이 겹친다. 두 원의 크기를 다르게 한 것은 일치 여부를 보이기 위해서다.

표시 기준은 캡슐 하단에서 약 8cm 위다. 노란 점선은 겹침을 줄이기 위해 여기서 2cm 더 높인다. **바닥 Trace로 경사면에 투영하는 방식은 아니다.** 경사면에서는 선이 지형에 가려질 수 있다.

### 수치 판독

넉백 본문은 상태와 관계없이 10행을 유지한다. 현재 소스가 없으면 `N/A`, 기록이 없으면 `NotCaptured`를 표시하며, 활성·종료 전환 시 아래 항목을 위로 당기지 않는다. 방향은 항상 X/Y를 표시한다.

| 패널 항목 | 판독 기준 |
| --- | --- |
| Current / Owner Gen | 현재 활성 상태와 이동을 소유한 Reaction 실행 세대 |
| Current source / Time | 활성 Root Motion Source의 ID와 진행 시간 / 설정 지속 시간 |
| Current velocity | 이동 소스에 설정된 수평 속도 벡터. 충돌 후 Actor 실측 속도와는 다름. 단위 cm/s |
| Last attempt | 마지막 진단 단계 / 결과 / 사유 / 실행 세대 |
| Last spec | 마지막 시도에서 사용한 설정과 방향 |
| Record age (world) | 마지막 기록 이후 월드 경과 시간 |
| Motion | 마지막 실제 이동의 세대 / Source ID / 상태 |
| Motion stopped time | 해당 이동 정리 시점의 소스 진행 시간. 후속 제외 시도와 독립적으로 유지 |
| Motion distance: Nominal / Actual XY | 이론 거리 / 수평 순변위 |

`Last`와 `Motion Gen`은 다를 수 있다. 새 시도가 제외되어도 이전에 실제 시작한 이동의 공간 기록은 남을 수 있으므로 세대를 함께 확인한다.

`Actual XY`는 시작·끝 Actor 위치 사이의 수평 직선 거리다. 전체 이동 경로 길이나 넉백만의 기여도를 분리한 값은 아니다. 짧게 이동했다고 곧바로 ‘벽 충돌’이라고 판정하지 않는다. 조기 종료·교체·충돌·다른 이동 등의 가능성을 종료 사유와 함께 확인한다.

HitStop·시간 배율이 있으면 `Record age`와 `Source time`이 다를 수 있다. 소스 진행을 판단할 때는 `Source time`을 보고, 화면 유지 시간을 볼 때는 월드 시간을 본다.

## 5. 공격 시작 페이싱 측정 조건

- 실제 사용하는 ComboAttack 액션 데이터에서 `bFaceCombatTargetOnStart`를 켠다.
- `StartFacingMaxDistance`, `StartFacingMaxAngle`이 시험하려는 위치를 허용하는지 확인한다.
- 현재 액션·Executor가 유효하고, 캐릭터가 살아 있으며 Action 상태여야 한다. Movement의 지상 조건도 통과해야 한다.
- 유효한 CombatTarget이 있어야 하고, 보정 대기 중 타깃이 바뀌거나 제거되면 생략·취소될 수 있다.
- 보정은 실제 액션 시작 후 대기했다가 Tick에서 한 번 적용한다. 콤보 각 단계의 실제 시작마다 새 판정이 생기며, 예약만으로 보정하지 않는다.

허용 각도는 ‘이 각도만큼만 돌린다’는 회전량 제한이 아니라 **보정을 허용할지 결정하는 조건**이다. 허용 조건 안이면 타깃 방향으로 정렬한다. 캐릭터 회전 보정이며 카메라 ControlRotation을 함께 돌리는 기능은 아니다.

첫 시험은 적을 정면에서 30~45도 벗어난 곳에 두고 거리·각도 제한 안에서 공격한다. 다음에는 이미 정면인 경우, 거리 초과, 각도 초과, 대기 중 타깃 변경을 비교한다.

## 6. 페이싱 시각 요소

| 요소 | 의미 |
| --- | --- |
| 회색 직선 | 보정 전 캐릭터 방향 |
| 청록 원호와 끝 화살촉 | 보정 전에서 보정 후까지 실제 Yaw 변화 방향·각도 |
| 초록 화살표 | `Applied` 결과의 보정 후 방향 |
| 주황 화살표 | `Applied` 이외 결과의 기록된 방향. 정확한 결과는 패널에서 확인 |
| 노란 작은 원 | 판정 당시 타깃을 향하는 방향 표시 |
| 선택적 주황 경계선·원호 | `DrawLimits`를 켰을 때의 허용 거리·각도 경계 |

**페이싱의 노란 원은 적의 실제 위치가 아니다.** 발밑 기준에서 타깃 방향으로 고정 110cm 떨어진 곳에 놓는 방향 표식이다. 공격 사거리·충돌 범위도 아니다. 결과 화살표 역시 길이 110cm의 표시용 화살표다.

초록 화살표 끝과 노란 원이 겹치면 보정 후 방향과 판정 당시 타깃 방향이 일치한다. 의도된 표현이다. 넉백의 노란 원은 이론 도착점이므로 두 기능의 노란 원을 혼동하지 않는다.

실제 Yaw 변화가 1도 이하이면 회색 선과 청록 원호를 생략한다. `Applied`, `Delta: 0`이면서 초록 화살표와 노란 원만 보이면 이미 적을 향하고 있던 정상적인 경우일 수 있다.

발밑 표시 높이는 캡슐 하단 약 12cm다. 보정 당시 위치·Yaw·타깃 위치를 보관하므로 이후 캐릭터나 적이 움직여도 과거 판정 도형이 따라가지 않는다. 유지되는 도형은 연속 보정의 증거가 아니다.

### 패널 판독

- `Current action / Gen`: 현재 액션 Key와 실행 세대.
- `Option / Pending`: 현재 액션 설정과 일회 보정 대기 여부.
- `Last`: 마지막 적용·생략·취소 등의 결과와 이유. 현재 상태와 별개다.
- `Target (recorded)`: 해당 기록에서 사용한 타깃.
- `Distance / Angle`: 판정 거리·각도와 각각의 허용 값.
- `Yaw (recorded) / Delta`: 보정 전후 Yaw와 최단 방향의 부호 있는 변화량.
- `Age (world)`: 기록 이후 월드 시간. 액션 진행률이 아니다.

## 7. 화면·수집·출력 제어

| 설정 | 역할 |
| --- | --- |
| `HUDVisible` | 전체 화면 표시. 꺼지면 이 HUD에서 호출하는 월드 드로우도 보이지 않음 |
| `Knockback.Enabled`, `AttackFacing.Enabled` | 해당 기능의 최신 진단 기록·이벤트 연결 활성화 |
| 각 기능의 `DrawWorld` | 월드 도형 표시 |
| Player/Enemy별 섹션 `Enabled` | 해당 패널만 표시·숨김. 월드 드로우와 독립 |
| `CaptureEnabled` | 이벤트 이력 수집. 최신 상태 표시와 독립 |
| `EventLogFilter`, `EventLogScope` | 이벤트 로그의 종류·대상 필터 |
| `Portfolio.Debug.*Audit` | Output Log 출력. 화면 표시와 독립 |

가독성 조정 예시:

```text
Portfolio.DebugOverlay.Knockback.DrawDuration 4
Portfolio.DebugOverlay.AttackFacing.DrawDuration 4
Portfolio.DebugOverlay.Knockback.DrawForeground 1
Portfolio.DebugOverlay.AttackFacing.DrawForeground 1
Portfolio.DebugOverlay.AttackFacing.DrawLimits 1
```

- `DrawDuration`: 기본 2초, 0~10초. 넉백은 활성 중 계속 표시하고 종료 기록을 유지한다. 페이싱은 최근 기록 이후 유지한다. 새 기록이 생기면 교체된다.
- `DrawForeground`: 기본 0. 1이면 가림을 줄이지만 벽 너머까지 보일 수 있다. 가시성 문제 확인용이며 실제 시야 판정이 아니다.
- `DrawLimits`: 기본 0. 범위 조건을 볼 때만 켠다. 큰 거리 설정에서는 화면을 많이 차지한다.
- 패널이 화면 아래로 넘치면 기존의 불필요한 상세 섹션을 숨긴다.

## 8. 안 보이거나 예상과 다를 때

| 증상 | 확인 순서 |
| --- | --- |
| 둘 다 안 보임 | Non-Shipping PIE → HUDVisible → 기능 Enabled → DrawWorld → 새 동작 수행 |
| Enemy만 안 보임 | 실제 피격자와 디버그 Focus Enemy가 같은지 확인 |
| Current Active인데 도형 없음 | 적용 전에 진단을 켰는지 확인. 뒤늦게 켜도 과거 시작 좌표를 새로 만들지 않음 |
| 넉백 Last가 NotCaptured | 실제 공격 키, Speed/Duration, 진단 활성화 시점 확인. 0/0 설정은 기록 생략 |
| Damage Resolved만 있음 | 설정 해석 성공이지 이동 시작 성공이 아님. Reaction/Movement 후속 결과 확인 |
| MontageRootMotion / AnimationRootMotion | 피격자 Hit 몽타주·현재 Root Motion 확인 |
| 선이 몸·바닥에 가려짐 | 시점 변경 → 평지에서 비교 → DrawForeground를 잠시 1로 설정 |
| 도형이 금방 사라짐 | DrawDuration 조정. 콤보에서는 새 기록에 의해 교체되는지도 확인 |
| 페이싱 회색 선·원호 없음 | Delta가 1도 이하인지 확인. 이미 정렬된 경우 정상 |
| 페이싱 주황 화살표 | Last의 사유 확인. 색만으로 거리 초과·취소 등을 단정하지 않음 |
| 로그만 없음 | CaptureEnabled와 Filter/Scope 확인. Output Log는 별도 Audit 확인 |
| Last와 현재 이동이 달라 보임 | Current / Last / Motion Gen 및 기록 나이를 구분 |

## 9. 비교 시험 체크리스트

| 시험 | 확인할 결과 |
| --- | --- |
| 평지 단발 Hit | Movement Started, 실제 변위 증가, 종료 사유와 소스 시간 확인 |
| 연속 Hit | 새 실행 세대·소스로 교체. 오래된 정리가 새 이동을 지우지 않음 |
| Guard / Parry / 공중 / Root Motion | 넉백 적용 제외. 가능한 기록의 단계·사유로 확인 |
| 완료·중단·사망·처형 전환 | 활성 소스가 남지 않음. 마지막 종료 사유 확인 |
| 벽 근처·경사면 | Nominal과 Actual XY 비교. 작은 변위만으로 충돌 원인 확정 금지 |
| HitStop·시간 배율 | 소스 진행 시간과 월드 기록 나이를 분리해 판독 |
| 30~45도 방향 보정 | 보정 전 선, 실제 회전 원호, 결과 방향과 타깃 방향 일치 |
| 이미 정면인 공격 | Delta 약 0, 불필요한 이전 방향 선·원호 생략 |
| 거리·각도 초과 | Last에 생략 사유, 무조건 타깃으로 정렬하지 않음 |
| 콤보 단계 전환 | 각 실제 시작의 Key·Gen·판정 기록 확인 |
| 대기 중 타깃 변경·액션 종료 | 잘못된 타깃에 뒤늦게 보정하지 않음 |
| 표시·수집 OFF 비교 | 게임플레이 결과는 유지. 표시와 수집 제어만 달라짐 |

위 표는 시험 절차이며 모든 항목을 현재 수동 검증 완료했다는 뜻은 아니다. 문제가 발생하면 공격 Key, 양쪽 Actor, 실행 세대, 설정값, Last 결과·사유와 함께 화면을 남긴다. 넉백은 Nominal / Actual XY / Source time, 페이싱은 Distance / Angle / Yaw Delta를 함께 기록하면 재현하기 쉽다.

## 10. 구현 근거와 확인 상태

- `FCombatKnockbackDebug`: 넉백 관측 기록, 패널 수치, 월드 드로우.
- `FActionFacingDebug`: 페이싱 판정 기록, 패널 수치, 월드 드로우.
- `FCombatMotionDebugDraw`: 발밑 기준, 원·점선·회전 원호 공통 표현.
- 기존 SnapshotStore / ViewDataBuilder / TextFormatter / SettingsRegistry / DebugOverlayHUD를 활용한다. 진단 데이터로 게임플레이를 판정하지 않는다.
- 사용자 PIE 피드백으로 발밑 넉백·페이싱 표현의 개선을 확인했다(2026-09-21). 경사면·모든 실패 분기의 검증 완료를 뜻하지는 않는다.
- 기존 35개 자동 테스트 통과 이력은 발밑 표시 보완 이전 기준이다. 2026-09-21 고정 행 보완 후 Editor Development 빌드 및 `Portfolio.DebugOverlay.CombatMotion` 8개 테스트가 통과했다(`DrawGeometry`, 행 안정성 검증 포함). 로그: `Saved/Logs/CombatMotionStableRows.log`. 화면 없는 실행이므로 고정 행의 실제 PIE 가독성 확인은 별도다.
