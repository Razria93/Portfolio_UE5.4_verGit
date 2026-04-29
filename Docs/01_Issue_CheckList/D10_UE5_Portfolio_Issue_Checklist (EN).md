# UE5 Portfolio – Issue Checklist

## Title

**M02-05: Implement post-TakeDamage Reaction pipeline (Introduce CReactionComponent + Result-based orchestration)**

### Date

- **Day 10**
    
- **Date : 2026.01.11**
    

---

### Goals

- `UCTakeDamageComponent` focuses on damage receiving and result computation, and **hit reactions (presentation / state transition requests) are separated into `UCReactionComponent`**
    
- Use `FTakeDamageResult` as input to **build Reaction requests into standardized data**, and make `ReactionComponent` responsible for execution
    
- Confirm the minimum implementation scope as **HitReaction / DeadReaction**, and leave extension points for follow-ups (GuardBreak, Launch, HitStop, Stagger Tier, etc)
    

---

### Branch

- feature/combat-reaction
    

---

### TODO List

#### 1. Define standardized Reaction data structures

- [x] Define `FReactionPayload / FReactionContext / FReactionResult`
    
- [x] Finalize principles for struct responsibility separation
    
    - [x] Payload: raw input
        
	    - Attacker / DamageCauser / SpecKey / TakeDamageResult, etc
        
    - [x] Context: resolved objects + state snapshot + per-stage derived values
        
		- ex. HitDirection, KnockbackStrength
        
    - [x] Result: final decision
        
	    - Executed/Rejected, RejectReason, PlayedMontage, bKilledReaction, etc
        
- [x] Review whether Reaction needs its own RejectReason separate from `ETakeDamageRejectReason`
    
    - ex. `NoReactionComp`, `AlreadyReacting`, `NoMontage`, `DeadStateBlocked`, `RuleBlocked`
        

---

#### 2. Create UCReactionComponent and define entry APIs

- [x] Create `UCReactionComponent` and secure an attach path on Enemy (or any Damageable Actor)
    
- [x] Define entry API
    
    - `RequestReaction(const FTakeDamagePayload& Payload, const FTakeDamageContext& Context, const FTakeDamageResult& Result)`
        
- [x] Define tick policy
    
    - Keep `PrimaryComponentTick.bCanEverTick = false`
        
    - If continuous processing is required (ex. HitStop timers), use Timer-based processing only
        

---

#### 3. Finalize the TakeDamageComponent → ReactionComponent integration point

- [x] Generate and delegate the Reaction request from a single point **after Commit** in `UCTakeDamageComponent`
    
    - Recommended location: after `CommitTakeDamage()` succeeds or after a `Finalize` stage
        
- [x] Finalize integration rules
    
    - Proceed with Reaction only when `FTakeDamageResult.Accepted == true`
        
    - If `FinalAppliedDamage <= 0`, define a policy for whether Reaction proceeds
        
        - ex. allow only “minor reactions” when Shield reduces it to 0
        
- [x] Finalize component lookup / caching rules
    
    - [x] Cache `ReactionComp_Cached` during Enemy initialization
        
    - [x] If missing, safely reject + print logs
        

---

#### 4. Implement Reaction routing and minimum reactions

- [x] Implement minimum routing rules for DefaultDamageEvent
    
    - Input: `DamageEventTypeID`, `SpecKey`, `FinalAppliedDamage`, `bKilled`
        
- [x] Minimum Reaction 1: HitReaction
    
    - [x] Play Hit montage (or animation)
        
    - [x] Add a rule to prevent duplicate playback on consecutive hits (cooldown or an “AlreadyReacting” flag)
        
- [x] Minimum Reaction 2: DeadReaction
    
    - [x] Execute death reaction when `bKilled == true` or `Health <= 0`
        
    - [x] Finalize priority so HitReaction is blocked while dead (Dead > Hit)
        
- [ ] Optional implementation: Knockback / Launch / HitStop
    
    - [ ] Fix the derived-value computation location to `ReactionContext` (for data-driven extensibility)
        
    - [ ] Apply effects by sending requests to CharacterMovement/Physics (minimize direct dependencies)
        

---

#### 5. Design Reaction data sources (for future extensibility)

- [x] Initial: implement with `TMap<FDamageSpecKey, FReactionSpec>` or simple conditional branching
    
- [x] For follow-up extensibility: secure a DataAsset split point
    
    - ex. `UReactionSpecDataAsset` or `UReactionProfileDataAsset`
        
- [x] Finalize fallback policy when `SpecKey` is missing or matching fails
    
    - ex. Default HitReaction, or reject
        

---

#### 6. Ensure debug output and traceability

- [x] Implement `UCReactionComponent::PrintReactionSummaryInfo()`
    
    - Target, Instigator, DamageCauser, SpecKey, ExecutedReactionType, bKilledReaction, etc
        
- [x] Implement `UCReactionComponent::PrintReactionContextInfo()`
    
    - Detailed output for Payload/Context/Result
        
- [x] Apply the rule “one Reaction log per single hit event”
    
    - Apply the same duplicate-prevention strategy as TakeDamage
        

---

#### 7. Integration validation scenarios

- [x] Verify call flow: Apply → TakeDamage → Health → Reaction
    
- [x] Scenario 1: single valid hit
    
    - HitReaction executes once + one log
        
- [x] Scenario 2: consecutive hits (duplicate within overlap window)
    
    - Duplicate reactions are blocked or limited by policy
        
- [x] Scenario 3: killing hit
    
    - DeadReaction executes, and HitReaction priority policy is verified
        
- [ ] Scenario 4: AppliedDamage becomes 0 due to Shield/Absorb
    
    - Verify execute/block behavior matches the Reaction policy
        

---

### Notes

- Because Reaction is “presentation execution”, separate **request generation (Policy)** and execution (ReactionComp) so it does not mix strongly with TakeDamage compute/policy logic
    
- The initial implementation confirms only the minimum scope (Hit/Dead), and leaves only the structure for Knockback / HitStop / GuardBreak, expanding them in follow-up issues
    

---
