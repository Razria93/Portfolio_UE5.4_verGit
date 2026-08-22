# Runtime LOD 프로파일링

이 폴더는 Runtime LOD 프로파일링 계획, 결과 노트, 후속 분석을 보관한다.

> 역사성 주의: 일부 계획·결과 문서는 Evidence-centric Combat Participation 이전의
> `UCWorldSubsystem_CombatEngage`, request container, lease 용어를 사용한다. 해당 문서의 측정값과
> CSV stat 이름은 당시 증적으로 보존한다. 현재 runtime 계약은
> [S34 Combat Participation Policy](../../../05_System_Architecture/S34_UE5_Portfolio_Combat_Participation_Policy.md)를 따른다.

raw CSV / log 파일은 이 폴더에 저장하지 않는다. 각 측정 노트는 다음 항목을 기록한다.

- 측정 조건
- 대표 source capture ID
- p95 중심 결과 표
- 해석과 후속 판단

## 현재 주제

| 주제 | 주요 문서 |
| --- | --- |
| Enemy mesh / weapon actor | `Enemy_Mesh_Runtime_LOD_Measurements.md` |
| Perception | `AI_Perception_Runtime_LOD_Measurements.md` |
| Animation / pose | `AI_Animation_Pose_LOD_Measurement_Plan.md` |
| Movement / nav | `AI_Movement_Nav_LOD_Measurement_Plan.md` |
| BT update interval | `AI_BT_Update_Interval_LOD_Result_Note.md` |
| AIContext interval split legacy experiment | `AI_BT_Update_Interval_AIContext_Level_Split_Note.md` |
| Assignment warmup | `AI_CombatEngage_Assignment_Bootstrap_Warmup_Plan.md` |
| Alert cap | `AI_AlertCap_Comparison_Plan.md` |
| Combat collision / hit window | `AI_Combat_Collision_HitWindow_Measurement_Plan.md` |
| Combat feedback presentation | `AI_Combat_Feedback_Presentation_Measurement_Plan.md` |
| Enemy actor tick audit | `AI_Enemy_Actor_Tick_Audit_Plan.md` |
| State-based Runtime LOD policy | `AI_State_Based_Runtime_LOD_Policy_Plan.md` |
| Runtime LOD tier snapshot refactor | `AI_Runtime_LOD_Tier_Snapshot_Refactor_Plan.md` |
| Dormant Runtime LOD deferred plan | `AI_Dormant_Runtime_LOD_Deferred_Plan.md` |
| Debugging obstacles | `AI_Runtime_LOD_Debugging_Obstacle_Note.md` |
