/// @file mp1020.cpp
///
/// @brief MP1020固有クラス
///
/// @date 2011.08.11 Create
///
/// @author Copyright (c) 2011, Sasaji. All rights reserved.
///
#include "mp1020.h"
#include "dpbuffer.h"

#define MP1020_BUF_WIDTH_N	1452	///< 縮小印字の幅(11px x 132char)
#define MP1020_BUF_WIDTH_L	960		///< ロングライン時の幅(12px x 80char)
#define MP1020_BUF_WIDTH_S	768		///< ショートライン時の幅(12px x 64char)


MP1020::MP1020(MP_PRINTER *obj) : MP_MODEL(obj)
{
	def_ppix = 128;					///< pixel per inch
	def_ppiy = 144;					///< pixel per inch
	def_lf_height = def_ppiy / 6;	///< 改行幅(1/6inch)

	disp_aspect_ratio.Set(2, 2);

	double_density = false;

	clear_vt();
	clear_ht();
}

void MP1020::ClearSetting()
{
	MP_MODEL::ClearSetting();

	param.char_per_inch = 10;				///< 未使用
	param.char_set = 0;					///< インターレース=0 ノンインターレース=1
	param.wide_mode = 0;					///< 標準=0 拡大文字=1
	param.under_line = 0;					///< アンダーラインなし
	/// 描画領域の幅
	SetLineMode(1);

	param.graphic_mode = 0;				///< テキストモード
	param.graphic_size = 0;				///< グラフィックモード時の印字サイズ

	double_density = false;

	clear_vt();
	clear_ht();

	return;
}

void MP1020::PopSetting()
{
	MP_MODEL::PopSetting();

	clear_vt();
	clear_ht();
}

/// 印字フォントの読み込み
bool MP1020::ReadFont()
{
	return read_font(MP1020_FONT_NAME, font_data, MP1020_FONT_DATA_SIZE);
}

/// 印字データを描画エリアに出力
/// @param[in]     src   データ
/// @param[in]     dst   書きこむバッファ
/// @param[in]     draw  印字処理するかしないか
/// @return              読んだデータ数
int MP1020::PutData(wxByte *src, DpBufferDw *dst, bool draw)
{
	int rc = 1;

	set_density();

	if (param.graphic_mode == 0) {
		if (param.feed == 0) {
			// 文字印字
			rc = PutFont(src, dst, draw);
		}
	} else {
		// グラフィック印字
		rc = PutGraphic(src, dst, draw);
		param.graphic_size -= rc;
		if (param.graphic_size <= 0) {
			param.graphic_mode = 0;
			param.graphic_size = 0;
		}
	}

	return rc;
}

/// 文字コードに対応したフォントを出力
/// @param[in]     src   文字コード
/// @param[in]     dst   書きこむバッファ
/// @param[in]     draw  印字処理するかしないか
/// @return              読んだデータ数
int MP1020::PutFont(wxByte *src, DpBufferDw *dst, bool draw)
{
	wxUint32 chr_code = src[0];
	wxByte *bit_data;
	DpType *dp;
	int wide;
	int uline;
	int wpc;
	int dy = disp_aspect_ratio.y;
	int xx = 0;
	int xl = 0;
	int yy = 0;
	int yl = 0;

	// 一文字の幅
	wpc = get_width_per_char();

	// 拡大文字にするか
	wide = (param.wide_mode == 1 && (dst->Rect().width + wpc) < param.line_width) ? 2 : 1;

	// アンダーラインの有無 改行幅2/144未満では無効
	uline = (param.line_feed_height > 2) ? param.under_line : 0;

	if (draw) {
		if (param.char_set == 0) {
			// インターレース時
			// グラフィック文字にはアンダーラインは引けない
			if ((chr_code >= 0x1c && chr_code <= 0x1f) || chr_code == 0xfe || chr_code == 0x00) {
				uline = 0;
			}
		} else {
			// ノンインターレース時
			// グラフィック文字にはアンダーラインは引けない
			if ((chr_code >= 0x1c && chr_code <= 0x1f) || (chr_code >= 0x80 && chr_code <= 0x9f) || (chr_code >= 0xe0 && chr_code <= 0xf0) || (chr_code >= 0xf8 && chr_code <= 0xfe) || chr_code == 0x00) {
				uline = 0;
			}
			// 文字フォントの位置をシフト
			if (chr_code >= 0x80 && chr_code <= 0x9f) {
				chr_code += 0x80;
			} else if (chr_code >= 0xe0 && chr_code <= 0xff) {
				chr_code += 0x40;
			}
		}
		// フォントデータ開始位置
		bit_data = &font_data[chr_code * 16];

		// 文字をバッファに出力
		for(xl=0; xl<12; xl++) {
			for(yy=0; yy<8; yy++) {
				dp = dst->Ptr(dst->Rect().width + (xx * wide), yy * dy);
				if (((((*bit_data) & (1 << yy)) != 0) || (uline && yy == 7)) != reverse) {
					for(yl=0; yl<dy; yl++) {
						update_dot(dp+dst->Width()*yl);
						if (wide == 2) {
							update_dot(dp+dst->Width()*yl+1);
						}
					}
				}
			}
#if 0
			// アンダーライン
			if (uline) {
				for(yy=7; yy<8; yy++) {
					dp = dst->Ptr(*dw+(xx * wide), yy * dy);
					for(yl=0; yl<dy; yl++) {
						update_dot(dp+dst->Width()*yl);
						if (wide == 2) {
							update_dot(dp+dst->Width()*yl+1);
						}
					}
				}
			}
#endif
			// 縮小印字では一部ビットは印字しない。
			if (!(param.line_mode == 0 && (xl >= 11))) {
				xx++;
			}
			bit_data++;
		}
	}
	dst->Rect().AddWidth(wpc * wide);
	dst->Rect().height = (8 * dy);
	return 1;
}

