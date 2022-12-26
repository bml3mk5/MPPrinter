/// @file mp_printer.h
///
/// @brief MPPRINTER 制御クラス
///
/// @date 2013.11.27 Create
///
/// @author Copyright (c) 2013, Sasaji. All rights reserved.
///
#ifndef MP_PRINTER_H
#define MP_PRINTER_H

#include "common.h"
#include <wx/wx.h>
#include <wx/printdlg.h>
#include <wx/print.h>
#include <wx/cmndata.h>
#include <wx/dc.h>
#include <wx/dcclient.h>
#include <wx/dcmemory.h>
#include <wx/dcbuffer.h>
#include <wx/rawbmp.h>
#include "errorinfo.h"
#include "marginbox.h"
#include "coordinate.h"
#include "dpbuffer.h"
#include "buffer.h"

#define MP_PRN_BUF_WIDTH	1584	///< 印字最大幅
#define MP_PRN_BUF_HEIGHT	24

#define MP_MODEL_MP1020		0
#define MP_MODEL_MP80		1

#define MP_BIT_CR	0x01	///< 復帰指示
#define MP_BIT_LF	0x02	///< 改行指示
#define MP_BIT_FF	0x04	///< 改ページ指示
#define MP_BIT_PF	0x08	///< 紙送り指示

#define USE_CASH_BUFFER
//#define USE_PAGE_BUFFER
//#define USE_PIXELDATA

/// MPPRINTER 制御クラス
class MP_MODEL;

/// 印字デバイスの種類
typedef enum enumCapsInfoTypeTech {
	capsDisplayType = 0,	///< 画面
	capsPrinterType,		///< プリンター
	capsPreviewType,		///< プレビュー画面
	capsUnknownType,
} CapsInfoTypeTech;

/// 印字するデバイスの情報
class CapsInfoType
{
public:
	CapsInfoTypeTech tech;	///< 印字先デバイスの種類
	MySize  size_mm;
	MyVert  margin_mm;	///< 上下マージン(mm)
	MyPoint ph_ppi;		///< 解像度(pixel per inch)(拡大、縮小した値)
	MySize  ph_size;	///< 紙のサイズ(拡大、縮小した値)（印刷不可領域も含む）
	MyPoint ph_offset;
	MyVert  ph_margin;
	MyPoint vm_ppi;		///< 論理座標ppi
	MySize  vm_size;	///< サイズ 論理座標
	MyPoint vm_offset;	///< オフセット 論理座標
	MyVert  vm_margin;	///< 余白(pixel) 論理座標
	wxRasterOperationMode mBlitMode;	///< 画面に重ね合わせるときの方法
};

/// 印字本体 クラス
class MP_PRINTER
{
//	friend class MP1020;
//	friend class MP80;

private:
	const wxApp *mApp;
	wxString	mDataPath;

	MP_MODEL   *mp_model;
	MP_MODEL   *mp_model_list[2];
	int         selected_model;

	DpBufferDw	*bufDw;	///< 描画用バッファ

	DpBufferMx	*bufDm;		///< 合成用バッファ
	wxMemoryDC  *dcMx[2];	///< 合成用バッファDC
#ifdef USE_PIXELDATA
	wxBitmap    *bmpMx;	///< 合成用バッファのBitmap
	wxAlphaPixelData *bufMx;	///< 合成用バッファ
#else
	wxByte		*bufMix;	///< 合成用バッファ
#endif
#ifdef USE_PAGE_BUFFER
	wxMemoryDC  *dcPage;	///< 1ページ分の描画用バッファ
#endif
#ifdef USE_CASH_BUFFER
	bool        mUseCash;	///< 描画したデータを保持するか
	bool		mDataCashd;	///< 描画したデータを保持しているか
	wxBitmap   *bmpCashd;	///< 描画したデータのBitmap
	wxMemoryDC *dcCashd;	///< 描画したデータ
#endif
	int mRotate;		///< 回転するか

	int mMaxPage;		///< 最大ページ数
	int mCurPage;		///< 描画中のページ
	int mIngPage;		///< 描画開始したページ(画面描画用)
	bool endPage;		///< ページ終りか？

	MyPoint dp_offset;	///< 画面オフセット

