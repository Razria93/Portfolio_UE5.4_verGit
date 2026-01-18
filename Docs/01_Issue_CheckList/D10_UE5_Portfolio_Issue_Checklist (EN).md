# UE5 Portfolio – Issue Checklist

## Title

**M2-05: Implement post-TakeDamage Reaction pipeline (Introduce CReactionComponent + Result-based orchestration)**

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

- [ ] Define `FReactionPayload / FReactionContext / FReactionResult`
    
- [ ] Finalize principles for struct responsibility separation
    
    - [ ] Payload: raw input
        
	    - Attacker / DamageCauser / SpecKey / TakeDamageResult, etc
        
    - [ ] Context: resolved objects + state snapshot + per-stage derived values
        
		- ex. HitDirection, KnockbackStrength
        
    - [ ] Result: final decision
        
	    - Executed/Rejected, RejectReason, PlayedMontage, bKilledReaction, etc
        
- [ ] Review whether Reaction needs its own RejectReason separate from `ETakeDamageRejectReason`
    
    - ex. `NoReactionComp`, `AlreadyReacting`, `NoMontage`, `DeadStateBlocked`, `RuleBlocked`
        

---

#### 2. Create UCReactionComponent and define entry APIs

- [ ] Create `UCReactionComponent` and secure an attach path on Enemy (or any Damageable Actor)
    
- [ ] Define entry API
    
    - `RequestReaction(const FTakeDamagePayload& Payload, const FTakeDamageContext& Context, const FTakeDamageResult& Result)`
        
- [ ] Define tick policy
    
    - Keep `PrimaryComponentTick.bCanEverTick = false`
        
    - If continuous processing is required (ex. HitStop timers), use Timer-based processing only
        

---

#### 3. Finalize the TakeDamageComponent → ReactionComponent integration point

- [ ] Generate and delegate the Reaction request from a single point **after Commit** in `UCTakeDamageComponent`
    
    - Recommended location: after `CommitTakeDamage()` succeeds or after a `Finalize` stage
        
- [ ] Finalize integration rules
    
    - Proceed with Reaction only when `FTakeDamageResult.Accepted == true`
        
    - If `FinalAppliedDamage <= 0`, define a policy for whether Reaction proceeds
        
        - ex. allow only “minor reactions” when Shield reduces it to 0
        
- [ ] Finalize component lookup / caching rules
    
    - [ ] Cache `ReactionComp_Cached` during Enemy initialization
        
    - [ ] If missing, safely reject + print logs
        

---

#### 4. Implement Reaction routing and minimum reactions

- [ ] Implement minimum routing rules for DefaultDamageEvent
    
    - Input: `DamageEventTypeID`, `SpecKey`, `FinalAppliedDamage`, `bKilled`
        
- [ ] Minimum Reaction 1: HitReaction
    
    - [ ] Play Hit montage (or animation)
        
    - [ ] Add a rule to prevent duplicate playback on consecutive hits (cooldown or an “AlreadyReacting” flag)
        
- [ ] Minimum Reaction 2: DeadReaction
    
    - [ ] Execute death reaction when `bKilled == true` or `Health <= 0`
        
    - [ ] Finalize priority so HitReaction is blocked while dead (Dead > Hit)
        
- [ ] Optional implementation: Knockback / Launch / HitStop
    
    - [ ] Fix the derived-value computation location to `ReactionContext` (for data-driven extensibility)
        
    - [ ] Apply effects by sending requests to CharacterMovement/Physics (minimize direct dependencies)
        

---

#### 5. Design Reaction data sources (for future extensibility)

- [ ] Initial: implement with `TMap<FDamageSpecKey, FReactionSpec>` or simple conditional branching
    
- [ ] For follow-up extensibility: secure a DataAsset split point
    
    - ex. `UReactionSpecDataAsset` or `UReactionProfileDataAsset`
        
- [ ] Finalize fallback policy when `SpecKey` is missing or matching fails
    
    - ex. Default HitReaction, or reject
        

---

#### 6. Ensure debug output and traceability

- [ ] Implement `UCReactionComponent::PrintReactionSummaryInfo()`
    
    - Target, Instigator, DamageCauser, SpecKey, ExecutedReactionType, bKilledReaction, etc
        
- [ ] Implement `UCReactionComponent::PrintReactionContextInfo()`
    
    - Detailed output for Payload/Context/Result
        
- [ ] Apply the rule “one Reaction log per single hit event”
    
    - Apply the same duplicate-prevention strategy as TakeDamage
        

---

#### 7. Integration validation scenarios

- [ ] Verify call flow: Apply → TakeDamage → Health → Reaction
    
- [ ] Scenario 1: single valid hit
    
    - HitReaction executes once + one log
        
- [ ] Scenario 2: consecutive hits (duplicate within overlap window)
    
    - Duplicate reactions are blocked or limited by policy
        
- [ ] Scenario 3: killing hit
    
    - DeadReaction executes, and HitReaction priority policy is verified
        
- [ ] Scenario 4: AppliedDamage becomes 0 due to Shield/Absorb
    
    - Verify execute/block behavior matches the Reaction policy
        

---

### Notes

- Because Reaction is “presentation execution”, separate **request generation (Policy)** and execution (ReactionComp) so it does not mix strongly with TakeDamage compute/policy logic
    
- The initial implementation confirms only the minimum scope (Hit/Dead), and leaves only the structure for Knockback / HitStop / GuardBreak, expanding them in follow-up issues
    

---
