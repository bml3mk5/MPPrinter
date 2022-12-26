/// @file mp_printer.cpp
///
/// @brief MPPRINTER 制御クラス
///
/// @date 2013.11.27 Create
///
/// @author Copyright (c) 2013, Sasaji. All rights reserved.
///
#include "mp_printer.h"
#include "mp1020.h"
#include "mp80.h"
#include <wx/filename.h>
#include <wx/wfstream.h>
#include "config.h"

#ifdef __WXOSX__
#include "wx/osx/printdlg.h"
// 描画範囲外のデータをトリミング
//#define USE_TRIMING_AREA
#define PRINT_FRAMEWORK 1
#else
#define PRINT_FRAMEWORK 2
#endif
#ifdef _DEBUG_LOG
static const wxString capsTypeStr[4] = {_T("Display"),_T("Printer"),_T("Preview"),_T("?")};
#endif

#ifndef USE_PIXELDATA
#define MX_RGB_SIZE	3
#endif

/// コンストラクタ
MP_PRINTER::MP_PRINTER(const wxApp &app, const wxString &data_path)
{
	mApp     = &app;
	mDataPath = data_path;

	bufDw = NULL;

#ifdef USE_PIXELDATA
	bmpMx = NULL;
	bufMx = NULL;
#endif
	bufDm = NULL;
	bufMix = NULL;
	dcMx[0] = NULL;
	dcMx[1] = NULL;

#ifdef USE_CASH_BUFFER
	mUseCash = false;
	mDataCashd = false;
	bmpCashd = NULL;
	dcCashd = NULL;
#endif
	zoom_mag = 100;
	mRotate = 0;

	mMaxPage = 0;
	mCurPage = 1;
	mIngPage = -1;

	mPrintDC = NULL;

	mp_model = NULL;
	mp_model_list[MP_MODEL_MP1020] = new MP1020(this);
	mp_model_list[MP_MODEL_MP80] = new MP80(this);

	for(int i=0; i<capsUnknownType; i++) {
		device_info[i].tech = (CapsInfoTypeTech)i;
	}

	reverse = false;

	text_data.Allocate(524288);
	subst_data.Allocate(1024);

#if defined(__WXOSX__)
	// MacではANDがエラーになる
	device_info[capsDisplayType].mBlitMode = wxCOPY;
	device_info[capsPrinterType].mBlitMode = wxCOPY;
	device_info[capsPreviewType].mBlitMode = wxCOPY;
#elif defined(__WXGTK__)
	device_info[capsDisplayType].mBlitMode = wxCOPY;
	device_info[capsPrinterType].mBlitMode = wxCOPY;
	device_info[capsPreviewType].mBlitMode = wxCOPY;
#else
	device_info[capsDisplayType].mBlitMode = wxCOPY;
	device_info[capsPrinterType].mBlitMode = wxCOPY;
	device_info[capsPreviewType].mBlitMode = wxCOPY;
#endif
}

MP_PRINTER::~MP_PRINTER()
{
	release_cashdata();
#ifdef USE_PAGE_BUFFER
	delete dcPage;
#endif
	delete dcMx[0];
	delete dcMx[1];
#ifdef USE_PIXELDATA
	delete bufMx;
	delete bmpMx;
#else
	delete[] bufMix;
#endif
	delete bufDm;
	delete bufDw;
	delete mp_model_list[MP_MODEL_MP80];
	delete mp_model_list[MP_MODEL_MP1020];
}

/// 初期化
/// @param[in] win 親ウィンドウ
/// @param[in] w ウィンドウ幅
/// @param[in] h ウィンドウ高さ
bool MP_PRINTER::Init(wxWindow *win, int w, int h)
{
	int i;

	bufDw = new DpBufferDw(MP_PRN_BUF_WIDTH, MP_PRN_BUF_HEIGHT);
	bufDm = new DpBufferMx(MP_PRN_BUF_WIDTH, MP_PRN_BUF_HEIGHT);
#ifdef USE_PIXELDATA
	bmpMx = new wxBitmap(MP_PRN_BUF_WIDTH, MP_PRN_BUF_HEIGHT, 32);
	if (!bmpMx->IsOk()) {
		gErrInfo.SetInfo(__FILE__, __LINE__, myError, myErrSystem, _T("Cannot create bmpMx."));
		gErrInfo.ShowMsgBox(win);
		return false;
	}
	bufMx = new wxAlphaPixelData(*bmpMx);
	if (bufMx->GetWidth() != MP_PRN_BUF_WIDTH
	 || bufMx->GetHeight() != MP_PRN_BUF_HEIGHT) {
		gErrInfo.SetInfo(__FILE__, __LINE__, myError, myErrSystem, _T("Invalid create bufMx."));
		gErrInfo.ShowMsgBox(win);
		return false;
	}
#else
	bufMix = new wxByte[MP_PRN_BUF_WIDTH * MP_PRN_BUF_HEIGHT * MX_RGB_SIZE];
#endif

	if (!recreate_mixdc()) {
		gErrInfo.ShowMsgBox(win);
		return false;
	}

#ifdef USE_PAGE_BUFFER
	dcPage = NULL;
#endif

	SetWindowSize(w, h);
	ClearData(_T(""));

	for (i=MP_MODEL_MP1020; i<=MP_MODEL_MP80; i++) {
		// read font data
		if (mp_model_list[i]->ReadFont() != true) {
			gErrInfo.SetInfo(__FILE__, __LINE__, myError, myErrFontFile);
			gErrInfo.ShowMsgBox(win);
			return false;
		}
		mp_model_list[i]->ClearSetting();
		mp_model_list[i]->PushSetting();
	}
	SelectModel(MP_MODEL_MP1020);
	ClearSetting();

#ifdef USE_CASH_BUFFER
	mUseCash = gConfig.GetMemoryCash();
#endif

	return true;
}

/// 合成用DCを作成
bool MP_PRINTER::recreate_mixdc()
{
	int w, h;

	for(int i=0; i<2; i++) {
		delete dcMx[i];
	
		switch(i) {
		case 1:
			w = MP_PRN_BUF_HEIGHT;
			h = MP_PRN_BUF_WIDTH;
			break;
		default:
			w = MP_PRN_BUF_WIDTH;
			h = MP_PRN_BUF_HEIGHT;
			break;
		}

		wxBitmap bmpDummy(w, h, 24);
		if (!bmpDummy.IsOk()) {
			gErrInfo.SetInfo(__FILE__, __LINE__, myError, myErrSystem,
				wxString::Format(_T("Cannot create bmpDummy%d."), i));
			return false;
		}
		dcMx[i] = new wxMemoryDC(bmpDummy);
		if (!dcMx[i]->IsOk()) {
			gErrInfo.SetInfo(__FILE__, __LINE__, myError, myErrSystem,
				wxString::Format(_T("Cannot create dcMx%d."), i));
			return false;
		}
		dcMx[i]->SetBackground(wxBrush(wxColour(0xff,0,0)));
		dcMx[i]->Clear();
	}

	return true;
}

/// 機種選択
/// @param[in] model 0:MP1020 1:MP80
void MP_PRINTER::SelectModel(int model)
{
	mp_model = mp_model_list[model];
	selected_model = model;

	for(int i=0; i<capsUnknownType; i++) {
		device_info[i].vm_ppi.Set(mp_model->GetPPIX(), mp_model->GetPPIY());
	}
	rDpPpi.Set(mp_model->GetPPIX(), mp_model->GetPPIY());

	mp_model->GetDispAspectRatio(&dsr);

	Zoom(NULL, zoom_mag);
}

/// 紙サイズなどを初期状態に戻す
void MP_PRINTER::ClearSetting()
{
	mp_model->ClearSetting();

	// A4
	printdata.SetOrientation(wxPORTRAIT);
	printdata.SetPaperId(wxPAPER_A4);
	printdata.SetNoCopies(1);

	printdlg.SetPrintData(printdata);
	pagesetdlg.SetPrintData(printdata);

	// 紙の余白
	pagesetdlg.SetMarginTopLeft(wxPoint(10,10));	// 10mm
	pagesetdlg.SetMarginBottomRight(wxPoint(10,10));	// 10mm
	for(int i=0; i<capsUnknownType; i++) {
		device_info[i].margin_mm.Set(10, 10);
	}

	pagesetdlg.SetMinMarginTopLeft(wxPoint(0,0));	// 0mm
	pagesetdlg.SetMinMarginBottomRight(wxPoint(0,0));	// 0mm

	return;
}

/// データバッファクリア
/// @param[in] title タイトル名
void MP_PRINTER::ClearData(const wxString &title)
{
	text_file = title;
//	memset(text_data, 0, sizeof(text_data));
//	text_len = 0;
	text_data.Clear();

	draw_mode = false;
	draw_mode_bkup = false;
	printing = 0;
}

/// データバッファにセット
/// @param[in] win 親ウィンドウ
/// @param[in] data データ
/// @param[in] size データサイズ
/// @param[in] reverse データを反転するか
void MP_PRINTER::SetData(wxWindow *win, const wxByte* data, size_t size, bool reverse)
{
	size_t first_pos = text_data.GetWritePos();

	text_data.AddData(data, size);
	if (reverse) {
		text_data.Invert(first_pos);
	}

	// 画面に反映
	if (first_pos == 0) {
		mp_model->PushSetting();
		draw_mode = true;
		if (win != NULL) Preview(win);
	}
}

/// データバッファにあるデータサイズを返す
int MP_PRINTER::GetLength() const
{
//	return (int)text_len;
	return (int)text_data.GetWritePos();
}

/// ページ設定ダイアログ
/// @param[in] win 親ウィンドウ
void MP_PRINTER::PageSetup(wxWindow *win)
{
	/* ページ設定ダイアログ */
	if (set_printer_page(win) != true) {
		return;
	}
	return;
}

/// マージン設定ダイアログ
/// @param[in] win 親ウィンドウ
void MP_PRINTER::MarginSetup(wxWindow *win)
{
	if (set_margin_page(win) != true) {
		return;
	}
	return;
}

/// 印刷
/// @param[in] win 親ウィンドウ
bool MP_PRINTER::Print(wxWindow *win)
{
	bool rc = true;

//	if (text_len == 0) return true;
	if (text_data.GetWritePos() == 0) return true;

	/* プリンター選択ダイアログ */
	if (select_printer(win) != true) {
		return true;
	}

	mRotate = 0;

#if PRINT_FRAMEWORK == 1
	wxPrinterDC dc(printdata);
	rc = print_data(&dc);
	if (!rc) {
		gErrInfo.SetInfo(__FILE__, __LINE__, myError, myErrPrinting);
		gErrInfo.ShowMsgBox(win);
	}
#endif
	if (draw_mode_bkup) {
		draw_mode = draw_mode_bkup;
		Preview(win);
	}

	return rc;
}

/// 印刷プレビュー
/// @param[in] win 親ウィンドウ
bool MP_PRINTER::PrintPreview(wxWindow *win)
{
	bool rc = true;

	if (text_data.GetWritePos() == 0) return true;

	mRotate = 0;

	wxPrintPreview *preview =
	new wxPrintPreview(new MpPrintout(this, text_file, &pagesetdlg), NULL, &printdlg);
	if (!preview->IsOk()) {
		delete preview;
		gErrInfo.SetInfo(__FILE__, __LINE__, myError, myErrPreview);
		gErrInfo.ShowMsgBox(win);
		return false;
	}

	new MpPreviewFrame(preview, win);

	return rc;
}

/// 画面用プレビュー
/// @param[in] win 親ウィンドウ
void MP_PRINTER::Preview(wxWindow *win)
{
	if (text_data.GetWritePos() == 0 || !draw_mode) return;

	set_paper_size(&device_info[capsDisplayType], rDpPpi.x, rDpPpi.y);

	draw_mode = true;

	mIngPage = -1;

	// 描画はOnDrawイベントで行う
//	draw_data(&dc, &ddinfo);
}

/// ズーム
/// @param[in] win 親ウィンドウ
/// @param[in] mag %で指定 100=等倍
/// @return true のみ
bool MP_PRINTER::Zoom(wxWindow *win, int mag)
{
	rDpPpi.Set((int)((mp_model->GetPPIX()) * mag / 100.0),
				(int)((mp_model->GetPPIY()) * mag / 100.0));
	zoom_mag = mag;

	// サイズを再設定
	set_paper_size(&device_info[capsDisplayType], rDpPpi.x, rDpPpi.y);

	return true;
}

