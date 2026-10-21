# Review 06 — 몸 발광 Point Notify, 2026-09-23

## 변경

- CAnimNotify_BodyGlow.h/cpp: 공유 상태 없는 단일 Start/End marker. PerfectDodge 주 메시 및 현재 몽타주 인스턴스 검사. PeakWeight, FadeIn, FadeOut, SafetyTimeout 편집.
- CReactionFeedbackComponent.h/cpp: 성공 시 몸 자동 시작 제거. StartBodyGlow/EndBodyGlow/EndBodyGlowOwner 및 sustain/release 구현. generation+instance 소유권, 중복 Start tombstone, timeout/소유권 watchdog. 기존 파츠·태그·재질·overlay 복원 재사용. PerfectBodyEnvelope는 이행 전용 숨김 데이터로 남기고 런타임 읽기 제거.
- CReactionComponent.cpp: 실행 컨텍스트 정리 시 몸 발광 fallback 해제. 무기 인계 코드는 유지. 몸은 카운터로 인계하지 않음.
- CDodgeAssetsCommandlet.cpp: 실제 Stellar PerfectDodge 참조 기반 BodyGlowPoints dry-run/apply/verify. 명시적 시작·종료 시간, 모든 경로 사전 검사, 기존 notify 보존.
- CCombatDodgeTests.cpp: BodyGlowOwnership 추가. 자동 시작 없음, 실제 marker, 중복/이전 실행 이벤트, 부분 fade에서 release, 후속 실행 보호, timeout, 취소, 몸 해제/무기 인계 분리, 사망·EndPlay 정리.

## 현재 에셋과 초기 타이밍

작업 시작 시 사용자 변경: BP_CEnemy, SciFiMap, M_PerfectDodge_B_Root_Stellar. 적 BP·맵은 수정·스테이징하지 않았다. 몽타주는 작업 시작 상태를 `Saved/Backups/BodyGlowPoints_20260923/M_PerfectDodge_B_Root_Stellar.uasset`에 보존하고 두 BodyGlow marker만 추가했다.

확인한 사용자 설정:

- WeaponGlow Start .083, Slow/Vignette .167, CounterOpportunity .667, WeaponGlow End 1.133, CompleteReaction 1.150 (몽타주 초).
- 플레이어 slow .3배, real envelope fade-in .1 / hold .4 / fade-out .1, play rate1.
- 기존 몸 real envelope delay0 / in.08 / hold.4 / out.22.

선택: Body Start .01 / End .30, Peak1 / FadeIn .08 / FadeOut .22 / Safety8.

근거: 기존 몸 release는 실시간 약 .48초. 다른 시간 효과가 없고 play rate1이면 slow .167부터 첫 .1초의 평균 dilation 약 .65, 이후 .3이므로 실시간 .48초는 몽타주 약 .296초에 해당한다. End .30은 근사 초기값이며 정확한 화면 동등성을 주장하지 않는다. Start .01은 실행 활성화 전 time-zero callback 위험을 피하는 작은 양의 시점이다. Fade는 여전히 실시간이고 marker는 몽타주 시간이다.

재로드 Verify에서 BodyGlow 이외 notify의 class/time/end/reachability/playRate 목록은 dry-run과 동일함을 비교했다. 원래 사용자 카운터 시점을 .45로 되돌리지 않았다. 최종 몽타주 커밋은 작업 시작부터 존재한 사용자 수정도 함께 보존한다.

## 검증

- PortfolioEditor Win64 Development 최신 빌드 성공.
- BodyGlowApply / BodyGlowVerify: 0 errors, 0 warnings. Verify 새 프로세스에서 Start .010 / End .300, 추가 필요0/0.
- 최초 Apply는 PowerShell의 인용하지 않은 소수 인수 분리로 사전 검사에서 거절됨. 에셋 저장 전 차단했고 인용 후 적용 성공.
- 최초 전체 테스트는 BeginPlay 없는 fixture에서 직접 EndPlay를 호출하여 엔진 assertion. fixture의 등록/BeginPlay 순서를 보완함. 런타임 assertion을 숨기지 않음.
- 최종 `Saved/Logs/BodyGlowFinalRegression.log`: 65개 전부 성공. 새 테스트는 여러 시나리오를 포함한다.
- 화면 PIE/GPU 시각 검토는 사용자 대기. 시작·종료 위치와 fade 체감은 사용자 편집 대상.

## 한계 / 유지 정책

- Body Glow는 현재 PerfectDodge 리액션 전용이다. 카운터에 새 몸 발광을 넣는 Action-domain 기능은 이번 범위에 포함하지 않았다.
- 새로운 성공이 들어오면 이전 표현을 정리한 뒤 새 marker가 시작한다. 여러 실행의 몸 발광을 동시에 합성하지 않는다.
- End 누락 시 정상 종료 fallback, watchdog timeout 및 사망/EndPlay 즉시 복원의 여러 정리 경로를 유지한다.
- 숨김 legacy envelope는 호환 이행용이며 에디터에서 조절하는 활성 설정이 아니다. 전체 자산 이행이 확인되기 전 직렬화 필드를 삭제하지 않는다.
