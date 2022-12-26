/// @file main.h
///
/// @brief MPPRINTER メイン
///
/// @date 2013.11.27 Create
///
/// @author Copyright (c) 2013, Sasaji. All rights reserved.
///
#ifndef _MPPRINTER_H_
#define _MPPRINTER_H_

#include "common.h"
#include <wx/wx.h>
#include <wx/dynarray.h>
#include <wx/dnd.h>
#include <wx/fontdlg.h>
#include "config.h"
#include "mp_printer.h"
#include "ctrl_setting.h"
#include "server.h"
#include "server_setting.h"

class MpPrinterApp;
class MpPrinterFrame;
class MpPrinterPanel;
class MpPrinterFileDialog;
class MpPrinterFileDropTarget;
class MpPrinterServer;

#define SCROLLBAR_UNIT	2

/// @brief アプリメインクラス
class MpPrinterApp: public wxApp
{
private:
	wxString app_path;
	wxString ini_path;
	wxString res_path;
	wxLocale mLocale;

	void SetAppPath();
public:
	MpPrinterApp() : mLocale(wxLANGUAGE_DEFAULT) {}
	bool OnInit();
	int  OnExit();
	const wxString &GetAppPath();
	const wxString &GetIniPath();
	const wxString &GetResPath();
};

DECLARE_APP(MpPrinterApp)

/// @brief メインウィンドウクラス
class MpPrinterFrame: public wxFrame
{
private:
	// gui
	wxMenu *menuFile;
	wxMenu *menuRecentFiles;
	wxMenu *menuView;
	wxMenu *menuControl;
	wxMenu *menuOther;
	wxMenu *menuHelp;
	MpPrinterPanel *panel;

	MP_PRINTER *mpri;

	CtrlSettingBox *cfgbox;

	wxString mFilePath;

	bool mMenuEnable;

	MpPrinterServer *server;

	wxTimer mTimer;

public:

    MpPrinterFrame(const wxSize& size);
	~MpPrinterFrame();

	bool Init();

	// event procedures
	void OnQuit(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);

	void OnOpenFile(wxCommandEvent& event);
	void OnCloseFile(wxCommandEvent& event);
	void OnOpenRecentFile(wxCommandEvent& event);

	void OnPageSetup(wxCommandEvent& event);
	void OnMarginSetup(wxCommandEvent& event);
	void OnPrintPreview(wxCommandEvent& event);
	void OnPrint(wxCommandEvent& event);

	void OnZoomIn(wxCommandEvent& event);
	void OnZoomOut(wxCommandEvent& event);
	void OnDensity(wxCommandEvent& event);
	void OnRotate(wxCommandEvent& event);
	void OnInitScreen(wxCommandEvent& event);
	void OnRefresh(wxCommandEvent& event);

	void OnChangeType(wxCommandEvent& event);
	void OnReset(wxCommandEvent& event);
	void OnSettings(wxCommandEvent& event);

	void OnServer(wxCommandEvent& event);
	void OnServerReverse(wxCommandEvent& event);
	void OnServerSettings(wxCommandEvent& event);

	void OnTimer(wxTimerEvent& event);

#ifdef _DEBUG_LOG
	void OnDebugLog(wxCommandEvent& event);
#endif

	// functions
	void OpenDroppedFile(wxString &path);
	void OpenDataFile(wxString &path);
	void CloseDataFile();
	void EnableMenuItems(bool enable);
	void UpdateMenuRecentFiles();
	void ZoomIn();
	void ZoomOut();
	void SetFocus();
	void Rotate(int direction);
	void InitScreen();
	void Refresh();

	bool StartServer();
	void StopServer();
	void SetData(const wxByte* data, int size);

	void SetFrameTitle();

	// properties
	MpPrinterPanel *GetMpPrinterPanel() { return panel; }
	MP_PRINTER *GetMpPrinter() { return mpri; }

