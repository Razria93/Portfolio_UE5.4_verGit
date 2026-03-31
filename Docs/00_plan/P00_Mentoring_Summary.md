
**작성일:** 2026년 3월 13일  
**프로젝트명:** UE5 Combat Portfolio  
**엔진:** Unreal Engine 5.4  

---

## 1. 프로젝트 개요

### 기본 정보
- **장르:** 3인칭 액션 전투 데모
- **플랫폼:** Windows PC (에디터 실행 기준)
- **개발 기간:** 2025.12.01 ~ (MVP + 확장)
- **엔진 버전:** Unreal Engine 5.4

### 프로젝트 목표
1. **게임의 최소 구성 요소를 이해하고 설계 및 구현함**
   
2. **실무 스타일 워크플로우를 경험함**
   - Git/GitHub 브랜치 전략 및 버전 관리
   - Issue 기반 작업 관리
   - Pull Request & Code Review
   - 체계적인 문서화 (Obsidian 기반)

---

## 2. 구현 범위

### MVP (필수 범위)
| 시스템      | 구현 항목                         |
| -------- | ----------------------------- |
| **플레이어** | 이동 / 점프 / 회피(구르기)             |
| **카메라**  | 3인칭 카메라, Lock-on 타게팅          |
| **전투**   | 무기 장착/해제, 기본 공격, 콤보 시스템       |
| **적 AI** | 더미 적, 간단한 전투 AI               |
| **데미지**  | Hit 판정, HP 시스템, 피격 리액션, 사망    |
| **UI**   | HP/리소스 UI, 데미지 표시             |
| **레벨**   | Test Room (테스트용), World (데모용) |
| **VFX**  | 공격/피격 이펙트 (Niagara/Particle)  |

### 확장 범위 (선택)
- [ ] 고급 전투: 히트 스탑, 공중 콤보, 패링/가드, 완벽 회피
- [ ] AI 확장: 고급 전투 행동, 팀 시스템, 다양한 적 유형
- [ ] 이동 확장: 파쿠르, 텔레포트
- [ ] 애니메이션: Foot IK, ALS 스타일 이동

---

## 3. 구현 범위 및 설계 철학

### 구현 범위 결정 원칙

#### 1. 기능 범위보다 구조 설계 우선
본 프로젝트는 "많은 기능 구현"보다 "확장 가능한 구조 설계"에 중점을 두었음.
- 10가지 스킬 구현보다 1가지 스킬을 데이터 기반으로 확장 가능하게 설계함
- 다양한 적 타입보다 하나의 AI 시스템을 BehaviorTree로 모듈화함

#### 2. MVP 기준: 단일 전투 루프 완성
최소한의 완성도 있는 전투 경험을 제공하는 것을 목표로 함.
- Player → 이동/공격 → Enemy 피격/사망 → 다음 적 대응
- Enemy → 감지/추적/공격 → Player 피격/회피 → 반격

#### 3. 확장 포인트 명시
현재 구현하지 않더라도, 확장 시 어디를 수정해야 하는지 명확히 함.
- `// TODO: Separate DataAsset` - 데이터 분리 시점 명시
- `SpecKey` 기반 설계 - 새로운 타입 추가 시 키만 확장
- Interface 기반 설계 - 새로운 구현체 추가 가능

#### 4. 실무 워크플로우 경험 우선
코드만큼 중요한 것은 협업 과정임.
- Git Feature 브랜치 전략
- Issue → PR → Review → Merge 사이클
- 체계적인 문서화 (Issue Checklist, PR, Analysis Report)

---

## 4. 핵심 설계 구조 및 기술

### 설계 철학: Component 기반 책임 분리 + 데이터 주도 확장

본 프로젝트의 핵심은 "단일 책임 원칙(SRP)"과 "데이터 기반 설계(Data-Driven)"의 조화임.

### 4.1 Payload/Context/Result 패턴

**문제:** 함수 간 데이터 전달이 파라미터 나열로 복잡해지고, 중간 단계 추적이 어려웠음

**해결:** 3단계 구조체로 데이터 흐름을 표준화함

