#include <stddef.h>
#include "raylib.h"
#include "common.h"
#include "game_logic.h"
#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif


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

static void GameFrame(void) {
        globalTime += GetFrameTime(); //経過時間の更新
        Vector2 mPos = GetMousePosition(); //マウス座標の取得
#ifdef __EMSCRIPTEN__
        // Browsers require a user gesture before starting audio.
        if (!musicStarted &&
            (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_R))) {
            PlayMusicStream(bgm);
            musicStarted = true;
        }
        if (musicStarted) UpdateMusicStream(bgm);
#else
        UpdateMusicStream(bgm); //BGMのバッファ更新
#endif


        //【状態遷移】
        if (currentState == STATE_TITLE) { //タイトル
            if (IsKeyPressed(KEY_ENTER)) currentState = STATE_TUTORIAL_1; 
        } 
        else if (currentState == STATE_TUTORIAL_1) { //チュートリアル1
            if (IsKeyPressed(KEY_ENTER)) currentState = STATE_TUTORIAL_2; 
        }
        else if (currentState == STATE_TUTORIAL_2) { //チュートリアル2
            if (IsKeyPressed(KEY_ENTER)) currentState = STATE_GAME; 
        }
        else if (currentState == STATE_GAME) { //ゲーム
            //Rキーでステージリセット
            if (IsKeyPressed(KEY_R)) { 
                ResetInventory(); 
                isGameOver = false; 
                contaminationLevel = 0; 
            }
            //数字キーでステージ移動
            if (IsKeyPressed(KEY_ONE) || IsKeyPressed(KEY_TWO) || IsKeyPressed(KEY_THREE) || 
                IsKeyPressed(KEY_FOUR) || IsKeyPressed(KEY_FIVE) || IsKeyPressed(KEY_SIX) || 
                IsKeyPressed(KEY_SEVEN) || IsKeyPressed(KEY_EIGHT) || IsKeyPressed(KEY_NINE) || IsKeyPressed(KEY_ZERO)) {
                if (IsKeyPressed(KEY_ONE)) currentIdx = 0;
                else if (IsKeyPressed(KEY_TWO)) currentIdx = 1;
                else if (IsKeyPressed(KEY_THREE)) currentIdx = 2;
                else if (IsKeyPressed(KEY_FOUR)) currentIdx = 3;
                else if (IsKeyPressed(KEY_FIVE)) currentIdx = 4;
                else if (IsKeyPressed(KEY_SIX)) currentIdx = 5;
                else if (IsKeyPressed(KEY_SEVEN)) currentIdx = 6;
                else if (IsKeyPressed(KEY_EIGHT)) currentIdx = 7;
                else if (IsKeyPressed(KEY_NINE)) currentIdx = 8;
                else if (IsKeyPressed(KEY_ZERO)) currentIdx = 9;
                ResetInventory();
                missionCompletePlayed = false;
                isGameOver = false;
                contaminationLevel = 0;
            }

            //動く壁
            for (int i = 0; i < currentStage.wallCount; i++) {
                if (currentStage.walls[i].moveRange != 0) {
                    float offset = sinf(globalTime * currentStage.walls[i].speed) * currentStage.walls[i].moveRange;
                    currentStage.walls[i].start.y = currentStage.walls[i].originStart.y + offset;
                    currentStage.walls[i].end.y = currentStage.walls[i].originEnd.y + offset;
                }
            }

            //【マウスのドラッグ操作】
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                Vector2 lPos = Vec2Add(currentStage.emitter.pos, (Vector2){ cosf(currentStage.emitter.angle)*60, sinf(currentStage.emitter.angle)*60 });
                if (Vec2Distance(mPos, lPos) < 20) { 
                    dragMode = 3; PlaySound(seRotate); //発射台回転
                } 
                else {
                    //鏡のドラッグ判定
                    for (int i=0; i<currentStage.mirrorCount; i++) {
                        if (Vec2Distance(mPos, currentStage.mirrors[i].start) < 20) { 
                            dragMode = 1; dragIdx = i; dragStartNode = true; 
                            break; 
                        }
                        if (Vec2Distance(mPos, currentStage.mirrors[i].end) < 20) { 
                            dragMode = 1; dragIdx = i; dragStartNode = false; 
                            break; 
                        }

                    }
                    //プリズムのドラッグ判定
                    if (dragMode == 0) for (int i=0; i<currentStage.prismCount; i++) {
                        if (Vec2Distance(mPos, currentStage.prisms[i].start) < 20) { 
                            dragMode = 2; 
                            dragIdx = i; 
                            dragStartNode = true; 
                            break; 
                        }
                        if (Vec2Distance(mPos, currentStage.prisms[i].end) < 20) { 
                            dragMode = 2; 
                            dragIdx = i; 
                            dragStartNode = false; 
                            break; 
                        }
                    }
                }
            }

            //ドラッグ中の座標の更新
            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) dragMode = 0;
            if (dragMode == 3) { //発射台の向きをマウスの方向へ
                currentStage.emitter.angle = atan2f(mPos.y - currentStage.emitter.pos.y, mPos.x - currentStage.emitter.pos.x);
            }
            else if (dragMode == 1) { //鏡の操作
                Vector2 d = Vec2Subtract(currentStage.mirrors[dragIdx].end, currentStage.mirrors[dragIdx].start); //今の長さを保存
                if(dragStartNode) { //始点をつかんだ場合
                    currentStage.mirrors[dragIdx].start = mPos; //始点をマウスの位置にする
                    currentStage.mirrors[dragIdx].end = Vec2Add(mPos, d); //保存した長さの分だけ終点もずらす
                } else currentStage.mirrors[dragIdx].end = mPos; //終点をつかんだ場合→終点だけをマウスの位置にする
                currentStage.mirrors[dragIdx].inInventory = (mPos.y > GAME_H); //インベントリ内にあるかないか判定
            } else if (dragMode == 2) { //プリズムの操作（鏡と同様）
                Vector2 d = Vec2Subtract(currentStage.prisms[dragIdx].end, currentStage.prisms[dragIdx].start);
                if(dragStartNode) { 
                    currentStage.prisms[dragIdx].start = mPos; 
                    currentStage.prisms[dragIdx].end = Vec2Add(mPos, d); 
                } else currentStage.prisms[dragIdx].end = mPos;
                currentStage.prisms[dragIdx].inInventory = (mPos.y > GAME_H);
            }
            //毎フレームでヒットフラグをリセット
            for(int i=0; i<currentStage.targetCount; i++) currentStage.targets[i].tempColorFlags = 0;
        }


        //【描画開始】
        BeginDrawing();
            ClearBackground(BLACK);
            if (currentState != STATE_GAME && bgTex.id > 0) DrawTexture(bgTex, 0, 0, (Color){150, 150, 150, 255});

            //シーンごとの描画
            if (currentState == STATE_TITLE) { //タイトル
                DrawText("LUMINA PATH", WIN_W/2 - 220, WIN_H/2 - 100, 60, SKYBLUE);
                DrawText("Designed by Freepik", 670, 570, 10, DARKGRAY); 
                if (((int)(globalTime * 2) % 2) == 0) DrawText("PRESS ENTER TO START", WIN_W/2 - 140, WIN_H/2 + 150, 20, RAYWHITE);
            } 
            else if (currentState == STATE_TUTORIAL_1 || currentState == STATE_TUTORIAL_2) {
                DrawRectangle(WIN_W/2 - 300, WIN_H/2 - 200, 600, 400, (Color){20, 20, 35, 230});
                DrawRectangleLines(WIN_W/2 - 300, WIN_H/2 - 200, 600, 400, SKYBLUE);
                if (currentState == STATE_TUTORIAL_1) { //チュートリアル1
                    DrawText("HOW TO PLAY: BASICS", WIN_W/2 - 270, WIN_H/2 - 170, 30, GOLD);
                    DrawText("* Drag yellow node to ROTATE the emitter.", WIN_W/2 - 250, WIN_H/2 - 100, 20, RAYWHITE);
                    DrawText("* Drag mirrors/prisms from the inventory.", WIN_W/2 - 250, WIN_H/2 - 60, 20, RAYWHITE);
                    DrawText("* Drag one end to MOVE, ", WIN_W/2 - 250, WIN_H/2 - 20, 20, RAYWHITE);
                    DrawText("       the other end to ROTATE mirrors/prisms.", WIN_W/2 - 250, WIN_H/2 + 20, 20, RAYWHITE);
                    DrawText("* Hit targets until they fill up.", WIN_W/2 - 250, WIN_H/2 +60, 20, RAYWHITE);
                    DrawText("PRESS ENTER FOR NEXT", WIN_W/2 - 100, WIN_H/2 + 160, 15, GRAY);

                    //鏡サンプル
                    Vector2 mirrorStart = { WIN_W/2 - 200, WIN_H/2 + 140 };
                    Vector2 mirrorEnd   = { WIN_W/2 - 120, WIN_H/2 + 110 };
                    DrawLineEx(mirrorStart, mirrorEnd, 4, SKYBLUE);
                    DrawCircleV(mirrorStart, 6, WHITE);
                    DrawCircleV(mirrorEnd,   6, WHITE);
                    DrawText("MIRROR", mirrorStart.x - 5, mirrorStart.y + 18, 14, SKYBLUE);
                    DrawText("MOVE",   mirrorStart.x - 22, mirrorStart.y - 20, 10, LIGHTGRAY);
                    DrawText("ROT",    mirrorEnd.x   - 10, mirrorEnd.y   - 20, 10, LIGHTGRAY);

                    //プリズムサンプル
                    Vector2 prismStart = { WIN_W/2 + 120,  WIN_H/2 + 140 };
                    Vector2 prismEnd   = { WIN_W/2 + 200, WIN_H/2 + 110 };
                    DrawLineEx(prismStart, prismEnd, 6, VIOLET);
                    DrawCircleV(prismStart, 6, MAGENTA); 
                    DrawCircleV(prismEnd,   6, MAGENTA);   
                    DrawText("PRISM", prismStart.x - 5, prismStart.y + 18, 14, VIOLET);
                    DrawText("MOVE",  prismStart.x - 22, prismStart.y - 20, 10, LIGHTGRAY);
                    DrawText("ROT",   prismEnd.x   - 10, prismEnd.y   - 20, 10, LIGHTGRAY);

                } else { //チュートリアル2
                    DrawText("HOW TO PLAY: OPTICS", WIN_W/2 - 270, WIN_H/2 - 170, 30, GOLD);
                    DrawText("* MIRRORS only change direction, not color.", WIN_W/2 - 250, WIN_H/2 - 100, 20, RAYWHITE);
                    DrawText("* PRISM splits WHITE beam into RGB.", WIN_W/2 - 250, WIN_H/2 - 60, 20, RAYWHITE);
                    DrawText("* Combined beams create secondary colors.", WIN_W/2 - 250, WIN_H/2 - 20, 20, RAYWHITE);
                    DrawText("* Wrong colors cause CONTAMINATION.", WIN_W/2 - 250, WIN_H/2 + 20, 20, RAYWHITE);
                    DrawText("* Too much contamination -> STAGE OVER.", WIN_W/2 - 250, WIN_H/2 + 60, 20, RAYWHITE);
                    DrawText("* Beware of space and deep-sea anomalies!", WIN_W/2 - 250, WIN_H/2 + 100, 20, RAYWHITE);
                    DrawText("PRESS ENTER TO START", WIN_W/2 - 110, WIN_H/2 + 160, 15, GRAY);
                }
            }
            else if (currentState == STATE_GAME) { //ゲーム
                for(int i=0; i<60; i++) {
                    DrawPixel((i * 137) % WIN_W, (i * 243) % GAME_H, (Color){200, 200, 255, (unsigned char)(100 * (sinf(globalTime + i) + 1.0f) * 0.5f)});
                }

                if (currentStage.isHybridMode) { //ハイブリッドのとき
                    DrawRectangleGradientV(0, 0, WIN_W, 300, (Color){0, 5, 20, 255}, (Color){0, 15, 50, 255});
                    DrawRectangleGradientV(0, 300, WIN_W, 200, (Color){0, 15, 50, 255}, (Color){0, 5, 10, 255});
                    DrawLine(0, 300, WIN_W, 300, (Color){255, 255, 255, 40});
                } else if (currentStage.isDeepSeaMode) ClearBackground((Color){0, 10, 25, 255}); //深海モードのとき

                //深海モードの背景
                if (currentStage.isDeepSeaMode) {
                    //グラデーション
                    DrawRectangleGradientV(0, 0, WIN_W, GAME_H, (Color){0, 5, 20, 255}, (Color){0, 25, 60, 255});
                    //揺れる光
                    for (int i = 0; i < 8; i++) {
                        float y = fmodf(globalTime * 30 + i * 80, GAME_H);
                        DrawRectangleGradientH(0, (int)y, WIN_W, 60, (Color){20, 80, 120, 30}, BLANK);
                    }
                    //微粒子
                    for (int i = 0; i < 80; i++) {
                        float x = fmodf(i * 73 + globalTime * 10, WIN_W);
                        float y = fmodf(i * 191 + globalTime * 15, GAME_H);
                        DrawPixel((int)x, (int)y, (Color){180, 220, 255, 80});
                    }
                }

                //ハイブリッドor重力モードの背景
                if (currentStage.isHybridMode || currentStage.isGravityMode) {
                    //暗背景
                    DrawRectangleGradientV(0, 0, WIN_W, GAME_H, (Color){5, 0, 20, 255}, (Color){0, 0, 5, 255});
                    //星
                    for (int i = 0; i < 120; i++) {
                        int x = (i * 97) % WIN_W;
                        int y = (i * 233) % GAME_H;
                        unsigned char a = (unsigned char)(150 + 100 * sinf(globalTime + i));
                        DrawPixel(x, y, (Color){255, 255, 255, a});
                    }
                    // 星雲
                    DrawCircleGradient(WIN_W / 3, GAME_H / 3, 200, (Color){120, 60, 200, 40}, BLANK);
                    DrawCircleGradient(WIN_W * 2 / 3, GAME_H / 2, 250, (Color){40, 120, 200, 30}, BLANK);
                }

                //インベントリの境界線と背景
                DrawRectangle(0, GAME_H, WIN_W, INV_H, (Color){15, 15, 22, 255});
                DrawLine(0, GAME_H, WIN_W, GAME_H, DARKGRAY);

                // ブラックホールの描画
                if (currentStage.isGravityMode || currentStage.isHybridMode) {
                    Vector2 bhPos = (currentStage.isHybridMode) ? (Vector2){400, 150} : (Vector2){400, 300};
                    float pulse = sinf(globalTime * 2.0f) * 0.3f + 0.7f;
                    DrawCircleGradient((int)bhPos.x, (int)bhPos.y, 80 * pulse, (Color){120, 60, 200, 60}, BLANK);
                    DrawCircleGradient((int)bhPos.x, (int)bhPos.y, 50 * pulse, (Color){80, 40, 160, 100}, BLANK);
                    DrawCircleGradient((int)bhPos.x, (int)bhPos.y, 25 * pulse, (Color){40, 20, 100, 150}, BLANK);
                    DrawCircle((int)bhPos.x, (int)bhPos.y, 12, (Color){10, 5, 30, 255});
                    
                    for (int i = 0; i < 8; i++) {
                        float angle = globalTime * 0.5f + i * PI * 2.0f / 8.0f;
                        float radius = 60 + sinf(globalTime * 3.0f + i) * 10;
                        Vector2 p = {bhPos.x + cosf(angle) * radius, bhPos.y + sinf(angle) * radius};
                        DrawCircle((int)p.x, (int)p.y, 2, (Color){150, 100, 255, 180});
                    }
                }

                //【ヒット判定、効果音トリガーのロジック】
                //前フレームのヒット状態をリセット
                for (int i = 0; i < currentStage.mirrorCount; i++) {
                    currentStage.mirrors[i].isHitThisFrame = false;
                }
                for (int i = 0; i < currentStage.prismCount; i++) {
                    currentStage.prisms[i].isHitThisFrame = false;
                }

                //レーザー計算（この中でisHitThisFrameがtrueになる）
                if (!isGameOver) {
                    CalculateLaserHybrid(currentStage.emitter.pos, (Vector2){cosf(currentStage.emitter.angle), sinf(currentStage.emitter.angle)}, WHITE_FLAG, 0, 0, globalTime);
                }

                //前フレームでfalse、今フレームでtrueの瞬間だけ鏡の反射音を鳴らす
                for (int i = 0; i < currentStage.mirrorCount; i++) {
                    if (!currentStage.mirrors[i].inInventory) {
                        if (!currentStage.mirrors[i].wasHitPrevFrame && currentStage.mirrors[i].isHitThisFrame) {
                            PlaySound(seMirror);
                        }
                    }
                    if (currentStage.mirrors[i].inInventory) {
                        currentStage.mirrors[i].wasHitPrevFrame = false;
                        currentStage.mirrors[i].isHitThisFrame = false;
                    } else {
                        currentStage.mirrors[i].wasHitPrevFrame = currentStage.mirrors[i].isHitThisFrame;
                    }
                }
                
                //プリズムも同様
                for (int i = 0; i < currentStage.prismCount; i++) {
                    if (!currentStage.prisms[i].inInventory) {
                        if (!currentStage.prisms[i].wasHitPrevFrame && currentStage.prisms[i].isHitThisFrame) {
                            PlaySound(sePrism);
                        }
                    }
                    if (currentStage.prisms[i].inInventory) {
                        currentStage.prisms[i].wasHitPrevFrame = false;
                        currentStage.prisms[i].isHitThisFrame = false;
                    } else {
                        currentStage.prisms[i].wasHitPrevFrame = currentStage.prisms[i].isHitThisFrame;
                    }
                }

                //【オブジェクトの描画】
                //発射台
                Vector2 lPos = Vec2Add(currentStage.emitter.pos, (Vector2){ cosf(currentStage.emitter.angle)*60, sinf(currentStage.emitter.angle)*60 });
                DrawCircleV(currentStage.emitter.pos, 15, DARKGRAY); 
                DrawCircleGradient((int)lPos.x, (int)lPos.y, 12, YELLOW, BLANK);

                //壁、鏡、プリズムの描画
                for (int i=0; i<currentStage.wallCount; i++) {
                    DrawLineEx(currentStage.walls[i].start, currentStage.walls[i].end, 6, (Color){60, 60, 70, 255});
                }
                for (int i=0; i<currentStage.mirrorCount; i++) { 
                    DrawLineEx(currentStage.mirrors[i].start, currentStage.mirrors[i].end, 4, SKYBLUE); 
                    DrawCircleV(currentStage.mirrors[i].start, 5, WHITE); 
                    DrawCircleV(currentStage.mirrors[i].end, 5, WHITE); 
                }
                for (int i=0; i<currentStage.prismCount; i++) { 
                    DrawLineEx(currentStage.prisms[i].start, currentStage.prisms[i].end, 6, VIOLET); 
                    DrawCircleV(currentStage.prisms[i].start, 5, MAGENTA); 
                    DrawCircleV(currentStage.prisms[i].end, 5, MAGENTA); 
                }

                //【ターゲットの判定とクリア条件】
                bool allCleared = true;
                bool isAnyTargetGlow = false;
                bool isAnyContam = false;
                bool contamThisFrame = false;

                for(int i=0; i<currentStage.targetCount; i++) {
                    int hitColor = currentStage.targets[i].tempColorFlags;
                    int required = currentStage.targets[i].requiredColor;

                    //ステージ6の特殊ギミック
                    if (currentIdx == 5 && i == 2 && currentStage.targets[i].isCleared) continue;
                    
                    
                    if (currentIdx == 5 && currentStage.targets[0].isCleared && currentStage.targets[1].isCleared) currentStage.targets[2].isCleared = false;

                    //汚染の判定
                    if ((hitColor & ~required) != 0 && hitColor != 0) {
                        contamThisFrame = true;
                    }
                    //汚染ゲージの増減
                    if (contamThisFrame) {
                        contaminationLevel += GetFrameTime() * 0.8f; //汚染進行
                        isAnyContam = true;
                    } else {
                        contaminationLevel -= GetFrameTime() * 0.4f; //ゲージ回復
                    }
                    contaminationLevel = Clamp(contaminationLevel, 0.0f, CONTAMINATION_LIMIT);
                    if (contaminationLevel >= CONTAMINATION_LIMIT) isGameOver = true;

                    //色一致判定：正しい色が当たっている時間をカウント
                    if (hitColor == required) {
                        isAnyTargetGlow = true;
                        float prevHitTime = currentStage.targets[i].hitTime;
                        currentStage.targets[i].hitTime += GetFrameTime();
                        if (currentStage.targets[i].hitTime > TARGET_HIT_TIME_NEEDED && !currentStage.targets[i].isCleared) {
                            currentStage.targets[i].isCleared = true;
                            PlaySound(seTargetClear);
                        }
                    } else {
                        currentStage.targets[i].hitTime = 0; //外れるとリセット
                    }
                    if (!currentStage.targets[i].isCleared) allCleared = false;

                    //ターゲットの描画
                    Color tc = WHITE;
                    if (required == RED_FLAG) tc = RED;
                    else if (required == GREEN_FLAG) tc = GREEN;
                    else if (required == BLUE_FLAG) tc = BLUE;
                    else if (required == MAGENTA_FLAG) tc = MAGENTA;
                    else if (required == CYAN_FLAG) tc = COLOR_CYAN;
                    else if (required == YELLOW_FLAG) tc = YELLOW;
                    DrawCircleLinesV(currentStage.targets[i].position, 35, (Color){tc.r, tc.g, tc.b, (unsigned char)(currentStage.targets[i].hitTime > 0 ? 255 : 100)});
                    if (currentStage.targets[i].isCleared) {
                        DrawCircleGradient((int)currentStage.targets[i].position.x, (int)currentStage.targets[i].position.y, 30, tc, BLANK);
                    } else {
                        DrawCircleSectorLines(currentStage.targets[i].position, 25, 0, (currentStage.targets[i].hitTime / TARGET_HIT_TIME_NEEDED) * 360, 32, tc);
                    }
                    DrawText(currentStage.targets[i].label, currentStage.targets[i].position.x - 30, currentStage.targets[i].position.y + 45, 12, tc);
                }

                
                if (isAnyTargetGlow && !isGameOver) { //ターゲットに正しい光が当たっている間チャージ音
                    if(!IsSoundPlaying(seTargetGlow)) PlaySound(seTargetGlow); 
                } else StopSound(seTargetGlow);

                if (isAnyContam && !isGameOver) { //汚染の音
                    if(!IsSoundPlaying(seContam)) PlaySound(seContam); 
                } else StopSound(seContam);

                //汚染ゲージの描画
                DrawRectangle(20, 60, 200, 12, DARKGRAY);
                DrawRectangle(20, 60, (int)(200 * (contaminationLevel / CONTAMINATION_LIMIT)), 12, RED);
                DrawText("CONTAMINATION", 20, 45, 14, RED);

                //ゲームオーバーの演出
                if (isGameOver) {
                    DrawRectangle(0, 0, WIN_W, WIN_H, (Color){120, 0, 0, 120});
                    DrawText("WARNING", WIN_W/2 - 140, WIN_H/2 - 80, 60, RED);
                    DrawText("COLOR CONTAMINATION DETECTED", WIN_W/2 - 190, WIN_H/2 - 10, 20, RAYWHITE);
                    DrawText("PRESS [R] TO RESET THIS STAGE", WIN_W/2 - 150, WIN_H/2 + 40, 18, (Color){255,220,220,255});
                }

                //ステージ遷移ロジック
                if (currentState == STATE_GAME && allCleared && currentIdx < STAGE_COUNT - 1) {
                    if (IsKeyPressed(KEY_ENTER)) {
                        currentIdx++;
                        ResetInventory();
                        missionCompletePlayed = false;
                        isGameOver = false;
                        contaminationLevel = 0;
                        allCleared = false; 
                        
                        for(int i=0; i<currentStage.targetCount; i++) {
                            currentStage.targets[i].isCleared = false;
                            currentStage.targets[i].hitTime = 0;
                        }
                    }
                }
                if (allCleared) {
                    if (currentIdx == STAGE_COUNT - 1) { //最後のステージ(10)をクリアしたときだけ表示
                        if (!missionCompletePlayed) {
                            PlaySound(seMissionClear);
                            missionCompletePlayed = true;
                        }
                        DrawRectangle(0, 0, WIN_W, WIN_H, (Color){0, 0, 0, 180});
                        DrawText("MISSION COMPLETE!", WIN_W/2 - 260, WIN_H/2 - 60, 50, GOLD);
                        DrawText("YOU CLEARED ALL 10 STAGES", WIN_W/2 - 150, WIN_H/2 + 10, 20, RAYWHITE);
                    } else { // 1〜9ステージのクリア表示
                        DrawText("STAGE CLEAR [ENTER]", WIN_W/2 - 130, WIN_H/2 - 50, 20, GOLD);
                    }
                }

                DrawText(TextFormat("STAGE: %s", currentStage.title), 20, 20, 20, SKYBLUE);
                DrawText("R: Reset  |  1-9,0: Jump to Stage", 20, WIN_H - 30, 15, GRAY);
            }
        EndDrawing();
}