	enum
	{
		// menu id
		IDM_EXIT = 1,
		IDM_DEBUGLOG,
		IDM_OPEN_FILE,
		IDM_CLOSE_FILE,
		IDM_PAGE_SETUP,
		IDM_MARGIN_SETUP,
		IDM_PRINT_PREVIEW,
		IDM_PRINT,
		IDM_ZOOM_IN,
		IDM_ZOOM_OUT,
		IDM_DENSITY,
		IDM_ROTATE_LEFT,
		IDM_ROTATE_RIGHT,
		IDM_INIT_SCREEN,
		IDM_REFRESH,
		IDM_HI_TYPE,
		IDM_EP_TYPE,
		IDM_RESET,
		IDM_SETTINGS,
		IDM_SERVER,
		IDM_SERVER_REVERSE,
		IDM_SERVER_SETTINGS,

		IDM_RECENT_FILE_0,
		IDM_RECENT_FILE_1,
		IDM_RECENT_FILE_2,
		IDM_RECENT_FILE_3,
		IDM_RECENT_FILE_4,
		IDM_RECENT_FILE_5,
		IDM_RECENT_FILE_6,
		IDM_RECENT_FILE_7,
		IDM_RECENT_FILE_8,
		IDM_RECENT_FILE_9,

		IDD_CONFIGBOX,
		IDD_SERVERBOX,

		IDT_TIMER,
	};

	DECLARE_EVENT_TABLE()
};

/// @brief メインパネルクラス
class MpPrinterPanel: public wxScrolledWindow
{
private:
	MpPrinterFrame *frame;

	wxPoint mMouseDownPoint;
	int  mMouseWheelEvent;
	bool ctrl_down;
	bool left_down;
	bool mDrawPart;

	wxCursor *curMag;
	wxCursor *curHand;

public:
	MpPrinterPanel(MpPrinterFrame *parent);
	~MpPrinterPanel();

	// event procedures
	void OnDraw(wxDC &dc);
//	void OnPaint(wxPaintEvent& event);
	void OnSize(wxSizeEvent& event);
	void OnMouseLeftDown(wxMouseEvent& event);
	void OnMouseLeftUp(wxMouseEvent& event);
	void OnMouseRightDown(wxMouseEvent& event);
	void OnMouseMove(wxMouseEvent& event);
	void OnMouseCaptureLost(wxMouseCaptureLostEvent& event);
	void OnMouseWheel(wxMouseEvent& event);
	void OnKeyDown(wxKeyEvent& event);
	void OnKeyUp(wxKeyEvent& event);
	void OnScrollWin(wxScrollWinEvent& event);

	// functions
    void Refresh(bool eraseBackground = true, const wxRect *rect = (const wxRect *)NULL);
	void RefreshPanel(bool eraseBackground = true, bool drawPart = false);
	void ChangeCursor();
	void SetScrollBarPos(int new_ux, int new_uy, int new_px, int new_py);
	void ZoomScrollBarPos(int mag);
	void ScrollArea(int x, int y);
	wxCursor *CreateCursor(const char *bits, const char *mask);

	// properties

//	enum {
//	};

	DECLARE_EVENT_TABLE()
};

/// @brief ファイルダイアログ
class MpPrinterFileDialog: public wxFileDialog
{
public:
	MpPrinterFileDialog(const wxString& message, const wxString& defaultDir = wxEmptyString, const wxString& defaultFile = wxEmptyString, const wxString& wildcard = wxFileSelectorDefaultWildcardStr, long style = wxFD_DEFAULT_STYLE);
};

/// @brief ドロップ
class MpPrinterFileDropTarget : public wxFileDropTarget
{
private:
    MpPrinterFrame *frame;
public:
    MpPrinterFileDropTarget(MpPrinterFrame *parent);
    bool OnDropFiles(wxCoord x, wxCoord y ,const wxArrayString &filenames);
};

/// @brief Aboutダイアログ
class MpPrinterAbout : public wxDialog
{
public:
	MpPrinterAbout(wxWindow* parent, wxWindowID id);
};

#endif /* _MPPRINTER_H_ */