```cpp
// 모든 주요 시스템에 적용된 패턴
struct Payload  { /* 입력 원본 (불변) */ }
struct Context  { /* 처리 중 상태 (가변) */ }  
struct Result   { /* 최종 결과 (출력) */ }

// 예시: TakeDamage 시스템
FTakeDamagePayload  // Instigator, Causer, SpecKey, ApplyResult
  ↓
FTakeDamageContext  // Resolved 객체, 상태 스냅샷, 중간 계산값
  ↓
FTakeDamageResult   // Accepted, FinalDamage, bKilled, RejectReason
```

**장점:**
- 각 처리 단계의 입력/출력이 명확함
- 디버깅 시 중간 상태 추적이 용이함
- 함수 시그니처가 단순화됨 (3개 구조체로 통일)
- 확장 시 기존 코드 수정을 최소화함

**적용 시스템:**
- ApplyDamage: `HitContext` → `ApplyDamageSpec` → `ApplyDamageResult`
- TakeDamage: `TakeDamagePayload` → `TakeDamageContext` → `TakeDamageResult`  
- Reaction: `ReactionPayload` → `ReactionContext` → `ReactionResult`
- AI: `TargetData` → `FAIContext` → `PatrolContext/CombatContext`

---

### 4.2 SpecKey 기반 데이터 주도 설계

**문제:** 새로운 공격/무기/적 추가 시마다 코드 수정이 필요했음

**해결:** Key-Value 구조로 데이터와 로직을 분리함

```cpp
// 데미지 계산 예시
FApplyDamageSpecKey Key {
    AttachmentType,   // Sword, Axe, Bow...
    EquipmentType,    // Default, Fire, Ice...
    ActionType,       // LightAttack, HeavyAttack...
    ActionIndex       // Combo 단계
};

TMap<FApplyDamageSpecKey, FApplyDamageSpec> SpecContainer;
// SpecContainer에서 Key로 조회 → BaseDamage, Multiplier 등 획득
```

**장점:**
- 코드 수정 없이 데이터만 추가 가능함
- 향후 DataAsset/DataTable로 분리 가능함
- 디자이너가 데이터 조정 가능한 구조임
- 조합(Attachment × Equipment × Action)을 키 하나로 표현함

**확장 경로:**
```cpp
// 현재: TMap (하드코딩)
UPROPERTY(EditAnywhere)
TMap<FApplyDamageSpecKey, FApplyDamageSpec> SpecContainer;

// 향후: DataAsset
UPROPERTY(EditDefaultsOnly)
UDamageSpecDataAsset* DamageSpecs;
```

---

### 4.3 Pipeline 아키텍처: 단계별 처리 표준화

**문제:** 복잡한 로직을 한 함수에서 처리하면 유지보수가 어려웠음

**해결:** 모든 주요 시스템 을 5단계 파이프라인으로 통일함

```cpp
// 표준 파이프라인 구조
1. Validate   - 입력 유효성 검증 (Early Return)
2. Build      - Payload/Context 구성
3. Evaluate   - 규칙 적용, 값 계산 (Side-effect Free)
4. Commit     - 상태/리소스 변경 (실제 적용)
5. Dispatch   - 후속 처리 위임 (Reaction, UI 등)
```

**실제 적용: TakeDamage 파이프라인**
```cpp
float UCTakeDamageComponent::RequestTakeDamage(...)
{
    return ProcessTakeDamage(...);
}

float ProcessTakeDamage(...)
{
    // 1. Validate
    if (!ValidateRequest(...)) return 0.f;
    
    // 2. Build
    Payload = BuildPayload(...);
    Context = BuildContext(Payload);
    
    // 3. Evaluate (순수 계산, 상태 변경 없음)
    EvaluateTakeDamage(Context);  
    // → ComputeMitigatedDamage()
    // → ComputeFinalTakenDamage()
    
    // 4. Commit (실제 HP 감소)
    CommitTakeDamage(Context);
    // → ApplyDamageToHealth()
    
    // 5. Dispatch (Reaction 요청)
    Result = BuildResult(Context);
    DispatchReaction(Payload, Context, Result);
    
    return Result.FinalAppliedDamage;
}
```

