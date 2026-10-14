#include "game_logic.h"
#include <stdio.h>


//グローバル変数
Sound seMirror, sePrism, seContam, seTargetGlow, seTargetClear, seRotate, seMissionClear;
Music bgm;
Stage currentStage; //現在のステージ
int currentIdx = 0; //ステージ番号
bool isGameOver = false; //ゲーム終了フラグ
float contaminationLevel = 0.0f; //（違う色による）汚染レベル


//ステージ設定（それぞれの値の意味はcommon.hのStage構造体を参照）
Stage allStages[] = {
    { "1. Tutorial: Direct", 
        {40, 250, 0.0f}, 
        1, {{{0,0},{0,0},true,false,false}}, 
        0, {}, 
        0, {}, 
        1, {{{700, 400}, WHITE_FLAG, 0, 0, false, "WHITE", 0}}, 
        0,0,0 },
    { "2. Tutorial: Synthesis", 
        {40, 250, 0.0f}, 
        2, {{{0,0},{0,0},true,false,false},{{0,0},{0,0},true,false,false}}, 
        1, {{{0,0},{0,0},true,false,false}}, 
        0, {}, 
        1, {{{700, 450}, MAGENTA_FLAG, 0, 0, false, "MAGENTA", 0}}, 
        0,0,0 },
    { "3. Obstacle Avoidance", 
        {40, 300, 0.0f}, 
        3, {{{0,0},{0,0},true,false,false},{{0,0},{0,0},true,false,false},{{0,0},{0,0},true,false,false}}, 
        0, {}, 
        2, {{{300, 100}, {300, 500}, {300,100}, {300,500}, 0, 0}, {{500, 0}, {500, 400}, {500,0}, {500,400}, 0, 0}}, 
        1, {{{750, 300}, WHITE_FLAG, 0, 0, false, "WHITE", 0}}, 
        0,0,0 },
    { "4. Optical Maze", 
        {40, 90, 0.5f}, 
        3, {{{0,0},{0,0},true,false,false},{{0,0},{0,0},true,false,false},{{0,0},{0,0},true,false,false}}, 
        0, {}, 
        6, 
        {
            {{0,150},{300,150},{0,150},{300,150},0,0}, 
            {{400,0},{400,250},{400,0},{400,250},0,0}, 
            {{150,300},{800,300},{150,300},{800,300},0,0}, 
            {{450,430},{450,500},{450,430},{450,500},0,0}, 
            {{600,300},{600,450},{600,300},{600,450},0,0}, 
            {{0,450},{300,450},{0,450},{300,450},0,0}
        }, 
        1, {{{700, 450}, WHITE_FLAG, 0, 0, false, "WHITE", 0}}, 
        0,0,0 },
    { "5. Filter Challenge", 
        {40, 250, 0.0f}, 
        2, {{{0,0},{0,0},true,false,false},{{0,0},{0,0},true,false,false}}, 
        1, {{{0,0},{0,0},true,false,false}}, 
        1, {{{400, 240}, {400, 260}, {400,240}, {400,260}, 0, 0}}, 
        2, {{{700, 250}, MAGENTA_FLAG, GREEN_FLAG, 0, false, "MAGENTA", 0}, {{700, 50}, GREEN_FLAG, 0, 0, false, "GREEN", 0}}, 
        0,0,0 },
    { "6. Backtracking", 
        {40, 300, 0.0f}, 
        4, {{{0,0},{0,0},true,false,false},{{0,0},{0,0},true,false,false},{{0,0},{0,0},true,false,false},{{0,0},{0,0},true,false,false}}, 
        1, {{{0,0}, {0,0}, true, false, false}}, 
        4, 
        {
            {{400, 100}, {400, 500}, {400, 100}, {400, 500}, 0, 0}, 
            {{650, 200}, {750, 200}, {650, 200}, {750, 200}, 0, 0},
            {{650, 200}, {650, 400}, {650, 200}, {650, 400}, 0, 0},
            {{650, 400}, {750, 400}, {650, 400}, {750, 400}, 0, 0}
        }, 
        3,
        {
            {{600, 200}, RED_FLAG,   GREEN_FLAG | BLUE_FLAG, 0, false, "RED", 0},
            {{600, 400}, BLUE_FLAG,  GREEN_FLAG | RED_FLAG,  0, false, "BLUE", 0},
            {{720, 300}, MAGENTA_FLAG, GREEN_FLAG,           0, true, "MAGENTA",     0}
        },
        0,0,0 },
    { "7. Moving Shields", 
        {40, 250, 0.0f}, 
        3, {{{0,0},{0,0},true,false,false},{{0,0},{0,0},true,false,false},{{0,0},{0,0},true,false,false}}, 
        1, {{{0,0},{0,0},true,false,false}}, 
        2, {{{300, 50}, {300, 200}, {300, 50}, {300, 200}, 200, 2.0f}, {{500, 300}, {500, 450}, {500, 300}, {500, 450}, 200, -2.5f}}, 
        1, {{{750, 250}, MAGENTA_FLAG, 0, 0, false, "MAGENTA", 0}}, 
        0,0,0 },
    { "8. Space Mode: Gravity", 
        {40, 100, 0.0f}, 
        3, {{{0,0},{0,0},true,false,false},{{0,0},{0,0},true,false,false},{{0,0},{0,0},true,false,false}}, 
        1, {{{0,0},{0,0},true,false,false}}, 
        0, {}, 
        1, {{{750, 450}, CYAN_FLAG, 0, 0, false, "CYAN", 0}}, 
        1,0,0 },
    { "9. Deep-sea Mode: Light Attenuation", 
        {40, 250, 0.0f}, 
        4, {{{0,0},{0,0},true,false,false},{{0,0},{0,0},true,false,false},{{0,0},{0,0},true,false,false},{{0,0},{0,0},true,false,false}}, 
        1, {{{0,0},{0,0},true,false,false}}, 
        1, {{{450, 250}, {450, 250}, {450, 250}, {450, 250}, 0, 0}}, 
        2, {{{450, 100}, RED_FLAG, 0, 0, false, "RED", 0}, {{450, 400}, BLUE_FLAG, 0, 0, false, "BLUE", 0}}, 
        0,1,0 },
    { "10. Hybrid Mode", 
        {40, 100, 0.5f}, 
        4, {{{0,0},{0,0},true,false,false},{{0,0},{0,0},true,false,false},{{0,0},{0,0},true,false,false},{{0,0},{0,0},true,false,false}}, 
        2, {{{0,0},{0,0},true,false,false},{{0,0},{0,0},true,false,false}}, 
        0, {}, 
        3, {
            {{200, 80}, RED_FLAG, 0, 0, false, "RED", 0}, 
            {{700, 280}, GREEN_FLAG, 0, 0, false, "GREEN", 0},
            {{400, 450}, YELLOW_FLAG, 0, 0, false, "YELLOW", 0}
        }, 
        0,0,1 }
};


