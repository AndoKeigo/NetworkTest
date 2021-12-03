#include "Player.h"

/// <summary>
/// 初期化関数
/// </summary>
/// <param name="newColor"> このプレイヤーの色 </param>
/// <param name="netMng"> ネットワークのポインタ </param>
/// <returns> 成功のフラグ </returns>
int Player::Init(BoardColor newColor, NetworkManager* netMng)
{
	net = netMng;
	myColor = newColor;

	mData.isClick = FALSE;
	mData.mx = 0;
	mData.my = 0;

	return TRUE;
}

/// <summary>
/// 自分のターンの時に毎フレーム実行される関数
/// </summary>
void Player::ProcessMyTurn()
{
	// 1フレーム前のマウスデータを記した変数
	static MouseData oldMouse = mData;

	// マウスデータの更新
	mData.isClick = GetMouseInput() & MOUSE_INPUT_LEFT;
	GetMousePoint(&mData.mx, &mData.my);

	// マウス座標を盤面上の座標に合わせる
	mData.mx -= 50;
	mData.my -= 50;

	mData.mx /= 50;
	mData.my /= 50;

	if (0 > mData.mx || mData.mx > 7 || 0 > mData.my || mData.my > 7)
	{
		// マウスが盤面外にあれば、1フレーム前のマウスデータを参照させる
		mData = oldMouse;
	}
	
	// 前フレームとのマウスデータが違えば、ネットワークで送信する
	if (memcmp(&mData, &oldMouse, sizeof(MouseData)) != 0)
	{
#ifndef _LONLY
		if (mData.isClick == TRUE)	net->SendDataTcp(&mData, sizeof(MouseData));
		else						net->SendDataUdp(&mData, sizeof(MouseData));
#endif
		oldMouse = mData;
	}
}

const MouseData* Player::GetMouseData()
{
	return &mData;
}

/// <summary>
/// ネットワークで相手のデータを受け取ろうと必死な子
/// </summary>
void NetEnemy::ProcessMyTurn()
{
	static int isFirst = TRUE;

	if (isFirst)
	{
		while(net->ReceiveDataUdp(&mData, sizeof(MouseData)) == 0){}
		isFirst = FALSE;
	}

	if (net->ReceiveDataTcp(&mData, sizeof(MouseData)) == 1)
	{
		net->ReceiveDataUdp(&mData, sizeof(MouseData));
	}
	else
	{
		isFirst = TRUE;
	}
}