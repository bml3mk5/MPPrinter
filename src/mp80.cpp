/// @file mp80.cpp
///
/// @brief MP80固有クラス
///
/// @date 2011.09.08 Create
///
/// @author Copyright (c) 2011, Sasaji. All rights reserved.
///
#include "mp80.h"
#include "dpbuffer.h"

#define MP80_BUF_WIDTH_L10	1320	///< A4横印字(10cpi,132字)の幅(10px x 132char)
#define MP80_BUF_WIDTH_L12	1320	///< A4横印字(12cpi,132字)の幅(8px x 165char)
#define MP80_BUF_WIDTH_P10	960		///< A4縦印字(10cpi,80字)の幅(12px x 80char)
#define MP80_BUF_WIDTH_P12	960		///< A4縦印字(12cpi,80字)の幅(10px x 96char)

/// 文字幅に対する文字フォントのドット適用位置
static const wxByte poc[9][24]={
	{ 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9,10,10,11,11 },	// 24px
	{ 0, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9,10,11, 0, 0, 0, 0 },	// 20px
	{ 0, 1, 1, 2, 3, 3, 4, 5, 6, 7, 8, 8, 9,10,10,11, 0, 0, 0, 0, 0, 0, 0, 0 },	// 16px
	{ 0, 1, 2, 3, 3, 4, 5, 6, 7, 8, 8, 9,10,11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// 14px
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// 12px
	{ 1, 2, 3, 4, 5, 6, 7, 8, 9,10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// 10px
	{ 1, 2, 4, 5, 6, 7, 9,10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	//  8px
	{ 1, 2, 4, 6, 7, 9,10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	//  7px
	{ 1, 3, 5, 6, 8,10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	//  6px
};
static const int poc_idx[26]={
	0,0,0,0,0,0,8,7,6,0,5,0,4,0,3,0,2,0,0,0,1,0,0,0,0,0
};

MP80::MP80(MP_PRINTER *obj) : MP_MODEL(obj)
{
	def_ppix = 120;					///< pixel per inch
	def_ppiy = 144;					///< pixel per inch
	def_lf_height = def_ppiy / 6;	///< 改行幅(1/6inch)

	disp_aspect_ratio.Set(2, 2);

	clear_vt();
	clear_ht();
}

void MP80::ClearSetting()
{
	MP_MODEL::ClearSetting();

	param.char_per_inch = 10;			///< 10cpi
	param.char_set = 0;					///< 0=L3記号 1=L3ひらがな 2=FM-8/7 3=PC-8001
	param.wide_mode = 0;				///< 標準=0 拡大=1 or 2 縮小=4
	param.under_line = 0;				///< アンダーラインなし

	/// 描画領域の幅
	SetLineMode(1);

	param.graphic_mode = 0;				///< テキストモード
	param.graphic_size = 0;				///< グラフィックモード時の印字サイズ

	clear_vt();
	clear_ht();
	return;
}

void MP80::PopSetting()
{
	MP_MODEL::PopSetting();

	clear_vt();
	clear_ht();
}

/// 印字フォントの読み込み
bool MP80::ReadFont()
{
	return read_font(MP80_FONT_NAME, font_data, MP80_FONT_DATA_SIZE);
}

/// 印字データを描画エリアに出力
/// @param[in]     src   データ
/// @param[in]     dst   書きこむバッファ
/// @param[in]     draw  印字処理するかしないか
/// @return              読んだデータ数
int MP80::PutData(wxByte *src, DpBufferDw *dst, bool draw)
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
		param.graphic_size--;
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
int MP80::PutFont(wxByte *src, DpBufferDw *dst, bool draw)
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
	wide = ((param.wide_mode & 3) != 0 && (dst->Rect().width + wpc) < param.line_width) ? 2 : 1;
	wpc *= wide;

	// アンダーラインの有無 改行幅2/144未満では無効
	uline = (param.line_feed_height > 2) ? param.under_line : 0;

	if (draw) {
		if (param.char_set == 1) {
			// ひらがな文字セット
			// グラフィック文字にはアンダーラインは引けない
			if (chr_code == 0xfe || chr_code == 0x00) {
				uline = 0;
			}
		} else {
			// 標準文字セット
			// グラフィック文字にはアンダーラインは引けない
			if ((chr_code >= 0x80 && chr_code <= 0x9f) || (chr_code >= 0xe0 && chr_code <= 0xf0) || (chr_code == 0xfe) || chr_code == 0x00) {
				uline = 0;
			}
		}

		// 文字フォントの位置をシフト
		chr_code = chr_code + (param.char_set * 256);

		// フォントデータ開始位置
		bit_data = &font_data[chr_code * 16];

		// 文字をバッファに出力
		int poc_no=poc_idx[wpc];
		for(xx=0; xx<wpc; xx++) {
			xl=poc[poc_no][xx];
			for(yy=0; yy<8; yy++) {
				dp = dst->Ptr(dst->Rect().width + xx, yy * dy);
				if (((((bit_data[xl]) & (1 << yy)) != 0) || (uline && yy == 7)) != reverse) {
					for(yl=0; yl<dy; yl++) {
						update_dot(dp+dst->Width()*yl);
					}
				}
			}
#if 0
			// アンダーライン
			if (uline) {
				for(yy=7; yy<8; yy++) {
					dp = dst->Ptr(*dw + xx, yy * dy);
					for(yl=0; yl<dy; yl++) {
						update_dot(dp+dst->Width()*yl);
					}
				}
			}
#endif
		}
	}
	dst->Rect().AddWidth(wpc);
	dst->Rect().height = (8 * dy);
	return 1;
}

/// グラフィックを出力
/// @param[in]     src   グラフィックデータ
/// @param[in]     dst   書きこむバッファ
/// @param[in]     draw  印字処理するかしないか
/// @return              読んだデータ数
int MP80::PutGraphic(wxByte *src, DpBufferDw *dst, bool draw)
{
	DpType *dp;
	int gdot = (param.graphic_mode == 1) ? 2 : 1;
	int dy = disp_aspect_ratio.y;
	int yy = 0;
	int yl = 0;

	if (draw) {
		for(yy=0; yy<8; yy++) {
			dp = dst->Ptr(dst->Rect().width, yy * dy);
			if (((src[0] & (0x80 >> yy)) != 0) != reverse) {
				for(yl=0; yl<dy; yl++) {
					update_dot(dp+dst->Width()*yl);
					if (gdot == 2) {
						update_dot(dp+dst->Width()*yl+1);
					}
				}
			}
		}
	}
	dst->Rect().AddWidth(gdot);
	dst->Rect().height = (8 * dy);
	return 1;
}

/// 制御コードの解析
/// @param[in]     data  印字データ
/// @param[in,out] dst   位置(pixel)
/// @param[out]    pfeed 改行指示
/// @return              解析したコードの長さ（バイト）、通常文字なら0
int MP80::ParseCtrlCode(wxByte *data, DpBufferDw *dst, wxUint32 *pfeed)
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
	case 0x00:	// NUL
		// [未対応] // TODO: NUL
		pos++;
		break;
	case 0x07:	// BEL
		// [未対応] // TODO: BEL
		pos++;
		break;
	case 0x08:	// BS
		// 左へ移動
		dst->Rect().SubWidth(get_width_per_char() * ((param.wide_mode & 3) ? 2 : 1));
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
					if (get_width_per_char() * now_pos * ((param.wide_mode & 3) ? 2 : 1) + dst->Rect().width < param.line_width) {
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
		// 垂直タブ
		if (vt_max_num > 0) {
			int now_line_num = mpri->GetVtLineNumber();
			if (now_line_num >= vt[vt_max_num-1]) {
				// 最終タブ位置を超えている場合改ページ
				param.feed |= MP_BIT_FF;
			} else {
				for(int i=0; i < vt_max_num; i++) {
					if (now_line_num < vt[i]) {
						now_line_num = vt[i] - now_line_num;
						break;
					}
				}
				if (now_line_num > 0) {
					// 改行数を設定
					param.feed |= MP_BIT_LF;
					mpri->SetLineFeedNumber(now_line_num);
				}
			}
		} else if (vt_max_num == 0) {
			// ESC B nul でタブをクリアしている場合は復帰
			param.feed |= MP_BIT_CR;
		} else {
			// 初期状態では1行改行
			param.feed |= MP_BIT_LF;
			mpri->SetLineFeedNumber(1);
		}
		pos++;
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
		// 横幅拡大文字モード(改行で解除)
		param.wide_mode |= 1;
		pos++;
		break;
	case 0x0f:	// SI
		// 横幅縮小文字モード
		param.wide_mode |= 4;
		pos++;
		break;
	case 0x11:	// DC1
		// [未対応] // TODO: DC1
		pos++;
		break;
	case 0x12:	// DC2
		// 横幅縮小文字モード解除
		param.wide_mode &= ~4;
		pos++;
		break;
	case 0x13:	// DC3
		// [未対応] // TODO: DC3
		pos++;
		break;
	case 0x14:	// DC4
		// 横幅拡大文字モード解除(SOで拡大した場合)
		if (param.wide_mode & 1) param.wide_mode &= ~1;
		pos++;
		break;
	case 0x18:	// CAN
		// バッファクリア
		mpri->ClearPrintBuffer();
		pos++;
		break;
	case 0x1b:	// ESC
		pos++;
		switch(data[pos]) {
		case 0x20:	// ' '
			// 文字間隔の設定
			// [未対応] // TODO: ESC ' '
			pos+=2;
			break;
		case 0x21:	// '!'
			// 文字モードの一括設定
			// [一部未対応] // TODO: ESC !
			param.under_line = (data[pos + 1] & 0x80) ? 1 : 0;	// アンダーライン
			// 横幅拡大
			if (data[pos + 1] & 0x20) {
				param.wide_mode |= 2; 
			} else {
				param.wide_mode &= ~2; 
			}
			// 横幅縮小
			if (data[pos + 1] & 0x04) {
				param.wide_mode |= 4; 
			} else {
				param.wide_mode &= ~4; 
			}
			param.char_per_inch = (data[pos + 1] & 0x01) ? 12 : 10;	// 文字ピッチ
			pos+=2;
			break;
		case 0x24:	// '$'
			// 絶対位置移動
			// [未対応] // TODO: ESC $
			pos+=3;
			break;
		case 0x25:	// '%'
			// 1バイトコード登録文字セットの指定/解除
			// [未対応] // TODO: ESC %
			pos+=3;
			break;
		case 0x2d:	// '-'
			// アンダーライン印字開始/終了
			param.under_line = (data[pos + 1] & 1);
			pos+=2;
			break;
		case 0x30:	// '0'
			// 1/8inch改行
			param.line_feed_height = (def_ppiy / 8.0);
			pos++;
			break;
		case 0x31:	// '1'
			// 7/72inch改行
			param.line_feed_height = (def_ppiy * 7.0 / 72.0);
			pos++;
			break;
		case 0x32:	// '2'
			// 1/6inch改行
			param.line_feed_height = (def_ppiy / 6.0);
			pos++;
			break;
		case 0x33:	// '3'
			// n/216inch改行
			param.line_feed_height = (data[pos + 1] * def_ppiy / 216.0);
			pos+=2;
			break;
		case 0x34:	// '4'
			// イタリック体設定
			// [未対応] // TODO: ESC 4
			pos++;
			break;
		case 0x35:	// '5'
			// イタリック体解除
			// [未対応] // TODO: ESC 5
			pos++;
			break;
		case 0x36:	// '6'
			// 上位側コントロール・コード解除
			// [未対応] // TODO: ESC 6
			pos++;
			break;
		case 0x37:	// '7'
			// 上位側コントロール・コード指定
			// [未対応] // TODO: ESC 7
			pos++;
			break;
		case 0x38:	// '8'
			// 用紙切れセンサー切
			// [未対応] // TODO: ESC 8
			pos++;
			break;
		case 0x39:	// '9'
			// 用紙切れセンサー入
			// [未対応] // TODO: ESC 9
			pos++;
			break;
		case 0x40:	// '@'
			// リセット
			mpri->ClearPrintBuffer();	// バッファクリア
			ClearSetting();
			pos++;
			break;
		case 0x41:	// 'A'
			// n/72inch改行
			param.line_feed_height = (data[pos + 1] * def_ppiy / 72.0);
			pos+=2;
			break;
		case 0x42:	// 'B'
			// 垂直タブ設定
			pos++;
			mpri->SetVtLineNumber(0);
			vt_max_num=0;
			memset(vt, 0, sizeof(vt));
			for(int m=0, i=0; i < 16 && data[pos] != 0; i++) {
				if (data[pos] > m) {
					vt[vt_max_num++]=data[pos];
					m=data[pos];
				} else {
					m = 99999;
				}
				pos++;
			}
			if (data[pos] == 0) pos++;
			break;
		case 0x43:	// 'C'
			// ページ長設定
			// [未対応] // TODO: ESC C ?
			if (data[pos + 1] == 0x00) {
				// インチ単位での設定
				pos++;
			}
			pos+=2;
			break;
		case 0x44:	// 'D'
			// 水平タブ設定
			pos++;
			ht_max_num=0;
			memset(ht, 0, sizeof(ht));
			for(int m=0, i=0; i < 32 && data[pos] != 0; i++) {
				if (data[pos] > m) {
					ht[ht_max_num++]=data[pos];
					m=data[pos];
				} else {
					m = 99999;
				}
				pos++;
			}
			if (data[pos] == 0) pos++;
			break;
		case 0x45:	// 'E'
			// 強調印字開始
			// [未対応] // TODO: ESC E
			pos++;
			break;
		case 0x46:	// 'F'
			// 強調印字解除
			// [未対応] // TODO: ESC F
			pos++;
			break;
		case 0x47:	// 'G'
			// 二重印字開始
			// [未対応] // TODO: ESC G
			pos++;
			break;
		case 0x48:	// 'H'
			// 二重印字解除
			// [未対応] // TODO: ESC H
			pos++;
			break;
		case 0x49:	// 'I'
			// キャラクタ/制御コード選択
			// [未対応] // TODO: ESC I
			pos+=2;
			break;
		case 0x4a:	// 'J'
			// n/216inch順方向紙送り(ヘッド位置は変わらず)
			param.paper_feed_height = (data[pos + 1] * def_ppiy / 216.0);
			param.feed |= MP_BIT_PF;
			pos+=2;
			break;
		case 0x4b:	// 'K'
			// 8ドット単精度グラフィック
			param.graphic_mode = 1;
			// 長さ2byte
			param.graphic_size = data[pos + 1] + data[pos + 2] * 256;
			if (param.graphic_size == 0) {
				param.graphic_mode = 0;
			}
			pos+=3;
			break;
		case 0x4c:	// 'L'
			// 8ドット倍精度グラフィック
			param.graphic_mode = 2;
			// 長さ2byte
			param.graphic_size = data[pos + 1] + data[pos + 2] * 256;
			if (param.graphic_size == 0) {
				param.graphic_mode = 0;
			}
			pos+=3;
			break;
		case 0x4d:	// 'M'
			// 12cpi
			param.char_per_inch = 12;
			pos++;
			break;
		case 0x4e:	// 'N'
			// ミシン目スキップ行数設定（下マージン）
			// [未対応] // TODO: ESC N
			pos++;
			break;
		case 0x4f:	// 'O'
			// ミシン目スキップ行数設定解除
			// [未対応] // TODO: ESC O
			pos++;
			break;
		case 0x50:	// 'P'
			// 10cpi
			param.char_per_inch = 10;
			pos++;
			break;
		case 0x51:	// 'Q'
			// 右マージン位置設定
			// [未対応] // TODO: ESC Q
			pos+=2;
			break;
		case 0x52:	// 'R'
			// 国別文字コード選択
			// [未対応] // TODO: ESC R
			pos+=2;
			break;
		case 0x53:	// 'S'
			// スーパー／サブスクリプトの設定
			// [未対応] // TODO: ESC S
			pos++;
			break;
		case 0x54:	// 'T'
			// スーパー／サブスクリプトの解除
			// [未対応] // TODO: ESC T
			pos++;
			break;
		case 0x55:	// 'U'
			// 単方向印字指定/解除
			// [未対応] // TODO: ESC U
			pos+=2;
			break;
		case 0x57:	// 'W'
			// 横幅拡大文字モード開始/終了
			if (data[pos + 1] & 1) {
				param.wide_mode |= 2;
			} else {
				param.wide_mode &= ~2;
			}
			pos+=2;
			break;
		case 0x59:	// 'Y'
			// 8ドット倍速倍精度グラフィック
			param.graphic_mode = 2;
			// 長さ2byte
			param.graphic_size = data[pos + 1] + data[pos + 2] * 256;
			if (param.graphic_size == 0) {
				param.graphic_mode = 0;
			}
			pos+=3;
			break;
		case 0x5a:	// 'Z'
			// 8ドット4倍精度グラフィック
			// [未対応] // TODO: ESC Z
//			param.graphic_mode = 3;
			// 長さ2byte
//			param.graphic_size = data[pos + 1] + data[pos + 2] * 256;
//			if (param.graphic_size == 0) {
//				param.graphic_mode = 0;
//			}
			pos += data[pos + 1] + data[pos + 2] * 256;
			pos+=3;
			break;
		case 0x5c:	// '\'
			// 相対位置移動
			// [未対応] // TODO: ESC '\'
			pos+=3;
			break;
		case 0x74:	// 't'
			// １バイトコード表選択
			// [未対応] // TODO: ESC t
			pos+=2;
			break;
		default:
			// 不明or未対応コード
			pos++;
			break;
		}
		break;
	case 0x1c:	// FS
	case 0x1d:	//
	case 0x1e:	//
	case 0x1f:	//
		// [未対応] // TODO: FS
		pos++;
		break;
	case 0x7f:	// DEL
		// 一文字削除
		// [未対応] // TODO: DEL
		pos++;
		break;
	default:
		// 未対応コード
		if (0 <= data[pos] && data[pos] < 0x20) {
			pos++;
		}
		break;
	}

	*pfeed = param.feed;
	return pos;
}

/// 1行の幅設定
/// @param[in] val 0:A4横 1:A4縦
void MP80::SetLineMode(int val)
{
	param.line_mode = val;
	set_line_width();
}

/// 文字幅設定
/// @param[in] val 10 / 12
void MP80::SetCPI(int val)
{
	param.char_per_inch = val;
	set_line_width();
}

///
void MP80::set_line_width()
{
	switch(param.line_mode) {
		case 0:
			// A4横
			if (param.char_per_inch == 12) {
				param.line_width = MP80_BUF_WIDTH_L12;
			} else {
				param.line_width = MP80_BUF_WIDTH_L10;
			}
			break;
		case 1:
			// A4縦
			if (param.char_per_inch == 12) {
				param.line_width = MP80_BUF_WIDTH_P12;
			} else {
				param.line_width = MP80_BUF_WIDTH_P10;
			}
			break;
	}
}

/// 1文字の幅
int MP80::get_width_per_char()
{
	int wpc = (param.char_per_inch == 12 ? 10 : 12);
	if (param.line_mode == 0) wpc -= 2;
	// 縮小モード
	if (param.wide_mode & 4) {
		wpc = (param.char_per_inch == 12 ? 6 : 7);
	}
	return wpc;
}

/// 改行or復帰時のイベント
void MP80::OnLineFeed()
{
	if (param.wide_mode & 1) param.wide_mode &= ~1;
}

/// 垂直タブ位置初期化
void MP80::clear_vt()
{
	vt_max_num = -1;
	memset(vt, 0, sizeof(vt));
}

/// 水平タブ位置初期化
void MP80::clear_ht()
{
	for(ht_max_num=0; ht_max_num<32; ht_max_num++) {
		ht[ht_max_num]=8*(ht_max_num+1);
	}
}