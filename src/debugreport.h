///
/// Debugging
///
#ifndef _DEBUGREPORT_H_
#define _DEBUGREPORT_H_

#ifdef _DEBUG_LOG

#include "wx/wx.h"
#include "wx/ffile.h"
#include "wx/log.h"

class MyDebugReport
{
private:
	wxString mFileName;
	wxFFile *mFile;

	wxLogWindow *mLogWin;
	wxLogRecordInfo mLogInfo;
public:
	MyDebugReport();
	~MyDebugReport();
	void SetFile(const wxString &filename);
	void AddText(const wxString &msg, const char *file, int line);
	void SetInfo(const char *file, int line);
	void Show(wxWindow *win);
};

extern MyDebugReport gDebugReport;

#define AddDebugLog3(x, file, line)  gDebugReport.AddText(x, file, line)
#define AddDebugLog(x)  gDebugReport.AddText(x, __FILE__, __LINE__)

#else

#define AddDebugLog3(x, file, line)
#define AddDebugLog(x)

#endif
#endif /* _DEBUGREPORT_H_ */