**장점:**
- 각 단계의 책임이 명확함
- 테스트 시 단계별 검증이 가능함
- Evaluate는 Side-effect Free로 유지함 (함수형 프로그래밍 원칙)
- 모든 시스템이 동일한 흐름을 가짐 (학습 비용 감소)

---

### 4.4 Component 기반 책임 분리

**문제:** Actor/Character 클래스가 비대해지고 결합도가 증가했음

**해결:** 기능별 Component 분리 + 느슨한 결합

#### **전투 시스템 Component 구조**
```cpp
ACharacter
├── CApplyDamageComponent   // 공격 → 데미지 계산 → TakeDamage 호출
│   └── Validate → ResolveSpec → Compute → Apply
│
├── CTakeDamageComponent    // TakeDamage 수신 → 처리 오케스트레이션
│   └── Validate → Build P/C/R → Evaluate → Commit
│
├── CHealthComponent        // HP 리소스 관리 (증감, Dead 판정)
│   └── TakeDamage / TakeHeal / UpdateDeadState
│
└── CReactionComponent      // 피격 리액션 실행 (Hit/Dead 몽타주)
    └── ResolveType → ResolveData → QueryAccept → Play/End
```

**컴포넌트 간 통신:**
```cpp
// 느슨한 결합: 컴포넌트 탐색 + 캐싱
void UCTakeDamageComponent::BeginPlay()
{
    HealthComp_Cached = OwnerActor->FindComponentByClass<UCHealthComponent>();
    ReactionComp_Cached = OwnerActor->FindComponentByClass<UCReactionComponent>();
}

// 위임 방식
void CommitTakeDamage(Context)
{
    if (HealthComp_Cached)
        HealthComp_Cached->TakeDamage(Context.FinalDamage);
        
    if (ReactionComp_Cached)
        ReactionComp_Cached->RequestReaction(Result);
}
```

**장점:**
- 단일 책임 원칙 (SRP)을 준수함
- 컴포넌트 단위 테스트가 가능함
- 조합이 가능함 (Player/Enemy 모두 동일 컴포넌트 사용)
- 확장 시 기존 컴포넌트 수정이 불필요함

---

### 4.5 BB-BT 중심 AI 설계

**문제:** FSM 방식은 상태 확장 시 코드 수정이 필요하고, 시각화가 어려웠음

**해결:** Blackboard(데이터) + BehaviorTree(로직)를 분리함

#### **AI 상태 관리: Blackboard 키 설계**
```cpp
namespace CAIKey
{
    // Target
    static const FName TargetActor = "TargetActor";
    static const FName TargetPriority = "TargetPriority";
    
    // State
    static const FName AIStateType = "AIStateType";  // Enum
    
    // Perception
    static const FName bHasLOS = "bHasLOS";
    static const FName LastKnownLocation = "LastKnownLocation";
    
    // Metric
    static const FName DistanceToTarget = "DistanceToTarget";
    static const FName DistanceToHome = "DistanceToHome";
}
```

#### **BehaviorTree 모듈화: Task/Service/Decorator 분리**
```
BehaviorTree (행동 결정 및 실행)
│
├── Service (상태 업데이트, 매 틱)
│   ├── UpdateAIState         → AIStateType 전환
│   ├── UpdateAIContext       → TargetData 갱신
│   ├── UpdateCombatContext   → 전투 거리 계산
│   └── UpdatePatrolContext   → 순찰 도착 판정
│
├── Task (행동 실행, 일회성)
│   ├── SelectPatrolPoint     → 다음 순찰 포인트
│   ├── BeginInvestigate      → 수색 시작
│   ├── SelectAttackIndex     → 공격 패턴 선택
│   └── PlayAttack            → 공격 실행
│
└── Decorator (조건 체크)
    └── HasValidTarget        → TargetActor 유효성
```

**핵심 설계 결정:**
```cpp
// AIController는 Perception만 담당
// → OnTargetPerceptionUpdated에서 TargetDataMap 갱신

// Service가 Context 구성
EPerceptionBuildResult BuildPerceptionContext(FTargetData& Out)
{
    // TargetDataMap에서 최우선순위 선택
    SelectTopPriority(Out);
}

// BehaviorTree가 최종 결정
// Dead → Combat → Investigate → Patrol → Idle
```

