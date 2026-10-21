# Combat Dodge 브랜치 감사 — 2026-09-23

## 기준과 범위

- 요청: 프로젝트 관례와 이번 브랜치 구현을 비교하여 결함, 하드코딩, 확장성 및 저작 가능성을 검토하고 개선안을 제시한다. 구현 수정이나 커밋은 수행하지 않는다.
- 브랜치: `feat/combat-dodge`, HEAD `4799f5f7a`. main과의 merge-base `35891fca9272669f2853448635c077f9ad0e3e9a`.
- HEAD까지 30개 커밋, 변경 Source 경로 97개. 현재 Source 목록은 474개 파일. 미커밋 트레일 코드·테스트, BP/DA/스켈레톤, Counter 폴더 이동, LightSaber 선택도 포함했다.
- 프로젝트 구조 목록 및 S37/S38/S39 계약을 기준으로 브랜치 변경 경로와 연결된 실행·방어·피드백·무기·테스트·저작 도구를 집중 검토했다. 독립 검토자는 아키텍처, 생애주기, 저작/테스트의 세 역할로 나눴다. 주 검토자가 주요 소스 경로와 실행 로그를 재확인했다.
- **프로젝트 474개 소스 전체의 모든 줄, 모든 BP 그래프와 외부 에셋 내부를 전부 검증했다는 의미는 아니다.** 미변경 AI/UI/외부 플러그인 등의 심층 감사는 잔여 범위다. 이번 결과는 전체 구조를 기준으로 한 브랜치 중심 1차 감사다.
- 새 빌드 및 화면 PIE 검증은 수행하지 않았다. 기존 실행 바이너리로 NullRHI 자동화 테스트를 새로 실행했다. GPU 표현, 최종 아트 품질과 사용자 조작감은 인증하지 않는다.

## 결론

전면 재작성은 권장하지 않는다. 실행/판정/이동/표현 분리의 기본 방향과 소유권 정리는 유지 가치가 있다. 문제는 일부 전환 권한이 타이머에 의존하는 점, 공통 실행 계층에 들어간 특수 처리, 현재 저작 설정과 검증 도구의 불일치다.

수치가 있다는 사실 자체는 결함이 아니다. 게임 디자인 수치는 편집 가능하게 하고, 시스템 계약 값은 이름과 근거를 제공하며, 일회 마이그레이션 상수는 런타임 및 범용 검증과 분리하는 것이 적절하다.

## 검증 결과

실행: `UnrealEditor-Cmd Portfolio.uproject -unattended -nop4 -nosplash -NullRHI -nosound -ExecCmds="Automation RunTests Portfolio." -TestExit="Automation Test Queue Empty"`.

- 로그: `Saved/Logs/BranchAuditRegression.log`.
- **62개 실행, 61개 성공, 1개 실패.** 실패: `Portfolio.Combat.Dodge.AuthoredPlayback`.
- 로그 1101~1113: 이전 `/Game/04_Montage/Dodge/Stellar/M_CounterRush_Stellar` 로드 실패, 테스트 소스 619행의 not-null assertion 실패.
- 프로세스 종료 코드는 0이었다. 따라서 종료 코드 또는 `62 tests performed`만으로 성공 판정하면 안 된다. 개별 Result/실패 목록을 확인해야 한다.
- 구형 Mannequin 머티리얼 참조 누락 경고도 존재한다. 이번 브랜치 기원이라고 확인하지 않았으므로 별도 기존 에셋 정리 후보로 남긴다.
- 중간 진행 안내에서 마지막 요약을 통과 수로 오독했으며, 전체 로그 확인 후 즉시 정정했다. 이 문서의 61/1이 최종 결과다.

## 우선 개선 항목

### A01. 현재 에셋 이동을 따라가지 못한 테스트 — 우선 수정 / 실행 확인

근거: `Source/PortfolioEditor/Private/Tests/CCombatDodgeTests.cpp:618`에 이전 카운터 몽타주 경로가 고정돼 있다. 실제 이번 실행에서 실패했다. 사용자에 의한 폴더 이동 자체나 현재 게임의 몽타주 연결이 잘못됐다는 뜻은 아니다.

권장: 실제 플레이어 설정의 CounterRush ActionData에서 몽타주를 찾아 통합 검증한다. 고정 fixture 기반 단위 테스트는 별도로 유지한다. 새 폴더 문자열로 단순 교체하는 것보다 재이동에도 견디는 참조 조회가 낫다. CI/검증 스크립트는 개별 테스트 실패를 실패 종료로 집계한다.

