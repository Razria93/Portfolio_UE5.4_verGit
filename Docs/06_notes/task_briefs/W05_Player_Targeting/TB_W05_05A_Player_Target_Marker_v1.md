# TB W05-05A Player Target Marker v1

## 작업명

Player Target Marker v1

## 브랜치

```text
feat/player-targeting-component
```

## 상태

```text
완료
```

## 목적

Player의 `CurrentTarget`을 화면 공간 마커로 표시한다. 타겟 선택·유효성은 기존 `UCTargetingComponent`가 유지하고, HUD 생성과 화면 좌표 투영은 별도의 Presenter가 담당한다.

이번 단계는 Enemy Status HUD의 기반이 되는 Target HUD 뼈대와 실제 Widget Blueprint, Marker 이미지 자산 및 Enemy Socket 연결까지 구현한다.

## 책임 경계

```text
UCTargetingComponent
- CurrentTarget 선택 / 전환 / 해제
- OnTargetChanged 발행
- 타겟 생존 / 거리 유효성

UCTargetHUDPresenterComponent
- OnTargetChanged 구독
- Target HUD Widget 생성 / 제거
- 타겟 월드 위치를 DPI 보정된 Widget 좌표로 투영
- 화면 안 / 밖에 따른 Marker 표시 상태 결정
- FTargetMarkerViewData 전달

UCTargetHUDWidget
- 전달받은 ViewData 캐시
- Blueprint 구현 이벤트 발행
- 구체적인 이미지, 애니메이션, 레이아웃은 소유하지 않음

Widget Blueprint
- Marker 이미지와 시각 표현
- Canvas 내 Marker 위치와 Visibility 반영
```

Enemy Actor에는 `WidgetComponent`를 추가하지 않는다. Player HUD가 현재 선택된 단일 타겟을 표현하는 구조를 사용한다.

## 런타임 구조

```text
ACPlayerController
└─ UCTargetingComponent
   └─ OnTargetChanged
      └─ UCTargetHUDPresenterComponent
         ├─ World → Widget Position
         └─ UCTargetHUDWidget::UpdateTargetMarker()
            └─ BP_OnTargetMarkerUpdated()
```

`UCTargetHUDWidget`은 전체 화면에 한 번 생성하고 타겟 전환 때 재사용한다. Enemy A에서 Enemy B로 전환되어도 Widget을 제거하거나 새로 만들지 않는다.

## 데이터 계약

```cpp
USTRUCT(BlueprintType)
struct FTargetMarkerTuning
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = "Targeting|Marker")
    int32 WidgetZOrder = 10;
};

USTRUCT(BlueprintType)
struct FTargetMarkerViewData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    bool bVisible = false;

    UPROPERTY(BlueprintReadOnly)
    FVector2D WidgetPosition = FVector2D::ZeroVector;
};
```

초기 ViewData에는 표시 여부와 위치만 둔다. Target Actor나 Enemy Resource를 Widget에 직접 전달하지 않으며, 후속 Enemy Status HUD 데이터도 Presenter가 별도 ViewData로 변환해 전달한다.

Enemy는 다음 Target Presentation 계약을 소유한다.

```cpp
UPROPERTY(EditDefaultsOnly, Category = "Targeting|Presentation")
FName TargetMarkerSocketName = TEXT("TargetMarker");

UPROPERTY(EditDefaultsOnly, Category = "Targeting|Presentation")
FVector TargetMarkerFallbackOffset = FVector::ZeroVector;

FVector GetTargetMarkerWorldLocation() const;
```

## 투영 및 가시성 정책

- `UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(..., true)`를 사용해 DPI가 제거된 Widget 좌표를 얻는다.
- Presenter는 `ACEnemy::GetTargetMarkerWorldLocation()`이 제공한 위치만 투영한다.
- Enemy Mesh에 `TargetMarker` 소켓이 있으면 해당 소켓의 월드 위치를 사용한다.
- 소켓이 없으면 `Enemy ActorLocation + TargetMarkerFallbackOffset`을 사용한다.
- 현재 타겟이 있을 때만 Presenter Tick을 활성화한다.
- 타겟이 없으면 즉시 숨기고 Tick을 중지한다.
- Controller 또는 Pawn이 유효하지 않으면 숨긴다.
- 투영 실패 또는 카메라 뒤에 있는 타겟은 숨긴다.
- DPI 보정 Viewport 범위 밖이면 숨긴다.
- 화면 밖으로 나갔다고 `CurrentTarget`을 해제하지 않는다.
- 같은 타겟이 화면 안으로 돌아오면 마커를 다시 표시한다.
- v1에서는 화면 가장자리 Clamp와 Off-screen 방향 표시는 구현하지 않는다.

## 생명주기

```text
BeginPlay
→ Local PlayerController이며 Widget Class가 있으면 HUD Widget 생성
→ AddToViewport(WidgetZOrder)
→ 기존 CurrentTarget 상태 동기화

Target Selected / Switched
→ Presenter Tick 활성화
→ 동일 Widget에 새 좌표 전달

Target Cleared / Dead / Distance Invalid / Destroyed
→ Hidden ViewData 전달
→ Presenter Tick 비활성화

EndPlay
→ OnTargetChanged 구독 해제
→ Widget 숨김 / Viewport 제거
```

Widget Class가 지정되지 않은 상태는 에셋 연결 전 정상 상태로 취급하며 크래시하지 않는다.

## C++ / Blueprint 분리

### C++

- Target 변경 구독과 생명주기
- Widget 생성과 소유
- 월드 좌표 투영 및 Viewport 경계 검사
- ViewData 구성과 전달
- 타겟 해제 시 Tick 정지

