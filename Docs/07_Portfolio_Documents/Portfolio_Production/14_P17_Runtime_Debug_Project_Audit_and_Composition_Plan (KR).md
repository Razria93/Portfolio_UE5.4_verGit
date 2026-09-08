# 문서상 p.20 Runtime Debug 프로젝트 감사 및 구성 계획

> 파일명은 초기 22페이지 체계의 감사 기록을 보존한다. 현행 25페이지 체계에서는 p.20에 적용한다.

> 상태: **코드 대조 완료 / 요구조건·구현 구조·관찰 경계 다이어그램·Focused Enemy 실사용 캡처 반영**

## 확인 근거와 주장 범위

- `CDebugOverlayHUD::DrawHUD`는 Targeting, Movement, Combat Participation, Balance/Collapse, Execution 등의 상태를 표시용 ViewData로 구성한다.
- `FDebugOverlaySnapshotStore`는 Debug 수집이 켜진 경우 최근 이벤트를 기록한다. 이는 디버그 데이터의 수집 경로이며 gameplay 상태·판정의 소유권을 가져가지 않는다.
- `FDebugOverlayViewDataBuilder`와 Canvas renderer는 Actor panel·Event Log를 구성한다. World DrawDebug는 CVar로 표시 범위를 정하는 별도 출력 경로다.
- Debug helper와 FocusComponent는 `!UE_BUILD_SHIPPING` 경계에서만 사용된다. Shipping HUD·gameplay UI·BT active node·Runtime LOD actual로 표현하지 않는다.
- `debug_overlay_p1_final_enemy_current_ai.png`에는 Focus된 Enemy의 Target, IntentState, HasLOS, DistanceToTarget, IsCombatAction이 보인다. 이는 Current AI 블록의 표시 근거이며 Event Log와의 같은 화면 대조 근거는 아니다.

## 페이지 구성

1. **요구조건 및 의도**: 분산된 런타임 상태와 최근 이벤트를 시스템별로 따로 추적하지 않고, Focus된 Actor 기준으로 빠르게 대조할 필요를 문제로 제기한다. 관찰 대상·필요 정보·도구 의도를 3칸으로 고정한다.
2. **구현 구조**: `Focus Request → Focus Resolve`가 관찰 Actor를 확정해 `ViewData Builder`에 전달합니다. `Runtime Systems → Read-only Input → ViewData Builder → Canvas Overlay`는 상태 Snapshot과 최근 Event History를 Focus 기준의 화면 데이터로 조합하며, `World Debug CVar → World DrawDebug`는 별도 보조 표시 경로입니다. p.21 Editor Plugin은 Focus·CVar 제어를 위임하는 외부 입력으로만 표시합니다.
3. **실사용 검증**: 중심 캡처는 Focus된 Enemy의 `FocusActor`·State·Action·HP와 Combat Target Facing 문맥을 같은 패널에서 보여준다. 콜아웃은 관찰 대상·실행 문맥·생존 상태·Target Facing으로 한정한다.
4. **사용 범위**: FocusComponent가 선택한 Debug 관찰 대상 기준이며, 일반 Lock-on·BT active node·Shipping HUD·Runtime LOD actual은 이 페이지의 증명 범위가 아님을 명시한다.

## 표현 제한

- 이 도구는 게임플레이 상태를 변경하지 않는 관찰 경로다. Debug Snapshot Store의 이벤트 기록을 gameplay 상태의 소유·수정으로 설명하지 않는다.
- Current AI FinalCandidate는 보조 증거로만 사용한다. Event Log·NearestFocus 단독 캡처도 중심 증거로 사용하지 않으며, 같은 Focused Enemy의 Details와 Event Log를 함께 보여주는 신규 Runtime 캡처가 필요하다.
- 디버그 도구가 AI 판단의 정확성, Runtime LOD의 성능, Shipping HUD 품질을 보장한다고 표현하지 않는다.
