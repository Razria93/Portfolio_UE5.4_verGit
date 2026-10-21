# AI Perception 개선 — 대상 필터링과 18회 반복 측정

[대표 README](../../README.md#problem-solving) · [사례 목록](00_Issue_Analysis_Report_Index.md) · [영상 13과 공개 상태](../07_Portfolio_Documents/Portfolio_Production/Video_Catalog.md#video-13)

**핵심:** AI가 처리하는 인지 대상을 줄여 첫 플레이어 인지 지연과 AI Perception 비용을 낮췄습니다. 40/80/120개체에서 전후 각 3회, 총 18회를 측정했습니다. 전체 프레임 시간은 별도로 공개합니다.

## 원인과 구현 판단

인지된 Actor가 모두 전투 타깃 후보인 것은 아닙니다. 타깃으로 사용할 수 없는 대상이 들어오는 경로를 관찰하고, C++ Team Attitude 판정과 Blueprint의 Sight DetectionByAffiliation 설정을 함께 조정했습니다. 뒤에서 불필요한 대상을 걸러내는 비용뿐 아니라 인지 단계의 유입 자체를 줄이는 방향입니다.

현재 저장소에서는 [CAIController](../../Source/Portfolio/Controller/CAIController.cpp)의 GetTeamAttitudeTowards와 ConfigureSightConfig를 따라가며 적대 관계 판정 및 Sight 설정을 읽을 수 있습니다. 성능 수치는 아래 역사적 측정 환경에 한정되며 현재 브랜치에서 다시 측정한 결과는 아닙니다.

## 측정 조건과 결과

| 조건 | 기록 |
| --- | --- |
| 측정일 / 재검토일 | 2026-10-02 / 2026-10-21 |
| 실행 환경 | UE 5.4.4 · Development · Windows 11 · Ryzen 9 7950X3D · RTX 4070 Ti · D3D12 |
| 화면 / 구간 | 1600×900 · 60 FPS 제한 · VSync 0 · 실행당 약 40초 · NullRHI 사용 안 함 |
| 통제 | 공통 고정 카메라, HUD·역할 표기·카메라 흔들림 비활성. 같은 규모의 전후 배치·스폰 순서 일치 |
| 120개체 구성 | 80Enemy 맵에 런타임 적 40개 추가. 원래 120개체 전용 맵으로 오인하지 않음 |
| 비교 소스 기록 | Before c9088f0f1fb7cdd26b118ca3dc09f7cd4c9bc884 / After 42aaef31db521218a399fc2eb0a55f31676cf56e · 공통 계측 코드 |

표는 **각 실행의 p95를 계산한 뒤 3회 중앙값**을 취했습니다. 3회의 원시 표본을 합친 p95가 아닙니다.

| 적 수 | 첫 유효 인지 p95 (초) 전→후 | AI Perception p95 (ms) 전→후 | FrameTime p95 (ms) 전→후 |
| --- | --- | --- | --- |
| 40 | 5.060 → 0.744 | 0.2915 → 0.0517 | 17.6966 → 17.7116 |
| 80 | 9.638 → 0.908 | 0.7695 → 0.0859 | 17.5269 → 17.3630 |
| 120 | **20.816 → 1.053** | **1.2582 → 0.1199** | **20.7365 → 21.1009** |

120개체 조건에서 첫 인지 지연은 약 **94.9%**, AI Perception 비용은 약 **90.5% 감소**했습니다. FrameTime은 개선되지 않았으므로 “전체 FPS 90% 개선”이라는 표현은 사용하지 않습니다.

- 첫 유효 인지: 개별 AI의 계측 초기화부터 첫 성공 플레이어 인지 콜백까지의 시간, 개체별 값의 p95.
- 처리 비용: PerceptionCaptureActive=1인 프레임 중 측정 구간 앞뒤 3초를 제외한 AI Perception / FrameTime의 p95.
- p95 계산: nearest-rank, 정렬 후 ceil(0.95×N)번째 값.
- 120개체 첫 인지 p95의 실행별 범위: 전 20.7999–20.8251초, 후 1.0486–1.0671초. [모든 규모의 중앙값·범위](../98_Evidence/Combat_Case_Studies_20261021/ai-aggregates.json).

[18회 결과 CSV](../98_Evidence/Combat_Case_Studies_20261021/ai-18-runs.csv) · [상세 측정값·조건](../98_Evidence/Combat_Case_Studies_20261021/ai-18-runs.json) · [원시 파일 해시와 검토 범위](../98_Evidence/Combat_Case_Studies_20261021/README.md)

이번 문서 검토에서 18개 실행의 results.json과 capture.csv를 다시 계산해 기존 summary.json의 지연·Perception·FrameTime 및 사용 프레임 수와 일치함을 확인했습니다. 엔진을 새로 실행한 측정은 아닙니다. 녹화 OFF는 당시 측정 기록의 조건이며 실행 프로세스 목록을 사후 검증했다는 뜻은 아닙니다.

## 기능 보존과 주장 한계

18회 모두 예정된 40/80/120개체가 플레이어 인지를 완료했습니다. 다만 시작부터 시야·거리 안에 있는 단일 플레이어를 보는 fixture이므로, 시야 이탈·재획득·사망·다중 타깃·장시간 플레이까지 보장하지 않습니다. SightLossAndReacquisition 테스트 소스가 있어도 이번 자료에는 실행 로그가 없으며, 상태 조작 기반 검사와 실제 센서 통합 검사도 구분해야 합니다.

120개체의 raw actors p95 대표값은 111→1, invalid_providers는 110→0이었습니다. 후자는 ITargetContextProvider 미구현 대상 수입니다. 이를 일반적인 아군·적군 수로 바꾸어 설명하지 않습니다. 최대 타깃 맵 관찰값은 1이었습니다.

[기존 공개 설명 영상 13](https://youtu.be/fGTAndeuBo8)은 약 74초이며 URL 접근을 확인했습니다. 영상·이전 웹의 단회 수치와 이 문서의 18회 중앙값은 표본이 다를 수 있습니다. 외부 영상과 최신 로컬 파일의 내용 일치는 미확인으로 두고, 성능 주장에는 위 표와 측정 기록을 사용합니다.