/// グラフィックを出力
/// @param[in]     src   グラフィックデータ
/// @param[in]     dst   書きこむバッファ
/// @param[in]     draw  印字処理するかしないか
/// @return              読んだデータ数
int MP1020::PutGraphic(wxByte *src, DpBufferDw *dst, bool draw)
{
	DpType *dp;
	int gdot = (param.graphic_mode == 1) ? 2 : 1;
	int dy = disp_aspect_ratio.y;
	int yy = 0;
	int yl = 0;
	int yymax = 8;
	wxUint16 src16,smask;

	if (double_density) {
		// 縦16ドット(mp1052相当)で印字 MSBが下-LSBが上
		dy /= 2;
		yymax *= 2;
		src16 = (src[1] << 8) | src[0];
		smask = 0x0001;
	} else {
		// 縦8ドットで印字 MSBが上-LSBが下
		src16 = src[0];
		smask = 0x80;
	}

	if (draw) {
		for(yy=0; yy<yymax; yy++) {
			dp = dst->Ptr(dst->Rect().width, yy * dy);
			if (((src16 & smask) != 0) != reverse) {
				for(yl=0; yl<dy; yl++) {
					update_dot(dp+dst->Width()*yl);
					if (gdot == 2) {
						update_dot(dp+dst->Width()*yl+1);
					}
				}
			}
			smask = (double_density ? smask << 1 : smask >> 1);
		}
	}
	dst->Rect().AddWidth(gdot);
	dst->Rect().height = (yymax * dy);
	return double_density ? 2 : 1;
}

