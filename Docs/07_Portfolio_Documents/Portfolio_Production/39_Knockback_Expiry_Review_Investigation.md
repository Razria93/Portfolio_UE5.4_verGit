# 넉백 자연 만료 리뷰 조사

2026-09-21, PR #124의 `MaintainLastRootMotionVelocity` 리뷰 대응을 위한 검토 기록. 런타임 수정·원격 답변·Resolve·커밋·푸시는 하지 않았다.

## 결론

현재 프로젝트의 선행 Tick 순서를 재현한 조건에서는 자연 만료 후 잔여 이동이 재현되지 않았다. CharacterMovement를 인위적으로 먼저 한 번 실행하면 리뷰가 지적한 소스 부재·잔여 속도 경로가 재현된다. 따라서 조건부 위험은 확인했으나 현재 정상 실행에서 항상 발생하는 버그로 확정하지 않는다.

## 코드 근거

- Movement 초기화는 CharacterMovement에 자기 Tick prerequisite를 등록한다.
- Movement Tick의 UpdateKnockback은 Finished 소스를 ClearKnockback으로 정리한다.
- UE 5.4 CharacterMovement의 PerformMovement는 CleanUpInvalidRootMotion을 호출한 뒤 힘·Launch 및 이동을 처리한다.
- 소스가 엔진에 남아 있으면 프로젝트가 먼저 XY 정리를 수행할 수 있다. 소스가 이미 제거되었으면 현재 ClearKnockback의 ID/참조 비교 조건을 통과하지 못한다.
- 프로젝트 소스 검색에서 일반 런타임의 Movement Tick 비활성화·간격 변경 또는 CharacterMovement의 별도 직접 PerformMovement 호출 경로는 확인되지 않았다. BP·네트워크·모든 외부 호출을 전수 배제한 것은 아니다.

## 최초 조사 재현 구성

`Portfolio.Combat.Knockback.ExpiryReview`를 테스트 파일에 추가했다. 일반 순서를 수동 호출하는 기존 Fixture를 사용하므로 엔진 Tick scheduler 자체를 검증한 테스트는 아니다.

- 실제 CharacterMovement, PlayerController가 소유한 플레이어, 충돌 가능한 바닥.
- 속도 300cm/s, 지속 시간 0.213초(프레임 간격과 어긋나는 값).
- 30/60/144Hz × 기본 제동/마찰 또는 제동/마찰 0.
- 평지, 30도 회전 벽, 15도 경사 바닥, 더 높은 우선순위의 별도 Override 소스, 엔진 선행 Tick 대조군: 총 30개 조합.
- 매 Tick에서 Finished 상태를 확인한 후 프로젝트 정리 전후 속도, 소스 존재 여부, 다음 이동 및 후속 5 Tick의 순변위를 기록했다.
- 벽 시험의 방향 변경 변위와 경사 시험의 Z 변위로 환경 접촉을 확인했다.
- 기존 Ownership / ExternalLaunch 등 넉백 테스트 6개도 함께 실행했다.

## 결과

| 조건 | 관측 결과 |
| --- | --- |
| 정상 순서·평지 | 6개 조합 모두 정리 시 소스 존재, XY 0, 만료 후 관측 변위 0 |
| 정상 순서·비스듬한 벽 | 6개 조합 모두 정리 후 변위 0. 벽에 의해 X/Y 이동 경로가 바뀌었지만 정리 시 Velocity XY는 소스와 일치 |
| 정상 순서·15도 경사 | 6개 조합 모두 정리 후 변위 0. 약 17~19cm Z 변화로 경사 이동 확인 |
| 높은 우선순위의 별도 소스 | 별도 Y=40cm/s 이동 유지. 넉백 종료 후 이동을 잔여 넉백으로 분류하면 안 됨 |
| 엔진 선행 Tick 대조군 | 소스 부재 및 잔여 속도 재현. 기본 제동에서는 감속, 무제동에서는 지속 이동 |
| 기존 외부 Launch / 소유권 등 | 6개 모두 성공 |

60Hz 대조군은 프로젝트 정리 전에 CharacterMovement Tick을 추가로 호출한다. 그 결과 기본 제동에서는 정리 직후 X 속도 약 185.87cm/s, 관측 구간 추가 변위 약 5.48cm였다. 무제동에서는 X=300cm/s가 유지되고 약 35cm 이동했다. 이 변위 구간은 추가 엔진 Tick을 포함한 7개 이동 Tick이므로 정상 조건의 6개 Tick과 같은 시간 구간으로 직접 비교하지 않는다. 로그의 `firstDrift`도 대조군에서는 추가 Tick을 포함한다.

## 별도 발견: 마지막 프레임의 시간 초과

정상 평지에서 이론 거리는 63.9cm지만 실제 만료 경계 변위는 30Hz 70cm, 60Hz 65cm, 144Hz 약 64.58cm였다. 이후 잔여 변위는 0이다.

