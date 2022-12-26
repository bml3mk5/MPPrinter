/// @file main.cpp
///
/// @brief MPPRINTER メイン
///
/// @date 2013.11.27 Create
///
/// @author Copyright (c) 2013, Sasaji. All rights reserved.
///
#include "main.h"
#include <wx/filename.h>
#include <wx/fontenum.h>
#include "errorinfo.h"
#include "densitybox.h"
#include "res/mpprinter.xpm"
#include "res/magnify.xbm"
#include "res/magnify_mask.xbm"
#include "res/hand.xbm"
#include "res/hand_mask.xbm"


IMPLEMENT_APP(MpPrinterApp)

bool MpPrinterApp::OnInit()
{
	SetAppPath();
	SetAppName(_T("mpprinter"));

#ifdef _DEBUG_LOG
	// set debug log
	gDebugReport.SetFile(ini_path + GetAppName() + _T(".log"));
#endif

	// load ini file
	gConfig.Load(ini_path + GetAppName() + _T(".ini"));

	// set locale search path and catalog name
	mLocale.AddCatalogLookupPathPrefix(res_path + _T("lang"));
	mLocale.AddCatalogLookupPathPrefix(_T("lang"));
	mLocale.AddCatalog(_T("mpprinter"));

	if (!wxApp::OnInit()) {
		return false;
	}

	MpPrinterFrame *frame = new MpPrinterFrame(wxSize(800, 600));
	frame->Show(true);
	SetTopWindow(frame);

	if (!frame->Init()) {
		return false;
	}

	return true;
}

int MpPrinterApp::OnExit()
{
	// save ini file
	gConfig.Save();

	return 0;
}

void MpPrinterApp::SetAppPath()
{
	app_path = wxFileName::FileName(argv[0]).GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR);
#ifdef __WXOSX__
	if (app_path.Find(_T("MacOS")) >= 0) {
		wxFileName file = wxFileName::FileName(app_path+"../../../");
		file.Normalize();
		ini_path = file.GetPath(wxPATH_GET_SEPARATOR);
		file = wxFileName::FileName(app_path+"../../Contents/Resources/");
		file.Normalize();
		res_path = file.GetPath(wxPATH_GET_SEPARATOR);
	} else
#endif
	{
		ini_path = app_path;
		res_path = app_path;
	}
}

const wxString &MpPrinterApp::GetAppPath()
{
	return app_path;
}

const wxString &MpPrinterApp::GetIniPath()
{
	return ini_path;
}

const wxString &MpPrinterApp::GetResPath()
{
	return res_path;
}

//
// Frame
//
// Attach Event
BEGIN_EVENT_TABLE(MpPrinterFrame, wxFrame)
	// menu event
	EVT_MENU(wxID_EXIT, MpPrinterFrame::OnQuit)
	EVT_MENU(wxID_ABOUT, MpPrinterFrame::OnAbout)

	EVT_MENU(IDM_OPEN_FILE,  MpPrinterFrame::OnOpenFile)
	EVT_MENU(IDM_CLOSE_FILE, MpPrinterFrame::OnCloseFile)

	EVT_MENU_RANGE(IDM_RECENT_FILE_0, IDM_RECENT_FILE_9, MpPrinterFrame::OnOpenRecentFile)

	EVT_MENU(IDM_PAGE_SETUP,    MpPrinterFrame::OnPageSetup)
	EVT_MENU(IDM_MARGIN_SETUP, MpPrinterFrame::OnMarginSetup)
	EVT_MENU(IDM_PRINT_PREVIEW, MpPrinterFrame::OnPrintPreview)
	EVT_MENU(IDM_PRINT,         MpPrinterFrame::OnPrint)

	EVT_MENU(IDM_ZOOM_IN,  MpPrinterFrame::OnZoomIn)
	EVT_MENU(IDM_ZOOM_OUT, MpPrinterFrame::OnZoomOut)
	EVT_MENU(IDM_DENSITY,  MpPrinterFrame::OnDensity)
	EVT_MENU(IDM_ROTATE_LEFT,  MpPrinterFrame::OnRotate)
	EVT_MENU(IDM_ROTATE_RIGHT, MpPrinterFrame::OnRotate)
	EVT_MENU(IDM_INIT_SCREEN,  MpPrinterFrame::OnInitScreen)
	EVT_MENU(IDM_REFRESH,  MpPrinterFrame::OnRefresh)

	EVT_MENU(IDM_HI_TYPE, MpPrinterFrame::OnChangeType)
	EVT_MENU(IDM_EP_TYPE, MpPrinterFrame::OnChangeType)

	EVT_MENU(IDM_RESET,    MpPrinterFrame::OnReset)
	EVT_MENU(IDM_SETTINGS, MpPrinterFrame::OnSettings)

	EVT_MENU(IDM_SERVER, MpPrinterFrame::OnServer)
	EVT_MENU(IDM_SERVER_REVERSE, MpPrinterFrame::OnServerReverse)
	EVT_MENU(IDM_SERVER_SETTINGS, MpPrinterFrame::OnServerSettings)
