## Gitmoji 사용법

### 1) 기본 포맷

* **형식**:
  `:gitmoji: 한 줄 요약(영문 명령형 / or 한글 요약형)`

* 예시:
  * `🎉 initial commit`
  * `✨ add character movement input`
  * `🐛 fix crash when loading SPH assets`

---
### 2) 권장 gitmoji 세트 (짧은 버전)

#### 프로젝트 시작 / 구조
* `🎉 :tada:` – initial commit / 저장소 첫 커밋
* `🌱 :seedling:` – 프로젝트/시스템 뼈대 추가 (초기 구조, 골격 코드)

#### 기능 / 코드 변경
* `✨ :sparkles:` – 새 기능 추가 (게임플레이, 엔진 기능, 에디터 기능 등)
* `🐛 :bug:` – 버그 수정
* `♻️ :recycle:` – 리팩터링 (동작은 유지, 구조/가독성 개선)
* `⚡️ :zap:` – 성능 개선 / 최적화

#### 문서 / 테스트 / 설정
* `📝 :memo:` – 문서, 주석, 노트, README 수정
* `✅ :white_check_mark:` – 테스트 추가/수정 (단위 테스트, 테스트 맵, 디버그 씬)
* `🔧 :wrench:` – 설정 파일, 빌드 스크립트, 툴 세팅 변경 (.gitignore, .editorconfig, vcpkg.json 등)

#### 의존성 / 자원 / 정리
* `➕ :heavy_plus_sign:` – 라이브러리/패키지/플러그인 추가
* `⬆️ :arrow_up:` – 라이브러리/엔진 버전 업
* `🚚 :truck:` – 파일/폴더/네이밍 이동/리네임
* `🔥 :fire:` – 코드/파일/리소스 삭제 (더 이상 사용하지 않는 것)

---
## 2. 추가설명

### Gitmoji 커밋 가이드
#### 0. 기본 규칙
```bash
  <gitmoji> <영문 명령형 요약> 
  # 또는
  <gitmoji> <한글 요약형>
```
- 커밋 메시지 1줄 포맷
  
* 가능하면 **영문 명령형** 사용
	* `add`, `fix`, `refactor`, `update`, `remove`, `optimize` …
	* 예) `✨ add basic movement component`

* 한글 쓸 때도 “~추가”, “~수정” 정도의 짧은 요약:
	* 예) `🐛 입력 값 누락으로 인한 크래시 수정`

* 상세 설명이 필요하면 **본문**에 bullet로 적기
	- ✨ add basic movement input system
		- bind move/jump inputs with Enhanced Input
		- add UCPlayerInputManager skeleton
		- connect to character via interface

---
#### 1. 프로젝트 라이프사이클 / 구조

##### 🎉 :tada: – Initial commit / 프로젝트 생성

* 용도
  * 새 저장소 첫 커밋
  * Unreal 프로젝트 템플릿 생성 직후
  * DirectX/엔진 프레임워크 골격 처음 올릴 때

* 예시
```text
🎉 initial Unreal project setup
🎉 init DirectX11 graphics framework
```

##### 🌱 :seedling: – 초기 뼈대 / 베이스 구조 추가

* 용도
  * 큰 시스템/레이어의 “첫 골격” 추가
  * EngineCore, 기본 Renderer, InputSystem, GameFramework 뼈대 등

* 예시
```text
🌱 add core engine skeleton (Window, Renderer, Input)
🌱 add UE5 gameplay framework base (GameMode, PlayerController, Character)
```

---
#### 2. 기능 / 코드 변경
##### ✨ :sparkles: – 새 기능 추가

* 용도
  * 눈에 띄는 “새 기능”이 들어갔을 때
  * UE: 새 캐릭터 시스템, 새 액션, 새 UI, 새 AI Behavior 등
  * DX: 새 렌더 패스, 새 쉐이더, 새 디버그 기능 등

* 예시
```text
✨ add basic player movement and camera control
✨ implement SPH density and pressure force update
✨ add parry/guard input handling with requestor system
```

##### 🐛 :bug: – 버그 수정

* 용도
  * Crash, 논리 버그, 잘못된 렌더링, 잘못된 상태 전이 등 수정

* 예시
```text
🐛 fix crash when no InputMappingContext is set
🐛 fix wrong NDC to screen-space Y flip in sprite rotation
🐛 fix SPH kernel radius mismatch causing unstable simulation
```

##### ♻️ :recycle: – 리팩터링 (동작 유지, 구조 개선)

* 용도
  * 함수/클래스 분리
  * 책임 분리, 네이밍 정리
  * 성능 최적화가 목적이 아니라 **읽기/구조 개선**이 주목적일 때

