# 일반 Hit 지상 넉백 구현

2026-09-21 최신화. 코드 기준은 `b0291c0e`, 테스트는 `361aeeb2`다. 전체 읽기 순서·재현 명령·미검증 경계는 [통합 검토 가이드](37_Gameplay_Knockback_and_Facing_Review_Guide.md)를 참조한다.

## 범위

- 브랜치: `feat/combat-knockback` (최신 main 기준 생성).
- 플레이어와 Enemy의 일반 Hit 실행에 수평 넉백을 연결한다.
- Guard / Parry / CollapseHit / 처형 / 사망 / 공중 피격은 제외한다.
- 넉백은 독립 Reaction이 아니라 성공한 Hit 실행에 소유되는 이동 효과다.
- C++의 Speed / Duration 기본값은 0으로 유지한다. 사용자가 저장한 Stellar 플레이어 BP는 별도 애셋 커밋 `4a1487a3`에 포함했다. 다른 BP와 몽타주의 설정을 일괄 변경하지 않았다.
- 공중 발사, 넘어짐, 벽꿍, 저항, 슈퍼아머, DataAsset 전환은 범위 밖이다.

## 애셋 선행 확인

아래는 구현 전 읽기 전용 커맨드렛과 Python으로 캐릭터 BP CDO의 ReactionDatas 및 참조 애셋을 조사했던 기록이다. 이번 문서 최신화에서 모든 BP·몽타주의 속성을 다시 추출한 결과는 아니다.

| 캐릭터 | Hit 몽타주 | 피격 시퀀스 |
| --- | --- | --- |
| BP_CPlayer | Default/M_HitReact | Default/Lifecycle/HitReact |
| BP_CPlayer_Stellar | Stellar/M_HitReact_Stellar | Stellar/Lifecycle/HitReact_Stellar |
| BP_CEnemy | Default/M_HitReact | Default/Lifecycle/HitReact |

- 실행기: CReaction_Hit, DamageSpec / Hit / INDEX_NONE.
- 두 시퀀스 모두 Enable Root Motion = false, Force Root Lock = false.
- Animation Blueprint CDO는 Root Motion From Montages Only.
- CompleteReaction은 각 몽타주에 1개, 1.1초에 존재한다.
- 레벨 인스턴스 override, 실행 중 변경, 다른 몽타주와의 실제 블렌딩은 별도 PIE 확인 대상이다.
- 당시 로컬 조사 경로: `Saved/Logs/KnockbackRootMotionAudit.log`, `Saved/Audit/KnockbackExecutionMontageAudit`, `Saved/Audit/KnockbackEnemyAudit`. Git 추적 증거가 아니며 현재 체크아웃에 존재함을 보장하지 않는다. 최신화 시 `Saved/Audit`는 존재하지 않았다.

## 데이터 및 책임

| 파일 | 변경 | 이유 |
| --- | --- | --- |
| Type/CCombatKnockbackTypes.h | 설정(Speed, Duration) 및 실행 Context(Direction), 유효성 검사 | 설정과 확정된 실행 정보를 분리 |
| Type/CCombatDamageTypes.h | FDamageSpec.Knockback 추가 | 기존 DamageSpecContainer에서 공격별 조절 |
| Component/CCombatSignalTargetComponent.h/.cpp | ResolveDamageKnockback 추가 | 정상 Hit 및 지상 여부 확인 후 공격자에서 대상 방향을 확정 |
| Type/CCombatSignalTargetTypes.h | 대상 Context에 넉백 정보 추가 | 판정 결과를 이동 없이 전달 |
| Type/CReactionOrchestrationTypes.h | Candidate에 넉백 정보 추가 | 기존 후보 선택 경로 유지 |
| Type/CReactionDataTypes.h | ExecutionContext에 넉백 정보 추가 | 실행 때 공격자를 다시 조회하지 않음 |
| Component/CReactionOrchestratorComponent.cpp | 후보와 Context로 복사 | 기존 실행 판정·개입 규칙 유지 |
| Component/CReactionComponent.h/.cpp | 실행 세대 및 성공 후 시작, 종료·초기화 정리 | 시작 실패·동기 완료·오래된 소유권 방어 |
| Component/CMovementComponent.h/.cpp | Root Motion Source 시작·소유권·정리 | 충돌을 CharacterMovement에 맡기고 자기 이동 소스만 관리 |
| Type/CCharacterComponentReferenceTypes.h, Character/Player/CPlayer.cpp, Character/Enemy/CEnemy.cpp | CharacterMovementComponent 참조 구성·직접 주입 | Movement 내부의 Owner 경유 조회 대신 캐릭터의 BuildReferences에서 의존성을 확정 |
| PortfolioEditor/Private/Tests/CCombatKnockbackTests.cpp, CCombatKnockbackProbe.h | 자동화 테스트·일시적 실행기 | 데이터 경로와 경계 조건, 실제 이동 검증 |