#ifdef _DEBUG_LOG
	EVT_MENU(IDM_DEBUGLOG, MpPrinterFrame::OnDebugLog)
#endif

	EVT_TIMER(IDT_TIMER, MpPrinterFrame::OnTimer)
END_EVENT_TABLE()

// 翻訳用
#define APPLE_MENU_STRING _TX("Hide mpprinter"),_TX("Hide Others"),_TX("Show All"),_TX("Quit mpprinter"),_TX("Services"),_TX("Preferences…")
#define DIALOG_STRING _TX("OK"),_TX("Cancel")

MpPrinterFrame::MpPrinterFrame(const wxSize& size)
       : wxFrame(NULL, wxID_ANY, wxEmptyString, wxDefaultPosition, size)
{
	// init
	mpri = NULL;
	panel = NULL;
	server = NULL;

	// icon
#ifdef __WXMSW__
	SetIcon(wxIcon(_T("mpprinter")));
#elif defined(__WXGTK__) || defined(__WXMOTIF__)
	SetIcon(wxIcon(mpprinter_xpm));
#endif

	// menu
	menuFile = new wxMenu;
	menuView = new wxMenu;
	menuControl = new wxMenu;
	menuOther = new wxMenu;
	menuHelp = new wxMenu;
//	wxMenuItem *mitm = NULL;

	// file menu
	menuFile->Append( IDM_OPEN_FILE, _("&Open...") );
	menuFile->Append( IDM_CLOSE_FILE, _("&Close") );
	menuFile->AppendSeparator();
	menuFile->Append( IDM_PAGE_SETUP, _("Page Set&up...") );
	menuFile->Append( IDM_MARGIN_SETUP, _("&Margin Setup...") );
	menuFile->Append( IDM_DENSITY, _("Color &Density...") );
	menuFile->AppendSeparator();
	menuFile->Append( IDM_PRINT_PREVIEW, _("Print Pre&view") );
	menuFile->Append( IDM_PRINT, _("&Print...") );
	menuFile->AppendSeparator();
	menuRecentFiles = new wxMenu();
	UpdateMenuRecentFiles();
	menuFile->AppendSubMenu(menuRecentFiles, _("&Reccent Files") );
	menuFile->AppendSeparator();
	menuFile->Append( wxID_EXIT, _("E&xit") );
	// view menu
	menuView->Append( IDM_ZOOM_IN, _("Zoom &In") );
	menuView->Append( IDM_ZOOM_OUT, _("Zoom &Out") );
	menuView->AppendSeparator();
	menuView->Append( IDM_ROTATE_LEFT,  _("Rotate &Anticlockwise") );
	menuView->Append( IDM_ROTATE_RIGHT, _("Rotate &Clockwise") );
	menuView->AppendSeparator();
	menuView->Append( IDM_INIT_SCREEN, _("&Default Screen") );
	menuView->AppendSeparator();
	menuView->Append( IDM_REFRESH, _("&Refresh Screen") );
	// control menu
	menuControl->AppendRadioItem( IDM_HI_TYPE, _("&HI Type") );
	menuControl->AppendRadioItem( IDM_EP_TYPE, _("&EP Type") );
	menuControl->AppendSeparator();
	menuControl->Append( IDM_RESET, _("&Reset") );
	menuControl->Append( IDM_SETTINGS, _("&Settings...") );
	// other menu
	menuOther->AppendCheckItem( IDM_SERVER, _("&Enable Server") );
//	menuOther->AppendSeparator();
//	mitm = menuOther->AppendCheckItem( IDM_SERVER_REVERSE, _("Data &Reversed") );
//	mitm->Check(gConfig.GetDataReverse());
	menuOther->AppendSeparator();
	menuOther->Append( IDM_SERVER_SETTINGS, _("&Server Settings...") );
	// help menu
	menuHelp->Append( wxID_ABOUT, _("About mpprinter...") );
#ifdef _DEBUG_LOG
	menuHelp->Append( IDM_DEBUGLOG, _T("Debug...") );
#endif

	// menu bar
	wxMenuBar *menuBar = new wxMenuBar;
	menuBar->Append( menuFile, _("&File") );
	menuBar->Append( menuView, _("&View") );
	menuBar->Append( menuControl, _("&Control") );
	menuBar->Append( menuOther, _("&Other") );
	menuBar->Append( menuHelp, _("&Help") );

	SetMenuBar( menuBar );

	// update menu
	EnableMenuItems(false);
	menuControl->Check(IDM_HI_TYPE, true);
//	menuView->Check(IDM_ROTATE, gConfig.GetRotate() != 0);

	// config box
	cfgbox = new CtrlSettingBox(this, IDD_CONFIGBOX);

	// timer
	mTimer.SetOwner(this, IDT_TIMER);

	// panel
	panel = new MpPrinterPanel(this);

	// title
	SetFrameTitle();
}

