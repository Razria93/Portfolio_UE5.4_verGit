# UE5 Portfolio Pull Request

## 제목

**P17: Action / Reaction Intervention Rule 정리 및 Runtime Cleanup 보강**

## 날짜

**2026.06.04**

## 상태

- [x] **완료**

---

## 브랜치

- `feature/orchestration-refactor`

---

## 요약

이번 PR에서는 **새 행동이 현재 행동을 끊고 들어올 수 있는지 판단하는 기준을 정리하고, 행동 종료 후 임시 상태가 남지 않도록 정리 순서를 보강했다.**

이를 통해 피격 중 회피처럼 기존 행동을 중단해야 하는 상황을 명확히 처리하고, 중단 / 완료 이후 trail, collision, hit context, feedback 실행 순서가 어긋나지 않도록 만들었다.

성격별 핵심 변경은 다음과 같다.

### Feature

- **Dodge 개입 흐름 추가**: 피격 리액션 중 허용된 구간에서는 Dodge 액션이 현재 리액션을 중단하고 진입할 수 있도록 구성했다.

### Refactoring

- **중단 판단 책임 분리**: 새로 실행 요청된 행동의 개입 조건과 현재 실행 중인 행동의 중단 허용 조건을 서로 다른 데이터 규칙으로 나눴다.

- **중단 결과 통합**: 외부 요청으로 행동이 멈춘 결과를 `Interrupted`로 통합해 중단 결과 해석을 단순화했다.

- **허용 구간 역할 축소**: notify는 중단 허용 timing만 전달하고, 실제 중단 대상 판단은 Action / Reaction data가 담당하도록 정리했다.

### Troubleshooting

- **Runtime cleanup 보강**: 행동 종료 후 trail, collision, hit context 같은 임시 상태가 남지 않도록 executor 종료 경로에서 명시적으로 정리했다.

- **Terminal feedback 순서 보강**: 종료 feedback과 finish event에 필요한 값을 먼저 확보한 뒤, runtime 상태를 정리하고 feedback / event를 실행하도록 순서를 바꿨다.

### Documentation

- **Bug Report 문서화**: 중단 허용 구간 문제와 runtime effect 정리 누락 문제를 `B09`, `B10` bug report로 분리해 기록했다.

---

## 핵심 개념

이 섹션은 아래 설명에서 반복해서 사용할 프로젝트 고유 용어를 먼저 정리한다.

```text
active execution(현재 실행 중인 행동)
-> 현재 실행 중인 action 또는 reaction
-> 새로 실행 요청된 행동의 중단 대상이 될 수 있음
```

```text
incoming execution(새로 실행 요청된 행동)
-> 새로 실행 요청된 action 또는 reaction
-> 현재 실행 중인 행동을 중단하고 진입할 수 있는 후보 행동
```

```text
WantInterventionRules(개입 조건 규칙)
-> incoming execution이 active execution을 중단 대상으로 보는지 판단하는 규칙

[구성요소]
- ParticipantFilters: 중단 대상으로 볼 active execution 조건
```

```text
AllowInterventionRules(중단 허용 규칙)
-> active execution이 incoming execution에 의해 중단될 수 있는지 판단하는 규칙

[구성요소]
- Timing            : 중단 허용 timing 조건 (항상 / 특정 window)
- WindowKey         : window timing 평가에 사용할 구간 식별자
- ParticipantFilters: 중단을 허용할 incoming execution 조건
```

```text
FExecutionInterventionQuery(중단 판단 context)
-> active execution과 incoming execution의 중단 가능성을 판단하기 위해 필요한 값을 묶은 구조체

[구성요소]
- Snapshot    : 판단 시점의 body 실행 상태
- IncomingPart: 새로 실행 요청된 행동 정보
- ActivePart  : 현재 실행 중인 행동 정보
- StopReason  : 중단 허용 시 active execution에 기록할 중단 결과
```

