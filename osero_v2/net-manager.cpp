#include "net-manager.h"
#include <stdarg.h>
#include <process.h>

// ↓LogWriterクラスのメンバ関数

/// <summary>
/// ログ管理クラスのコンストラクタ
/// </summary>
LogWriter::LogWriter()
{
	memset(logFilePass, 0, sizeof(logFilePass));
	isCSV = FALSE;
	isWriteTime = TRUE;
	isInit = FALSE;
}

/// <summary>
/// ログ初期化関数
/// </summary>
/// <param name="isWriteTime_"> 時間をログに書き込むかのフラグ </param>
/// <param name="isCSV_"> csv形式で書き込むかのフラグ </param>
/// <param name="filePass"> ログファイルを生成するファイルパス </param>
/// <returns> 初期化成功のフラグ </returns>
int LogWriter::Init(int isWriteTime_, int isCSV_, const char* filePass)
{
	// すでに初期化されていたらややこしくなるのでエラーにする
	if (isInit)
	{
		MessageBox(NULL, "ログファイルはすでに生成されています。", "えらぁ", NULL);
		return FALSE;
	}

	// 各値の初期化
	isWriteTime = isWriteTime_;
	isCSV = isCSV_;
	strcpy_s(logFilePass, sizeof(logFilePass), filePass);

	// CSV形式なら「,」、その他なら「　」で区切る
	const char DELIMITER = (isCSV) ? ',' : ' ';

	FILE* fp;

	// 書き込みモードでログテキストを生成
	if (fopen_s(&fp, logFilePass, "w") != 0)
	{
		MessageBox(NULL, "ログファイルの生成に失敗しました。", "えらぁ", NULL);
		return FALSE;
	}

	// ファイルが生成されたことをファイルに記述
	DATEDATA time;
	GetDateTime(&time);

	fprintf_s(fp, "[%d/%d %02d:%02d:%02dsec]%cログファイル生成\n", time.Mon, time.Day, time.Hour, time.Min, time.Sec, DELIMITER);

	fclose(fp);

	memset(logStr, 0, sizeof(logStr));

	// スレッド開始処理
	isRun = TRUE;

	// ミューテックスの生成
	// (ミューテックスに関する参考サイト) https://wa3.i-3-i.info/word13360.html
	mutex = CreateMutex(
		NULL,				// セキュリティ関係の指定ができるけど、使ったことないしわからんのでNULLで
		FALSE,				// このスレッドがミューテックスの所有者であるかどうかを示すフラグ（基本FALSE）
		NULL				// ミューテックスの名前（基本NULL、名前が重複するとエラーになる）
	);

	// スレッドの生成
	thread = (HANDLE)_beginthreadex(
		NULL,						// ミューテックスと同じくセキュリティ関係の記述ができる
		0,							// スレッドのスタック数。基本は0
		LogWriter::BeginThread,		// 呼び出したい関数のアドレス（unsigned int __stdcall (void*)の関数じゃないと受け入れない）
		this,						// 呼び出した関数の(void*)引数に渡す値、今回は自分自身を渡すのでthis
		NULL,						// 生成後すぐに実行するかのフラグ（0ですぐ実行）
		NULL						// スレッドの識別番号（基本NULL）
	);

	return TRUE;
}

/// <summary>
/// スレッド開始用の静的関数
/// </summary>
/// <param name="self"> スレッドを開始したいLogWriterクラス </param>
unsigned int __stdcall LogWriter::BeginThread(void* self)
{
	// ポインタをキャストして関数の開始
	reinterpret_cast<LogWriter*>(self)->WriteLog();
	return 0;
}


