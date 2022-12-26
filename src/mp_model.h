/// @file mp_model.h
///
/// @brief MPPRINTER 機種依存クラス
///
/// @date 2011.09.02 Create
///
/// @author Copyright (c) 2011, Sasaji. All rights reserved.
///
#ifndef _MP_MODEL_H_
#define _MP_MODEL_H_

#include "common.h"
#include <wx/wx.h>
#include "mp_printer.h"

class MP_PRINTER;
class DpBuffer;

/// 印字制御用パラメータ
class MP_MODEL_PARAM
{
public:
	//
	wxUint32 feed;				///< 復帰:1 改行:2 紙送り:4 改ページ:8

	double line_feed_height;	///< 改行幅 n pixel
	double paper_feed_height;	///< 紙送り時の幅 n pixel

	int line_mode;			///< 1行の長さモード
	int line_width;			///< 1行の幅 pixel

	int wide_mode;			///< 拡大/標準文字幅モード
	int char_per_inch;		///< １インチの文字数
	int char_set;			///< 文字コードセット
	int under_line;			///< アンダーライン印字=1

	int graphic_mode;		///< グラフィックモード
	int graphic_size;		///< グラフィックモード時の印字サイズ
};

///
/// 機種依存 クラス
///
class MP_MODEL
{
protected:
	MP_PRINTER *mpri;

	// 初期値保存用
	int def_ppix;		///< pixel per inch (X)
	int def_ppiy;		///< pixel per inch (Y)
	double def_lf_height;	///< 改行幅 n pixel

	MyPoint disp_aspect_ratio;	///< 画面表示でのフォントの縦横比

	//
	MP_MODEL_PARAM param;	///< 印字制御用パラメータ
	MP_MODEL_PARAM param_st; ///< 印字開始時パラメータ

//	int density;		///< 色の濃さ
	bool reverse;		///< 色を反転

	bool read_font(const wxString &file_name, wxByte *font_data, size_t font_data_size);
	virtual void set_density();
	virtual void update_dot(DpType *dp);
public:
	MP_MODEL(MP_PRINTER *obj = NULL);
	virtual ~MP_MODEL();

	virtual void ClearSetting();
	virtual void PushSetting();
	virtual void PopSetting();

	virtual bool ReadFont()				{ return false; }

	virtual int GetPPIX()				{ return def_ppix; }
	virtual int GetPPIY()				{ return def_ppiy; }
	virtual double GetDefLineFeedHeight()		{ return def_lf_height; }

	virtual double GetLineFeedHeight()			{ return param.line_feed_height; }
	virtual void SetLineFeedHeight(double val)	{ param.line_feed_height = val;	}

	virtual double GetPaperFeedHeight()			{ return param.paper_feed_height; }
	virtual void SetPaperFeedHeight(double val)	{ param.paper_feed_height = val; }


	virtual int GetLineMode()			{ return param.line_mode; }
	virtual void SetLineMode(int val)	{ param.line_mode = val; }
	virtual int GetLineWidth()			{ return param.line_width; }

	virtual wxUint32 GetFeed()				{ return param.feed; }

	virtual int GetWideMode()			{ return param.wide_mode; }
	virtual void SetWideMode(int val)	{ param.wide_mode = val; }
	virtual int GetCharSet()			{ return param.char_set; }
	virtual void SetCharSet(int val)	{ param.char_set = val; }
	virtual int GetCPI()				{ return param.char_per_inch; }
	virtual void SetCPI(int val)		{ param.char_per_inch = val; }

	virtual int GetGraphicMode()		{ return param.graphic_mode; }
	virtual void GetDispAspectRatio(MyPoint *val) { *val = disp_aspect_ratio; }

	/// 印字データを描画エリアに出力
	/// @param[in]     src   データ
	/// @param[in,out] dst   書きこむバッファ
	/// @param[in]     draw  印字処理するかしないか
	/// @return              読んだデータ数
	virtual int PutData(wxByte *src, DpBufferDw *dst, bool draw) { return 0; }
	/// 文字の印字データを描画エリアに出力
	/// @param[in]     src   文字コード
	/// @param[in,out] dst   書きこむバッファ
	/// @param[in]     draw  印字処理するかしないか
	/// @return              読んだデータ数
	virtual int PutFont(wxByte *src, DpBufferDw *dst, bool draw) { return 0; }
	/// グラフィックの印字データを描画エリアに出力
	/// @param[in]     src   グラフィックデータ
	/// @param[in,out] dst   書きこむバッファ
	/// @param[in]     draw  印字処理するかしないか
	/// @return              読んだデータ数
	virtual int PutGraphic(wxByte *src, DpBufferDw *dst, bool draw) { return 0; }
	/// 制御コードの解析
	/// @param[in]     data  印字データ
	/// @param[in,out] dst   位置(pixel)
	/// @param[out]    pfeed 改行指示
	/// @return              解析したコードの長さ（バイト）、通常文字なら0
	virtual int ParseCtrlCode(wxByte *data, DpBufferDw *dst, wxUint32 *pfeed) { return 0; }

	/// 改行or復帰時のイベント
	virtual void OnLineFeed()			{}
};

#endif
