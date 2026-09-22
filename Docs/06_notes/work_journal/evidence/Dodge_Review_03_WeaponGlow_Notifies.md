# Review 03 — 무기 발광 포인트 노티파이 전환

작성일: 2026-09-22. 코드·에셋 적용 및 자동 검증 완료, 실제 화면/타이밍은 사용자 PIE 검토 대기.

## 요청과 합의

- 무기 발광 시작·종료를 각각 포인트 Notify로 배치한다. Notify State는 사용하지 않는다.
- 퍼펙트 닷지에서 시작한 발광을 반격 준비부터 카운터 러시 공격까지 유지할 수 있어야 한다.
- 러시로 정상 전환하면 유지하고, 다른 중단·러시 취소·종료 알림 누락에서는 발광이 남지 않아야 한다.
- 정상 연출은 Notify가 결정한다. 실행 종료 정리와 실시간 안전 제한은 누락/중단 대비 장치다.
- 퍼펙트 판정, 반격 기회/입력 규칙, 이동, 공격 판정, 몸 발광, 프레즈넬 머티리얼 자체는 이번 범위에서 변경하지 않는다.

## 에디터 사용법

몽타주 Notify 트랙에 `Weapon Glow (Point)`를 추가하고 Details를 설정한다.

| 속성 | 의미 |
|---|---|
| Cue = Start | 발광 시작 |
| Cue = End | 현재 강도에서 발광 해제 시작 |
| Counter Rush = false | 퍼펙트 닷지 리액션 소유 Notify |
| Counter Rush = true | 카운터 러시 액션 소유 Notify |
| Peak Weight | 최대 Alpha, 0~1. 발광색/Emissive 강도와는 별개 |
| Fade In | Start 이후 상승 시간, 실시간 초 |
| Fade Out | Start에서는 비정상 종료 기본값, End에서는 해당 종료의 하강 시간 |
| Safety Timeout | Start 이후 End 누락에 대비한 실시간 제한. 기본 8초 |

Start/End 마커 사이에서 유지한다. Delay/Hold를 별도 숫자로 작성하지 않는다. End는 완전 소멸 시점이 아니라 FadeOut 시작점이다. 페이드에는 기존 smoothstep 보간을 사용한다.

무기 머티리얼은 기존 `ReactionFeedbackComponent → CounterWeaponMaterial`, Alpha 파라미터 이름은 `GlowWeightParameter`에서 지정한다. 컴포넌트의 `CounterWeaponEnvelope`는 제거했다. 몸의 `PerfectBodyEnvelope`는 유지했다.

### 이번에 저장한 초기 배치

| 몽타주 | Start | End |
|---|---:|---:|
| `/Game/04_Montage/Dodge/Default/M_PerfectDodge` | 0.010초 | 1.290초 |
| `/Game/04_Montage/Dodge/Stellar/M_PerfectDodge_B_Root_Stellar` | 0.010초 | 약 1.123초 |
| `/Game/04_Montage/Dodge/Stellar/M_CounterRush_Stellar` | 없음: 기존 발광 인계 | 약 0.583초 |

Start 초기값은 Peak=1, FadeIn=0.08초, 기본 FadeOut=0.18초, SafetyTimeout=8초. End FadeOut=0.18초.

닷지 End는 기존 CompleteReaction보다 0.01초 앞, 러시 End는 현재 공격 Collision 구간 종료 시점에 초기 배치했다. **러시 End와 Collision의 런타임 연결은 없다.** 이후 두 마커를 독립적으로 이동할 수 있다. 퍼펙트 닷지의 몸은 성공 직후 시작하는 기존 방식이고, 무기는 초기 프레임의 0.01초 마커부터 시작한다.

반격 기회에는 기존 종료 후 grace 규칙이 여전히 존재한다. 따라서 이번 무기 End는 반격 가능 여부와 일대일 표시가 아니다. 기회 윈도우 자체를 몽타주로 변경하는 작업은 별도 범위다.

## 코드 변경과 이유

### `Source/Portfolio/Notify/CAnimNotify_WeaponGlow.h/.cpp`

- Start/End 공통 포인트 Notify 추가. 가변 재생 상태는 공유 Notify 객체에 저장하지 않는다.
- 플레이어 주 메시, 실행 종류, 원본 몽타주 인스턴스 ID를 검증한다.
- 검증된 액션/리액션 실행 generation과 함께 컴포넌트를 호출한다. 이전 실행에서 뒤늦게 도착한 End를 거부한다.

### `Source/Portfolio/Component/CReactionFeedbackComponent.h/.cpp`