/// 制御コードの解析
/// @param[in]     data  印字データ
/// @param[in,out] dst   位置(pixel)
/// @param[out]    pfeed 改行指示
/// @return              解析したコードの長さ（バイト）、通常文字なら0
int MP1020::ParseCtrlCode(wxByte *data, DpBufferDw *dst, wxUint32 *pfeed)
{
	int pos = 0;
	param.feed = 0;

	// グラフィックモード時は解析しない
	if (param.graphic_mode != 0) {
		*pfeed = param.feed;
		return pos;
	}
	// コントロールコード
	switch(data[pos]) {
	case 0x00:	//
		// [未対応] // TODO: NUL
		pos++;
		break;
	case 0x08:	// BS
		// 左へ移動
		dst->Rect().SubWidth(get_width_per_char() * ((param.wide_mode != 0) ? 2 : 1));
		pos++;
		break;
	case 0x09:	// HT
		// 水平タブ
		if (ht_max_num > 0) {
			int now_pos = mpri->GetHtPos();
			if (now_pos < ht[ht_max_num-1]) {
				for(int i=0; i < ht_max_num; i++) {
					if (now_pos < ht[i]) {
						now_pos = ht[i] - now_pos;
						break;
					}
				}
				if (now_pos > 0) {
					// 右端をはみ出していないなら文字数を設定
					if (get_width_per_char() * now_pos * (param.wide_mode != 0 ? 2 : 1) + dst->Rect().width < param.line_width) {
						mpri->SetSubstChar(0x00, now_pos);
					}
				}
			}
		}
		pos++;
		break;
	case 0x0a:	// LF
		param.feed |= MP_BIT_LF;
		mpri->SetLineFeedNumber(1);
		pos++;
		break;
	case 0x0b:	// VT
		// 垂直タブ用紙送り
		pos++;
		if (data[pos] >= 0x31 && data[pos] <= 0x39) {
			int now_line_num = vt[data[pos]-0x31] - mpri->GetVtLineNumber();
			if (now_line_num > 0) {
				// 改行数を設定
				param.feed |= MP_BIT_LF;
				mpri->SetLineFeedNumber(now_line_num);
			} else {
				// 1行改行
				param.feed |= MP_BIT_LF;
				mpri->SetLineFeedNumber(1);
			}
			pos++;
		} else {
			// 無効なチャンネルNo.の場合は空白を出力
			mpri->SetSubstChar(0x20, 1);
			pos++;
		}
		break;
	case 0x0c:	// FF
		param.feed |= MP_BIT_FF;
		pos++;
		break;
	case 0x0d:	// CR
		param.feed |= MP_BIT_CR;
		pos++;
		break;
	case 0x0e:	// SO
		param.char_set = 1;
		pos++;
		break;
	case 0x0f:	// SI
		param.char_set = 0;
		pos++;
		break;
	case 0x14:	// DC4
		// 垂直タブ設定
		pos++;
		mpri->SetVtLineNumber(0);
		vt_max_num = 0;
		memset(vt, 0, sizeof(vt));
		for(int sp=0; sp < 128 && data[pos] != '?';) {
			if (data[pos] >= 0x31 && data[pos] <= 0x39) {
				// チャンネルNo.
				int new_vt_num = data[pos] - 0x30;
				for(int v=vt_max_num; v < new_vt_num; v++) {
					vt[v]=sp;
				}
				if (vt_max_num < new_vt_num) {
					vt_max_num = new_vt_num;
				}
			} else {
				sp++;
			}
			pos++;
		}
		if (data[pos] == '?') pos++;
		break;
	case 0x18:	// CAN
		// バッファクリア
		mpri->ClearPrintBuffer();
		// 標準文字モードにする
		param.wide_mode = 0;

		pos++;
		break;
	case 0x1b:	// ESC
		pos++;
		switch(data[pos]) {
		case 0x0b:	// VT
			// スキップ量設定
			{
				int num = (data[pos + 1] & 0x0f) * 10 + (data[pos + 2] & 0x0f);
				if (num > 0) {
					param.feed |= MP_BIT_LF;
					mpri->SetLineFeedNumber(num);
				}
				pos+=3;
			}
			break;
		case 0x25:	// '%'
			if (data[pos + 1] == 0x31) { // '1'
				// 8ドット倍精度グラフィック/16ドット倍精度グラフィック
				param.graphic_mode = 2;
				// 長さ2byte
				param.graphic_size = data[pos + 2] * 256 + data[pos + 3];
				if (param.graphic_size == 0) {
					param.graphic_mode = 0;
				} else {
					// 改行幅を強引に1/9inchにする
					param.line_feed_height = (16 * def_ppiy / 144);
				}
				if (double_density) {
					// (ESC Sのとき、長さの単位はワード)
					param.graphic_size *= 2;
				}
				pos+=3;
			}
			else if (data[pos + 1] == 0x32) { // '2'
				// 8ドット単精度グラフィック/16ドット単精度グラフィック
				param.graphic_mode = 1;
				// 長さ2byte
				param.graphic_size = data[pos + 2] * 256 + data[pos + 3];
				if (param.graphic_size == 0) {
					param.graphic_mode = 0;
				}
				if (double_density) {
					// (ESC Sのとき、長さの単位はワード)
					param.graphic_size *= 2;
				}
				pos+=3;
			}
			else if (data[pos + 1] == 0x39) { // '9'
				pos+=2;
				// n/144inch改行
				param.line_feed_height = (data[pos] * def_ppiy / 144);
			}
			pos++;
			break;
		case 0x35:	// '5'
			// top of from位置設定
			// [未対応] // TODO: ESC 5
			pos++;
			break;
		case 0x36:	// '6'
			// 1/6inch改行
			param.line_feed_height = (def_ppiy / 6);
			pos++;
			break;
		case 0x38:	// '8'
			// 1/8inch改行
			param.line_feed_height = (def_ppiy / 8);
			pos++;
			break;
		case 0x41:	// 'A'
			// ロングライン(80字)モード
			SetLineMode(1);

			pos++;
			break;
		case 0x42:	// 'B'
			// ショートライン(64字)モード
			SetLineMode(2);

			pos++;
			break;
		case 0x43:	// 'C'
			// アンダーライン印字開始
			param.under_line = 1;
			pos++;
			break;
		case 0x44:	// 'D'
			// アンダーライン印字終了
			param.under_line = 0;
			pos++;
			break;
		case 0x46:	// 'F'
			// ページ長設定(1/2インチ単位)
			// [未対応] // TODO: ESC F x1 x2
			pos+=3;
			break;
		case 0x47:	// 'G'
			// 縮小文字モード
			// ここでは、ロング(132字)モード+標準文字モードで対応
			SetLineMode(0);
			param.wide_mode = 0;

			pos++;
			break;
		case 0x49:	// 'I'
			// 漢字モード開始
			// 以降をJISコードと判断する
			// [未対応] // TODO: ESC I (mp1052/mp1053?)
			pos++;
			break;
		case 0x4a:	// 'J'
			// 漢字モード終了
			// 以降をASCIIコードと判断する
			// [未対応] // TODO: ESC J (mp1052/mp1053?)
			pos++;
			break;
		case 0x4b:	// 'K'
			// 不明
			// [未対応] // TODO: ESC K (mp1052/mp1053?)
			pos++;
			break;
		case 0x52:	// 'R'
			// 標準文字モード
			param.wide_mode = 0;
			pos++;
			break;
		case 0x53:	// 'S' (mp1052/mp1053?)
			// 倍密度モード？(縦16/24ドット)
			double_density = true;
			pos++;
			break;
		case 0x54:	// 'T' (mp1052/mp1053?)
			// 単密度モード？(縦8ドット)
			double_density = false;
			pos++;
			break;
		case 0x55:	// 'U'
			// 拡大文字モード
			param.wide_mode = 1;
			pos++;
			break;
		case 0x59:	// 'Y'
			// 不明
			// [未対応] // TODO: ESC Y (mp1052/mp1053?)
			pos++;
			break;
		default:
			// 不明or未対応コード
			pos++;
			break;
		}
		break;
#if 0
	case 0x1c:	// 漢字モード時の拡張コード？ (mp1052 or 1053用)
		pos++;
		switch(data[pos]) {
		case 0x24:
			break;
		case 0x32:
			break;
		}
		break;
#endif
	default:
		// 未対応コード
		if (0 <= data[pos] && data[pos] < 0x1c) {
			pos++;
		}
		break;
	}

	*pfeed = param.feed;
	return pos;
}