MpPrinterFrame::~MpPrinterFrame()
{
	StopServer();

	delete cfgbox;
	delete mpri;
}
/// 初期化
bool MpPrinterFrame::Init()
{
	// initialize
	mpri = new MP_PRINTER(wxGetApp(), wxGetApp().GetResPath());
	wxSize sz = panel->GetClientSize();
	if (!mpri->Init(this, sz.GetWidth(), sz.GetHeight())) {
		return false;
	}

	// set density
	mpri->SetDensity(gConfig.GetDensity());
	// set magnify
	mpri->Zoom(NULL, cZoomMags[gConfig.GetMagnify()]);

	return true;
}

/// 終了
void MpPrinterFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
 	CloseDataFile();
	Close(true);
}
/// About
void MpPrinterFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
	MpPrinterAbout(this, wxID_ANY).ShowModal();
}
/// 開く
void MpPrinterFrame::OnOpenFile(wxCommandEvent& WXUNUSED(event))
{
	MpPrinterFileDialog *dlg = new MpPrinterFileDialog(
		_("Open file"),
		gConfig.GetFilePath(),
		wxEmptyString,
		_("Supported files (*.bas;*.lpt;*.txt)|*.bas;*.lpt;*.txt|All files (*.*)|*.*"),
		wxFD_OPEN);

	int rc = dlg->ShowModal();
	wxString path = dlg->GetPath();

	delete dlg;

	if (rc == wxID_OK) {
		OpenDataFile(path);
	}
}
/// 最近使用したファイル
void MpPrinterFrame::OnOpenRecentFile(wxCommandEvent& event)
{
	wxMenuItem *item = menuRecentFiles->FindItem(event.GetId());
	if (!item) return;
	wxString path = item->GetItemLabel();
	CloseDataFile();
	OpenDataFile(path);
}
/// 閉じる
void MpPrinterFrame::OnCloseFile(wxCommandEvent& WXUNUSED(event))
{
	CloseDataFile();
}
/// ページ設定
void MpPrinterFrame::OnPageSetup(wxCommandEvent& event)
{
	if (mpri) {
		mpri->PageSetup(panel);
		panel->RefreshPanel();
	}
}
/// 印刷
void MpPrinterFrame::OnPrint(wxCommandEvent& event)
{
	if (mpri) {
		mpri->Print(panel);
		panel->RefreshPanel();
	}
}
/// 印刷プレビュー
void MpPrinterFrame::OnPrintPreview(wxCommandEvent& event)
{
	if (mpri) {
		mpri->PrintPreview(this);
		panel->RefreshPanel();
	}
}
/// マージン設定
void MpPrinterFrame::OnMarginSetup(wxCommandEvent& event)
{
	if (mpri) {
		mpri->MarginSetup(panel);
		panel->RefreshPanel();
	}
}
/// 拡大
void MpPrinterFrame::OnZoomIn(wxCommandEvent& event)
{
	ZoomIn();
}
/// 縮小
void MpPrinterFrame::OnZoomOut(wxCommandEvent& event)
{
	ZoomOut();
}
/// 色の濃さ
void MpPrinterFrame::OnDensity(wxCommandEvent& event)
{
	DensityBox dlg(this, wxID_ANY, gConfig.GetDensity(), mpri->GetReverse());
	if (dlg.ShowModal() == wxID_OK) {
		gConfig.SetDensity(dlg.GetDensity());
		mpri->SetDensity(dlg.GetDensity());
		mpri->SetReverse(dlg.GetReverse());
		Refresh();
	}
}
/// 回転
void MpPrinterFrame::OnRotate(wxCommandEvent& event)
{
	Rotate(event.GetId() == IDM_ROTATE_LEFT ? 1 : -1);
}
/// 画面初期状態
void MpPrinterFrame::OnInitScreen(wxCommandEvent& event)
{
	InitScreen();
}
/// 更新
void MpPrinterFrame::OnRefresh(wxCommandEvent& event)
{
	Refresh();
}
/// プリンタタイプ変更
void MpPrinterFrame::OnChangeType(wxCommandEvent& event)
{
	int id = event.GetId();

	// メニュー設定
	switch(id) {
	case IDM_HI_TYPE:
		if (mpri) mpri->SelectModel(MP_MODEL_MP1020);
		menuControl->Check(id, true);
		break;
	case IDM_EP_TYPE:
		if (mpri) mpri->SelectModel(MP_MODEL_MP80);
		menuControl->Check(id, true);
		break;
	}
	mpri->ClearCashData();
	panel->RefreshPanel();
}
/// 初期化
void MpPrinterFrame::OnReset(wxCommandEvent& event)
{
	if (mpri) {
		mpri->ClearSetting();
		mpri->PushSetting();
		mpri->ClearCashData();
		mpri->Preview(panel);
		panel->RefreshPanel();
	}
}
/// 設定
void MpPrinterFrame::OnSettings(wxCommandEvent& event)
{
	int rc;
	int i;

	if (mpri) {
		for(i=0; i<2; i++) {
			cfgbox->SetPPIX(i,mpri->GetPPIX(i));
			cfgbox->SetPPIY(i,mpri->GetPPIY(i));
			cfgbox->SetLineFeedHeight(i,mpri->GetLineFeedHeight(i));
			cfgbox->SetWideMode(i,mpri->GetWideMode(i));
			cfgbox->SetLineMode(i,mpri->GetLineMode(i));
			cfgbox->SetCharSet(i,mpri->GetCharSet(i));
			cfgbox->SetCPI(i,mpri->GetCPI(i));
		}
		// 現在のプリンタタイプ
		for(i=IDM_HI_TYPE; i<=IDM_EP_TYPE; i++) {
			if (menuControl->IsChecked(i)) {
				cfgbox->SelectTabNo(i-IDM_HI_TYPE);
				break;
			}
		}
		rc = cfgbox->ShowModal();

		if (rc == wxID_OK) {
			for(i=0; i<2; i++) {
				mpri->SetLineFeedHeight(i,cfgbox->GetLineFeedHeight(i));
				mpri->SetWideMode(i,cfgbox->GetWideMode(i));
				mpri->SetLineMode(i,cfgbox->GetLineMode(i));
				mpri->SetCharSet(i,cfgbox->GetCharSet(i));
				mpri->SetCPI(i,cfgbox->GetCPI(i));

				mpri->PushSetting(i);
			}
			mpri->ClearCashData();
			panel->RefreshPanel();
		}
	}
}
/// サーバー起動・停止
void MpPrinterFrame::OnServer(wxCommandEvent& event)
{
	if (event.IsChecked()) {
		if (!StartServer()) {
			wxMenu *menu = (wxMenu *)event.GetEventObject();
			menu->Check(IDM_SERVER, false);
		}
	} else {
		StopServer();
	}
}
/// データ反転
void MpPrinterFrame::OnServerReverse(wxCommandEvent& event)
{
	gConfig.SetDataReverse(event.IsChecked());
}
/// サーバー設定
void MpPrinterFrame::OnServerSettings(wxCommandEvent& event)
{
	// server box
	ServerBox servbox(this, IDD_SERVERBOX);
	servbox.SetHostname(gConfig.GetServerHost());
	servbox.SetPort(gConfig.GetServerPort());
	servbox.SetReverse(gConfig.GetDataReverse());
	servbox.SetEchoBack(gConfig.GetEchoBack());
	servbox.SetSendAck(gConfig.GetSendAck());
	servbox.SetDelay(gConfig.GetPrintDelay());
	int rc = servbox.ShowModal();
	if (rc == wxID_OK) {
		gConfig.SetServerHost(servbox.GetHostname());
		gConfig.SetServerPort(servbox.GetPort());
		gConfig.SetDataReverse(servbox.GetReverse());
		gConfig.SetEchoBack(servbox.GetEchoBack());
		gConfig.SetSendAck(servbox.GetSendAck());
		gConfig.SetPrintDelay(servbox.GetDelay());
	}
}

