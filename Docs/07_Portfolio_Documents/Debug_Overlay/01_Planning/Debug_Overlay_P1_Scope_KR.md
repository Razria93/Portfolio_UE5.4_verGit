# Debug Overlay P1 Scope

## 1. 목적

이 문서는 debug overlay P1에서 구현할 필수/보강/보류 범위를 확정한다.

P1의 목적은 P0.5 overlay의 표시 항목을 무작정 늘리는 것이 아니라, 최종 촬영 전에 enemy 대상 신뢰도와 EventLog 제어력을 높이는 것이다. 최종 촬영/패키징은 P1 검증 이후로 미룬다.

## 2. P1 목표

P1 목표는 다음 네 가지다.

| 목표 | 설명 |
| --- | --- |
| Enemy panel 신뢰도 강화 | `WorldScanFallback` 중심 claim에서 벗어나 실제 target source 기반 표시로 전환한다. |
| Target source 명시 | overlay에 enemy source가 `TargetComponent.Trace`, `TargetComponent.Nearest`, `None` 중 무엇인지 명확히 표시한다. |
| EventLog 가독성/제어 개선 | category filter로 필요한 종류의 log만 볼 수 있게 한다. |
| 최종 촬영 전 품질 확보 | P1 검증 이후 최종 evidence 촬영으로 넘어갈 수 있는 상태를 만든다. |

## 3. P1 필수 범위

P1 필수 범위는 최소 성공선을 기준으로 고정한다.

| 필수 항목 | 포함 내용 | 완료 기준 |
| --- | --- | --- |
| Target Component 기반 Enemy Selection 설계 | target source, 명시 선택/None 표시 정책 확정 | 설계 문서 작성 |
| Target Component / Target Provider 구현 | 현재 target actor를 제공하는 최소 구조 구현 | HUD에서 target actor 조회 가능 |
| HUD EnemySource 전환 | `TargetComponent.Trace/Nearest` 명시 선택 우선, target 없음은 `None` 표시 | overlay에 source와 선택 대상 표시 |
| EventLog category filter | `All`, `Execution`, `Combat`, `AI` 표시 제어 | filter 기준에 맞게 EventLog 표시 |
| P1 검증 체크리스트 작성 | P1 표시값과 claim 검증 기준 정리 | PIE 확인 절차 문서화 |

P1 필수 성공선:

```text
Target Component 기반 Enemy Selection
HUD EnemySource TargetComponent 우선 전환
EventLog category filter
P1 검증
```

## 4. EnemySource Selection Policy

P1의 enemy source selection policy는 `Debug_Overlay_P1_Target_Selection_Decision_KR.md`를 우선한다.

```text
TargetComponent.Trace
TargetComponent.Nearest
None
```

해석 기준:

| Source | 의미 | Evidence claim |
| --- | --- | --- |
| `TargetComponent.Trace` | camera forward trace로 명시 선택한 target | 최종 enemy panel claim의 우선 근거 |
| `TargetComponent.Nearest` | 사용자 명령으로 nearest enemy를 명시 선택한 target | 명시 command 기반 보조 target selection evidence |
| `None` | 명시 target 없음 | Enemy panel target evidence 없음 |

`RecentCombatTarget`과 `WorldScanFallback`은 P1 기본 source chain에서 제외한다. 이후 diagnostic/source 검증 후보로 유지할 수 있지만, target 없음 상태에서 Enemy panel을 자동으로 채우지 않는다.

범위 제한:

- Target Component는 debug overlay가 읽을 현재 target source를 제공하는 최소 구조로 제한한다.
- 완전한 lock-on 시스템, target cycling UI, target selection gameplay는 P1 필수 범위가 아니다.
- `RecentCombatTarget`은 actor raw pointer 장기 보관을 피한다.
- 필요하면 weak reference, actor name, frame/time metadata를 사용하고 stale 상태를 표시한다.

## 5. EventLog Category Filter Scope

EventLog category filter는 P1 필수 범위다.

필수 category:

| Preset | 표시 |
| --- | --- |
| `All` | 모든 EventLog |
| `Execution` | action/reaction decision 관련 log |
| `Combat` | combat signal/result 관련 log |
| `AI` | AI combat task 관련 log |

정책:

- filter는 Store subject 분리 전/후 또는 병렬로 구현 가능하다.
- category filter는 Player/Enemy별 분리와 별도 기능이다.
- P1에서는 EventLog 추가 compact를 다시 진행하지 않는다.
- category filter가 켜져도 실제 발생하지 않은 log를 성공 evidence처럼 만들지 않는다.

