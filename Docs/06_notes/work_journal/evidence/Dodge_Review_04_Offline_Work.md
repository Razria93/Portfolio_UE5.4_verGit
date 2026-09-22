# 외부 체류 중 선행 작업 — 2026-09-22

## 재개 후 검증 완료 (아래 초안/차단 기록을 대체하는 현재 상태)

사용자의 프로젝트 종료 통보 후 Portfolio 에디터 종료를 프로세스로 확인했다. 별도 Temp_AssetImport 에디터는 건드리지 않았다. PortfolioEditor Development DLL 링크까지 성공했고, 최종 빌드로 전체 자동 테스트 **60/60 통과** (`Saved/Logs/OfflineResumeRegressionFinal.log`). 새 `Portfolio.Combat.Dodge.MontageWindows`도 통과했다. 아래 미검증/차단 설명은 재개 이전의 이력이다.

검사 도구의 추가 원인은 `FParse::Value` 기본 동작이 `Roots=`의 쉼표에서 값을 끊는 것이었다. separator 중단을 해제하고 경로 정규화, 존재하지 않는 루트 및 빈 검사 실패 처리를 적용했다. 최종 검사는 **패키지 12/12 로드, 몽타주 5개, 노티파이 17개, 오류/경고 0** (`OfflineResumeAuditFinal.log`). 유효 경로 뒤에 없는 경로를 붙인 음성 검사는 Main 반환값 2로 실패함을 확인했다 (`OfflineResumeInvalidRoot.log`). 무기 발광 좁은 Verify 검사도 에셋 변경 없이 통과했다 (`OfflineResumeGlowVerify.log`). 최초 `OfflineResumeAudit.log`는 첫 경로만 읽은 결과이므로 최종 범위 검증으로 사용하지 않는다.

### 새 판정 구간 기능의 사용 범위

- `DefenseComponent`의 `DodgeTuning.bUseMontageWindows`는 기본 false로 유지한다. 이번 작업은 에셋에 표식을 넣거나 옵션을 켜지 않았다.
- 향후 활성화 시 일반 Dodge 몽타주 자체에 `Dodge Judgment Window` Notify State를 두고 `Kind`를 `Invulnerability`와 `Perfect`로 각각 설정한다. 성공 후 PerfectDodge 리액션 몽타주의 면역을 편집하는 기능은 아니다.
- 시작 포함/끝 제외, 여러 동일 종류 구간은 합집합. Perfect는 Invulnerability와 겹치는 구간에서만 인정한다. 누락/유효하지 않은 구간은 방어를 부여하지 않는다.
- Notify Begin/End 전달로 bool을 켜고 끄지 않고, 현재 실행이 소유한 몽타주의 위치를 조회하므로 중단 후 구간 플래그가 남지 않는다. 무기 발광의 단발 Notify 설계와는 별개다.
- 기존 시간 설정을 복사해 표식을 배치하고 비교 검증한 뒤 활성화해야 한다. 현재 BP 판정/발광/러시 에셋은 변경하지 않았다.

### 아직 남은 작업

몸 발광의 Notify 전환, 실제 회피 판정 표식 배치/활성화, 카운터 전용 Niagara 연결은 미완료다. 이번 60개 테스트 통과는 해당 미구현 기능이나 시각 품질의 검증을 의미하지 않는다. 사용자 PIE 기준 태그 `dodge-pie-review-03`는 이동하지 않는다.

## 완료한 기준점

`dodge-pie-review-03` 태그 → `6609620a`. 사용자 PIE 정상 경로 보고, 저장된 에셋 변경 전체, 테스트 맵, 검사 기록을 보존했다. 에셋 커밋 이후 현재 DLL로 전체 자동 테스트 59/59 통과 (`CheckpointRegression.log`). 해당 태그에는 아래 초안이 포함되지 않는다.

## 후속 소스 초안 (아직 미커밋·런타임 미검증)

1. `CExecutionMontageAuditCommandlet`: 디렉터리와 파일 경로를 절대 경로/슬래시로 정규화하고 경계 검사, 파일이 없는 검사 실패 처리. 기존 도구가 0개를 검사하고 성공하던 결과는 검증 증거에서 제외.
2. `CDodgeAssetsCommandlet`: 현재 몽타주/CounterRush_Locked 경로 사용. 삭제된 원본 경로의 자동 재생성 제거. 전체 bootstrap Apply는 명시적 `-LegacyBootstrapApply` 필요. 좁은 HairOnly/WeaponGlowOnly 작업은 기존 동작 유지.
3. `CAnimNotifyState_DodgeWindow`: 몽타주에 Perfect/Invulnerability 구간을 배치하는 편집용 표식. Begin/End 콜백으로 무적 bool을 유지하지 않고 현재 소유 몽타주 재생 위치로 판정.
4. `FDefenseDodgeTuning::bUseMontageWindows`: 기본 false. 기존 BP와 판정 숫자는 유지되며 자동 전환하지 않는다. true인 경우 표식 누락은 방어 실패이며 숫자로 암묵적 복귀하지 않는다. 퍼펙트 판정은 무적 구간 내부에서만 가능.
5. `Portfolio.Combat.Dodge.MontageWindows` 테스트 초안: 기존 모드 보존, 표식 누락, 경계, 퍼펙트/무적 교집합, 1회 성공, 중단 소유권 검사.

## 빌드 차단과 정확한 검증 상태

변경 소스와 UHT 컴파일 단계는 진행됐으나 `UnrealEditor-Portfolio.dll`, `UnrealEditor-PortfolioEditor.dll` 링크가 LNK1104로 실패했다. 프로젝트 에디터 PID 44260 실행 및 DLL 잠금 확인. 에디터 종료 여부를 한 번 잘못 판단해 사용자에게 정정했다. 임의 종료·저장·Live Coding은 수행하지 않았다.

이후 `CheckpointExecutionFixed.log`는 이전 DLL로 실행된 것이므로 수정된 도구의 검증 증거가 아니다. 새 테스트도 실행되지 않았다. **59/59는 기준점 결과이며 후속 소스 초안의 통과 결과가 아니다.**

## 재개 순서

1. 사용자가 프로젝트를 저장하고 에디터를 종료한 것을 프로세스로 확인.
2. PortfolioEditor Development 빌드 및 전체 테스트 재실행. 새 테스트 포함 예상 60개이나 실제 결과 개수로 판정.
3. 실제 패키지/몽타주 검사 개수가 0보다 큰지 확인하고 오류 항목 검토. 좁은 에셋 검증 재실행.
4. 소스 초안을 검토·커밋. 이후 몸 발광 Notify 전환과 트레일 독립화 진행.

현재 체감을 바꾸는 에셋 수정은 하지 않았다. 몸 발광 전환·카운터 전용 Niagara 복제/연결·판정 표식 실제 배치와 활성화는 아직 미완료다. 실시간 Envelope와 슬로우가 적용된 몽타주 시간은 다르므로 몸 발광 End 위치를 기계적으로 환산하지 않는다.