## 실행 흐름

```text
FDamageSpec.Knockback
→ 피해 처리 / 일반 Hit 판정
→ 수평 방향 확정 (Target 위치 - Source 위치)
→ Target Context → ReactionCandidate → ReactionExecutionContext
→ 기존 실행 판정
→ 실행기 Start 성공 및 실행 세대 확인
→ Movement.StartKnockback
→ 시간 만료 또는 Reaction 종료·중단·교체·초기화에서 정리
```

- 방향이 없거나 유효하지 않으면 적용하지 않는다. ImpactNormal을 공격 진행 방향으로 간주하지 않는다.
- 조회 API와 판정 단계는 위치를 바꾸지 않는다.
- Speed 단위는 cm/s, Duration은 초다. 충돌·조기 종료가 없으면 이동량은 대략 Speed × Duration이다.
- FRootMotionSource_ConstantForce의 Override 모드와 IgnoreZAccumulate를 사용한다.
- 실제 몽타주에 Root Motion이 있거나 현재 애니메이션 Root Motion이 실행 중이면 넉백을 적용하지 않는다. 애셋 설정을 자동으로 변경하지 않는다.
- 시작 전 AI 경로 이동을 중지한다. 해당 콜백 중 새 실행이 생기면 이전 시작 요청은 소스를 덮어쓰지 않는다.
- 시간 경과는 CharacterMovement에 맡기며 별도 월드 타이머를 두지 않는다.

## 정리 및 안전성

- Reaction 실행 세대와 이동 요청 세대를 구분한다. 오래된 Stop은 현재 소유자와 다르면 무시한다.
- 이동 소스는 ID와 실제 포인터가 모두 일치할 때만 제거한다.
- 엔진의 소스 제거는 지연 처리되므로 FinishVelocity에서 전체 속도를 나중에 덮어쓰지 않는다.
- 단독 넉백의 지상 속도가 여전히 자체 출력과 일치할 때만 잔여 XY 속도를 정리한다. 공중 속도, 다른 Root Motion Source 및 애니메이션 Root Motion은 일괄 초기화하지 않는다.
- Movement는 CharacterMovement보다 먼저 tick하여 시간 만료·사망·공중·Balance 제한을 확인한다.
- EndPlay, 참조 재주입, Reaction 초기화에서도 소유 소스를 정리한다.

## 최종 API와 고정 정책

- `IsActiveReactionGeneration(InGeneration)`은 private `Active Context`에 있다. 시작 과정에서 현재 Reaction인지 검사하는 공통 조회이며 정리는 하지 않는다.
- `StopReactionKnockback(InGeneration)`은 private `Knockback`에 있다. Movement 참조 확인과 종료 요청만 공통화한다. 초기화·종료는 현재 세대, 시작 중 교체 검사는 시작 당시 세대를 전달한다.
- Movement 공개 API는 `IsKnockbackActive`, `StartKnockback`, `StopKnockback`이다. 내부 구현 순서는 `CanApplyKnockback` → `UpdateKnockback` → `ClearKnockback`이다.
- `KnockbackOwnerSerial`에는 외부 공격 요청 Serial이 아니라 Reaction 실행 세대가 들어간다. `KnockbackGeneration`은 Movement 자체의 시작 중 재진입 감지용이다.
- `StartKnockback`은 내부 세대를 보관한 **뒤** `StopActiveAIMovement`를 호출한다. 순서를 바꾸면 AI 중단 콜백 중 발생한 변경을 감지하지 못한다.
- 공격별 설정은 Speed / Duration이다. `CombatKnockbackPriority = 500`은 CPP의 명명된 상수이며 엔진이 지정한 넉백 전용 값이 아니다. InstanceName, Override, IgnoreZAccumulate, MaintainLastRootMotionVelocity는 고정 구현 정책이다.
- `ClearKnockback`은 현재/추가 대기 Root Motion Source에 다른 소스가 있는지 검사한다. 다른 소스가 있으면 잔여 XY 속도 초기화를 생략하지만 자신의 소스 제거는 수행한다.
- `bHasOwnedVelocity`는 실제 속도 소유권 토큰이 아니라 현재 XY 속도와 소스 출력의 근사 일치 검사다. 모든 이동 효과 조합의 소유권을 증명하는 것으로 해석하지 않는다.
- Balance가 없으면 넉백 적용 조건에서 허용한다. Owner / CharacterMovement / Health는 필수 참조다. Tick 기반 정리는 Movement Tick이 실행된다는 전제이며 별도의 전역 감시자는 없다.