int main(void) {
    //初期化
    InitWindow(WIN_W, WIN_H, "Lumina Path");
    SetTargetFPS(60);
    InitAudioDevice();

    //サウンド、イメージの読み込み
    seMirror       = LoadSound("se_mirror.wav"); //鏡反射音「キラーン」
    sePrism        = LoadSound("se_prism.wav"); //プリズム分光音「シュピッ」
    seContam       = LoadSound("se_contam.wav"); //汚染警告音「ジリジリ」
    seTargetGlow   = LoadSound("se_target_glow.wav"); //ターゲット蓄積音「シャラシャラ」
    seTargetClear  = LoadSound("se_target_clear.wav"); //ターゲット達成音「ピロン」
    seRotate       = LoadSound("se_rotate.wav"); //エミッター操作音「ギー」
    seMissionClear = LoadSound("se_mission_clear.wav"); //ミッションクリア「テッテレー」
    bgm = LoadMusicStream("bgm.ogg"); //BGM
    
    bgm.looping = true;
    SetMusicVolume(bgm, 0.35f);
#ifndef __EMSCRIPTEN__
    PlayMusicStream(bgm);
#endif

    //背景画像読み込み、リサイズ

    Image img = LoadImage("background.jpg");
    if (img.data != NULL) {
        ImageResize(&img, WIN_W, WIN_H);
        bgTex = LoadTextureFromImage(img);
        UnloadImage(img);
    } else bgTex = (Texture2D){ 0 };


    //ゲーム状態初期化
    ResetInventory();
    currentState = STATE_TITLE; //最初はタイトル
    dragMode = 0; //【ドラッグモード】0:なし 1:鏡 2:プリズム 3:発射台
    dragIdx = -1; //何をドラッグしているか（↑）
    dragStartNode = true; //線の始点か否か
    globalTime = 0;
    missionCompletePlayed = false;



#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(GameFrame, 0, 1);
#else
    while (!WindowShouldClose()) {
        GameFrame();
    }
    //【メモリ解放】
    UnloadSound(seMirror); 
    UnloadSound(sePrism); 
    UnloadSound(seContam);
    UnloadSound(seTargetGlow); 
    UnloadSound(seTargetClear); 
    UnloadSound(seRotate);
    UnloadSound(seMissionClear);
    UnloadTexture(bgTex); 
    UnloadMusicStream(bgm); 
    CloseAudioDevice(); 
    CloseWindow();

#endif
    return 0;
}