	MyRect rVm;			///< 印刷範囲(pixel) 論理座標
	MySize rVmMax;		///< 印刷範囲の最大幅 論理座標
	MyRect rVmIn;		///< 画面表示で描画処理を行う範囲 論理座標
	double vm_y;		///< 印字行(ページ内) 論理座標
	double vm_y_prev;	///< 印字行(ページ内) 論理座標
	double vm_ry;		///< 画面の下端 論理座標
	double vm_ry_prev;	///< 画面の下端 論理座標

	double line_feed_height;	///< 改行幅
	double paper_feed_height;	///< 紙送り幅

	int vt_line_number;	///< 現在行(垂直タブ用)
	int ht_pos;			///< 現在位置(水平タブ用)

	/// 画面/プリンタ/印刷プレビューの情報
	CapsInfoType	device_info[capsUnknownType];
	CapsInfoType	*dinfo;

	MyPoint rDpPpi;		///< 画面のppi(拡大縮小で変化する)
	int zoom_mag;		///< 画面の倍率
	MyPoint dsr;			///< 画面表示でのフォントの縦横比
	MySize  win_size;		///< 画面サイズ
	MyPoint win_offset;		///< 画面オフセット

	wxPageSetupDialogData pagesetdlg;	///< ページ設定ダイアログ
	wxPrintDialogData printdlg;		///< プリンターダイアログ
	wxPrintData printdata;		///< プリンター
	wxDC *mPrintDC;

	wxString  text_file;			///< テキストファイル名
	FIFOBuffer text_data;			///< テキストデータ
	FIFOBuffer subst_data;			///< 代替テキストデータ

	bool draw_mode;		///< 画面にプレビューするか
	bool draw_mode_bkup;	///< 画面にプレビューするか
	int  printing;			///< 印刷中=1,印刷プレビュー中=2か
	wxUint32 feed;				///< 紙送りフラグ
	int  line_feed_number;		///< 改行数

	bool draw_win;		///< 画面内か
	bool print_page;	///< 印刷対象ページか
	bool change_page;	///< 改ページ指示

	wxByte	color_density_tbl[DpTypeMax + 1];	///< 色の濃さ
	bool reverse;		///< 色を反転するか

	bool recreate_mixdc();
	bool create_cashdata(wxDC *dc);
	void release_cashdata();
	void get_device_caps(wxDC *dc, CapsInfoType *inf);
	void set_paper_size(CapsInfoType *, int, int);
	bool select_printer(wxWindow *win);
	bool set_printer_page(wxWindow *win);
	bool set_margin_page(wxWindow *win);
	int  parse_ctrl_code(wxByte *);
	bool draw_data(wxDC *dc, CapsInfoType *inf, bool draw_part);
	bool print_data(wxDC *dc);
#ifdef USE_CASH_BUFFER
	bool draw_from_cashdata(wxDC *dc, CapsInfoType *inf);
	bool draw_to_cashdata(wxDC *dc, CapsInfoType *inf, bool draw_part);
#endif
	bool draw_start_document(wxDC *dc, CapsInfoType *inf);
	bool draw_start_page(wxDC *dc, CapsInfoType *inf, int target_page, bool draw_part);
	bool draw_page(wxDC *dc, CapsInfoType *inf, int target_page);
	bool draw_end_page(wxDC *dc, CapsInfoType *inf, int target_page);
	bool draw_end_document(wxDC *dc, CapsInfoType *inf);
//	bool is_inwindow(wxDC *dc, CapsInfoType *inf, double y);
	bool is_invmwindow(wxDC *dc, CapsInfoType *inf, double y);
	void put_buffer(wxDC *dc, CapsInfoType *inf, bool draw, bool last);
	void draw_page_no(wxDC *dc, CapsInfoType *inf, int page, bool draw_part);
//	long calc_dp_bottom(CapsInfoType *, int);
	long calc_vm_bottom(CapsInfoType *, int);
	void set_sys_font(wxDC *dc, int w, int h);
	bool read_font();
//	void shift_pixeldata(int width);
//	void clear_pixeldata(int width);
//	void copy_pixeldata(int dx, int dy, int width, int height, int sx, int sy);
//	void shift_mixbuffer(int width, int height);
//	void clear_mixbuffer(int width);
	void expand_pixeldata();
#if 0
	void shift_mixbuffer_mx(int width, int height);
	void clear_mixbuffer_mx(int width);
#endif

public:
	MP_PRINTER(const wxApp &app, const wxString &data_path);
	~MP_PRINTER();

