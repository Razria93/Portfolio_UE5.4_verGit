# W05 Player Targeting

플레이어가 전투 중 적을 선택하고 유지하는 락온 타게팅 기반을 기록한다.

| ID | 제목 | 파일 | 상태 |
| --- | --- | --- | --- |
| W05-01 | Player Targeting Component v1 | `TB_W05_01_Player_Targeting_Component_v1.md` | 진행 (Destroy / 0도 경계 PIE 보류) |
| W05-02 | Player Targeting Debug Observability v1 | `TB_W05_02_Player_Targeting_Debug_Observability_v1.md` | 완료 |
| W05-03 | Player Target Switching v1 | `TB_W05_03_Player_Target_Switching_v1.md` | 계획 확정 |

## 확정 후속 순서

```text
W05-03: 좌우 타겟 전환
W05-04: 카메라 / 이동 락온 보정
W05-05: 타겟 마커와 Enemy Status HUD
```

W05 타게팅 작업군은 W05-05까지 순차 구현하고 검증한 뒤 다음 시스템 작업으로 이동한다. W05-03은 구현 정책을 확정했으며, W05-04와 W05-05는 해당 작업 시작 전에 별도 Task Brief로 세부 정책과 완료 조건을 고정한다.
