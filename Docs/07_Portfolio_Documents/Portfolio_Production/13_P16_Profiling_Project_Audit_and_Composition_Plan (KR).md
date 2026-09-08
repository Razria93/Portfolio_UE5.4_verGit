# 문서상 p.19 Profiling 프로젝트 감사 및 구성 계획

> 파일명은 초기 22페이지 체계의 감사 기록을 보존한다. 현행 25페이지 체계에서는 p.19에 적용한다.

> 상태: **코드·원본 CSV·감사 로그 대조 완료 / 80 Enemy 1회 Before/After 수치 동결 / 반복 측정 보완 예정**

## 확인 근거와 주장 범위

- p.19의 대표 원본은 `Docs/98_Evidence/Profiling/AI_Perception_Candidate_Audit_20260803/`에 고정한다. 이전 PA05·PA09 문서 기록은 분석 이력으로만 보존하며, 페이지 대표 수치로 사용하지 않는다.
- Before `Profile(20260803_143437).csv`와 동명 로그는 80 Enemy에서 RawActors p95 81, InvalidProviders p95 80, MaxTargetDataMap p95 81, FirstValidLatency p95 10.064s를 기록한다.
- After `Profile(20260803_151028).csv`와 동명 로그는 RawActors p95 1, InvalidProviders p95 0, MaxTargetDataMap p95 1, FirstValidLatency p95 1.070s를 기록한다.
- 같은 CSV 분석 계약으로 AIPerception p95는 0.8292ms→0.1128ms다. FrameTime p95는 22.6775ms→22.5861ms로 거의 동일하다.
- 측정 계약은 동일 80 Enemy 테스트, 약 37초 캡처, 시작·끝 각 3초 제외, CSV와 Enemy별 Candidate Audit 로그의 p95 비교다. 분석 경로는 같은 Evidence 폴더의 `Analyze_AI_Perception_Candidate_Audit.ps1`로 고정한다.

## 페이지 구성

1. **문제 제기와 측정 설계**: 유효 Player와 무효 Enemy가 같은 후보 집합에 들어오는 관찰 문제를 먼저 두고, 80 Enemy·동일 테스트 맵·FirstValidLatency p95·Team Attitude / Affiliation 변경 범위를 고정한다.
2. **중심 비교 그래프**: `Before p95 = 100` 정규화 가로 막대그래프로 First Valid Target, AIPerception, AI Context Update, BT Tick, FrameTime, GameThreadTime, GPUTime을 함께 비교한다. 각 행에는 원 단위 p95도 병기한다.
3. **후보 필터 적용 패널**: 그래프 오른쪽에서 Before Raw/Invalid/Map 81/80/81, Enemy Friendly·Player Hostile의 Team Attitude / Affiliation 적용, After 1/0/1을 세 단계로 읽게 한다.
4. **측정 계약·제한**: 하단 3카드에서 측정 환경, 원본·산출, 해석 범위를 분리한다. 약 37초/처음·마지막 3초 제외/p95 조건, 1회 비교 및 반복 3회 보완 필요를 명시하며 Frame/Game/GPU가 거의 동일한 사실로 전체 성능 개선 과장을 막는다.

## 표현 제한

- `10.064s → 1.070s`는 80 Enemy의 1회 Before/After 실행을 나타낸다. 반복 측정의 평균·분산 또는 일반적 성능 개선으로 표현하지 않는다.
- 이 수치는 후보 수·유효 전투 대상 확정 지연과 AIPerception p95의 관찰값이다. AIPerception 엔진 전체 비용 또는 전체 FrameTime 개선으로 표현하지 않는다.
- FrameTime p95는 거의 동일하므로 중심 그래프·제목에 성과 수치로 사용하지 않는다.
