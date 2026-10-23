# Project Stellar 공개 자료 운영

제출용 PPT/PDF, 영상 14개, 별도 가드 PIE 1개, 공개 문서 9개와 필수 첨부 자료를 함께 게시합니다.

- 자료 안내: https://razria93.github.io/Portfolio_UE5.4_verGit/
- 영상·문서 대응표: https://razria93.github.io/Portfolio_UE5.4_verGit/catalog.html
- 제출 버전: stellar-portfolio-20261023-v41

## 수정과 배포

1. HTML·CSS·근거 JSON/CSV는 `portfolio-web/site/`에서 수정합니다.
2. 영상과 PDF/PPT 등 바이너리는 GitHub Release에 새 버전으로 보존합니다. 영상 원본은 Git에 커밋하지 않습니다.
3. `release-manifest.json`의 태그·파일명·크기·SHA-256을 실제 파일과 대조해 갱신합니다. 자료를 바꿀 때 공개 `site/manifest.json`과 대응표도 함께 갱신합니다.
4. main의 게시 파일 변경은 기존 Pages workflow를 실행합니다. 지정한 Release 파일만 받아 크기와 해시를 검사한 후 게시합니다.
5. 게시 후 PDF의 링크, 문서·앵커, 영상 재생·탐색, 비로그인 접근을 확인합니다. 실패한 배포는 기존 공개본을 교체하지 않습니다.

## 보존 범위

영상은 원본 제작 MP4를 유지합니다. 제출된 버전의 바이너리를 덮어쓰지 않고 다음 버전을 발행합니다. 기존 root HTML은 `legacy-portfolio.html`로 보존하고 기존 Assets 경로를 유지합니다. 프로젝트 코드·외부 에셋 원본·로컬 조사 폴더는 이 게시 작업에 추가하지 않습니다.

## 검증 범위

영상과 문서의 검증 기록 시점은 각각 명시된 조건을 따릅니다. 게시 자료는 최신 main 전체의 재검증 결과를 뜻하지 않습니다. Pages 최종 패키지는 1GB 미만이어야 합니다.
