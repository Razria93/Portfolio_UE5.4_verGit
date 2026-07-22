# N27. Debug / Profiling CVar Ownership Final Note

## 목적

이 문서는 debug log, diagnostic hook, profiling audit, CSV counter 정리 이후의 최종 CVar 소유권 기준을 기록한다.

관련 문서:

```text
N23_Debug_Log_And_Diagnostic_Code_Policy_Note.md
N24_Debug_Log_Cleanup_Inventory_Note.md
N25_Diagnostic_Log_Gating_And_Audit_Category_Plan_Note.md
N26_Diagnostic_Log_Full_Audit_Inventory_Note.md
```

책임 범위:

```text
N23: 일반 debug / diagnostic 정책
N24: cleanup inventory
N25: helper / gating 설계
N26: full audit inventory
N27: CVar ownership 최종 기준
```

이 문서는 이후 작업에서 debug, profiling, runtime policy 책임이 다시 섞이지 않도록 최종 경계를 고정한다.

---

## 최종 소유권 기준

```text
1. Debug / Diagnostic output CVar
   Owner: Core/Debug helper
   Responsibility: CVar, gate, message format, Output Log 호출, non-shipping no-op

2. Debug Dump CVar
   Owner: Core/Debug helper
   Responsibility: context / payload / result 상세 dump gate와 print formatting

3. Profiling audit / CSV counter CVar
   Owner: Core/Profiling helper
   Responsibility: CVar, gate, CSV counter, profiling summary, non-shipping no-op

4. Runtime policy / tuning CVar
   Owner: owning policy/system cpp
   Responsibility: 실제 gameplay policy 선택 또는 tuning 값
```

실제 게임 동작을 바꾸는 runtime policy / tuning CVar는 해당 policy 또는 system cpp에 남길 수 있다.
다만 policy selector CVar가 audit / counter 출력 의미까지 함께 가져서는 안 된다.

---

## Mixed CVar 금지

하나의 CVar가 runtime behavior와 diagnostic / profiling output을 동시에 제어하지 않도록 분리한다.

올바른 분리:

```text
StatePolicyMode
-> RuntimeLOD policy source만 선택

StatePolicyAudit
-> state tier CSV profiling counter만 제어
```

잘못된 분리:

```text
StatePolicyMode
-> policy source 선택
-> audit / counter output까지 같이 제어
```

이 기준은 animation, movement, BT interval, perception, combat feedback, combat collision profiling에도 동일하게 적용한다.

---

## CSV macro 기준

```text
CSV_CUSTOM_STAT_GLOBAL
-> event / counter 기록
-> 직접 macro 호출은 Core/Profiling helper 구현부에 둔다.
-> gameplay / BT / anim code는 Record...ForProfiling() API만 호출한다.

CSV_SCOPED_TIMING_STAT_GLOBAL
-> RAII scope timing 계측
-> 측정 대상 scope 본문에 유지한다.
-> helper 뒤로 숨기면 측정 범위가 helper 호출 범위로 왜곡될 수 있다.
```

최종 scan 기대값:

```text
Core/Profiling 밖 직접 CSV_CUSTOM_STAT_GLOBAL: 0개
Core/Profiling 밖 직접 CSV_SCOPED_TIMING_STAT_GLOBAL: 측정 scope 진입부만 허용
```

---

## 현재 적용 결과

Core/Debug 소유:

```text
AI combat BT audit gates
AI perception audit summaries
CombatEngage assignment audit output format
Combat signal / result / feedback diagnostic output
Action / reaction / movement / overlay / component reference diagnostic output
```

Core/Profiling 소유:

```text
DisableEnemyPerception
DisableEnemyHitProcessing
DisableEnemyWeaponActor
DisableEnemyCombatFeedback
AnimationRefreshAudit
StatePolicyAudit
AI animation refresh CSV counters
AI behavior tree CSV counters
AI state RuntimeLOD tier CSV counters
Combat collision CSV counters
Combat feedback CSV counters
```

Runtime policy / system owner 유지:

```text
EngageAssignmentWarmupTime
EngageAssignmentEngageCap
EngageAssignmentAlertCap
EnemyMeshMode
EnemyActorTickMode
EnemyMovementMode
EnemyAnimationMode
EnemyAnimationReducedRefreshInterval
BTUpdateIntervalMode
StatePolicyMode
```

위 CVar들은 debug output gate가 아니라 policy / tuning control이므로 owner cpp에 남긴다.

---

## 리뷰 체크리스트

새 CVar를 추가하거나 기존 CVar를 검토할 때 다음 기준으로 판단한다.

```text
1. text 출력 또는 diagnostic output을 제어하는가?
   -> Core/Debug helper

2. CSV counter, profiling summary, profiling audit output을 만드는가?
   -> Core/Profiling helper

3. gameplay policy 또는 tuning behavior를 선택하는가?
   -> owning policy/system cpp

4. 위 책임 중 둘 이상을 동시에 갖는가?
   -> merge 전에 CVar를 분리한다.

5. CSV_CUSTOM_STAT_GLOBAL인가?
   -> 직접 호출은 Core/Profiling 내부에만 둔다.

6. CSV_SCOPED_TIMING_STAT_GLOBAL인가?
   -> 측정 대상 scope 진입부에 둔다.
```
