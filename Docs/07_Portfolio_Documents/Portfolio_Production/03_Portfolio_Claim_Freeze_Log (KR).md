# Portfolio Claim Freeze Log

## 목적

제출 문구와 캡처가 서로 다른 주장을 하지 않도록, 제출 전 확정해야 할 용어·수치·구현 범위를 관리한다.

## 확정 전 항목

| 항목 | 현재 사용 원칙 | 확정에 필요한 근거 | 현재 상태 |
| --- | --- | --- | --- |
| 프로젝트 표기 | Project Stellar를 기본 표기로 사용 | 문서·저장소·표지 전체 대조 | Confirmed |
| 성능 수치 | 80 Enemy 1회 Before/After에서 First Valid Target p95 `10.064s → 1.070s`를 대표 수치로 사용하고, 후보 지표 `81/80/81 → 1/0/1`과 측정 조건을 함께 표기 | `AI_Perception_Candidate_Audit_20260803` CSV·로그·분석 스크립트, 동일 테스트 조건, 반복 횟수 | Scope Limited: 원본 확인 / 3회 반복 보완 예정 |
| Profiling 주장 | Team Attitude / Affiliation 적용 뒤 후보 단계의 무효 Enemy가 제거되고 유효 대상 확정 지연이 감소한 1회 비교로 표현한다 | 변경 코드와 80 Enemy Before/After CSV·로그 | Evidence Ready: 전체 FrameTime 개선·일반 재현성 주장 금지 |
| Balance / Collapse | Count·상태·TTL·Reset의 수명주기 구조로만 표현 | 상태 전이·TTL·Reset의 Runtime capture | Needs Capture |
| Execution | 구조와 기본 구현 범위로만 표현 | 제출용 캡처·실행 검증 | Scope Limited |
| Execution Transition | Action/Reaction은 별도 도메인 실행이지만 공통 판단·전환 계약으로 Intervention을 처리한다 | E11a·E11b, S37 | Needs Capture |
| Intervention Policy | 일반 Exclusive 개입은 incoming Want·active Allow·필요한 timing gate로 결정한다 | E12, B09, S37 | Needs Capture |
| Combat Signal Source | Source는 전투 사실을 전달하며 Target의 Defense·Reaction Outcome을 직접 결정하지 않는다 | E13, S38 | Needs Capture |
| Target Outcome Resolution | Target은 Packet·자기 상태·방어 규칙으로 Outcome을 확정한다 | E14, S38 | Needs Capture |
| Execution Pair Coordination | Pair는 양측 조건·reservation·Active·Commit·Release를 관리한다 | E15a~c, S36 | Needs Capture |
| Death Lifecycle | Dead 이후 Presentation·Facing 억제·Finalize·Destroy를 분리된 수명주기로 표현 | Dead Presentation, Facing Suppressed, Finalize 경로 Runtime capture | Needs Capture |
| Root Motion Tool | raw track 수정 전 Preflight·backup을 수행하고, Root·direct root child 범위에서 Apply/Revert를 지원하는 Editor 작업으로 표현 | 동일 복제 Asset Before·Apply·Revert 캡처, `CAnimModifier_TransferBodyMotionToRoot` 코드 | Needs Capture |
| Runtime Debug | Focus된 Enemy의 현재 상태와 최근 이벤트를 같은 Runtime 화면에서 대조하는 개발용 read-only 관찰 도구로 표현 | Current AI FinalCandidate, Focused Enemy Character Details + Event Log 통합 캡처, 기능 범위 대조 | Needs Capture |
| Overlay Editor Plugin | Editor-only 도구로 표현 | Toolbar·Panel·CVar 캡처 | Evidence Ready |
| Asset Inspector | 선택 Asset의 참조 조사·기록 도구로 표현하며, Unused Candidate를 삭제 판단으로 표현하지 않음 | README·현재 코드, Selected Asset·Tree·CSV 실사용 캡처 | Needs Capture |
| AI Workflow | AI 제안·초안은 Work Brief·개발자 수용 판단·검증 기록 뒤에만 반영하고, 미수행 검증은 성공으로 바꾸지 않고 남긴다 | 동일 Work ID의 Work Brief·Review / Decision·Verification Record | Needs Capture |
| Change Traceability | B05 문제 기록·D13 선행 기준·P12 변경 기록·과거 commit·현재 코드의 역할을 대조하며, 과거 `CAttachment`와 현재 `ACWeaponActor`의 클래스 동일성 및 현행 runtime 성공을 주장하지 않는다 | B05·P12 발췌, D13 경로·날짜, commit `746ea548`, 현재 코드, first-overlap runtime capture | Needs Capture |

## 제출 직전 점검

1. 이 문서의 Blocked와 Needs Review 항목이 Final 문구에 완료 결과로 쓰이지 않는지 확인한다.
2. 성능 수치가 자기소개서, 포트폴리오, CSV 증거에서 같은 조건·단위·대표값을 쓰는지 확인한다.
3. 페이지 제목, 캡처 캡션, GitHub 문서 링크에서 Project Stellar 표기가 일치하는지 확인한다.
