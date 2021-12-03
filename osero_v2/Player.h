#pragma once

#include "Main.h"
#include "net-manager.h"

/// <summary>
/// �v���C���[�N���X
/// </summary>
class Player
{
protected:
	BoardColor myColor;							// ���������������\���ϐ�
	MouseData mData;
	NetworkManager* net;

public:
	// �������֐�
	int Init(BoardColor color, NetworkManager* net);

	// ���^�[���Ŗ��t���[�����s�����֐�
	virtual void ProcessMyTurn();

	// �}�E�X�̃f�[�^���擾����֐�
	const MouseData* GetMouseData();
};

class NetEnemy : public Player
{
public:
	virtual void ProcessMyTurn() override;
};