- `PlayPerfectDodgeSuccess`의 무기 자동 시작 삭제. 몸 처리는 유지.
- Tick의 반격 기회 만료 기반 종료 삭제. 대신 현재 소유 발광의 실시간 deadline 검사.
- `StartWeaponGlow`, `EndWeaponGlow`: Notify 설정으로 sustain/페이드 시작. End 처리 시 deadline을 지운 뒤 하강은 끝까지 진행한다.
- `BeginWeaponGlowTransfer`, `TransferCounterWeaponToAttack`, `FinishWeaponGlowTransfer`: 이전 닷지 종료와 러시 시작 사이에서 발광을 보존하고 실제 러시 commit에서만 소유권 변경.
- `EndWeaponGlowOwner`: 현재 소유 실행 종료 시 누락 대비 페이드. 전환 대기 중에는 보류하고, 전환 실패 시 해제한다.
- 실행 종류 + generation으로 End 소유권을 구분하며, 성공 결과 serial로 러시 정리를 구분한다.
- 전환/중복 Start로 안전 제한을 연장하지 않는다. 별도 TimerManager 콜백 없이 기존 Tick과 `FPlatformTime::Seconds()`로 검사하므로 오래된 타이머 콜백이 새 발광을 끄는 문제를 피한다.
- 무기 교체/제거 또는 사망 확인 Tick에서는 기존 overlay를 즉시 복원한다. EndPlay/재초기화도 복원한다. Tick을 강제로 끈 상태에서는 watchdog도 실행되지 않으므로 EndPlay 정리가 마지막 안전장치다.
- 기존 Overlay 소유권 보호를 유지하여 다른 시스템이 덮어쓴 Overlay는 되돌리지 않는다.

### `Source/Portfolio/Component/CActionComponent.cpp`

- CounterRush 결정 적용 전체를 발광 전환 대기 구간으로 감싼다. 기존 순서는 리액션 중단 → 액션 시작이므로, 러시 시작 함수만 수정하면 닷지 중단 시 먼저 꺼지는 문제가 생긴다.
- scope 종료에서 전환 실패 정리를 수행한다.
- `HandleActionCollisionWindowEnd`에서 발광 종료 호출 제거.
- 액션 컨텍스트 정리에서도 해당 generation 발광을 정리한다.

### `Source/Portfolio/Component/CReactionComponent.cpp`

- 리액션 컨텍스트 정리에 해당 generation 발광 정리를 추가한다. 정상 종료라도 End 누락 시 무한 유지하지 않는다.

### `Source/Portfolio/Action/CAction_CounterRush.h/.cpp`

- 공격 충돌 종료용 `EndCounterAttackPresentation` API 제거.
- 실행 cleanup의 serial 기반 발광 해제는 유지한다. 러시 취소/종료 시 fallback이며 공격 히트 여부와 무관하다.

### `Source/PortfolioEditor/Private/Audit/CDodgeAssetsCommandlet.cpp`

- `-WeaponGlowOnly`: BP가 실제 참조하는 몽타주를 조회한다. 변경된 에셋 경로를 이전 경로로 되돌리지 않는다.
- `-Apply`: 없는 발광 Notify만 추가·저장한다. 이미 배치된 마커/파라미터는 덮어쓰지 않는다.
- `-Verify`: 별도 프로세스에서 저장된 마커 존재/종류를 확인한다.
- 기존 머리카락 추가 전용 `-HairOnly` 변경도 작업 트리에 남아 있던 이전 작업이다.

## 검증

- PortfolioEditor Development 빌드 성공.
- 전체 자동 테스트 **59/59 성공**, `Saved/Logs/WeaponGlowFinalTests.log`.
- 에셋 적용/별도 프로세스 재로드 검증 0 errors / 0 warnings:
  `Saved/Logs/WeaponGlowApply.log`, `Saved/Logs/WeaponGlowVerify.log`.
- `WeaponGlowOwnership`: 중복 Start, 이전 generation End, 전환 대기, 러시 commit, deadline 유지, 러시 취소, 누락 timeout, 실패 전환, End 페이드, 무기 제거 검증.
- `PerfectLifecycle`: 성공만으로 자동 발광하지 않음, Start로 시작, 충돌 종료와 분리, 이전 닷지 End가 러시를 끄지 않음 검증.
- 기존 AuthoredPlayback 테스트는 사용자가 0.167초로 옮긴 슬로우/비네트 마커를 0.04초라고 가정해 실패했다. 에셋을 되돌리지 않고 실제 마커 시점을 읽도록 수정했다.
- timeout 테스트로 발견한 0초 페이드의 한 Tick 지연은 Tick 내 동일한 시각을 사용하도록 수정했다.
- NullRHI 검증이므로 화면에서의 색/윤곽선/체감 타이밍은 검증하지 않았다.

## 사용자 PIE 검토

1. 닷지 Start/End를 옮겼을 때 무기만 해당 시점에 켜지고 꺼지는지.
2. End 전에 러시를 입력하면 중간에 꺼지지 않고 러시 End부터 해제되는지.
3. 일반 중단/러시 취소/무기 제거 후 발광이 남지 않는지.
4. 임시로 End를 제거했을 때 실행 종료 또는 안전 제한으로 정리되는지.
5. Start의 PeakWeight/FadeIn, End의 FadeOut 조절 결과.

## 변경 보존과 커밋 범위

코드 커밋: `55ea0db6` (`feat(combat): author counter weapon glow with point notifies`).

적용 직전 3개 몽타주 복사본: `Saved/WeaponGlowBackup_20260922_202719/`.
사용자가 진행 중인 애니메이션 이동/삭제, BP 참조 변경, 맵/머티리얼 수정은 되돌리지 않았다.
이번 몽타주 변경은 기존 사용자 변경 및 새 경로 애니메이션 의존성과 섞여 있으므로, 그 이동 작업 전체를 임의로 묶어 커밋하지 않는다. 코드와 문서는 분리 커밋하고 에셋 작업 트리는 보존한다. 따라서 코드 커밋만 다른 작업 트리에 가져오면 별도로 `-WeaponGlowOnly -Apply` 또는 수동 Notify 배치가 필요하다.
