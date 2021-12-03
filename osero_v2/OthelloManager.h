#pragma once
#include "Player.h"
#include "Main.h"

class OthelloManager
{
private:
	int table[8][8];				// ���ۂɔ��ƍ����L����Ă���Ֆ�
	int evalTable[8][8];			// �]���l���L�����Ֆ�
	int turn;						// ���̃^�[�������̃^�[����
	Player* pl[2];
	NetworkManager* net;

public:
	// �������֐�
	int Init(int plColor, NetworkManager* newNet);

	// �Q�[�������t���[����������֐�
	void Process();

	// �I�Z���̕`��֐�
	void Draw();

	// �I�Z���̃Q�[����j������֐�
	void Destroy();

	// ���̃}�X�ɂ����邩���m�F����֐�
	int CheckCanPut(Vector2Int putPos, int turn, int isChange = FALSE);
};

