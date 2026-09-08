# Portfolio Capture Shot List

## 촬영 원칙

- 게임플레이 결과용 화면과 Debug Overlay 검증용 화면을 한 프레임에 섞지 않는다.
- 한 페이지에는 최대 3장의 캡처만 사용한다.
- 동일 조건 비교는 같은 맵, 같은 카메라 거리, 같은 대상 수를 유지한다.
- 캡처 파일을 교체하면 Evidence Capture Ledger와 Page Enrichment Backlog를 함께 갱신한다.

## 신규 캡처

| ID | 사용 페이지 | 장면 | 권장 비율 | 화면에 반드시 보여야 할 것 | 상태 |
| --- | ---: | --- | --- | --- | --- |
| C01 | 1 | 대표 전투 게임플레이 | 16:9 | Debug Overlay 없는 콤보·방어·회피 중 하나의 완결된 흐름 | Needs Capture |
| C02 | 4 | Player Target Lock 사례 | 16:9 + Overlay Crop 3장 | Hero에는 대상 마커·카메라 구도·대상 정면 이동을, 보조 캡처에는 획득·전환·해제 상황의 Target·평가 값·회전 상태를 표시 | Needs Capture |
| C03 | 5 | Combo / Hit 사례 | 16:9 + Overlay 또는 Event Log Crop 3장 | Hero에는 2~3타 콤보와 Enemy Hit React를, 보조 캡처에는 연계 예약·HitWindowId·피격 결과 수용 상태를 표시 | Needs Capture |
| C04 | 6 | Guard / Parry 게임플레이 비교 | 16:9 또는 3분할 | Debug Overlay 없이 Guard 자세·Parry 성공·Player Hit의 차이가 읽히는 장면 | Needs Capture |
| C04a | 6 | Guard / Parry 결과 데이터 | FinalCandidate Overlay 3장 (출처 전용) | Block Hit·Parry·Player Hit의 Outcome·Reaction·Final·Commit 가시 값을 p.6 텍스트 목업의 출처로 사용. 이미지를 직접 배치하거나 동일 공격 조건 비교로 주장하지 않음 | Evidence Ready |
| C05a | 7 | Dodge Hero | 16:9 | Debug Overlay 없는 Guard → Dodge 또는 PIE로 검증된 Action → Dodge 전환 | Needs Capture |
| C05b | 7 | Dodge decision runtime | 4:3 Crop 2~3장 | Incoming Dodge, Active, Decision, ApplyMode, RejectReason 중 실제로 보이는 값 | Needs Capture |
| C05c | 7 | Dodge during active Reaction | 4:3 Crop | 실제 결과가 Intervened 또는 Rejected 중 무엇인지 그대로 기록. 성공을 전제하지 않음 | Needs Capture |
| C06a | 8 | Balance / Collapse Runtime | Existing Crop 2장 | Stagger stack 2·3의 Count 누적과 Collapse 진입을 보조 증거로 사용 | Evidence Ready |
| C06b | 8 | Balance → Collapse Hero | 16:9 | Debug Overlay 없이 Parry 누적 또는 Collapse 상태 전환이 읽히는 장면 | Needs Capture |
| C06c | 9 | Execution session runtime | 4:3 Crop 2~4장 | SessionId·Partner Actor·Snapshot Revision·Reservation/Commit/Release 중 실제로 읽히는 값 | Needs Capture |
| C06d | 9 | Execution gameplay | 16:9 | Standard 또는 Lethal 중 실제 완료한 한 경로. Debug Overlay 없는 동기 Pair 장면 | Needs Capture |
| C06e | 9 | HP 1 Standard pre-check | Event Log 또는 Overlay Crop | AppliedDamage 0에 따른 예약 전 거절. 성공 실행 결과가 아닌 회귀 방지 계약 증거 | Needs Capture |
| C06f | 10 | Death Presentation Hero | 16:9 | Debug Overlay 없이 Dead 전이와 사망 표현이 읽히는 프레임 | Needs Capture |
| C06g | 10 | Dead lifecycle runtime | 4:3 Crop 2~3장 | Health Dead, Gameplay Focus, Rotation Mode, deferred sync 또는 finalize 흐름 중 실제 값을 분리해 표시 | Needs Capture |
| E11a | 11 | Action→Reaction transition | Overlay 또는 Event Log Crop | Candidate/Context, Decision, Relationship, ApplyMode, Directive와 terminal cleanup이 같은 전환에서 읽힘 | Needs Capture |
| E11b | 11 | Reaction→Reaction transition | Overlay 또는 Event Log Crop | 같은 공통 전환 계약이 재피격에서 읽힘 | Needs Capture |
| E12 | 12 | Intervention policy runtime | Overlay 또는 Event Log Crop | incoming Want, active Allow, timing gate, actual Intervene/Reject 결과 | Needs Capture |
| E13 | 13 | Source→Target trace | Overlay 또는 Log Crop 2장 | 동일 사건의 Source Context와 Target Packet 연결 | Needs Capture |
| E14 | 14 | Target Outcome comparison | Overlay Crop 3장 | 동일 계열 hit에서 Normal / Guard / Parry의 Accepted·Defense·Reaction·HP 전후 비교 | Needs Capture |
| E15a | 15 | Pair success | Overlay 또는 Event Log Crop | Reservation → Active → Commit → terminal | Needs Capture |
| E15b | 15 | Pair pre-check reject | Overlay 또는 Event Log Crop | HP 1 Standard 또는 조건 불충족의 Commit 전 거절 | Needs Capture |
| E15c | 15 | Pair pre-commit release | Overlay 또는 Event Log Crop | 취소 뒤 Reservation Release와 Loop TTL 복구 | Needs Capture |
| C07 | 19 | CSV Profiler 전후 비교 | 16:9 | 동일 AI 수·맵·측정 구간과 후보 수집 비용 비교 | Blocked: metric |
| C08 | 20 | Runtime Debug Overlay | 16:9 + 보조 crop | Enemy Current AI 상태, FocusComponent 기준, Event Log panel을 각 증거 범위 안에서 분리해 제시 | Existing evidence |
| C09 | 21 | Overlay Editor Plugin | 4:3 | Toolbar, Nomad Panel, 대상 선택, CVar 제어 | Existing evidence |
| C10 | 22 | Asset Inspector | 4:3 | Tree View, Dependencies/Referencers, CSV Export 중 핵심 흐름 | Existing evidence |
| C11 | 23 | Root Motion Tool | 16:9 또는 3분할 | 원본 Body Motion, Apply 결과, Revert 결과가 같은 Asset에서 비교 | Needs Capture |
| C12 | 24 | AI Workflow Artifact | 4:3 | Work Brief, 프롬프트 규칙, 검증 결과의 연결 | Existing documents |
| C13 | 25 | Change Trace Relation Diagram | 16:9 | D13 baseline과 B05 symptom이 P12·historical commit·current code contract로 이어지는 관계. 시간·역할을 직선 인과로 오독하지 않음 | Needs Capture |
| C14 | 25 | First-overlap Runtime Evidence | 16:9 또는 4:3 | 현재 빌드에서 유효 Hit Window ID·`InvalidRequest` 미발생·실행 대상이 함께 읽히는 화면 | Needs Capture |

## 기존 증거의 사용 범위

| 증거 | 우선 사용 페이지 | 주의 사항 |
| --- | --- | --- |
| DebugOverlay FinalCandidate Block Hit / Parry / Player Hit | 6 | 게임플레이 미관용이 아니라 판정·결과 검증용으로 사용 |
| DebugOverlay FinalCandidate AI 상태·이벤트 | 15, 19 | Behavior Tree 활성 노드 추적 기능으로 표현하지 않음 |
| DebugOverlay Editor Tooling | 20 | Editor-only 도구임을 표시 |
| Asset Reference Inspector Toolbar / Nomad Panel | 21 | 미사용 후보는 삭제 확정이 아님 |
| Stagger stack 2 / 3 | 8 | Execution 결과 캡처로 대체하지 않음 |
