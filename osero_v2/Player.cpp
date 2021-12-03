#include "Player.h"

/// <summary>
/// �������֐�
/// </summary>
/// <param name="newColor"> ���̃v���C���[�̐F </param>
/// <param name="netMng"> �l�b�g���[�N�̃|�C���^ </param>
/// <returns> �����̃t���O </returns>
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
/// �����̃^�[���̎��ɖ��t���[�����s�����֐�
/// </summary>
void Player::ProcessMyTurn()
{
	// 1�t���[���O�̃}�E�X�f�[�^���L�����ϐ�
	static MouseData oldMouse = mData;

	// �}�E�X�f�[�^�̍X�V
	mData.isClick = GetMouseInput() & MOUSE_INPUT_LEFT;
	GetMousePoint(&mData.mx, &mData.my);

	// �}�E�X���W��Ֆʏ�̍��W�ɍ��킹��
	mData.mx -= 50;
	mData.my -= 50;

	mData.mx /= 50;
	mData.my /= 50;

	if (0 > mData.mx || mData.mx > 7 || 0 > mData.my || mData.my > 7)
	{
		// �}�E�X���ՖʊO�ɂ���΁A1�t���[���O�̃}�E�X�f�[�^���Q�Ƃ�����
		mData = oldMouse;
	}
	
	// �O�t���[���Ƃ̃}�E�X�f�[�^���Ⴆ�΁A�l�b�g���[�N�ő��M����
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
/// �l�b�g���[�N�ő���̃f�[�^���󂯎�낤�ƕK���Ȏq
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