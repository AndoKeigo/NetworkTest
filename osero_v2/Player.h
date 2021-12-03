#pragma once

#include "Main.h"
#include "net-manager.h"

/// <summary>
/// プレイヤークラス
/// </summary>
class Player
{
protected:
	BoardColor myColor;							// 自分が白か黒か表す変数
	MouseData mData;
	NetworkManager* net;

public:
	// 初期化関数
	int Init(BoardColor color, NetworkManager* net);

	// 自ターンで毎フレーム実行される関数
	virtual void ProcessMyTurn();

	// マウスのデータを取得する関数
	const MouseData* GetMouseData();
};

class NetEnemy : public Player
{
public:
	virtual void ProcessMyTurn() override;
};