# Review 05 — 감사 후 보완, 2026-09-23

## 변경과 의도

- CDefenseComponent: 기존 grace 조회와 독립된 CounterSerial 예약. ReservedPerfectGeneration으로 실패 시 이미 중단한 리액션의 권한만 폐쇄하고, 기존 legacy grace 중 재생 실패는 원래 재시도 가능성을 유지.
- CDefenseTuningTypes / CAnimNotify_CounterOpportunity: opt-in 몽타주 기회 모드와 Open/Close point notify. 주 메시, 활성 PerfectDodge, generation/instance 검증. 기존 모드는 기본 false로 호환 보존.
- CAction / CActionComponent: 실행 확정 후 virtual CommitStarted hook 및 재진입 검사. 구체 CounterRush Cast 제거. ReactionFeedback 참조 주입 및 실행기 전달.
- CAction_CounterRush / CCounterRushTypes: 반복 컴포넌트 검색 제거, 회전 거리/각도 편집 및 유효성 검사. 기본 600/180 유지.
- CombatFeedback subsystem: HitStop 적용·거절 audit 복구. invalid actor/world/duration/dilation 검증. slow lease 또는 Tick에서 이벤트 중복 생성하지 않음.
- CDodgeAssetsCommandlet: 현재 선택 트레일 검증을 Scifi Apply 마이그레이션에서 분리. 실제 BP PerfectDodge 참조를 따른 CounterOpportunityWindow dry-run/apply/verify 추가.
- CCombatDodgeTests: 현재 Stellar BP ActionDatas에서 카운터 몽타주 조회. CounterOpportunityOwnership 테스트 추가(legacy/authored, zero grace, stale marker, 예약 성공/미소비 정리).
- CCombatTimeEffectsTests: HitStopAudit 추가. 초기 로그 수집 동기화 실패를 발견하여 FlushThreadedLogs + 잠금 스냅샷으로 수정.
- CWeaponTrailAlignmentTests: 실제 DA 정의·상대 transform·효과 개수 기준 검증. Niagara Tick 없이 socket 추종 검사 유지.
- Scripts/Validation/Test-AutomationLog.ps1: 개별 결과와 완료 실행 수를 확인해 실패를 exit 1로 전달.

## 에셋

- 기존 미커밋 LightSaber 및 소켓/DA/카운터 몽타주 폴더 이동을 보존한다. 이번 보완이 이 과거 작업을 새로 만든 것처럼 기록하지 않는다.
- 현재 BP 및 PerfectDodge 몽타주는 `Saved/Backups/CounterOpportunity_20260923`에 이행 전 복사.
- M_PerfectDodge_B_Root_Stellar: 기존 .45초 숫자 시작값을 Open point notify로 이행. terminal CompleteReaction 1.133초. 다른 notify 유지.
- BP_CPlayer_Stellar: bUseMontageCounterWindow=true. 실제 새 프로세스 재로드 Verify 성공.
- 몸 envelope, Fresnel/무기 아트, 무기 glow marker, 러시 root lock은 유지.

## 검증

- PortfolioEditor Win64 Development 새 빌드 성공.
- 최초 감사: 62개 중61성공/1실패(옛 경로).
- 보완 첫 실행: 64개 중63성공/1실패(새 HitStopAudit의 비동기 로그 수집).
- 보완 최종: **64개 전부 성공**, `Saved/Logs/RefinementFinalRegression.log`; 집계 스크립트 `Success=64 Fail=0 Executed=64`.
- CounterOpportunity dry-run/apply/새 프로세스 Verify 성공, 각0 error/0 warning. 로그 CounterOpportunityDryRun/Apply/Verify.log.
- CurrentTrailVerify.log: 현재 LightSaber 파라미터/부착 검증 성공, 0 error/0 warning.
- git diff --check 통과. 자동화의 기존 Mannequin 머티리얼 누락 경고는 별개 기존 에셋 후보로 남음.
- 에이전트: 아키텍처 구현, 저작·트레일 테스트 구현, 생애주기 진단 구현/독립 재검토. 교차검토 중 실패한 legacy grace 재시도 호환 회귀를 발견·수정.

## 남겨둔 범위와 판단

- 모든 Action/Reaction에 대한 Notify 소유권 전역 이행은 하지 않았다. 새 point marker의 stale instance 거절은 테스트했지만 기존 Combo/Hit의 모든 지연 이벤트를 검증한 것은 아니다.
- ActionComponent의 카운터 전환 브리지는 기존 scope cleanup을 유지했다. concrete 실행기 commit와 반복 lookup은 제거했으나 모든 전환을 범용 예약 프레임워크로 확장하지 않았다.
- 범용 CombatPresentation 컴포넌트 분리, 몸 발광 notify 이행, pause용 공통 clock은 이번에 적용하지 않았다.
- 구형 bootstrap 경로는 역사적 마이그레이션 용도다. 현재 저작 검증은 명시된 좁은 Verify 모드를 사용한다.
- UI PIE/GPU 최종 품질은 사용자 확인 대기. 현재 저작 위치·검토 항목은 `Docs/06_notes/N_CounterRush_Current_Authoring.md` 참고.
- 목표모드: 기존 미완료 목표 때문에 새 목표 등록이 거부되었다. 기존 목표를 허위로 완료 처리하지 않았으며 실제 작업/검증 상태를 이 문서로 남긴다.

## 보존 방식

- 코드/테스트: `efa88daa`.
- 콘텐츠: `338cd0b7` (기존 사용자 Counter 폴더 이동 포함; 옛 경로 삭제는 새로 에셋을 폐기한 것이 아님).
- 후속 문서 커밋을 포함한 HEAD를 이번 오프라인 검증 기준점으로 사용한다.

사전 dirty worktree를 강제로 초기화하거나 한 덩어리 baseline으로 커밋하지 않았다. 관련 코드/테스트, 현재 에셋, 감사/저작 문서를 분리하여 검증 완료 상태를 보존한다. 에셋 복원은 별도 백업 및 Git 보존점을 이용한다.