#ifdef _DEBUG_LOG
void MpPrinterFrame::OnDebugLog(wxCommandEvent& event)
{
	gDebugReport.Show(this);
}
#endif

/// タイマー
void MpPrinterFrame::OnTimer(wxTimerEvent& event)
{
	panel->RefreshPanel(false, true);
	mTimer.Stop();
}

/// ドロップされたファイルを開く
void MpPrinterFrame::OpenDroppedFile(wxString &path)
{
	CloseDataFile();
	OpenDataFile(path);
}

/// 指定したファイルを開く
void MpPrinterFrame::OpenDataFile(wxString &path)
{
	// set recent file path
	gConfig.SetFilePath(wxFileName::FileName(path).GetPath());

	CloseDataFile();

	if (!mpri->ReadFile(panel, path)) {
		return;
	}
	// set file path
	mFilePath = path;
	// update window
	SetFrameTitle();
	// update menu
	EnableMenuItems(true);
	// zoom
	mpri->Zoom(panel, cZoomMags[gConfig.GetMagnify()]);
	// update panel
	panel->RefreshPanel();
	// add recent
	gConfig.AddRecentFile(path);
	UpdateMenuRecentFiles();
//	// update panel
//	if (!mTimer.IsRunning()) {
//		mTimer.Start(1000, true);
//	}
}