/// 描画領域の幅を設定
/// @param[in] val 0:縮小印字の幅(11px x 132char)
///                1:ロングライン時の幅(12px x 80char)
///                2:ショートライン時の幅(12px x 64char)
void MP1020::SetLineMode(int val)
{
	param.line_mode = val;
	param.line_width = MP1020_BUF_WIDTH_L;
	// 描画領域の幅
	switch (param.line_mode) {
		case 0:
			param.line_width = MP1020_BUF_WIDTH_N;
			break;
		case 1:
			param.line_width = MP1020_BUF_WIDTH_L;
			break;
		case 2:
			param.line_width = MP1020_BUF_WIDTH_S;
			break;
	}
}

/// 1文字の幅
int MP1020::get_width_per_char()
{
	int wpc = (param.line_mode == 0 ? 11 : 12);
	return wpc;
}

/// 改行or復帰時のイベント
void MP1020::OnLineFeed()
{
	param.under_line = 0;
}

/// 垂直タブ位置初期化
void MP1020::clear_vt()
{
	vt_max_num = 0;
	memset(vt, 0, sizeof(vt));
}

/// 水平タブ位置初期化
void MP1020::clear_ht()
{
	for(ht_max_num=0; ht_max_num<32; ht_max_num++) {
		ht[ht_max_num]=8*(ht_max_num+1);
	}
}