```text
FExecutionInterventionDirective(중단 실행 지시값)
-> 현재 실행 중인 행동을 중단한 뒤 새 행동을 어떻게 처리할지 component에 전달하는 최종 지시값

[구성요소]
- bRequested     : 중단 요청 여부
- StopSource     : 중단 지시를 만든 주체
- SourceDomain   : 새로 실행 요청된 행동이 속한 영역
- TargetDomain   : 중단 대상인 현재 실행 중인 행동이 속한 영역
- StopReason     : 현재 실행 중인 행동에 기록할 중단 결과
- AfterStopAction: 중단 이후 새로 실행 요청된 행동을 적용하는 방식
```

```text
Dodge(회피 action)
-> 피격 리액션 중 허용된 구간에서 현재 reaction을 중단하고 진입할 수 있는 회피 action
-> 이 PR에서는 intervention rule 검증에 사용하는 대표 feature 사례
```

```text
terminal guard(최종 상태 보호 정책)
-> Dead reaction처럼 일반 intervention rule보다 강하게 유지되어야 하는 최종 상태 보호 기준
-> 이미 실행 중인 Dead reaction이 다른 incoming execution에 의해 끊기지 않도록 보호함
```

```text
Runtime cleanup(runtime 정리)
-> 행동 종료 시 executor 내부 상태와 외부 runtime side effect를 함께 정리하는 종료 단계
-> 이 PR에서는 trail, collision, hit context, feedback runtime 상태 정리를 포함함
```

---

## 변경 배경

이 섹션은 이번 PR이 필요했던 이유와 기존 구조에서 분리해야 했던 책임을 정리한다.

### Intervention 판단 책임 분리 필요성

기존 구조에서는 animation notify가 특정 animation frame에서 이벤트를 발생시키는 역할을 넘어, 어떤 행동이 어떤 행동을 끊을 수 있는지에 대한 정책 metadata까지 가지고 있었다.

하지만 새로 실행 요청된 행동의 개입 조건은 montage에 진입하기 전 판단해야 하는 정보다.

따라서 개입 조건을 notify / montage 쪽에 두는 것은 잘못된 책임 배치였고, incoming execution의 개입 조건과 active execution의 중단 허용 조건을 행동 data로 분리할 필요가 있었다.

### 중단 결과 통합 필요성

외부 요청으로 현재 행동이 멈춘 결과가 `Cancel`과 `Interrupt`로 나뉘면, 행동 종료 상태를 해석할 때 불필요한 분기가 생길 수 있었다.

이번 PR에서는 외부 요청으로 행동이 멈춘 결과를 `Interrupted`로 통합해, action / reaction 종료 결과를 같은 기준으로 해석하도록 정리했다.

### Reaction 우선순위 정리 필요성

Hit reaction과 Dead reaction은 일반 action 중단과 성격이 다르다.

Hit reaction은 공통 중단 판단 흐름에 편입할 수 있지만, Dead reaction은 최종 상태에 가까우므로 일반 규칙만으로 처리하면 다른 행동에 의해 다시 끊길 위험이 있었다.

따라서 일반 reaction의 중단 판단과 Dead reaction의 최종 우선권을 분리할 필요가 있었다.

### Runtime cleanup 순서 보강 필요성

행동이 중단되거나 완료된 뒤 trail, collision, hit context 같은 임시 상태가 남을 수 있었다.

또한 terminal feedback과 finish event는 종료 시점의 action / reaction 정보를 필요로 하는데, 내부 상태를 먼저 정리하면 feedback / event에 필요한 값이 사라질 수 있었다.

따라서 종료 시점에 필요한 값을 먼저 snapshot으로 확보하고, runtime cleanup 이후 확보한 값으로 feedback과 event를 실행하는 순서가 필요했다.

---

## 변경 범위

이 섹션은 intervention 판단 기준, 중단 결과, runtime cleanup 순서를 어떤 책임으로 나눴는지 정리한다.

### 1. Dodge 개입 흐름 추가

- **왜**:
  피격 리액션 중에도 플레이어가 회피 가능한 구간에서는 Dodge 액션으로 빠져나올 수 있어야 했다.
  동시에 새로 정리한 intervention 구조가 실제 gameplay 흐름에서 동작하는지 확인할 대표 사례가 필요했다.

