#include "net-manager.h"
#include <stdarg.h>
#include <process.h>

// ��LogWriter�N���X�̃����o�֐�

/// <summary>
/// ���O�Ǘ��N���X�̃R���X�g���N�^
/// </summary>
LogWriter::LogWriter()
{
	memset(logFilePass, 0, sizeof(logFilePass));
	isCSV = FALSE;
	isWriteTime = TRUE;
	isInit = FALSE;
}

/// <summary>
/// ���O�������֐�
/// </summary>
/// <param name="isWriteTime_"> ���Ԃ����O�ɏ������ނ��̃t���O </param>
/// <param name="isCSV_"> csv�`���ŏ������ނ��̃t���O </param>
/// <param name="filePass"> ���O�t�@�C���𐶐�����t�@�C���p�X </param>
/// <returns> �����������̃t���O </returns>
int LogWriter::Init(int isWriteTime_, int isCSV_, const char* filePass)
{
	// ���łɏ���������Ă������₱�����Ȃ�̂ŃG���[�ɂ���
	if (isInit)
	{
		MessageBox(NULL, "���O�t�@�C���͂��łɐ�������Ă��܂��B", "���炟", NULL);
		return FALSE;
	}

	// �e�l�̏�����
	isWriteTime = isWriteTime_;
	isCSV = isCSV_;
	strcpy_s(logFilePass, sizeof(logFilePass), filePass);

	// CSV�`���Ȃ�u,�v�A���̑��Ȃ�u�@�v�ŋ�؂�
	const char DELIMITER = (isCSV) ? ',' : ' ';

	FILE* fp;

	// �������݃��[�h�Ń��O�e�L�X�g�𐶐�
	if (fopen_s(&fp, logFilePass, "w") != 0)
	{
		MessageBox(NULL, "���O�t�@�C���̐����Ɏ��s���܂����B", "���炟", NULL);
		return FALSE;
	}

	// �t�@�C�����������ꂽ���Ƃ��t�@�C���ɋL�q
	DATEDATA time;
	GetDateTime(&time);

	fprintf_s(fp, "[%d/%d %02d:%02d:%02dsec]%c���O�t�@�C������\n", time.Mon, time.Day, time.Hour, time.Min, time.Sec, DELIMITER);

	fclose(fp);

	memset(logStr, 0, sizeof(logStr));

	// �X���b�h�J�n����
	isRun = TRUE;

	// �~���[�e�b�N�X�̐���
	// (�~���[�e�b�N�X�Ɋւ���Q�l�T�C�g) https://wa3.i-3-i.info/word13360.html
	mutex = CreateMutex(
		NULL,				// �Z�L�����e�B�֌W�̎w�肪�ł��邯�ǁA�g�������ƂȂ����킩���̂�NULL��
		FALSE,				// ���̃X���b�h���~���[�e�b�N�X�̏��L�҂ł��邩�ǂ����������t���O�i��{FALSE�j
		NULL				// �~���[�e�b�N�X�̖��O�i��{NULL�A���O���d������ƃG���[�ɂȂ�j
	);

	// �X���b�h�̐���
	thread = (HANDLE)_beginthreadex(
		NULL,						// �~���[�e�b�N�X�Ɠ������Z�L�����e�B�֌W�̋L�q���ł���
		0,							// �X���b�h�̃X�^�b�N���B��{��0
		LogWriter::BeginThread,		// �Ăяo�������֐��̃A�h���X�iunsigned int __stdcall (void*)�̊֐�����Ȃ��Ǝ󂯓���Ȃ��j
		this,						// �Ăяo�����֐���(void*)�����ɓn���l�A����͎������g��n���̂�this
		NULL,						// �����シ���Ɏ��s���邩�̃t���O�i0�ł������s�j
		NULL						// �X���b�h�̎��ʔԍ��i��{NULL�j
	);

	return TRUE;
}

