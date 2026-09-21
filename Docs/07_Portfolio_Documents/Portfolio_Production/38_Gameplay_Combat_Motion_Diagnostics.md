# 넉백·공격 시작 페이싱 진단

## 범위와 원칙

`feat/combat-knockback`의 기존 게임플레이 동작을 관측하는 non-shipping 진단이다. 이동·회전 정책, 공격 설정값, 카메라, 에셋은 변경하지 않는다. 기존 Debug Overlay / SnapshotStore / Editor 설정 화면을 확장한다. Enemy AI Focus용 `Facing`과 공격 시작 보정용 `AttackFacing`은 별개다.

- 현재 상태는 컴포넌트에서 읽고, 마지막 처리 결과는 진단 저장소에서 읽는다.
- 실제 분기에서 사유를 확보한다. 표시를 위해 실행 조건을 재평가하거나 이동 API를 호출하지 않는다.
- 공격 페이싱은 다음 Action Tick에서 한 번 적용된다. 화살표 유지 시간은 연속 보정 시간이 아니다.
- 진단 설정은 기본 꺼짐. Shipping에서 수집·저장·출력 본문과 공격자 진단 참조를 제외한다.

## 사용 방법

기존 `ACDebugOverlayHUD`가 연결된 플레이 세션에서 Editor Debug Overlay 설정 화면을 사용하거나 다음 콘솔 명령을 실행한다.

```text
Portfolio.DebugOverlay.HUDVisible 1
Portfolio.DebugOverlay.Knockback.Enabled 1
Portfolio.DebugOverlay.AttackFacing.Enabled 1
Portfolio.DebugOverlay.CaptureEnabled 1
Portfolio.DebugOverlay.EventLogFilter Knockback
```

페이싱 이력은 마지막 명령의 값을 `AttackFacing`, 두 종류 모두는 `All`로 바꾼다. Focus Enemy 선택은 기존 도구를 사용한다. `FocusedEnemy` 범위는 기록의 Owner/Source/Target 관계로 필터링한다.

| 설정 | 의미 / 기본값 |
|---|---|
| `Portfolio.DebugOverlay.HUDVisible` | 패널과 월드 드로우 표시 / 0 |
| `Portfolio.DebugOverlay.CaptureEnabled` | 이후 이벤트 수집 / 0 |
| `Portfolio.DebugOverlay.Knockback.Enabled` | 넉백 최신 결과 관측 및 이벤트 생성 / 0 |
| `Portfolio.DebugOverlay.AttackFacing.Enabled` | 공격 페이싱 최신 결과 관측 및 이벤트 생성 / 0 |
| `Portfolio.DebugOverlay.Knockback.DrawWorld` | 넉백 공간 표시 / 1 |
| `Portfolio.DebugOverlay.AttackFacing.DrawWorld` | 기록된 방향 표시 / 1 |
| `Portfolio.DebugOverlay.AttackFacing.DrawLimits` | 보정 전 방향 기준 허용 각도·거리 / 0 |
| `Portfolio.DebugOverlay.Knockback.DrawDuration` / `Portfolio.DebugOverlay.AttackFacing.DrawDuration` | 종료 넉백/페이싱 기록의 표시 유지 시간(월드 초, 0~10) / 2 |
| `Portfolio.DebugOverlay.Knockback.DrawForeground` / `Portfolio.DebugOverlay.AttackFacing.DrawForeground` | 지형·캐릭터 가림을 무시하는 전경 표시 / 0 |
| `Portfolio.DebugOverlay.Player.Knockback.Enabled` / `Enemy.Knockback.Enabled` | 해당 Actor 패널 섹션 / 1 |
| `Portfolio.DebugOverlay.Player.AttackFacing.Enabled` / `Enemy.AttackFacing.Enabled` | 해당 Actor 패널 섹션 / 1 |
| `Portfolio.Debug.KnockbackAudit` / `Portfolio.Debug.AttackFacingAudit` | Output Log 출력 / 0 |

표시·수집·Audit은 독립적이다. HUD를 숨겨도 도메인과 Capture가 켜져 있으면 이벤트가 수집된다. Capture를 꺼도 도메인이 켜져 있으면 최신 관측 결과가 갱신된다. Player/Enemy 부모 및 섹션 토글은 패널에만 영향을 주며 월드 드로우·수집을 끄지 않는다.

### 화면 공간

