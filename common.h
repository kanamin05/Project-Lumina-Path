#ifndef COMMON_H
#define COMMON_H

#include "raylib.h"
#include <math.h>

// レイアウト設定
#define WIN_W 800 //ウィンドウ幅
#define WIN_H 600 //ウィンドウ高さ
#define INV_H 100 //インベントリ部分の高さ           
#define GAME_H (WIN_H - INV_H) //パズルを解くエリア

// ゲーム定数
#define MAX_REFLECTIONS 16 //無限ループ防止
#define MAX_OBJ 15 //1つのステージに配置できるオブジェクトの最大数
#define TARGET_HIT_TIME_NEEDED 1.5f //ターゲットをクリアするために光を当て続ける秒数
#define STAGE_COUNT 10 //ステージ数
#define CONTAMINATION_LIMIT 1.0f //違う色の光（汚染）の時間限界値

// 色フラグ（ビットで管理、論理和で色の和を取る）
#define RED_FLAG     (1 << 0)   //001:赤
#define GREEN_FLAG   (1 << 1)   //010:緑
#define BLUE_FLAG    (1 << 2)   //100:青
#define MAGENTA_FLAG (RED_FLAG | BLUE_FLAG) //101:マゼンタ
#define CYAN_FLAG    (GREEN_FLAG | BLUE_FLAG) //110:シアン
#define YELLOW_FLAG  (RED_FLAG | GREEN_FLAG) //011:イエロー
#define WHITE_FLAG   (RED_FLAG | GREEN_FLAG | BLUE_FLAG) //111:白
#define COLOR_CYAN (Color){ 0, 255, 255, 255 } //シアン定義

// ゲーム状態
typedef enum { STATE_TITLE, STATE_TUTORIAL_1, STATE_TUTORIAL_2, STATE_GAME } GameState;

// 構造体定義
typedef struct { 
    Vector2 pos; //位置
    float angle; //角度
} LaserEmitter; //発射台

typedef struct { 
    Vector2 start; Vector2 end; //鏡の線分の両端
    bool inInventory; //インベントリ内にあるか
    bool wasHitPrevFrame; //前のフレームで光が当たっていたか
    bool isHitThisFrame; //現在のフレームで光が当たっているか
} Mirror; //鏡

typedef struct { 
    Vector2 start; Vector2 end; 
    bool inInventory; 
    bool wasHitPrevFrame;
    bool isHitThisFrame;
} Prism; //プリズム

typedef struct { 
    Vector2 start; Vector2 end; 
    Vector2 originStart; Vector2 originEnd; //動く壁の初期位置
    float moveRange; //移動範囲
    float speed; //移動速度
} Wall; //障害壁

typedef struct { 
    Vector2 position; //ターゲットの座標
    int requiredColor; //クリアに必要な光の色
    int forbiddenColor; //当ててはいけない色
    float hitTime; //光が当たっている累積時間
    bool isCleared; //クリア済みフラグ
    const char* label; //表示名
    int tempColorFlags; //現在当たっている光の色
} Target; //ターゲット

typedef struct {
    const char* title; //ステージ名
    LaserEmitter emitter; //発射台
    int mirrorCount; Mirror mirrors[MAX_OBJ]; //セットされた鏡
    int prismCount; Prism prisms[MAX_OBJ]; //セットされたプリズム
    int wallCount; Wall walls[MAX_OBJ]; //配置された壁
    int targetCount; Target targets[MAX_OBJ]; //配置されたターゲット

    //特殊モードのフラグ
    bool isGravityMode; //ブラックホール
    bool isDeepSeaMode; //深海
    bool isHybridMode; //複合
} Stage; //ステージ

// インライン数学関数（2次元ベクトル）
static inline Vector2 Vec2Add(Vector2 v1, Vector2 v2) { //加算
    return (Vector2){ v1.x + v2.x, v1.y + v2.y }; 
}
static inline Vector2 Vec2Subtract(Vector2 v1, Vector2 v2) { //減算
    return (Vector2){ v1.x - v2.x, v1.y - v2.y }; 
}
static inline float Vec2Length(Vector2 v) { //ベクトルの大きさ（長さ）
    return sqrtf(v.x * v.x + v.y * v.y); 
}
static inline Vector2 Vec2Normalize(Vector2 v) { //ベクトル正規化
    float len = Vec2Length(v); 
    return (len > 0) ? (Vector2){ v.x / len, v.y / len } : (Vector2){ 0, 0 }; 
}
static inline Vector2 Vec2Scale(Vector2 v, float scale) { //乗算（長さの伸縮）
    return (Vector2){ v.x * scale, v.y * scale }; 
}
static inline float Vec2Dot(Vector2 v1, Vector2 v2) { //内積
    return v1.x * v2.x + v1.y * v2.y; 
}
static inline Vector2 Vec2Reflect(Vector2 v, Vector2 n) { //反射の向きの計算
    float dot = Vec2Dot(v, n); 
    return (Vector2){ v.x - 2.0f * dot * n.x, v.y - 2.0f * dot * n.y }; 
}
static inline float Vec2Distance(Vector2 v1, Vector2 v2) { //距離
    return sqrtf(powf(v1.x - v2.x, 2) + powf(v1.y - v2.y, 2)); 
}
static inline float Clamp(float v, float min, float max) { //値の制御
    return (v < min) ? min : (v > max) ? max : v; 
}

// プロトタイプ宣言
bool CheckLineIntersection(Vector2 p1, Vector2 p2, Vector2 p3, Vector2 p4, Vector2 *intersection);

#endif