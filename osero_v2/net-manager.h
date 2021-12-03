#pragma once

#include "main.h"

/// <summary>
/// ログファイルを生成し、書き込みを行うクラス
/// </summary>
class LogWriter
{
private:
	char logFilePass[128];			// ログファイルのパス
	int isCSV;						// csv形式で保存するかのフラグ
	int isWriteTime;				// 時間までログに記述するかのフラグ
	int isInit;						// 初期化が済んでいるかのフラグ
	char logStr[2048];
	HANDLE thread;
	HANDLE mutex;
	BOOL isRun;

	static unsigned int __stdcall BeginThread(void* self);
	void WriteLog();

public:
	LogWriter();

	// 初期化関数
	int Init(int isWriteTime = TRUE, int isCSV = FALSE, const char* filePass = "MyLog.txt");

	// ログファイルに文字列を書き込む関数
	void AddLog(const char* format, ...);

	// ネットワークでやりとりしたデータをバイナリ形式で書き込む関数
	void AddLogNet(const void* buffer, size_t bufSize, int isSendData, int isTCP);

	// ログファイルへの書き込みを終了する関数 
	void Exit();
};

/// <summary>
/// DXライブラリのネットワーク系関数をラップしたクラス
/// （1つのIPアドレスに対して1つのインスタンスを作る）
/// </summary>
class NetworkManager
{
private:
	int netHandleTcp;			// ネットワークハンドル(TCP用)
	int netHandleUdp;			// ネットワークハンドル(UDP用)
	IPDATA targetIp;			// やり取りしたいIPアドレス先
	LogWriter* log;				// 書き込むログクラスへのポインタ
	//int connectStatus;		// 接続状況

public:
	// 初期化関数
	int Init(LogWriter* writeLog, IPDATA targetIp);

	// 接続を確立する関数
	int StartConnection(int isListen);

	// データを送信する関数
	int SendDataTcp(const void* buffer, size_t sendLength);

	// データを受信する関数
	int ReceiveDataTcp(void* buffer, size_t targetLength, int isPeek = FALSE);

	// データを送信する関数
	int SendDataUdp(const void* buffer, size_t sendLength);

	// データを受信する関数
	int ReceiveDataUdp(void* buffer, size_t targetLength, int isPeek = FALSE);

	// 接続を破棄する関数
	void DeleteConnection();

	// 接続できているかを確認する関数
	int GetIsConnect();
};