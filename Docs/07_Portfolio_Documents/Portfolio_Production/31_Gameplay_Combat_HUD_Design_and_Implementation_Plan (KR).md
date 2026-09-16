# Gameplay Combat HUD Design and Implementation Plan

> 상태: `Deferred — implementation not started`
>
> 목적: 영상용 Clean Gameplay 화면에 Player 생존 정보와 Focused Enemy 전투 정보를 읽기 쉽게 표시하는 최소 UMG HUD를 설계·구현한다. 이 문서는 현재 작업 트리를 건드리지 않고, 이후 착수를 위한 범위·분리·검증 기준을 고정한다.

## 1. 목표와 비범위

### 목표

- Clean 영상에서 Player HP, Focused Enemy HP, Balance, 의미 있는 Enemy 상태를 읽을 수 있게 한다.
- Target이 없는 평상시에는 Enemy panel을 숨겨 중앙 전투 화면을 비운다.
- 기존 Runtime Debug Overlay와 별도로 동작하는 **게임플레이용 읽기 전용 HUD**를 만든다.
- [Gameplay Video Evidence Plan](30_Portfolio_Gameplay_Video_Evidence_Plan%20%28KR%29.md)의 Clean Take와 Proof Take를 명확히 구분한다.

### 비범위

- Action / Reaction 판정, Target 선택, Health / Balance 계산, AI 상태를 새로 만들거나 변경하지 않는다.
- Debug Overlay, Event Log, CVar, Editor Plugin의 책임을 Gameplay HUD로 옮기지 않는다.
- 레퍼런스 게임의 로고·고유 아이콘·스킬 체계·성능 정보·격자 자산을 복제하지 않는다.
- 스킬 아이콘, BE / SH 같은 현재 프로젝트에 없는 자원, FPS / GPU / CPU 표시, 신규 VFX를 추가하지 않는다.
- 이번 계획 문서 단계에서는 Source, Config, Content, UMG asset을 수정하지 않는다.

## 2. 현재 작업 트리와 착수 게이트

현재 브랜치는 `main`이며 기존 Content 변경과 untracked Stellar 에셋이 남아 있다. HUD 구현은 아래 조건이 충족될 때만 시작한다.

| Gate | 착수 조건 | 이유 |
| --- | --- | --- |
| G1. 기준선 고정 | 현재 Content / capture 변경을 별도 커밋·백업 브랜치·작업 단위로 분류한다. | HUD 변경에 기존 에셋·촬영 변경이 섞이지 않게 한다. |
| G2. 분기 | 깨끗한 기준 커밋에서 `feature/gameplay-combat-hud` 같은 전용 브랜치를 만든다. | 구현·검증·되돌리기 범위를 분리한다. |
| G3. 데이터 감사 | Health, Balance, Combat Target, Enemy 상태의 실제 읽기 경로·delegate·lifecycle를 확인한다. | UI가 새 상태를 소유하거나 폴링 오류를 만들지 않게 한다. |
| G4. 시각 명세 승인 | 색상·폰트·배치·No Target / Dead 동작을 담은 Visual Spec을 승인한다. | UMG 제작 중 디자인 판단을 반복하지 않게 한다. |
| G5. TestRoom preflight | TestRoom에서 Target 획득, HP 변경, Balance 변경, Guard / Collapse / Dead의 재현 가능성을 확인한다. | 영상에 쓸 수 없는 HUD를 만들지 않게 한다. |

G1–G5 전에는 구현 브랜치 생성, widget asset 생성, Config 변경, Content 저장을 하지 않는다.

## 3. 레퍼런스에서 가져올 것과 가져오지 않을 것

레퍼런스의 목적은 UI 자산 복제가 아니라, 전투 장면에서 정보가 읽히는 **계층과 화면 점유율**을 분석하는 것이다.

| 관찰 요소 | Project Stellar 적용 | 제외 이유 / 경계 |
| --- | --- | --- |
| 상단 중앙 Target 정보 | Focused Enemy의 이름 없는 `TARGET`, HP, Balance, 상태 배지 | 레퍼런스 고유 보스명·격자형 게이지·장식은 사용하지 않음 |
| 좌하단 Player 정보 | Player HP 한 줄 | 레퍼런스의 여러 자원·소모품·스킬 목록은 현재 기능 범위 밖 |
| 넓은 중앙 전투 영역 | HUD를 모서리와 상단 safe area에 한정 | 피격·Parry·Execution 장면을 가리지 않음 |
| 밝은 게이지 / 저대비 배경 | 원본 색상 토큰으로 명도 대비만 재해석 | 고유 색조·아이콘·폰트를 모사하지 않음 |
| 상태가 바뀔 때만 강조 | `GUARD`, `COLLAPSE`, `DEAD`처럼 전투 이해에 필요한 badge | `Idle`, 내부 enum, Decision, Event Log는 Debug Overlay 전용 |