**장점:**
- UE AI 인프라를 100% 활용함
- BT 그래프로 흐름을 시각화함
- Task/Service를 재사용 가능함
- 디자이너가 BT만 수정하여 행동을 변경 가능함

---

### 4.6 Context Push 패턴: 상태 스냅샷 백업

**문제:** Overlap 시점에 "어떤 공격"인지 정보가 소실되었음

**해결:** AnimNotify에서 Context를 미리 Attachment에 Push함

```cpp
// 1. AnimNotify_Action(Begin) 호출
UCAnimNotify_Action::Notify(...)
{
    Action->BeginPlayAction();
}

// 2. Action이 현재 상태를 Context로 구성
UCAction_ComboAttack::BeginPlayAction()
{
    FActionContext Context {
        .CurrentActionType = LightAttack,
        .ActionIndex = 2  // 콤보 2번째
    };
    
    WeaponComp->PushContextToAttachment(Context);
}

// 3. WeaponComponent가 모든 Context 결합
UCWeaponComponent::PushContextToAttachment(FActionContext)
{
    FAttachmentContext = { CurrentAttachmentType };
    FEquipmentContext = { CurrentEquipmentType };
    
    // Attachment에 백업
    Attachment->SetLastAttachmentContext(...);
    Attachment->SetLastEquipmentContext(...);
    Attachment->SetLastActionContext(...);
}

// 4. Overlap 시 백업된 Context 사용
ACAttachment::OnComponentBeginOverlap(...)
{
    FHitContext Hit {
        .OverlapContext = BuildOverlapContext(...),
        .AttachmentContext = LastAttachmentContext,  // 백업 사용
        .EquipmentContext = LastEquipmentContext,
        .ActionContext = LastActionContext
    };
    
    ApplyDamageComp->RequestApplyDamage(Hit);
}
```

**장점:**
- Overlap 시점에 풍부한 컨텍스트를 확보함
- ApplyDamage가 SpecKey 생성이 가능함
- 타이밍과 데이터가 분리됨 (Push는 준비, Overlap은 실행)

---

### 4.7 Instigator Resolution: Fallback 체계

**문제:** DamageCauser에 Instigator가 없을 수 있음 (Projectile, AoE 등)

**해결:** 다단계 Fallback으로 안전하게 해석함

```cpp
AController* ResolveInstigatorController(
    AController* EventInstigator, 
    AActor* DamageCauser)
{
    // 1순위: 명시적 Instigator
    if (EventInstigator) return EventInstigator;
    
    if (DamageCauser)
    {
        // 2순위: Causer의 InstigatorController
        if (auto* Ctrl = DamageCauser->GetInstigatorController())
            return Ctrl;
        
        // 3순위: Causer가 Pawn인 경우
        if (auto* Pawn = Cast<APawn>(DamageCauser))
            return Pawn->GetController();
        
        // 4순위: Causer의 Owner 체크 (Proxy)
        if (auto* Owner = DamageCauser->GetOwner())
        {
            if (auto* OwnerCtrl = Owner->GetInstigatorController())
                return OwnerCtrl;
            
            if (auto* OwnerPawn = Cast<APawn>(Owner))
                return OwnerPawn->GetController();
        }
    }
    
    return nullptr;  // 모두 실패 → RejectReason::InvalidInstigator
}
```

**적용 사례:**
- Sword (Direct) → Instigator = PlayerController
- Arrow (Proxy) → Owner = Player → Instigator 추적
- Magic AoE → DamageCauser의 Owner가 Caster

---

## 5. 주요 기술적 성과 및 학습

### 5.1 아키텍처 설계 역량

**단일 책임 원칙(SRP) 실천**
- 7개 Component로 전투 시스템을 분리함 (Apply/Take/Health/Reaction/Weapon/Action/State)
- 각 Component는 하나의 명확한 책임을 가짐

**개방-폐쇄 원칙(OCP) 적용**
- SpecKey 설계로 확장에는 열려있고 수정에는 닫혀있음
- Interface를 활용함 (`IHitContextProducer`, `ITargetContextProducer`)

**의존성 역전 원칙(DIP)**
- Component 간 직접 참조 대신 탐색 + 캐싱을 사용함
- 존재하지 않아도 안전하게 동작함 (nullptr 체크)

