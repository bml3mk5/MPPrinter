/// @file dpbuffer.cpp
///
/// @brief 描画用バッファ
///
/// @date 2019.11.20 Create
///
/// @author Copyright (c) 2019, Sasaji. All rights reserved.
///

#include "dpbuffer.h"
#include <wx/utils.h>

//

DpBuffer::DpBuffer()
{
	buffer = NULL;
	dummy = 0;
}
/// @param[in] width_   幅(pixel)
/// @param[in] height_  高さ(pixel)
DpBuffer::DpBuffer(int width_, int height_)
{
	Create(width_, height_);
}
DpBuffer::~DpBuffer()
{
	Delete();
}

/// バッファを作成
/// @param[in] width_   幅(pixel)
/// @param[in] height_  高さ(pixel)
void DpBuffer::Create(int width_, int height_)
{
	siz.Set(width_, height_);
	buffer = new DpType[siz.width * siz.height];
	dummy = 0;
}
/// バッファを削除
void DpBuffer::Delete()
{
	delete [] buffer;
	buffer = NULL;
	siz.Clear();
}

/// データをセット
/// @param[in] x    X
/// @param[in] y    Y
/// @param[in] data データ
void DpBuffer::Set(int x, int y, DpType data)
{
	if (!buffer) return;
//	if (x < 0 || x >= siz.width || y < 0 || y >= siz.height) return;
	buffer[y * siz.width + x] = data;
}

/// データを返す
/// @param[in] x    X
/// @param[in] y    Y
/// @return データ
const DpType &DpBuffer::Get(int x, int y) const
{
	if (!buffer) return dummy;
//	if (x < 0 || x >= siz.width || y < 0 || y >= siz.height) return dummy;
	return buffer[y * siz.width + x];
}

/// バッファポインタを返す
/// @param[in] x    X
/// @param[in] y    Y
/// @return バッファポインタ
DpType *DpBuffer::Ptr(int x, int y)
{
	if (!buffer) return NULL;
//	if (x < 0 || x >= siz.width || y < 0 || y >= siz.height) return NULL;
	return &buffer[y * siz.width + x];
}

/// ピクセルデータの内容を左へシフト
/// @param[in] width シフトする幅
void DpBuffer::LeftShiftTo(int width)
{
	DpType *src = NULL;
	DpType *dst = NULL;

	if (width > Width()) width = Width();
	if (width < 0) width = 0;

	for(int h=0; h < Height(); h++) {
		src = Ptr(width, h);
		dst = Ptr(0, h);
		for(int w = 0; w < (Width() - width); w++) {
			*dst = *src;
			src++;
			dst++;
		}
		for(int w=0; w < width; w++) {
			*dst = DpTypeMin;
			dst++;
		}
	}
}

/// ピクセルデータの内容をクリア
/// @param[in] width クリアする幅
void DpBuffer::ClearLeftTo(int width)
{
	DpType *src = NULL;

	if (width > Width()) width = Width();

	for(int h=0; h < Height(); h++) {
		src = Ptr(0, h);
		for(int w = 0; w < width; w++) {
			*src = DpTypeMin;
			src++;
		}
	}
}

/// ピクセルデータの内容をコピー
/// @param[out] dst_buffer コピー先
/// @param[in]  dx         コピー先開始位置X 
/// @param[in]  dy         コピー先開始位置Y
/// @param[in]  width      幅
/// @param[in]  height     高さ
void DpBuffer::CopyTo(DpBuffer *dst_buffer, int dx, int dy, int width, int height)
{
	DpType *src = NULL;
	DpType *dst = NULL;
	DpType dot;

	if (width + dx > Width()) width = Width() - dx;
	if (height + dy > Height()) height = Height() - dy;

	for(int h=0; h < height; h++) {
		src = Ptr(0, h);
		dst = dst_buffer->Ptr(dx, dy + h);

		for(int w=0; w < width; w++) {
			dot = *dst + *src;
			*dst = (dot >= DpTypeMax ? DpTypeMax : dot);
			src++;
			dst++;
		}
	}
}

//