* 예시
```text
♻️ refactor UCInputActionRequestor evaluation logic
♻️ split Renderer into Device, PassManager, and DebugOverlay
♻️ clean up Character spawn/possess flow with deferred spawning helper
```

##### ⚡️ :zap: – 성능 개선 / 최적화

* 용도
  * 성능이 실제로 개선되도록 수정했을 때
  * GPU 최적화, CPU 메모리 접근 최적화, 알고리즘 교체 등

* 예시
```text
⚡️ optimize 3D fluid advection compute shader
⚡️ reduce SPH neighbor search cost using spatial hash
⚡️ cache Unreal DDC results to speed up PIE startup
```

---
#### 3. 문서 / 테스트 / 설정
##### 📝 :memo: – 문서, 주석, 노트

* 용도
  * README, 문서, 강의 노트, 디자인 문서
  * 대규모 주석 정리/추가

* 예시
```text
📝 document spawn/possess flow and safe deferred spawning rules
📝 update README with UE5.4 project structure and build steps
📝 add comments for SPH kernel math and parameters
```

##### ✅ :white_check_mark: – 테스트 / 검증 관련

* 용도
  * 단위 테스트, 통합 테스트
  * 디버그/테스트 전용 맵, 시뮬레이션 테스트용 케이스 추가

* 예시
```text
✅ add test map for input requestor and trigger system
✅ add unit tests for SPH density and gradient kernels
✅ add benchmark scene for directional shadow PCF/PCSS
```

##### 🔧 :wrench: – 설정 / 빌드 / 툴

* 용도
  * `.gitignore`, `.editorconfig`, `.vsconfig`
  * Unreal Build.cs 세팅, 프로젝트 세팅
  * vcpkg, CMake, 빌드 스크립트 등

* 예시
```text
🔧 update .gitignore for Unreal and VS artifacts
🔧 configure vcpkg dependencies (Assimp, DirectXTK, Eigen)
🔧 tweak Unreal DefaultEngine.ini for DX12 and shader dev options
```

---
#### 4. 의존성 / 리소스 / 정리

##### ➕ :heavy_plus_sign: – 의존성 / 패키지 / 플러그인 추가

* 용도
  * vcpkg, NuGet, 서브모듈, UE 플러그인 추가
  * 새 외부 라이브러리 도입

* 예시
```text
➕ add Assimp via vcpkg for model loading
➕ add custom Unreal plugin for input debugging
➕ add glm library for math utilities in DX framework
```

##### ⬆️ :arrow_up: – 의존성 업데이트

* 용도
  * 라이브러리 버전 업, UE 버전업, 엔진/툴 체인 업데이트

* 예시
```text
⬆️ upgrade Unreal Engine to 5.4
⬆️ bump Assimp from 5.2.x to 5.3.x
⬆️ update HLSL shader model to 6.6
```

### 🚚 :truck: – 파일 / 폴더 / 네이밍 이동/개편

* 용도
  * 폴더 구조 정리
  * 파일 위치 이동, 리네이밍
  * 네임스페이스/모듈 구조 변경

* 예시
```text
🚚 move SPH simulation into Engine/Fluid directory
🚚 reorganize Unreal project folders (Core, Game, Test)
🚚 rename InputManager classes to follow UC/AC naming convention
```

### 🔥 :fire: – 삭제 / 제거

* 용도
  * 더 이상 사용하지 않는 코드, 테스트, 에셋 삭제
  * 옛 프토로타입 정리

* 예시
```text
🔥 remove old DirectX9 prototype code
🔥 delete unused test maps and placeholder assets
🔥 drop deprecated input evaluation path
```

---

#### 5. 실전 템플릿 모음

##### 5.1 Unreal 프로젝트 예시

```text
🎉 initial Unreal C++ project setup
🔧 update .gitignore and .vsconfig for UE5 + VS2022
✨ add basic player character and controller
✨ implement input requestor and trigger manager flow
🐛 fix crash when possessing character before input manager init
♻️ refactor deferred spawn + possess sequence into helper functions
⚡️ optimize tick order and disable unused components in editor
📝 document input evaluation and dispatch design (UCPlayerInputManager)
```

##### 5.2 DirectX/그래픽스 프레임워크 예시

```text
🎉 init DX11 graphics framework project
🌱 add EngineCore with Window, Device, and basic Renderer
✨ implement PBR shading with IBL and BRDF LUT
✨ add SPH fluid simulation with compute shaders
🐛 fix incorrect normal reconstruction in G-Buffer pass
⚡️ optimize 3D texture sampling in curl noise
♻️ refactor render passes into RenderGraph-like structure
📝 add notes on Jacobi iteration for diffusion and pressure solve
```

---
