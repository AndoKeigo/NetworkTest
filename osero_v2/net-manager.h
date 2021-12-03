#pragma once

#include "main.h"

/// <summary>
/// ���O�t�@�C���𐶐����A�������݂��s���N���X
/// </summary>
class LogWriter
{
private:
	char logFilePass[128];			// ���O�t�@�C���̃p�X
	int isCSV;						// csv�`���ŕۑ����邩�̃t���O
	int isWriteTime;				// ���Ԃ܂Ń��O�ɋL�q���邩�̃t���O
	int isInit;						// ���������ς�ł��邩�̃t���O
	char logStr[2048];
	HANDLE thread;
	HANDLE mutex;
	BOOL isRun;

	static unsigned int __stdcall BeginThread(void* self);
	void WriteLog();

public:
	LogWriter();

	// �������֐�
	int Init(int isWriteTime = TRUE, int isCSV = FALSE, const char* filePass = "MyLog.txt");

	// ���O�t�@�C���ɕ�������������ފ֐�
	void AddLog(const char* format, ...);

	// �l�b�g���[�N�ł��Ƃ肵���f�[�^���o�C�i���`���ŏ������ފ֐�
	void AddLogNet(const void* buffer, size_t bufSize, int isSendData, int isTCP);

	// ���O�t�@�C���ւ̏������݂��I������֐� 
	void Exit();
};

/// <summary>
/// DX���C�u�����̃l�b�g���[�N�n�֐������b�v�����N���X
/// �i1��IP�A�h���X�ɑ΂���1�̃C���X�^���X�����j
/// </summary>
class NetworkManager
{
private:
	int netHandleTcp;			// �l�b�g���[�N�n���h��(TCP�p)
	int netHandleUdp;			// �l�b�g���[�N�n���h��(UDP�p)
	IPDATA targetIp;			// ����肵����IP�A�h���X��
	LogWriter* log;				// �������ރ��O�N���X�ւ̃|�C���^
	//int connectStatus;		// �ڑ���

public:
	// �������֐�
	int Init(LogWriter* writeLog, IPDATA targetIp);

	// �ڑ����m������֐�
	int StartConnection(int isListen);

	// �f�[�^�𑗐M����֐�
	int SendDataTcp(const void* buffer, size_t sendLength);

	// �f�[�^����M����֐�
	int ReceiveDataTcp(void* buffer, size_t targetLength, int isPeek = FALSE);

	// �f�[�^�𑗐M����֐�
	int SendDataUdp(const void* buffer, size_t sendLength);

	// �f�[�^����M����֐�
	int ReceiveDataUdp(void* buffer, size_t targetLength, int isPeek = FALSE);

	// �ڑ���j������֐�
	void DeleteConnection();

	// �ڑ��ł��Ă��邩���m�F����֐�
	int GetIsConnect();
};