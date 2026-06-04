# Action / Reaction Intervention Rule 정리 및 Runtime Cleanup 보강

## 제목

`♻️ refactor: action / reaction intervention rule 정리 및 runtime cleanup 보강 (#52)`

## 요약

- 이번 PR에서는 **Action / Reaction intervention 판단 구조를 data rule 기반으로 정리**하고, 실제 중단 결과를 `Interrupted`로 통합함.

- 핵심 방향은 다음과 같음.

	- Action / Reaction intervention filter를 notify 주입 방식에서 `ActionData` / `ReactionData` rule 방식으로 이전함.

	- `Cancel` 계열 stop / finish / feedback timing을 제거하고 실제 중단 결과를 `Interrupted`로 통합함.

	- `WantInterventionRules`와 `AllowInterventionRules`를 분리하여 incoming 의사와 active 허용 정책의 책임을 명확히 함.

	- `AllowInterventionWindow`를 `WindowKey` 기반 runtime state로 정리하고, notify는 window timing만 전달하도록 축소함.

	- Dodge를 실제 action intervention 검증 예제로 구성하고, 관련 timing / allow window asset data를 갱신함.

	- action / reaction finish 시 trail, collision, hit context 같은 runtime side effect가 남지 않도록 cleanup 경로를 보강함.

	- feedback request 생성과 실행을 `BuildFeedbackRequest()` / `PlayFeedbackRequest()`로 분리하여 finish 이후 feedback 실행 순서를 명확히 함.

- 또한 intervention allow window / notify range 문제와 runtime effect cleanup 문제를 bug report로 문서화함.

---

## 완료 작업

### 1. Intervention Rule 데이터화

- 기존 notify가 owner / counterpart filter를 직접 열고 닫던 구조를 제거함.

- `FExecutionInterventionWantRule`과 `FExecutionInterventionAllowRule`을 추가함.

- `FActionData` / `FReactionData`에 다음 rule 배열을 추가함.

	- `WantInterventionRules`

	- `AllowInterventionRules`

- `WantIntervention()`은 incoming context의 want rule을 기준으로 active participant를 매칭하도록 정리함.

- `AllowIntervention()`은 active data의 allow rule을 기준으로 incoming participant를 매칭하도록 정리함.

- Window timing은 Allow rule에서만 평가하도록 정리함.

### 2. Allow Intervention Window 정리

- `CAnimNotifyState_ExecutionInterventionWindow`의 책임을 `WindowKey` begin / end 전달로 축소함.

- notify class / file 이름은 asset 안정성을 위해 유지하되, 표시 이름은 `Allow Intervention Window`로 정리함.

- `ActiveInterventionWindowKeys` 계열 이름을 실제 의미에 맞게 `AllowInterventionWindowKeys`로 정리함.

- Action / Reaction component의 window handler를 `HandleActionAllowInterventionWindowBegin/End`, `HandleReactionAllowInterventionWindowBegin/End`로 정리함.

- active executor는 열린 `WindowKey`를 runtime set으로 관리하고, `AllowInterventionRules`의 `Window` timing 평가에만 사용함.

### 3. Cancel / Interrupt 통합

- `EExecutionStopReason::Cancelled`를 제거함.

- Action / Reaction stop reason, finish reason, feedback timing의 `Cancel` 계열을 제거함.

- 외부 실행이 active execution을 중단하는 결과는 `Interrupted`로 통합함.

- `EExecutionApplyMode::Intervene`는 개입 적용 의미로 유지하고, 실제 stop reason은 directive의 `Interrupted`가 담당하도록 정리함.

### 4. Reaction 특수 정책 정리

- `UCReaction_Hit::WantIntervention()` override를 제거하고 data rule 방식으로 편입함.

- `UCReaction_Dead::WantIntervention()` override를 제거함.

- Dead reaction의 force intervention 정책은 orchestrator에서 유지함.

- `UCReaction_Dead::AllowIntervention()`은 active dead reaction이 다른 incoming execution에 의해 끊기지 않도록 막는 terminal guard로 유지함.

### 5. Runtime Effect Cleanup 보강

- `UCAction::CleanupRuntimeEffects()`와 `UCReaction::CleanupRuntimeEffects()`를 추가함.

- `UCWeaponComponent::ClearRuntimeWeaponState()`를 추가하여 hit context와 collision runtime state를 정리하도록 함.

