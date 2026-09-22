# 외부 체류 중 선행 작업 — 2026-09-22

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
