# p.8 Balance / Collapse · p.9 Execution Collaboration 감사 및 구성 계획

> 상태: **코드·설계 문서 감사 완료 / Gameplay Hero·제출용 Runtime Capture 대기 / 기존 SVG는 구조 보조 자산으로 보존**

## 1. 문서상 25페이지 체계

- p.8 `Balance / Collapse`와 p.9 `Execution Collaboration`은 각각 게임플레이 페이지로 유지한다.
- p.15 `Execution Pair Coordination`은 구조 설명을 맡는 신규 페이지다. 기존 p.15~p.24은 p.16~p.25로 이동했으며, 현행 HTML도 25페이지로 동기화했다.
- 이유는 두 기능이 공유하는 Collapse opportunity가 있어도, 전자는 단일 Actor의 상태 수명주기이고 후자는 두 Actor의 Pair Transaction이기 때문이다.

## 2. p.8 Balance / Collapse

### 한 줄 메시지

Collapse는 단일 피격 연출이 아니라 Balance 누적·상태 전이·Loop TTL·Reset을 함께 관리하는 전투 수명주기다.

### 중심 구성

1. **Gameplay Hero:** Debug Overlay 없는 Parry 누적 → Collapse 진입 16:9 캡처(C06b). 실제 Hero를 확보하기 전에는 슬롯으로만 표기한다.
2. **런타임 증거 목업:** Count/Threshold, Lifecycle State/Serial, Loop TTL, Reset/Recovery 네 슬롯. C06a Stack 2·3은 Count/Threshold의 보조 근거로만 사용하고 나머지는 `Needs Capture`다.
3. **대표 사례:** `UCBalanceComponent`가 lifecycle state·serial·timer를 소유한다. Threshold는 요청 발생 기준, `CollapseIn Started`는 활성 권한, `CollapseIn Completed`은 Loop TTL 시작, `Reset Notify`는 회복 권한이다.

### 표현 제한

- Stagger stack 2·3만으로 모든 lifecycle 분기를 검증했다고 표현하지 않는다.
- Execution의 세션·결과·취소를 p.8에서 다시 설명하지 않는다.

## 3. p.9 Execution Collaboration

### 한 줄 메시지

Execution Pair가 게임 안에서 성립·실행되는 결과와, 그때 관찰해야 하는 최소 런타임 데이터를 보여 준다.

### 중심 구성

1. **Gameplay Hero:** Debug Overlay 없는 Standard 또는 Lethal Execution Pair 16:9 캡처(C06d). 실제 Pair 진행을 확보하기 전에는 슬롯으로만 표기하며, 성공 결과를 전제하지 않는다.
2. **런타임 증거 목업:** SessionId/Partner, Outcome/Reservation, Commit/Terminal, HP=1 preflight 네 슬롯은 모두 `Needs Capture`다.
3. **표현 경계:** Pair Transaction의 예약·Commit·Release 시퀀스 구조는 p.15에서만 설명한다. p.9에는 큰 구조 다이어그램을 배치하지 않는다.

### 표현 제한

- HP 1 Standard 사례는 사전 거절·회귀 방지 계약의 보조 데이터다. 실제 Pair 실행·Commit·Terminal을 이미 시각적으로 검증한 결과처럼 표현하지 않는다.
- Standard는 Balance Down/Recovery, Lethal은 Health Dead/Death lifecycle에 위임하는 책임 경계만 설명한다.

## 4. 필수 신규 캡처

| ID | 페이지 | 목적 | 상태 |
| --- | ---: | --- | --- |
| C06b | 8 | Debug Overlay 없는 Balance 누적 → Collapse Hero | Needs Capture |
| C06c | 9 | Pair SessionId·Reservation·Commit/Release Runtime crop | Needs Capture |
| C06d | 9 | Debug Overlay 없는 Standard 또는 Lethal Pair gameplay | Needs Capture |
| C06e | 9 | HP 1 Standard 예약 전 거절 log 또는 Overlay | Needs Capture |

## 5. 코드·문서 근거

- `Source/Portfolio/Component/CBalanceComponent.cpp`
- `Source/Portfolio/Component/CExecutionCollaborationComponent.cpp`
- `Source/Portfolio/Component/CCombatSignalTargetComponent.cpp`
- `Docs/05_System_Architecture/S35_UE5_Portfolio_Enemy_Balance_Collapse_Architecture.md`
- `Docs/05_System_Architecture/S36_UE5_Portfolio_Execution_Collaboration_Architecture.md`