	bool Init(wxWindow *win, int w, int h);
	void SelectModel(int);
	void ClearSetting();
	void ClearData(const wxString &title);
	void SetData(wxWindow *win, const wxByte* data, size_t size, bool reverse);
	int  GetLength() const;
	void PageSetup(wxWindow *win);
	void MarginSetup(wxWindow *win);
	/// @brief 印刷
	bool Print(wxWindow *win);
	/// @brief 印刷プレビュー
	bool PrintPreview(wxWindow *win);
	/// @brief 画面用プレビュー
	void Preview(wxWindow *win);
	bool Zoom(wxWindow *win, int);
	void DrawData(wxDC *dc, int, int, bool);
	void SetDocumentSize(wxPoint *scr_ppi, wxPoint *prn_ppi, wxRect *prect, wxRect *psize);

	bool ReadFile(wxWindow *win, const wxString &text_path);
	void CloseFile();

	void ClearPrintBuffer();

	void PushSetting();
	void PushSetting(int);

	bool PrintingStart(wxDC *dc, bool preview);
	bool PrintStartDocument(wxDC *dc);
	bool PrintPage(wxDC *dc, int pageNum);
	bool PrintEndDocument(wxDC *dc);
	bool PrintingEnd(wxDC *dc);

	void SetSubstChar(wxByte ch, size_t len);

	// properties
	const wxString &GetDataPath() const { return mDataPath; }
	int  GetDocWidth();
	int  GetDocHeight();

	int  GetModel() const { return selected_model; }
	void GetPageInfo(int *minPage, int *maxPage, int *pageFrom, int *pageTo);

	int  GetPPIX(int);
	int  GetPPIY(int);
	void   SetLineFeedHeight(int, double);
	double GetLineFeedHeight(int);
	void SetLineFeedNumber(int val) { line_feed_number = val; }
	int  GetLineFeedNumber() const { return line_feed_number; }
	void SetVtLineNumber(int val) { vt_line_number = val; }
	int  GetVtLineNumber() const { return vt_line_number; }
	void SetHtPos(int val) { ht_pos = val; }
	int  GetHtPos() const { return ht_pos; }
	void SetWideMode(int, int);
	int  GetWideMode(int);
	void SetLineMode(int, int);
	int  GetLineMode(int);
	void SetCharSet(int, int);
	int  GetCharSet(int);
	void SetCPI(int, int);
	int  GetCPI(int);

	void SetDensity(int);
	void SetReverse(bool val) { reverse = val; }
	bool GetReverse() const { return reverse; }

	void SetWindowSize(int, int);

#ifdef USE_CASH_BUFFER
	void ClearCashData() { release_cashdata(); }
#else
	void ClearCashData() {}
#endif

	wxDC *AttachPrinterDC();
	void DetachPrinterDC();
};

///
/// 印刷制御クラス
///
class MpPrintout : public wxPrintout
{
private:
	MP_PRINTER *mpri;
	wxDC *pdc;
	wxPageSetupDialogData *pagesetdlg;
public:
	MpPrintout(MP_PRINTER *new_mpri, const wxString &title, wxPageSetupDialogData *new_pagesetdlg);
	void OnBeginPrinting();
	bool OnBeginDocument(int startPage, int endPage);
	bool OnPrintPage(int pageNum);
	void OnEndDocument();
	void OnEndPrinting();
	void GetPageInfo(int *minPage, int *maxPage, int *selPageFrom, int *selPageTo);
	bool HasPage(int pageNum);
	bool WritePageHeader(wxPrintout *printout, wxDC *dc, const wxString&text, float mmToLogical);
};

///
/// 印刷プレビュークラス
///
class MpPreviewFrame : public wxPreviewFrame
{
private:
	wxWindow *frame;
public:
	MpPreviewFrame(wxPrintPreview *preview, wxWindow *parent);
	~MpPreviewFrame();
};

#endif /* MP_PRINTER_H */