### A02. 유예시간 0이 정상 카운터 전환까지 차단 — 우선 수정 / 정적 경로 확인

근거:

- `Source/Portfolio/Component/CActionComponent.cpp:340`: 카운터 전환을 시작한 뒤 Intervene 처리에서 기존 리액션을 종료한다.
- `Source/Portfolio/Component/CDefenseComponent.cpp:220`: 전환 중 종료 시에도 `now + CounterGraceSeconds`로 만료시각을 정하고 PerfectGeneration을 지운다.
- 같은 파일 `:252`: 활성 PerfectGeneration이 없으면 만료시각으로 거절한다.
- `Source/Portfolio/Action/CAction_CounterRush.cpp:46`, `:68`: Start/Commit에서 기회와 serial을 다시 조회한다.

결과: 애니메이션 종료 후 반격을 막으려고 grace를 0으로 설정하면, 애니메이션 도중 유효했던 반격도 리액션 종료 후 조회에서 거절된다. 이는 코드 경로로 확인한 결함이며 이번 감사에서 별도 PIE 재현은 하지 않았다.

권장: 전환 전에 serial/대상을 예약하는 명시적 권한을 만들고 성공 commit에서 한 번 소비한다. 실패하면 예약을 해제한다. 전환 권한과 몽타주 종료 후 유예시간을 분리해야 한다. epsilon 시간 추가로 우회하지 않는다.

완료 기준: grace 0/양수, 몽타주 도중 전환, Start 실패, 대상 사망, 중복 입력 테스트.

### A03. 카운터 기회는 아직 몽타주 저작 방식이 아님 — 요구사항 정리 후 변경

근거: `CDefenseComponent.cpp:249`는 `CounterConnectStart` 숫자와 몽타주 위치를 비교하고, `:224` 및 `:252`는 완료 후 플랫폼 실시간 grace를 허용한다. `CDefenseTuningTypes.h:39`, `:43`의 기본값은 .45/.4다. 이는 클래스 기본값이며 현재 BP override 값으로 단정하지 않는다.

문제: 사용자 의도인 “특정 시점 이후 열리고 종료/중단 시 사라짐”과 다르다. CounterConnectStart를 몽타주 끝보다 뒤로 설정해도 완료 후 grace는 생긴다.

권장: generation/몽타주 인스턴스에 소유된 카운터 기회를 point notify로 연다. 필요하면 닫기 notify를 제공하고, 실행 종료는 항상 폐쇄한다. 카운터 전환 예약만 A02 규칙으로 넘긴다. 종료 후 grace를 유지하려면 별도 명시 옵션으로 한다. 무기 발광의 안전 타이머와 게임플레이 기회 타이머는 혼동하지 않는다.

### A04. 러시 회전의 600cm/180도 고정 — 수치 확장 시 오동작

근거: `Source/Portfolio/Action/CAction_CounterRush.cpp:98`의 `TryFaceTarget(TargetLocation, 600.f, 180.f)`. `CDefenseComponent.cpp:241`의 허용 거리는 편집 가능하다. `CMovementComponent.cpp:336`은 회전 최대거리를 넘으면 거절한다.

문제: 반격 허용거리와 이동거리를 늘려도 600cm 밖에서는 별도 회전 호출이 실패할 수 있다. 공통 ActionData 회전 정책과도 별개다.

권장: 확정한 카운터 TargetLocation은 유지하되 공통 StartFacing 설정 또는 명시된 카운터 회전 정책을 사용한다. 반환 실패를 진단 로그에 기록한다. 기존 공통 경로는 Combo/CombatTarget 계약이 있으므로 단순 호출 이동으로 끝내지 않는다.

### A05. 일반 실행 컴포넌트에 CounterRush 특수 처리 집중 — 구조 부채

근거: `CActionComponent.cpp:339` 전환 분기, `:439` 구체 실행기 Cast 후 CommitStarted, `:1094` ReactionFeedback 발광 정리. `CAction_CounterRush.cpp:100` 이후 ReactionFeedback 검색·호출이 반복된다. `CCharacterComponentReferenceTypes.h:61`에는 이미 주입 가능한 참조가 있다.

권장 순서:

