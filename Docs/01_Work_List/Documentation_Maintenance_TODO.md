# Documentation Maintenance TODO

> 목적: 제출 이후 문서의 내용 보완·증거 보강·조회성 유지 작업을 한 곳에서 추적한다.
> 기준: 이 목록은 구현 완료를 주장하지 않는다. 상태 변경은 코드·캡처·로그 또는 문서 검토 근거를 확인한 뒤에만 수행한다.

## P0 — Submission status synchronization

- [ ] `Portfolio_Evidence_Capture_Ledger`와 `00_Portfolio_Page_Spec_Index`의 제출 상태를 23페이지 기준으로 재판정한다.
  - 상태: Needs Review
  - 대상: `Docs/98_Evidence/Portfolio_Evidence_Capture_Ledger (KR).md`, `Docs/07_Portfolio_Documents/Portfolio_Production/00_Portfolio_Page_Spec_Index (KR).md`
  - 근거: 제출 HTML에는 p.1~p.23의 캡처·다이어그램이 삽입됐으나 기존 계획은 25페이지·다수 `Needs Capture` 기준을 유지한다.
  - 다음 작업: 각 페이지를 `Ready` / `Partial` / `Needs Capture`로 재분류하고 p.24·p.25는 `Submitted scope 제외`로 명시한다.

- [ ] 제출 원본·PDF·정적 배포본의 생성 관계를 매 배포 시 기록한다.
  - 상태: Needs Review
  - 대상: `Docs/07_Portfolio_Documents/Portfolio_Production/README.md`
  - 근거: PDF와 Pages 패키지는 HTML에서 생성되는 산출물이다.
  - 다음 작업: 배포 시 source commit, 생성일, URL 또는 release 위치를 기록한다.

## P1 — Current portfolio evidence

- [ ] 대표 Gameplay Video를 Issue #119에 업로드하고 Evidence Ledger를 갱신한다.
  - 상태: Needs Capture
  - 대상: p.1 Cover, `Docs/98_Evidence/Portfolio_Evidence_Capture_Ledger (KR).md`
  - 근거: Gameplay Video 링크는 준비됐지만 20~30초 HUD 없는 대표 플레이 영상은 아직 확정되지 않았다.
  - 다음 작업: clean gameplay 영상을 업로드하고 Issue 링크·상태를 `Ready`로 갱신한다.

- [ ] p.4~p.10의 기존 캡처가 각 페이지의 핵심 주장을 실제로 증명하는지 재검토한다.
  - 상태: Needs Review
  - 대상: p.4 Targeting ~ p.10 Death Lifecycle
  - 근거: 제출 HTML에는 이미 이미지가 있으나 Evidence Ledger는 일부를 계속 `Needs Capture`로 기록한다.
  - 다음 작업: 획득/전환/해제, Combo Hit Window, Guard·Parry, Dodge, Collapse, Execution, Dead lifecycle 중 실제로 읽히지 않는 항목만 신규 캡처로 분리한다.

- [ ] p.20 Runtime Debug의 증거 범위를 확정한다.
  - 상태: Needs Decision
  - 대상: p.20, `14_P17_Runtime_Debug_Project_Audit_and_Composition_Plan (KR).md`
  - 근거: 제출 화면은 Focused Enemy의 상태·Facing을 보여 주지만, 과거 계획은 같은 화면의 Event Log 대조까지 요구한다.
  - 다음 작업: 현재 범위를 상태·Facing으로 고정하거나, Event Log 동시 화면을 추가 캡처한다.

- [ ] p.22 Asset Inspector의 CSV export 결과 증거를 확보하거나 표현 범위를 제한한다.
  - 상태: Needs Capture
  - 대상: p.22, `16_P19_Asset_Inspector_Project_Audit_and_Composition_Plan (KR).md`
  - 근거: Selected Asset·Tree는 보이지만 timestamped CSV 생성 결과는 화면에서 확인되지 않는다.
  - 다음 작업: export 파일·경로가 보이는 캡처를 추가하거나 문구를 `export 기능 제공`으로 한정한다.

- [ ] p.23 Root Motion의 Revert 동등성 증거를 분리한다.
  - 상태: Needs Capture
  - 대상: p.23, `17_P20_Root_Motion_Tool_Project_Audit_and_Composition_Plan (KR).md`
  - 근거: 현재는 Apply 전후 포즈 비교만 있으며 Before→Apply→Revert 3분할 증거는 없다.
  - 다음 작업: 동일 복제 Asset의 경로·Root/Source bone·track 정보를 보이는 3분할 캡처를 확보하거나 Revert 성공 주장을 보류한다.

