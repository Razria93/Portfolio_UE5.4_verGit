# Payload/Context/Result 기반 TakeDamage 파이프라인 구현: CEnemy / CTakeDamageComponent / CHealthComponent

## 제목

✨ feat: TakeDamage 파이프라인 구현 (CEnemy / CTakeDamageComponent / CHealthComponent) (#22)

---

## 요약

- 기존에는 `AActor::TakeDamage()` 오버라이드 지점에서 **수신 검증 및 해석 / HP 반영 / 디버그 출력**까지 한 객체에 집중되어 다음 문제가 발생할 여지가 있었음
    
    1. **액터에 대한 책임 비대화**
        
        - `CEnemy`가 데미지 처리 파이프라인을 직접 소유하면 액터와 데미지 처리 기능이 강하게 결합됨
            
        - 상속 확장 시 `CEnemy` 내부 예외처리가 복잡해지고 객체 간 결합이 증가할 여지가 큼
            
    2. **확장성 저하**
        
        - 신규 액터 추가 시 유사한 데미지 처리 로직을 반복 구현하게 될 가능성이 큼
            
        - 데미지 연산이 필요 없는 액터에도 기능이 섞일 여지가 있음
            
    3. **자원 책임 혼재**
        
        - HP 자원 관리 Clamp Damage Heal Dead 와 평가 및 정책 Reject Resolve Commit 이 분리되지 않으면 후속 기능 추가 시 결합도가 빠르게 증가할 수 있음
            
- 이를 개선하기 위해 이번 PR에서는 다음 목표로 구현함
    
    1. `CEnemy`는 **TakeDamage Entry Override + Component Routing** 역할로 축소
        
    2. `TakeDamage` 파이프라인은 `UCTakeDamageComponent`로 이관
        
    3. `UCTakeDamageComponent`는 **`FDamageEvent` 타입 분기 오케스트레이션** 역할 중심으로 구현
        
    4. `UCHealthComponent`는 **HP 감소 증가 Dead 판정** 자원 관리 역할 중심으로 구현
        
    5. `TakeDamage` 데이터 흐름은 `FTakeDamagePayload / FTakeDamageContext / FTakeDamageResult`로 표준화
        
    6. 구현 과정의 데이터 흐름과 값을 확인할 수 있도록 Debug Print 함수 구성
        

---

## 완료 항목

### 1. CEnemy 구성 및 TakeDamage 재정의: 수신 지점과 처리 지점 분리

- `CEnemy` 또는 피격 대상 액터에서 `TakeDamage(...)`를 오버라이드하여 **수신 진입점**을 확보함
    
- 오버라이드 내부에서 직접 연산 적용을 수행하지 않고 `UCTakeDamageComponent::RequestTakeDamage(...)`로 위임하여 `CEnemy`를 `Entry`와 `Routing` 역할로 제한함
    

---

### 2. CTakeDamageComponent 구현 및 데미지처리 API 제공

- `UCTakeDamageComponent`에 외부 호출용 API를 제공함
    
    - `RequestTakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)`
        
- `TakeDamage` 처리 흐름은 `UCTakeDamageComponent` 내부 파이프라인으로 통합함
    
    - `RequestTakeDamage()` → `ProcessTakeDamage()` → `HandleDamageEvent()`
        

---

### 3. DamageEvent 표준화 및 타입별 Handle 분리를 통한 확장 가능한 이벤트 라우팅 구조 구성

- `FDamageEvent` 기반으로 `FDefaultDamageEvent`를 정의하여 Take 단계에서 필요한 정보를 유지함
    
    - `FApplyDamageSpecKey / FApplyDamageSpec / FApplyDamageResult` 포함
        
    - `EDamageEventTypeId::DefaultDamage(0x1001)` 기반 `ClassID`를 명시하여 타입 판별 가능하도록 구성함
        
- `ProcessTakeDamage()`에서 `DamageEvent.IsOfType(FDefaultDamageEvent::ClassID)`로 타입을 판별하고 이벤트별 처리 함수를 분리함
    
    - 현재 구현 `HandleDefaultDamageEvent(...)`
        
    - 후속 이벤트 타입 예 `PointDamage` `RadialDamage` `Status Effect` 추가 시 `HandleXDamageEvent()` 단위로 확장 가능한 구조로 구현함
        

---

### 4. Payload/Context/Result 기반 오케스트레이션 처리 알고리즘 구현: TakeDamage 파이프라인 확립

- `HandleDefaultDamageEvent` 내부에 오케스트레이션 파이프라인을 구성하여 처리 단계를 명확히 분리함
    
    - `ValidateRequest()`
        
    - `BuildPayload()`
        
    - `BuildContext()`
        
    - `EvaluateTakeDamage()` (Compute / Rule 처리 단계)
        
    - `CommitTakeDamage()` (Resource / State 처리 단계)
        
- 데이터는 3구조체로 역할을 분리하여 보관 전달함
    
    - `FTakeDamagePayload`: 입력 원본 (`Instigator` `Causer` + `ApplyResult` `ApplySpec`)
        
    - `FTakeDamageContext`: `Resolved` 객체 + 상태 스냅샷 예 `bWasDead` + 절차 별 데미지값
        
    - `FTakeDamageResult`: `Accepted` `Rejected` `RejectReason` `FinalTakeDamage` `bKilled` 등 결과 데이터
        
- Instigator 해석은 `ResolveInstigatorController()`에서 fallback 체계로 구성함
    
    - `EventInstigator` 우선
        
    - `DamageCauser`의 `InstigatorController` `Pawn Controller`
        
    - `DamageCauser Owner` 기반 `Proxy Case` `Owner Instigator` `Owner Pawn Controller`
        

---

### 5. CHealthComponent 구성: HP 감소 증가 알고리즘 및 Dead 판정 분리

- `UCHealthComponent`로 HP 자원 관리 책임을 분리함
    
    - 초기화 `InitializeHealth(InitMaxHP, InitCurrentHP, bFillToInitMaxHP)`
        
    - 감소 `TakeDamage(float InTakeDamageAmount)`
        
    - 증가 `TakeHeal(float InTakeHealAmount)`
        
    - 상태 `UpdateDeadState()`로 Dead 전이 관리
        
- 데미지 힐 처리 시 Clamp 및 유효성 체크로 음수 무의미 입력을 방어함
    
- Delegate BroadCast는 TODO로 유지하고 알고리즘 출력 기반으로 먼저 기반을 확립함  
    `OnHealthChanged` `OnDead` `OnRevived`
    

---

### 6. 파이프라인 디버깅 출력 구성: 단계별 추적 가능하도록 프린트 함수 제공

- `UCTakeDamageComponent`
    
    - `PrintTakeDamageSummaryInfo()` 핵심 오브젝트 최종 데미지 요약
        
    - `PrintTakeDamageContextInfo()` `Payload` `Context` `SpecKey` `Amount` 세부 출력
        
- `UCHealthComponent`
    
    - `PrintTakeDamageContextInfo()` `PrintTakeHealContextInfo()`
        
    - HP 변화량 Delta 비율 Percent Dead 상태 출력 포함
        

---

## 테스트 방법

1. `CEnemy` 피격 대상에 `UCHealthComponent` `UCTakeDamageComponent` 부착 여부 확인함
    
2. 공격 Apply 흐름에서 `Target->TakeDamage(...)` 호출 여부 확인함
    
3. `CEnemy::TakeDamage(...)`에서 `UCTakeDamageComponent::RequestTakeDamage(...)`로 위임되는지 확인함
    
4. `UCTakeDamageComponent`에서 `FDamageEvent` 타입 판별이 정상 동작하는지 확인함
    
    - `DamageEvent.IsOfType(FDefaultDamageEvent::ClassID)` → `HandleDefaultDamageEvent(...)`
        
5. TakeDamage Summary 로그 출력 여부 확인함
    
    - `DamagedActor / ResolvedInstigator / ResolvedDamageCauser`
        
    - `FinalTakeDamage`
        
6. `UCHealthComponent` Damage Heal 로그 출력 및 HP 값 갱신 확인함
    
    - `PreviousHP / CurrentHP / HPDelta / HPPercent / bIsDead`
        

---

## 관련 이슈 / 브랜치

- 브랜치: `feature/combat-take-damage`
    
- 이슈: #22
    

---

## 노트

- 본 PR의 핵심은 TakeDamage 동작 자체가 아니라 **이벤트 분기 + Payload Context Result 오케스트레이션 + Health 책임 분리**를 성립시켜 추후 기능 추가와 유지보수를 용이하게 하는 것임
    
- `UCTakeDamageComponent`는 현재 연출 상태 전이를 직접 수행하지 않으며 Health 반영 이후 리액션은 TODO로 남겨 결합도 확장을 방지함
    
- `Instigator` 해석은 보수적으로 구성했으며 엔진 구조 이해와 추후 확장성을 위한 선택임
    

---

## In Scope (이번 PR 포함)

- `CEnemy::TakeDamage(...)` 오버라이드 및 TakeDamageComponent 대상으로 라우팅
    
- `UCTakeDamageComponent` 추가 및 `DefaultDamageEvent` 처리 파이프라인 구성
    
- `FDefaultDamageEvent(EDamageEventTypeId)` 및 `FTakeDamagePayload/Context/Result` 데이터 구조 도입
    
- `UCHealthComponent` 추가 및 HP 감소 증가 Dead 판정 알고리즘 구성
    
- 단계별 디버깅 프린트 함수 구성
    

---

## Out of Scope (이번 PR 제외)

- `FinalTakeDamage` 계산 정책 감쇠 클램프 최소 데미지 상한 반올림 확정 구현
    
- 중복 히트 방지 AlreadyHit Set 팀 무적 상태 기반 필터링 정책 구현
    
- 피격 리액션 경직 넉백 히트스톱 애니메이션 VFX SFX 처리 및 상태 전이
    
- Health 변경 사망 부활 Delegate 브로드캐스트
    

---

## Follow-ups (후속 작업)

- `EvaluateTakeDamage()`에서 `FinalTakeDamage` 산출 정책 구현  
    예 `Mitigated` `FinalizeTaken` `FinalizeApplied` 단계 분리
    
- `ETakeDamageRejectReason` 기반 Reject 정책 확장  
    예 `AlreadyDead` `Invulnerable` `FriendlyFire` `RuleBlocked` 등
    
- `UCHealthComponent` Delegate 브로드캐스트 도입  
    예 `OnHealthChanged / OnDead / OnRevived`
    
- TakeDamage 후속 처리 상태 전이 리액션 요청 등을 별도 컴포넌트로 분리 구현
    
- 불필요한 `Tick` 오버라이드 정리 `PrimaryComponentTick` 비활성화와의 일관성 유지
    

---