1. 기존 참조 주입 활용.
2. 실행 확정 이후 hook을 기본 실행기에 두고 구체 타입 Cast 제거. 실패·재진입 가드 유지.
3. Action/Reaction 사이 표현 인계를 작은 전용 API로 캡슐화.
4. 필요성이 확인되면 공유 전투 표현 컴포넌트로 분리. 대형 범용 실행 프레임워크를 새로 만들지는 않는다.

현재 정상 PIE 흐름을 부정하는 지적이 아니라, Blink 등 다음 기능 추가 시 예외 누적을 막기 위한 개선이다.

### A06. 검증 도구가 Scifi/초기 에셋에 묶여 있음 — 저작 도구 부채

근거: `Source/PortfolioEditor/Private/Audit/CDodgeAssetsCommandlet.cpp:112`는 `NS_SlashTrail_Scifi_Sample` 이름을 요구한다. 현재 LightSaber 선택과 맞지 않는다. 같은 파일 `:263` 이후 초기 Fresnel/몽타주 경로 검증도 유효한 사용자 편집을 불합격으로 처리할 수 있다.

권장: 일회 Bootstrap/Migration과 현재 상태 Validator를 분리한다. 전자는 특정 원본 상수를 유지해도 되지만, 후자는 실제 BP→ActionData→몽타주/DA→선택 Niagara를 따라가며 필요한 소켓과 파라미터 이름·타입을 검사한다. 삭제된 Scifi를 복원하는 방식으로 해결하지 않는다. LegacyBootstrapApply 안전장치는 유지한다.

### A07. 트레일 테스트가 허용된 아트 설정을 제한 — 테스트 계약 수정

근거: `Source/PortfolioEditor/Private/Tests/CWeaponTrailAlignmentTests.cpp`의 고정 Z 정렬 assertion과 실제 액터 테스트의 raw socket transform 비교. `CWeaponTrailDataAsset.h:31` 이후에는 상대 위치/회전이 공개돼 있다.

문제: 정상적인 상대 위치·회전 변경에도 테스트가 실패할 수 있다. 효과 개수와 기본 Combo 모드를 고정한 기대도 제작 정책과 기능 검증을 혼합한다.

권장: 통합 테스트는 설정된 RelativeTransform × SocketTransform을 검증하고, 개수는 설정 정의와 비교한다. “기본 소켓 Z가 칼 방향” 같은 정확한 fixture 검증은 별도 테스트로 둔다. Niagara Tick 없이 부착이 따라오는 검증은 유지한다.

### A08. HitStop 진단 이벤트 호출 유실 — 기존 관측 계약 회귀

근거: `Source/Portfolio/System/Combat/CWorldSubsystem_CombatFeedback.cpp:63` 이후 새 ApplyHitStop/AddTimeEffect 경로에 applied/rejected audit 호출이 없다. `RecordCombatFeedbackHitStopAppliedForAudit` / `RejectedForAudit`는 저장소 검색에서 정의·선언만 남았다. 기준 커밋에는 실행 호출이 있었다.

권장: 유효성 검사 및 effect 삽입 시점에 성공/거절 기록을 복구한다. Tick마다 기록하지 않는다. 요청→적용 또는 거절의 진단 생애주기를 테스트한다.

### A09. 몸 발광은 여전히 성공 즉시 + 컴포넌트 envelope — 디자인 선택 필요

근거: `Source/Portfolio/Component/CReactionFeedbackComponent.cpp:96`의 PlayPerfectDodgeSuccess에서 BodyEnvelopeState를 직접 시작한다. 헤더 주석도 몸은 success cue, 무기/슬로우/화면은 point notify임을 명시한다.

따라서 몸·무기 모두 이미 몽타주 제어라는 설명은 부정확하다. 현재 부드러운 표현과 cleanup이 실패했다는 뜻은 아니다.

권장: 모든 성공에 동일한 발광이면 현재 구조 유지 가능. 몽타주마다 시작점을 편집하려면 몸 전용 point notify를 추가하고 fade 곡선/시간은 재사용한다. 판정 노티파이와 표현 노티파이는 분리한다.

## 확정 결함과 분리할 잔여 위험