## 검증 결과

- 2026-09-21 최신 소스의 PortfolioEditor Win64 Development 빌드 성공.
- 최신 `Portfolio.` 자동화 테스트는 28개 Success, 종료 코드 0. 그중 HUD `RenderPreview`는 NullRHI에서 본 검사를 생략하므로 이번 실행을 렌더 검증으로 계산하지 않는다. 넉백 6개·방향 보정 4개는 실행됐다.
- 최신 로컬 로그: `Saved/Logs/KnockbackCommitValidation.log`. 초기 넉백 단독 개발 단계의 24/24 기록과 구분한다.
- 기존 Regacy 메쉬의 머티리얼 참조 누락과 콘솔 변수 반복 조회 경고가 있다. 경고가 없는 빌드/실행이라고 주장하지 않는다.
- 사용자 검토 소스 커밋에는 줄 끝 공백 5곳이 남아 있다. 최신 브랜치 전체 `git diff --check` 통과로 표기하지 않는다. 검토 완료 소스를 문서 작업 중 임의 수정하지 않았다.

| 신규 테스트 | 확인 내용 |
| --- | --- |
| Ownership | 기본값·무효 입력·공중·사망 거절, 오래된 Stop 무시, 다른 소스 보존, 참조 재주입 정리 |
| ReactionLifecycle | Start 실패·동기 완료·타입 변경 콜백 취소, 완료·강제 취소, 제외 Reaction, 명시적 EndPlay 정리 |
| DamagePipeline | 준비된 조회 무실행성, 실제 피해 요청 → 방향 판정 → 후보/Context → 이동, 공중 제외 |
| Movement | 실제 CharacterMovement tick의 수평 이동·벽 충돌·시간 만료·잔여 이동·사망 정리 |
| ExternalLaunch | Launch 요청 접수 및 Falling 전환 후 넉백 정리 시 공중 속도 보존 |
| FrameRate | 30/60/144Hz 시간 간격에서 이동량 및 만료 확인 (한 30Hz 스텝 수준 허용 오차) |

테스트의 Reaction 실행기는 성공·실패·동기 완료를 재현하는 일시적 대역이다. 이동 검증은 실제 CharacterMovement와 충돌 월드를 사용하지만 실제 게임 맵·애니메이션 재생의 종합 PIE 검증은 아니다.

미검증 항목은 실제 AI MoveTo 중단 콜백 재진입, HitStop과 애니메이션 체감, 경사면·계단·다른 캐릭터 충돌, 실제 맵의 인스턴스 설정과 Root Motion 블렌딩이다. 코드상 방어와 자동화 테스트로 검증한 범위를 구분한다.

## 사용자 PIE 확인 절차

1. 실제 공격자의 BP 또는 테스트 인스턴스에서 CombatSignalSourceComponent의 DamageSpecContainer를 연다.
2. 확인할 공격 키의 Knockback에 임시로 Speed = 300, Duration = 0.2를 설정한다. 기본 0 상태에서는 밀리지 않는 것이 정상이다.
3. Enemy에게 해당 공격을 맞혀 Hit와 함께 공격자 반대 방향으로 밀리는지 확인한다.
4. 반대편 공격 데이터도 설정하여 플레이어 피격을 확인한다. 대상의 설정이 아니라 **공격자의 DamageSpec**을 조정한다.
5. 가드·패링·CollapseHit·처형·사망·공중 피격에서는 적용되지 않는지 확인한다.
6. 연속 Hit, 벽·경사면·계단·다른 캐릭터, AI 추적 중 피격을 확인한다.
7. HitStop 중 이동도 함께 느려지는지, 끝난 뒤 원치 않는 밀림이 남지 않는지 확인한다.
8. 피격 종료 후 이동 입력·AI 이동 재개 및 공격 Root Motion에서 Hit로 넘어가는 블렌드 구간을 확인한다.

이 절차의 체감·애니메이션 품질 확인은 자동화 테스트 성공과 별개다. 멀티플레이 예측·복제 지원은 이번 작업에서 검증하지 않는다.
