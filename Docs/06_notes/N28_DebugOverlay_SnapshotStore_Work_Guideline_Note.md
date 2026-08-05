# N28 DebugOverlay SnapshotStore Work Guideline Note

## 1) 목적
- `FDebugOverlaySnapshotStore` 작업을 기능 추가 중심이 아니라, 계약 안정화와 책임 분리 중심으로 진행한다.
- 소비자(HUD/ViewDataBuilder/FocusResolver)와의 데이터 의미 충돌을 방지한다.
- 디버그 오버레이 수집 코드의 장기 유지보수성과 회귀 안정성을 확보한다.

## 2) 범위
- 포함
  - SnapshotStore API 계약 고정
  - 내부 책임 분리(기록/저장/필터/조회)
  - 이벤트 필터 신뢰성 개선
  - Pawn별 AI 캐시 수명 정책 정의
  - 소비자 정합성 검증 및 회귀 체크
- 제외
  - Shipping 빌드 동작 변경
  - 디버그 오버레이 외 타 시스템 설계 변경
  - 시각 디자인 변경

## 3) 고정 원칙
1. 외부 계약 우선: public API 의미를 먼저 고정한 뒤 내부 리팩터를 진행한다.
2. 구조화 우선: 필터 판단을 요약 문자열 파싱보다 구조화된 필드 기반으로 이동한다.
3. 단일 책임: Record 계열은 기록 의도 전달에 집중하고, 필터/링버퍼/정규화는 전담 헬퍼가 담당한다.
4. 안전한 확장: 기존 호출부를 깨지 않는 점진적 변경(호환 계층 유지 후 정리)으로 간다.
5. 검증 선행: 단계별 빌드 성공과 체크리스트 통과 없이 다음 단계로 넘어가지 않는다.

## 4) 단계별 실행 계획

### Phase 1. 계약 고정
- 대상
  - `FDebugOverlaySnapshotStore.h`
  - `FDebugOverlaySnapshotTypes.h`
- 작업
  - `TryGet*` 반환 조건과 실패 조건 명문화
  - `CaptureState` 전이 규칙 명확화
  - EventLog Filter canonical 규칙 고정
- 완료 기준
  - 호출자 관점에서 API 기대 동작이 명확하고 모호성이 없다.

### Phase 2. 내부 책임 분리
- 대상
  - `FDebugOverlaySnapshotStore.cpp`
- 작업
  - EventRing 관리, Event 필터, Record 빌더를 내부 유틸 단위로 분리
  - Record 함수 본문에서 중복 로직 제거
- 완료 기준
  - 주요 Record 함수가 읽기 쉬운 짧은 흐름(입력 검증 -> 요약 구성 -> 스냅샷 반영 -> 이벤트 추가)으로 정리된다.

### Phase 3. 필터 신뢰성 개선
- 대상
  - `FDebugOverlaySnapshotTypes.h`
  - `FDebugOverlaySnapshotStore.cpp`
- 작업
  - 필요 시 `FDebugOverlayEventEntry`에 필터 보조 메타 필드 추가
  - `ExtractSummaryFieldValue` 의존 축소
- 완료 기준
  - 노이즈/충돌 이벤트 숨김 판단이 문자열 포맷 변화에 덜 민감하다.

### Phase 4. 수명/용량 정책
- 대상
  - `FDebugOverlaySnapshotStore.cpp`
- 작업
  - `LastAIByPawnName` 정리 전략 도입(최대 엔트리 또는 stale 기반)
  - 월드 정리 시점 정책 재확인
- 완료 기준
  - 장시간 세션에서도 데이터 증가 상한이 예측 가능하다.

### Phase 5. 소비자 정합성 검증
- 대상
  - `FDebugOverlayViewDataBuilder.cpp`
  - `FDebugOverlayFocusResolver.cpp`
- 작업
  - stale 기준, fallback 기준 일치 여부 점검
  - Snapshot 부재/부분 부재 시 표시 정책 일관화
- 완료 기준
  - 동일 상황에서 Focus 선택과 HUD 표시에 의미 충돌이 없다.

### Phase 6. 회귀 검증
- 검증 시나리오
  1. Execution Reject/Ignore 노이즈 필터
  2. Combat TargetAccepted/TargetRejected 표시
  3. AI Recent Event stale 전이
  4. Subject Filter 역할 표기(Outgoing/Incoming/Self)
  5. Reset/ResetAll 이후 조회 API 동작
- 완료 기준
  - `PortfolioEditor Win64 Development` 빌드 성공
  - 위 5개 수동 체크리스트 통과

## 5) 커밋 운영 가이드
- 원칙
  - Phase 단위 또는 영향 축 단위로 커밋 분리
  - 계약 변경과 내부 리팩터를 가능한 분리
- 권장 순서
  1. 계약/타입
  2. 내부 분리
  3. 필터 신뢰성
  4. 수명 정책
  5. 소비자 정합성
- 권장 커밋 메시지 prefix
  - `refactor(debug-overlay): ...`
  - `feat(debug-overlay): ...`
  - `chore(debug-overlay): ...`

## 6) 리스크와 대응
- 리스크: 문자열 요약 포맷 변경 시 필터 오작동
  - 대응: 구조화 필드 기반 필터 우선 적용
- 리스크: 호출부 기대값과 SnapshotStore 반환 규칙 불일치
  - 대응: 계약 고정 문서 우선, 호출부 동시 점검
- 리스크: 캐시 증가로 장시간 플레이 메모리 사용 증가
  - 대응: 엔트리 상한 또는 stale 정리 정책 즉시 도입

## 7) Done 정의
- 계획된 Phase 반영 완료
- 빌드 성공
- 회귀 체크리스트 통과
- 호출부와 문서가 동일한 용어/의미를 사용

## 8) Objective-Mode 진행 메모 (2026-08-06)
- 물리 분리 적용 완료: `FDebugOverlaySnapshotStore.cpp` 내부 helper를 책임별 파일로 분리함.
- 분리 파일:
  - `FDebugOverlaySnapshotStoreStoreLifecycle.cpp`
  - `FDebugOverlaySnapshotStoreEventFilterPolicy.cpp`
  - `FDebugOverlaySnapshotStoreEventRingAccess.cpp`
  - `FDebugOverlaySnapshotStoreRecordBuilders.cpp`
  - `FDebugOverlaySnapshotStoreInternals.h`
- `FDebugOverlaySnapshotStore.cpp`는 orchestration 책임(Gate/Record/Query/Lifecycle API) 중심으로 정리함.
- 검증: `PortfolioEditor Win64 Development` 빌드 성공.