- **어떻게**:
  Dodge data의 `WantInterventionRules`에는 Hit reaction을 중단 대상으로 보는 조건을 설정했다.
  Hit reaction data의 `AllowInterventionRules`에는 특정 중단 허용 window에서만 Dodge를 허용하는 조건을 설정했다.

- **결과**:
  피격 리액션 중 허용된 구간에서는 Dodge가 현재 reaction을 `Interrupted`로 중단하고 진입할 수 있게 됐다.
  Dodge / Hit reaction 조합이 intervention rule의 feature 검증 사례가 됐다.

### 2. Intervention 판단 책임 분리

- **왜**:
  개입 조건은 새로 실행 요청된 행동이 판단해야 하고, 중단 허용 조건은 현재 실행 중인 행동이 판단해야 하는 정보였다.
  기존 구조에서는 이 두 조건이 notify / montage 설정에 섞여 있어 판단 책임이 불명확했다.

- **어떻게**:
  개입 조건은 Action / Reaction data의 `WantInterventionRules`에, 중단 허용 조건은 `AllowInterventionRules`에 설정하도록 분리했다.
  실행 중에는 이 data 설정을 기준으로 incoming execution과 active execution의 중단 가능성을 판단한다.

- **결과**:
  중단 판단은 `incoming 개입 조건 확인 -> active 허용 조건 확인 -> FExecutionInterventionDirective 생성` 순서로 정리됐다.
  notify는 중단 허용 timing만 전달하는 역할로 축소됐다.

### 3. 중단 결과 통합

- **왜**:
  중단 결과가 `Cancel`과 `Interrupt`로 나뉘어 있으면, 외부 요청으로 행동이 멈춘 경우를 정의할 때 불필요한 상태 분기가 생길 수 있었다.

- **어떻게**:
  외부 요청으로 현재 행동이 멈춘 결과는 `Interrupted`로 통합했다.
  Action / Reaction component는 `EExecutionStopReason::Interrupted`를 각 domain의 stop / finish reason으로 변환하도록 정리했다.

- **결과**:
  외부 중단 결과가 `Interrupted` 하나로 정리되어, action / reaction 종료 상태 해석이 단순해졌다.

### 4. Reaction 우선순위 정책 정리

- **왜**:
  Hit reaction은 일반 행동 중단 흐름으로 편입할 수 있지만, Dead reaction은 다른 행동에 의해 다시 끊기면 안 되는 최종 상태에 가깝다.

- **어떻게**:
  Hit reaction의 일반 중단 판단은 data rule 기반 공통 intervention 흐름에 편입했다.
  Dead reaction의 최종 우선권과 이미 실행 중인 Dead reaction 보호는 orchestrator / terminal guard 정책으로 유지했다.

- **결과**:
  일반 reaction은 공통 중단 판단 흐름을 따르고, Dead reaction은 다른 행동에 의해 끊기지 않는 최종 상태로 남는다.

### 5. Runtime cleanup과 terminal feedback 순서 보강

- **왜**:
  기존 행동이 중단되거나 완료된 뒤 trail, collision, hit context 같은 runtime side effect가 남을 수 있었다.
  동시에 중단 / 완료 시점에 실행해야 하는 feedback과 event는 상태 정리 이후 필요한 값을 잃을 수 있었다.

- **어떻게**:
  중단 또는 완료 시점에 실행할 feedback request와 finish event payload를 먼저 snapshot으로 확보했다.
  이후 montage 중단, runtime side effect 정리, 내부 runtime state 정리를 수행한 뒤 확보한 값으로 terminal feedback과 finish event를 실행하도록 순서를 재구성했다.

- **결과**:
  기존 행동의 runtime side effect는 종료 경로에서 정리된다.
  terminal feedback과 finish event는 cleanup 이후에도 필요한 정보를 잃지 않고 실행된다.

### 6. 검증 데이터와 문서 보강

- **왜**:
  intervention 판단 구조가 실제 gameplay 흐름에서 동작하는지 확인할 대표 시나리오가 필요했다.
  또한 중단 허용 구간 문제와 runtime effect 정리 누락 문제를 bug report로 남길 필요가 있었다.