/// <summary>
/// 別スレッドで行いたいファイルに直接書き込む関数
/// </summary>
void LogWriter::WriteLog()
{
	char cpyStr[2048] = { 0 };		// logStrの退避用変数


	// スレッドが走っているかのフラグが立っていれば無限ループ
	// （二つ目の条件は書き込み漏れが無いようにしているだけ）
	while (isRun || logStr[0] != '\0')
	{
		if (logStr[0] == '\0')continue;

		// *** 共有変数へのアクセス開始 *** //

		WaitForSingleObject(mutex, INFINITE);

		// 文字列を退避させずにファイルに書き込むと処理が重くなるので退避
		strcpy_s(cpyStr, logStr);

		// 書き込んだ分配列を空にする
		memset(logStr, 0, sizeof(logStr));

		ReleaseMutex(mutex);

		// *** 共有変数へのアクセス終了 *** //


		FILE* fp;

		// 追記モードでログテキストを開く
		if (fopen_s(&fp, logFilePass, "a") != 0)
		{
			MessageBox(NULL, "ログファイルの書き込みに失敗しました。", "えらぁ", NULL);
			return;
		}

		fprintf_s(fp, "%s", cpyStr);

		fclose(fp);
	}
}


/// <summary>
/// ネットワークの送受信データをログに書き込む関数
/// </summary>
/// <param name="buffer"> 書き込むデータのポインタ </param>
/// <param name="bufSize"> 書き込むデータのサイズ </param>
/// <param name="isSend"> 送信側かのフラグ </param>
void LogWriter::AddLogNet(const void* buffer, size_t bufSize, int isSend, int isTCP)
{
	const char DELIMITER = (isCSV) ? ',' : ' ';

	char logStr[1024] = { 0 };		// ログに書き込む文字列

	// 1Byteずつ書き込むためにunsigned char型にキャストする
	const unsigned char* p = static_cast<const unsigned char*>(buffer);

	// 送信か受信か書き込む
	if (isSend)	sprintf_s(logStr, "[送信]%c", DELIMITER);
	else		sprintf_s(logStr, "[受信]%c", DELIMITER);

	if (isTCP)	strcat_s(logStr, "[TCP]");
	else		strcat_s(logStr, "[UDP]");

	// データを書き込んでいく
	for (int i = 0; i < bufSize; i++)
	{
		char addStr[4];
		sprintf_s(addStr, "%02X%c", p[i], DELIMITER);
		strcat_s(logStr, addStr);
	}

	// 出来た文字列をログ書き込み用文字列に追加
	AddLog(logStr);
}


/// <summary>
/// ログに文字列を追加する関数
/// </summary>
/// <param name="format"> printfと同じ書式付き文字列 </param>
void LogWriter::AddLog(const char* format, ...)
{
	const char DELIMITER = (isCSV) ? ',' : ' ';

	char argStr[1024];
	char writeStr[2048];

	va_list ap;
	va_start(ap, format);
	vsprintf_s(argStr, format, ap);
	va_end(ap);

	// 日時を記述する
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


	// 別スレッドで使用していないタイミングを待って書き込み用文字列に写す
	WaitForSingleObject(mutex, INFINITE);

	strcat_s(logStr, writeStr);

	// もう共有の変数使ってないよって示してあげる
	ReleaseMutex(mutex);
}

/// <summary>
/// 書き込みを終了する関数
/// （もしも再度ファイルを作り直したいときはこの関数を通す）
/// </summary>
void LogWriter::Exit()
{
	// スレッドのループフラグを倒して処理が終わるのを待つ
	isRun = FALSE;
	WaitForSingleObject(thread, INFINITY);

	// ここでスレッドとミューテックスの解放（これをしないとリークの原因になる）
	CloseHandle(thread);
	CloseHandle(mutex);

	memset(logFilePass, 0, sizeof(logFilePass));
	isCSV = FALSE;
	isWriteTime = TRUE;
	isInit = FALSE;
	return;
}


// ↓LogWriterクラスのメンバ関数

