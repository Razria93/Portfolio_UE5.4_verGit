## 1. 문서 목적

본 문서는 본 프로젝트의 Combat Data Processing Pipeline에 대해서 설명한다.
핵심은 공격 객체와 피격 객체 사이에서 **Damage Event에 담길 데이터를 어떻게 생성하고, 송신하고, 수신하고, 해석하고, 적용하는가**를 책임 구조 기준으로 정리하고 구조화하는 데 있다.

---
## 2. 관련 자료 링크

##### Github

- PR: feature/combat-core-shared (#36)
	https://github.com/Razria93/Portfolio_UE5.4_verGit/pull/36#issue-4210305099
  
---
## 3. 문제 정의

[1차 프로젝트 전투 처리 파이프라인]
![[Pasted image 20260409200050.png]]
이 구조는 아래 문제를 가진다.

1. 공격 객체가 피격 객체를 **직접 호출 하는 구조여서 객체 간 의존도가 형성**되어 있었다.
2. CAction이  **Action 실행 + 전투 데이터 생성 + 송신 책임**을 함께 가지고 있었다.
3. 피격 객체가 **전투 데이터 수신 및 해석, State 변경, Health 반영, Reaction 처리까지 직접 담당**하고 있어 책임이 과도하게 집중되어 있었다.
4. 결과적으로 전투 데이터의 **송신 / 수신 / 해석 / 적용** 계층이 명확히 분리되지 않아, 공통 규칙 확장과 예외 처리에 구조적 한계가 있었다.
   
---
## 4. 책임 경계 설정

[공격 객체 측]
- **CWeaponComponent**: Attachment에 Context 주입
- **CAttachment**: Hit Context 구성 및 보관
- **CApplyDamageComponent**: Damage Event 구성 및 송신

[피격 객체 측]
- **CTakeDamageComponent**: Damage Event 수신 및 해석
- **CHealthComponent**: Health에 Damage Commit 처리
- **CDamageFeedbackComponent**: Damage Feedback 처리
- **Reaction Execution Pipeline:** Damage Reaction 실행 처리
  
1차 프로젝트의 문제를 해결하기 위해, 2차 프로젝트에서는 전투 데이터의 **컨텍스트 주입 / 구성 및 송신 / 수신 및 해석 / 결과 적용** 책임을 단계별로 분리하는 방향으로 전투 데이터 처리 구조를 재구성하였다.

이를 통해 전투 데이터 처리 과정 내에서 책임을 명확히 분산하고, 공통 규칙 적용과 후속 확장을 보다 안정적으로 다룰 수 있는 구조를 기대할 수 있다.

---
## 5. 전투 데이터 처리 구조 재구성

[2차 프로젝트 전투 처리 파이프라인]
![[CombatData Processing Pipeline.png]]
2차 프로젝트에서는 위에서 제시한 책임 경계 설정을 기반으로 전투 데이터 처리를 **Combat Data Processing Pipeline** 위에서 처리하는 구조로 재구성하였다.

---
## 6. 문제 해결

해당 재구성 작업를 통해 기대하는 효과는 아래와 같다.

- 송신 이전에 걸러야 하는 예외와 수신 이후에 판단해야 하는 예외를 분리할 수 있다.
- duplicate hit, dead target 보호, 상태 기반 거부 정책을 공용 계층에서 다룰 수 있다.
- 데이터와 처리 과정이 분리되어 Data Driven 구조로 전투 데이터를 구성할 수 있다.
- 향후 전투 데이터 처리 구조가 필요한 객체는 컴포넌트 구성 만으로 처리할 수 있다.
- 이후 전투 관련 구조 및 정책을 확장하기 용이해진다.

---
## 7. 정리

1차 프로젝트에서는 공격 판정 이후의 전투 데이터 처리가 `Action`과 피격 객체 내부에 직접 결합되어 있었다.

그 결과 공격 실행, 데이터 생성, 데이터 송신, 피격 처리, 상태 변경, 리액션 실행 책임이 한 흐름에 집중되어 있었다.

2차 프로젝트에서는 이를 다음 흐름으로 재구성했다.

```text
Action
-> HitContext
-> ApplyDamage
-> TakeDamage
-> CommitDamageToHealth
-> DamageFeedback
-> Reaction Execution Pipeline
```

책임 경계 설정을 기반으로 전투 데이터 처리를 **Combat Data Processing Pipeline** 위에서 처리하는 구조로 재구성하였다.

이를 통해 **예외 규칙 분리 / 공용 거부 정책 추가 / Data-Driven 구조로 변경 / 전투 데이터 처리 객체 구성 용이 / 구조 및 정책 확장 용이** 등을 기대할 수 있다.

---
