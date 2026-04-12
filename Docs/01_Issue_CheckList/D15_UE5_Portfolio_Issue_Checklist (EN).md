# UE5 Portfolio Issue Checklist

## Title

**M04-01: Implement and Organize Combat Feedback**

### Date

- **Day 15**
  
- **Date : 2026.04.11**


---

### Goals

- Move combat beyond a structure-only presentation and implement and organize **Combat Feedback** so that hit results are actually readable and tangible in play.
  
- Connect the post-hit presentation flow consistently around `TakeDamage -> Reaction -> ReactionFX`.


---

### Branch
- `feature/combat-feedback`


---

### TODO List

#### 1. Organize Feedback Structure

- [ ] Re-check current responsibilities of `ReactionComponent` and `ReactionFXComponent`
      
- [ ] Organize the feedback trigger point after `TakeDamageCommitted`
      
- [ ] Organize the execution order between Reaction and Feedback
      
- [ ] Organize feedback block or finish rules for Dead / Revive


#### 2. Implement First-Pass Hit Feedback

- [ ] Apply Hit Stop
      
- [ ] Apply Hit VFX
      
- [ ] Apply Hit Sound
      
- [ ] Apply Camera Shake
      
- [ ] Check whether strong hits / weak hits need differentiated feedback


#### 3. Check Perceived Combat Quality

- [ ] Check the feel of a basic single-hit reaction
      
- [ ] Check chained feedback during combo hits
      
- [ ] Check feedback behavior right before Dead / after Dead
      
- [ ] Check whether the end of Reaction feels natural


#### 4. Minimal Validation
- [ ] Scenario 1: Player attack -> Enemy HitReact + Feedback
      
- [ ] Scenario 2: Combo attack -> chained feedback works correctly
      
- [ ] Scenario 3: Dead transition -> feedback finish rule works correctly
      
- [ ] Scenario 4: Feedback reset after Revive


---

### Notes

- This issue focuses on **making combat actually visible and tangible in play**, rather than adding new combat rules.
- Treat this branch as the presentation baseline for later `Combat Validation`, `AI Combat Loop`, and `CounterAction` work.


---
