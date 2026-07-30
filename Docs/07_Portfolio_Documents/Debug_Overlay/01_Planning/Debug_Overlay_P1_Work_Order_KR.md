# Debug Overlay P1 Work Order

## Target Selection 결정 보강

P1 Target Selection은 현재 브랜치에서 debug overlay evidence를 닫는 것을 우선한다.

- P1 구현은 debug overlay 한정 `UCDebugOverlayTargetComponent`로 진행한다.
- component 소유 위치는 `ACPlayerController`를 우선한다.
- source chain 결정은 `Debug_Overlay_P1_Target_Selection_Decision_KR.md`를 우선한다.
- P1 기본 Enemy panel은 명시 target 기반이며, target이 없으면 `EnemySource: None`을 표시한다.
- `RecentCombatTarget`과 `WorldScanFallback`은 기본 자동 fallback이 아니라 diagnostic 후보로 격하한다.
- 범용 combat target component는 이번 브랜치에서 구현하지 않는다.
- 브랜치 마감 후 별도 리팩터링에서 `UCTargetSelectionComponent` 또는 `UCTargetProviderComponent`로 승격을 검토한다.

세부 설계 기준은 `Debug_Overlay_P1_Target_Selection_Design_KR.md`를 따른다.

구현 계획 기준은 `Debug_Overlay_P1_Target_Component_Implementation_Plan_KR.md`를 따른다.

## 1. 목적

이 문서는 debug overlay P1 작업 순서와 최종 촬영 연기 정책을 고정한다.

P0.5에서 반복 촬영/패키징은 중단한다. Round1과 Round1_StaggerCount는 임시 검증 evidence로 유지하고, 최종 제출용 촬영은 P1 설계/구현/검증 이후 한 번에 진행한다.

앞으로 debug overlay 작업 제안은 촬영보다 P1 설계/구현을 우선한다.

## 2. 촬영 정책 변경

| 항목 | 결정 |
| --- | --- |
| Round1 | 임시 검증 evidence로 유지 |
| Round1_StaggerCount | Stagger Count 표시 검증 evidence로 유지 |
| 반복 촬영/패키징 | P1 완료 전까지 중단 |
| FinalCandidate 촬영 | P1 검증 이후 한 번에 진행 |
| 최종 제출 claim | P1에서 실제 표시/검증된 값만 사용 |

주의:

- Round1 파일을 final candidate로 승격하지 않는다.
- 새 캡처 요청은 P1 기능이 닫힌 후에만 제안한다.
- 실제 코드에서 읽지 못하는 값을 성공 evidence처럼 표시하지 않는다.
- overlay는 개발 전용 evidence 화면이며 Shipping HUD가 아니다.

## 3. P0.5 완료 상태

P0.5에서 완료된 항목은 다음으로 본다.

| 항목 | 상태 |
| --- | --- |
| Player/Enemy panel 분리 | 완료 |
| Player/Enemy 동일 항목 순서 | 완료 |
| Movement 표시 | 완료 |
| HP 표시 | 완료 |
| Stagger Count 표시 | 완료 |
| Player blue tab / Enemy red tab | 완료 |
| background/text 가독성 개선 | 완료 |
| Store 기반 Recent block | 완료 |
| EventLog 표시 | 완료 |
| Round1 패키징 | 임시 evidence로 완료 |
| StaggerCount 보강 패키징 | 임시 evidence로 완료 |

P0.5 panel 항목 순서는 P1에서도 유지한다.

```text
State
Action
Reaction
Stagger
Guard
Movement
HP
Runtime LOD
AI
```

## 4. P1 방향

사용자 요구와 P0.5 한계를 기준으로 P1 방향은 다음으로 고정한다.

| 방향 | 설명 |
| --- | --- |
| Target Component 기반 Enemy Selection | `WorldScanFallback`이 아니라 실제 target source 기반 enemy 표시를 우선한다. |
| Player/Enemy evidence 분리 | Player/Enemy panel뿐 아니라 Recent/EventLog의 subject 분리까지 검토한다. |
| EventLog category filter | `All`, `Execution`, `Combat`, `AI` 등 특정 종류의 log만 볼 수 있게 한다. |
| Player/Enemy별 EventLog 분리 | Store subject 분리 이후 진행한다. |
| Recent Execution/Combat/AI subject 분리 | world 단위 recent summary를 actor/subject 기준으로 나눌 수 있는지 검토한다. |
| Runtime LOD 실제 표시 | `N/A`가 아닌 실제 enemy Runtime LOD tier 표시를 검토한다. |
| AI 정보 보강 | `AI: NotCaptured`를 줄이고, intent/task/result 중 의미 있는 값을 표시한다. |
| 개발 전용 UI 유지 | UMG/Slate 전환이나 제품 HUD화는 목표가 아니다. |

## 5. P1 최소 성공선

P1 최소 성공선은 다음으로 둔다.

```text
Target Component 기반 Enemy Selection
HUD EnemySource TargetComponent 우선 전환
EventLog category filter
P1 검증
```

위 항목이 닫히면 P1의 핵심 약점인 enemy 대상 신뢰도와 log 가독성은 개선된 것으로 본다.

다음 항목은 P1 보강 또는 P2 후보로 둘 수 있다.

- Player/Enemy별 Recent 완전 분리
- Player/Enemy별 EventLog 완전 분리
- Runtime LOD 실제 표시
- AI detail 확장
- capture automation

## 6. P1 작업 순서

권장 작업 순서는 다음이다.

