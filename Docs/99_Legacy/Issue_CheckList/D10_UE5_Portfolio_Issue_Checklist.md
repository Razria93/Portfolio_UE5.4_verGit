# UE5 Portfolio – Issue Checklist

## 제목

**M02-05: CReactionComponent 및 Reaction Pipeline 구현**

### 날짜

- **Day 10**

- **Date : 2026.01.11**

---
### 브랜치

- feature/combat-reaction

---
### 목표

- `UCTakeDamageComponent`는 데미지 수신 및 결과 산출에 집중하고, **피격 리액션 (연출/상태 전이 요청)은 `UCReactionComponent`로 분리**함

- `FTakeDamageResult`를 입력으로 받아 **Reaction 요청을 표준화된 데이터로 구성**하고, `ReactionComponent`가 실행 책임을 갖도록 구성함

- 최소 구현 범위에서 **HitReaction / DeadReaction**을 확정하고, 후속 확장 (GuardBreak, Launch, HitStop, Stagger Tier 등)을 위한 확장 포인트를 남김

---
### TODO 리스트

#### 1. Reaction 데이터 표준화 구조 정의

- [x] `FReactionPayload / FReactionContext / FReactionResult` 정의

- [x] 구조체 책임 분리 원칙 확정
    - [x] Payload: 입력 원본 
	    - Attacker / DamageCauser / SpecKey / TakeDamageResult 등
    - [x] Context: Resolved 객체 + 상태 스냅샷 + 절차별 파생 값
		- ex. HitDirection, KnockbackStrength
    - [x] Result: 최종 결정
	    - Executed/Rejected, RejectReason, PlayedMontage, bKilledReaction 등

- [x] `ETakeDamageRejectReason`와 별개로 Reaction 단계 전용 RejectReason 필요 여부 검토
    - ex. `NoReactionComp`, `AlreadyReacting`, `NoMontage`, `DeadStateBlocked`, `RuleBlocked`

#### 2. UCReactionComponent 생성 및 엔트리 API 정의

- [x] `UCReactionComponent` 클래스 생성 및 Enemy(또는 Damageable Actor)에 부착 경로 확보

- [x] 엔트리 API 정의
    - `RequestReaction(const FTakeDamagePayload& Payload, const FTakeDamageContext& Context, const FTakeDamageResult& Result)`

- [x] Tick 정책 정리
    - `PrimaryComponentTick.bCanEverTick = false` 유지
    - 지속 처리(HitStop 타이머 등)가 필요하면 Timer 기반으로만 처리

#### 3. TakeDamageComponent → ReactionComponent 연동 지점 확정

- [x] `UCTakeDamageComponent`의 **Commit 이후** 단일 지점에서 Reaction 요청을 생성 및 위임
    - 권장 위치: `CommitTakeDamage()` 성공 이후 또는 `Finalize` 단계 이후

- [x] 연동 규칙 확정
    - `FTakeDamageResult.Accepted == true`일 때만 Reaction 진행
    - `FinalAppliedDamage <= 0`이면 Reaction 진행 여부 정책화
        - ex. Shield로 0이 된 경우 “경미 리액션”만 허용 등

- [x] 컴포넌트 탐색/캐싱 규칙 확정
    - [x] Enemy 초기화 시 `ReactionComp_Cached` 캐싱
    - [x] 없으면 안전하게 Reject 처리 + 로그 출력

#### 4. Reaction 라우팅 및 최소 리액션 구현

- [x] DefaultDamageEvent 기준 최소 라우팅 규칙 구현
    - 입력: `DamageEventTypeID`, `SpecKey`, `FinalAppliedDamage`, `bKilled`

- [x] 최소 리액션 1: HitReaction
    - [x] Hit 몽타주(또는 애님) 재생
    - [x] 연속 피격 시 중복 재생 방지 규칙 추가(쿨다운 또는 “AlreadyReacting” 플래그)

- [x] 최소 리액션 2: DeadReaction
    - [x] `bKilled == true` 또는 `Health <= 0` 조건에서 사망 리액션 실행
    - [x] 사망 상태에서는 HitReaction 차단 우선순위 확정(Dead > Hit)

- [ ] 선택 구현(옵션): Knockback / Launch / HitStop
    - [ ] 수치 산출 위치를 `ReactionContext`로 고정(데이터 기반 확장 대비)
    - [ ] 실제 적용은 CharacterMovement/Physics 쪽으로 요청만 보내도록 구성(직접 의존 최소화)

#### 5. Reaction 데이터 소스 설계(확장 대비)

- [x] 초기: `TMap<FDamageSpecKey, FReactionSpec>` 또는 단순 조건 분기 기반으로 구성

- [x] 후속 확장 대비: DataAsset 분리 포인트 확보
    - ex. `UReactionSpecDataAsset` 또는 `UReactionProfileDataAsset`

- [x] `SpecKey`가 없거나 매칭 실패 시의 폴백 정책 확정
    - ex. Default HitReaction, 또는 Reject 처리

#### 6. 디버그 출력 및 추적성 확보

- [x] `UCReactionComponent::PrintReactionSummaryInfo()` 구성
    - Target, Instigator, DamageCauser, SpecKey, ExecutedReactionType, bKilledReaction 등

- [x] `UCReactionComponent::PrintReactionContextInfo()` 구성
    - Payload/Context/Result 상세 출력

- [x] “단일 히트 이벤트당 1회 Reaction 로그” 규칙 적용
    - TakeDamage와 동일한 중복 방지 전략 적용

#### 7. 통합 검증 시나리오

- [x] Apply → TakeDamage → Health → Reaction까지 호출 흐름 확인

- [x] 시나리오 1: 정상 히트 1회
    - HitReaction 1회 실행 + 로그 1회

- [x] 시나리오 2: 연속 히트(Overlap window 내 중복)
    - 중복 리액션 차단 또는 정책대로 제한

- [x] 시나리오 3: 사망 조건 히트
    - DeadReaction 실행, HitReaction 우선순위 정책 확인

- [ ] 시나리오 4: Shield/Absorb로 AppliedDamage가 0인 케이스
    - Reaction 정책대로 실행/차단 확인

---
### 비고

- Shield / Absorb / Knockback / HitStop / GuardBreak는 후속 확장 검토 범위로 남긴다.

- Reaction은 “연출 실행”이므로, TakeDamage 계산/정책 로직과 강하게 섞이지 않도록 **요청 생성(Policy)과 실행(ReactionComp)을 분리**함

- 초기 구현은 최소 범위(Hit/Dead)만 확정하고, Knockback / HitStop / GuardBreak는 구조만 열어두고 후속 이슈에서 확장

---