### Blueprint

- `WBP_TargetHUD` 생성 및 `UCTargetHUDWidget` 상속
- 전체 화면 Canvas 구성
- 중앙 정렬된 Marker 이미지 구성
- `BP_OnTargetMarkerUpdated`에서 위치와 Visibility 반영
- PlayerController의 Presenter에 Widget Class 지정

## 제외 범위

- Enemy Name / HP / Balance HUD
- Player BU / BE / HP / SH HUD
- Segment Bar 시각 자산
- Off-screen 방향 표시
- 화면 가장자리 Clamp
- Marker 애니메이션과 사운드
- Enemy별 WidgetComponent

## 구현 및 검증 기록

- `FTargetMarkerTuning`과 `FTargetMarkerViewData`를 Targeting 공용 타입에 추가했다.
- `UCTargetHUDWidget`이 ViewData를 캐시하고 Blueprint 구현 이벤트로 전달한다.
- `UCTargetHUDPresenterComponent`가 Target 변경 이벤트 구독, Widget 수명, 화면 좌표 투영과 표시 상태를 담당한다.
- Target Marker의 월드 기준점은 Enemy가 소유하며, Presenter는 Mesh나 Socket 구조를 직접 알지 않는다.
- Enemy별 `TargetMarker` 소켓을 우선 사용하고 누락 시 Enemy 설정의 fallback offset으로 복구한다.
- Socket 기반 위치 API 추가 후 UHT 및 `PortfolioEditor Win64 Development` 빌드가 성공했다.
- 타겟이 카메라 뒤에 있거나 DPI 보정 Viewport 범위 밖이면 마커만 숨기고 Target Lock은 유지한다.
- 타겟이 없거나 Widget Class가 지정되지 않은 경우 Presenter Tick을 실행하지 않는다.
- `ACPlayerController`가 Presenter를 Default Subobject로 소유하고 기존 TargetingComponent 참조를 주입한다.
- Runtime 모듈의 Public Dependency에 `UMG`를 추가했다.
- 첫 빌드에서 UE 5.4 `GetViewportSize()`의 비-const World Context 계약을 확인하고 PlayerController를 전달하도록 수정했다.
- UHT 및 `PortfolioEditor Win64 Development` 재빌드가 성공했다.
- `BP_CTargetHUDWidget`에 전체 화면 Canvas, 중앙 정렬 Marker Image, Visibility와 Canvas Slot 위치 갱신을 연결했다.
- 흰색 원형 `T_TargetMarkerDot` 자산을 Marker Image Brush에 연결하고 최종 색상과 투명도는 Widget에서 조정하도록 구성했다.
- Enemy Skeleton의 `TargetMarker` 소켓을 기준점으로 사용해 애니메이션 중에도 마커가 Enemy를 추적하도록 구성했다.
- 1280x720 미리보기의 DPI Scale 0.67 환경에서 DPI가 제거된 Widget Position을 Canvas 좌표로 그대로 사용하고 위치가 일치하는 것을 확인했다.
- PIE에서 Target 선택, 전환, 해제, 화면 밖 숨김, Lock 유지와 화면 재진입 표시가 정상 동작하는 것을 확인했다.

## 예상 변경 파일

```text
Docs/06_notes/task_briefs/W05_Player_Targeting/README.md
Docs/06_notes/task_briefs/W05_Player_Targeting/TB_W05_05A_Player_Target_Marker_v1.md

Source/Portfolio/Portfolio.Build.cs
Source/Portfolio/Type/CTargetingTypes.h
Source/Portfolio/Character/Enemy/CEnemy.h
Source/Portfolio/Character/Enemy/CEnemy.cpp
Source/Portfolio/UI/CTargetHUDWidget.h
Source/Portfolio/UI/CTargetHUDWidget.cpp
Source/Portfolio/Component/CTargetHUDPresenterComponent.h
Source/Portfolio/Component/CTargetHUDPresenterComponent.cpp
Source/Portfolio/Controller/CPlayerController.h
Source/Portfolio/Controller/CPlayerController.cpp
```

## 완료 조건

- C++ Target HUD Widget과 Presenter가 빌드된다.
- Target 선택 시 Presenter가 동일 Widget에 Visible ViewData를 전달한다.
- Target 전환 시 Widget을 재생성하지 않고 위치만 갱신한다.
- Target 해제·사망·거리 초과·Destroy 시 Hidden ViewData를 전달하고 Tick을 중지한다.
- 타겟이 화면 밖이면 마커만 숨고 Target Lock은 유지된다.
- 화면 안으로 돌아오면 마커가 다시 표시된다.
- DPI Scale이 1이 아닌 환경에서도 Marker 좌표와 Viewport 경계를 같은 좌표계로 비교한다.
- Widget Class 미지정 상태에서 크래시하지 않는다.
- `PortfolioEditor Win64 Development` 빌드가 성공한다.

## 에디터 구성 및 PIE 완료 기록

```text
1. `UCTargetHUDWidget` 기반 `BP_CTargetHUDWidget` 생성
2. 전체 화면 Canvas와 Marker Image 배치
3. `BP_OnTargetMarkerUpdated`에서 Visibility와 Canvas Slot Position 반영
4. PlayerController의 TargetHUDPresenter에 Widget Class 지정
5. Enemy Skeleton에 `TargetMarker` 소켓 생성 및 위치 조정
6. PIE에서 선택 / 전환 / 해제 / 화면 밖 숨김과 재진입 표시 검증 완료
```

## 후속 작업

```text
별도 UI 후속 작업: Enemy Status HUD
```
