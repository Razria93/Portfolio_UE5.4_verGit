## 1. 기술 스택

**개발 환경**

- Unreal Engine 5.4
- C++
- Visual Studio 2022
- Windows PC

**AI 활용 도구**

- Codex

**프로젝트 관리 및 문서화**

- Git / GitHub
- GitHub Issues
- GitHub Pull Requests
- Obsidian
- Markdown

---
## 2. 구현 기능

**Character / Movement**

- Player / Enemy character 구성
- movement request 처리 흐름 구현
- Move / Walk / Run / Sprint 입력 처리 구현
- Jump / StopJump 입력 처리 구현
- Idle / Action / Reaction / Dead execution state 관리 구현

**Weapon / Combat Action**

- WeaponActor 장착 / 해제 처리 구현
- Equip / Unequip action 구현
- ComboAttack action 구현
- Combo chain reserve / consume 흐름 구현
- Dodge action 구현
- action montage lifecycle 처리 구현
- action notify command 처리 구현

**Damage 처리**

- weapon collision 기반 hit context 구성 구현
- hit window id 관리 구현
- duplicate hit 방지 처리 구현
- apply damage request 생성 처리 구현
- `FDamageEvent` 기반 damage 전달 구현
- take damage event 해석 처리 구현
- damage 계산 및 health commit 처리 구현
- damage result 기반 hit / dead reaction request 생성 처리 구현

**Combat Reaction**

- HitReaction 구현
- DeadReaction 구현
- reaction montage lifecycle 처리 구현
- reaction notify command 처리 구현
- action / reaction 간 intervention 처리 구현

**Feedback 처리**

- action feedback 요청 및 재생 처리 구현
- reaction feedback 요청 및 재생 처리 구현
- damage feedback 요청 및 재생 처리 구현
- VFX / SFX / trail feedback 처리 구현
- hit impact feedback 처리 구현
- feedback request / component dispatch 구조 구성

**AI 처리**

- Behavior Tree / Blackboard 기반 Enemy AI 구성
- AI context update service 구현
- patrol / investigate / alert / engage 상태 처리 구현
- movement intent request task 구현
- AI combat action request task 구현
- AI reaction / dead 처리 구현
- Player와 공유하는 action / reaction execution request 흐름 구성

**Data 구성**
- ActionData container / key resolve 구조 구성
- ReactionData container / key resolve 구조 구성
- ApplyDamageSpec container / key resolve 구조 구성
- feedback data resolve 구조 구성
- wildcard / fallback lookup 처리 구현

**State 및 Resource 처리** 
- state / health / movement component 구현

**System / Utility**
- combat engage world subsystem 구성
- combat feedback world subsystem 구성
- target / hit context provider interface 구성
- custom debug log utility 구현

---
## 3. 핵심 설계 포인트

**Combat Data Processing Pipeline**

- 공격 판정 이후 hit context, damage event, health commit, reaction request로 이어지는 damage 처리 흐름을 정리했다.
- 상세 내용: `PF02_UE5_Portfolio_Document`

**Action / Reaction Execution Pipeline**

- Action과 Reaction을 공통 execution pipeline 위에서 처리하도록 정리했다.
- Orchestrator / Component / Executor / Data의 책임을 분리했다.
- 상세 내용: `PF03_UE5_Portfolio_Document`

**Enemy AI Combat Behavior**

- AI가 별도 전투 실행 흐름을 갖지 않고, Player와 같은 action / reaction execution 구조를 사용하도록 정리했다.
- Behavior Tree는 판단과 request 생성을 담당하고, 실제 실행은 공통 execution pipeline에서 처리한다.
- 상세 내용: `PF04_UE5_Portfolio_Document`

**Data-Driven Design**

- 전투 기능을 코드 분기보다 data container와 key 기반 resolve 흐름으로 구성했다.
- action / reaction / damage / feedback 설정을 실행 로직과 분리했다.
- 상세 내용: `PF05_UE5_Portfolio_Document`

---
## 4. 트러블 슈팅 사례

**Unreal Asset / Editor 처리 문제**

- skeleton retargeting, `USTRUCT` reference, Blueprint parent class redirect 문제를 정리했다.

**AI Context / Blackboard 상태 문제**

- Blackboard cleanup 누락, `AttackIndex` 초기화 누락으로 인한 AI 상태 불일치 문제를 정리했다.

**Damage Pipeline Timing 문제**

- collision enable과 hit window id 준비 순서가 어긋나 첫 hit가 reject되던 문제를 정리했다.

**AI Combo Chain 흐름 문제**

- AI가 Player와 같은 combo chain request 흐름을 사용하지 않아 combo가 이어지지 않던 문제를 정리했다.

**Reaction Takeover 상태 정리 문제**

- HitReaction 진입 시 기존 active action 상태가 정리되지 않아 action / state / blackboard 상태가 어긋나던 문제를 정리했다.

상세 내용: `PF06_UE5_Portfolio_Document`

---
