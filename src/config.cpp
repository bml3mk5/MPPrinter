/// @file config.cpp
///
/// @brief config
///
#include "config.h"
#include <wx/filename.h>
#include <wx/fileconf.h>

Config gConfig;

const int cZoomMags[] = {25, 50, 75, 100, 150, 200, 300, 400};

//
//
//

Config::Config()
{
	ini_file = _T("");

	// default value
	mFilePath = _T("");
	mMemoryCash = false;
	mServerHost = _T("localhost");
	mServerPort = 10200;
	mSendAck = false;
	mPrintDelay = 16;
	mDataReverse = true;
	mEchoBack = false;
	mMagnify = 1;
	mRotate = 0;
	mDensity = COLOR_DENSITY_MAX;
}

Config::~Config()
{
}

/// iniファイルパスをセット
/// @param[in] file iniファイルパス 
void Config::SetFileName(const wxString &file)
{
	ini_file = file;
}

/// iniファイルからロード
void Config::Load()
{
	if (ini_file.IsEmpty()) return;

	// load ini file
	wxFileConfig *ini = new wxFileConfig(wxEmptyString,wxEmptyString,ini_file,wxEmptyString
		,wxCONFIG_USE_LOCAL_FILE | wxCONFIG_USE_RELATIVE_PATH | wxCONFIG_USE_NO_ESCAPE_CHARACTERS);

	int i;
	long l;
	wxString s;
	ini->Read(_T("Path"), &mFilePath, mFilePath);
	ini->Read(_T("MemoryCash"), &mMemoryCash, mMemoryCash);
	ini->Read(_T("ServerHost"), &mServerHost, mServerHost);
	ini->Read(_T("ServerPort"), &l, mServerPort);
	if (0 < l && l <= 65535) mServerPort = l;
	ini->Read(_T("PrintDelay"), &i, mPrintDelay);
	if (0 <= i && i <= 10000) mPrintDelay = i; 
	ini->Read(_T("SendAck"), &mSendAck, mSendAck);
	ini->Read(_T("Density"), &i, mDensity);
	if (0 <= i && i <= COLOR_DENSITY_MAX) mDensity = i; 
	ini->Read(_T("DataReversed"), &mDataReverse, mDataReverse);
	ini->Read(_T("EchoBack"), &mEchoBack, mEchoBack);
	ini->Read(_T("Magnify"), &i, mMagnify);
	if (0 <= i && i < GetMaxMagnify()) mMagnify = i; 
	ini->Read(_T("Rotate"), &i, mRotate);
	if (0 <= i && i <= 3) mRotate = i; 
	for(int i=0; i<MAX_RECENT_FILES; i++) {
		wxString sval;
		ini->Read(wxString::Format(_T("Recent%d"), i), &sval);
		if (!sval.IsEmpty()) {
			mRecentFiles.Add(sval);
		}
	}
	delete ini;
}

/// iniファイルからロード
/// @param[in] file iniファイルパス 
void Config::Load(const wxString &file)
{
	SetFileName(file);
	Load();
}

/// iniファイルに保存
void Config::Save()
{
	if (ini_file.IsEmpty()) return;

	// save ini file
	wxFileConfig *ini = new wxFileConfig(wxEmptyString,wxEmptyString,ini_file,wxEmptyString
		,wxCONFIG_USE_LOCAL_FILE | wxCONFIG_USE_RELATIVE_PATH | wxCONFIG_USE_NO_ESCAPE_CHARACTERS);
	ini->Write(_T("Path"), mFilePath);
	ini->Write(_T("MemoryCash"), mMemoryCash);
	ini->Write(_T("ServerHost"), mServerHost);
	ini->Write(_T("ServerPort"), mServerPort);
	ini->Write(_T("PrintDelay"), mPrintDelay);
	ini->Write(_T("SendAck"), mSendAck);
	ini->Write(_T("Density"), mDensity);
	ini->Write(_T("DataReversed"), mDataReverse);
	ini->Write(_T("EchoBack"), mEchoBack);
	ini->Write(_T("Magnify"), mMagnify);
	ini->Write(_T("Rotate"), mRotate);
	for(int i=0,row=0; row<MAX_RECENT_FILES && i<(int)mRecentFiles.Count(); i++) {
		wxString sval = mRecentFiles.Item(i);
		if (sval.IsEmpty()) continue;
		ini->Write(wxString::Format(_T("Recent%d"), row), sval);
		row++;
	}
	// write
	delete ini;
}

/// 最近使用したファイルを追加
/// @param[in] val ファイルパス
void Config::AddRecentFile(const wxString &val)
{
	wxFileName fpath = wxFileName::FileName(val);
	mFilePath = fpath.GetPath(wxPATH_GET_SEPARATOR);
	// 同じファイルがあるか
	int pos = mRecentFiles.Index(fpath.GetFullPath());
	if (pos >= 0) {
		// 消す
		mRecentFiles.RemoveAt(pos);
	}
	// 追加
	mRecentFiles.Insert(fpath.GetFullPath(), 0);
	// 10を超える分は消す
	if (mRecentFiles.Count() > MAX_RECENT_FILES) {
		mRecentFiles.RemoveAt(MAX_RECENT_FILES);
	}
}

/// 最近使用したファイルを返す
wxString &Config::GetRecentFile()
{
	return mRecentFiles[0];
}

/// 最近使用したファイル一覧を得る
/// @param[out] vals 一覧
void Config::GetRecentFiles(wxArrayString &vals)
{
	vals = mRecentFiles;
}

/// 倍率リストの数を返す
int Config::GetMaxMagnify()
{
	return (int)(sizeof(cZoomMags)/sizeof(cZoomMags[0]));
}
