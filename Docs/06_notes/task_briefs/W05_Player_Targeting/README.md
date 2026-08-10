# W05 Player Targeting

플레이어가 전투 중 적을 선택하고 유지하는 락온 타게팅 기반을 기록한다.

| ID | 제목 | 파일 | 상태 |
| --- | --- | --- | --- |
| W05-01 | Player Targeting Component v1 | `TB_W05_01_Player_Targeting_Component_v1.md` | 진행 (Destroy / 0도 경계 PIE 보류) |
| W05-02 | Player Targeting Debug Observability v1 | `TB_W05_02_Player_Targeting_Debug_Observability_v1.md` | 완료 |
| W05-03 | Player Target Switching v1 | `TB_W05_03_Player_Target_Switching_v1.md` | 진행 (이전 타겟 Destroy PIE 보류) |
| W05-04 | Player Target Lock Assist v1 | `TB_W05_04_Player_Target_Lock_Assist_v1.md` | 완료 |

## 확정 후속 순서

```text
W05-03: 좌우 타겟 전환
W05-04: 카메라 / 이동 락온 보정
W05-05: 타겟 마커와 Enemy Status HUD
```

W05 타게팅 작업군은 W05-05까지 순차 구현하고 검증한 뒤 다음 시스템 작업으로 이동한다. W05-03은 C++ 구현과 Development 빌드, 좌우 전환 PIE 검증을 완료했으며 이전 타겟 Destroy 경계 검증만 남겨 두었다. W05-04는 C++ 구현, Development 빌드와 PIE 검증을 완료했다. W05-05는 Target Marker부터 분리 구현한다.