/// <summary>
/// �X���b�h�J�n�p�̐ÓI�֐�
/// </summary>
/// <param name="self"> �X���b�h���J�n������LogWriter�N���X </param>
unsigned int __stdcall LogWriter::BeginThread(void* self)
{
	// �|�C���^���L���X�g���Ċ֐��̊J�n
	reinterpret_cast<LogWriter*>(self)->WriteLog();
	return 0;
}


/// <summary>
/// �ʃX���b�h�ōs�������t�@�C���ɒ��ڏ������ފ֐�
/// </summary>
void LogWriter::WriteLog()
{
	char cpyStr[2048] = { 0 };		// logStr�̑ޔ�p�ϐ�


	// �X���b�h�������Ă��邩�̃t���O�������Ă���Ζ������[�v
	// �i��ڂ̏����͏������ݘR�ꂪ�����悤�ɂ��Ă��邾���j
	while (isRun || logStr[0] != '\0')
	{
		if (logStr[0] == '\0')continue;

		// *** ���L�ϐ��ւ̃A�N�Z�X�J�n *** //

		WaitForSingleObject(mutex, INFINITE);

		// �������ޔ��������Ƀt�@�C���ɏ������ނƏ������d���Ȃ�̂őޔ�
		strcpy_s(cpyStr, logStr);

		// �������񂾕��z�����ɂ���
		memset(logStr, 0, sizeof(logStr));

		ReleaseMutex(mutex);

		// *** ���L�ϐ��ւ̃A�N�Z�X�I�� *** //


		FILE* fp;

		// �ǋL���[�h�Ń��O�e�L�X�g���J��
		if (fopen_s(&fp, logFilePass, "a") != 0)
		{
			MessageBox(NULL, "���O�t�@�C���̏������݂Ɏ��s���܂����B", "���炟", NULL);
			return;
		}

		fprintf_s(fp, "%s", cpyStr);

		fclose(fp);
	}
}


/// <summary>
/// �l�b�g���[�N�̑���M�f�[�^�����O�ɏ������ފ֐�
/// </summary>
/// <param name="buffer"> �������ރf�[�^�̃|�C���^ </param>
/// <param name="bufSize"> �������ރf�[�^�̃T�C�Y </param>
/// <param name="isSend"> ���M�����̃t���O </param>
void LogWriter::AddLogNet(const void* buffer, size_t bufSize, int isSend, int isTCP)
{
	const char DELIMITER = (isCSV) ? ',' : ' ';

	char logStr[1024] = { 0 };		// ���O�ɏ������ޕ�����

	// 1Byte���������ނ��߂�unsigned char�^�ɃL���X�g����
	const unsigned char* p = static_cast<const unsigned char*>(buffer);

	// ���M����M����������
	if (isSend)	sprintf_s(logStr, "[���M]%c", DELIMITER);
	else		sprintf_s(logStr, "[��M]%c", DELIMITER);

	if (isTCP)	strcat_s(logStr, "[TCP]");
	else		strcat_s(logStr, "[UDP]");

	// �f�[�^����������ł���
	for (int i = 0; i < bufSize; i++)
	{
		char addStr[4];
		sprintf_s(addStr, "%02X%c", p[i], DELIMITER);
		strcat_s(logStr, addStr);
	}

	// �o��������������O�������ݗp������ɒǉ�
	AddLog(logStr);
}


/// <summary>
/// ���O�ɕ������ǉ�����֐�
/// </summary>
/// <param name="format"> printf�Ɠ��������t�������� </param>
void LogWriter::AddLog(const char* format, ...)
{
	const char DELIMITER = (isCSV) ? ',' : ' ';

	char argStr[1024];
	char writeStr[2048];

	va_list ap;
	va_start(ap, format);
	vsprintf_s(argStr, format, ap);
	va_end(ap);

	// �������L�q����
	if (isWriteTime)
	{
		DATEDATA time;
		GetDateTime(&time);

		sprintf_s(writeStr, "[%d/%d %d:%d:%dsec]%c%s\n", time.Mon, time.Day, time.Hour, time.Min, time.Sec, DELIMITER, argStr);
	}
	else
	{
		sprintf_s(writeStr, "%s\n", argStr);
	}


	// �ʃX���b�h�Ŏg�p���Ă��Ȃ��^�C�~���O��҂��ď������ݗp������Ɏʂ�
	WaitForSingleObject(mutex, INFINITE);

	strcat_s(logStr, writeStr);

	// �������L�̕ϐ��g���ĂȂ�����Ď����Ă�����
	ReleaseMutex(mutex);
}