/// ファイルを閉じる
void MpPrinterFrame::CloseDataFile()
{
	mpri->CloseFile();

	// clear file path
	mFilePath.Empty();
	// update window
	SetFrameTitle();
	// update menu
	EnableMenuItems(false);
	// update panel
	panel->SetScrollBarPos(0,0,0,0);
	panel->RefreshPanel();
}

/// メニューの有効無効を設定
void MpPrinterFrame::EnableMenuItems(bool enable)
{
	if (menuFile != NULL) {
		menuFile->Enable(IDM_CLOSE_FILE, enable);
//		menuFile->Enable(IDM_PAGE_SETUP, enable);
		menuFile->Enable(IDM_PRINT_PREVIEW, enable);
		menuFile->Enable(IDM_PRINT, enable);
//		menuFile->Enable(IDM_MARGIN_SETUP, enable);
	}
	if (menuView != NULL) {
//		menuView->Enable(IDM_ZOOM_IN, enable);
//		menuView->Enable(IDM_ZOOM_OUT, enable);
		menuView->Enable(IDM_REFRESH, enable);
	}
	mMenuEnable = enable;
}

/// 最近使用したファイル一覧を更新
void MpPrinterFrame::UpdateMenuRecentFiles()
{
	// メニューを更新
	wxArrayString names;
	gConfig.GetRecentFiles(names);
	for(int i=0; i<10 && i<(int)names.Count(); i++) {
		if (menuRecentFiles->FindItem(IDM_RECENT_FILE_0 + i)) menuRecentFiles->Delete(IDM_RECENT_FILE_0 + i);
		menuRecentFiles->Append(IDM_RECENT_FILE_0 + i, names[i]);
	}
}

/// 拡大
void MpPrinterFrame::ZoomIn()
{
	if (!mpri) return;

	int max_idx = gConfig.GetMaxMagnify();
	int zoom_idx = gConfig.GetMagnify();
	if (zoom_idx + 1 >= max_idx) return;

	if (mpri->Zoom(panel, cZoomMags[zoom_idx + 1])) {

		menuView->Enable(IDM_ZOOM_OUT, true);
		panel->ZoomScrollBarPos(cZoomMags[zoom_idx + 1] * 100 / cZoomMags[zoom_idx]);

		zoom_idx++;
		if (zoom_idx + 1 >= max_idx) menuView->Enable(IDM_ZOOM_IN, false);

		gConfig.SetMagnify(zoom_idx);
		mpri->ClearCashData();
		panel->RefreshPanel();
		SetFrameTitle();
	}
}
/// 縮小
void MpPrinterFrame::ZoomOut()
{
	if (!mpri) return;

	int zoom_idx = gConfig.GetMagnify();

	if (zoom_idx <= 0) return;

	if (mpri->Zoom(panel, cZoomMags[zoom_idx - 1])) {

		menuView->Enable(IDM_ZOOM_IN, true);
		panel->ZoomScrollBarPos(cZoomMags[zoom_idx - 1] * 100 / cZoomMags[zoom_idx]);

		zoom_idx--;
		if (zoom_idx <= 0) menuView->Enable(IDM_ZOOM_OUT, false);

		gConfig.SetMagnify(zoom_idx);
		mpri->ClearCashData();
		panel->RefreshPanel();
		SetFrameTitle();
	}
}
/// フォーカスセット
void MpPrinterFrame::SetFocus()
{
	wxFrame::SetFocus();
	panel->SetFocus();
}

/// 回転
/// @param[in] direction 1:左90度 -1:右90度
void MpPrinterFrame::Rotate(int direction)
{
	direction = (direction > 0 ? 1 : (direction < 0 ? -1 : 0));
	
	if (direction != 0) {
		direction = (direction + gConfig.GetRotate() + 4) % 4;
	}
	gConfig.SetRotate(direction);
	panel->RefreshPanel();
	SetFrameTitle();
}

/// 画面初期状態
void MpPrinterFrame::InitScreen()
{
	int init_zoom = 1;
	int zoom_idx = gConfig.GetMagnify();

	if (mpri && mpri->Zoom(panel, cZoomMags[init_zoom])) {

		menuView->Enable(IDM_ZOOM_IN, true);
		menuView->Enable(IDM_ZOOM_OUT, true);
		panel->ZoomScrollBarPos(cZoomMags[init_zoom] * 100 / cZoomMags[zoom_idx]);

		gConfig.SetMagnify(init_zoom);

		mpri->ClearCashData();
	}

	Rotate(0);
}

