# Lumina Path — Web Edition

This package adds a browser build **without replacing the original desktop Makefile**.

## Files to add

Copy `Makefile.web`, `web/shell.html`, `.github/workflows/deploy-web.yml`, and `tools/prepare_web.py` into the root of your existing Project-Lumina-Path repository. Keep your original C sources, images, and audio.

## Change `main.c` (once)

Run from the repository root:

```bash
python3 tools/prepare_web.py
```

On Windows you may need `py tools/prepare_web.py` rather than `python3`.
This creates a local `main.c.before-web.bak` backup. Do not commit the backup. Review `git diff -- main.c` before committing. To undo: `git restore main.c` (if changes have not been committed).

## Build locally (macOS / Linux / WSL recommended)

Install Emscripten SDK, CMake, Git, and Make. Activate the Emscripten environment so `emcc` and `emcmake` are available. Then:

```bash
make -f Makefile.web
python3 -m http.server 8000 -d docs
```

Open http://localhost:8000/ (do not open `docs/index.html` directly via `file://`). The web Makefile downloads raylib 5.5 into ignored `vendor/raylib` and builds a Web-targeted static library in ignored `build/raylib`.

On macOS with Homebrew, `brew install cmake` installs CMake. Emscripten is a separate toolchain; the native Homebrew raylib is not suitable for the Web target.

## GitHub Actions + Pages

1. Commit and push your web feature branch. The workflow will build the WebAssembly game and upload the Pages artifact, but it **only deploys on `main`**.
2. Inspect the **Actions** tab and confirm that `Build and deploy Lumina Path (Web)` succeeded.
3. Merge into `main` after the build is working.
4. In repository **Settings → Pages**, select **GitHub Actions** as the deployment source (if not already selected).
5. The site should appear at https://kanamin05.github.io/Project-Lumina-Path/ after the `main` deployment succeeds.

## Expected generated output (not committed)

```
docs/index.html
docs/index.js
docs/index.wasm
docs/index.data
```

`docs/`, `vendor/`, and `build/` are generated artifacts and should be ignored by Git. GitHub Actions publishes the `docs/` artifact directly.

## Add to `.gitignore`

```
/docs/
/vendor/
/build/
*.before-web.bak
```

## Known caveats

- Browser audio is usually locked until a user gesture. The conversion delays background music until a mouse click or Enter/R key press. Sound behavior should still be tested on Chrome/Safari/Firefox.
- The browser canvas uses the original game's fixed resolution and scales down via CSS for smaller screens; touch interaction might require later changes.
- This kit was prepared by inspection of the public repository, not fully compiled/tested against a live Emscripten installation. If CI reports compiler errors or browser issues, use the exact error to refine the port.
- Keep the original `Makefile` intact for Windows, macOS and Linux.
