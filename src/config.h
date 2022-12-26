/// @file config.h
///
/// config.h
///
#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "common.h"
#include <wx/wx.h>

#define MAX_RECENT_FILES 10

extern const int cZoomMags[];

/// @brief 設定
class Config
{
private:
	wxString ini_file;			///< iniファイルパス

	wxString mFilePath;			///< ファイルパス
	bool     mMemoryCash;		///< メモリキャッシュを使用するか
	wxString mServerHost;		///< サーバアドレス
	long     mServerPort;		///< サーバポート
	bool     mSendAck;			///< ACK(0x06)を送信するか
	int      mPrintDelay;		///< 印刷遅延
	int      mDensity;			///< 印刷濃度
	bool     mDataReverse;		///< データ反転
	bool     mEchoBack;			///< エコーバックするか
	int      mMagnify;			///< 倍率
	int      mRotate;			///< 回転表示
	wxArrayString mRecentFiles;	///< 最近使用したファイル一覧

public:
	Config();
	~Config();
	/// @brief iniファイルパスをセット
	void SetFileName(const wxString &file);
	/// @brief iniファイルからロード
	void Load(const wxString &file);
	/// @brief iniファイルからロード
	void Load();
	/// @brief iniファイルに保存
	void Save();

	/// @brief ファイルパスをセット
	void SetFilePath(const wxString &val) { mFilePath = val; }
	/// @brief ファイルパスを返す
	const wxString &GetFilePath() const { return mFilePath; }
	/// @brief メモリキャッシュを使用するかをセット
	void SetMemoryCash(bool val) { mMemoryCash = val; }
	/// @brief メモリキャッシュを使用するかを返す
	bool GetMemoryCash() const { return mMemoryCash; }
	/// @brief サーバアドレスをセット
	void SetServerHost(const wxString &val) { mServerHost = val; }
	/// @brief サーバアドレスを返す
	const wxString &GetServerHost() const { return mServerHost; }
	/// @brief サーバポートをセット
	void SetServerPort(long val) { mServerPort = val; }
	/// @brief サーバポートを返す
	long GetServerPort() const { return mServerPort; }
	/// @brief ACK送信するかをセット
	void SetSendAck(bool val) { mSendAck = val; }
	/// @brief ACK送信するかを返す
	bool GetSendAck() const { return mSendAck; }
	/// @brief 印刷遅延をセット
	void SetPrintDelay(int val) { mPrintDelay = val; }
	/// @brief 印刷遅延を返す
	int GetPrintDelay() const { return mPrintDelay; }
	/// @brief 印刷濃度をセット
	void SetDensity(int val) { mDensity = val; }
	/// @brief 印刷濃度を返す
	int GetDensity() const { return mDensity; }
	/// @brief データ反転をセット
	void SetDataReverse(bool val) { mDataReverse = val; }
	/// @brief データ反転を返す
	bool GetDataReverse() const { return mDataReverse; }
	/// @brief データ反転をセット
	void SetEchoBack(bool val) { mEchoBack = val; }
	/// @brief データ反転を返す
	bool GetEchoBack() const { return mEchoBack; }
	/// @brief 倍率をセット
	void SetMagnify(int val) { mMagnify = val; }
	/// @brief 倍率を返す
	int GetMagnify() const { return mMagnify; }
	/// @brief 倍率リストの数を返す
	static int GetMaxMagnify();
	/// @brief 回転表示をセット
	void SetRotate(int val) { mRotate = val; }
	/// @brief 回転表示を返す
	int GetRotate() const { return mRotate; }
	/// @brief 最近使用したファイルを追加
	void AddRecentFile(const wxString &val);
	/// @brief 最近使用したファイルを返す
	wxString &GetRecentFile();
	/// @brief 最近使用したファイル一覧を得る
	void GetRecentFiles(wxArrayString &vals);
};

extern Config gConfig;

#endif /* _CONFIG_H_ */