/// <summary>
/// ネットワークの初期化関数
/// </summary>
/// <param name="targetLog"> 書き込みたいログクラスのポインタ </param>
/// <param name="ip"> 通信したいIPアドレス </param>
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
/// ネットワークでのやり取りを開始する関数
/// </summary>
/// <param name="isListen"> サーバー側かクライアント側か </param>
/// <returns>
///  1...やり取りしてる途中
///  0...接続が確立できた
/// -1...接続に失敗した
/// </returns>
int NetworkManager::StartConnection(int isListen)
{
	static int isFirst = TRUE;		// この関数を通ったのが最初か

	int returnVal = 1;				// 最終的な戻り値

	// 最初にこの関数を通した時の処理
	if (isFirst)
	{
		if (isListen)
		{
			// 接続待ち状態にする
			PreparationListenNetWork(TARGET_PORT_TCP);
		}

		isFirst = FALSE;
	}

	// サーバー側の処理
	if (isListen)
	{
		// 新しい接続をチェック
		netHandleTcp = GetNewAcceptNetWork();

		// 接続がエラーじゃなければ
		if (netHandleTcp != -1)
		{
			// 接続待ちを解除
			StopListenNetWork();

			// 接続できた相手が通信したいIPアドレスと一致していれば接続成功
			IPDATA connectIp;
			GetNetWorkIP(netHandleTcp, &connectIp);

			if (memcmp(&targetIp, &connectIp, sizeof(IPDATA)) != 0)
			{
				MessageBox(NULL, "目的のIPアドレスが一致しませんでした。", "Error", NULL);
				CloseNetWork(netHandleTcp);
				returnVal = -1;
			}
			else
			{
				returnVal = 0;
			}
		}
	}
	// クライアント側の処理
	else
	{
		// 接続を試みる
		netHandleTcp = ConnectNetWork(targetIp, TARGET_PORT_TCP);

		// ここに関しては接続出来たか出来なかったかの二択
		if (netHandleTcp != -1)	returnVal = 0;
		else					returnVal = -1;
	}

	// もし処理に成功or失敗したら初期化フラグを戻す
	if (returnVal != 1)
	{
		if (returnVal == 0)
		{
			netHandleUdp = MakeUDPSocket(TARGET_PORT_UDP);
			if (netHandleUdp == -1)
			{
				MessageBox(NULL, "UDPソケットの作成に失敗しました。", "Error", NULL);
				CloseNetWork(netHandleTcp);
				returnVal = -1;
			}
		}
		isFirst = TRUE;
	}

	return returnVal;
}

/// <summary>
/// 相手にデータを送る関数
/// </summary>
/// <param name="buffer"> 送りたいデータ </param>
/// <param name="sendSize"> 送りたいデータのサイズ </param>
/// <returns>
///  0...成功
/// -1...エラー
/// （DXライブラリのNetWorkSend関数と同じ戻り値）
/// </returns>
int NetworkManager::SendDataTcp(const void* buffer, size_t sendSize)
{
	log->AddLogNet(buffer, sendSize, TRUE, TRUE);

	return NetWorkSend(netHandleTcp, buffer, sendSize);
}


/// <summary>
/// データを受信する関数
/// </summary>
/// <param name="buffer"> データを受け取るバッファ </param>
/// <param name="recvSize"> 受け取りたいデータサイズ </param>
/// <param name="isPeek"> 受け取ったデータをデータバッファから削除するか </param>
/// <returns>
///  1...未受け取り（まだ要求するデータサイズが届いていない）
///  0...受け取り完了
/// -1...エラー発生
/// </returns>
int NetworkManager::ReceiveDataTcp(void* buffer, size_t recvSize, int isPeek)
{
	int returnVal = 1;		// 最終的な戻り値

	// 要求するデータ以上が届いていたら受信処理
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

	// 受け取れていればログに書き込む
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
/// 接続を切断する関数
/// </summary>
void NetworkManager::DeleteConnection()
{
	DeleteUDPSocket(netHandleUdp);
	CloseNetWork(netHandleTcp);
}


/// <summary>
/// 接続が出来ているか確認する関数
/// </summary>
/// <returns> 通信できているかの真偽値 </returns>
int NetworkManager::GetIsConnect()
{
	if (netHandleTcp == -1)return FALSE;
	else return GetNetWorkAcceptState(netHandleTcp);
}