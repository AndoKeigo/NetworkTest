#include "Main.h"
#include "OthelloManager.h"

const IPDATA ip = { 172, 20, 43, 116 };

OthelloManager othello;
NetworkManager net;
LogWriter log;

GameState gameState;

void Draw();

// �v���O������ WinMain ����n�܂�܂�
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	ChangeWindowMode(TRUE);		// �E�B���h�E���[�h�ŋN��
	SetAlwaysRunFlag(TRUE);		// �l�b�g�f�[�^�͂��͂����킩��Ȃ��̂ŏ�ɃA�v���𓮂����Ă��ė~����

	if (DxLib_Init() == -1)		// �c�w���C�u��������������
	{
		return -1;			// �G���[���N�����璼���ɏI��
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

	SetDrawScreen(DX_SCREEN_BACK);			// ����ʂɕ`�悵�Ă�

	// Esc�L�[��������邩���b�Z�[�W�����ŃG���[��f���܂Ŗ������[�v
	while (!CheckHitKey(KEY_INPUT_ESCAPE) && ProcessMessage() != -1)
	{
		switch (gameState)
		{
		// �ڑ����T�[�o�[���Ƃ��邩�N���C�A���g���Ƃ��邩�̑I��
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

		// �ڑ��̊m�����s��
		case GameState::WaitConnect:
			if (net.StartConnection(isServer) == 0)
			{
				othello.Init(isServer ? 0 : 1, &net);
				gameState = GameState::PlayGame;
			}
			break;

		// ���ۂɃI�Z�����v���C����
		case GameState::PlayGame:

#ifndef _LONLY
			// ���t���[���ڑ��ł��Ă��邩�m�F����
			if (!net.GetIsConnect())
			{
				MessageBox(NULL, "�ڑ����ؒf����܂���", "Error", NULL);
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
	DxLib_End();				// �c�w���C�u�����g�p�̏I������
	return 0;				// �\�t�g�̏I�� 
}

void Draw()
{
	switch (gameState)
	{
	case GameState::SelectConnect:
		DrawString(0, 0, "�ڑ���҂ꍇ�͂y�L�[���A�ڑ�������ꍇ�͂w�L�[�������Ă�������", GetColor(255, 255, 255));
		break;

	case GameState::WaitConnect:
		DrawString(0, 0, "�ڑ���...", GetColor(255, 255, 255));
		break;

	case GameState::PlayGame:
		othello.Draw();
		break;
	}
}