- **어떻게**:
  Dodge / Hit reaction / Dead reaction을 대표 검증 흐름으로 구성했다.
  관련 timing, 중단 허용 window, montage, character data를 갱신했다.
  B09 / B10 bug report에 원인, 수정 기준, 검증 결과를 분리해 기록했다.

- **결과**:
  action 중단, reaction 중단, Dead reaction 우선권, 종료 후 runtime cleanup이 같은 브랜치에서 검증됐다.
  intervention rule 문제와 runtime cleanup 문제는 각각 B09 / B10으로 추적 가능해졌다.

---

## 주요 처리 흐름

이 섹션은 intervention 판단과 runtime cleanup 순서를 코드 구현 전에 흐름으로 먼저 설명한다.

### Intervention 판단 흐름

```text
새로 실행 요청된 행동
-> FExecutionInterventionQuery 구성
-> incoming execution이 active execution을 중단 대상으로 보는지 평가
-> Want 조건 충족?
   - No  -> 현재 행동 유지
   - Yes -> active execution이 incoming execution에게 중단을 허용하는지 평가
            - incoming 조건
            - 중단 허용 timing
         -> Allow 조건 충족?
            - No  -> 현재 행동 유지
            - Yes -> FExecutionInterventionDirective 생성
                  -> 현재 행동을 Interrupted로 중단
                  -> 새로 실행 요청된 행동 적용
```

이 흐름은 새로 실행 요청된 행동이 현재 실행 중인 행동을 끊고 들어올 수 있는지 판단하고, 허용된 경우 현재 행동을 `Interrupted` 결과로 정리한 뒤 새 행동을 적용하는 과정을 의미한다.

### Runtime cleanup 흐름

```text
기존 행동 Stop 또는 Complete
-> terminal feedback request snapshot 확보
-> finish event payload snapshot 확보
-> montage 중단
-> runtime side effect 정리
   - trail OFF
   - collision disabled
   - hit context clear
   - feedback runtime clear
-> 내부 runtime state 정리
-> snapshot 기반 terminal feedback 실행
-> snapshot 기반 finish event 발행
```

이 흐름은 행동 종료 시점에 feedback과 event에 필요한 값을 먼저 확보하고, runtime side effect와 내부 상태를 정리한 뒤 terminal feedback과 finish event를 실행하는 순서를 의미한다.

---

## 구현 결과

이 섹션은 변경 이후 시스템이 보장하는 동작을 정리한다.

- incoming execution은 먼저 active execution을 중단 대상으로 볼 수 있는지 평가된다.

- active execution은 어떤 incoming execution에게 중단을 허용할지 Action / Reaction data와 중단 허용 timing으로 판단한다.

- Want / Allow 조건이 모두 충족된 경우에만 `FExecutionInterventionDirective`가 생성되고, active execution은 `Interrupted` 결과로 종료된다.

- Dead reaction은 일반 intervention rule보다 강한 최종 우선권을 유지한다.

- 행동 종료 시점에는 terminal feedback / finish event에 필요한 값을 먼저 확보한 뒤 runtime cleanup을 수행한다.

- runtime cleanup 이후에도 terminal feedback과 finish event는 snapshot 기반으로 안정적으로 실행된다.

---

## 테스트 방법

### 빌드

- `PortfolioEditor Win64 Development` 빌드가 성공하는지 확인한다.

### 정적 확인

- 제거한 중단 결과, feedback 호출 방식, 구 중단 허용 구간 API가 코드에 남아 있지 않은지 확인한다.

- `Cancel` 잔여 검색 결과가 orchestration 변경과 무관한 코드에만 남는지 확인한다.

- `git diff --check`가 통과하는지 확인한다.

### 수동 검증

- equip / unequip, combo chain, combo 거부 조건, combo input window를 확인한다.

- Dodge, Hit reaction, Dead reaction이 행동 중단 정책에 맞게 실행되는지 확인한다.

- collision / hit context / 중단 허용 window notify begin / end가 정상 동작하는지 확인한다.

- trail ON 중 hit / dead interrupt 이후 trail, collision, hit context가 정리되는지 확인한다.

- reaction interrupt / complete feedback이 snapshot 기반 실행 경로에서 정상 재생되는지 확인한다.