기존 메인 패널은 모든 상세 항목을 켜면 화면 아래로 넘칠 수 있다. 이번 작업에서는 전체 패널 렌더러를 변경하지 않았다. Editor 설정에서 불필요한 Locomotion, Targeting, Execution Session, Recent Action/Reaction, AI, Balance 등의 상세 섹션을 끄고 Status + Knockback + Attack Facing 위주로 확인한다. Player와 Enemy를 각각 확인하는 것도 가능하다.

## 표시 의미

### Knockback

- `Current`: 실제 소스 활성 여부. `Unavailable`은 Movement 컴포넌트 부재.
- `ReactionGeneration`: 소스를 소유하는 Reaction 세대. Movement의 내부 재진입 세대와 다르다.
- `Source time`: Root Motion Source의 실제 진행 시간. HitStop을 무시한 월드 경과 시간으로 계산하지 않는다.
- `Last`: Damage / Reaction / Movement 단계와 결과·사유. `NotCaptured`는 기록 없음이며 비활성과 다르다.
- `Last spec`: 마지막 시도에 사용한 속도·시간·방향. 현재 소스 파라미터와 혼동하지 않는다.
- `Record age (world)`: 마지막 관측 후 월드 시간. 소스 진행률이 아니다.
- `Stopped source time`: 종료 시점의 소스 진행 시간.

월드의 흰 원은 시작 위치, 노란 점선과 큰 끝점 원은 `속도 × 시간`의 이론상 이동, 청록 실선과 작은 끝점 원은 실제 변위다. 표시 좌표는 스케일 적용 캡슐 하단 + 8cm이며, 메시의 발 애니메이션은 따라가지 않는다. 예상 점선은 실제 선보다 2cm 위에 그린다. 패널의 `Nominal / Actual XY`는 마지막 이동 세대의 이론 거리와 시작점 기준 수평 변위를 나타내며, 전체 경로 길이나 순수 넉백 기여량은 아니다.

충돌이나 조기 종료로 차이가 날 수 있으며, 차이만으로 벽 충돌이라고 판정하지 않는다. 활성 중에는 현재 발밑 위치를 읽고 종료 시 끝점을 고정한다. 종료 기록은 기본 2초간 그리며 DrawDuration으로 조절한다. 시작을 관측하지 못했다면 시작점을 추정해서 그리지 않는다. 긴 월드 텍스트는 제거했다. 바닥 투영은 하지 않으므로 경사면에서 선이 묻힐 수 있다. 필요하면 DrawForeground를 켜되, 이 모드는 벽 뒤의 도형도 보인다는 점을 구분한다.

### Attack Facing

현재 액션 Key/세대, 옵션, 대기 상태와 마지막 판정의 액션·타깃·거리·각도·Yaw를 분리한다. `Disabled/OptionOff`, `Queued`, `Applied`, `Skipped`, `Cancelled`를 구분한다. 대표 생략 사유는 `TargetChanged`, `TargetDead`, `OutOfRange`, `OutOfAngle`, `NotGrounded`다.

발밑(캡슐 하단 + 12cm)에서 회색 짧은 선은 보정 전 방향, 초록 화살표는 성공 후 방향, 주황 화살표는 미적용 결과 방향이다. 노란 작은 원은 고정 반경 110cm의 타깃 방향 표식이며 타깃까지의 거리나 공격 사거리를 의미하지 않는다. 전후 회전이 1도 이하이면 회색 선과 회전 호를 생략한다. 1도보다 크면 청록 회전 호로 실제 회전 방향을 표시한다. 일치하는 타깃 방향은 결과 화살표 끝의 노란 원으로 표현한다.

허용 경계는 보정 전 위치·Yaw 기준이다. 기본 2초간 당시 좌표를 보여주므로 타깃이 움직여도 이전 판정 도형은 따라가지 않는다. 유지 시간과 전경 표시는 각 CVar로 제어한다. 긴 월드 텍스트 대신 패널의 Delta 각도를 확인한다.

## 파일별 변경 이유

경로의 런타임 소스 기준은 `Source/Portfolio/`이다.

