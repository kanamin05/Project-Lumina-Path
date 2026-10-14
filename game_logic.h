#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include "common.h"

// グローバル変数宣言
extern Sound seMirror, sePrism, seContam, seTargetGlow, seTargetClear, seRotate, seMissionClear;
extern Music bgm;
extern Stage allStages[];
extern Stage currentStage;
extern int currentIdx;
extern bool isGameOver;
extern float contaminationLevel;

// 関数宣言
void ResetInventory();
void DrawGlowLine(Vector2 start, Vector2 end, int colorFlags, float alphaMult);
void CalculateLaserHybrid(Vector2 pos, Vector2 dir, int color, int depth, float totalDist, float time);

#endif