- `CCombatNotifyOwnership.cpp:17`, `:27`: 현재 실행이 Dodge/CounterRush/PerfectDodge가 아니면 소유권 검사를 통과시킨다. 오래된 신규 실행 notify가 기존 Combo/Hit에 영향을 주는 역방향은 추가 테스트가 필요하다. 지연 notify 재현 없이 실제 오동작으로 단정하지 않는다. 전역 fail-closed는 기존 context 없는 notify를 깨뜨릴 수 있어 점진 이행이 필요하다.
- 플랫폼 실시간 envelope/grace는 pause 중에도 시각이 진행된다. 슬로우 배율 독립과 pause 독립은 다른 요구다. 정책을 문서화하고 pause 테스트를 추가한다.
- CustomTimeDilation baseline 복구는 외부 writer와 충돌 가능하다. 이는 기존 HitStop에서도 있던 문제이며 현재 Source의 writer는 해당 subsystem뿐이다. 당장은 단일 writer 계약을 명시하고, 실제 두 번째 writer가 생길 때 중재를 확장한다.
- 몽타주 감사 도구는 terminal notify 검증 중심이다. DodgeWindow, WeaponGlow 인계, 실제 Niagara 파라미터 타입/유한한 폭 등의 의미 검증을 추가할 여지가 있다. 모든 아트 값에 특정 정답을 강제하지 않는다.

## 유지할 설계

- 일반 Dodge Action / 접촉 확정 PerfectDodge Reaction 구분과 판정→실행 오케스트레이션.
- 캡슐 반경+여유 거리 기반 순수 러시 계획, 시작 시 고정 방향, CharacterMovement 충돌, 이동과 공격 분리.
- 원본 모션 보존과 루트 고정 전용 복제본.
- generation/몽타주 인스턴스 검사, 중복 소비 방지, 대상 weak pointer/사망 delegate 정리.
- 무기 발광 인계 scope, 안전 만료, 무기 교체/사망/EndPlay 정리.
- overlay/camera가 자신의 소유권이 남아 있을 때만 복구하는 처리.
- 개별 시간 효과 lease, 자신이 만든 RMS만 제거하는 러시 cleanup.
- Water의 기존 소켓 샘플링과 새 트레일의 명시적 소켓 부착을 opt-in으로 공존시킨 점.
- 별도 트레일 DA 라우팅. 런타임에 특정 Niagara 제품 이름을 강제하지 않은 점.

## 권장 실제 작업 순서

### 1. 검증 신뢰도 회복

현재 에셋 참조를 기준으로 실패 테스트 및 validator를 정리한다. 상대 transform 테스트 계약을 수정하고 진단 이벤트를 복구한다. 최신 소스로 빌드한 뒤 전체 자동화 결과를 실패 목록까지 기록한다. 기존 사용자 에셋 튜닝은 유지한다.

### 2. 카운터 전환 결함 수정

zero-grace 회귀 테스트를 먼저 추가하고 예약/소비 경로를 분리한다. 600cm 회전 제한을 정책에 맞춘다. 이 단계는 현재 시각 표현을 바꾸지 않는 것을 목표로 한다.

### 3. 저작 권한 정리

카운터 기회 point notify + 실행 종료 cleanup을 구현한다. 몸 발광을 notify로 옮길지는 사용자 선택 후 처리한다. 수치/노티파이/컴포넌트 각각의 소유 항목을 한 장의 현재 저작 가이드로 정리한다.

### 4. 제한적 구조 정리

참조 주입, 실행 확정 hook, 표현 인계 API를 정리한다. 기능별 작은 커밋으로 나누고 매 단계 회귀 테스트한다. Action/Reaction 통합이나 거대한 범용 시스템 재작성은 하지 않는다.

### 5. 경계 상황 및 사용자 PIE

자동 검증: 중단/죽음/대상 소멸/무기 교체/연속 입력/zero grace/먼 타깃/벽 충돌/지연 notify/설정 offset.

사용자 확인: 판정·반격 가능 시점, 공격 사거리/방향, 발광 fade, 트레일 폭·색상·잔류, FOV/슬로우/비네트 체감. NullRHI 테스트로 시각적 만족도를 대신하지 않는다.

## 문서와 커밋 원칙

- 과거 62/62 및 Scifi 선택 기록은 당시 증거로 보존하고, 현재 상태와 구분되는 날짜·후속 문서 링크를 추가한다. 역사를 현재 값으로 덮어쓰지 않는다.
- 현재 사용법 문서와 마이그레이션/실험 기록을 분리한다.
- 이번 감사는 이 보고서 외 구현·에셋 변경 및 커밋을 하지 않았다. 이후 수정 승인 시 코드/테스트, 에셋 저작, 문서를 기능적으로 검증 가능한 단위로 나눈다.
