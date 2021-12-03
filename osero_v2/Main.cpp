#include "Main.h"
#include "OthelloManager.h"

const IPDATA ip = { 172, 20, 43, 116 };

OthelloManager othello;
NetworkManager net;
LogWriter log;

GameState gameState;

void Draw();

// プログラムは WinMain から始まります
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	ChangeWindowMode(TRUE);		// ウィンドウモードで起動
	SetAlwaysRunFlag(TRUE);		// ネットデータはいつ届くかわからないので常にアプリを動かしていて欲しい

	if (DxLib_Init() == -1)		// ＤＸライブラリ初期化処理
	{
		return -1;			// エラーが起きたら直ちに終了
	}

#ifdef _LONLY
	othello.Init(0, &net);
	gameState = GameState::PlayGame;
#else
	gameState = GameState::SelectConnect;
#endif

	log.Init();
	net.Init(&log, ip);

	int isServer = FALSE;

	int count = 0;

	SetDrawScreen(DX_SCREEN_BACK);			// 裏画面に描画してね

	// Escキーが押されるかメッセージ処理でエラーを吐くまで無限ループ
	while (!CheckHitKey(KEY_INPUT_ESCAPE) && ProcessMessage() != -1)
	{
		switch (gameState)
		{
		// 接続をサーバー側とするかクライアント側とするかの選択
		case GameState::SelectConnect:
			if (CheckHitKey(KEY_INPUT_Z))
			{
				isServer = TRUE;
				gameState = GameState::WaitConnect;
			}
			else if (CheckHitKey(KEY_INPUT_X))
			{
				isServer = FALSE;
				gameState = GameState::WaitConnect;
			}
			break;

		// 接続の確立を行う
		case GameState::WaitConnect:
			if (net.StartConnection(isServer) == 0)
			{
				othello.Init(isServer ? 0 : 1, &net);
				gameState = GameState::PlayGame;
			}
			break;

		// 実際にオセロをプレイする
		case GameState::PlayGame:

#ifndef _LONLY
			// 毎フレーム接続できているか確認する
			if (!net.GetIsConnect())
			{
				MessageBox(NULL, "接続が切断されました", "Error", NULL);
				othello.Destroy();
				net.DeleteConnection();
				DxLib_End();
				return -1;
			}
#endif

			othello.Process();
			break;
		}

		ClearDrawScreen();
		Draw();
		ScreenFlip();
	}

	othello.Destroy();
	net.DeleteConnection();
	log.Exit();
	DxLib_End();				// ＤＸライブラリ使用の終了処理
	return 0;				// ソフトの終了 
}

void Draw()
{
	switch (gameState)
	{
	case GameState::SelectConnect:
		DrawString(0, 0, "接続を待つ場合はＺキーを、接続をする場合はＸキーを押してください", GetColor(255, 255, 255));
		break;

	case GameState::WaitConnect:
		DrawString(0, 0, "接続中...", GetColor(255, 255, 255));
		break;

	case GameState::PlayGame:
		othello.Draw();
		break;
	}
}