//交差判定
bool CheckLineIntersection(Vector2 p1, Vector2 p2, Vector2 p3, Vector2 p4, Vector2 *intersection) {
    //線分1(p1->p2), 線分2(p3->p4)
    //2本の方向ベクトルの外積
    float den = (p1.x - p2.x) * (p3.y - p4.y) - (p1.y - p2.y) * (p3.x - p4.x);
    //交点が存在しない（平行or同一直線）
    if (fabs(den) < 0.001f) return false;
    //線分1のどこに交点があるか
    float t = ((p1.x - p3.x) * (p3.y - p4.y) - (p1.y - p3.y) * (p3.x - p4.x)) / den;
    //線分2のどこに交点があるか
    float u = -((p1.x - p2.x) * (p1.y - p3.y) - (p1.y - p2.y) * (p1.x - p3.x)) / den;
    
    if (t >= 0 && t <= 1 && u >= 0 && u <= 1) { //線分の中で交差点がある場合
        if (intersection != NULL) { 
            //交点をintersectionに記録
            intersection->x = p1.x + t * (p2.x - p1.x); 
            intersection->y = p1.y + t * (p2.y - p1.y); 
        }
        return true;
    }
    return false;
}


//インベントリのリセット
void ResetInventory() {
    currentStage = allStages[currentIdx]; //現在のステージのデータを読み込み
    //鏡をインベントリ内に整列
    for(int i=0; i<currentStage.mirrorCount; i++) {
        if(currentStage.mirrors[i].inInventory) { 
            currentStage.mirrors[i].start = (Vector2){ 120 + i*60, GAME_H + 30 }; 
            currentStage.mirrors[i].end = (Vector2){ 150 + i*60, GAME_H + 70 }; 
        }
        currentStage.mirrors[i].wasHitPrevFrame = false;
        currentStage.mirrors[i].isHitThisFrame = false;
    }
    //プリズムの整列
    for(int i=0; i<currentStage.prismCount; i++) {
        if(currentStage.prisms[i].inInventory) { 
            currentStage.prisms[i].start = (Vector2){ 450 + i*90, GAME_H + 30 }; 
            currentStage.prisms[i].end = (Vector2){ 500 + i*90, GAME_H + 70 }; 
        }
        currentStage.prisms[i].wasHitPrevFrame = false;
        currentStage.prisms[i].isHitThisFrame = false;
    }
}