## 6. P1 보강 범위

다음은 P1에서 가능하면 진행하되, 필수 성공선은 아니다.

| 보강 항목 | 목적 | 선행 조건 |
| --- | --- | --- |
| Store subject 분리 설계 | Player/Enemy Recent/EventLog 분리 기반 마련 | 기존 Store event 구조 검토 |
| Player/Enemy Recent/EventLog 분리 | world 단위 공통 block을 actor/subject 기준으로 분리 | Store subject 분리 |
| Runtime LOD 실제 표시 | Enemy `Runtime LOD: N/A` 해소 | 안정적인 enemy target source |
| AI 표시 보강 | `AI: NotCaptured` 감소 | Target Component 이후 AI source 검토 |
| 다중 enemy selector 검토 | 다중 enemy 상황 표시 개선 | Target source 정책 확정 |

보강 범위는 필수 작업을 완료한 뒤 진행한다. 구현 중 범위가 커지면 P2로 넘긴다.

## 7. P1 보류 / P2 후보

다음은 P1에서 즉시 구현하지 않아도 된다.

| 항목 | 보류 이유 |
| --- | --- |
| capture automation | 최종 촬영은 P1 검증 이후 수동으로 충분할 수 있음 |
| UMG/Slate 전환 | 개발 전용 Canvas overlay 유지가 현재 목적에 맞음 |
| Shipping HUD화 | debug evidence 목적과 맞지 않음 |
| Player/Enemy EventLog 완전 분리 | Store subject 분리가 과도하게 커질 경우 P2로 이동 |
| Runtime LOD/AI detail 심화 | getter/hook 부담이 큰 경우 P1 후반 또는 P2로 이동 |
| 최종 영상/스크린샷 패키징 | P1 검증 이후 진행 |

## 8. P1 비목표

P1에서는 다음을 목표로 삼지 않는다.

- 최종 촬영/패키징
- 포트폴리오 본문 연결
- 성능 성공 주장
- gameplay flow 변경
- 기존 audit log format 변경
- Round1을 final candidate로 승격
- EventLog 추가 compact
- Shipping HUD처럼 보이는 UI 개선

## 9. P1 완료 기준

P1 완료 여부는 다음 기준으로 판단한다.

| 기준 | 완료 판단 |
| --- | --- |
| `EnemySource: TargetComponent.Trace/Nearest` 표시 가능 | Target Component target이 있을 때 HUD에 표시 |
| target 없음 표시 | 명시 target이 없을 때 `EnemySource: None` 표시 |
| 자동 fallback 제한 | RecentCombatTarget/WorldScanFallback이 target 없음 상태를 자동으로 채우지 않음 |
| EventLog filter 동작 | `All/Execution/Combat/AI` 기준으로 표시 제어 가능 |
| P1 검증 체크리스트 | PIE 확인 절차가 문서화됨 |
| 최종 촬영 판단 | P1 검증 이후 촬영/패키징으로 넘어갈지 결정 가능 |

## 10. P1 이후 작업 연결

P1 Scope 확정 이후 다음 작업은 `Target Component 기반 Enemy Selection 설계`다.

권장 흐름:

```text
P1 Scope 확정
Target Component 기반 Enemy Selection 설계
Target Component / Target Provider 구현
HUD EnemySource 전환
EventLog category filter
P1 검증 체크리스트
P1 검증
FinalCandidate 촬영/패키징
포트폴리오 문서 연결
```

최종 촬영은 P1 검증 이후 진행한다. 포트폴리오 문서 연결은 최종 evidence package 이후 진행한다.

## 11. 주의

- Target Component 구현 전에는 enemy claim을 fallback 기준으로만 설명한다.
- Store subject 분리 전에는 Player/Enemy Recent/EventLog 분리를 구현 완료처럼 말하지 않는다.
- Runtime LOD가 `N/A`이면 Runtime LOD 성공 evidence로 사용하지 않는다.
- Stagger Count는 현재 parry stack/threshold이며 누적 총량이나 장기 통계가 아니다.
- `.umap`, `.uasset`, config, `Build.cs` 변경은 별도 결정 없이는 하지 않는다.

## 12. 결론

P1의 필수 범위는 enemy 대상 신뢰도와 EventLog 제어 개선에 집중한다.

P1에서 반드시 닫을 것은 Target Component 기반 Enemy Selection, HUD EnemySource 전환, EventLog category filter, P1 검증이다. Store subject 분리, Player/Enemy Recent/EventLog 분리, Runtime LOD/AI 보강은 보강 범위로 두고, 구현 부담이 커지면 P2로 넘긴다.
