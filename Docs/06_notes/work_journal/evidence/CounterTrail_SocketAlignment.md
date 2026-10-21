# Counter trail socket attachment — 2026-09-23

> 후속: 이 문서는 Scifi 단계의 기록이다. 이후 LightSaber 선택과 검증 도구 보완은 [Review 05](Dodge_Review_05_Audit_Refinement.md), 현재 설정은 [저작 가이드](../../N_CounterRush_Current_Authoring.md)를 참고한다.

## 교정한 문제

이전 미커밋 구현은 SetAbsolute(true,true,true)로 자동 추종을 끄고 UNiagaraComponent::TickComponent에서 두 소켓의 월드 위치로 트레일을 갱신했다. Niagara의 non-Solo 실행에서는 개별 컴포넌트 Tick에 의존할 수 없다. 기존 테스트는 UpdateBladeAlignment를 직접 호출했으므로 실제 런타임 추종을 입증하지 못했다. 이전 62개 테스트 통과를 시각적 해결의 증거로 해석하면 안 된다.

데모 저장 데이터에는 TimedNiagaraEffect가 FX_Slash에 부착되는 설정이 있었다. 다만 임포트된 Kwang 메시에서 그 이름이 유효한 소켓/본으로 확인되지 않았으므로 데모의 정상 재생을 확인했다고 주장하지 않는다.

## 현재 방식

- CWeaponTrailComponent: 개별 Tick/Absolute 변환/매 프레임 Position Offset 덮어쓰기를 제거했다. ConfigureAttachment에서 무기 메시의 지정 소켓에 직접 부착한다.
- CWeaponTrailDataAsset: bAttachToSocket, AttachmentSocket, RelativeLocation/Rotation, bOverrideRibbonWidth, RibbonWidthParameter, RibbonWidth를 노출한다. 기본은 기존 메시 원점 부착이며 폭 덮어쓰기도 꺼져 있다.
- CWeaponActor: 생성 때 부착 설정을 적용하고 시작 때 유효 소켓을 확인한다. 잘못된 소켓은 숨긴 상태로 유지한다. 기존 ActionFeedback 시작/종료와 DeactivateAllTrails 경로를 유지한다.
- 기존 bAlignToBladeSockets, BladeStart/EndSocket, PositionOffsetParameter와 수동 갱신 테스트는 폐기했다. 이전 Saved/apply_trail_alignment.py는 더 이상 사용하지 않는다.

## 에셋 설정

변경 대상은 scifi_sword_Skeleton과 DA_WeaponTrail_SciFiSword의 CounterRush_Trail이다.
NS_SlashTrail_Scifi_Sample, 카운터 몽타주, 기존 콤보 선택은 수정하지 않았다.

전용 FX_CounterTrail:
- Bone: Body3_ncl_1
- 본 기준 위치: (0, 0, 약 15.568)
- 로컬 Z: 칼날 방향
- 스케일: 1

기존 TrailStart는 참조 포즈 메시 좌표 원점, TrailEnd는 메시 Y=35였다. 메시 bounds의 Y 최대는 약 31.135여서 기존 가이드의 끝을 bounds에 맞춰 줄인 중간점을 초기 소켓 위치로 사용했다. 기존 소켓 자체는 보존했다. 이는 메시 bounds 기반 초기값이며 실제 날이 시작하는 위치를 GPU 화면으로 확인한 결과는 아니다.

CounterRush_Trail:
- Attach To Socket: true
- Attachment Socket: FX_CounterTrail
- Relative Location/Rotation: 0
- Override Ribbon Width: true
- Ribbon Width Parameter: User.Trail Width
- Ribbon Width: 약 31.135 cm (무기 BP 메시 기본 스케일 1 기준)
- Niagara 선택: NS_SlashTrail_Scifi_Sample 유지

폭은 월드 공간 숫자이며 런타임 캐릭터/무기 스케일 변화에 자동 비례하지 않는다. 최종 PIE의 무기 크기에 맞춰 DA에서 조정한다. Niagara의 User.Position Offset은 이제 그대로 적용되므로 소켓/DA 오프셋과 중복 보정하지 않는다.

## 검증과 제한

- PortfolioEditor Development 빌드 성공.
- SocketAttachment: 저장된 실제 소켓, Tick 비활성 상태의 위치/회전/스케일 추종, 디자인 오프셋, 없는/빈 소켓 차단, 기존 메시 원점 부착을 검사한다.
- FeedbackLifecycle: 실제 무기 BP에서 트레일 생성 수, 시작/종료 요청, 재시작 시 컴포넌트 재사용, 콤보 분리, DeactivateAllTrails 정리를 검사한다.
- NullRHI에서는 GPU 리본 화면을 판정할 수 없다. 골격 애니메이션과 GPU 시뮬레이션 간 프레임 지연, 리본 폭/밀도/꼬리 모양의 최종 판정은 PIE가 필요하다.
- 정상 종료는 기존 즉시 숨김 방식이다. 잔상 페이드 정책은 이번 작업 범위에 포함하지 않았다.
- 전체 회귀 62/62 통과. 최종 로그: Saved/Logs/TrailAttachmentFinal.log.
- 실제 무기 파괴 시 트레일 컴포넌트 등록 해제까지 검사했다. 테스트 월드 컨텍스트 경고도 최종 실행에서 해소했다.
- 적용 로그: Saved/Logs/TrailAttachmentApply.log.
- 변경 전 백업: Saved/Backups/TrailAttachment/ (DA, Skeleton).
- 커밋하지 않음.

## PIE 확인 및 조정 순서

1. 에디터를 새로 열고 카운터 러시를 실행한다.
2. 과거에 남은 리본 전체가 아니라 **새로 생성되는 리본의 폭 방향과 중심이 검에 맞는지** 확인한다.
3. 일반 콤보는 기존 트레일인지, 카운터 종료/취소 후 효과가 남지 않는지 확인한다.
4. 위치/축: scifi_sword 스켈레톤의 FX_CounterTrail 또는 DA CounterRush_Trail의 Relative Location/Rotation.
5. 폭: DA CounterRush_Trail의 Ribbon Width. 캐릭터/무기에 추가 스케일이 있으면 그 크기를 반영한다.
6. Niagara User.Position Offset은 추가 오프셋이다. 폭/중심/방향이 맞은 다음에 수명·밀도를 조정한다.
7. 이전 portfolio.WeaponTrail.DebugBlade 명령은 제거했다. 현재 소켓의 축/위치는 스켈레톤 에디터에서 확인한다.

## 재현 명령

헤드리스 에디터에서 -run=CDodgeAssets -CounterTrailAttachment -Verify 로 저장 설정을 확인할 수 있다.
-Apply는 전용 소켓이 없으면 생성하고 카운터 DA의 오프셋/폭을 초기화하므로, 사용자 튜닝 후에는 재실행하지 않는다.
기존 전용 소켓이 있다면 위치/회전을 덮어쓰지 않는다.
