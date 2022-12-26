///
/// Debugging
///
#ifdef _DEBUG_LOG

#include "debugreport.h"

MyDebugReport gDebugReport;

MyDebugReport::MyDebugReport() {
	mFile = NULL;
	mLogWin = NULL;
}
MyDebugReport::~MyDebugReport() {
	if (mFile != NULL) {
		mFile->Close();
		delete mFile;
	}
	delete wxLog::SetActiveTarget(mLogWin);
}
void MyDebugReport::SetFile(const wxString &filename) {
	mFileName = filename;
	mFile = new wxFFile(mFileName, wxT("w"));
	mLogWin = new wxLogWindow(NULL, _T("debug log"), false);
}
void MyDebugReport::AddText(const wxString &msg, const char *file, int line) {
	SetInfo(file, line);
	if (mFile != NULL) {
		mFile->Write(msg);
		mFile->Write(wxT("\n"));
		mFile->Flush();
	}
	if (mLogWin != NULL) {
		mLogWin->LogRecord(wxLOG_Debug, msg, mLogInfo);
		mLogWin->Flush();
	}
}
void MyDebugReport::SetInfo(const char *file, int line) {
	mLogInfo.filename = file;
	mLogInfo.line = line;
	mLogInfo.func = NULL;
	mLogInfo.timestamp = time(NULL);
	mLogInfo.threadId = wxThread::GetCurrentId();
}
void MyDebugReport::Show(wxWindow *win) {
	if (mLogWin != NULL) {
		mLogWin->Show(true);
	}
}

#endif
