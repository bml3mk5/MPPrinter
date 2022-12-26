/// @file errorinfo.h
///
/// errorinfo.h
///
#ifndef _ERRORINFO_H_
#define _ERRORINFO_H_

#include "common.h"
#include <wx/wx.h>
#include "debugreport.h"

/// エラータイプ
typedef enum enumMyErrType {
	myCancel = -1,
	myOK = 0,
	myError,
	myWarning
} MyErrType;

/// エラーコード
typedef enum enumMyErrCode {
	myErrNone = 0,
	myErrFileNotFound,
	myErrCannotWrite,
	myErrFileEmpty,
	myErrSameFile,
	myErrFontFile,
	myErrPrinting,
	myErrPreview,
	myErrServer,
	myErrConnect,
	myErrSystem,
	myErrUnknown = 9999
} MyErrCode;

/// エラー情報保存用
class MyErrInfo
{
private:
	MyErrType mType;
	MyErrCode mCode;
	wxString  mMsg;
	int       mLine;
	const char *mFile;

public:
	MyErrInfo();
	~MyErrInfo();

	// エラーメッセージ
	wxString ErrMsg(MyErrCode code);
	// エラー情報セット
	void SetInfo(const char *file, int line, MyErrType type, MyErrCode code, const wxString &msg = wxEmptyString);
	// エラー情報セット
	void SetInfo(const char *file, int line, MyErrType type, MyErrCode code1, MyErrCode code2, const wxString &msg = wxEmptyString);

	// gui メッセージBOX
	void ShowMsgBox(wxWindow *win = 0);

};

extern MyErrInfo gErrInfo;

#endif /* _ERRORINFO_H_ */
