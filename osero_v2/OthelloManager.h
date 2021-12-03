#pragma once
#include "Player.h"
#include "Main.h"

class OthelloManager
{
private:
	int table[8][8];				// 実際に白と黒が記されている盤面
	int evalTable[8][8];			// 評価値を記した盤面
	int turn;						// 白のターンか黒のターンか
	Player* pl[2];
	NetworkManager* net;

public:
	// 初期化関数
	int Init(int plColor, NetworkManager* newNet);

	// ゲーム中毎フレーム処理する関数
	void Process();

	// オセロの描画関数
	void Draw();

	// オセロのゲームを破棄する関数
	void Destroy();

	// そのマスにおけるかを確認する関数
	int CheckCanPut(Vector2Int putPos, int turn, int isChange = FALSE);
};

