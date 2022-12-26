///
/// エラー情報
///
#include "errorinfo.h"

MyErrInfo gErrInfo;

MyErrInfo::MyErrInfo()
{
	mType = myOK;
	mCode = myErrNone;
	mMsg = _T("");
	mLine = 0;
}

MyErrInfo::~MyErrInfo()
{
}

// エラーメッセージ
wxString MyErrInfo::ErrMsg(MyErrCode code)
{
	wxString str;
	switch(code) {
		case myErrNone:
			// no error
			str = _T("");
			break;
		case myErrFileNotFound:
			// ファイルがみつかりません。
			str = _("File not found.");
			break;
		case myErrCannotWrite:
			// ファイルを出力できません。
			str = _("Cannot write file.");
			break;
		case myErrFileEmpty:
			// ファイルが空です。
			str = _("File is empty.");
			break;
		case myErrSameFile:
			// 同じファイルを指定することはできません。
			str = _("Cannot specify the same file.");
			break;
		case myErrFontFile:
			// フォントファイルをプログラムと同じフォルダに置いてください。
			str = _("Put the font file on the program folder.");
			break;
		case myErrPrinting:
			// 印刷に失敗しました。
			str = _("Printing failed.");
			break;
		case myErrPreview:
			// プレビューできません。プリンターの設定を確認してください。
			str = _("Cannot preview. Please check your printer setting.");
			break;
		case myErrServer:
			// サーバーの起動ができません。
			str = _("Cannot start server.");
			break;
		case myErrConnect:
			// 接続に失敗しました。
			str = _("Connection failed.");
			break;
		case myErrSystem:
			// システムエラーが発生しました。
			str = _("System error occured.");
			break;
		default:
			// 不明なエラー: %d
			str.Printf(_("Unknown error: %d"), code);
			break;
	}
	return str;
}

// エラー情報セット
void MyErrInfo::SetInfo(const char *file, int line, MyErrType type, MyErrCode code, const wxString &msg)
{
		mType = type;
		mCode = code;
		mMsg = ErrMsg(code);
		if (!msg.IsEmpty()) {
			mMsg += _T(" (") + msg + _T(")");
		}
		mLine = line;
		mFile = file;
}

void MyErrInfo::SetInfo(const char *file, int line, MyErrType type, MyErrCode code1, MyErrCode code2, const wxString &msg)
{
		mType = type;
		mCode = code1;
		mMsg = ErrMsg(code1);
		if (!msg.IsEmpty()) {
			mMsg += _T(" (") + msg + _T(")");
		}
		mMsg += _T("\n") + ErrMsg(code2);
		mLine = line;
		mFile = file;
}

// gui メッセージBOX
void MyErrInfo::ShowMsgBox(wxWindow *win)
{
	switch(mType) {
		case myError:
			AddDebugLog3(_("Error") + _T(":") + mMsg, mFile, mLine);
			wxMessageBox(mMsg, _("Error"), wxOK | wxICON_ERROR, win);
			break;
		case myWarning:
			AddDebugLog3(_("Warning") + _T(":") + mMsg, mFile, mLine);
			wxMessageBox(mMsg, _("Warning"), wxOK | wxICON_WARNING, win);
			break;
		default:
			break;
	}
}
