# Portfolio Page Enrichment Backlog

## 목적

이 문서는 분석형 A4 와이어프레임의 하단 보조 영역을 페이지별 주장과 증거에 맞춰 채우기 위한 작업 백로그다.

하단 영역은 설명을 반복하는 곳이 아니다. 중심 시각 자료만으로 부족한 판단 근거를 보강하는 Support Evidence, Case Strip, Boundary Note 중 하나를 사용한다.

## 공통 완료 기준

- 중심 시각 자료와 하단 보조 영역이 서로 다른 정보를 제공한다.
- 구현·성능·검증 주장은 Evidence Ledger의 상태와 일치한다.
- 실제 증거가 아직 없으면 Placeholder에 필요한 캡처와 검증 조건을 명시한다.
- 하단 모듈은 25~35mm 안에 들어가며, 본문 20페이지 예산을 늘리지 않는다.
- 모든 페이지는 구조 설계·측정·검증·문서·기록 중 해당 페이지가 직접 증명하는 축을 우선 제시한다. 세 축을 같은 형식의 박스로 반복하지 않는다.

## 페이지별 작업

| p. | 페이지 | 하단 모듈 | 포함 내용 | 필요한 증거 | 상태 |
| ---: | --- | --- | --- | --- | --- |
| 1 | Cover | Focus + Representative Evidence | 구조 설계·측정·검증·문서·기록 3축, 일반 본문형 Focus 설명, 대표 근거, 외부 링크 | Hero 게임플레이, GitHub·Docs 링크, Gameplay Video URL | Needs Capture |
| 2 | Index | Reading Groups | 6개 읽기 그룹으로 페이지 흐름을 안내 | 없음 | Drafted |
| 3 | System Map | 전투 대상 관리 흐름 | Player 선택·AI 인지 → 교전 가능 여부 판단 → 전투 대상 정보 갱신 → Player·AI 활용 | 통합 전투 시스템 맵 + 전투 대상 데이터 흐름 다이어그램 | Drafted |
| 4 | Targeting | Runtime Data Cases + Design Resolution | 대상 획득·전환·해제 시 현재 대상·거리·Dot·점수·회전 상태 변화, 선택과 적용의 책임 분리 | Lock Hero 1장 + 획득·전환·해제 Overlay Crop 3장 | Needs Capture |
| 5 | Combo / Hit | Runtime Data Cases + B05 Resolution | 연계 예약·Hit Window 진입·피격 결과 수용 상태, 첫 overlap 전에 Hit Window를 준비하는 순서 보정 | Combo Hero 1장 + 연계·판정·결과 Overlay 또는 Event Log Crop 3장 | Needs Capture |
| 6 | Guard / Parry | Gameplay Flow + Runtime Validation Mockup | Guard·Parry·일반 피격 게임플레이 흐름, Final·Commit·Reaction 필드 목업, Guard Out 중 Hit reaction 시작 실패의 B12 해결 | 비교 Hero 1장 또는 3분할 + C04a 결과 필드 출처 + B12 | Needs Capture |
| 7 | Dodge Intervention | Gameplay Flow + Runtime Decision Cases | Guard → Dodge 또는 검증된 Action → Dodge, incoming/active의 Decision·ApplyMode·RejectReason, Want/Allow 책임 분리 | Hero 1장 + Runtime Crop 2~3장 + B09 | Needs Capture |
| 8 | Balance / Collapse | Gameplay Hero + Runtime Validation + State Authority | Debug Overlay 없는 Parry 누적 → Collapse Hero, Count·State·Serial·TTL·Reset 검증 | C06b Hero + Count/Threshold 보조 근거 + 전이 Crop 3장 | Drafted: transition captures needed |
| 9 | Execution Collaboration | Gameplay Hero + Runtime Validation + Pre-check Contract | Debug Overlay 없는 Execution Pair Hero, Reservation·Commit/Release 검증, HP 1 Standard 사전 거절 | C06d Hero + Session Crop 2~4장 + C06e HP 1 pre-check log | Drafted: runtime captures needed |
| 10 | Shared Execution | Responsibility Compare | 분산된 주체별 실행과 공통 요청·실행 구조 비교 | 재작성 책임 다이어그램 | Planned |
| 11 | Intervention Policy | Policy Matrix | 현재 실행·새 요청·결정 결과 3개 사례 | 정책 표·코드 대조 | Planned |
| 12 | Combat Signal | Data Card | Hit Context의 핵심 데이터와 전달 단계 | 재작성 데이터 흐름도 | Planned |
| 13 | Data-Driven Resolve | Variation Table | 공통 계층과 데이터로 바뀌는 지점 비교 | Data Resolve 다이어그램 | Planned |
| 14 | AI Intent | Decision Trace | Patrol → Alert → Engage의 판단·요청 사례 | AI 상태·이벤트 캡처 | Evidence Ready |
| 15 | Combat Target | Lifecycle Strip | 전투 대상 획득 → 유효성 상실 → 해제 | 전투 대상 상태 다이어그램 | Planned |
| 16 | Participation | Participation Timeline | AI 3기의 참여 요청 → 승인 → 해제 | 재작성 타임라인 | Planned |
| 17 | Profiling | Measurement Evidence | 전후 CSV 구간, 조건, 대표값 기준 | CSV 전후 캡처·정식 측정값 | Blocked: metric |
| 18 | Runtime Debug | Runtime Observation Map | Runtime Source → Snapshot/ViewData → Canvas Overlay와 CVar → World DrawDebug의 분리 | Current AI + NearestFocus·EventLog FinalCandidate | Drafted |
| 19 | Overlay Editor Plugin | Editor Bridge Map | Toolbar / Menu → Nomad Panel → CVar·Focus command bridge | Nomad·Toolbar·Outliner FinalCandidate | Drafted |
| 20 | Asset Inspector | Investigation Flow + Schema | Selected Asset → Registry → Tree → Sync/CSV, code-derived export fields | Nomad evidence + CSV exporter schema | Drafted |
| 21 | Root Motion Tool | Apply / Revert Safety Contract | 검증 → backup → transfer/compensate → backup 기반 revert | Source·Apply·Revert 신규 캡처 | Drafted: captures needed |
| 22 | AI Workflow | Human-controlled Workflow + Same-work Evidence | Brief → 조사 → AI 초안 → 개발자 검토 → 검증 기록 | 동일 식별자의 Work Brief·Review·Verification 신규 캡처 | Drafted: linked evidence capture needed |
| 23 | Traceability | B05 Change Trace | Bug Record B05 → Checklist D13 → P12 → 현재 코드·검증 기록 → Commit Record | B05·P12 연결 문서 발췌 및 first-overlap runtime 신규 캡처 | Drafted: runtime capture needed |

## 제작 순서

1. Analytical A4 Wireframe의 24페이지 구조·주장·A4 페이지 수 검수를 유지한다.
2. p.4~p.10과 p.22의 신규 게임플레이·애니메이션 캡처를 확보한다.
3. p.18 raw CSV와 p.23·p.24의 연결 문서·런타임 증거를 확보한다.
4. 모든 Placeholder를 실제 증거로 교체한 뒤 Review Checklist로 제출 PDF를 검수한다.