//光の描画（発光しているような装飾付き）
void DrawGlowLine(Vector2 start, Vector2 end, int colorFlags, float alphaMult) {
    if (alphaMult <= 0.05f || colorFlags == 0) return;
    
    //基本色の決定（Color構造体）
    Color baseCol = WHITE;
    if (colorFlags == RED_FLAG) baseCol = RED;
    else if (colorFlags == GREEN_FLAG) baseCol = GREEN;
    else if (colorFlags == BLUE_FLAG) baseCol = BLUE;
    else if (colorFlags == MAGENTA_FLAG) baseCol = MAGENTA;
    else if (colorFlags == CYAN_FLAG) baseCol = COLOR_CYAN;
    else if (colorFlags == YELLOW_FLAG) baseCol = YELLOW;

    //一番外側
    Color halo = baseCol; 
    halo.a = (unsigned char)(baseCol.a * 0.2f * alphaMult); //元の20%まで透明にし、距離による減衰をかける
    DrawLineEx(start, end, 9.0f, halo);

    //中間
    Color glow = baseCol; 
    glow.a = (unsigned char)(baseCol.a * 0.5f * alphaMult); //透明度は50%
    DrawLineEx(start, end, 4.5f, glow);

    //中心（白に近い色） 
    Color core = WHITE; 
    if (colorFlags != WHITE_FLAG) {
        core = (Color){ (unsigned char)((baseCol.r + 255)/2), (unsigned char)((baseCol.g + 255)/2), (unsigned char)((baseCol.b + 255)/2), 255 };
    }
    core.a = (unsigned char)(255 * alphaMult);
    DrawLineEx(start, end, 1.2f, core);
}