---

### 5.2 데이터 흐름 설계

**Payload/Context/Result 패턴**
- 5개 주요 시스템에 일관되게 적용함
- 중간 상태 추적이 가능하고, 디버깅 효율을 극대화함

**SpecKey 기반 확장성**
- 13개 조합 (Attachment × Equipment × Action × Index) → 단일 키로 표현함
- 향후 DataAsset 분리 포인트를 명확화함

**Context Push 패턴**
- AnimNotify 타이밍에서 상태 스냅샷을 백업함
- Overlap 시점에 풍부한 정보를 제공함

---

### 5.3 UE5 C++ 고급 기술

**Custom DamageEvent 구현**
```cpp
struct FDefaultDamageEvent : public FDamageEvent
{
    static const int32 ClassID = EDamageEventTypeId::DefaultDamage;
    
    FApplyDamageSpecKey SpecKey;
    FApplyDamageSpec Spec;
    FApplyDamageResult Result;
};

// TakeDamage에서 타입 판별
if (DamageEvent.IsOfType(FDefaultDamageEvent::ClassID))
    HandleDefaultDamageEvent(...);
```

**BehaviorTree 확장**
- 13개 Custom Task를 구현함 (SelectPatrolPoint, PlayAttack...)
- 5개 Service를 구현함 (UpdateAIState, UpdateCombatContext...)
- 1개 Decorator를 구현함 (HasValidTarget)

**AI Perception 활용**
```cpp
// Sight Sense 구성
UAISenseConfig_Sight* SightConfig;
SightConfig->SightRadius = 1500.f;
SightConfig->LoseSightRadius = 2000.f;
SightConfig->PeripheralVisionAngleDegrees = 90.f;

// TargetDataMap 업데이트
OnTargetPerceptionUpdated(Actor, Stimulus)
{
    if (Stimulus.WasSuccessfullySensed())
        TargetDataMap.Add(Actor, BuildTargetData(...));
    else
        TargetDataMap.Remove(Actor);
}
```

**AnimNotify 커스터마이징**
- `CAnimNotify_Action`을 구현함 (Begin/End/Next 구분)
- `CAnimNotifyState_Reaction`을 구현함 (Interruptible/Cancelable Window)
- Action 타이밍 제어 + Reaction 정책 게이트

---

### 5.4 실무 워크플로우 경험