UE 5.4 `FRootMotionSource_ConstantForce` 생성자는 `DisablePartialEndTick`을 기본으로 설정한다. 이 시험의 시간 초과는 마지막 이동 프레임 처리와 부합하며, 만료 후 마찰로 미끄러지는 문제와 구분한다. 정확한 Duration 거리 보장이 필요한지는 별도 정책 판단이지 이번 리뷰를 근거로 조용히 바꿀 사항이 아니다.

## 최초 조사 테스트 해석 및 한계

Editor Development 빌드 성공. 넉백 자동 테스트 7개 성공, 종료 코드 0. 로그: `Saved/Logs/KnockbackExpiryReviewFinal.log`.

신규 테스트는 재현 조사용이다. 성공은 Fixture·시작·만료 경로가 성립했다는 뜻이며, 일부러 만든 엔진 선행 대조군의 잔여 이동까지 버그가 없다고 판정한 것이 아니다. 수치 로그를 기준으로 위 결론을 도출했다. 초기 직접 CleanUpInvalidRootMotion 호출은 엔진 전체 이동 순서와 다르므로 최종 근거에서 제외하고 전체 CharacterMovement Tick으로 재검증했다.

수동 순서의 격리 월드이며 실제 PIE Tick scheduler, 네트워크 보정, 이동 플랫폼, 모든 경사·충돌 조합을 검증하지 않았다. 벽/경사만으로 속도 일치 검사가 실패한다는 가설은 이번 조건에서 재현되지 않았다.

## 대응 권장

1. 리뷰에 현재 prerequisite와 정상 순서 18개 단독 넉백 시험의 만료 후 변위 0을 근거로 설명한다.
2. 엔진 선제 제거 상황의 위험은 인정하되, 현재 정상 실행의 재현 조건이 확보되지 않았음을 명시한다.
3. 종료 모드를 즉시 SetVelocity/ClampVelocity로 바꾸지 않는다. 엔진 SetVelocity는 전체 벡터, ClampVelocity는 양수 Z도 변경하므로 외부 Launch·다른 소스 보존을 별도로 고려해야 한다.
4. 정상 순서의 만료 직후 검증을 명시적인 회귀 assertion으로 정리하는 것을 우선 권장한다. 순서 독립적인 종료 보장을 추가하려면 별도 구현 범위를 합의한다.

## 후속 검증: 회귀 assertion 및 World Tick

위 4번의 회귀 검증과 실제 스케줄러 확인을 수행했다. 게임플레이 코드는 변경하지 않았다.

| 테스트 | 검증 내용 | 결과 |
| --- | --- | --- |
| `Portfolio.Combat.Knockback.ExpiryBoundary` | 평지·벽·경사·별도 Override 소스 × 30/60/144Hz × 기본/무제동, 24개 조합. 단독 넉백은 만료 정리 후 XY 속도·변위 0, 다른 소스는 Y=40cm/s 유지 | 통과 |
| `Portfolio.Combat.Knockback.ExpiryCounterfactual` | prerequisite를 의도적으로 우회하여 CharacterMovement를 먼저 호출하는 60Hz 무제동 대조군. 소스 선제 제거와 잔여 속도 발생을 명시적으로 검증 | 통과: 정상 경로의 무결성을 의미하지 않는 대조군 |
| `Portfolio.Combat.Knockback.WorldTickExpiry` | prerequisite 등록 확인 후 `UWorld::Tick`으로 실제 TickTaskManager 실행. 30/60/144Hz 무제동에서 만료 후 6프레임 동안 XY 속도·변위 0 및 소스 정리 확인 | 통과 |

World Tick 시험의 총 변위는 각각 70.000004 / 65.000003 / 64.583334cm이며, 만료 후 잔여 변위는 모두 0.000000cm다. 마지막 프레임의 시간 초과와 만료 후 잔여 이동은 계속 구분한다.

테스트용 Character는 무기 에셋에 의존하는 Player 초기화만 우회하며 실제 Movement와 CharacterMovement를 사용한다. 동기식 테스트에서 여러 월드 프레임을 진행하기 위해 `GFrameCounter`를 증가시킨다. 이를 생략하면 TickTaskManager가 동일 프레임으로 판단하여 반복 Tick을 수행하지 않으므로, 실제 이동과 만료 도달도 assertion으로 확인한다.

Editor Development 빌드 성공. 전체 `Portfolio.` 자동 테스트 **39개 모두 성공**(넉백 9개 포함). 로그: `Saved/Logs/KnockbackWorldTickFinal2.log`.

이 검증은 격리된 Game World에서 실제 스케줄러를 사용한 테스트다. 프로젝트 맵·BP 에셋을 사용한 PIE, 시각화, 네트워크 보정, 이동 플랫폼 및 모든 충돌 조합의 검증을 대신하지 않는다. 현재 정상 실행 순서에서는 리뷰의 잔여 미끄러짐이 재현되지 않았다는 결론을 보강하며, 순서 독립적인 종료 보장을 구현한 것은 아니다. 커밋·푸시·리뷰 답변·Resolve는 수행하지 않았다.