| 순서 | 작업 | 목적 | 산출물 |
| --- | --- | --- | --- |
| 1 | P1 범위 확정 | 필수/보강/보류 항목 분리 | P1 scope 문서 |
| 2 | Target Component 기반 Enemy Selection 설계 | Enemy panel claim 강화 | target source 설계 문서 |
| 3 | Target Component / Target Provider 구현 | 실제 target source 확보 | C++ component/provider |
| 4 | HUD EnemySource 전환 | `TargetComponent.Trace/Nearest` 명시 선택 우선, target 없음은 `None` 표시 | HUD 표시 변경 |
| 5 | Store subject 분리 설계 | Player/Enemy Recent/EventLog 분리 기반 마련 | Store subject 설계 문서 |
| 6 | EventLog category filter 설계/구현 | 필요한 종류의 log만 표시 | CVar 또는 preset 기반 filter |
| 7 | Player/Enemy Recent/EventLog 분리 | subject 기반 evidence 분리 | Store/HUD 확장 |
| 8 | Runtime LOD 실제 표시 | enemy Runtime LOD claim 가능성 확보 | getter/hook/HUD 표시 |
| 9 | AI 표시 보강 | `AI: NotCaptured` 감소 | intent/task/result 표시 |
| 10 | P1 검증 | 표시값과 문서 claim 일치 확인 | PIE checklist |
| 11 | FinalCandidate 재촬영/패키징 | 최종 후보 evidence 확보 | final candidate 이미지/문서 |
| 12 | 포트폴리오 문서 연결 | PF 문서에 evidence 연결 | PF02/PF03/PF04 mapping |

## 7. 기술 의존성

| 의존성 | 판단 |
| --- | --- |
| Target Component 먼저 | Enemy panel이 특정 대상의 상태라고 주장하려면 target source가 먼저 필요하다. |
| HUD EnemySource 전환 | Target Component 구현 직후 `EnemySource: TargetComponent`가 표시되어야 한다. |
| Store subject 분리 | Player/Enemy Recent/EventLog 분리의 선행 조건이다. |
| EventLog category filter | subject 분리 전/후 또는 병렬로 가능하다. category 기반이라 subject 분리보다 독립적이다. |
| Runtime LOD 표시 | target enemy가 안정적으로 잡힌 이후 진행하는 것이 안전하다. |
| AI 표시 보강 | Target Component 이후 보조 단계로 진행한다. |
| 최종 촬영 | P1 검증 이후로 미룬다. |

이전 target source fallback chain:

```text
TargetComponent
RecentCombatTarget
WorldScanFallback
```

위 chain은 `Debug_Overlay_P1_Target_Selection_Decision_KR.md`로 대체한다. P1 기본 HUD path에서는 `RecentCombatTarget`과 `WorldScanFallback`이 Enemy panel을 자동으로 채우지 않는다.

## 8. P1 보류 가능 항목

다음 항목은 P1 중간 판단에 따라 후반 또는 P2로 넘길 수 있다.

| 항목 | 보류 기준 |
| --- | --- |
| Player/Enemy별 EventLog 완전 분리 | Store subject 분리가 과도하게 커질 때 |
| Player/Enemy별 Recent 완전 분리 | recent summary의 owner/source/target 의미가 불명확할 때 |
| Runtime LOD 실제 표시 | getter/hook 부담이 크거나 표시값 신뢰도가 낮을 때 |
| AI detail 확장 | blackboard/BT 노출 범위가 커질 때 |
| capture automation | 수동 최종 촬영으로 충분할 때 |

P1에서 과도한 확장을 피한다. 표시 가능한 값과 claim이 분명한 항목부터 처리한다.

## 9. 다음 작업 제안 기준

이 문서 이후 debug overlay 작업 제안 기준은 다음으로 전환한다.

1. 촬영/패키징보다 P1 범위 확정을 먼저 제안한다.
2. 그 다음 Target Component 기반 Enemy Selection 설계를 제안한다.
3. Target Component 구현 전에는 enemy claim을 fallback 기준으로만 설명한다.
4. Store subject 분리 전에는 Player/Enemy Recent/EventLog 분리를 구현 완료처럼 말하지 않는다.
5. EventLog 추가 compact는 사용자 결정에 따라 보류 상태를 유지한다.
6. 최종 촬영/패키징은 P1 검증 이후에만 제안한다.

## 10. 금지/주의

- 코드에서 읽지 못하는 값을 성공 evidence처럼 표시하지 않는다.
- Shipping HUD처럼 보이게 만들지 않는다.
- UMG/Slate 전환은 목표가 아니다.
- `.umap`, `.uasset`, config, `Build.cs` 변경은 별도 결정 없이는 하지 않는다.
- `WorldScanFallback`을 Target Component 기반 evidence로 설명하지 않는다.
- Stagger Count는 현재 parry stack/threshold이며 누적 총량이나 장기 통계가 아니다.
- Runtime LOD가 `N/A`이면 Runtime LOD 성공 evidence로 사용하지 않는다.

## 11. 결론

P0.5는 임시 evidence 확보와 overlay 검증 단계로 닫고, 반복 촬영은 중단한다.

P1은 Target Component 기반 Enemy Selection을 중심으로 enemy 대상 신뢰도를 먼저 강화한다. 이후 EventLog filter, Store subject 분리, Player/Enemy Recent/EventLog 분리, Runtime LOD/AI 보강을 순차적으로 진행한다.

최종 촬영은 P1 검증 이후 한 번에 진행한다.