/// 画面更新
void MpPrinterFrame::Refresh()
{
	mpri->ClearCashData();
	mpri->Preview(panel);
	panel->RefreshPanel();
}

/// サーバー構築
bool MpPrinterFrame::StartServer()
{
	server = new MpPrinterServer(this);
	if (!server->IsOk()) {
		StopServer();
		return false;
	}
	return true;
}

/// サーバ機能停止
void MpPrinterFrame::StopServer()
{
	delete server;
	server = NULL;
}

/// 受信したデータをセット
void MpPrinterFrame::SetData(const wxByte* data, int size)
{
	if (mpri) {
		mpri->SetData(panel, data, size, gConfig.GetDataReverse());
		// update menu
		EnableMenuItems(true);
		// update panel
		if (!mTimer.IsRunning()) {
			mTimer.Start(50, true);
		}
	}
}

/// ウィンドウタイトルをセット
void MpPrinterFrame::SetFrameTitle()
{
	wxString title = wxGetApp().GetAppName();
	int mag = cZoomMags[gConfig.GetMagnify()];
	int deg = gConfig.GetRotate() * 90;
	title += wxString::Format(wxT(" [mag:%d%%, deg:%d]")
		, mag
		, deg);
	if (!mFilePath.IsEmpty()) {
		title += _T(" - ");
		title += mFilePath;
	}
	SetTitle(title);
}

//
// Control Panel
//
// Attach Event
BEGIN_EVENT_TABLE(MpPrinterPanel, wxScrolledWindow)
	// event
//	EVT_PAINT(MpPrinterPanel::OnPaint)
	EVT_SIZE(MpPrinterPanel::OnSize)

    EVT_LEFT_DOWN(MpPrinterPanel::OnMouseLeftDown)
    EVT_LEFT_UP(MpPrinterPanel::OnMouseLeftUp)
    EVT_RIGHT_DOWN(MpPrinterPanel::OnMouseRightDown)
    EVT_MOTION(MpPrinterPanel::OnMouseMove)
    EVT_MOUSE_CAPTURE_LOST(MpPrinterPanel::OnMouseCaptureLost)
	EVT_MOUSEWHEEL(MpPrinterPanel::OnMouseWheel)

	EVT_KEY_DOWN(MpPrinterPanel::OnKeyDown)
	EVT_KEY_UP(MpPrinterPanel::OnKeyUp)

	EVT_SCROLLWIN(MpPrinterPanel::OnScrollWin)
END_EVENT_TABLE()

MpPrinterPanel::MpPrinterPanel(MpPrinterFrame *parent)
       : wxScrolledWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize
#if defined(__WXGTK__)
	   , wxHSCROLL | wxVSCROLL)
#else
       , wxHSCROLL | wxVSCROLL | wxALWAYS_SHOW_SB)
#endif
{
	frame = parent;
	ctrl_down = false;
	left_down = false;
	mMouseWheelEvent = 0;
	mDrawPart = false;

	curMag = CreateCursor((const char *)magnify_bits, (const char *)magnify_mask_bits);
	curHand = CreateCursor((const char *)hand_bits, (const char *)hand_mask_bits);

	SetScrollBarPos(0, 0, 0, 0);

	EnableScrolling(true, true);
	ShowScrollbars(wxSHOW_SB_ALWAYS, wxSHOW_SB_ALWAYS);
	// キーイベントは自前で処理する
	DisableKeyboardScrolling();

	// drag and drop
	SetDropTarget(new MpPrinterFileDropTarget(parent));

}

MpPrinterPanel::~MpPrinterPanel()
{
	delete curHand;
	delete curMag;
}

// 描画
void MpPrinterPanel::OnDraw(wxDC &dc)
{
	int px,py;
	GetViewStart(&px, &py);
	px *= SCROLLBAR_UNIT;
	py *= SCROLLBAR_UNIT;
	MP_PRINTER *mpri = frame->GetMpPrinter();
	if (mpri) {
		mpri->DrawData(&dc, px, py, mDrawPart);
		SetScrollBarPos(mpri->GetDocWidth(),mpri->GetDocHeight(), px, py);
	}
}
#if 0
void MpPrinterPanel::OnPaint(wxPaintEvent& event)
{
	wxPaintDC dc(this);
	DoPrepareDC(dc);

	OnDraw(dc);
}
#endif

// リサイズ
void MpPrinterPanel::OnSize(wxSizeEvent& event)
{
	wxSize size = GetClientSize();
	MP_PRINTER *mpri = frame->GetMpPrinter();
	if (mpri) {
		mpri->SetWindowSize(size.x, size.y);
		RefreshPanel();
	}
}