/// <summary>
/// �������݂��I������֐�
/// �i�������ēx�t�@�C������蒼�������Ƃ��͂��̊֐���ʂ��j
/// </summary>
void LogWriter::Exit()
{
	// �X���b�h�̃��[�v�t���O��|���ď������I���̂�҂�
	isRun = FALSE;
	WaitForSingleObject(thread, INFINITY);

	// �����ŃX���b�h�ƃ~���[�e�b�N�X�̉���i��������Ȃ��ƃ��[�N�̌����ɂȂ�j
	CloseHandle(thread);
	CloseHandle(mutex);

	memset(logFilePass, 0, sizeof(logFilePass));
	isCSV = FALSE;
	isWriteTime = TRUE;
	isInit = FALSE;
	return;
}


// ��LogWriter�N���X�̃����o�֐�

/// <summary>
/// �l�b�g���[�N�̏������֐�
/// </summary>
/// <param name="targetLog"> �������݂������O�N���X�̃|�C���^ </param>
/// <param name="ip"> �ʐM������IP�A�h���X </param>
int NetworkManager::Init(LogWriter* targetLog, IPDATA ip)
{
	log = targetLog;

	targetIp.d1 = ip.d1;
	targetIp.d2 = ip.d2;
	targetIp.d3 = ip.d3;
	targetIp.d4 = ip.d4;

	netHandleTcp = -1;

	return TRUE;
}

/// <summary>
/// �l�b�g���[�N�ł̂������J�n����֐�
/// </summary>
/// <param name="isListen"> �T�[�o�[�����N���C�A���g���� </param>
/// <returns>
///  1...����肵�Ă�r��
///  0...�ڑ����m���ł���
/// -1...�ڑ��Ɏ��s����
/// </returns>
int NetworkManager::StartConnection(int isListen)
{
	static int isFirst = TRUE;		// ���̊֐���ʂ����̂��ŏ���

	int returnVal = 1;				// �ŏI�I�Ȗ߂�l

	// �ŏ��ɂ��̊֐���ʂ������̏���
	if (isFirst)
	{
		if (isListen)
		{
			// �ڑ��҂���Ԃɂ���
			PreparationListenNetWork(TARGET_PORT_TCP);
		}

		isFirst = FALSE;
	}

	// �T�[�o�[���̏���
	if (isListen)
	{
		// �V�����ڑ����`�F�b�N
		netHandleTcp = GetNewAcceptNetWork();

		// �ڑ����G���[����Ȃ����
		if (netHandleTcp != -1)
		{
			// �ڑ��҂�������
			StopListenNetWork();

			// �ڑ��ł������肪�ʐM������IP�A�h���X�ƈ�v���Ă���ΐڑ�����
			IPDATA connectIp;
			GetNetWorkIP(netHandleTcp, &connectIp);

			if (memcmp(&targetIp, &connectIp, sizeof(IPDATA)) != 0)
			{
				MessageBox(NULL, "�ړI��IP�A�h���X����v���܂���ł����B", "Error", NULL);
				CloseNetWork(netHandleTcp);
				returnVal = -1;
			}
			else
			{
				returnVal = 0;
			}
		}
	}
	// �N���C�A���g���̏���
	else
	{
		// �ڑ������݂�
		netHandleTcp = ConnectNetWork(targetIp, TARGET_PORT_TCP);

		// �����Ɋւ��Ă͐ڑ��o�������o���Ȃ��������̓��
		if (netHandleTcp != -1)	returnVal = 0;
		else					returnVal = -1;
	}

	// ���������ɐ���or���s�����珉�����t���O��߂�
	if (returnVal != 1)
	{
		if (returnVal == 0)
		{
			netHandleUdp = MakeUDPSocket(TARGET_PORT_UDP);
			if (netHandleUdp == -1)
			{
				MessageBox(NULL, "UDP�\�P�b�g�̍쐬�Ɏ��s���܂����B", "Error", NULL);
				CloseNetWork(netHandleTcp);
				returnVal = -1;
			}
		}
		isFirst = TRUE;
	}

	return returnVal;
}

