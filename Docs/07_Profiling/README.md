# 프로파일링 문서

## 목적

이 폴더는 프로파일링 계획, 분석 노트, 측정 결과 요약을 보관한다.
raw CSV / log 캡처 파일은 이 폴더에 저장하지 않는다.

raw 캡처는 로컬 전용 폴더에 보관한다.

```text
Csvprofile/
Saved/Profiling/CSV/
```

나중에 특정 측정을 다시 추적해야 한다면, 해석된 수치는 관련 노트에 남기고 원본 캡처 ID는 evidence manifest에 기록한다.

## 구조

```text
Docs/07_Profiling/
|-- CSV_Analysis_Guide.md
|-- AI_Update_Interval/
|   |-- README.md
|   `-- CSV_Evidence_Manifest.md
`-- AI_Performance/
    |-- README.md
    |-- CSV_Evidence_Manifest.md
    `-- Runtime_LOD/
        `-- README.md
```

## 원본 근거 정책

- 측정 계획, 결과 표, 해석 노트, PR 요약 문서는 커밋한다.
- `.csv`, `.log`, `.log.txt` raw 캡처 파일은 이 폴더에 커밋하지 않는다.
- raw 캡처 파일명은 측정 근거 식별이 필요할 때만 manifest에 기록한다.
- raw 파일 보관보다 p95 중심 요약과 명시적인 측정 조건 기록을 우선한다.
