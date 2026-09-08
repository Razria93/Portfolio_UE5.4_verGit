# AI Perception Candidate Audit — 2026-08-03

## 사용 범위

포트폴리오 p.19의 80 Enemy Before/After 비교 원본이다. 이 세트는 후보 수와 `FirstValidLatency`의 1회 비교를 뒷받침하며, 전체 FrameTime 개선 또는 반복 재현성의 근거가 아니다.

| 조건 | CSV / 로그 | Candidate Audit p95 | CSV p95 |
| --- | --- | --- | --- |
| Before | `Profile(20260803_143437).csv` / `Log(20260803_143437).txt` | Raw 81, Invalid 80, MaxTargetDataMap 81, FirstValidLatency 10.064s | FrameTime 22.6775ms, GameThread 22.6964ms, AIPerception 0.8292ms |
| After | `Profile(20260803_151028).csv` / `Log(20260803_151028).txt` | Raw 1, Invalid 0, MaxTargetDataMap 1, FirstValidLatency 1.070s | FrameTime 22.5861ms, GameThread 22.6251ms, AIPerception 0.1128ms |

## 분석 계약

- 조건: 80 Enemy, 동일 테스트 맵의 Before/After 실행.
- CSV: 총 약 37초 캡처에서 시작과 끝 각 3초를 제외하고 p95를 계산한다.
- 로그: 각 Enemy의 `[PerceptionCandidateAudit]` 행을 집계하고 p95를 계산한다.
- 재현: `Analyze_AI_Perception_Candidate_Audit.ps1`을 이 폴더에서 실행한다.
- 시각화: p.19의 막대 길이는 각 항목의 Before p95를 100으로 환산한 After 상대 지수다. 막대만으로 단위를 비교하지 않으며, 실제 p95 값도 함께 표기한다.

## 제한

- 각 조건은 1회 실행이다. 독립 3회 반복 전에는 수치 안정성·전체 성능 개선으로 일반화하지 않는다.
- `FrameTime`은 두 실행에서 거의 변하지 않았다. 이 자료의 중심 결과는 후보 구성과 유효 대상 확정 지연이다.
