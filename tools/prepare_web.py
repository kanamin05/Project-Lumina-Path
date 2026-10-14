#!/usr/bin/env python3
"""Convert the upstream Project-Lumina-Path main.c to a dual desktop/web game loop.
Run once on the web-version branch from the repository root.
"""
from pathlib import Path
import re

path = Path('main.c')
if not path.exists():
    raise SystemExit('Run this script from the repository root (main.c not found).')
original = path.read_text(encoding='utf-8')
if 'emscripten_set_main_loop(GameFrame' in original:
    raise SystemExit('main.c already appears to have been converted. No changes made.')
if 'int main() {' not in original:
    raise SystemExit('Cannot find original main function. No changes made.')

prefix, main_content = original.split('int main() {', 1)
loop_marker = '    //メインループ\n    while (!WindowShouldClose()) {'
if loop_marker not in main_content:
    raise SystemExit('Original main loop not found. Check the repository revision.')
init, remaining = main_content.split(loop_marker, 1)
match = re.search(r'\n    }\s*\n\s*//【メモリ解放】', remaining)
if not match:
    raise SystemExit('Original loop ending / cleanup marker not found.')
frame = remaining[:match.start()]
cleanup = remaining[match.end():]
if not re.search(r'return 0;\s*}\s*$', cleanup):
    raise SystemExit('Original cleanup / return block not found.')
cleanup = re.sub(r'\s*return 0;\s*}\s*$', '\n', cleanup)

# State must survive between browser animation-frame callbacks.
globals_code = '''
static Texture2D bgTex;
static GameState currentState;
static int dragMode;
static int dragIdx;
static bool dragStartNode;
static float globalTime;
static bool missionCompletePlayed;
#ifdef __EMSCRIPTEN__
static bool musicStarted = false;
#endif
'''

substitutions = [
    ('    Texture2D bgTex;', ''),
    ('    GameState currentState = STATE_TITLE;', '    currentState = STATE_TITLE;'),
    ('    int dragMode = 0;', '    dragMode = 0;'),
    ('    int dragIdx = -1;', '    dragIdx = -1;'),
    ('    bool dragStartNode = true;', '    dragStartNode = true;'),
    ('    float globalTime = 0;', '    globalTime = 0;'),
    ('    bool missionCompletePlayed = false;', '    missionCompletePlayed = false;'),
]
for old, new in substitutions:
    if init.count(old) != 1:
        raise SystemExit(f'Expected exactly one initialization: {old.strip()}')
    init = init.replace(old, new, 1)

if init.count('    PlayMusicStream(bgm);') != 1:
    raise SystemExit('Expected one initial PlayMusicStream call.')
init = init.replace('    PlayMusicStream(bgm);', '''#ifndef __EMSCRIPTEN__
    PlayMusicStream(bgm);
#endif''', 1)

if frame.count('        UpdateMusicStream(bgm);') != 1:
    raise SystemExit('Expected one UpdateMusicStream call inside frame.')
frame = frame.replace('        UpdateMusicStream(bgm); //BGMのバッファ更新', '''#ifdef __EMSCRIPTEN__
        // Browsers require a user gesture before starting audio.
        if (!musicStarted &&
            (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_R))) {
            PlayMusicStream(bgm);
            musicStarted = true;
        }
        if (musicStarted) UpdateMusicStream(bgm);
#else
        UpdateMusicStream(bgm); //BGMのバッファ更新
#endif''', 1)

prefix = prefix.replace('#include "game_logic.h"', '''#include "game_logic.h"
#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif''', 1)
if '#include <emscripten/emscripten.h>' not in prefix:
    raise SystemExit('Could not insert web header.')
# Keep native cleanup, but the web callback is managed by the browser.
converted = (prefix + globals_code + '\nstatic void GameFrame(void) {' + frame + '\n}\n\n'
             + 'int main(void) {' + init + '''
#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(GameFrame, 0, 1);
#else
    while (!WindowShouldClose()) {
        GameFrame();
    }
    //【メモリ解放】''' + cleanup + '\n#endif\n    return 0;\n}\n')
# Verify structure before mutating user content.
assert converted.count('static void GameFrame(void)') == 1
assert converted.count('emscripten_set_main_loop(GameFrame, 0, 1)') == 1
assert converted.count('int main(void)') == 1
backup = path.with_suffix('.c.before-web.bak')
if backup.exists():
    raise SystemExit(f'Backup already exists: {backup}. Resolve before proceeding.')
backup.write_text(original, encoding='utf-8')
path.write_text(converted, encoding='utf-8')
print('Updated main.c for desktop + WebAssembly.')
print(f'Backup saved as {backup} (do not commit it).')