- `UCActionFeedbackComponent::ClearRuntimeFeedback()`을 추가하여 weapon trail을 강제 OFF할 수 있도록 함.

- `UCReactionFeedbackComponent::ClearRuntimeFeedback()`을 no-op hook으로 추가하여 향후 loop VFX / SFX cleanup 확장 지점을 맞춤.

- action / reaction `Stop()`과 `Complete()`에서 runtime cleanup 이후 내부 runtime state를 clear하도록 정리함.

### 6. Feedback Request Snapshot 분리

- 기존 `RequestFeedback()`를 제거함.

- 모든 feedback 호출을 `BuildFeedbackRequest()` + `PlayFeedbackRequest()`로 통일함.

- terminal feedback은 cleanup / clear 전에 request snapshot을 먼저 생성하고, cleanup / clear 이후 실행하도록 정리함.

- Action event payload도 `ClearRuntime()` 전에 필요한 값을 캡처하여 clear 이후에도 올바른 action index를 전달하도록 정리함.

### 7. Dodge / Asset / Data 변경

- Dodge가 active action을 `Interrupted` intervention으로 끊고 실행될 수 있도록 data rule과 allow window를 설정함.

- Dodge timing, effect socket name, allow intervention window asset data를 갱신함.

- Player / Enemy BP의 intervention rule data를 갱신함.

- Hit reaction montage와 sword attack montage의 allow window / timing 설정을 조정함.

- Quinn skeleton socket 변경을 반영함.

### 8. Bug Report 문서 추가

- `B09`: Action / Reaction intervention window 설정 누락 및 notify 구간 문제로 active execution이 중단되지 않는 문제를 문서화함.

- `B10`: Action trail / collision runtime effect가 hit 또는 dead interrupt 이후 정리되지 않는 문제를 문서화함.

---

## 검증 내역

### 빌드

- `PortfolioEditor Win64 Development` 빌드 성공함.

```text
Target is up to date
Total execution time: 0.54 seconds
```

### 정적 확인

- 코드 검색 기준 `Cancelled`, `RequestFeedback`, 구 intervention window API 잔여 없음.

- `Cancel` 잔여 검색 결과는 `UCHealthComponent::TryCancelRevive()`만 남아 있으며 이번 orchestration 변경과 무관함.

- `git diff --check` 통과 필요함.

### 수동 검증

- sword equip / unequip 정상 동작 확인함.

- 무기 장착 상태에서 combo attack 0-1-2 chain 정상 동작 확인함.

- 무기 미장착 상태에서 combo attack 거부 확인함.

- combo chain window 밖 입력이 reserve되지 않는지 확인함.

- Dodge가 허용된 allow window에서 active action을 interrupt하고 실행되는지 확인함.

- Hit reaction이 action interrupt 가능 구간에서 정상 개입하는지 확인함.

- Dead reaction이 active action / reaction 여부와 무관하게 최종 우선 실행되는지 확인함.

- collision / hit context / allow intervention window notify begin / end 정상 동작 확인함.

- trail ON 중 hit / dead interrupt 이후 trail, collision, hit context가 정리되는지 확인함.

- reaction interrupt / complete feedback이 snapshot 기반 실행 경로에서 정상 재생되는지 확인함.

---

## 리뷰 포인트

- Want는 incoming data rule로 active participant를 매칭하고, Allow는 active data rule로 incoming participant를 매칭하는 구조가 의도에 맞는지 확인이 필요함.

- Window timing이 Allow rule에만 적용되는 현재 정책이 intervention 책임 분리와 맞는지 확인이 필요함.

- Dead reaction force intervention을 data rule이 아니라 orchestrator policy로 유지한 결정이 적절한지 확인이 필요함.

- `OnMontageEnd(bInterrupted=true)`는 context 단일 소유권을 위해 이번 PR에서 변경하지 않았으며, 예상 밖 montage interruption 처리 정책은 다음 브랜치에서 별도 검토가 필요함.

- notify가 `WindowKey`만 전달하고 filter policy는 data rule이 소유하는 구조가 asset 설정 UX와 맞는지 확인이 필요함.

---

## 비고

- 이번 PR은 `feature/orchestration-refactor` 마감 목적의 intervention rule / runtime cleanup 정리임.

- `S23` 계획 문서는 최종 구현과 일부 불일치하므로 이번 PR 범위에 포함하지 않음.

- 추가 전역 정책, unexpected montage interruption report API, loop feedback cleanup은 다음 브랜치에서 확장 검토함.
