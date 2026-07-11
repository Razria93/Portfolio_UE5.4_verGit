# AI Update Interval CSV Evidence Manifest

이 manifest는 P33 / N17에 기록된 요약 결과가 어떤 측정 의도를 바탕으로 했는지 추적할 수 있도록 원본 case 이름과 측정 조건만 남긴다.

## Source Cases

| Case | 원본 캡처명 | 측정 조건 |
| --- | --- | --- |
| 01 | `case_01_001_enemy_idle_patrol.csv` | 1 Enemy / Idle Patrol |
| 02 | `case_02_001_enemy_engage.csv` | 1 Enemy / Engage |
| 03 | `case_03_010_enemy_engage.csv` | 10 Enemy / Engage |
| 04 | `case_04_020_enemy_engage.csv` | 20 Enemy / Engage |
| 05 | `case_05_040_enemy_dense_engage.csv` | 40 Enemy / Dense Engage |
| 06 | `case_06_060_enemy_dense_engage.csv` | 60 Enemy / Dense Engage |
| 07 | `case_07_060_enemy_logs_disabled.csv` | 60 Enemy / Logs Disabled |
| 08 | `case_08_060_enemy_distributed_patrol_engage.csv` | 60 Enemy / Distributed Patrol-Engage |
| 09 | `case_09_060_enemy_friendly_hit_disabled.csv` | 60 Enemy / Friendly Hit Disabled |
| 10 | `case_10_040_enemy_boundary_fullscreen.csv` | 40 Enemy / Boundary / F11 Fullscreen |
| 11 | `case_11_060_enemy_boundary_fullscreen.csv` | 60 Enemy / Boundary / F11 Fullscreen |
| 12 | `case_12_080_enemy_boundary_fullscreen.csv` | 80 Enemy / Boundary / F11 Fullscreen |
| 13 | `case_13_100_enemy_boundary_fullscreen.csv` | 100 Enemy / Boundary / F11 Fullscreen |
| 14 | `case_14_120_enemy_boundary_fullscreen.csv` | 120 Enemy / Boundary / F11 Fullscreen |
| 15 | `case_15_140_enemy_boundary_fullscreen.csv` | 140 Enemy / Boundary / F11 Fullscreen |
| 16 | `case_16_160_enemy_boundary_fullscreen.csv` | 160 Enemy / Boundary / F11 Fullscreen |
| 17 | `case_17_180_enemy_boundary_fullscreen.csv` | 180 Enemy / Boundary / F11 Fullscreen |
| 18 | `case_18_200_enemy_boundary_fullscreen.csv` | 200 Enemy / Boundary / F11 Fullscreen |
| 19 | `case_19_120_enemy_dirty_write_guard.csv` | 120 Enemy / Blackboard Dirty Write Guard |

## 제외한 로컬 캡처

```text
Csvprofile/Profile(20260701_153725).csv
```

이 파일은 F11 fullscreen boundary 조건을 고정하기 전의 중간 측정이므로 evidence 목록에서 제외한다.
