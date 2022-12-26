/// @file mp1020.h
///
/// @brief MP1020固有クラス
///
/// @date 2011.08.11 Create
///
/// @author Copyright (c) 2011, Sasaji. All rights reserved.
///
#ifndef _MP1020_H_
#define _MP1020_H_

#include "mp_model.h"
#include "mp_printer.h"

#define MP1020_FONT_NAME		_T("prn_font_l3.dat")
#define MP1020_FONT_DATA_SIZE	5128

/// MP1020固有クラス
class MP1020 : public MP_MODEL
{
private:
	wxByte font_data[MP1020_FONT_DATA_SIZE];	///< 印字用フォント

	int	vt_max_num;
	int vt[10];	///< 垂直タブ位置
	int	ht_max_num;
	int ht[32];	///< 水平タブ位置

	bool double_density;	///< 倍精度モード

	int  get_width_per_char();
	void clear_vt();
	void clear_ht();

public:
	MP1020(MP_PRINTER *mp);

	bool ReadFont();
	void ClearSetting();
	void PopSetting();

	int PutData(wxByte *src, DpBufferDw *dst, bool draw);
	int PutFont(wxByte *src, DpBufferDw *dst, bool draw);
	int PutGraphic(wxByte *src, DpBufferDw *dst, bool draw);
	int ParseCtrlCode(wxByte *data, DpBufferDw *dst, wxUint32 *pfeed);

	void SetLineMode(int);

	void OnLineFeed();

};

#endif