## P2 — Runtime and measurement evidence

- [ ] p.19 Profiling의 반복 측정 정책을 확정한다.
  - 상태: Needs Review
  - 대상: p.19, `13_P16_Profiling_Project_Audit_and_Composition_Plan (KR).md`
  - 근거: 현재 80 Enemy Before/After는 1회 사례이며 반복 측정은 보완 예정이다.
  - 다음 작업: 동일 조건 3회와 중앙값·범위를 추가하거나 1회 사례라는 제한을 명시한다.

- [ ] p.11~p.15 공통 실행·Signal·Pair의 최소 런타임 trace를 확보한다.
  - 상태: Needs Capture
  - 대상: `S37_UE5_Portfolio_Execution_Transition_and_Intervention_Architecture.md`, `S38_UE5_Portfolio_Combat_Signal_Architecture.md`, `S36_UE5_Portfolio_Execution_Collaboration_Architecture.md`
  - 근거: Decision/Relationship/ApplyMode, Source→Target Packet, Pair Reserve→Commit/Release는 코드·문서 확인 상태이며 제출용 런타임 증거가 부족하다.
  - 다음 작업: 동일 사건의 상관 정보가 읽히는 최소 trace를 캡처하고, historical bug record와 현재 실행 증거를 분리한다.

- [x] p.21 PIE Focus Command Bridge의 실사용 증거 필요 여부를 판단한다.
  - 코드 점검: 2026-09-08에 Panel → PIE ConsoleCommand → PlayerController Exec → FocusComponent → HUD/Event Log 경로와 session-only CVar 경로를 확인했다. 런타임 화면 증거만 미확인 상태다.
  - 런타임 검증: 2026-09-08 TestRoom headless 실행에서 `GM_DebugOverlay_C` 로드 후 `DebugOverlaySelectNearestFocus`가 `BP_CEnemy_C_1`을 선택했다. `EventLog.Visible`을 0으로 설정한 프로세스를 종료한 뒤 새 프로세스에서 기본값 1로 복귀하는 것도 확인했다.
  - 상태: Verified
  - 대상: p.21, `15_P18_Overlay_Editor_Plugin_Project_Audit_and_Composition_Plan (KR).md`
  - 근거: 현재 캡처는 session CVar 설정 UI이며 Focus Action 뒤 Runtime actor 갱신은 보이지 않는다.
  - 검증 결과: TestRoom PIE에서 Event Log 표시를 끄면 Character Details·World Summary는 유지되고, Nearest Focus 및 Player Target Focus 요청 뒤 FocusDriver·RuntimeFocusSource·FocusActor가 갱신되는 것을 확인했다. 에디터를 완전히 재시작한 뒤 HUD·Capture는 Off, Event Log Visible은 On 기본값으로 복귀했다.
  - 결정: 현재 p.21의 Panel 설정 화면과 p.20의 Runtime Overlay 실사례로 범위를 설명한다. Focus Action의 별도 제출 캡처는 추가하지 않는다.

## P3 — Deferred scope and document hygiene

- [ ] p.24 AI Workflow와 p.25 Traceability의 재개 여부를 결정한다.
  - 상태: Deferred
  - 대상: 제출 HTML의 숨김 초안, Evidence Ledger rows 24·25
  - 근거: 두 페이지는 제출본에서 제외됐으며 placeholder와 미확보 evidence를 포함한다.
  - 다음 작업: 재개 시 Work ID board, traceability diagram, current first-overlap capture를 확보한다. 재개 전에는 제출 범위에 포함하지 않는다.

- [ ] 활성 문서의 환경 종속 절대 경로를 점진적으로 상대 경로·환경 변수·external source 표기로 바꾼다.
  - 상태: Needs Review
  - 대상: Debug Overlay verification, build work plans, notes의 로컬 경로 16건
  - 근거: 다른 PC·외부 조회 환경에서 원본 경로를 해석할 수 없다.
  - 다음 작업: 실행 명령은 `$env:UE_ROOT` 등 환경 변수 예시로, 개인 Bandicam 원본은 저장소 캡처 또는 `external source`로 전환한다.

- [ ] `Docs/ignore/`를 비정식 보관 영역으로 유지한다.
  - 상태: Deferred
  - 대상: `Docs/ignore/**`
  - 근거: 보존 초안에 깨진 링크가 있으나 활성 문서 경로가 아니다.
  - 다음 작업: 삭제 대신 최상위 인덱스의 archive 표기와 함께 유지하며, 필요 시 별도 archive 정리 작업으로 분리한다.