/// プレビュー用画面を描く
/// @param[in] dc        デバイスコンテキスト
/// @param[in] left      左端
/// @param[in] top       上端
/// @param[in] draw_part 追加されたデータのみ描画するか
void MP_PRINTER::DrawData(wxDC *dc, int left, int top, bool draw_part)
{
	if (draw_mode) {
		mRotate = gConfig.GetRotate();

#ifdef USE_CASH_BUFFER
		if (mUseCash) {
			// 描画データをメモリにキャッシュしておく
			if (mDataCashd) {
				draw_from_cashdata(dc, &device_info[capsDisplayType]);
			} else {
				// 高さと幅の計算を行う
				dp_offset.x = 0;
				dp_offset.y = 0;
				draw_data(NULL, &device_info[capsDisplayType], draw_part);
				// バッファを作成
				if (create_cashdata(dc)) {
					// バッファに描画
					draw_to_cashdata(dc, &device_info[capsDisplayType], draw_part);
				}
			}
		} else
#endif
		{
			dp_offset.Set(left, top);
			draw_data(NULL, &device_info[capsDisplayType], draw_part);
			draw_data(dc, &device_info[capsDisplayType], mIngPage != mMaxPage ? false : draw_part);
			mIngPage = mMaxPage;
		}
	}
}
/// 印刷用 MpPrintoutクラスより呼ばれる
/// @param[in] dc デバイスコンテキスト
/// @param[in] preview プレビューモード
/// @return true のみ
bool MP_PRINTER::PrintingStart(wxDC *dc, bool preview)
{
	draw_mode_bkup = draw_mode;
	draw_mode = false;
	mRotate = 0;

	if (preview) {
		printing = 2;
		dinfo = &device_info[capsPreviewType];
	} else {
		printing = 1;
		dinfo = &device_info[capsPrinterType];
	}
	get_device_caps(dc, dinfo);

	return true;
}
/// 印刷用 MpPrintoutクラスより呼ばれる
/// @param[in] dc デバイスコンテキスト
/// @return true
bool MP_PRINTER::PrintStartDocument(wxDC *dc)
{
	mRotate = 0;

	return draw_start_document(dc, dinfo);
}
/// 印刷用 MpPrintoutクラスより呼ばれる
/// @param[in] dc デバイスコンテキスト
/// @param[in] pageNum ページ番号
/// @return true
bool MP_PRINTER::PrintPage(wxDC *dc, int pageNum)
{
	bool rc = true;
	// 途中のページから印刷する場合でも1ページ目から印刷処理を行う
	for(int page = mCurPage; page <= pageNum && rc; page++) {
		draw_start_page(dc, dinfo, pageNum, true);
		draw_page(dc, dinfo, pageNum);
		rc = draw_end_page(dc, dinfo, pageNum);
	}
	return rc;
}
/// 印刷用 MpPrintoutクラスより呼ばれる
/// @param[in] dc デバイスコンテキスト
/// @return true
bool MP_PRINTER::PrintEndDocument(wxDC *dc)
{
	return draw_end_document(dc, dinfo);
}
/// 印刷用 MpPrintoutクラスより呼ばれる
/// @param[in] dc デバイスコンテキスト
/// @return true
bool MP_PRINTER::PrintingEnd(wxDC *dc)
{
	draw_mode = draw_mode_bkup;

	return true;
}

/// ファイルを開いてバッファに読み込む
/// @param[in] win 親ウィンドウ
/// @param[in] text_path 入力ファイル
/// @return true / false
bool MP_PRINTER::ReadFile(wxWindow *win, const wxString &text_path)
{
	wxFileInputStream stream(text_path);
	if (!stream.IsOk()) {
		return false;
	}

	// ファイル読み込み
//	memset(text_data, 0, sizeof(text_data));
//	stream.Read((void *)text_data,sizeof(text_data));
//	// filesize
	size_t text_len = stream.GetSize();
	text_data.Clear();
	text_data.Allocate(text_len);
	stream.Read(text_data.GetData(), text_len);
	text_data.SetWritePos(text_len);

	// ファイル名だけ保存
	wxFileName file_name(text_path);
	text_file = file_name.GetName();

	// 画面に反映
//	if (text_len > 0) {
	if (text_data.GetWritePos() > 0) {
		draw_mode = true;
		mp_model->PushSetting();
	}
	if (win != NULL) {
		Preview(win);
	}

	return true;
}

/// ファイルを閉じる
void MP_PRINTER::CloseFile()
{
	ClearData(_T(""));
	release_cashdata();
}

/// ドキュメントの幅を返す
/// @return 幅
int  MP_PRINTER::GetDocWidth()
{
	if (draw_mode && mp_model) {
		switch(mRotate & 1) {
		case 1:
			mp_model->GetDispAspectRatio(&dsr);
			return device_info[capsDisplayType].ph_size.height * dsr.x * mMaxPage / dsr.y;
		default:
			return device_info[capsDisplayType].ph_size.width;
		}
	} else {
		return 0;
	}
}

/// ドキュメントの高さを返す
/// @return 高さ
int  MP_PRINTER::GetDocHeight()
{
	if (draw_mode && mp_model) {
		switch(mRotate & 1) {
		case 1:
			return device_info[capsDisplayType].ph_size.width;
		default:
			mp_model->GetDispAspectRatio(&dsr);
			return device_info[capsDisplayType].ph_size.height * dsr.x * mMaxPage / dsr.y;
		}
	} else {
		return 0;
	}
}

/// ウィンドウの幅と高さを保存する
/// @param[in] width 幅
/// @param[in] height 高さ
void MP_PRINTER::SetWindowSize(int width, int height)
{
	win_size.Set(width, height);
}

/// 印字パラメータを保存
void MP_PRINTER::PushSetting()
{
	mp_model->PushSetting();
}
/// 印字パラメータを保存
/// @param[in] idx 0:MP1020 1:MP80
void MP_PRINTER::PushSetting(int idx)
{
	mp_model_list[idx]->PushSetting();
}
/// ページ数を渡す
/// @param[out] minPage 最小ページ
/// @param[out] maxPage 最大ページ
/// @param[out] pageFrom 開始ページ
/// @param[out] pageTo 終了ページ
void MP_PRINTER::GetPageInfo(int *minPage, int *maxPage, int *pageFrom, int *pageTo)
{
	*pageFrom = printdlg.GetFromPage();
	*pageTo = printdlg.GetToPage();
	*minPage = printdlg.GetMinPage();
	*maxPage = printdlg.GetMaxPage();
}
/// X方向のピクセル/インチ
/// @param[in] idx 0:MP1020 1:MP80
/// @return ppi
int  MP_PRINTER::GetPPIX(int idx) {
	return mp_model_list[idx]->GetPPIX();
}
/// Y方向のピクセル/インチ
/// @param[in] idx 0:MP1020 1:MP80
/// @return ppi
int  MP_PRINTER::GetPPIY(int idx) {
	return mp_model_list[idx]->GetPPIY();
}
/// 改行幅を設定
/// @param[in] idx 0:MP1020 1:MP80
/// @param[in] val 幅(pixel)
void MP_PRINTER::SetLineFeedHeight(int idx, double val) {
	mp_model_list[idx]->SetLineFeedHeight(val);
}
/// 改行幅
/// @param[in] idx 0:MP1020 1:MP80
/// @return 幅(pixel)
double MP_PRINTER::GetLineFeedHeight(int idx) {
	return mp_model_list[idx]->GetLineFeedHeight();
}
/// 1行の長さモード設定
/// @param[in] idx 0:MP1020 1:MP80
/// @param[in] val モード
void MP_PRINTER::SetLineMode(int idx, int val) {
	mp_model_list[idx]->SetLineMode(val);
}
/// 1行の長さモード
/// @param[in] idx 0:MP1020 1:MP80
/// @return モード
int  MP_PRINTER::GetLineMode(int idx) {
	return mp_model_list[idx]->GetLineMode();
}
/// 拡大文字モード設定
/// @param[in] idx 0:MP1020 1:MP80
/// @param[in] val モード
void MP_PRINTER::SetWideMode(int idx, int val) {
	mp_model_list[idx]->SetWideMode(val);
}
/// 拡大文字モード
/// @param[in] idx 0:MP1020 1:MP80
/// @return モード
int  MP_PRINTER::GetWideMode(int idx) {
	return mp_model_list[idx]->GetWideMode();
}
/// 文字セット設定
/// @param[in] idx 0:MP1020 1:MP80
/// @param[in] val セット番号
void MP_PRINTER::SetCharSet(int idx, int val) {
	mp_model_list[idx]->SetCharSet(val);
}
/// 文字セット
/// @param[in] idx 0:MP1020 1:MP80
/// @return セット番号
int  MP_PRINTER::GetCharSet(int idx) {
	return mp_model_list[idx]->GetCharSet();
}
/// 1文字の幅を設定
/// @param[in] idx 0:MP1020 1:MP80
/// @param[in] val cpi
void MP_PRINTER::SetCPI(int idx, int val) {
	mp_model_list[idx]->SetCPI(val);
}
/// 1文字の幅
/// @param[in] idx 0:MP1020 1:MP80
/// @return cpi
int  MP_PRINTER::GetCPI(int idx) {
	return mp_model_list[idx]->GetCPI();
}
/// プリンタデバイスに接続
/// @return デバイスコンテキスト
wxDC *MP_PRINTER::AttachPrinterDC() {
	if (mPrintDC == NULL) {
		mPrintDC = new wxPrinterDC(pagesetdlg.GetPrintData());
	}
	return mPrintDC;
}
/// プリンタデバイスから解放
void MP_PRINTER::DetachPrinterDC() {
	if (mPrintDC != NULL) {
		delete mPrintDC;
	}
	mPrintDC = NULL;
}

/// 印字用バッファをクリアする
void MP_PRINTER::ClearPrintBuffer()
{
	bufDw->Clear();
	ht_pos = 0;
}

/// ドキュメントサイズをセット
/// @param[in] scr_ppi 画面ppi (x,y)
/// @param[in] prn_ppi プリンタppi (x,y)
/// @param[in] prect   プリンタ印字限界 (left,top,width,height)
/// @param[in] psize   プリンタ用紙の大きさ (left,top,width,height)
void MP_PRINTER::SetDocumentSize(wxPoint *scr_ppi, wxPoint *prn_ppi, wxRect *prect, wxRect *psize)
{
	dinfo->ph_ppi.Set(prn_ppi->x,
		prn_ppi->y);
	dinfo->ph_size.Set(prect->GetWidth(),
		prect->GetHeight());
	dinfo->ph_offset.Set(-prect->GetLeft(),
		-prect->GetTop());
	dinfo->ph_margin.Set(dinfo->margin_mm.top * dinfo->ph_ppi.y * 10 / 254,
		dinfo->margin_mm.bottom * dinfo->ph_ppi.y * 10 / 254);
	dinfo->vm_size.Set(dinfo->ph_size.width * dinfo->vm_ppi.x / dinfo->ph_ppi.x,
		dinfo->ph_size.height * dinfo->vm_ppi.y / dinfo->ph_ppi.y);
	dinfo->vm_offset.Set(dinfo->ph_offset.x * dinfo->vm_ppi.x / dinfo->ph_ppi.x,
		dinfo->ph_offset.y * dinfo->vm_ppi.y / dinfo->ph_ppi.y);
	dinfo->vm_margin.Set(dinfo->margin_mm.top * dinfo->vm_ppi.y * 10 / 254,
		dinfo->margin_mm.bottom * dinfo->vm_ppi.y * 10 / 254);

#ifdef _DEBUG_LOG
	wxString dbg = _T("SetDocumentSize\n");
	dbg = dbg + _T("  tech: ") + capsTypeStr[dinfo->tech] + _T("\n")
	+ wxString::Format(_T("  margin_mm top: %d mm  bottom: %d mm\n"),dinfo->margin_mm.top, dinfo->margin_mm.bottom)
	+ wxString::Format(_T("  ph_ppi    x: %d  y: %d ppi\n"),dinfo->ph_ppi.x, dinfo->ph_ppi.y)
	+ wxString::Format(_T("  ph_size   w: %d  h: %d px.\n"),dinfo->ph_size.width, dinfo->ph_size.height)
	+ wxString::Format(_T("  ph_offset x: %d  y: %d px.\n"),dinfo->ph_offset.x, dinfo->ph_offset.y)
	+ wxString::Format(_T("  ph_margin top: %d  bottom: %d px.\n"),dinfo->ph_margin.top, dinfo->ph_margin.bottom)
	+ wxString::Format(_T("  vm_ppi    x: %d  y: %d ppi\n"),dinfo->vm_ppi.x, dinfo->vm_ppi.y)
	+ wxString::Format(_T("  vm_size   w: %d  h: %d px.\n"),dinfo->vm_size.width, dinfo->vm_size.height)
	+ wxString::Format(_T("  vm_offset x: %d  y: %d px.\n"),dinfo->vm_offset.x, dinfo->vm_offset.y)
	+ wxString::Format(_T("  vm_margin top: %d  bottom: %d px.\n"),dinfo->vm_margin.top, dinfo->vm_margin.bottom)
	;
	AddDebugLog(dbg);
#endif
}

/// 代替印字データをセットする
/// @param[in] ch  代替印字文字
/// @param[in] len 長さ
void MP_PRINTER::SetSubstChar(wxByte ch, size_t len)
{
	subst_data.ClearPos();
	subst_data.AddData(ch, len);
}

/// 色の濃さを設定
/// @param[in] density 濃さ 0-7
void MP_PRINTER::SetDensity(int density)
{
	int dot = 0;

	for(int i=DpTypeMax; i>=0; i--) {
		color_density_tbl[i] = (wxByte)dot;
		dot += ((COLOR_DENSITY_MAX - density) * 5);
		if (dot > 0xff) dot = 0xff;
	}
	color_density_tbl[0] = 0xff;
}

//
//
//
//
//