## 4. MVP 정보 구조

```text
                        [ TARGET ]
                  HP       ━━━━━━━━━━━
                  BALANCE  ◆ ◆ ◆ ◇
                  STATE    GUARD / COLLAPSE / DEAD

[ PLAYER ]
HP              ━━━━━━━━━━━
```

| 영역 | 표시 조건 | 표시 데이터 | 표현 경계 |
| --- | --- | --- | --- |
| Player Vital | 항상 | 현재 HP / 최대 HP 또는 비율 | 기존 Health 값만 읽음 |
| Target Vital | 유효한 Focused Combat Target이 있을 때 | Target HP / 최대 HP, Balance 현재 / 최대 | Target을 직접 찾거나 변경하지 않음 |
| Target State Badge | `Guard`, `Collapse`, `Dead` 등 시각적으로 의미 있는 상태일 때 | 사용자 친화 상태명 | 내부 Action key, AI intent, Debug enum을 노출하지 않음 |
| No Target | Target 없음·무효·Destroy 뒤 | Target panel fade out | stale Target 데이터를 유지하지 않음 |

상태값 중 어떤 것을 최종 badge로 지원할지는 G3 데이터 감사에서 **실제 public 읽기 경로와 상태 수명**이 확인된 것만 확정한다. 확정 전에는 `Guard / Collapse / Dead`를 설계 후보로만 다룬다.

## 5. 화면 배치와 Visual Spec 작성 범위

후속 Visual Spec은 16:9(2560×1440 기준)와 21:9 / 다른 해상도에서 같은 safe area를 유지하도록, 절대 pixel이 아닌 정규화 anchor와 margin을 사용한다.

| 영역 | 기준 배치 | 설계 질문 |
| --- | --- | --- |
| Target panel | 상단 중앙, 화면 폭 약 30–40% 이내 | 카메라 상단·Enemy silhouette와 겹치지 않는가? |
| Player panel | 좌하단 safe area, 화면 폭 약 20–25% 이내 | 조작 UI / 자막 영역과 겹치지 않는가? |
| 중앙 전투 영역 | 화면 중앙 50% 이상 비움 | Combo, Guard, Execution의 충돌·피격을 가리지 않는가? |
| 상태 badge | Target HP 아래의 한 줄 | 긴 상태명·동시 상태가 발생해도 overflow하지 않는가? |

Visual Spec에서 확정할 항목:

- 배경 투명도, border, 간격, 게이지 두께, segment와 continuous bar의 선택
- HP, Balance, Guard, Collapse, Dead의 원본 색상 토큰과 명도 대비
- 기존 프로젝트 / 엔진에서 사용할 수 있는 폰트와 fallback
- Fade in / out, HP 보간, 상태 badge 전환의 최대 시간
- 화면 녹화 시 글자 크기와 1080p downscale 가독성
- UMG가 Debug Overlay 및 Gameplay subtitle과 겹치지 않는 capture safe area

외부 폰트·아이콘·텍스처를 도입하기 전에는 라이선스, 저장소 정책, LFS 영향을 별도 결정한다. MVP는 기존 엔진 또는 프로젝트에서 확인된 font / primitive widget만 사용한다.

## 6. 데이터·책임 감사 계획

구현 전 아래 세 가지 읽기 전용 감사를 수행한다. 감사자는 파일·Git·GitHub 상태를 변경하지 않는다.

| 감사 | 확인할 것 | 필요한 결과 |
| --- | --- | --- |
| UI Data Auditor | Player / Enemy Health, Balance, Guard / Collapse / Dead, Combat Target의 public accessor·delegate·Destroy 처리 | widget이 구독할 최소 데이터 계약과 해제 시점 |
| UI Architecture Auditor | 현재 HUD / PlayerController / UMG 생성 경로, Runtime Debug Overlay와의 경계, module 의존성 | `WBP_CombatHUD`의 생성 owner와 runtime-only 의존성 |
| Visual & Capture Auditor | 레퍼런스 화면의 hierarchy, 16:9 safe area, 기존 Hero / Debug capture와 subtitle 영역 | Visual token 초안과 Clean / Proof 겹침 방지 표 |

