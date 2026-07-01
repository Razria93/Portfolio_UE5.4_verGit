# AI Update Interval Raw CSV Manifest

This folder stores the raw CSV profiler files used by `P33` and `N17`.

## Folder Layout

```text
baseline/
-> Case 01-05. Early baseline scale checks.

comparison/
-> Case 06-09. 60 Enemy condition comparisons.
-> Case 19. Blackboard dirty write guard comparison.

boundary/
-> Case 10-18. F11 fullscreen boundary scale checks.
```

## Files

| Case | File | Source Condition |
| --- | --- | --- |
| 01 | `baseline/case_01_001_enemy_idle_patrol.csv` | 1 Enemy / Idle Patrol |
| 02 | `baseline/case_02_001_enemy_engage.csv` | 1 Enemy / Engage |
| 03 | `baseline/case_03_010_enemy_engage.csv` | 10 Enemy / Engage |
| 04 | `baseline/case_04_020_enemy_engage.csv` | 20 Enemy / Engage |
| 05 | `baseline/case_05_040_enemy_dense_engage.csv` | 40 Enemy / Dense Engage |
| 06 | `comparison/case_06_060_enemy_dense_engage.csv` | 60 Enemy / Dense Engage |
| 07 | `comparison/case_07_060_enemy_logs_disabled.csv` | 60 Enemy / Logs Disabled |
| 08 | `comparison/case_08_060_enemy_distributed_patrol_engage.csv` | 60 Enemy / Distributed Patrol-Engage |
| 09 | `comparison/case_09_060_enemy_friendly_hit_disabled.csv` | 60 Enemy / Friendly Hit Disabled |
| 10 | `boundary/case_10_040_enemy_boundary_fullscreen.csv` | 40 Enemy / Boundary / F11 Fullscreen |
| 11 | `boundary/case_11_060_enemy_boundary_fullscreen.csv` | 60 Enemy / Boundary / F11 Fullscreen |
| 12 | `boundary/case_12_080_enemy_boundary_fullscreen.csv` | 80 Enemy / Boundary / F11 Fullscreen |
| 13 | `boundary/case_13_100_enemy_boundary_fullscreen.csv` | 100 Enemy / Boundary / F11 Fullscreen |
| 14 | `boundary/case_14_120_enemy_boundary_fullscreen.csv` | 120 Enemy / Boundary / F11 Fullscreen |
| 15 | `boundary/case_15_140_enemy_boundary_fullscreen.csv` | 140 Enemy / Boundary / F11 Fullscreen |
| 16 | `boundary/case_16_160_enemy_boundary_fullscreen.csv` | 160 Enemy / Boundary / F11 Fullscreen |
| 17 | `boundary/case_17_180_enemy_boundary_fullscreen.csv` | 180 Enemy / Boundary / F11 Fullscreen |
| 18 | `boundary/case_18_200_enemy_boundary_fullscreen.csv` | 200 Enemy / Boundary / F11 Fullscreen |
| 19 | `comparison/case_19_120_enemy_dirty_write_guard.csv` | 120 Enemy / Blackboard Dirty Write Guard |

## Excluded Local File

```text
Csvprofile/Profile(20260701_153725).csv
```

This file is kept out of the raw CSV archive because it was an intermediate measurement before the F11 fullscreen boundary condition was fixed.
