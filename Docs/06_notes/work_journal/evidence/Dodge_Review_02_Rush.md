# Review 02: 카운터 러시 거리와 디버그

## 구현과 의도

- `Source/Portfolio/Type/CCounterRushTypes.h/.cpp`: 설정(`FCounterRushTuning`)과 순수 계산(`FCounterRushPlan`)을 분리했다. 목표 중심 간 거리는 플레이어의 scaled capsule radius + 대상의 scaled capsule radius + ExtraGap이다. 캡슐이 없는 대상의 반경은 0으로 취급한다.
- 이동은 XY 평면에서 계산하며 시작 높이를 유지한다. 이미 충분히 가까우면 이동하지 않고, 부족한 거리는 MaxTravelDistance로 제한한다. 이 제한은 계획된 돌진 거리이며 외부 이동이나 벽 슬라이딩까지 포함하는 절대 경로 길이 제한은 아니다.
- `Source/Portfolio/Component/CActionComponent.h`: 캐릭터 BP의 ActionComponent에서 `Action | CounterRush`의 Duration(초), MaxTravelDistance(cm), ExtraGap(cm)을 조절할 수 있다. 기본값은 0.25초 / 450cm / 30cm이다.
- `Source/Portfolio/Action/CAction_CounterRush.cpp`: Start에서 설정과 대상 위치를 스냅샷으로 고정한다. CommitStarted에서 기존 CharacterMovement Root Motion Source로 이동한다. 지속 추적, 순간이동, 추가 무적은 넣지 않았다. 공격 몽타주/충돌 구간과 이동 종료는 계속 별개다.
- `Source/Portfolio/Core/Debug/FCombatCounterRushDebug.h/.cpp`: 넉백 디버그의 발밑 좌표 계산을 재사용하되 러시 실행별 경로를 별도로 수집한다. Shipping에서는 동작하지 않는다. 전용 HUD 패널 연동은 하지 않았다.

## PIE 확인

러시 전에 콘솔에서 활성화한다.

```text
Portfolio.DebugOverlay.CounterRush.Enabled 1
Portfolio.DebugOverlay.CounterRush.DrawDuration 2
Portfolio.Debug.CounterRushAudit 1
```

가림 없이 보려면 `Portfolio.DebugOverlay.CounterRush.DrawForeground 1`을 사용한다. 끄려면 Enabled와 Audit을 0으로 돌린다.

- 은색 선: 거리 제한 전 희망 도착점.
- 청록 화살표와 원: 거리 제한 후 계획 경로와 도착 플레이어 반경.
- 주황 원: 시작 시 대상 캡슐 반경.
- 노란 원: 합의한 목표 중심 간 간격.
- 자홍 선/점: 실제 이동 경로/위치.
- 텍스트 및 로그: 계획 거리, 실제 XY 변위, 누적 XY 경로, 도착점 오차와 종료 사유.

벽이 막으면 계획점까지 도달하지 않는 것이 정상이다. 오차만으로 벽 충돌이라고 단정하지 않는다. 경로 샘플은 128개로 제한하며 초과 시 마지막 샘플을 갱신하므로 긴 실행의 후반 경로는 거칠어질 수 있다.

## 검증과 남은 일

2026-09-22 Development 빌드 성공. `Saved/Logs/DodgeRushSpacing03Tests.log`에서 Portfolio 자동 테스트 **57/57 성공**, Error 없음, exit 0을 확인했다.

- CapsulePlan: 간격 공식, 최대 거리, 높이 보존, 가까운/동일 위치, 0 거리, 잘못된 수치.
- PerfectLifecycle: 실제 실행의 scaled capsule radius 반영과 대상 스냅샷 고정.
- MovementAndWall: 실제 CharacterMovement 이동, 벽 앞 정지, 종료 후 수평 미끄러짐 방지.

NullRHI 검증이므로 디버그 가독성과 실제 연출 품질은 사용자 PIE 검토가 필요하다. 이동 전용 고속 연출과 디졸브 BP 동시 표현 검토는 아직 별도 후속 작업이다. 기존 퍼펙트 닷지 판정은 변경하지 않았다.

## 향후 Blink 참고

Blink는 별도 실행 객체로 만든다. 반경 합 + 여유 거리라는 간격 정책과 입력 검증은 참고할 수 있지만, CounterRush의 앞쪽 도착점 계산을 그대로 사용하지 않는다. 대상 뒤쪽 후보점, 지면/충돌/공간 유효성, 실패 시 처리, 이동 방식은 Blink에서 독립적으로 설계한다. 이 단계에는 Blink 실행을 구현하지 않았다.

## 이동 전용 FOV 연출 (2026-09-22 추가)

`CAction_CounterRush::Tick`은 실제 위치 변화의 전진 방향 성분을 해당 프레임의 계획 이동량으로 나눈 값을 0~1 강도로 전달한다. 이동이 없는 실행에서는 시작하지 않는다. 벽에 막히면 강도가 감소하며 RMS 종료/액션 중단에서는 해제를 요청한다. 공격 충돌이나 무기 발광 종료를 기다리지 않는다.

`CReactionFeedbackComponent`의 `ReactionFeedback | CounterRush`에서 `RushFOVOffset`(기본 5도, 0으로 끔), `RushFOVBlendIn`(0.04초), `RushFOVBlendOut`(0.12초)을 조절한다. 블렌드는 실시간 기준이며 기본 카메라 FOV에 오프셋을 더한다. 로컬 제어 캐릭터의 카메라만 적용한다. 다른 코드가 FOV를 변경하면 소유권을 포기하고, 자신이 마지막으로 기록한 값일 때만 원래 값으로 복원한다. 사망/재초기화/EndPlay에서는 즉시 정리한다.

이 값은 시작 시 고정된 목적지 방향의 실제 변위에 반응하는 근사치다. 같은 방향의 외부 이동도 포함될 수 있으며 네트워크 보정까지 분류하는 시스템은 아니다. 카메라 활성 전환과 다른 BP 카메라 로직의 실제 상호작용은 PIE에서 추가 확인한다.

Development 빌드 성공. `DodgeRushFOV04Tests.log`에서 **58/58 성공**, Error 없음, exit 0. 추가 RushFOV 테스트는 무이동 시 비활성 강도, 중간 보간, 최대 강도, 벽 차단에 해당하는 0 이동 입력, 오래된 실행 ID 무시, 종료/재초기화 복원, 외부 FOV 보존을 검증했다. 이는 카메라 값 검증이며 실제 화면의 속도감 승인은 아니다.

Niagara 이동 리본/파티클과 잔상 검토는 아직 남아 있다. FOV만으로 이동 전용 연출 전체가 완료됐다고 간주하지 않는다.