감사 뒤 확정할 구현 계약은 다음을 만족해야 한다.

```text
Combat Target / Health / Balance / State source
            ↓ read-only snapshot or delegate
WBP_CombatHUD / WBP_PlayerVital / WBP_TargetVital / WBP_CombatStateBadge
            ↓ display only
Gameplay screen
```

Widget은 Target state, Health, Balance, Guard, Collapse, Dead를 소유하거나 변경하지 않는다. Target 교체·Destroy·PIE 종료 시 binding을 해제하고, 이전 Target 값이 새 Target에 남지 않게 한다.

## 7. 구현 작업 분해

G1–G5를 통과한 뒤 아래 작업을 별도 Work Brief로 수행한다.

1. **Visual Spec 확정** — `32_Gameplay_Combat_HUD_Visual_Spec (KR).md`에 wireframe, token, 상태별 mock, data contract를 고정한다.
2. **UI Shell** — `WBP_CombatHUD`와 Player / Target / State 하위 widget의 빈 layout을 만든다.
3. **Read-only binding** — 검증된 accessor 또는 delegate로 HP, Balance, Target validity, 상태 badge를 갱신한다.
4. **Lifecycle handling** — Target acquire / switch / clear / Destroy, PIE restart에서 show·hide·unbind를 확인한다.
5. **Capture tuning** — 1080p / 16:9에서 글자·게이지·상태 badge가 Clean footage를 가리지 않는지 조정한다.
6. **Proof separation** — 동일 장면에서 Gameplay HUD와 Debug Overlay를 함께 켰을 때 필요한 Proof preset만 남기고, Clean preset은 Debug UI가 없어야 한다.

## 8. 검증 시나리오

| 시나리오 | 기대 결과 | 실패로 보는 조건 |
| --- | --- | --- |
| PIE 시작, Target 없음 | Player HP만 표시, Target panel 숨김 | Enemy 정보가 stale 상태로 표시 |
| Target acquire / switch | 새 Target의 HP / Balance로 즉시 교체 | 이전 Target 값·state badge가 남음 |
| Target HP / Balance 변경 | 게이지와 값이 실제 Component 상태를 반영 | 새 계산·보정·판정을 UI가 수행 |
| Guard / Collapse / Dead | 실제 지원 상태일 때만 badge 전환 | 내부 enum / 기대값을 임의로 표시 |
| Target clear / Destroy | Target panel fade out, binding 해제 | Destroy Actor 참조 접근, UI 오류 |
| Clean capture | HUD만 보이고 Debug Overlay / Console / Editor window 없음 | 중앙 전투가 가려지거나 글자가 흐림 |
| Proof capture | HUD와 필요한 Debug Overlay가 같은 Actor를 가리킴 | Focus 불일치, `N/A`, `NotCaptured`, 겹친 패널 |

## 9. 촬영 문서와의 연결

- Clean Gameplay HUD는 [Gameplay Video Evidence Plan](30_Portfolio_Gameplay_Video_Evidence_Plan%20%28KR%29.md)의 `Clean` take에 사용한다.
- Debug Overlay의 CVar, Event Log, World Debug와 재촬영 기준은 [Runtime Capture and Video Runbook](29_Portfolio_Runtime_Capture_and_Video_Runbook%20%28KR%29.md)을 유지한다.
- Gameplay HUD의 구현 완료 자체를 AI 검증, Performance 개선, Shipping UI 완성의 근거로 표현하지 않는다.

## 10. 이 계획의 완료 기준

- HUD 구현은 별도 전용 브랜치와 Work Brief에서만 시작한다.
- 구현 전 Visual Spec이 데이터 source, 배치, token, 상태 visibility, capture safe area를 확정한다.
- MVP는 Player HP, Focused Target HP / Balance, 실제 지원 상태 badge를 넘지 않는다.
- Clean HUD와 Debug Overlay의 책임·표시 범위가 문서와 영상에서 구분된다.
- 구현 후에는 위 검증 시나리오와 Clean / Proof capture를 통과한 결과만 포트폴리오·Issue #119에 반영한다.