DpBufferDw::DpBufferDw()
	: DpBuffer()
{
}
DpBufferDw::DpBufferDw(int width_, int height_)
	: DpBuffer()
{
	Create(width_, height_);
}
DpBufferDw::~DpBufferDw()
{
}
/// バッファを作成
/// @param[in] width_   幅(pixel)
/// @param[in] height_  高さ(pixel)
void DpBufferDw::Create(int width_, int height_)
{
	DpBuffer::Create(width_, height_);
}
/// バッファのクリア
void DpBufferDw::Clear()
{
	ClearLeftTo(Width());
	rect.width = 0;
}
/// 描画したサイズの初期化
/// @param[in] line_width 1行の幅
void DpBufferDw::ClearRect(int line_width)
{
	rect.Set(0,		// 描画用バッファに書かれたデータのwidth
		Height());	// 描画用バッファに書かれたデータのheight

	line.Set(line_width,	// 1行の幅
		Height());			// 1行の高さ
}
/// 改行
void DpBufferDw::NewLine()
{
	rect.SubWidth(line.width);
}
/// バッファを左にシフト
void DpBufferDw::LeftShift()
{
	LeftShiftTo(line.width);
}
/// バッファの左側をクリア
void DpBufferDw::ClearLeft()
{
	ClearLeftTo(line.width);
}
/// バッファの右端に達しているか
bool DpBufferDw::IsRightSide() const
{
	return (rect.width >= line.width);
}
/// データ更新されているか
bool DpBufferDw::IsUpdatedData() const
{
	return (rect.width > 0 && rect.height > 0);
}

//

DpBufferMx::DpBufferMx()
	: DpBuffer()
{
	maxh = NULL;
}
DpBufferMx::DpBufferMx(int width_, int height_)
	: DpBuffer()
{
	Create(width_, height_);
}

DpBufferMx::~DpBufferMx()
{
	Delete();
}

/// バッファを作成
/// @param[in] width_   幅(pixel)
/// @param[in] height_  高さ(pixel)
void DpBufferMx::Create(int width_, int height_)
{
	DpBuffer::Create(width_, height_);
	maxh = new MyHoriz[height_];
}

/// バッファを削除
void DpBufferMx::Delete()
{
	delete [] maxh;
	maxh = NULL;
	DpBuffer::Delete();
}

/// ピクセルデータの内容を上へシフト
/// @param[in] width    幅
/// @param[in] height   高さ
void DpBufferMx::UpperShiftTo(int width, int height)
{
	DpType *src = NULL;
	DpType *dst = NULL;

	if (height > Height()) height = Height();
	if (height < 0) height = 0;

	for(int h=0; h < (Height() - height); h++) {
		src = Ptr(0, h + height);
		dst = Ptr(0, h);

		for(int w=0; w < width; w++) {
			*dst = *src;
			src++;
			dst++;
		}
		maxh[h] = maxh[h + height];
	}
	for(int h=(Height() - height); h < Height(); h++) {
		dst = Ptr(0, h);
		for(int w=0; w < width; w++) {
			*dst = DpTypeMin;
			dst++;
		}
		maxh[h].Set(width, 0);
	}
}

/// ピクセルデータの内容を上へシフト
/// @param[in] height   高さ
void DpBufferMx::UpperShift(int height)
{
	UpperShiftTo(Width(), height);
}

/// 合成エリアををクリア
void DpBufferMx::Clear()
{
	DpBuffer::ClearLeftTo(Width());

	for(int h=0; h < Height(); h++) {
		maxh[h].Set(Width(), 0);
	}
}

/// バッファに書かれたデータの1ラインごとの端を返す
/// @param[in] height   高さ
MyHoriz &DpBufferMx::MaxH(int height)
{
	if (height < Height()) {
		return maxh[height];
	} else {
		return dummymaxh;
	}
}

/// 一番上端と下端を決定
/// @param[in] top    上
/// @param[in] bottom 下
void DpBufferMx::SetMaxTopBottom(int top, int bottom)
{
	maxr.top = top;
	maxr.bottom = bottom;
}

/// 一番左端と右端を決定
/// @param[in] left  左
/// @param[in] right 右
void DpBufferMx::SetMaxLeftRight(int left, int right)
{
	maxr.left  = Width();
	maxr.right = 0;
	for(int i=0; i<maxr.bottom; i++) {
		if (left < maxh[i].left) {
			maxh[i].left = left;
		}
		if (maxh[i].left < maxr.left) {
			maxr.left = maxh[i].left;
		}
		if (maxh[i].right < right) {
			maxh[i].right = right;
		}
		if (maxr.right < maxh[i].right) {
			maxr.right = maxh[i].right;
		}
	}
}

/// データがあるか
bool DpBufferMx::IsUpdatedData() const
{
	return (maxr.right > maxr.left && maxr.bottom > maxr.top);
}