---

## 검증 결과

### 빌드

- `PortfolioEditor Win64 Development` 빌드 성공을 확인했다.

```text
Target is up to date
Total execution time: 0.54 seconds
```

### 정적 확인

- 구 중단 결과, 구 feedback 호출 방식, 구 중단 허용 구간 API 잔여가 없음을 확인했다.

- `Cancel` 잔여 검색 결과는 `UCHealthComponent::TryCancelRevive()`만 남아 있으며 이번 orchestration 변경과 무관하다.

- `git diff --check` 통과를 확인했다.

### 수동 검증

- sword equip / unequip 정상 동작을 확인했다.

- 무기 장착 상태에서 combo attack 0-1-2 chain 정상 동작을 확인했다.

- 무기 미장착 상태에서 combo attack 거부를 확인했다.

- combo chain window 밖 입력이 reserve되지 않는지 확인했다.

- Dodge가 허용된 구간에서 현재 reaction을 중단하고 실행되는지 확인했다.

- Hit reaction이 action interrupt 가능 구간에서 정상 개입하는지 확인했다.

- Dead reaction이 active action / reaction 여부와 무관하게 최종 우선 실행되는지 확인했다.

- collision / hit context / 중단 허용 window notify begin / end 정상 동작을 확인했다.

- trail ON 중 hit / dead interrupt 이후 trail, collision, hit context가 정리되는지 확인했다.

- reaction interrupt / complete feedback이 snapshot 기반 실행 경로에서 정상 재생되는 것을 확인했다.

---

## 설계 판단 기준

- incoming execution의 개입 조건 판단과 active execution의 중단 허용 판단을 서로 다른 책임으로 분리했다.

- 중단 허용 timing은 active execution이 열어 둔 상태이므로, active execution의 허용 규칙에서만 평가하도록 정리했다.

- Dead reaction의 최종 우선권은 일반 규칙보다 강한 정책으로 보고 orchestrator에 유지했다.

- 이미 실행 중인 Dead reaction이 다른 incoming execution에 의해 끊기지 않도록 terminal guard를 유지했다.

- notify는 중단 허용 window의 시작과 끝만 전달하고, 실제 중단 대상 판단은 Action / Reaction data가 소유하도록 역할을 축소했다.

---

## 후속 작업

- Action / Reaction 흐름 조율 코드에 남아 있는 공통 중단 판단 알고리즘 추출 범위를 검토한다.

- 도메인별 유지 책임을 분리하고, 상속 구조로 확장하기 전에 helper / utility 분리 가능성을 먼저 검토한다.

- notify window 기반 상시 정책 문제를 재현하고 개선 기준을 정리한다.

- 실제 행동 중단이 발생했을 때 `FExecutionInterventionDirective`의 주체, 영역, 중단 결과, 후속 처리 정보를 한 번에 확인할 수 있도록 추적 로그를 보강한다.

- 예상 밖 montage interruption report API와 loop feedback cleanup 확장 범위는 다음 브랜치에서 별도 검토한다.

---

## 관련 문서

- Issue Checklist: `D18_UE5_Portfolio_Issue_Checklist.md`

- Bug Report:
  - `B09_UE5_Portfolio_Bug_Report.md`
  - `B10_UE5_Portfolio_Bug_Report.md`

---

## 비고

- 이번 PR은 `feature/orchestration-refactor` 마감 목적의 행동 중단 규칙과 종료 정리 흐름 보강 작업이다.

- System Architecture 문서는 이후 재정리 예정이므로 이번 PR의 관련 문서 범위에 포함하지 않는다.

---

## 정리

- Action / Reaction 중단 판단을 notify가 직접 판단 조건을 들고 있는 구조에서 Action / Reaction data가 판단 기준을 소유하는 구조로 정리했다.

- 새로 실행 요청된 행동이 현재 행동을 멈추는 결과는 `Interrupted`로 통합하여 외부 중단 결과 해석을 단순화했다.

- 행동 종료 전 필요한 값을 먼저 확보하고, 종료 후 trail / collision / hit context를 정리한 뒤 feedback과 event를 실행하도록 순서를 보강했다.
