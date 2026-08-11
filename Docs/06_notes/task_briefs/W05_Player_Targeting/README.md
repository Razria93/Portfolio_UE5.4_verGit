# W05 Player Targeting

플레이어가 전투 중 적을 선택하고 유지하는 락온 타게팅 기반을 기록한다.

| ID | 제목 | 파일 | 상태 |
| --- | --- | --- | --- |
| W05-01 | Player Targeting Component v1 | `TB_W05_01_Player_Targeting_Component_v1.md` | 구현 완료 (Destroy Lifecycle 연계 검증 이관) |
| W05-02 | Player Targeting Debug Observability v1 | `TB_W05_02_Player_Targeting_Debug_Observability_v1.md` | 완료 |
| W05-03 | Player Target Switching v1 | `TB_W05_03_Player_Target_Switching_v1.md` | 구현 완료 (Destroy Lifecycle 연계 검증 이관) |
| W05-04 | Player Target Lock Assist v1 | `TB_W05_04_Player_Target_Lock_Assist_v1.md` | 완료 |
| W05-05A | Player Target Marker v1 | `TB_W05_05A_Player_Target_Marker_v1.md` | 완료 |

## 구현 순서와 후속 경계

```text
W05-03: 좌우 타겟 전환
W05-04: 카메라 / 이동 락온 보정
W05-05A: 타겟 마커
후속: Character Destroy Lifecycle 연계 검증
별도 UI 작업: Enemy Status HUD
```

W05 타게팅 작업군은 W05-05A Target Marker까지 C++ 구현, Development 빌드와 핵심 PIE 검증을 완료했다. W05-01의 Destroy delegate 처리와 W05-03의 전환 후 구독 교체는 코드 계약으로 구현했으며, 실제 Actor Destroy 정책이 없는 현재 브랜치에서는 직접 Destroy, 사망 후 Destroy 중복 event, 이전 타겟 Destroy 경계를 재현하지 않는다. 이 항목들은 다음 Character Destroy Lifecycle 작업에서 실제 Destroy 경로를 만든 뒤 통합 검증한다. `MaxTargetAngleDegrees = 0` 경계는 안전한 계산 경로를 구현했으며 자동화 검증은 같은 후속 검증 묶음으로 이관한다. Enemy Status HUD는 Health / Balance 표현 책임을 포함하므로 별도 UI 후속 작업으로 분리한다.
