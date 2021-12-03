#include "OthelloManager.h"

/// <summary>
/// 初期化関数
/// </summary>
/// <param name="plColor"> 操作するプレイヤーの色 </param>
/// <param name="newNet"> 送受信を行うネットワークのポインタ </param>
/// <returns> 成功のフラグ </returns>
int OthelloManager::Init(int plColor, NetworkManager* newNet)
{
	net = newNet;

	// テーブルの初期位置を設定
	for(int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			table[i][j] = 2;
		}
	}
	table[3][3] = 0;
	table[4][4] = 0;
	table[3][4] = 1;
	table[4][3] = 1;

	// ターンの始まりは黒
	turn = 0;

	// 置ける位置を最初に評価
	for (int y = 0; y < 8; y++)
	{
		for (int x = 0; x < 8; x++)
		{
			evalTable[y][x] = CheckCanPut({ x, y }, turn);
		}
	}

	// プレイヤーの初期化処理
	// pl[0]は黒のプレイヤー
	// pl[1]は白のプレイヤー
#ifdef _LONLY
	pl[0] = new Player;
	pl[1] = new Player;
#else
	if (plColor == 0)
	{
		pl[0] = new Player;
		pl[1] = new NetEnemy;
	}
	else
	{
		pl[1] = new Player;
		pl[0] = new NetEnemy;
	}
#endif

	pl[0]->Init(BoardColor::Black, net);
	pl[1]->Init(BoardColor::White, net);

	return TRUE;
}

void OthelloManager::Process()
{
	pl[turn]->ProcessMyTurn();

	const MouseData* md = pl[turn]->GetMouseData();

	if(md->isClick)
	{
		Vector2Int mPoint = { md->mx, md->my };
		if (CheckCanPut(mPoint, turn))
		{
			CheckCanPut(mPoint, turn, TRUE);
			table[mPoint.y][mPoint.x] = turn;
			turn ^= 1;

			int isPass = TRUE;
			for (int y = 0; y < 8; y++)
			{
				for (int x = 0; x < 8; x++)
				{
					evalTable[y][x] = CheckCanPut({ x, y }, turn);
					if (evalTable[y][x] > 0)isPass = FALSE;
				}
			}

			if (isPass)
			{
				MessageBox(NULL, "置ける場所がありません。", "Pass", NULL);
				turn ^= 1;
				for (int y = 0; y < 8; y++)
				{
					for (int x = 0; x < 8; x++)
					{
						evalTable[y][x] = CheckCanPut({ x, y }, turn);
						if (evalTable[y][x] > 0)isPass = FALSE;
					}
				}
				if (isPass)
				{
					// ゲーム終了処理
				}
			}
		}
	}
}

void OthelloManager::Draw()
{
	int mx, my;
	mx = pl[turn]->GetMouseData()->mx;
	my = pl[turn]->GetMouseData()->my;

	DrawBox(50, 50, (8 * 50) + 50, (8 * 50) + 50, GetColor(0, 200, 0), TRUE);

	for (int y = 0; y < 8 + 1; y++)
	{
		int yy = (y * 50) + 50;
		DrawLine(50, yy, (8 * 50) + 50, yy, GetColor(255, 255, 255));

		for (int x = 0; x < 8 + 1; x++)
		{
			int xx = (x * 50) + 50;
			DrawLine(xx, 50, xx, (8 * 50) + 50, GetColor(255, 255, 255));
		}
	}

	for (int y = 0; y < 8; y++)
	{
		int yy = (y * 50) + 50;

		for (int x = 0; x < 8; x++)
		{
			int xx = (x * 50) + 50;

			switch (table[y][x])
			{
			case 0:
				DrawCircle(xx + 25, yy + 25, 20, GetColor(0, 0, 0), TRUE);
				break;

			case 1:
				DrawCircle(xx + 25, yy + 25, 20, GetColor(255, 255, 255), TRUE);
				break;
				
			default:
				if (mx == x && my == y)
				{
					DrawBox(xx + 1, yy + 1, xx + 49, yy + 49, GetColor(0, 0, 255), TRUE);
				}
				if (evalTable[y][x])
				{
					DrawCircle(xx + 25, yy + 25, 5, GetColor(255, 0, 0), TRUE);
				}
				break;
			}
		}
	}
}

int OthelloManager::CheckCanPut(Vector2Int putPos, int turn, int isChange)
{
	int returnVal = 0;

	static const Vector2Int dir[8] = {
		{  1,  1 },
		{  0,  1 },
		{  0, -1 },
		{  1, -1 },
		{  1,  0 },
		{ -1,  1 },
		{ -1,  0 },
		{ -1, -1 }
	};

	if (table[putPos.y][putPos.x] != 2)return 0;

	for(int i = 0; i < 8; i++)
	{
		Vector2Int targetPos = { putPos.x, putPos.y };

		for(int j = 0; j < 8; j++)
		{
			targetPos.x += dir[i].x;
			targetPos.y += dir[i].y;

			if(0 > targetPos.x || targetPos.x > 7 || 0 > targetPos.y || targetPos.y > 7)
			{
				break;
			}
			else if(table[targetPos.y][targetPos.x] == 2)
			{
				break;
			}

			if(table[targetPos.y][targetPos.x] == turn)
			{
				returnVal += j;

				if (isChange)
				{
					for (int k = 1; k < 8; k++)
					{
						int* tableNum = &table[putPos.y + dir[i].y * k][putPos.x + dir[i].x * k];

						if (*tableNum == turn)break;
						*tableNum = turn;
					}
				}
				break;
			}
		}
	}

	return returnVal;
}

void OthelloManager::Destroy()
{
	delete(pl[0]);
	delete(pl[1]);
}