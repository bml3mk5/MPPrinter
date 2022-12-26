/// @file mp_model.cpp
///
/// @brief MPPRINTER 機種依存クラス
///
/// @date 2011.09.02 Create
///
/// @author Copyright (c) 2011, Sasaji. All rights reserved.
///
#include "mp_model.h"
#include "mp_printer.h"
#include "config.h"
#include <wx/filename.h>
#include <wx/wfstream.h>

MP_MODEL::MP_MODEL(MP_PRINTER *obj)
{
	mpri = obj;
//	density = 0;
	reverse = mpri->GetReverse();
}

MP_MODEL::~MP_MODEL()
{
}

/// 初期化
void MP_MODEL::ClearSetting()
{
	param.line_feed_height = def_lf_height;
	param.paper_feed_height = 0.0;
	param.feed = 0;
}

/// 現在のパラメータを保存
void MP_MODEL::PushSetting()
{
	param_st = param;
}

/// 保存したパラメータを取り出す
void MP_MODEL::PopSetting()
{
	param = param_st;

	param.feed = 0;
}


/// 印字フォントの読み込み
bool MP_MODEL::read_font(const wxString &file_name, wxByte *font_data, size_t font_data_size)
{
	wxFileName font_path(mpri->GetDataPath(), file_name);

	wxFileInputStream stream(font_path.GetFullPath());
	if (!stream.IsOk()) {
		return false;
	}
	stream.Read((void *)font_data, font_data_size);
	return true;
}

/// 色の濃さを設定
void MP_MODEL::set_density()
{
//	density = gConfig.GetDensity();
	reverse = mpri->GetReverse();
}

/// 点を更新（色反転はしない）
void MP_MODEL::update_dot(DpType *dp)
{
	int dot = (int)(*dp);
	dot++;
	*dp = (dot <= COLOR_DENSITY_MAX ? (DpType)dot : COLOR_DENSITY_MAX);
}
