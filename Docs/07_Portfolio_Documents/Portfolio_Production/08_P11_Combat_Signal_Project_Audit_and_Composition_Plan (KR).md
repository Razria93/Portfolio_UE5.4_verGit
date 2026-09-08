# 현행 p.13 Combat Signal Source Boundary 프로젝트 감사 및 구성 계획

> 파일명은 초기 22페이지 체계의 감사 기록을 보존한다. 현행 25페이지 체계에서는 p.13에 적용한다.

> 상태: **코드·설계 문서 감사 완료 / Damage·Cue Source Boundary 다이어그램 반영 / 런타임 캡처 대기**  
> 목적: p.13은 Source가 hit/cue 사실을 Packet으로 구성·전달하고 Target을 직접 조작하지 않는 단방향 책임 경계를 설명한다. Target Outcome 상세는 p.14에서 다룬다.

## 확인 근거

- `CCombatSignalSourceComponent.cpp`: Hit Window / overlap의 `FHitContext`를 source context로 정규화하고 DamageSpec·request damage를 resolve한 뒤 target에 전달한다.
- `CCombatSignalTargetComponent.cpp`: target context에서 유효성·방어·피해·반응을 resolve하고, `FCombatSignalTargetPacket`을 만든 뒤 accepted 결과를 dispatch한다.
- `FDebugOverlaySnapshotStore.cpp`: TargetPacket의 damage breakdown, defense/reaction outcome, HP before/after를 complete resolution으로 기록한다.
- `FCombatSignalDebug.cpp`: packet과 dispatch 결과를 audit 가능한 형식으로 출력한다.

## 확정 주장 범위

1. Source는 hit input을 정규화하고, Target은 방어·자원·반응 결과를 확정한다.
2. `FCombatSignalTargetPacket`은 payload·target context·result를 결합한 결과 전달 계약이다.
3. Resource Commit, Reaction Request, Feedback Dispatch는 같은 결과 packet을 소비하지만 서로를 직접 실행하지 않는다.
4. p.13은 Source/전달 경계 페이지다. Guard/Parry Outcome 비교는 p.14, execution policy는 p.11~p.12, Pair 거래는 p.15에서 다룬다.

## 페이지 구성

1. 구조 문제: 공격자가 Target 상태·Montage·반응 종류까지 지정할 때 발생하는 객체 간 결합을 짧은 callout으로 제시한다.
2. 중심: 사용자 제작 `Diagram/PP/Combat Signal Source.png`, `Combat Signal Source2.png`를 `Assets/Diagrams/p13_combat_signal_damage.png`, `p13_combat_signal_cue.png`로 복사해 삽입했다. Hit 경로는 `Hit Window / Overlap → FHitContext → Source Payload/Context/DamageSpec → FDefaultDamageEvent → TargetActor::TakeDamage → RequestCombatDamageTarget`, Cue 경로는 `Cue Tag → FCombatSignal → SendCueSignal → RequestCombatSignalTarget`으로 표기한다.
3. 구조 해결: Source Normalize, Delivery Entry, Target Boundary의 책임 3칸으로 정리한다. Target Outcome의 결과 확정은 p.14로 넘긴다.

## 증거 제한

- 코드·타입 계약이 주 근거다. p.6 FinalCandidate 이미지를 중복 배치하지 않는다.
- E13은 같은 사건의 Source Context와 Target 수신/dispatch trace를 보조 증거로 사용한다. Normal/Guard/Parry 비교와 HP 전후는 p.14 E14에서만 사용한다.
- p.13 본문에는 `런타임 검증` 카드나 별도 문제 해결 경험 카드를 배치하지 않는다. E13은 Evidence Ledger에서만 추적한다.

## 25페이지 전환 기준 (2026-09-06)

- 이 문서는 S38의 p.13 Source Boundary 감사로 사용한다.
- B10 terminal cleanup과 C14 Data-driven Resolve는 p.13 대표 근거에서 제외한다.
- Timing Cue는 현재 tag 전달·수용 코드까지만 확인됐다. 실제 Blink/Repulse gameplay effect로 표현하지 않는다.