| 파일 | 변경 / 이유 |
|---|---|
| `Type/CCombatKnockbackTypes.h` | non-shipping 약한 `DebugSourceActor` 추가. 연속 피격에서 직전 이벤트의 공격자를 잘못 상속하지 않도록 실제 Context와 함께 전달 |
| `Component/CCombatSignalTargetComponent.cpp` | 실제 Damage 분기에 제외/해석 결과 기록. 기본 0 설정은 헬퍼에서 기록 억제 |
| `Component/CReactionComponent.h/.cpp` | 실행 시작 실패·시작 중 완료·몽타주 Root Motion 제외 관측, 완료/중단 사유를 기존 정리 시점에 전달 |
| `Component/CMovementComponent.h/.cpp` | 동일 조건 평가에서 선택적 사유 반환, 등록/교체/종료 관측. 이동 소유권·소스 설정·정리 순서는 유지 |
| `Component/CActionComponent.h/.cpp` | 대기 등록/소비/취소 구분, 실제 페이싱 결과 기록. 진단 헬퍼의 읽기 전용 접근 허용 |
| `Core/Debug/FCombatMotionDebugTypes.h` | 관측 레코드 및 넉백 Last/Motion 분리. 새 실패가 이전 실제 이동의 시작점을 훼손하지 않음 |
| `Core/Debug/FCombatMotionDebugDraw.h` | 캡슐 하단 표시 좌표, 수평 원, 점선, 회전 호의 작은 공통 시각화 함수 |
| `Core/Debug/FCombatKnockbackDebug.h/.cpp` | 넉백 gate, 기록, 화면 문자열, 월드 드로우, Audit 소유 |
| `Core/Debug/FActionFacingDebug.h/.cpp` | 동일 책임 분리로 공격 페이싱 진단 제공 |
| `Core/Debug/FDebugOverlaySnapshotStore.h/.cpp` | 최신 진단 API 및 Actor 제거 시 정리 |
| `Core/Debug/FDebugOverlaySnapshotStoreInternals.h` | 기존 월드 저장소에 약한 Actor 키의 도메인별 저장소 추가 |
| `Core/Debug/FDebugOverlaySnapshotStoreCombatMotion.cpp` | 도메인별 64 Actor 제한, 무효 Actor 제거, 세대 역행 방지, 조회. 기존 월드 Cleanup/Reset 재사용 |
| `Core/Debug/CDebugOverlayHUD.cpp` | 플레이어·Focus Enemy 월드 드로우 연결 |
| `Core/Debug/FDebugOverlayViewDataTypes.h` | 두 도메인의 패널 데이터와 포함 여부 |
| `Core/Debug/FDebugOverlayViewDataBuilder.cpp` | Actor별 표시 설정에 따라 진단 데이터 구성 |
| `Core/Debug/FDebugOverlayTextFormatter.cpp` | Knockback / Attack Facing 섹션 출력 |
| `Core/Debug/FDebugOverlayViewDataBuilder.h`, `FDebugOverlayTextFormatter.h` | Editor 테스트 모듈이 실제 표시 파이프라인을 호출할 수 있도록 클래스 export 추가 |
| `Core/Debug/FDebugOverlayDisplayConfig.h/.cpp` | 플레이어·Enemy별 섹션 표시 설정 |
| `Core/Debug/FDebugOverlayEventCategory.h` | 두 이벤트 카테고리 추가 |
| `Core/Debug/FDebugOverlaySnapshotStoreFilterPolicy.cpp` | 새 카테고리 정규화·필터 인식 |
| `Core/Debug/FDebugOverlaySettingsRegistry.cpp` | Editor 자동 생성 설정 화면과 필터 목록에 등록. 플러그인 별도 UI 수정 불필요 |
| `Source/PortfolioEditor/Private/Tests/CCombatMotionDebugTests.cpp` | 수집 gate, 세대, 용량, 정리, 필터, 소스 소유권 검증 |
| `Source/PortfolioEditor/Private/Tests/CActionFacingTests.cpp` | 실제 페이싱 실행의 대기·적용·취소·거절·기록 고정 검증 |
| `Source/PortfolioEditor/Private/Tests/CCombatKnockbackTests.cpp` | 실제 Reaction 경계의 진단 사유 검증 |
| `Source/PortfolioEditor/Private/Tests/CCombatMotionPresentationTests.cpp` | 실제 Builder → Formatter 경로의 양쪽 Actor 표시·NoActor·개별/부모 숨김·도메인 gate 검증 |

오래된 Stop 요청은 실제 이동뿐 아니라 마지막 진단도 변경하지 않는다. 넉백 기록 세대의 상한을 별도로 보존하므로, 중간의 Damage 단계(세대 0) 기록이 오래된 실행 결과를 다시 허용하지 않는다. 매 Tick의 대기 소비·빈 정리는 이벤트가 아니다.

## 검증

2026-09-21 실행 결과:

- `PortfolioEditor Win64 Development` 빌드 성공.
- `Portfolio Win64 Shipping` 빌드 성공. 패키징·Cook·Shipping 게임 실행 검증은 수행하지 않았다.
- 렌더링 활성화 `UnrealEditor-Cmd -RenderOffscreen`에서 `Automation RunTests Portfolio.`: **35 Success, 0 Fail, 종료 코드 0**.
- 최종 로그: `Saved/Logs/CombatMotionDebugFinalRHI.log` (로컬 생성물, Git 추적 제외).
- 추가 진단 테스트 7개: `Portfolio.DebugOverlay.CombatMotion.Gates`, `History`, `Registry`, `Ownership`, `FacingLifecycle`, `ReactionLifecycle`, `Presentation`.
- 기존 실제 CharacterMovement / Root Motion / 콤보 동작 테스트도 함께 통과했다. 비활성 기본 설정의 회귀와 진단을 켠 실행 경로를 모두 검사했다.
- 최초 NullRHI 실행에서는 HUD RenderPreview가 생략되었지만, 최종 RHI 실행은 NullRHI가 아니다.
- 드로우 함수 호출 smoke 및 표시 데이터 생성은 검사했지만, **새 월드 드로우와 오버레이의 실제 플레이 화면 캡처·시각 검수는 미수행**이다. 아래 PIE 절차가 필요하다.
- 자동 검사에서 Actor 데이터 명시 제거, 월드 저장소 Reset, 용량 제한을 확인했다. 실제 PIE 종료→재시작과 Actor EndPlay 순서는 수동 확인 항목으로 남긴다.
- 기존 Regacy 머티리얼 참조 경고 및 Editor 설정 화면의 반복 CVar 조회 성능 경고가 로그에 있다. 이번 범위에서 관련 에셋이나 플러그인을 수정하지 않았다.
- `git diff --check` 통과. 에셋·Config 변경 없음. 커밋·푸시·PR 없음.

## 사용자 PIE 확인 절차

### 발밑 표시 보완 검증 상태

위 35개 통과 기록은 발밑 표시 보완 이전 결과다. 2026-09-21 사용자 PIE 피드백으로 두 표현의 가독성 개선을 확인했다. 이후 넉백 패널을 10개 고정 행으로 보완했으며, Editor Development 빌드와 `Portfolio.DebugOverlay.CombatMotion` 8개 테스트가 통과했다(`DrawGeometry`, 활성·종료·기록 없음 및 후속 제외의 행 안정성 검증 포함). 로그는 `Saved/Logs/CombatMotionStableRows.log`다. 화면 없는 검증이므로 고정 행의 PIE 가독성, 경사면 가림 및 모든 실패 분기의 수동 확인 완료를 뜻하지는 않는다. 실제 측정 조건과 도형 판독은 [측정 가이드](../Debug_Overlay/02_Operation/Combat_Motion_Debug_Measurement_Guide_KR.md)를 참고한다.

1. 위 설정을 켜고 기존 도구에서 Enemy를 Focus로 선택한다. 관련 상세 섹션만 표시한다.
2. 넉백 Speed/Duration이 설정된 일반 Hit 공격을 지상에서 적중시킨다. Damage → Movement Started, 실제 소스 ID와 청록 변위를 확인한다.
3. 연속으로 적중시킨다. 새 Reaction 세대로 교체되고 새 공격자와 연결되어야 한다. 종료 후 캐릭터를 이동해도 이전 종료 끝점은 이동하면 안 된다.
4. 벽·경사면·다른 캐릭터 근처에서 확인한다. 노랑 이론 거리와 청록 실제 변위의 차이는 가능하다. 근거 없는 WallBlocked 표시가 없어야 한다.
5. HitStop을 발생시켜 소스 진행 시간과 월드 기록 경과가 서로 다른 의미로 표시되는지 확인한다.
6. ComboAttack을 0~3 단계 진행한다. 각 시작마다 Queued → Applied와 세대가 바뀌고 카메라 회전은 진단 때문에 바뀌면 안 된다.
7. 보정 거리·각도 밖에서 공격하고, 대기 중 타깃 변경/취소도 확인한다. 해당 사유와 주황 결과 방향을 확인한다.
8. HUD만 끄고 공격한 후 다시 켠다. Capture가 켜져 있었다면 이력은 남아야 한다. 반대로 Capture만 끄면 최신 상태는 갱신되지만 새 이벤트는 늘지 않아야 한다.
9. Player/Enemy 패널 섹션만 끈다. 월드 드로우는 유지돼야 한다. DrawWorld를 끄면 해당 도형만 사라져야 한다.
10. PIE를 종료·재시작한다. 이전 세션의 최신 결과가 새 세션에 나타나면 안 된다.

에셋 값 변경은 사용자 테스트용이며 이 구현에서 에셋을 자동 수정하지 않는다.
