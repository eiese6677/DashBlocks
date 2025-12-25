# DashBlocks
## 프로젝트 구조

```
eslint.config.js
index.html
package.json
README.md
vite.config.ts
client/
	main.py
public/
server/
	app_minimal.py
	app.py
	game_logic.cpp
	requirements.txt
	server.cpp
	test.cpp
	test.py
	__pycache__/
	ai/
		gomoku_ai.cpp
src/
	App.css
	App.tsx
	Board.tsx
	index.css
	main.tsx
	Player.jsx
```

## 실행 방법

이 프로젝트는 Docker를 사용하여 쉽게 빌드하고 실행할 수 있습니다. 또는 프런트엔드와 백엔드를 개별적으로 로컬에서 실행할 수도 있습니다.

### Docker를 이용한 실행 (권장)

1.  **Docker 이미지 빌드:**
    프로젝트 루트 디렉토리에서 다음 명령어를 실행하여 Docker 이미지를 빌드합니다. 이 과정은 프런트엔드와 백엔드를 모두 포함합니다.
    ```bash
    docker build -t dashblocks .
    ```

2.  **Docker 컨테이너 실행:**
    빌드가 완료되면 다음 명령어를 실행하여 컨테이너를 백그라운드에서 시작합니다. `5000`번 포트가 호스트 머신으로 노출됩니다.
    ```bash
    docker run -d -p 5000:5000 --name dashblocks-game dashblocks
    ```
    *(`-d` 플래그는 컨테이너를 백그라운드에서 실행하며, `--name`으로 컨테이너에 `dashblocks-game`이라는 이름을 부여합니다.)*

3.  **애플리케이션 접속:**
    컨테이너가 실행되면 웹 브라우저를 열고 다음 주소로 이동하여 애플리케이션에 접속할 수 있습니다:
    [http://localhost:5000](http://localhost:5000)

4.  **컨테이너 로그 확인 (선택 사항):**
    실행 중인 컨테이너의 로그를 확인하려면 다음 명령어를 사용합니다:
    ```bash
    docker logs dashblocks-game
    ```

5.  **컨테이너 중지:**
    애플리케이션 사용을 마쳤으면 다음 명령어로 컨테이너를 중지할 수 있습니다:
    ```bash
    docker stop dashblocks-game
    ```

6.  **컨테이너 삭제 (선택 사항):**
    컨테이너를 완전히 삭제하려면 중지 후 다음 명령어를 사용합니다:
    ```bash
    docker rm dashblocks-game
    ```

### 로컬에서 개별 실행 (대안)

로컬 개발 환경에서 프런트엔드와 백엔드를 따로 실행할 수도 있습니다.

1.  **백엔드 실행 (Python):**

    ```bash
    # (선택 사항) 가상 환경 생성 및 활성화
    python -m venv .venv
    # Windows: .\.venv\Scripts\Activate.ps1
    # macOS/Linux: source ./.venv/bin/activate

    # 의존성 설치
    pip install -r server/requirements.txt

    # 서버 실행
    python server/app.py
    ```

2.  **프런트엔드 개발 서버 실행 (React with Vite):**

    ```bash
    # 프런트엔드 의존성 설치
    npm install

    # 개발 서버 실행
    npm run dev
    ```
    프런트엔드는 보통 `http://localhost:5173`과 같은 주소에서 실행됩니다.

## 각 파일 설명

- `eslint.config.js`: ESLint 설정 파일 (코드 스타일/검사 설정).
- `index.html`: 웹앱의 루트 HTML 파일.
- `package.json`: 프로젝트 메타데이터, 스크립트, 프론트엔드 의존성 목록.
- `README.md`: 프로젝트 설명서(이 파일).
- `vite.config.ts`: Vite 개발/번들 설정 (TypeScript).
- `tsconfig.json`: TypeScript 컴파일러 설정 (클라이언트).
- `tsconfig.node.json`: TypeScript 컴파일러 설정 (Node.js 환경 파일).

- `client/`: 클라이언트 관련 코드/스크립트 폴더.
  - `main.py`: 클라이언트 관련 Python 스크립트(프로젝트에 따라 역할이 다를 수 있음).

- `public/`: 정적 자원(이미지, 폰트 등)을 두는 폴더.

- `server/`: 서버 쪽 소스와 테스트, 빌드 관련 파일들.
  - `app_minimal.py`: 간단하거나 예제용 서버 애플리케이션.
  - `app.py`: 메인 서버 애플리케이션(주요 엔드포인트 포함).
  - `game_logic.cpp`: 게임 로직을 구현한 C++ 소스파일 (현재 Rust 마이그레이션 예정).
  - `requirements.txt`: Python 의존성 목록(서버 실행에 필요한 패키지).
  - `server.cpp`: C++로 작성된 서버 또는 관련 네이티브 코드.
  - `test.cpp`: C++ 테스트/샘플 코드.
  - `test.py`: Python 단위/통합 테스트 스크립트.
  - `__pycache__/`: Python 바이트코드 캐시 디렉터리(자동 생성).
  - `ai/gomoku_ai.cpp`: 오목(Gomoku) 인공지능 로직(C++).

- `src/`: 프론트엔드 React 소스 코드 (TypeScript).
  - `App.css`: 애플리케이션 전역 스타일.
  - `App.tsx`: 메인 React 컴포넌트 (TypeScript).
  - `Board.tsx`: 보드 표시용 컴포넌트 (TypeScript).
  - `index.css`: 기본 스타일 시트.
  - `main.tsx`: 프론트엔드 진입점(React 렌더링 시작 파일) (TypeScript).
  - `Player.jsx`: 플레이어 관련 컴포넌트/로직 (JavaScript/JSX).