// マウス左ボタン押下
void MpPrinterPanel::OnMouseLeftDown(wxMouseEvent& event)
{
	left_down = event.LeftIsDown();
	if (ctrl_down) {
		// 拡大
		frame->ZoomIn();
	}
	mMouseDownPoint = event.GetPosition();
	ChangeCursor();
}

// マウス左ボタン離す
void MpPrinterPanel::OnMouseLeftUp(wxMouseEvent& event)
{
	left_down = event.LeftIsDown();
	ChangeCursor();
}

// マウス右ボタン押下
void MpPrinterPanel::OnMouseRightDown(wxMouseEvent& event)
{
	if (ctrl_down) {
		// 縮小
		frame->ZoomOut();
	}
}

// マウス移動
void MpPrinterPanel::OnMouseMove(wxMouseEvent& event)
{
	if (left_down && !ctrl_down) {
		wxPoint nPos = event.GetPosition();
		wxPoint nDelta = mMouseDownPoint - nPos;

		ScrollArea(nDelta.x, nDelta.y);

		mMouseDownPoint = nPos;
	}
}

// マウスがウィンドウから離れた
void MpPrinterPanel::OnMouseCaptureLost(wxMouseCaptureLostEvent& event)
{
}

// マウスホイール
void MpPrinterPanel::OnMouseWheel(wxMouseEvent& event)
{
	mMouseWheelEvent = 1;
	int rotate = gConfig.GetRotate();
	int delta = event.GetWheelRotation();
	if (rotate & 1) {
		ScrollArea(-delta, 0);
	} else {
		ScrollArea(0, -delta);
	}
}

// スクロールした
void MpPrinterPanel::OnScrollWin(wxScrollWinEvent& event)
{
	if (mMouseWheelEvent > 0) {
#ifdef __WXMSW__
		// マウスホイールのイベントは無視
		event.SetEventType(wxEVT_NULL);
#endif
		mMouseWheelEvent--;
	}
	event.Skip();
}

// キー押下
void MpPrinterPanel::OnKeyDown(wxKeyEvent& event)
{
	ctrl_down = event.ControlDown();

	int code = event.GetKeyCode();
	switch(code) {
		case WXK_UP:
			ScrollArea(0, -8);
			break;
		case WXK_DOWN:
			ScrollArea(0, 8);
			break;
		case WXK_LEFT:
			ScrollArea(-8, 0);
			break;
		case WXK_RIGHT:
			ScrollArea(8, 0);
			break;
		case WXK_PAGEUP:
			{
				wxSize sz = GetSize();
				ScrollArea(0, -sz.GetHeight());
			}
			break;
		case WXK_PAGEDOWN:
			{
				wxSize sz = GetSize();
				ScrollArea(0, sz.GetHeight());
			}
			break;
		case WXK_ADD:
		case WXK_NUMPAD_ADD:
			frame->ZoomIn();
			break;
		case WXK_SUBTRACT:
		case WXK_NUMPAD_SUBTRACT:
			frame->ZoomOut();
			break;
	}
	ChangeCursor();
	event.Skip();
}
// キー離す
void MpPrinterPanel::OnKeyUp(wxKeyEvent& event)
{
	ctrl_down = event.ControlDown();

	ChangeCursor();
}

// リフレッシュ
void MpPrinterPanel::Refresh(bool eraseBackground, const wxRect *rect)
{
	mDrawPart = false;
//#ifdef __WXMSW__
//	wxWindow::Refresh(false, rect);
//#else
	wxWindow::Refresh(eraseBackground, rect);
//#endif
}
void MpPrinterPanel::RefreshPanel(bool eraseBackground, bool drawPart)
{
	mDrawPart = drawPart;
	wxWindow::Refresh(eraseBackground);
}

// カーソル設定
void MpPrinterPanel::ChangeCursor()
{
	if (ctrl_down) {
		SetCursor(*curMag);
	} else if (left_down) {
		SetCursor(*curHand);
	} else {
		SetCursor(wxNullCursor);
	}
}

// スクロールバーを設定
void MpPrinterPanel::SetScrollBarPos(int new_ux, int new_uy, int new_px, int new_py)
{
	int ux, uy, px, py, sx, sy;
	GetVirtualSize(&ux, &uy);
	GetViewStart(&px, &py);
	px *= SCROLLBAR_UNIT;
	py *= SCROLLBAR_UNIT;
	GetClientSize(&sx, &sy);
	if (new_ux < sx) new_ux = sx;
	if (new_uy < sy) new_uy = sy;
	if (ux != new_ux || uy != new_uy || px != new_px || py != new_py) {
		SetScrollbars(SCROLLBAR_UNIT, SCROLLBAR_UNIT
			, new_ux / SCROLLBAR_UNIT, new_uy / SCROLLBAR_UNIT
			, new_px / SCROLLBAR_UNIT, new_py / SCROLLBAR_UNIT, true);
	}
}