/// <summary>
/// ����Ƀf�[�^�𑗂�֐�
/// </summary>
/// <param name="buffer"> ���肽���f�[�^ </param>
/// <param name="sendSize"> ���肽���f�[�^�̃T�C�Y </param>
/// <returns>
///  0...����
/// -1...�G���[
/// �iDX���C�u������NetWorkSend�֐��Ɠ����߂�l�j
/// </returns>
int NetworkManager::SendDataTcp(const void* buffer, size_t sendSize)
{
	log->AddLogNet(buffer, sendSize, TRUE, TRUE);

	return NetWorkSend(netHandleTcp, buffer, sendSize);
}


/// <summary>
/// �f�[�^����M����֐�
/// </summary>
/// <param name="buffer"> �f�[�^���󂯎��o�b�t�@ </param>
/// <param name="recvSize"> �󂯎�肽���f�[�^�T�C�Y </param>
/// <param name="isPeek"> �󂯎�����f�[�^���f�[�^�o�b�t�@����폜���邩 </param>
/// <returns>
///  1...���󂯎��i�܂��v������f�[�^�T�C�Y���͂��Ă��Ȃ��j
///  0...�󂯎�芮��
/// -1...�G���[����
/// </returns>
int NetworkManager::ReceiveDataTcp(void* buffer, size_t recvSize, int isPeek)
{
	int returnVal = 1;		// �ŏI�I�Ȗ߂�l

	// �v������f�[�^�ȏオ�͂��Ă������M����
	if (GetNetWorkDataLength(netHandleTcp) >= recvSize)
	{
		if (isPeek)
		{
			returnVal = NetWorkRecvToPeek(netHandleTcp, buffer, recvSize);
		}
		else
		{
			returnVal = NetWorkRecv(netHandleTcp, buffer, recvSize);
		}
	}

	// �󂯎��Ă���΃��O�ɏ�������
	if (returnVal == 0)log->AddLogNet(buffer, recvSize, FALSE, TRUE);

	return returnVal;
}

int NetworkManager::ReceiveDataUdp(void* buffer, size_t recvSize, int isPeek)
{
	int returnVal = 1;

	if(CheckNetWorkRecvUDP(netHandleUdp) == TRUE)
	{
		IPDATA recvIp;
		int recvPort;

		int recvVal = NetWorkRecvUDP(netHandleUdp, &recvIp, &recvPort, buffer, recvSize, isPeek);

		if (recvVal <= 0 || recvIp.d4 != targetIp.d4)
		{
			returnVal = -1;
		}
		else
		{
			returnVal = 0;
			log->AddLogNet(buffer, recvSize, FALSE, FALSE);
		}
	}

	return returnVal;
}

int NetworkManager::SendDataUdp(const void* buffer, size_t sendSize)
{
	log->AddLogNet(buffer, sendSize, TRUE, FALSE);

	return NetWorkSendUDP(netHandleUdp, targetIp, TARGET_PORT_UDP, buffer, sendSize);
}

/// <summary>
/// �ڑ���ؒf����֐�
/// </summary>
void NetworkManager::DeleteConnection()
{
	DeleteUDPSocket(netHandleUdp);
	CloseNetWork(netHandleTcp);
}


/// <summary>
/// �ڑ����o���Ă��邩�m�F����֐�
/// </summary>
/// <returns> �ʐM�ł��Ă��邩�̐^�U�l </returns>
int NetworkManager::GetIsConnect()
{
	if (netHandleTcp == -1)return FALSE;
	else return GetNetWorkAcceptState(netHandleTcp);
}