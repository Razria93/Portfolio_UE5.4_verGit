# 가드·TTL·AI 검증 근거 — 2026-10-21 정리

[대표 README](../../../README.md#problem-solving) · [가드 사례](../../03_Issue_Analysis_Report/I09_Guard_Cleanup_Case_Study.md) · [TTL 사례](../../03_Issue_Analysis_Report/I10_HitReactive_TTL_Case_Study.md) · [AI 사례](../../03_Issue_Analysis_Report/I11_AI_Perception_Case_Study.md)

PPT 제작 과정에서 확보한 기록 중 공개 설명에 필요한 CSV, 검사 결과, 테스트 소스, 로그 발췌를 모았습니다. 이 폴더는 **선별 근거**이며 전체 원시 프로젝트·실행 로그·영상 묶음은 아닙니다. 작성자 PC의 절대 경로와 기기 식별자는 공개본에서 제외했습니다. 결과를 재작성해 성공으로 바꾸지 않았으며 대조군 실패와 미해결 항목도 보존했습니다.

## 근거를 따라 읽기

| 사례 | 결과 | 더 확인할 자료 |
| --- | --- | --- |
| 가드 | [24조건 CSV](guard-24-conditions.csv), [최종·실패 대조 결과](guard-automation.json) | [경계 테스트 소스](GuardBoundaryTests.cpp), [수락 조건 대조 패치](guard-negative-control.patch), [실제 충돌 통합 이벤트](guard-integration.json), [Sprint 기대값 실패 기록](guard-sprint09-excerpt.json) |
| TTL | [시점별 Pending·Registry·Expiry](ttl-timeline.csv), [최종·결함 대조 결과](ttl-automation.json) | [경계 테스트 소스](TTLBoundaryTests.cpp), [합성 변이](ttl-negative-control.diff), [역사적 환경 재촬영 6회](ttl-historical-reshoot.json) |
| AI | [18회 CSV](ai-18-runs.csv), [상세 JSON](ai-18-runs.json), [중앙값·최솟값·최댓값](ai-aggregates.json) | 원시 results/conditions/summary/capture/runtime의 해시는 [출처 목록](source-manifest.json)의 각 run 이름으로 추적 |
| 영상 | [14개 최신 제작본 식별과 13개 기존 URL](video-versions.json) | [영상 공개 상태·번호 안내](../../07_Portfolio_Documents/Portfolio_Production/Video_Catalog.md) |

자동화 JSON은 UE의 최종 report에서 테스트 이름·상태·이벤트 메시지를 추출했습니다. 원본 파일 SHA-256을 함께 기록합니다. CSV는 가드·TTL 원본 값과 행 순서를 유지하며 UTF-8 BOM/줄바꿈만 정규화했습니다. AI CSV는 로컬 절대 경로 열을 제외한 공개용 요약입니다. 로그 발췌의 line은 원본 로그의 행 번호입니다. 원본 해시는 이 공개용 파생 파일 자체의 해시와 다릅니다.

## 검증 대상과 저장소 코드의 관계

가드·TTL 경계 검사는 **2026-10-21 로컬 Source 스냅샷**을 격리 복사해 실행한 결과입니다. 이 PR의 소스 기준은 35891fca9272669f2853448635c077f9ad0e3e9a이며 게임 소스는 변경하지 않았습니다.

[496개 소스 대조](source-alignment.json)에서 BOM·줄바꿈을 정규화한 뒤 **334개 일치, 162개 상이**를 확인했습니다. 후속 기능·튜닝·테스트가 로컬에 추가되어 있으므로 스냅샷 전체를 현재 PR의 실행 결과로 취급하지 않습니다. 로컬 원본은 기록된 스냅샷 해시와 전부 일치했습니다.

[10개 핵심 함수 대조](key-function-comparison.json)는 가드 수락·정리 3개와 TTL 등록·종료·전달·시리얼 검사 7개의 본문 및 양쪽 행 번호를 보존합니다. 이 함수들은 일치하지만 주변 시스템까지 검증된다는 의미는 아닙니다. 특히 TTL의 무효 WeakObject 조기 반환도 그대로 존재합니다.

AI는 2026-10-02의 Before/After 기록을 사용하고, TTL 재촬영은 10월 1일 보존 환경을 사용합니다. 이 둘을 10월 21일 경계 검사와 같은 실행으로 합치지 않습니다. [출처 SHA-256 목록](source-manifest.json)에는 원시 측정 파일·결과·영상 제작 기록의 출처를 패키지 상대 이름으로 남겼습니다.

## 재현 범위

가드 테스트는 실제 Defense/Movement/ObservableOverlay/Reaction_Hit 컴포넌트와 테스트 월드를 사용하며 생명주기 이벤트를 주입합니다. TTL은 합성 피격 패킷과 종료 이벤트로 실제 HitReactive→Participation→WorldSubsystem을 연결합니다. 조사 당시 UE 5.4.4 Development Editor, Content 제외, Entry 맵, NullRHI/NoSound 조건에서 실행했습니다. TTL 테스트는 사설 상태 관찰을 위한 friend FTTLBoundaryProbe 선언과 Editor 모듈 제외 등 **격리 fixture 설정**이 필요합니다. 여기의 소스를 현재 게임에 그대로 복사하면 즉시 같은 결과가 재현된다고 보장하지 않습니다.

가드 실제 충돌 통합 검증은 별도의 원본 BP·에셋 복사본에서 수행했습니다. Runtime 로그·프레임·오디오 신호 검사가 있고 실제 청취와 일반 수동 PIE 시각 동등성은 미확인입니다. 두 경계 검사에서 시작 시 HUD 리소스 오류가 발생했으므로 테스트 assertion 통과와 엔진 로그 전체 무오류를 구분합니다.

이번 문서 갱신에서 수행한 것은 원시 AI 18회 재집계, 14개 최신 영상 파일 해시 대조, 소스·검사 기록 비교, 문서 링크 검사입니다. Unreal 빌드·자동화·PIE를 새로 실행하지 않았습니다.

## 후속 확인

- TTL 파괴된 공격자의 Pending 제거 및 Unbind 순서 보완 후, 반복 파괴 Pending 8→0과 실제 Actor 종료 흐름 검증.
- 실제 입력을 유지한 Sprint 복원, 시야 이탈·재획득·사망·다중 타깃의 AI 기능 보존 확인.
- 최신 영상 01/02/03/09-B/12의 배포 주소 확보, 나머지 기존 URL과 최신 파일의 내용 대조. 현재 접근 가능 여부와 버전 일치를 별도 관리.