/// 描画したデータを保持しておく
/// @param[in] dc デバイスコンテキスト
bool MP_PRINTER::create_cashdata(wxDC *dc)
{
#ifdef USE_CASH_BUFFER
	release_cashdata();

	bmpCashd = new wxBitmap(GetDocWidth(), GetDocHeight(), dc->GetDepth());
	if (!bmpCashd->IsOk()) {
		return false;
	}
	dcCashd = new wxMemoryDC(*bmpCashd);
	if (!dcCashd->IsOk()) {
		delete bmpCashd;
		return false;
	}

	mDataCashd = true;
#endif
	return true;
}
/// 保持しておくバッファを破棄
void MP_PRINTER::release_cashdata()
{
#ifdef USE_CASH_BUFFER
	if (mDataCashd) {
		delete dcCashd;
		delete bmpCashd;
		dcCashd = NULL;
		bmpCashd = NULL;
	}
	mDataCashd = false;
#endif
}

/// プリンターなどのデバイス情報を得る
/// @param[in] dc  デバイスコンテキスト
/// @param[in] inf プリンタなどの情報
void MP_PRINTER::get_device_caps(wxDC *dc, CapsInfoType *inf)
{
	// not used
	// ミリメートル（mm）単位の画面の物理的な幅、高さ。
	wxSize msize = dc->GetSizeMM();
	inf->size_mm.Set(msize.GetWidth(), msize.GetHeight());

	// 論理インチ当たりのピクセル数。
	wxSize ppi = dc->GetPPI();
	inf->ph_ppi.Set(ppi.GetWidth(), ppi.GetHeight());

	if (inf->tech == capsPrinterType) {
		// 印刷デバイス用
		wxPrinterDC *pdc = (wxPrinterDC *)dc;
		wxRect prect = pdc->GetPaperRect();
		// ページの物理的な幅,高さをデバイス単位で表します。たとえば、8.5"×11" の用紙に 600dpi の解像度で印刷するよう設定されているプリンタでは、物理的な幅は 5,100 デバイス単位になります（8.5×600=5,100）。ほとんどの場合、用紙の印字可能領域は用紙の物理的な幅,高さよりも多少小さいこと、そして印字可能領域は物理的な幅,高さを決して上回らないことに注意してください。
		inf->ph_size.Set(prect.GetWidth(), prect.GetHeight());
		// 物理的なページの左端,上端から印刷可能領域の左端,上端までの距離をデバイス単位で表します。
		inf->ph_offset.Set(-prect.GetLeft(), -prect.GetTop());

		inf->vm_size.Set(inf->ph_size.width * inf->vm_ppi.x / inf->ph_ppi.x,
						inf->ph_size.height * inf->vm_ppi.y / inf->ph_ppi.y);
		inf->vm_offset.Set(inf->ph_offset.x * inf->vm_ppi.x / inf->ph_ppi.x,
						inf->ph_offset.y * inf->vm_ppi.y / inf->ph_ppi.y);
	}

	// 余白(pixcel単位)
	inf->ph_margin.Set(inf->margin_mm.top * inf->ph_ppi.y * 10 / 254,
		inf->margin_mm.bottom * inf->ph_ppi.y * 10 / 254);
	inf->vm_margin.Set(inf->margin_mm.top * inf->vm_ppi.y * 10 / 254,
		inf->margin_mm.bottom * inf->vm_ppi.y * 10 / 254);

#ifdef _DEBUG_LOG
	wxString dbg = _T("get_device_caps\n");
	dbg = dbg + _T("  tech: ") + capsTypeStr[inf->tech] + _T("\n")
	+ wxString::Format(_T("  size_mm width: %d mm  height: %d mm\n"),inf->size_mm.width, inf->size_mm.height)
	+ wxString::Format(_T("  margin_mm top: %d mm  bottom: %d mm\n"),inf->margin_mm.top, inf->margin_mm.bottom)
	+ wxString::Format(_T("  ph_ppi    x: %d  y: %d ppi\n"),inf->ph_ppi.x, inf->ph_ppi.y)
	+ wxString::Format(_T("  ph_size   w: %d  h: %d px.\n"),inf->ph_size.width, inf->ph_size.height)
	+ wxString::Format(_T("  ph_offset x: %d  y: %d px.\n"),inf->ph_offset.x, inf->ph_offset.y)
	+ wxString::Format(_T("  ph_margin top: %d  bottom: %d px.\n"),inf->ph_margin.top, inf->ph_margin.bottom)
	+ wxString::Format(_T("  vm_ppi    x: %d  y: %d ppi\n"),inf->vm_ppi.x, inf->vm_ppi.y)
	+ wxString::Format(_T("  vm_size   w: %d  h: %d px.\n"),inf->vm_size.width, inf->vm_size.height)
	+ wxString::Format(_T("  vm_offset x: %d  y: %d px.\n"),inf->vm_offset.x, inf->vm_offset.y)
	+ wxString::Format(_T("  vm_margin top: %d  bottom: %d px.\n"),inf->vm_margin.top, inf->vm_margin.bottom)
	+ wxString::Format(_T("  win_size w: %d  h: %d px.\n"),win_size.width, win_size.height)
	;
	AddDebugLog(dbg);
#endif
}