**Git 브랜치 전략**
- main ← develop ← feature/* 구조를 사용함
- v0.0-setup, v0.1-character-combat-core, v0.2-hit-damage-targeting 태그를 사용함

**Issue 기반 작업 관리**
- 11개 Issue Checklist를 작성함 (D01 ~ D11)
- TODO 항목을 세분화하여 진척도를 명확화함

**PR & Code Review**
- 9개 Pull Request 문서를 작성함
- Before/After, 테스트 방법, 관련 이슈를 명시함
- 설계 의도를 문서화함 (왜 이렇게 설계했는가?)

**체계적인 문서화**
- Issue Analysis Report를 작성함 (MoveTo 도착 판정 분석 등)
- System Architecture를 작성함 (AIStateComp vs BB-BT 비교)
- 150줄 이상의 상세한 PR 설명을 작성함

---

## 6. 현재 진행 상황 (2026.03.13 기준)

### 완료된 마일스톤

#### M0 - 초기 환경 세팅 (v0.0-setup)
- [x] UE5.4 프로젝트를 생성함
- [x] Git/GitHub 리포지토리를 구성함
- [x] 문서 구조 및 워크플로우를 정립함

#### M1 - 캐릭터 & 전투 코어 (v0.1-character-combat-core)
- [x] PlayerCharacter / PlayerController C++ 클래스를 구현함
- [x] 3인칭 카메라 (SpringArm)를 구성함
- [x] 기본 이동 / 점프 / FSM을 구현함 (Idle/Walk/Sprint/Jump)
- [x] 무기 장착/해제 로직을 구현함
- [x] 기본 공격을 구현함
- [x] Test Room 레벨을 구성함

#### M2 - 히트/데미지/리액션 시스템 (v0.2-hit-damage-targeting) 완료
- [x] 콤보 공격 시스템을 구현함
- [x] Dummy Enemy를 추가함
- [x] 근접 Hit 판정을 구현함
- [x] Payload/Context/Result 기반 TakeDamage 파이프라인을 구현함
- [x] CHealthComponent: HP 감소/증가, Dead 판정을 구현함
- [x] CReactionComponent: Hit/Dead Reaction 시스템을 구현함
- [x] AnimNotifyState 기반 Reaction Window 제어를 구현함
- [ ] Lock-on 타게팅 시스템은 보류 중

#### M3 - Enemy AI 시스템 (v0.3-enemy-ai) 진행 중
- [x] AIController + Blackboard + BehaviorTree 파이프라인을 구현함
  - CAIController (기본 클래스)
  - CAIController_Melee (근접 전투 특화)
  - AI Perception (Sight Config)
  
- [x] AI 상태 시스템을 구현함 (EAIStateType)
  - Idle / Patrol / Investigate / Alert / Chase / Combat / HitReact / Dead
  - Service: UpdateAIState, UpdateAIContext
  
- [x] Patrol 시스템을 구현함
  - PatrolPath / PatrolPoint 구조
  - Mode: Random / Loop / Reverse
  - Task: SelectPatrolPoint
  - Service: UpdatePatrolContext
  
- [x] Investigate (수색) 시스템을 구현함
  - 마지막 목격 위치 기반 수색
  - Task: BeginInvestigate / AdvanceInvestigateIndex / EndInvestigate
  - Service: UpdateInvestigateContext
  
- [x] Chase (추적) 시스템을 구현함
  - 마지막 목격 위치로 빠르게 이동
  - 버퍼 기반 완충거리 (ChaseRange + Buffer로 상태 전환 방지)
  - 추적 중 타겟 포커스 유지 (SetFocus)
  - Task: MoveTo (BT 기본 노드), SetFocus
  - Service: UpdateAIContext (거리 갱신, 버퍼 체크)
  
  - [x] Alert (경계) 시스템을 구현함
  - 경계 시 짧은거리를 좌우로 움직이는 AlertStep 구현
  - Task: SelectAlertPoint
  - Service: UpdateAIContext (타겟 재감지, AlertStep 관리)
  
- [x] Combat (전투) 시스템을 구현함
  - Task: InitCombat, SelectAttackIndex, PlayAttack, CommitAttackCooldown
  - Task: SetFocus / ClearFocus, SetMaxWalkSpeed
  - Service: UpdateCombatContext
  - Decorator: HasValidTarget
  
- [x] AIContext 구조를 구현함
  - TargetData (TargetActor, Priority, LOS, LastSeenTime/Location)
  - Combat Metric (DistanceToTarget, InAttackRange, InAlertRange)
  - Navigation Metric (DistanceToHome, ReturnHome)
  
- [ ] HitReact (피격 반응) 시스템 (구현 예정)
  - Task: PlayHitReaction (피격 montage 재생)
  - Service: UpdateHitReactContext (피격 누적, 강인도 체크)
  
- [ ] Dead (사망 처리) 시스템 (구현 예정)
  - Task: HandleDeath (Ragdoll/사망 애니메이션)
  - Service: CheckDeadState (HP 체크, BT 정지)
  
- [ ] 진행 중 / 개선 예정
  - 다수 Enemy vs Player 전투 밸런싱
  - 우선순위 기반 공격 순서 조정
  - 포지셔닝 및 분산(Dispersal) 로직

### 다음 단계 (M4 이후)
- [ ] VFX 추가 (공격/피격 이펙트 강화)
- [ ] UI 개선 (HP Bar, Damage Numbers)
- [ ] 고급 전투 시스템 (Guard, Parry, Dodge)
- [ ] 팀 시스템 / 진영 구분

---

## 주요 시스템 흐름도 (간략 참조용)

### 전투 시스템 데이터 흐름
```
Play Montage → AnimNotify(Begin) → PushContext(Action/Equipment/Attachment)
              ↓
        Collision Window
              ↓
        Overlap Event → BuildHitContext(Overlap + Pushed Contexts)
              ↓
        ApplyDamageComponent
         - BuildSpecKey(AttachmentType × EquipmentType × ActionType × Index)
         - ResolveSpec(SpecKey) → BaseDamage, Multiplier
         - ComputeResult → FinalDamage
         - Target->TakeDamage(FDefaultDamageEvent)
              ↓
        TakeDamageComponent
         - Validate → BuildPayload → BuildContext
         - Evaluate(ComputeMitigated → ComputeFinal)
         - Commit(ApplyToHealth)
         - Dispatch(RequestReaction)
              ↓
        HealthComponent        ReactionComponent
         - TakeDamage()         - ResolveType(Hit/Dead)
         - UpdateDeadState()    - ResolveData(SpecKey + Type)
                                - Play/End Reaction
```

### AI 시스템 상태 흐름
```
AIPerceptionComponent → OnTargetPerceptionUpdated
         ↓
    UpdateTargetDataMap (Actor → TargetData)
         ↓
    Service: UpdateAIContext
         ↓
    BuildPerceptionContext → SelectTopPriority
         ↓
    Blackboard Update (TargetActor, bHasLOS, LastKnownLocation...)
         ↓
    Service: UpdateAIState → Decide Next State
         ↓
    BehaviorTree Branch Selection
         - Dead → Stop All
         - Combat → InitCombat → SelectAttack → PlayAttack
         - Investigate → BeginInvestigate → MoveToLastKnown
         - Patrol → SelectPatrolPoint → MoveTo
         - Idle → Wait
```

---

## 7. 관련 문서 링크

### 기획 문서
- [프로젝트 개요 (KR)](P00_UE5_Portfolio_Plan_Overview%20(KR).md)
- [마일스톤 (KR)](P01_UE5_Portfolio_Milestones%20(KR).md)
- [개발 로드맵 (KR)](P02_UE5%20Portfolio_Development%20Roadmap%20(KR).md)

### 시스템 설계
- [시스템 아키텍처 (KR)](../05_SystemArchitecture/S01_UE5_Portfolio_System_Architecture%20(KR).md)

### 개발 이력
- [Issue Checklists](../01_Issue_CheckList/)
- [Pull Requests](../04_Pull_Request/)
  - [P08: TakeDamage 파이프라인](../04_Pull_Request/P08_UE5_Portfolio_Pull_Request%20(KR).md)
  - [P09: Reaction 파이프라인](../04_Pull_Request/P09_UE5_Portfolio_Pull_Request%20(KR).md)
- [Bug Reports](../03-01_Bug_Report/)

### 프로젝트 루트
- [README.md](../../README.md)

---

## 8. 추가 자료

### Feature별 주요 구현 파일

#### [1] 전투 시스템 (Combat System)
전투 파이프라인의 핵심 Component 및 데이터 구조

```
Component/
├── CApplyDamageComponent       # 공격 → TakeDamage 호출 파이프라인
├── CTakeDamageComponent        # TakeDamage 수신 → 처리 오케스트레이션
├── CHealthComponent            # HP 리소스 관리 (증감, Dead 판정)
└── CReactionComponent          # 피격 리액션 실행 (Hit/Dead)

Weapon/
├── CAttachment                 # 충돌 체크 Actor (Sword, Bow...)
└── CEquipment                  # 장비 속성 (Fire, Ice...)

Action/
├── CAction                     # 액션 베이스 클래스
├── CAction_ComboAttack         # 콤보 공격 구현
└── CAction_LightAttack         # 기본 공격 구현

Reaction/
├── CReaction                   # 리액션 베이스 클래스
├── CReaction_Hit               # 피격 리액션
└── CReaction_Dead              # 사망 리액션

Type/
├── CWeaponStructure.h          # FHitContext, FApplyDamageSpec, FApplyDamageResult
└── DamageEventId.h             # FDefaultDamageEvent 정의

Notify/
├── CAnimNotify_Collision       # 충돌 박스 On/Off
├── CAnimNotify_Action          # Begin/End/Next 타이밍 제어
└── CAnimNotifyState_Reaction   # Interruptible/Cancelable Window
```

#### [2] AI 시스템 (AI System)
Blackboard + BehaviorTree 기반 적 AI 시스템

```
Controller/
├── CAIController               # AI Perception, Blackboard, BehaviorTree 초기화
└── AIController/
    └── CAIController_Melee     # 근접 전투 특화 Controller

AI/BehaviorTree/
├── Task/ (13개)
│   ├── CBTTask_SelectPatrolPoint        # 순찰 포인트 선택
│   ├── CBTTask_BeginInvestigate         # 수색 시작
│   ├── CBTTask_AdvanceInvestigateIndex  # 수색 포인트 전진
│   ├── CBTTask_EndInvestigate          # 수색 종료
│   ├── CBTTask_InitCombat              # 전투 초기화
│   ├── CBTTask_SelectAttackIndex       # 공격 패턴 선택
│   ├── CBTTask_PlayAttack              # 공격 실행
│   ├── CBTTask_CommitAttackCooldown    # 공격 쿨다운 적용
│   ├── CBTTask_SelectAlertPoint        # 경계 위치 선택
│   ├── CBTTask_SetFocus                # 타겟 주시
│   ├── CBTTask_ClearFocus              # 주시 해제
│   └── CBTTask_SetMaxWalkSpeed         # 이동 속도 변경
│
├── Service/ (5개)
│   ├── CBTService_UpdateAIState            # AI 상태 전환 결정
│   ├── CBTService_UpdateAIContext          # Target 정보 갱신
│   ├── CBTService_UpdatePatrolContext      # 순찰 도착 판정
│   ├── CBTService_UpdateInvestigateContext # 수색 진행도 갱신
│   └── CBTService_UpdateCombatContext      # 전투 거리/쿨다운 갱신
│
└── Decorator/ (1개)
    └── CBTDecorator_HasValidTarget     # 타겟 유효성 체크

AI/Patrol/                              # 순찰 경로 시스템
AI/BlackBoard/                          # Blackboard Key 정의 (CAIKey)

Character/Enemy/
└── CEnemy                              # 적 캐릭터 베이스 클래스

Type/
├── CAIStructure.h                      # FAIContext, FTargetData
└── CAIStateStructure.h                 # EAIStateType, FPatrolContext
```

#### [3] 플레이어 시스템 (Player System)
플레이어 캐릭터 및 입력 처리

```
Character/Player/
└── CPlayer                             # 플레이어 캐릭터 클래스

Controller/
└── CPlayerController                   # 플레이어 입력 처리

Component/
├── CStateComponent                     # 캐릭터 상태 관리 (FSM)
├── CMovementComponent                  # 이동 로직 (Sprint, Dodge...)
├── CActionComponent                    # 액션 실행 관리
└── CWeaponComponent                    # 무기 장착/해제 관리

Type/
└── CStateStructure.h                   # EStateType, FStateData
```

#### [4] 애니메이션 시스템 (Animation System)
Animation Blueprint 및 Notify 시스템

```
Character/
└── CAnimInstance                       # Animation Blueprint C++ 클래스

Notify/
├── CAnimNotify                         # Notify 베이스 클래스
├── CAnimNotify_Action                  # 액션 타이밍 제어 (Begin/End/Next)
├── CAnimNotify_Collision               # 충돌 박스 On/Off
├── CAnimNotify_Equip                   # 무기 장착 타이밍
├── CAnimNotify_Unequip                 # 무기 해제 타이밍
├── CAnimNotify_PreInput                # 선입력 윈도우
├── CAnimNotifyState                    # NotifyState 베이스
└── CAnimNotifyState_Reaction           # 리액션 윈도우 (Interrupt/Cancel)
```

#### [5] 공통 / 유틸리티 (Common/Utilities)
Interface, 전역 타입, 시스템

```
Interface/
├── HitContextProducer                  # FHitContext 생성 인터페이스
└── TargetContextProducer               # FTargetContext 생성 인터페이스

Core/                                   # 시스템 초기화, 설정
System/                                 # WorldSubsystem (향후 확장)

Type/
├── DamageEventId.h                     # Custom DamageEvent ID
└── CWorldSubSystemStructure.h          # WorldSubsystem 타입 정의

ProjectGlobal.h                         # 전역 매크로/헬퍼 함수
Portfolio.Build.cs                      # 빌드 설정 (PCH, 모듈 의존성)
```

---