// スクロールバー位置の拡大
void MpPrinterPanel::ZoomScrollBarPos(int mag)
{
	int ux, uy, px, py;
	GetVirtualSize(&ux, &uy);
	GetViewStart(&px, &py);
	px *= SCROLLBAR_UNIT;
	py *= SCROLLBAR_UNIT;
	px = px * mag / 100;
	py = py * mag / 100;
	ux = ux * mag / 100;
	uy = uy * mag / 100;
	SetScrollBarPos(ux,uy,px,py);
}

// 移動
void MpPrinterPanel::ScrollArea(int x, int y)
{
	wxPoint pos = GetViewStart();
	pos.x *= SCROLLBAR_UNIT;
	pos.y *= SCROLLBAR_UNIT;
	Scroll((pos.x + x) / SCROLLBAR_UNIT, (pos.y + y) / SCROLLBAR_UNIT);
#ifdef __WXMSW__
	// ちらつきを避けるため更新する領域を絞る
	int ux, uy;
	GetVirtualSize(&ux, &uy);
	if (x > 0) {
		wxRect rex(ux - x, 0, x, uy);   
		RefreshRect(rex, false);
	} else if (x < 0) {
		wxRect rex(0, 0, -x, uy);   
		RefreshRect(rex, false);
	}
	if (y > 0) {
		wxRect rey(0, uy - y, ux, y);   
		RefreshRect(rey, false);
	} else if (y < 0) {
		wxRect rey(0, 0, ux, -y);   
		RefreshRect(rey, false);
	}
#else
	Refresh();
#endif
}

// カーソルの作成
wxCursor *MpPrinterPanel::CreateCursor(const char *bits, const char *mask)
{
    wxBitmap bmp_bits(bits, 32, 32);
    wxBitmap bmp_mask(mask, 32, 32);
#ifdef __WXOSX__
    bmp_bits.SetMask(new wxMask(bmp_mask, wxColor(*wxBLACK)));
#else
    bmp_bits.SetMask(new wxMask(bmp_mask));
#endif
    wxImage img = bmp_bits.ConvertToImage();
    img.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_X, 16);
    img.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y, 16);
    wxCursor *new_cursor = new wxCursor(img);
	return new_cursor;
}

//
// File Dialog
//
MpPrinterFileDialog::MpPrinterFileDialog(const wxString& message, const wxString& defaultDir, const wxString& defaultFile, const wxString& wildcard, long style)
            : wxFileDialog(NULL, message, defaultDir, defaultFile, wildcard, style)
{
}

//
// File Drag and Drop
//
MpPrinterFileDropTarget::MpPrinterFileDropTarget(MpPrinterFrame *parent)
			: frame(parent)
{
}

bool MpPrinterFileDropTarget::OnDropFiles(wxCoord x, wxCoord y ,const wxArrayString &filenames)
{
	if (filenames.Count() > 0) {
		wxString name = filenames.Item(0);
		frame->OpenDroppedFile(name);
	}
    return true;
}

//
// About dialog
//
MpPrinterAbout::MpPrinterAbout(wxWindow* parent, wxWindowID id)
	: wxDialog(parent, id, _("About..."), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE)
{
	wxSizerFlags flags = wxSizerFlags().Expand().Border(wxALL, 4);

	wxBoxSizer *szrLeft   = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *szrRight  = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *szrMain   = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *szrAll    = new wxBoxSizer(wxVERTICAL);

	szrLeft->Add(new wxStaticBitmap(this, wxID_ANY,
		wxBitmap(mpprinter_xpm), wxDefaultPosition, wxSize(64, 64))
		, flags);

	wxString str = _T("");
	str += _T("mpprinter, Version ");
	str += _T(APPLICATION_VERSION);
	str += _T(" \"");
	str += _T(PLATFORM);
	str += _T("\"\n\n");
	str	+= _T("using ");
	str += wxVERSION_STRING;
	str += _T("\n\n");
	str	+= _T(APP_COPYRIGHT);

	szrRight->Add(new wxStaticText(this, wxID_ANY, str), flags);

	wxSizer *szrButtons = CreateButtonSizer(wxOK);
	szrMain->Add(szrLeft, flags);
	szrMain->Add(szrRight, flags);
	szrAll->Add(szrMain, flags);
	szrAll->Add(szrButtons, flags);

	SetSizerAndFit(szrAll);
}