/// 画面用に仮想で紙サイズを設定
/// @param[in] inf  プリンタなどの情報
/// @param[in] ppix x pixel per inch
/// @param[in] ppiy y pixel per inch
void MP_PRINTER::set_paper_size(CapsInfoType *inf, int ppix, int ppiy)
{
#ifdef _DEBUG_LOG
	wxString dbg = _T("set_paper_size");
	dbg += _T("  tech: ") + capsTypeStr[inf->tech];
	AddDebugLog(dbg);
#endif
	// 解像度を指定（拡大・縮小率を計算した値をセット）
	inf->ph_ppi.Set(ppix, ppiy);
	// 画面の場合はDCの左上が原点となるのでオフセットは常に0にする
	inf->ph_offset.Clear();
	inf->vm_offset.Clear();

	wxPrinterDC pdc(printdata);
#ifdef __WXGTK__
	// TODO: GTKではGetPaperRectに失敗する。
	if (0) {
#else
	if (pdc.IsOk()) {
#endif
		// プリンター情報から計算
		wxSize msize = pdc.GetSizeMM();
		wxRect pdc_rect = pdc.GetPaperRect();
		wxSize pdc_ppi = pdc.GetPPI();
#ifdef _DEBUG_LOG
		wxSize pdc_size = pdc.GetSize();
		dbg = _T("set_paper_size --- get from printerDC\n");
		dbg += wxString::Format(_T("  pdc_rect left: %d  top: %d px.\n"),pdc_rect.GetLeft(),pdc_rect.GetTop())
		+ wxString::Format(_T("  pdc_rect w: %d  h: %d px.\n"),pdc_rect.GetWidth(),pdc_rect.GetHeight())
		+ wxString::Format(_T("  pdc_size w: %d  h: %d px.\n"),pdc_size.GetWidth(),pdc_size.GetHeight())
		+ wxString::Format(_T("  pdc_ppi  x: %d  y: %d ppi"),pdc_ppi.GetX(),pdc_ppi.GetY());
		AddDebugLog(dbg);
#endif
		// 設定がなければA4縦
		if (pdc_rect.GetWidth() <= 0) pdc_rect.SetWidth(2100 * pdc_ppi.GetX() / 254);
		if (pdc_rect.GetHeight() <= 0) pdc_rect.SetHeight(2970 * pdc_ppi.GetY() / 254);

		inf->size_mm.Set(msize.GetWidth(), msize.GetHeight());

		inf->ph_size.Set(pdc_rect.GetWidth() * inf->ph_ppi.x / pdc_ppi.GetX(),
			pdc_rect.GetHeight() * inf->ph_ppi.y / pdc_ppi.GetY());

		inf->vm_size.Set(pdc_rect.GetWidth() * inf->vm_ppi.x / pdc_ppi.GetX(),
			pdc_rect.GetHeight() * inf->vm_ppi.y / pdc_ppi.GetY());

#if 0
		// 余白はプリンターのサイズから計算する
		inf->ph_margin.Top(- pdc_rect.GetTop());
		inf->ph_margin.Bottom(pdc_rect.GetHeight() - pdc_size.GetHeight() + pdc_rect.GetTop());
		inf->margin_mm.Top(inf->ph_margin.top * 254 / pdc_ppi.GetY() / 10);
		inf->margin_mm.Bottom(inf->ph_margin.bottom * 254 / pdc_ppi.GetY() / 10);
		vm_margin.Top(inf->ph_margin.top * inf->vm_ppi.x / pdc_ppi.GetX());
		vm_margin.Bottom(inf->ph_margin.bottom * inf->vm_ppi.y / pdc_ppi.GetY());
		inf->ph_margin.Top(inf->ph_margin.top * inf->ph_ppi.y / pdc_ppi.GetY());
		inf->ph_margin.Bottom(inf->ph_margin.bottom * inf->ph_ppi.y / pdc_ppi.GetY());
#else
		inf->vm_margin.Set(inf->margin_mm.top * inf->vm_ppi.y * 10 / 254,
			inf->margin_mm.bottom * inf->vm_ppi.y * 10 / 254);
		inf->ph_margin.Set(inf->margin_mm.top * inf->ph_ppi.y * 10 / 254,
			inf->margin_mm.bottom * inf->ph_ppi.y * 10 / 254);
#endif
	} else {
		// ページ設定の紙サイズに合わせる
		wxSize psize = pagesetdlg.GetPaperSize();
#ifdef _DEBUG_LOG
		dbg = _T("set_paper_size -- get from pagesetdlg\n");
		dbg += wxString::Format(_T("  paper_size w: %d  h: %d"),psize.GetWidth(),psize.GetHeight());
		AddDebugLog(dbg);
#endif
		// 設定がなければA4縦
		if (psize.GetWidth() <= 0) psize.SetWidth(210);
		if (psize.GetHeight() <= 0) psize.SetHeight(297);

		if (pagesetdlg.GetPrintData().GetOrientation() == wxLANDSCAPE) {
			// 横
			int w = psize.GetWidth();
			psize.SetWidth(psize.GetHeight());
			psize.SetHeight(w);
		}
		inf->size_mm.Set(psize.GetWidth(), psize.GetHeight());

		inf->ph_size.Set(psize.GetWidth() * inf->ph_ppi.x * 10 / 254,
			psize.GetHeight() * inf->ph_ppi.y * 10 / 254);
		inf->vm_size.Set(psize.GetWidth() * inf->vm_ppi.x * 10 / 254,
			psize.GetHeight() * inf->vm_ppi.y * 10 / 254);

		inf->ph_margin.Set(inf->margin_mm.top * inf->ph_ppi.y * 10 / 254,
			inf->margin_mm.bottom * inf->ph_ppi.y * 10 / 254);
		inf->vm_margin.Set(inf->margin_mm.top * inf->vm_ppi.y * 10 / 254,
			inf->margin_mm.bottom * inf->vm_ppi.y * 10 / 254);
	}

	// バッファを再作成
#ifdef USE_PAGE_BUFFER
	delete dcPage;
	wxBitmap bmpPage(inf->ph_size.width, inf->ph_size.height, 24);
	if (!bmpPage.IsOk()) {
		gErrInfo.SetInfo(__FILE__, __LINE__, myError, myErrSystem, _T("Cannot create bmpPage."));
		gErrInfo.ShowMsgBox();
		return;
	}
	dcPage = new wxMemoryDC(bmpPage);
	dcPage->Clear();
#endif

#ifdef _DEBUG_LOG
	dbg = _T("")
	+ wxString::Format(_T("  size_mm width: %d mm  height: %d mm\n"),inf->size_mm.width, inf->size_mm.height)
	+ wxString::Format(_T("  margin_mm top: %d  bottom: %d mm\n"),inf->margin_mm.top, inf->margin_mm.bottom)
	+ wxString::Format(_T("  ph_ppi    x: %d  y: %d ppi\n"),inf->ph_ppi.x, inf->ph_ppi.y)
	+ wxString::Format(_T("  ph_size   w: %d  h: %d px.\n"),inf->ph_size.width, inf->ph_size.height)
	+ wxString::Format(_T("  ph_offset x: %d  y: %d px.\n"),inf->ph_offset.x, inf->ph_offset.y)
	+ wxString::Format(_T("  ph_margin top: %d  bottom: %d px.\n"),inf->ph_margin.top, inf->ph_margin.bottom)
	+ wxString::Format(_T("  vm_ppi    x: %d  y: %d ppi\n"),inf->vm_ppi.x, inf->vm_ppi.y)
	+ wxString::Format(_T("  vm_size   w: %d  h: %d px.\n"),inf->vm_size.width, inf->vm_size.height)
	+ wxString::Format(_T("  vm_offset x: %d  y: %d px.\n"),inf->vm_offset.x, inf->vm_offset.y)
	+ wxString::Format(_T("  vm_margin top: %d  bottom: %d px.\n"),inf->vm_margin.top, inf->vm_margin.bottom)
	;
	AddDebugLog(dbg);
#endif
}

//
//
//

/// プリンター選択ダイアログ を表示
/// @param[in] win 親ウィンドウ
/// @return true
bool MP_PRINTER::select_printer(wxWindow *win)
{
	/* プリンター選択ダイアログ */
#ifdef __WXGTK__
	printdlg.SetSelection(false);
#else
	printdlg.SetSelection(true);
#endif
	printdlg.SetNoCopies(1);
	printdlg.SetFromPage(1);
	printdlg.SetToPage(mMaxPage);
	printdlg.SetMinPage(1);
	printdlg.SetMaxPage(mMaxPage);

#if PRINT_FRAMEWORK == 1
	// TODO: MacでMyPrinterを使用するとStretchBlitに失敗して印刷できないので
	// こちらを使用する
#if 1
	wxPrintDialog dlg(win, &printdlg);
	if (dlg.ShowModal() != wxID_OK) {
		return false;
	}
	printdlg = dlg.GetPrintDialogData();
	printdata = printdlg.GetPrintData();
	pagesetdlg.SetPrintData(printdata);
#else
	wxPrinter printer(&printdlg);
	wxDC *pdc = printer.PrintDialog(win);
	if (pdc == NULL) {
        if (wxPrinter::GetLastError() == wxPRINTER_ERROR) {
			gErrInfo.SetInfo(__FILE__, __LINE__, myError, myErrPrinting);
			gErrInfo.ShowMsgBox(win);
        }
		return false;
	}
	printdlg = printer.GetPrintDialogData();
	printdata = printdlg.GetPrintData();
	pagesetdlg.SetPrintData(printdata);
#endif
#else
	wxPrinter printer(&printdlg);
	MpPrintout printout(this, text_file, &pagesetdlg);
    if (!printer.Print(win, &printout, true /*prompt*/)) {
        if (wxPrinter::GetLastError() == wxPRINTER_ERROR) {
			gErrInfo.SetInfo(__FILE__, __LINE__, myError, myErrPrinting);
			gErrInfo.ShowMsgBox(win);
        }
		// TODO: Winでは常にここに来るのだが。。。
//		return false;
	}
	printdlg = printer.GetPrintDialogData();
	printdata = printdlg.GetPrintData();
	pagesetdlg.SetPrintData(printdata);
#endif

	return true;
}

//
//
//

/// プリンターページ設定ダイアログ を表示
/// @param[in] win 親ウィンドウ
/// @return true
bool MP_PRINTER::set_printer_page(wxWindow *win)
{
	/* プリンターページ設定ダイアログ */
	pagesetdlg.SetDefaultMinMargins(true);

	wxPageSetupDialog dlg(win, &pagesetdlg);
	if (dlg.ShowModal() != wxID_OK) {
		return false;
	}
	pagesetdlg = dlg.GetPageSetupDialogData();

	printdata = pagesetdlg.GetPrintData();
	printdlg.SetPrintData(printdata);

	wxPoint pMarginTopLeft = pagesetdlg.GetMarginTopLeft();
	wxPoint pMarginBottomRight = pagesetdlg.GetMarginBottomRight();
	for(int i=0; i<capsUnknownType; i++) {
		device_info[i].margin_mm.Set(pMarginTopLeft.y, pMarginBottomRight.y);
	}

#ifdef _DEBUG_LOG
	wxSize sz = pagesetdlg.GetPaperSize();
	wxString dbg(_T("set_printer_page\n"));
	dbg += wxString::Format(_T("  PaperId: %d\n"),pagesetdlg.GetPaperId());
	dbg += wxString::Format(_T("  Size : w:%d h:%d mm\n")
		,sz.GetWidth(),sz.GetHeight());
	dbg += wxString::Format(_T("  Mergin: t:%d b:%d l:%d r:%d mm\n")
		,pMarginTopLeft.y,pMarginBottomRight.y,pMarginTopLeft.x,pMarginBottomRight.x);
	dbg += _T("  Printer: ") + printdata.GetPrinterName() + _T("\n");
	AddDebugLog(dbg);
#endif

	// 設定した紙サイズに合わせて画面も再設定
	set_paper_size(&device_info[capsDisplayType]
		, device_info[capsDisplayType].ph_ppi.x, device_info[capsDisplayType].ph_ppi.y);

	release_cashdata();
	return true;
}

//
//
//

/// マージン設定ダイアログ を表示
/// @param[in] win 親ウィンドウ
/// @return true / false
bool MP_PRINTER::set_margin_page(wxWindow *win)
{
	pagesetdlg.SetDefaultMinMargins(true);

	MarginBox dlg(win, wxID_ANY, &pagesetdlg);
	if (dlg.ShowModal() != wxID_OK) {
		return false;
	}
	printdata = pagesetdlg.GetPrintData();
	printdlg.SetPrintData(printdata);

	wxPoint pMarginTopLeft = pagesetdlg.GetMarginTopLeft();
	wxPoint pMarginBottomRight = pagesetdlg.GetMarginBottomRight();
	for(int i=0; i<capsUnknownType; i++) {
		device_info[i].margin_mm.Set(pMarginTopLeft.y, pMarginBottomRight.y);
	}

	// 設定した紙サイズに合わせて画面も再設定
	set_paper_size(&device_info[capsDisplayType]
		, device_info[capsDisplayType].ph_ppi.x, device_info[capsDisplayType].ph_ppi.y);

	release_cashdata();
	return true;
}

///
/// 制御コードの解析
///
/// @param [in] data 文字列
/// @return >0:制御コード 0:一般コード -1:連続改行中や代替データ出力中
int MP_PRINTER::parse_ctrl_code(wxByte *data)
{
	int bytes = 0;

	// 連続改行中や代替データ出力中はコードの解析をしない
	if (line_feed_number > 0 || subst_data.GetWritePos() > 0) {
		return -1;
	}
	// 制御コード（機種依存）
	bytes = mp_model->ParseCtrlCode(data, bufDw, &feed);
	if (bytes != 0) {
		bufDw->Line().width = (mp_model->GetLineWidth());	// 1行の幅
		line_feed_height = mp_model->GetLineFeedHeight();	// 改行幅
		paper_feed_height = mp_model->GetPaperFeedHeight();	// 紙送り幅
	}
	return bytes;
}

///
/// データを描画する
///
/// @param[in] dc        デバイスコンテキスト
/// @param[in] inf       プリンタなどの情報
/// @param[in] draw_part 追加されたデータのみ描画するか
bool MP_PRINTER::draw_data(wxDC *dc, CapsInfoType *inf, bool draw_part)
{
#ifdef USE_PAGE_BUFFER
	draw_start_document(dcPage, inf);
	for(int page = 1; page < 100 ; page++) {
		draw_start_page(dcPage, inf, page);
		draw_page(dcPage, inf, page);
		if (dc != NULL) {
			dc->Blit(win_offset.x, inf->ph_size.height * (page - 1), inf->ph_size.width, inf->ph_size.height, dcPage, 0, 0);
		}
		if (!draw_end_page(dc, inf, page)) {
			break;
		}
	}
#else
	draw_start_document(dc, inf);
	for(int page = 1; page < 100 ; page++) {
		draw_start_page(dc, inf, page, draw_part);
		draw_page(dc, inf, page);
		if (!draw_end_page(dc, inf, page)) {
			break;
		}
	}
#endif
	draw_end_document(dc, inf);

	return true;
}

///
/// 印刷する
///
/// @param[in] dc    デバイスコンテキスト
bool MP_PRINTER::print_data(wxDC *dc)
{
	PrintingStart(dc, false);
	PrintStartDocument(dc);
	for(int page = printdlg.GetFromPage(); page <= printdlg.GetToPage(); page++) {
		PrintPage(dc, page);
	}
	PrintEndDocument(dc);
	PrintingEnd(dc);
	return true;
}

/// ドキュメント描画開始時処理
/// @param[in] dc    デバイスコンテキスト
/// @param[in] inf   プリンタなどの情報
bool MP_PRINTER::draw_start_document(wxDC *dc, CapsInfoType *inf)
{
	// 画面上の縦横比
	mp_model->GetDispAspectRatio(&dsr);
	// 開始時のパラメータをセット
	mp_model->PopSetting();

	// 1行の幅セット
	bufDw->ClearRect(mp_model->GetLineWidth());	// 1行の幅

	// 印刷範囲は拡大縮小を考慮した値となる->Dp
	// 印刷範囲は拡大縮小を考慮した値とならない->Vm
	// 印刷範囲の最大幅
	rVmMax.Set(MP_PRN_BUF_WIDTH, 0);

	switch(mRotate & 1) {
	case 1:
		// 印刷範囲左端
		rVm.left  = ((inf->vm_size.width - rVmMax.width) / 2 - inf->vm_offset.y);
		break;
	default:
		// 印刷範囲左端
		rVm.left  = ((inf->vm_size.width - rVmMax.width) / 2 - inf->vm_offset.x);
		break;
	}
	// 印刷範囲右端
	rVm.right = rVm.left + rVmMax.width;
	// 印刷範囲上端
	rVm.top = (inf->vm_margin.top - inf->vm_offset.y);
	if (rVm.top < 0) rVm.top = 0;
	// 印刷範囲下端
	rVm.bottom = ((int)calc_vm_bottom(inf, 0));

	vm_y = rVm.top;		// 印字行(ページ内)
	vm_ry = rVm.top;	// 印字行(画面へ出力する場合、上端からの累積座標となる)

	bufDm->MaxR().Clear();		// 合成用バッファに書かれたデータ

	// 画面表示で描画処理を行う範囲
	rVmIn.left = 0;

	switch(mRotate & 1) {
	case 1:
		rVmIn.top =	-48 - inf->vm_margin.top - inf->vm_margin.bottom + (dp_offset.x * inf->vm_ppi.y / inf->ph_ppi.y);
		rVmIn.right = win_size.height * inf->vm_ppi.y / inf->ph_ppi.y;
		rVmIn.bottom = 48 + (win_size.width + dp_offset.x) * inf->vm_ppi.y / inf->ph_ppi.y;

		rVmIn.top = (rVmIn.top * dsr.y / dsr.x);
		rVmIn.bottom = (rVmIn.bottom * dsr.y / dsr.x);
		break;
	default:
		rVmIn.top =	-48 - inf->vm_margin.top - inf->vm_margin.bottom + (dp_offset.y * inf->vm_ppi.y / inf->ph_ppi.y);
		rVmIn.right = win_size.width * inf->vm_ppi.y / inf->ph_ppi.y;
		rVmIn.bottom = 48 + (win_size.height + dp_offset.y) * inf->vm_ppi.y / inf->ph_ppi.y;

		rVmIn.top = (rVmIn.top * dsr.y / dsr.x);
		rVmIn.bottom = (rVmIn.bottom * dsr.y / dsr.x);
		break;
	}

#ifdef _DEBUG_LOG
	{
		wxString msg;
		msg = wxT("draw_start_document\n");
		msg += wxString::Format(wxT("rVmMax: w:%d h:%d\n"), rVmMax.width, rVmMax.height);
		msg += wxString::Format(wxT("rVm: l:%d r:%d t:%d b:%d\n"), rVm.left, rVm.right, rVm.top, rVm.bottom);
		msg += wxString::Format(wxT("rVmIn: l:%d r:%d t:%d b:%d\n"), rVmIn.left, rVmIn.right, rVmIn.top, rVmIn.bottom);
		AddDebugLog(msg);
	}
#endif

	// 画面内か
	draw_win = false;
	// 印刷対象ページか
	print_page = false;
	// 紙送りフラグ
	feed = 0;
	// 改行数
	line_feed_number = 0;
	// 垂直、水平タブ位置初期化
	vt_line_number = 0;
	ht_pos = 0;
	// 改ページ指示
	change_page = false;
	// ページ数
	mCurPage = 1;
	// ページ終りか
	endPage = false;
	// 代替テキスト
	subst_data.Clear();

	if (inf->tech == capsDisplayType) {
		// ラスタディスプレイの場合

		if (dc != NULL) {
			// 画面に描画する場合の描画位置
#ifdef USE_CASH_BUFFER
			if (mUseCash) {
				win_offset.SetX(0);
				win_offset.SetY(0);
			} else
#endif
			{
				switch(mRotate & 1) {
				case 1:
					win_offset.SetX(0);
					win_offset.SetY((win_size.height - inf->ph_size.width) / 2);
#ifndef USE_PAGE_BUFFER
					rVm.AddLeft(win_offset.y * inf->vm_ppi.y / inf->ph_ppi.y);
					rVm.AddRight(win_offset.y * inf->vm_ppi.y / inf->ph_ppi.y);
#endif
					break;
				default:
					win_offset.SetX((win_size.width - inf->ph_size.width) / 2);
					win_offset.SetY(0);
#ifndef USE_PAGE_BUFFER
					rVm.AddLeft(win_offset.x * inf->vm_ppi.x / inf->ph_ppi.x);
					rVm.AddRight(win_offset.x * inf->vm_ppi.x / inf->ph_ppi.x);
#endif
					break;
				}
			}

			// 描画用ペンの設定
			dc->SetPen(wxPen(wxColour(0, 0, 0),1));
			// 描画用フォントの設定
			set_sys_font(dc, inf->ph_ppi.x / 5, inf->ph_ppi.x / 3);
		}

		// 画面内か
		draw_win = is_invmwindow(dc, inf, vm_ry);

		print_page = true;

	} else if (inf->tech == capsPrinterType) {
		// ラスタプリンタの場合
		draw_win = true;

		// 印刷開始
		dc->StartDoc(text_file);

	} else if (inf->tech == capsPreviewType) {
		// 印刷プレビューの場合
		draw_win = true;

	}

	if (draw_win) {
		bufDm->Clear();
		bufDw->Clear();
	}
	line_feed_height = mp_model->GetLineFeedHeight();
	paper_feed_height = mp_model->GetPaperFeedHeight();

	text_data.SetReadPos(0);

#ifdef _DEBUG_LOG
	{
		wxString msg;
		msg = wxString::Format(wxT("win_size: w:%d h:%d  win_offset: x:%d y:%d\n"), win_size.width, win_size.height, win_offset.x, win_offset.y);
		msg += wxString::Format(wxT("rVm: l:%d r:%d t:%d b:%d\n"), rVm.left, rVm.right, rVm.top, rVm.bottom);
		AddDebugLog(msg);
	}
#endif

	return true;
}

/// ページ開始時処理
/// @param[in] dc          デバイスコンテキスト
/// @param[in] inf         プリンタなどの情報
/// @param[in] target_page 印刷対象ページ
/// @param[in] draw_part   追加されたデータのみ描画するか
bool MP_PRINTER::draw_start_page(wxDC *dc, CapsInfoType *inf, int target_page, bool draw_part)
{
	if (inf->tech == capsDisplayType) {
		// ラスタディスプレイの場合
		vm_ry = inf->vm_size.height * (mCurPage - 1) + rVm.top;	// 先頭
		// 画面内か
		draw_win = is_invmwindow(dc, inf, vm_ry);

		// ページ番号＆枠の表示
		if (dc != NULL) {
			draw_page_no(dc, inf, mCurPage, draw_part);
		}
	} else if (inf->tech == capsPrinterType) {
		// ラスタプリンタの場合
		vm_ry = rVm.top;	// 先頭

		if (target_page == mCurPage) {
			print_page = true;
			dc->StartPage();
		}
	} else if (inf->tech == capsPreviewType) {
		// 印刷プレビューの場合
		vm_ry = rVm.top;	// 先頭

		if (target_page == mCurPage) {
			print_page = true;
		}
	}
	vm_y = rVm.top;	// 先頭
	change_page = false;

	return true;
}

/// ページ描画処理
/// @param[in] dc          デバイスコンテキスト
/// @param[in] inf         プリンタなどの情報
/// @param[in] target_page 印刷対象ページ
bool MP_PRINTER::draw_page(wxDC *dc, CapsInfoType *inf, int target_page)
{
//	AddDebugLog(wxString::Format(_T("draw_page: %d"), mPage));

	while(text_data.Remain() > 0) {
		// 制御コードか？
		int step = parse_ctrl_code(text_data.ReadData());
		if (step > 0) {
			text_data.AddReadPos(step - 1);
			if (feed == 0) {
				// 改行や改ページ以外の制御コードの場合は印字スキップ
				text_data.AddReadPos(1);
				continue;
			}
		}
		// 改行や復帰の指示なしで、バッファがいっぱいになった
		if (bufDw->IsRightSide() && (feed & (MP_BIT_CR | MP_BIT_LF)) == 0) {
			feed |= MP_BIT_LF;	// 改行を指示
			if (line_feed_number == 0) line_feed_number = 1;
		}
		// バッファがいっぱいになった or 紙送り/改行
		if (feed != 0) {
			// 画面内であれば描画処理
			put_buffer(dc, inf, draw_win && print_page, false);

			// 復帰 (改行、改ページも含む）
			if ((feed & ~MP_BIT_PF) != 0) {
				bufDw->NewLine();
				ht_pos = 0;

				// 画面内であれば描画処理
				if (draw_win) {
					// 描画用バッファも復帰にあわせてクリア
					bufDw->LeftShift();
				}
			} else {
				// 画面内であれば描画処理
				if (draw_win) {
					// 描画用バッファ印字した分はクリア
					bufDw->ClearLeft();
				}
			}

			// 改行 (改ページ、紙送りも含む)
			if ((feed & ~MP_BIT_CR) != 0) {
				// 改行幅
				double vm_next_line;
				if ((feed & MP_BIT_PF) == 0) {
					// 改行
					vm_next_line = line_feed_height;
					vt_line_number++;
				} else {
					// 紙送り
					vm_next_line = paper_feed_height;
					paper_feed_height = 0.0;
					mp_model->SetPaperFeedHeight(0.0);
				}
				vm_y_prev = vm_y;
				vm_y += vm_next_line;
				vm_ry_prev = vm_ry;
				vm_ry += vm_next_line;

				// 印刷範囲下端を再計算
				rVm.bottom = ((int)calc_vm_bottom(inf, bufDm->MaxR().bottom));

				// 合成バッファもクリア
				// MP_PRN_BUF_HEIGHT pixcel未満では印字が重なるように現在行の一部を残す
				if (draw_win) {
					bufDm->UpperShift((int)(vm_ry - vm_ry_prev));
				}

				// 画面内か
				draw_win = is_invmwindow(dc, inf, vm_ry);

			}

			// 改行or復帰時のイベント
			mp_model->OnLineFeed();

			// 印刷範囲の下端に達した場合、改ページ指示
			if ((feed & MP_BIT_FF) || (vm_y > rVm.bottom && rVm.bottom > 0)) {
				change_page = true;
				// 次ページの先頭位置を計算
				vm_ry = inf->vm_size.height * mCurPage + rVm.top;
				// 画面内か
				draw_win = is_invmwindow(dc, inf, vm_ry);
			}
		} // end of 改行処理

		//
		// 印字データを描画エリアに出力
		//
		int readed = 1;
		if (subst_data.GetWritePos() > 0) {
			// 代替データを出力
			readed = mp_model->PutData(subst_data.ReadData(), bufDw, draw_win);
			ht_pos++;
			subst_data.AddReadPos(readed);
			if (subst_data.Remain() >= 0) {
				subst_data.SetWritePos(0);
			}
		} else if (line_feed_number > 0) {
			// 連続改行中は印字データの出力はしない
			line_feed_number--;
			if (line_feed_number == 0) {
				if (step == 0) {
					// 一般の文字で自動改行した場合は印字
					readed = mp_model->PutData(text_data.ReadData(), bufDw, draw_win);
					ht_pos++;
				}
				text_data.AddReadPos(readed);
			}
		} else {
			// 文字印字
			readed = mp_model->PutData(text_data.ReadData(), bufDw, draw_win);
			ht_pos++;
			text_data.AddReadPos(readed);
		}

		// 改ページ
		if (change_page == true && (bufDw->Rect().width > 0 || (feed & MP_BIT_LF))) {
			break;
		}
	}
	// 最終ページの判断：
	// 改ページ指示があったがその後ろに印字するデータがない場合
	// 印字するデータがなくなった場合
	if ((change_page == true && bufDw->Rect().width == 0 && (feed & MP_BIT_LF) == 0) || (change_page == false && text_data.Remain() >= 0)) {
		endPage = true;

		// バッファの残りを出力
		if (bufDw->Rect().width > 0 || bufDm->MaxR().right > bufDm->MaxR().left) {
			put_buffer(dc, inf, draw_win && print_page, true);
		}
		// 改行or復帰時のイベント
		mp_model->OnLineFeed();

		return false;
	}
	return true;
}

/// ページ終了時処理
/// @param[in] dc          デバイスコンテキスト
/// @param[in] inf         プリンタなどの情報
/// @param[in] target_page 印刷対象ページ
bool MP_PRINTER::draw_end_page(wxDC *dc, CapsInfoType *inf, int target_page)
{
	if (inf->tech == capsDisplayType) {
		// ラスタディスプレイの場合

	} else if (inf->tech == capsPrinterType) {
		// ラスタプリンタの場合
		// 改ページ
		if (target_page == mCurPage) {
			dc->EndPage();
		}
		print_page = false;

	} else if (inf->tech == capsPreviewType) {
		// 印刷プレビューの場合
		print_page = false;
	}
	// 最終ページの場合
	if (endPage) {
		// 印字終わり
		return false;
	}

	mCurPage++;

	return true;
}

/// ドキュメント描画終了時処理
/// @param[in] dc  デバイスコンテキスト
/// @param[in] inf プリンタなどの情報
bool MP_PRINTER::draw_end_document(wxDC *dc, CapsInfoType *inf)
{
	if (inf->tech == capsDisplayType) {
		// ラスタディスプレイの場合
		// ページ数を設定
		printdlg.SetNoCopies(1);
		printdlg.SetFromPage(1);
		printdlg.SetToPage(mCurPage);
		printdlg.SetMinPage(1);
		printdlg.SetMaxPage(mCurPage);
		mMaxPage = mCurPage;

	} else if (inf->tech == capsPrinterType) {
		// ラスタプリンタの場合
		// 印刷終了
		dc->EndDoc();
	} else if (inf->tech == capsPreviewType) {
		// 印刷プレビューの場合

	}

	return true;
}

#if 0
/// 画面内か
bool MP_PRINTER::is_inwindow(wxDC *dc, CapsInfoType *inf, double y)
{
	bool rc = true;

	if (dc == NULL) {
		return false;
	}
#ifdef USE_CASH_BUFFER
	if (mUseCash) {
	} else
#endif
	{
		if (inf->tech == capsDisplayType) {
			// ラスタディスプレイの場合
			// 画面内か
//			int draw_winy = (int)(y * dsr.x / dsr.y);
//			rc = (draw_winy >= rDwIn.top && draw_winy <= rDwIn.bottom);
		}
	}
	return rc;
}
#endif

/// 画面内か
/// @param[in] dc  デバイスコンテキスト
/// @param[in] inf プリンタなどの情報
/// @param[in] y   Y座標
bool MP_PRINTER::is_invmwindow(wxDC *dc, CapsInfoType *inf, double y)
{
	bool rc = true;

	if (dc == NULL) {
		return false;
	}
#ifdef USE_CASH_BUFFER
	if (mUseCash) {
	} else
#endif
	{
		if (inf->tech == capsDisplayType) {
			// ラスタディスプレイの場合
			// 画面内か
			if (mRotate & 2) {
				int draw_winy = inf->vm_size.height * mMaxPage - (int)y;
				rc = (draw_winy >= rVmIn.top && draw_winy <= rVmIn.bottom);
			} else {
				int draw_winy = (int)y;
				rc = (draw_winy >= rVmIn.top && draw_winy <= rVmIn.bottom);
			}
		}
	}
	return rc;
}

/// バッファの内容をデバイスに出力
/// @param[in] dc   デバイスコンテキスト
/// @param[in] inf  プリンタなどの情報
/// @param[in] draw 印刷対象か
/// @param[in] last 印刷最後か
void MP_PRINTER::put_buffer(wxDC *dc, CapsInfoType *inf, bool draw, bool last)
{
	MyRectWH sp;
	MyRectWH dp;
	int dw_left, dw_width, dw_bottom;
	int im_width = 1;
	int im_height = 1;

	// 合成する範囲の決定
	dw_left  = (bufDw->Width() - bufDw->Line().width) / 2;
	dw_width = wxMin(bufDw->Rect().width, bufDw->Line().width);
	dw_bottom = bufDw->Rect().height;
	// 合成
	if (draw && bufDw->IsUpdatedData()) {
		bufDw->CopyTo(bufDm, dw_left, 0, dw_width, dw_bottom);
	}

	// 出力範囲の決定
	// 一番下端を決定
	if (last || dw_bottom > MP_PRN_BUF_HEIGHT) {
		dw_bottom = MP_PRN_BUF_HEIGHT;
	}
	bufDm->SetMaxTopBottom(0, dw_bottom);

	// 一番左端と右端を決定
	bufDm->SetMaxLeftRight(dw_left, dw_left + bufDw->Rect().width);

	// 出力
	if (draw) {
		if (inf->tech == capsDisplayType) {
			// 画面描画
			switch(mRotate & 3) {
			case 3:
				im_width = bufDm->Height();
				im_height = bufDm->Width();
				sp.Set(bufDm->Height() - bufDm->MaxR().bottom,
					bufDm->MaxR().left,
					bufDm->MaxR().bottom - bufDm->MaxR().top,
					bufDm->MaxR().right - bufDm->MaxR().left);
				dp.Set(
#ifdef USE_PAGE_BUFFER
					(int)(((double)inf->vm_size.height * mMaxPage - vm_y - bufDm->MaxR().bottom) * inf->ph_ppi.y * dsr.x / inf->vm_ppi.y / dsr.y),
#else
					(int)(((double)inf->vm_size.height * mMaxPage - vm_ry - bufDm->MaxR().bottom) * inf->ph_ppi.y * dsr.x / inf->vm_ppi.y / dsr.y),
#endif
					(sp.top + rVm.left) * inf->ph_ppi.x / inf->vm_ppi.x,
					bufDm->MaxR().bottom * inf->ph_ppi.y * dsr.x / inf->vm_ppi.y / dsr.y,
					sp.height * inf->ph_ppi.x / inf->vm_ppi.x);
				break;
			case 2:
				im_width = bufDm->Width();
				im_height = bufDm->Height();
				sp.Set(bufDm->Width() - bufDm->MaxR().right,
					bufDm->Height() - bufDm->MaxR().bottom,
					bufDm->MaxR().right - bufDm->MaxR().left,
					bufDm->MaxR().bottom - bufDm->MaxR().top);
				dp.Set(
					(sp.left + rVm.left) * inf->ph_ppi.x / inf->vm_ppi.x,
#ifdef USE_PAGE_BUFFER
					(int)(((double)inf->vm_size.height * mMaxPage - vm_y - bufDm->MaxR().bottom) * inf->ph_ppi.y * dsr.x / inf->vm_ppi.y / dsr.y),
#else
					(int)(((double)inf->vm_size.height * mMaxPage - vm_ry - bufDm->MaxR().bottom) * inf->ph_ppi.y * dsr.x / inf->vm_ppi.y / dsr.y),
#endif
					sp.width * inf->ph_ppi.x / inf->vm_ppi.x,
					bufDm->MaxR().bottom * inf->ph_ppi.y * dsr.x / inf->vm_ppi.y / dsr.y);
				break;
			case 1:
				im_width = bufDm->Height();
				im_height = bufDm->Width();
				sp.Set(bufDm->MaxR().top,
					bufDm->Width() - bufDm->MaxR().right,
					bufDm->MaxR().bottom - bufDm->MaxR().top,
					bufDm->MaxR().right - bufDm->MaxR().left);
				dp.Set(
#ifdef USE_PAGE_BUFFER
					(int)(vm_y * inf->ph_ppi.y * dsr.x / inf->vm_ppi.y / dsr.y),
#else
					(int)(vm_ry * inf->ph_ppi.y * dsr.x / inf->vm_ppi.y / dsr.y),
#endif
					(sp.top + rVm.left) * inf->ph_ppi.x / inf->vm_ppi.x,
					bufDm->MaxR().bottom * inf->ph_ppi.y * dsr.x / inf->vm_ppi.y / dsr.y,
					sp.height * inf->ph_ppi.x / inf->vm_ppi.x);
				break;
			default:
				im_width = bufDm->Width();
				im_height = bufDm->Height();
				sp.Set(bufDm->MaxR().left,
					bufDm->MaxR().top,
					bufDm->MaxR().right - bufDm->MaxR().left,
					bufDm->MaxR().bottom - bufDm->MaxR().top);
				dp.Set(
					(sp.left + rVm.left) * inf->ph_ppi.x / inf->vm_ppi.x,
#ifdef USE_PAGE_BUFFER
					(int)(vm_y * inf->ph_ppi.y * dsr.x / inf->vm_ppi.y / dsr.y),
#else
					(int)(vm_ry * inf->ph_ppi.y * dsr.x / inf->vm_ppi.y / dsr.y),
#endif
					sp.width * inf->ph_ppi.x / inf->vm_ppi.x,
					bufDm->MaxR().bottom * inf->ph_ppi.y * dsr.x / inf->vm_ppi.y / dsr.y);
				break;
			}
#ifdef USE_TRIMING_AREA
			// 左端がはみ出る場合のトリミング
			if (dp.left < 1) {
				dp.left = 1;
				bufDm->MaxR().left = dp.left * inf->vm_ppi.x / inf->ph_ppi.x - rVm.left;
			}
			// 右端がはみ出る場合のトリミング
			if (dp.right + 2 > inf->ph_size.Width()) {
				dp.right = inf->ph_size.Width() - 2;
				bufDm->MaxR().right = dp.right * inf->vm_ppi.x / inf->ph_ppi.x - rVm.left;
			}
#endif
		} else if (inf->tech == capsPrinterType
			|| inf->tech == capsPreviewType) {
			// 印刷・印刷プレビュー
			im_width = bufDm->Width();
			im_height = bufDm->Height();
			sp.Set(bufDm->MaxR().left,
				bufDm->MaxR().top,
				bufDm->MaxR().right - bufDm->MaxR().left,
				bufDm->MaxR().bottom - bufDm->MaxR().top);
			dp.Set((sp.left + rVm.left) * inf->ph_ppi.x / inf->vm_ppi.x,
				(int)(vm_ry * inf->ph_ppi.y / inf->vm_ppi.y),
				sp.width * inf->ph_ppi.x / inf->vm_ppi.x,
				bufDm->MaxR().bottom * inf->ph_ppi.y / inf->vm_ppi.y);
		}
#if 0
		wxString dbg;
		dbg = _T("put_buffer\n")
		+ wxString::Format(_T("rMx left:%d top:%d right:%d bottom:%d\n")
			,bufDm->MaxR().left, bufDm->MaxR().top, bufDm->MaxR().right, bufDm->MaxR().bottom)
		+ wxString::Format(_T("dp  left:%d top:%d right:%d bottom:%d\n")
			,dp.left, dp.top, dp.right, dp.bottom)
		;
		AddDebugLog(dbg);
#endif
		// 文字の入っている横幅と改行する高さ分だけの矩形を出力
		if (bufDm->IsUpdatedData()) {
			expand_pixeldata();

#ifdef USE_PIXELDATA
			dcMx[mRotate & 1]->DrawBitmap(*bmpMx, 0, 0);
#else
			dcMx[mRotate & 1]->DrawBitmap(
				wxBitmap(wxImage(im_width,im_height,bufMix,true), MX_RGB_SIZE * 8)
				,0 ,0);
#endif

			dc->StretchBlit(dp.left, dp.top, dp.width, dp.height
				,dcMx[mRotate & 1], sp.left, sp.top, sp.width, sp.height, inf->mBlitMode);
		}
	}
}


/// 画面表示用 ページ番号と枠の描画
/// @param[in] dc        デバイスコンテキスト
/// @param[in] inf       プリンタなどの情報
/// @param[in] page      ページ番号
/// @param[in] draw_part 追加されたデータのみ描画するか
void MP_PRINTER::draw_page_no(wxDC *dc, CapsInfoType *inf, int page, bool draw_part)
{
	if (page < 1) return;
#if defined(__WXMSW__)
	if (draw_part) return;
#endif

	// 枠を描画
	dc->SetPen(wxPen(wxColour(0, 0, 0), 1, wxPENSTYLE_SOLID));
	dc->SetBrush(wxBrush(wxColour(0xff, 0xff, 0xff), wxBRUSHSTYLE_SOLID));
	wxRect rf;
	switch(mRotate & 3) {
	case 3:
#ifdef USE_PAGE_BUFFER
		rf.x = 0;
		rf.x = (rf.x * dsr.x / dsr.y);
		rf.y = 0;
		rf.width = (inf->ph_size.height);
		rf.width = (rf.width * dsr.x / dsr.y - rf.x);
		rf.height = (inf->ph_size.width);
#else
		rf.x = (inf->ph_size.height * (mMaxPage - page));
		rf.x = (rf.x * dsr.x / dsr.y);
		rf.y = (win_offset.y);
		rf.width = (inf->ph_size.height * (mMaxPage - page + 1));
		rf.width = (rf.width * dsr.x / dsr.y - rf.x);
		rf.height = (inf->ph_size.width);
#endif
		break;
	case 2:
#ifdef USE_PAGE_BUFFER
		rf.x = 0;
		rf.y = 0;
		rf.y = (rf.y * dsr.x / dsr.y);
		rf.width = (inf->ph_size.width);
		rf.height = (inf->ph_size.height);
		rf.height = (rf.height * dsr.x / dsr.y - rf.y);
#else
		rf.x = (win_offset.x);
		rf.y = (inf->ph_size.height * (mMaxPage - page));
		rf.y = (rf.y * dsr.x / dsr.y);
		rf.width = (inf->ph_size.width);
		rf.height = (inf->ph_size.height * (mMaxPage - page + 1));
		rf.height = (rf.height * dsr.x / dsr.y - rf.y);
#endif
		break;
	case 1:
#ifdef USE_PAGE_BUFFER
		rf.x = 0;
		rf.x = (rf.x * dsr.x / dsr.y);
		rf.y = 0;
		rf.width = (inf->ph_size.height);
		rf.width = (rf.width * dsr.x / dsr.y - rf.x);
		rf.height = (inf->ph_size.width);
#else
		rf.x = (inf->ph_size.height * (page - 1));
		rf.x = (rf.x * dsr.x / dsr.y);
		rf.y = (win_offset.y);
		rf.width = (inf->ph_size.height * page);
		rf.width = (rf.width * dsr.x / dsr.y - rf.x);
		rf.height = (inf->ph_size.width);
#endif
		break;
	default:
#ifdef USE_PAGE_BUFFER
		rf.x = 0;
		rf.y = 0;
		rf.y = (rf.y * dsr.x / dsr.y);
		rf.width = (inf->ph_size.width);
		rf.height = (inf->ph_size.height);
		rf.height = (rf.height * dsr.x / dsr.y - rf.y);
#else
		rf.x = (win_offset.x);
		rf.y = (inf->ph_size.height * (page - 1));
		rf.y = (rf.y * dsr.x / dsr.y);
		rf.width = (inf->ph_size.width);
		rf.height = (inf->ph_size.height * page);
		rf.height = (rf.height * dsr.x / dsr.y - rf.y);
#endif
		break;
	}
	dc->DrawRectangle(rf);

	// ページ番号印刷
	dc->SetTextForeground(wxColour(0xc0, 0xc0, 0xc0));
	wxString str_page = _("Page ") + wxString::Format(_T("%d"), page);
	int len_page = (int)str_page.length();
	wxSize font_size = dc->GetTextExtent(str_page);
	if (font_size.x <= len_page || font_size.y <= 0) {
		font_size = dc->GetFont().GetPixelSize();
		font_size.x *= (len_page + 1);
	}
	wxPoint pp1,pp2;
	double angle;

	switch(mRotate & 3) {
	case 3:
		angle = -90.0;

#ifdef USE_PAGE_BUFFER
		pp1.x = inf->ph_size.height - 2;
		pp1.x = pp1.x * dsr.x / dsr.y;
		pp1.y = 2;

		pp2.x = 2;
		pp2.x = pp2.x * dsr.x / dsr.y + font_size.GetY();
		pp2.y = inf->ph_size.width - 2 - font_size.GetX();
#else
		pp1.x = inf->ph_size.height - 2 + (inf->ph_size.height * (mMaxPage - page));
		pp1.x = pp1.x * dsr.x / dsr.y;
		pp1.y = 2 + win_offset.y;

		pp2.x = 2 + (inf->ph_size.height * (mMaxPage - page));
		pp2.x = pp2.x * dsr.x / dsr.y + font_size.GetY();
		pp2.y = inf->ph_size.width - 2 - font_size.GetX() + win_offset.y;
#endif
		break;

	case 2:
		angle = 180.0;

#ifdef USE_PAGE_BUFFER
		pp1.x = inf->ph_size.width - 2;
		pp1.y = inf->ph_size.height - 2;
		pp1.y = pp1.y * dsr.x / dsr.y;

		pp2.x = font_size.GetX() + 2;
		pp2.y = 2;
		pp2.y = pp2.y * dsr.x / dsr.y + font_size.GetY();
#else
		pp1.x = inf->ph_size.width - 2 + win_offset.x;
		pp1.y = inf->ph_size.height - 2 + (inf->ph_size.height * (mMaxPage - page));
		pp1.y = pp1.y * dsr.x / dsr.y;

		pp2.x = font_size.GetX() + 2 + win_offset.x;
		pp2.y = 2 + (inf->ph_size.height * (mMaxPage - page));
		pp2.y = pp2.y * dsr.x / dsr.y + font_size.GetY();
#endif
		break;

	case 1:
		angle = 90.0;

#ifdef USE_PAGE_BUFFER
		pp1.x = 2;
		pp1.x = pp1.x * dsr.x / dsr.y;
		pp1.y = inf->ph_size.width - 2;

		pp2.x = inf->ph_size.height - 2;
		pp2.x = pp2.x * dsr.x / dsr.y;
		pp2.y = 2 + font_size.GetX();
#else
		pp1.x = 2 + (inf->ph_size.height * (page - 1));
		pp1.x = pp1.x * dsr.x / dsr.y;
		pp1.y = inf->ph_size.width - 2 + win_offset.y;

		pp2.x = inf->ph_size.height - 2 + (inf->ph_size.height * (page - 1));
		pp2.x = pp2.x * dsr.x / dsr.y - font_size.GetY();
		pp2.y = 2 + font_size.GetX() + win_offset.y;
#endif
		break;

	default:
		angle = 0.0;

#ifdef USE_PAGE_BUFFER
		pp1.x = 2;
		pp1.y = 2;
		pp1.y = pp1.y * dsr.x / dsr.y;

		pp2.x = inf->ph_size.width - font_size.GetX() - 2;
		pp2.y = inf->ph_size.height - 2;
		pp2.y = pp2.y * dsr.x / dsr.y - font_size.GetY();
#else
		pp1.x = 2 + win_offset.x;
		pp1.y = 2 + (inf->ph_size.height * (page - 1));
		pp1.y = pp1.y * dsr.x / dsr.y;

		pp2.x = inf->ph_size.width - font_size.GetX() - 2 + win_offset.x;
		pp2.y = inf->ph_size.height - 2 + (inf->ph_size.height * (page - 1));
		pp2.y = pp2.y * dsr.x / dsr.y - font_size.GetY();
#endif
		break;
	}

	dc->DrawRotatedText(str_page, pp1, angle);
	dc->DrawRotatedText(str_page, pp2, angle);

	// 余白位置を描画
	dc->SetPen(wxPen(wxColour(0xc0,0xc0,0xc0),1,wxPENSTYLE_LONG_DASH));
	wxPoint pm11,pm12,pm21,pm22;
	switch(mRotate & 3) {
	case 3:
#ifdef USE_PAGE_BUFFER
		pm11.x = inf->ph_margin.bottom;
		pm11.x = pm11.x * dsr.x / dsr.y;
		pm11.y = 1;
		pm12.x = pm11.x;
		pm12.y = inf->ph_size.width - 1;

		pm21.x = inf->ph_size.height - inf->ph_margin.top;
		pm21.x = pm21.x * dsr.x / dsr.y;
		pm21.y = pm11.y;
		pm22.x = pm21.x;
		pm22.y = pm12.y;
#else
		pm11.x = inf->ph_size.height * (mMaxPage - page) + inf->ph_margin.bottom;
		pm11.x = pm11.x * dsr.x / dsr.y;
		pm11.y = win_offset.y + 1;
		pm12.x = pm11.x;
		pm12.y = win_offset.y + inf->ph_size.width - 1;

		pm21.x = inf->ph_size.height * (mMaxPage - page + 1) - inf->ph_margin.top;
		pm21.x = pm21.x * dsr.x / dsr.y;
		pm21.y = pm11.y;
		pm22.x = pm21.x;
		pm22.y = pm12.y;
#endif
		break;

	case 2:
#ifdef USE_PAGE_BUFFER
		pm11.x = 1;
		pm11.y = inf->ph_margin.bottom;
		pm11.y = pm11.y * dsr.x / dsr.y;
		pm12.x = inf->ph_size.width - 1;
		pm12.y = pm11.y;

		pm21.x = pm11.x;
		pm21.y = inf->ph_size.height - inf->ph_margin.top;
		pm21.y = pm21.y * dsr.x / dsr.y;
		pm22.x = pm12.x;
		pm22.y = pm21.y;
#else
		pm11.x = win_offset.x + 1;
		pm11.y = inf->ph_size.height * (mMaxPage - page) + inf->ph_margin.bottom;
		pm11.y = pm11.y * dsr.x / dsr.y;
		pm12.x = inf->ph_size.width + win_offset.x - 1;
		pm12.y = pm11.y;

		pm21.x = pm11.x;
		pm21.y = inf->ph_size.height * (mMaxPage - page + 1) - inf->ph_margin.top;
		pm21.y = pm21.y * dsr.x / dsr.y;
		pm22.x = pm12.x;
		pm22.y = pm21.y;
#endif
		break;
	case 1:
#ifdef USE_PAGE_BUFFER
		pm11.x = inf->ph_margin.top;
		pm11.x = pm11.x * dsr.x / dsr.y;
		pm11.y = 1;
		pm12.x = pm11.x;
		pm12.y = inf->ph_size.width - 1;

		pm21.x = inf->ph_size.height - inf->ph_margin.bottom;
		pm21.x = pm21.x * dsr.x / dsr.y;
		pm21.y = pm11.y;
		pm22.x = pm21.x;
		pm22.y = pm12.y;
#else
		pm11.x = inf->ph_size.height * (page - 1) + inf->ph_margin.top;
		pm11.x = pm11.x * dsr.x / dsr.y;
		pm11.y = win_offset.y + 1;
		pm12.x = pm11.x;
		pm12.y = win_offset.y + inf->ph_size.width - 1;

		pm21.x = inf->ph_size.height * page - inf->ph_margin.bottom;
		pm21.x = pm21.x * dsr.x / dsr.y;
		pm21.y = pm11.y;
		pm22.x = pm21.x;
		pm22.y = pm12.y;
#endif
		break;

	default:
#ifdef USE_PAGE_BUFFER
		pm11.x = 1;
		pm11.y = inf->ph_margin.top;
		pm11.y = pm11.y * dsr.x / dsr.y;
		pm12.x = inf->ph_size.width - 1;
		pm12.y = pm11.y;

		pm21.x = pm11.x;
		pm21.y = inf->ph_size.height - inf->ph_margin.bottom;
		pm21.y = pm21.y * dsr.x / dsr.y;
		pm22.x = pm12.x;
		pm22.y = pm21.y;
#else
		pm11.x = win_offset.x + 1;
		pm11.y = inf->ph_size.height * (page - 1) + inf->ph_margin.top;
		pm11.y = pm11.y * dsr.x / dsr.y;
		pm12.x = inf->ph_size.width + win_offset.x - 1;
		pm12.y = pm11.y;

		pm21.x = pm11.x;
		pm21.y = inf->ph_size.height * page - inf->ph_margin.bottom;
		pm21.y = pm21.y * dsr.x / dsr.y;
		pm22.x = pm12.x;
		pm22.y = pm21.y;
#endif
		break;
	}
	if (inf->ph_margin.top != 0) {
		dc->DrawLine(pm11, pm12);
	}
	if (inf->ph_margin.bottom != 0) {
		dc->DrawLine(pm21, pm22);
	}
}

#if 0
/// 印刷範囲の下端を計算
long MP_PRINTER::calc_dp_bottom(CapsInfoType *inf, int h)
{
	long bottom;

	if (inf->ph_size.height > 0) {
		bottom = inf->ph_size.height - inf->ph_offset.y - inf->ph_margin.bottom - (h * inf->ph_ppi.y / inf->vm_ppi.y);
		if (bottom > inf->ph_size.height) bottom = inf->ph_size.height;
	} else {
		bottom = -1;
	}
	return bottom;
}
#endif

/// 印刷範囲の下端を計算
/// @param[in] inf   プリンタなどの情報
/// @param[in] h     高さ
long MP_PRINTER::calc_vm_bottom(CapsInfoType *inf, int h)
{
	long bottom;

	if (inf->vm_size.height > 0) {
		bottom = inf->vm_size.height - inf->vm_offset.y - inf->vm_margin.bottom - h;
		if (bottom > inf->vm_size.height) bottom = inf->vm_size.height;
	} else {
		bottom = -1;
	}
	return bottom;
}

/// 画面表示用システムフォントの設定
/// @param[in] dc    デバイスコンテキスト
/// @param[in] w     幅
/// @param[in] h     高さ
void MP_PRINTER::set_sys_font(wxDC *dc, int w, int h)
{
	wxFont font(wxSize(w,h), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, _T("Courier New"));
	dc->SetFont(font);
}

#if 0
/// ピクセルデータの内容を左へシフト
void MP_PRINTER::shift_pixeldata(int width)
{
	bufDw->LeftShift(width);
}

/// ピクセルデータの内容をクリア
void MP_PRINTER::clear_pixeldata(int width)
{
	bufDw->ClearRight(width);
}

/// ピクセルデータの内容をコピー
void MP_PRINTER::copy_pixeldata(int dx, int dy, int width, int height, int sx, int sy)
{
	bufDw->Copy(bufDm, dx, dy, width, height, sx, sy);
}

/// 合成エリアをを上へシフト
void MP_PRINTER::shift_mixbuffer(int width, int height)
{
	bufDm->UpperShift(width, height);
}

/// 合成エリアををクリア
void MP_PRINTER::clear_mixbuffer(int width)
{
	bufDm->ClearLeftTo(width);
}
#endif

/// ピクセルデータの内容をRGBに拡張
void MP_PRINTER::expand_pixeldata()
{
	DpType *src = NULL;
#ifdef USE_PIXELDATA
	wxAlphaPixelData::Iterator dst(*bufMx);
#else
	wxByte *dst = bufMix;
#endif

	int width;
	int height;

	switch(mRotate & 3) {
	case 3:
		// rotate (base is right and top)
		width = MP_PRN_BUF_HEIGHT;
		height = MP_PRN_BUF_WIDTH;
		for(int h=0; h < height; h++) {
#ifdef USE_PIXELDATA
			dst.MoveTo(*bufMx, width, h);
#else
			dst = bufMix + (height - h) * width * MX_RGB_SIZE;
#endif
			for(int w=0; w < width; w++) {
				src = bufDm->Ptr(height - 1 - h, w);
#ifdef USE_PIXELDATA
				dst--;
				dst.Red() = color_density_tbl[*src];
				dst.Green() = color_density_tbl[*src];
				dst.Blue() = color_density_tbl[*src];
				dst.Alpha() = 0xff;
#else
				dst-=MX_RGB_SIZE;
				for(int n=0; n<MX_RGB_SIZE; n++) {
					dst[n] = color_density_tbl[*src];
				}
#endif
			}
		}
		break;
	case 2:
		// rotate (upside down)
		width = MP_PRN_BUF_WIDTH;
		height = MP_PRN_BUF_HEIGHT;
		for(int h=0; h < height; h++) {
			src = bufDm->Ptr(0, h);
#ifdef USE_PIXELDATA
			dst.MoveTo(*bufMx, width, h);
#else
			dst = bufMix + (height - h) * width * MX_RGB_SIZE;
#endif
			for(int w=0; w < width; w++) {
#ifdef USE_PIXELDATA
				dst--;
				dst.Red() = color_density_tbl[*src];
				dst.Green() = color_density_tbl[*src];
				dst.Blue() = color_density_tbl[*src];
				dst.Alpha() = 0xff;
				src++;
#else
				dst-=MX_RGB_SIZE;
				for(int n=0; n<MX_RGB_SIZE; n++) {
					dst[n] = color_density_tbl[*src];
				}
				src++;
#endif
			}
		}
		break;
	case 1:
		// rotate (base is left and bottom)
		width = MP_PRN_BUF_HEIGHT;
		height = MP_PRN_BUF_WIDTH;
		for(int h=0; h < height; h++) {
#ifdef USE_PIXELDATA
			dst.MoveTo(*bufMx, 0, h);
#else
			dst = bufMix + h * width * MX_RGB_SIZE;
#endif
			for(int w=0; w < width; w++) {
				src = bufDm->Ptr(height - 1 - h, w);
#ifdef USE_PIXELDATA
				dst.Red() = color_density_tbl[*src];
				dst.Green() = color_density_tbl[*src];
				dst.Blue() = color_density_tbl[*src];
				dst.Alpha() = 0xff;
				dst++;
#else
				for(int n=0; n<MX_RGB_SIZE; n++) {
					dst[n] = color_density_tbl[*src];
				}
				dst+=MX_RGB_SIZE;
#endif
			}
		}
		break;
	default:
		// no rotate
		width = MP_PRN_BUF_WIDTH;
		height = MP_PRN_BUF_HEIGHT;
		for(int h=0; h < height; h++) {
			src = bufDm->Ptr(0, h);
#ifdef USE_PIXELDATA
			dst.MoveTo(*bufMx, 0, h);
#else
			dst = bufMix + h * width * MX_RGB_SIZE;
#endif
			for(int w=0; w < width; w++) {
#ifdef USE_PIXELDATA
				dst.Red() = color_density_tbl[*src];
				dst.Green() = color_density_tbl[*src];
				dst.Blue() = color_density_tbl[*src];
				dst.Alpha() = 0xff;
				src++;
				dst++;
#else
				for(int n=0; n<MX_RGB_SIZE; n++) {
					dst[n] = color_density_tbl[*src];
				}
				src++;
				dst+=MX_RGB_SIZE;
#endif
			}
		}
		break;
	}
}

#if 0
/// 合成エリアをを上へシフト
void MP_PRINTER::shift_mixbuffer_mx(int width, int height)
{
#ifdef USE_PIXELDATA
	wxNativePixelData::Iterator src(*bufMx);
	wxNativePixelData::Iterator dst(*bufMx);
#else
	wxByte *src = bufMx;
	wxByte *dst = bufMx;
#endif
	if (height > MP_PRN_BUF_HEIGHT) height = MP_PRN_BUF_HEIGHT;
	if (height < 0) height = 0;

	for(int h=0; h < (MP_PRN_BUF_HEIGHT - height); h++) {
#ifdef USE_PIXELDATA
		src.MoveTo(*bufMx,0,h + height);
		dst.MoveTo(*bufMx,0,h);
#else
		src = bufMx + ((h + height) * MP_PRN_BUF_WIDTH) * MX_RGB_SIZE;
		dst = bufMx + (h * MP_PRN_BUF_WIDTH) * MX_RGB_SIZE;
#endif
		for(int w=0; w < width; w++) {
#ifdef USE_PIXELDATA
			dst.Red() = src.Red();
			dst.Green() = src.Green();
			dst.Blue() = src.Blue();
			src++;
			dst++;
#else
			for(int n=0; n<MX_RGB_SIZE; n++) {
				dst[n] = src[n];
			}
			src+=MX_RGB_SIZE;
			dst+=MX_RGB_SIZE;
#endif
		}
		rMxMaxH[h].left = rMxMaxH[h + height].left;
		rMxMaxH[h].right = rMxMaxH[h + height].right;
	}
	for(int h=(MP_PRN_BUF_HEIGHT - height); h < MP_PRN_BUF_HEIGHT; h++) {
#ifdef USE_PIXELDATA
		dst.MoveTo(*bufMx,0,h);
#else
		dst = bufMx + (h * MP_PRN_BUF_WIDTH) * MX_RGB_SIZE;
#endif
		for(int w=0; w < width; w++) {
#ifdef USE_PIXELDATA
			dst.Red() = dst.Green() = dst.Blue() = DpTypeMax;
			dst++;
#else
			for(int n=0; n<MX_RGB_SIZE; n++) {
				dst[n] = DpTypeMax;
			}
			dst+=MX_RGB_SIZE;
#endif
		}
		rMxMaxH[h].left = width;
		rMxMaxH[h].right = 0;
	}
}

/// 合成エリアををクリア
void MP_PRINTER::clear_mixbuffer_mx(int width)
{
#ifdef USE_PIXELDATA
	wxNativePixelData::Iterator dst(*bufMx);
#else
	wxByte *dst = bufMx;
#endif
	for(int h=0; h < MP_PRN_BUF_HEIGHT; h++) {
#ifdef USE_PIXELDATA
		dst.MoveTo(*bufMx, 0, h);
#else
		dst = bufMx + (h * MP_PRN_BUF_WIDTH) * MX_RGB_SIZE;
#endif
		for(int w=0; w < width; w++) {
#ifdef USE_PIXELDATA
			dst.Red() = dst.Green() = dst.Blue() = DpTypeMax;
			dst++;
#else
			for(int n=0; n<MX_RGB_SIZE; n++) {
				dst[n] = DpTypeMax;
			}
			dst+=MX_RGB_SIZE;
#endif
		}
		rMxMaxH[h].left = width;
		rMxMaxH[h].right = 0;
	}
}
#endif

#ifdef USE_CASH_BUFFER
/// 保存しているデータで描画
bool MP_PRINTER::draw_from_cashdata(wxDC *dc, CapsInfoType *inf)
{
	int win_offset_x = 0;

	switch(mRotate & 1) {
	case 1:
		break;
	default:
		win_offset_x = (win_size.width - inf->ph_size.width) / 2;
		break;
	}
	if (win_offset_x < 0) win_offset_x = 0;
	dc->Blit(win_offset_x,0,bmpCashd->GetWidth(),bmpCashd->GetHeight(),dcCashd,0,0);
	return true;
}

/// 保存用デバイスに描画
bool MP_PRINTER::draw_to_cashdata(wxDC *dc, CapsInfoType *inf, bool draw_part)
{
	draw_data(dcCashd, inf, draw_part);
	return draw_from_cashdata(dc, inf);
}
#endif

///
/// 印刷制御クラス
///
MpPrintout::MpPrintout(MP_PRINTER *new_mpri, const wxString &title, wxPageSetupDialogData *new_pagesetdlg)
	: wxPrintout(title)
{
	mpri = new_mpri;
	pagesetdlg = new_pagesetdlg;
}

/// 印刷処理開始
void MpPrintout::OnBeginPrinting()
{
	AddDebugLog(_T("OnBeginPrinting"));

	pdc = this->GetDC();

	mpri->PrintingStart(pdc, IsPreview());

	wxPoint scr_ppi;
	wxPoint prn_ppi;
	GetPPIScreen(&scr_ppi.x, &scr_ppi.y);
	GetPPIPrinter(&prn_ppi.x, &prn_ppi.y);
	wxRect page_rect = GetLogicalPageRect();
	wxRect paper_rect = GetLogicalPaperRect();
	wxRect r_paper_rect = GetPaperRectPixels();
	MySize px;
	GetPageSizePixels(&px.width, &px.height);
	wxRect r_page_rect(0,0, px.width, px.height);

#ifdef _DEBUG_LOG
	wxString dbg;
	wxString cname = pdc->GetClassInfo()->GetClassName();
	dbg = _T("DC: ") + cname + _T("\n");
	dbg = dbg + _T("Before\n");
	dbg = dbg + wxString::Format(_T("  screen ppi x:%d y:%d\n"),scr_ppi.x,scr_ppi.y);
	dbg = dbg + wxString::Format(_T("  printer ppi x:%d y:%d\n"),prn_ppi.x,prn_ppi.y);
	dbg = dbg + wxString::Format(_T("  printer page size  w:%d h:%d\n"),r_page_rect.GetWidth(),r_page_rect.GetHeight());
	dbg = dbg + wxString::Format(_T("  printer paper rect left:%d top:%d width:%d height:%d\n"),r_paper_rect.GetLeft(),r_paper_rect.GetTop(),r_paper_rect.GetWidth(),r_paper_rect.GetHeight());
	dbg = dbg + wxString::Format(_T("  printer loc page size width:%d height:%d\n"),page_rect.GetWidth(),page_rect.GetHeight());
	dbg = dbg + wxString::Format(_T("  printer loc paper rect left:%d top:%d width:%d height:%d\n"),paper_rect.GetLeft(),paper_rect.GetTop(),paper_rect.GetWidth(),paper_rect.GetHeight());
	AddDebugLog(dbg);
#endif

	if (IsPreview()) {
		// 印刷プレビュー時
#if defined(__WXMSW__)
		mpri->SetDocumentSize(&scr_ppi, &prn_ppi, &paper_rect, &page_rect);
#elif defined(__WXOSX__)
		MapScreenSizeToPaper();
		MapScreenSizeToPage();

		GetPPIScreen(&scr_ppi.x, &scr_ppi.y);
		GetPPIPrinter(&prn_ppi.x, &prn_ppi.y);
		page_rect = GetLogicalPageRect();
		paper_rect = GetLogicalPaperRect();
		r_paper_rect = GetPaperRectPixels();
		GetPageSizePixels(&px.width, &px.height);
		r_page_rect = wxRect(0,0, px.width, px.height);

		mpri->SetDocumentSize(&scr_ppi, &scr_ppi, &paper_rect, &page_rect);
#elif defined(__WXGTK__)
		MapScreenSizeToPaper();
		MapScreenSizeToPage();

		GetPPIScreen(&scr_ppi.x, &scr_ppi.y);
		GetPPIPrinter(&prn_ppi.x, &prn_ppi.y);
		page_rect = GetLogicalPageRect();
		paper_rect = GetLogicalPaperRect();
		r_paper_rect = GetPaperRectPixels();
		GetPageSizePixels(&px.width, &px.height);
		r_page_rect = wxRect(0,0, px.width, px.height);

		mpri->SetDocumentSize(&scr_ppi, &scr_ppi, &paper_rect, &page_rect);
#endif
	} else {
		// 印刷時
#if defined(__WXMSW__)
		mpri->SetDocumentSize(&scr_ppi, &prn_ppi, &paper_rect, &page_rect);
#elif defined(__WXOSX__)
		mpri->SetDocumentSize(&scr_ppi, &prn_ppi, &paper_rect, &page_rect);
#elif defined(__WXGTK__)
		MapScreenSizeToPaper();
		MapScreenSizeToPage();

		GetPPIScreen(&scr_ppi.x, &scr_ppi.y);
		GetPPIPrinter(&prn_ppi.x, &prn_ppi.y);
		page_rect = GetLogicalPageRect();
		paper_rect = GetLogicalPaperRect();
		r_paper_rect = GetPaperRectPixels();
		GetPageSizePixels(&px.width, &px.height);
		r_page_rect = wxRect(0,0, px.width, px.height);

		// TODO: StretchBlitでスケーリングがおかしい／なぜ？
		// screen: 72ppiに換算して計算する。
		prn_ppi.x = prn_ppi.x * scr_ppi.x / 72;
		prn_ppi.y = prn_ppi.y * scr_ppi.y / 72;
		r_paper_rect.x = r_paper_rect.x * scr_ppi.x / 72;
		r_paper_rect.y = r_paper_rect.y * scr_ppi.y / 72;
		r_paper_rect.width = r_paper_rect.width * scr_ppi.x / 72;
		r_paper_rect.height = r_paper_rect.height * scr_ppi.y / 72;
		r_page_rect.x = r_page_rect.x * scr_ppi.x / 72;
		r_page_rect.y = r_page_rect.y * scr_ppi.y / 72;
		r_page_rect.width = r_page_rect.width * scr_ppi.x / 72;
		r_page_rect.height = r_page_rect.height * scr_ppi.y / 72;

		mpri->SetDocumentSize(&scr_ppi, &prn_ppi, &r_paper_rect, &r_page_rect);
#endif
	}

#ifdef _DEBUG_LOG
	dbg = _T("After\n");
	dbg = dbg + wxString::Format(_T("  screen ppi x:%d y:%d\n"),scr_ppi.x,scr_ppi.y);
	dbg = dbg + wxString::Format(_T("  printer ppi x:%d y:%d\n"),prn_ppi.x,prn_ppi.y);
	dbg = dbg + wxString::Format(_T("  printer page size  w:%d h:%d\n"),r_page_rect.GetWidth(),r_page_rect.GetHeight());
	dbg = dbg + wxString::Format(_T("  printer paper rect left:%d top:%d width:%d height:%d\n"),r_paper_rect.GetLeft(),r_paper_rect.GetTop(),r_paper_rect.GetWidth(),r_paper_rect.GetHeight());
	dbg = dbg + wxString::Format(_T("  printer loc page size width:%d height:%d\n"),page_rect.GetWidth(),page_rect.GetHeight());
	dbg = dbg + wxString::Format(_T("  printer loc paper rect left:%d top:%d width:%d height:%d\n"),paper_rect.GetLeft(),paper_rect.GetTop(),paper_rect.GetWidth(),paper_rect.GetHeight());
	AddDebugLog(dbg);
#endif

}
/// ドキュメント印刷開始
/// @param[in] startPage 開始ページ
/// @param[in] endPage   終了ページ
bool MpPrintout::OnBeginDocument(int startPage, int endPage)
{
	AddDebugLog(_T("OnBeginDocument"));
//	if (!wxPrintout::OnBeginDocument(startPage, endPage))
//		return false;

	return mpri->PrintStartDocument(pdc);
}
/// 印刷中
/// @param[in] pageNum ページ番号
bool MpPrintout::OnPrintPage(int pageNum)
{
	AddDebugLog(wxString::Format(_T("OnPrintPage %d"), pageNum));
	WritePageHeader(this, pdc, wxEmptyString, 1.0);
	return mpri->PrintPage(pdc, pageNum);
}
/// ドキュメント印刷終了
void MpPrintout::OnEndDocument()
{
	AddDebugLog(_T("OnEndDocument"));
	mpri->PrintEndDocument(pdc);
}
/// 印刷処理終了
void MpPrintout::OnEndPrinting()
{
	AddDebugLog(_T("OnEndPrinting"));
	mpri->PrintingEnd(pdc);
}
/// 印刷ページ設定
void MpPrintout::GetPageInfo(int *minPage, int *maxPage, int *selPageFrom, int *selPageTo)
{
	mpri->GetPageInfo(minPage, maxPage, selPageFrom, selPageTo);
	AddDebugLog(wxString::Format(_T("GetPageInfo: minpage:%d maxpage:%d frompage:%d topage:%d"),*minPage,*maxPage,*selPageFrom,*selPageTo));
}
/// 印字対象ページ？
bool MpPrintout::HasPage(int pageNum)
{
    return true;
}
/// ヘッダ
bool MpPrintout::WritePageHeader(wxPrintout *printout, wxDC *dc, const wxString&text, float mmToLogical)
{
	wxRect prect = GetLogicalPaperRect();
	dc->SetPen(wxPen(wxColour(0xff,0xff,0xff), 1));
	dc->SetBrush(wxBrush(wxColour(0xff, 0xff, 0xff), wxBRUSHSTYLE_SOLID));
	dc->DrawRectangle(prect.GetLeft(),prect.GetTop(),prect.GetWidth(),prect.GetHeight());
	prect = GetLogicalPageRect();
	dc->SetPen(wxPen(wxColour(0xc0,0xc0,0xc0), 1, wxPENSTYLE_DOT));
	if (IsPreview()) {
		dc->DrawRectangle(prect.GetLeft(),prect.GetTop(),prect.GetWidth(),prect.GetHeight());
	}
#ifdef _DEBUG
	dc->DrawRectangle(prect.GetLeft(),prect.GetTop(),prect.GetWidth(),prect.GetHeight());
	dc->DrawLine(prect.GetLeft(),prect.GetTop(),prect.GetRight(),prect.GetBottom());
	dc->DrawLine(prect.GetLeft(),prect.GetBottom(),prect.GetRight(),prect.GetTop());
#endif

	return true;
}

///
/// 印刷プレビュークラス
///
MpPreviewFrame::MpPreviewFrame(wxPrintPreview *preview, wxWindow *parent)
	: wxPreviewFrame(preview, parent, _("Preview"), wxPoint(100, 100), wxSize(600, 650))
{
	frame = parent;
	Centre(wxBOTH);
	InitializeWithModality(wxPreviewFrame_WindowModal);
	Show();
}
MpPreviewFrame::~MpPreviewFrame()
{
	frame->SetFocus();
}

