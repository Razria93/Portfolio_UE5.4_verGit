# UE5 Portfolio Pull Request Fix

## 제목

**F04: Combat Signal Target 흐름 순서 정렬 보정**

## 날짜

**2026.06.23**

## 상태

- [x] **완료**

---

## 브랜치

- `fix/combat-signal-target-flow-order`

---

## 요약

이번 Fix PR에서는 `UCTakeDamageComponent`의 선언부와 정의부 순서를 Combat Signal Target 기준 흐름에 맞게 정렬했다.

기능 동작이나 combat 처리 로직은 변경하지 않고, 이후 `Combat Signal Target` 리팩터링을 읽고 이어가기 쉽도록 API 배치만 보정했다.

---

## 변경 사항

- `CTakeDamageComponent.h`의 private API 구간을 다음 순서로 정리했다.

```text
Entry
Receive
Evaluate
Apply
Packet
Notify
Helper
Debug
```

- `CTakeDamageComponent.cpp`의 함수 정의 순서를 헤더 선언 순서와 일치시켰다.

- `BuildPacket`은 Packet 구간으로 이동했다.

- `ResolveInstigatorController`, `CommitDamageToHealth`, `ResolveCombatResultReceiverActor`, `BuildCombatResultPacket`은 Helper 구간으로 모았다.

---

## 변경하지 않은 것

- damage 수신/판정/적용 로직

- Guard / Parry / Defensive Outcome 처리

- CombatResultPacket 생성 값

- reaction / feedback / result dispatch 동작

- 문서 구조나 Work List 상태

---

## 검증 결과

- `git diff --check` 통과

- `PortfolioEditor Win64 Development` 빌드 성공

- 최종 워킹트리 기준 코드 변경 파일:

```text
Source/Portfolio/Component/CTakeDamageComponent.h
Source/Portfolio/Component/CTakeDamageComponent.cpp
```

---

## 관련 문서

- `Docs/04_Pull_Request/P22_UE5_Portfolio_Pull_Request.md`

- `Docs/06_Notes/Task_Briefs/TB_W04_03_Combat_Signal_Target_Boundary_v1.md`

---
