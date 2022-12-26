/// @file dpbuffer.cpp
///
/// @brief 描画用バッファ
///
/// @date 2019.11.20 Create
///
/// @author Copyright (c) 2019, Sasaji. All rights reserved.
///
#ifndef DPBUFFER_H
#define DPBUFFER_H

#include "common.h"
#include "coordinate.h"
#include <wx/defs.h>

//typedef wxUint16 DpType;
typedef wxByte DpType;

#ifdef _DEBUG
#define DpTypeMax COLOR_DENSITY_MAX
#define DpTypeMin 0
#else
#define DpTypeMax COLOR_DENSITY_MAX
#define DpTypeMin 0
#endif

/// @brief 描画用バッファベース
class DpBuffer
{
private:
	MySize   siz;		///< バッファサイズ
	DpType  *buffer;	///< バッファ
	DpType  dummy;		///< ダミーデータ

public:
	DpBuffer();
	DpBuffer(int width_, int height_);
	virtual ~DpBuffer();

	/// @brief バッファを作成
	virtual void Create(int width_, int height_);
	/// @brief バッファを削除
	virtual void Delete();

	/// @brief データをセット
	void Set(int x, int y, DpType data);
	/// @brief データを返す
	const DpType &Get(int x, int y) const;

	/// @brief バッファポインタを返す
	DpType *Ptr(int x, int y);

	/// @brief ピクセルデータの内容を左へシフト
	void LeftShiftTo(int width);
	/// @brief ピクセルデータの内容をクリア
	void ClearLeftTo(int width);
	/// @brief ピクセルデータの内容をコピー
	void CopyTo(DpBuffer *dst_buffer, int dx, int dy, int width, int height);

	/// @brief 幅を返す
	int Width() const { return siz.width; }
	/// @brief 高さを返す
	int Height() const { return siz.height; }
	/// @brief 更新したデータか
	virtual bool IsUpdatedData() const { return false; }
};

/// @brief 描画用バッファ
///
/// 印字ヘッドのある行をエミュレート。
class DpBufferDw : public DpBuffer
{
private:
	MySize   rect;	///< 描画した領域
	MySize   line;	///< 1行の幅

public:
	DpBufferDw();
	DpBufferDw(int width_, int height_);
	virtual ~DpBufferDw();

	/// @brief バッファを作成
	void Create(int width_, int height_);

	/// @brief 描画した領域を返す
	MySize &Rect() { return rect; }
	/// @brief 1行の幅を返す
	MySize &Line() { return line; }
	/// @brief バッファのクリア
	void Clear();
	/// @brief 描画したサイズの初期化
	void ClearRect(int line_width);
	/// @brief 改行
	void NewLine();
	/// @brief バッファを左にシフト
	void LeftShift();
	/// @brief バッファの左側をクリア
	void ClearLeft();
	/// @brief バッファの右端に達しているか
	bool IsRightSide() const;
	/// @brief データ更新されているか
	bool IsUpdatedData() const;
};

/// @brief 合成用バッファ
///
/// 描画用バッファからこのバッファにコピーする。
/// 重ね合わせて印字する状況をエミュレート。
/// ここからDCに拡大縮小を考慮して印字する。
class DpBufferMx : public DpBuffer
{
private:
	MyRect   maxr;	///< バッファの印刷対象の範囲
	MyHoriz *maxh;	///< バッファに書かれたデータの1ラインごとの端
	MyHoriz dummymaxh;

public:
	DpBufferMx();
	DpBufferMx(int width_, int height_);
	virtual ~DpBufferMx();

	/// @brief バッファを作成
	void Create(int width_, int height_);
	/// @brief バッファを削除
	void Delete();

	/// @brief ピクセルデータの内容を上へシフト
	void UpperShiftTo(int width, int height);
	/// @brief ピクセルデータの内容を上へシフト
	void UpperShift(int height);
	/// @brief 合成エリアををクリア
	void Clear();

	/// @brief バッファの印刷対象の範囲を返す
	MyRect  &MaxR() { return maxr; }
	/// @brief バッファに書かれたデータの1ラインごとの端を返す
	MyHoriz &MaxH(int height);

	/// @brief 一番上端と下端を決定
	void SetMaxTopBottom(int top, int bottom);
	/// @brief 一番左端と右端を決定
	void SetMaxLeftRight(int left, int right);
	/// @brief データがあるか
	bool IsUpdatedData() const;
};

#endif /* DPBUFFER_H */