//レーザーの計算
void CalculateLaserHybrid(Vector2 pos, Vector2 dir, int color, int depth, float totalDist, float time) {
    //終了条件（MAX_REFLECTIONSで無限ループ阻止）
    if (depth >= MAX_REFLECTIONS || color == 0 || totalDist > 2000.0f) return;
    
    //レーザー開始位置、進行方向
    Vector2 curP = pos, curD = dir;

    //重力モードorハイブリッドモード
    if (currentStage.isHybridMode || currentStage.isGravityMode) {
        for (int s = 0; s < 300; s++) { //最大300回で少しずつ進める
            if (currentStage.isGravityMode || (currentStage.isHybridMode && curP.y < 300)) {
                Vector2 bh = (currentStage.isHybridMode) ? (Vector2){400, 150} : (Vector2){400, 300}; //ブラックホールの位置
                Vector2 toG = Vec2Subtract(bh, curP); //ブラックホールへの方向
                float d2 = toG.x * toG.x + toG.y * toG.y; //距離^2

                if (d2 > 400 && d2 < 30000) {
                    float gravStrength = 1200.0f / d2;
                    curD = Vec2Normalize(Vec2Add(curD, Vec2Scale(Vec2Normalize(toG), gravStrength)));
                }
            }

            //次の位置へ進める（4pxずつ）
            Vector2 nextP = Vec2Add(curP, Vec2Scale(curD, 4.0f));
            if (nextP.y > GAME_H) break;
            Vector2 drawNextP = nextP;
            
            //ハイブリッドの場合、下半分に空間ゆがみ
            if (currentStage.isHybridMode && curP.y > 300 && curP.y < 500 && curP.x > 350 && curP.x < 450 && curP.y > 350) {
                drawNextP.x += sinf(time * 12.0f + s * 0.1f) * 1.5f;
            }

            //鏡との衝突判定
            bool hit = false; 
            Vector2 cp;

            for (int i=0; i<currentStage.mirrorCount; i++) {
                if (!currentStage.mirrors[i].inInventory && CheckLineIntersection(curP, nextP, currentStage.mirrors[i].start, currentStage.mirrors[i].end, &cp)) {
                    currentStage.mirrors[i].isHitThisFrame = true;
                    DrawCircleGradient((int)cp.x, (int)cp.y, 6.0f, WHITE, BLANK);
                    Vector2 v = Vec2Subtract(currentStage.mirrors[i].end, currentStage.mirrors[i].start);
                    CalculateLaserHybrid(cp, Vec2Reflect(curD, Vec2Normalize((Vector2){-v.y, v.x})), color, depth + 1, totalDist + s*4, time);
                    hit = true; break;
                }
            }

            //プリズムとの衝突判定
            if (!hit) {
                for (int i=0; i<currentStage.prismCount; i++) {
                    if (!currentStage.prisms[i].inInventory && CheckLineIntersection(curP, nextP, currentStage.prisms[i].start, currentStage.prisms[i].end, &cp)) {
                        currentStage.prisms[i].isHitThisFrame = true;
                        DrawCircleGradient((int)cp.x, (int)cp.y, 6.0f, WHITE, BLANK);
                        float a = atan2f(curD.y, curD.x);
                        if (color & RED_FLAG) CalculateLaserHybrid(cp, (Vector2){cosf(a+0.25f), sinf(a+0.25f)}, RED_FLAG, depth + 1, totalDist + s*4, time);
                        if (color & GREEN_FLAG) CalculateLaserHybrid(cp, curD, GREEN_FLAG, depth + 1, totalDist + s*4, time);
                        if (color & BLUE_FLAG) CalculateLaserHybrid(cp, (Vector2){cosf(a-0.25f), sinf(a-0.25f)}, BLUE_FLAG, depth + 1, totalDist + s*4, time);
                        hit = true; break;
                    }
                }
            }

            //何かに当たったらレーザー終了
            if (hit) return;

            //ターゲット判定
            for (int i = 0; i < currentStage.targetCount; i++) {
                if (Vec2Distance(nextP, currentStage.targets[i].position) < 35) {
                    currentStage.targets[i].tempColorFlags |= color;
                }
            }

            //深海＆ハイブリッド用の減衰
            float alphaMult = fmaxf(0, 1.0f - (totalDist + s*4) * (((currentStage.isDeepSeaMode || currentStage.isHybridMode) ? 0.4f : 0.0f) / 255.0f));
            
            //レーザー描画
            DrawGlowLine(curP, drawNextP, color, alphaMult);

            //次のステップへ
            curP = nextP;

            //画面外にでたら終了
            if (curP.x < 0 || curP.x > WIN_W || curP.y < 0 || curP.y > GAME_H) break;
        }
    } 
    
    //通常モード
    else {
        float closestT = 1200.0f; //衝突までの最短距離
        Vector2 colP = {0,0}, //衝突点
        colN = {0,0}; //衝突面の法線
        int hitType = 0, //0:なし、1:鏡、2:プリズム、3:壁
        hitIdx = -1; //ミラー、プリズムのインデックス

        Vector2 lEnd = Vec2Add(pos, Vec2Scale(dir, 2000.0f)); //レーザー終点

        //壁との衝突判定
        for (int i=0; i<currentStage.wallCount; i++) {
            Vector2 inter; 
            if (CheckLineIntersection(pos, lEnd, currentStage.walls[i].start, currentStage.walls[i].end, &inter)) {
                float d = Vec2Distance(pos, inter); 
                if (d < closestT && d > 0.5f) { //一番近い壁だけ記録
                    closestT = d; colP = inter; hitType = 3; 
                }
            }
        }

        //鏡との衝突判定
        for (int i=0; i<currentStage.mirrorCount; i++) {
            if (!currentStage.mirrors[i].inInventory) {
                Vector2 inter; 
                if (CheckLineIntersection(pos, lEnd, currentStage.mirrors[i].start, currentStage.mirrors[i].end, &inter)) {
                    float d = Vec2Distance(pos, inter); 
                    if (d < closestT && d > 0.5f) { 
                        closestT = d; 
                        colP = inter; 
                        hitType = 1; 
                        hitIdx = i; 
                        Vector2 v = Vec2Subtract(currentStage.mirrors[i].end, currentStage.mirrors[i].start); 
                        colN = Vec2Normalize((Vector2){ -v.y, v.x }); 
                    }
                }
            }
        }

        //プリズムとの衝突判定
        for (int i=0; i<currentStage.prismCount; i++) {
            if (!currentStage.prisms[i].inInventory) {
                Vector2 inter; 
                if (CheckLineIntersection(pos, lEnd, currentStage.prisms[i].start, currentStage.prisms[i].end, &inter)) {
                    float d = Vec2Distance(pos, inter); 
                    if (d < closestT && d > 0.5f) { 
                        closestT = d; 
                        colP = inter; 
                        hitType = 2; 
                        hitIdx = i; 
                        Vector2 v = Vec2Subtract(currentStage.prisms[i].end, currentStage.prisms[i].start); 
                        colN = Vec2Normalize((Vector2){ -v.y, v.x }); 
                    }
                }
            }
        }

        //ターゲット判定
        for (int i=0; i<currentStage.targetCount; i++) {
            Vector2 toT = Vec2Subtract(currentStage.targets[i].position, pos); 
            float proj = Vec2Dot(toT, dir);
            if (proj > 0 && proj < closestT) { 
                Vector2 nearP = Vec2Add(pos, Vec2Scale(dir, proj)); 
                if (Vec2Distance(nearP, currentStage.targets[i].position) < 35) {
                    currentStage.targets[i].tempColorFlags |= color; 
                }
            }
        }   

        //減衰
        float alphaMult = fmaxf(0, 1.0f - (totalDist + closestT) * ((currentStage.isDeepSeaMode ? 0.4f : 0.0f) / 255.0f));
        
        //レーザー描画
        DrawGlowLine(pos, Vec2Add(pos, Vec2Scale(dir, closestT)), color, alphaMult);

        //衝突後の処理
        if (hitType > 0 && alphaMult > 0.1f) {
            DrawCircleGradient((int)colP.x, (int)colP.y, 6.0f, WHITE, BLANK);
            if (hitType == 1) { //鏡→反射
                currentStage.mirrors[hitIdx].isHitThisFrame = true;
                CalculateLaserHybrid(colP, Vec2Reflect(dir, colN), color, depth + 1, totalDist + closestT, time);
            }
            else if (hitType == 2) { //プリズム→色の分解
                currentStage.prisms[hitIdx].isHitThisFrame = true;
                float a = atan2f(dir.y, dir.x);
                if (color & RED_FLAG) CalculateLaserHybrid(colP, (Vector2){cosf(a+0.25f), sinf(a+0.25f)}, RED_FLAG, depth + 1, totalDist + closestT, time);
                if (color & GREEN_FLAG) CalculateLaserHybrid(colP, dir, GREEN_FLAG, depth + 1, totalDist + closestT, time);
                if (color & BLUE_FLAG) CalculateLaserHybrid(colP, (Vector2){cosf(a-0.25f), sinf(a-0.25f)}, BLUE_FLAG, depth + 1, totalDist + closestT, time);
            }
        }
    }
}