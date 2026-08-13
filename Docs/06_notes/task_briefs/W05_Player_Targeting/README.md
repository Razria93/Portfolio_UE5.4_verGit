# W05 Player Targeting

플레이어가 전투 중 적을 선택하고 유지하는 락온 타게팅 기반 작업을 기록한다.

| ID | 제목 | 파일 | 상태 |
| --- | --- | --- | --- |
| W05-01 | Player Targeting Component v1 | `TB_W05_01_Player_Targeting_Component_v1.md` | 완료 (W06 Destroy 통합 검증 포함) |
| W05-02 | Player Targeting Debug Observability v1 | `TB_W05_02_Player_Targeting_Debug_Observability_v1.md` | 완료 |
| W05-03 | Player Target Switching v1 | `TB_W05_03_Player_Target_Switching_v1.md` | 완료 (W06 Destroy 통합 검증 포함) |
| W05-04 | Player Target Lock Assist v1 | `TB_W05_04_Player_Target_Lock_Assist_v1.md` | 완료 |
| W05-05A | Player Target Marker v1 | `TB_W05_05A_Player_Target_Marker_v1.md` | 완료 |

## 구현 순서와 후속 경계

```text
W05-03: 좌우 타겟 전환
W05-04: 카메라 / 이동 락온 보정
W05-05A: 타겟 마커
W06: Enemy Dead / Destroy와 Target OnEndPlay 통합 검증
별도 UI 작업: Enemy Status HUD
```

W05 작업군은 Target Marker까지 C++ 구현, Development 빌드와 핵심 PIE 검증을 완료했다. 타겟 수명은 `OnDestroyed`가 아니라 Actor의 모든 월드 이탈을 포괄하는 `OnEndPlay`를 기준으로 관리한다. 현재 타겟과 콜백 Actor는 weak object index/serial identity로 비교하므로 이전 타겟의 늦은 종료 콜백이 새 타겟을 해제하지 않는다.

W06에서 실제 Enemy 사망·Destroy 경로가 구현된 뒤 다음 통합 경계를 확인했다.

- 현재 타겟 Enemy Destroy 시 타겟·락온·마커 해제
- 사망 해제 뒤 Destroy가 중복 변경 이벤트를 만들지 않음
- A에서 B로 전환한 뒤 A가 종료되어도 B 유지

`MaxTargetAngleDegrees = 0`은 분모 0을 피하는 안전한 계산 경로를 구현했다. 이 수학 경계의 자동화 검증은 아직 별도 테스트 부채이며, 실제 Destroy 통합 완료 상태와 구분해 기록한다.

Enemy Status HUD는 Health / Balance 표현 책임을 포함하므로 별도 UI 후속 작업으로 분리한다.
