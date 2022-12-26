/// @file mp80.h
///
/// @brief MP80固有クラス
///
/// @date 2011.09.08 Create
///
/// @author Copyright (c) 2011, Sasaji. All rights reserved.
///
#ifndef _MP80_H_
#define _MP80_H_

#include "mp_model.h"
#include "mp_printer.h"

#define MP80_FONT_NAME		_T("prn_font_m8.dat")
#define MP80_FONT_DATA_SIZE	16388

/// MP80固有クラス
class MP80 : public MP_MODEL
{
private:
	wxByte font_data[MP80_FONT_DATA_SIZE];	///< 印字用フォント

	int	vt_max_num;
	int vt[16];	///< 垂直タブ位置
	int	ht_max_num;
	int ht[32];	///< 水平タブ位置

	void set_line_width();
	int  get_width_per_char();
	void clear_vt();
	void clear_ht();
public:
	MP80(MP_PRINTER *mp);

	bool ReadFont();
	void ClearSetting();
	void PopSetting();

	int PutData(wxByte *src, DpBufferDw *dst, bool draw);
	int PutFont(wxByte *src, DpBufferDw *dst, bool draw);
	int PutGraphic(wxByte *src, DpBufferDw *dst, bool draw);
	int ParseCtrlCode(wxByte *data, DpBufferDw *dst, wxUint32 *pfeed);

	void SetLineMode(int);
	void SetCPI(int);
	void OnLineFeed();
};

#endif
