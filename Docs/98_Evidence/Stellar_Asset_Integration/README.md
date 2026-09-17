# Stellar 에셋 통합 근거 목록

## 기준

2026-09-17 조사. 최종 Content 기준 `62378b3f`, 후속 종료 계약 `051a5884`, Fix 문서 `93c4ba22`.
코드·에셋 경로 존재, 당시 대화, 사용자 PIE 확인은 서로 다른 근거다. 자동 감사는 전체 런타임 안전을 보장하지 않는다.

## 주장별 근거

| ID | 문제·사용자 의도 | 당시 구현 → 최종 재구성 근거 | 현재 구현/에셋 | 적용·검증 상태 | 한계 |
| --- | --- | --- | --- | --- | --- |
| E01 | 작은 체구의 상체 눌림을 줄이고 새 캐릭터 세트 사용 | `a6e79e34` → `13299da5`; 세션 A와 후속 사용자 화면 | 임시 import 프로젝트 RTG_Celeste/RTG_Celeste_Move, 최종 StellarGirlCeleste와 BP_CPlayer_Stellar | Action/Movement 분리, Spine 0.7, Additive 미사용 확인; 최신 Player `62378b3f` | 화면 밖 체인과 전체 export 설정까지 독립 검증한 것은 아님 |
| E02 | 장탈착 이격과 손 전환을 연속 표현 | `bc9c396d` → `16c54976`, `16e00a4b`; A:472 | `Source/Portfolio/Component/CWeaponComponent.cpp`, Socket NotifyState | 현재 구현 확인 / 사용자 PIE 확인 | 당시 이격 Before 화면 별도 확보 필요 |
| E03 | 메시 원점 대신 손잡이 기준 회전, 충돌체 동반 변환 | 같은 구현 계보; A:621,1594,1748 | `Source/Portfolio/Weapon/CWeaponActor.cpp`, HandGrip, Pivot/ContentRoot | 현재 구현 확인 / 사용자 PIE 확인 | 수학 테스트 존재와 실행 성공은 별개 |
| E04 | 콤보 임시 변환 복구와 영구 장착 유지 | `bc9c396d` → `16c54976`; A:2144 | Action Pose Scope, CommitEquipWeapon/CommitUnequipWeapon | 현재 구현 확인 / 사용자 PIE 확인 | MontageEnd 자체가 복구 주체라는 설명 금지 |
| E05 | Trail 설정 분산과 이중 매핑 제거 | `bc9c396d` → `16c54976`; B:5,62 | CWeaponTrailDataAsset, CWeaponActor, CWeaponTrailComponent | 현재 구현 확인 / 당시·현재 사용자 동작 확인 | Niagara 파생 Component 자체를 manager로 설명하지 않음 |
| E06 | 환경 맵을 통합하고 충돌·조명 구성 | `c543cfa3` → `13299da5` | `Content/SciFi/SciFiMap.umap` | 도입 이력·파일 확인 / 사용자 작업 진술 | 간단한 내역으로 한정. 상세 원인·설정은 주장하지 않고 추가 증거 수집 종료 |
| E07 | 필요 에셋 보존과 경로 정리 | `a6e79e34`, `90ba6748`, `8d6905bf` → `13299da5` | `T_CELESTE_FlatNormal.uasset`, Content 새 경로 | 세션 C와 사용자 제공 자체 Inspector Referencers 화면으로 조회·보존 판단 확인 | material graph는 당시 도구 기록; CSV export 실행은 주장하지 않음 |
| E08 | 기존 Blueprint의 필드 참조 보존 | `16c54976` | `Config/DefaultEngine.ini` Weapon PropertyRedirects | 현재 구현 확인 | enum 값 이동과 별개 조치 |
| E09 | 현재 Unreal 에셋 LFS 정책 정규화 | `ace28d4c` | `.gitattributes` | 당시 528개 포인터 정규화 이력 | 과거 전체 이력 재작성 아님; 현재 원격 무결성 미검증 |
| E10 | 잘못된 Complete 설정이 자동 종료에 가려짐 | `15271535` → `8b23f642`; 이후 `af6778aa`, `051a5884` | CAction/CReaction, CExecutionMontageAuditCommandlet | 감사 결과·사용자 PIE 확인 | 감사 범위는 direct Notify/CDO 데이터에 한정 |
| E11 | 안전한 이주 검증 | `16c54976`, `af6778aa`, `051a5884` | PortfolioEditor Tests/Audit | 도구 코드와 감사 로그 확인 | 로드 성공은 실행 성공과 다름 |

위 커밋 대응은 의미상 구현 계보이며 1:1 cherry-pick 또는 tree 동일성을 뜻하지 않는다. 커밋별 파일 수를 합산해 고유 에셋 수로 주장하지 않는다.

## 세션 출처 식별

원본은 로컬 `C:/Users/starb/.codex/sessions/2026/09/`에 있다. 공개 저장소에 세션 원문·개인정보를 복사하지 않는다. 아래 파일명과 줄은 내부 재조회용이며 독자는 위 구현 경로·커밋·보고서로 내용을 확인할 수 있다.

- A: `13/rollout-2026-09-13T17-38-04-01a099ea-9a06-79b1-8d8e-ee287a39a219.jsonl`
- B: `09/rollout-2026-09-09T16-41-16-01a07189-4915-7763-a111-41c23ed79afd_01a0851d-2c20-75e2-b47c-44092c53ffa6.jsonl`
- C: `15/rollout-2026-09-15T00-06-05-01a099ea-9a06-79b1-8d8e-ee287a39a219_01a0a074-3485-7562-9c9a-2925fb96182a.jsonl`
- 프리뷰 제약: `15/rollout-2026-09-15T14-49-03-01a099ea-9a06-79b1-8d8e-ee287a39a219_01a0a39c-9634-72a0-a639-1627c51d7002.jsonl:97`
- 당시 Trail 재생 확인: `15/rollout-2026-09-15T20-07-58-01a0a4c0-903c-7d70-a05a-1782d172a01e.jsonl:181`

## 로컬 검증 기록

- [사용자 첨부 화면의 설정·관찰 기록](Capture_Record.md): 원본 이미지 파일의 저장소 포함 여부와 입증 범위를 명시.

- [추가 세션 조사·로컬 검증·최소 사용자 확인](Session_Search_and_Verification.md)

- UBT `Log.txt`: 두 수정 cpp 컴파일, Portfolio/PortfolioEditor 링크, WriteMetadata 8/8 완료를 재확인. 이번 문서 작업에서 빌드를 재실행하지 않음.
- `Saved/Logs/Portfolio-backup-2026.09.17-08.49.08.log:829`: 1,399 packages 로드, 실패 0, 감사 Errors 0 / Warnings 0, DataRows 93.
- 같은 로그: 49 Montage / 151 직접 Notify, commandlet result 0. 별도 legacy material import 경고 2개 존재.
- 사용자 최종 PIE 정상 확인은 세션의 수동 검증 근거. 원시 실행 영상은 현재 문서에 포함하지 않음.
- [F08](../../04-02_Fix_Pull_Request/F08_UE5_Portfolio_Pull_Request_Fix.md)에 검사 범위·비범위를 기록함.

---
