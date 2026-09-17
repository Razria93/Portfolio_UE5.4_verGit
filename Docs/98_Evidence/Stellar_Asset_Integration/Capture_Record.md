# 사용자 제공 화면 기록

현재 문서화 세션에서 사용자가 제공한 화면을 직접 판독한 기록이다. 캡처 촬영 날짜는 확정하지 않는다. 로컬 attachments 검색에서는 해당 PNG 원본을 찾지 못하여 이미지 파일 자체는 저장소에 포함하지 않았다. 존재하지 않는 이미지 링크를 만들지 않고 아래에 관찰 내용을 기록한다. 원본 파일이 확보되면 공개 문서에 삽입할 수 있으나 추가 검증의 필수 조건은 아니다.

## 1. Placeholder 참조 조회 화면

- 사용자가 실제 문제 해결 당시 사용한 자체 플러그인 화면이라고 확인.
- 창 제목: Asset Reference Inspector.
- 대상: `/Game/StellarGirlCeleste/Demo/Textures/Placeholder_Normal`.
- Referencers / Max Depth 2 / Path Filter `/Game/`.
- 결과 Tree: M_CELESTE_HEAD, MI_CELESTE_HEAD_01/02, SK_CELESTE_HEAD_01 등.
- 입증 범위: 자체 플러그인의 해당 에셋 참조 조회 활용. Material 내부의 실제 사용 판단은 별도 과거 도구 기록과 연결. CSV export 실행 여부는 화면으로 확정하지 않음.

## 2. Retargeter 화면 5장

| 화면 | 관찰 내용 |
| --- | --- |
| 1 | RTG_Celeste_Move / Run_F_Anim / Spine / Interpolated / Rotation Alpha 0.7 / Translation None |
| 2 | RTG_Celeste_Move / Run_F_Anim / Head / Rotation Alpha 0.0 |
| 3 | RTG_Celeste / Attack_Combo_01_02_Anim / Spine / Rotation Alpha 0.7 |
| 4 | RTG_Celeste / Attack_Combo_01_02_Anim / Neck / Rotation Alpha 0.8 |
| 5 | Idle_Anim_Temp의 흉부 접힘 예시. 사용자가 보정 전 상태로 지정 |

사용자 확인: Additive Layer Track 미사용. Action과 Movement를 분리해 리타게팅하고 흉부의 접힘을 펴는 방향으로 조정함. 서로 다른 애니메이션 화면을 동일 프레임의 정량 Before/After로 제시하지 않음.

## 3. 연결

- [I03 리타게팅](../../03_Issue_Analysis_Report/I03_UE5_Portfolio_Issue_Analysis_Report.md)
- [I08 참조 정리](../../03_Issue_Analysis_Report/I08_UE5_Portfolio_Issue_Analysis_Report.md)

---
