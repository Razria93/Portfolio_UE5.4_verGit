# Review 02 보완 진행 기록

상태: 목표 active, 구현 진행 중. 기준점 `dodge-review-01`은 변경하지 않는다. 사용자 시각 검토와 최종 기준점 고정은 아직 수행하지 않았다.

## 2026-09-22 선행 조사

- `9191896`에서 기존 파일이 없던 `Content/CharacterOverlay/Materials/M_Fresnel_D`와 `Material_Instance/MI_Fresnel`을 복원. Git의 과거 non-LFS 형식 경고가 있었으나 엔진 로드 성공. 원본 재질을 수정하지 않음.
- 읽기 전용 `CDodgePresentationAudit` 추가, 빌드 및 실행 성공(0 errors/0 warnings). 로그: `Saved/Logs/DodgePresentationAudit02.log`.
- 재질 파라미터 확인: Color, Fpower, BR, Alpha, Divide, Angle. Time/Sine과 PreSkinnedPosition 기반 계산이 존재하므로 파츠별 위치 기준 차이는 시각 검토 대상이다. Alpha는 Fresnel과 곱해진다. 최종 출력 연결과 MI override를 추가 확인한 뒤 런타임 MID에 적용한다.
- Stellar SCS의 HEAD/NECK/눈/치아/헤어 등 실제 이름 확인. CLOTH 템플릿은 현재 Asset=None. 네이티브 대표 메시와 무기 메시의 CDO 설정은 추가 조사 필요.
- `FCombatPresentationEnvelope` 및 실행별 상태를 추가: 지연/진입/유지/복귀, 옵션 Curve, sustain, 현재 강도에서 해제. 아직 런타임 효과에는 연결하지 않음.
- `DodgeEnvelope02Tests.log`: EnvelopePhases/EnvelopeRelease 2/2 통과, 프로세스 exit 0. 전체 회귀 검증 및 시각 검증은 아직 아님.
- 새 소스 추가로 Unity 묶음이 변경되자 기존 CDodgeAssets의 Save와 HUD 도구의 익명 namespace Save가 충돌. DodgeAssets::Save로 호출을 한정해 수정했고 Development 빌드 성공. 게임 동작 변경 없음.

## 다음 작업

### 2026-09-22 몸·무기 발광 연결 진행

- `DodgePresentationAudit03.log`: 원본 `M_Fresnel_D`는 Additive이며 Emissive=Color, Opacity는 Alpha가 곱해진 프레넬 출력. 복원된 `MI_Fresnel`의 Color=(40,0,3.937435). 원본 변경 없이 사용한다.
- Stellar 네이티브 메시=SK_CELESTE_SUIT 확인. 현재 기본 몸 적용 대상은 네이티브 메시 + HEAD/NECK. 눈/속눈썹/치아/머리카락은 초기 목록에서 제외하며, BP `PerfectBodyParts` 또는 `PerfectBodyPartTag`로 추가할 수 있다. 이 목록의 미적 적합성은 사용자 PIE 검토 전이다.
- `FCombatOverlayPlayback`: 메시별 MID와 기존 Overlay를 보관하고 Alpha를 보간한다. 중복 메시 제외, 다른 시스템이 덮어쓴 Overlay는 소유권을 포기하고 복원하지 않는다. UObject 소유 컴포넌트의 UPROPERTY에 상태를 저장한다.
- `CReactionFeedbackComponent`: 몸은 `PerfectBodyEnvelope`의 실시간 Delay/FadeIn/Hold/FadeOut, 무기는 별도 `CounterWeaponEnvelope`의 FadeIn 후 sustain. 현재 몸 기본값은 0/0.08/0.4/0.22초이며 몽타주 정규화 시간이 아니다. 무기는 0/0.08초 진입, 해제 후 0.18초 복귀; sustain 중 Hold는 사용하지 않는다. 필요하면 BlendCurve로 보간을 조절한다.
- 무기는 성공 당시 액터를 기억한다. 반격 기회 만료/취소 시 현재 강도에서 해제, 실제 러시 소비 성공 시 공격 수명으로 인계, CollisionWindowEnd 또는 러시 중단/종료 시 해제. 빗나가도 Collision 종료 기준이며 피해 발생 여부와 무관하다. 사망/무기 교체/EndPlay는 즉시 정리한다.
- `CDefenseComponent::IsCounterRushPresentationPending`은 입력 가능 여부와 구분되는 읽기 전용 조회다. 연결 시작 이전에도 준비 표현을 유지하며 일시적인 거리 초과는 표현을 끊지 않는다. 기존 실제 접촉 판정/무적/반격 입력 규칙은 변경하지 않았다.
- `CDodgeAssets -Apply`로 Default/Stellar BP에 몸·무기 원본 MI와 파츠 목록을 저장. 새 프로세스 read-only 검증 성공 (`DodgeGlowApply02.log`, `DodgeGlowVerify02.log`). 기존 임시 M_PerfectDodgeOverlay 파일은 삭제하지 않았지만 플레이어 연결은 원본 MI로 변경했다.
- Development 빌드 통과. 전체 Portfolio 자동 테스트 53/53 통과 (`DodgeGlow02Tests.log`), 이후 PerfectLifecycle에 준비 표현/거리 초과/소비/미적중 종료 검증을 추가했다. NullRHI 검증은 실제 렌더링 검증이 아니다.
- 추가 테스트 첫 실행은 빈 네이티브 무기의 HandGrip 소켓 부재로 ensure 실패했다(`DodgeGlowLifecycle02Tests.log`). 실제 BP_CWeaponActor_Sword를 사용하도록 픽스처를 수정한 뒤 전체 53/53 재통과(`DodgeGlow03Tests.log`, Error 없음). 프로세스 exit 0만으로 성공을 판단하지 않고 테스트 Result도 확인했다.
- 미완료: 슬로우 시작 Notify/보간, 비네트 보간, 러시 거리/캡슐 간격/디버그/고속 이동 연출, 디졸브 BP와 동시 사용 시 시각 검토, 최종 사용자 검토 및 커밋. 원본의 파츠별 PreSkinnedPosition 차이와 메시 경계는 영상으로 확인해야 한다.

### 이어서 할 작업

중간 코드 커밋: `4fa45af3`. 몸·무기 단계의 코드/에셋/문서 분리 커밋이며 최종 Review 02 승인 기준점은 아니다. `dodge-review-01` 태그는 그대로 유지한다. 원격 push는 수행하지 않는다.

1. 디졸브 BP의 실제 타임라인/이벤트와 동시 표현 시 충돌 여부 추가 확인.
2. 연결된 몸·무기 사용자 PIE 검토: HEAD/NECK 경계, 강도, 페이드, 반격 미사용/사용/빗나감/중단/무기 교체.
3. 몸·무기 단계 코드를 기준으로 수명/표현 회귀 보강 및 단계별 커밋.
4. 시작 Notify 슬로우/비네트 보간, 캡슐 러시 간격·디버그, 이동 연출을 순차 구현.
5. 회귀 테스트, 문서, 분리 커밋. 최종 시각 검토 전 완료 선언 금지.
