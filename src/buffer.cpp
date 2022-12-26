/// @file buffer.cpp
///
/// @brief 描画用バッファ
///
/// @date 2020.06.30 Create
///
/// @author Copyright (c) 2020, Sasaji. All rights reserved.
///

#include "buffer.h"
#include <wx/utils.h>

//

FIFOBuffer::FIFOBuffer()
{
	m_data = NULL;
	m_size = 0;
	m_rpos = 0;
	m_wpos = 0;
}

FIFOBuffer::FIFOBuffer(size_t val)
{
	m_data = new wxByte[val];
	m_size = val;
	m_rpos = 0;
	m_wpos = 0;
	memset(m_data, 0, val);
}

FIFOBuffer::~FIFOBuffer()
{
	delete m_data;
}

/// バッファサイズを設定
/// @param[in] val : サイズ
void FIFOBuffer::Allocate(size_t val)
{
	if (m_size < val) {
		wxByte *new_data = new wxByte[val];
		if (m_data) memcpy(new_data, m_data, m_size);
		memset(&new_data[m_size], 0, val - m_size);
		delete m_data;
		m_data = new_data;
		m_size = val;
	}
}

/// バッファサイズが足りなければ再設定
/// @param[in] val : サイズ
void FIFOBuffer::Reallocate(size_t val)
{
	if (m_size <= m_wpos + val) {
		size_t new_size = m_wpos + val;
		new_size = ((new_size + 1023) / 1024) * 1024;
		Allocate(new_size);
	}
}

/// バッファをクリア
void FIFOBuffer::Clear()
{
	m_rpos = 0;
	m_wpos = 0;
	memset(m_data, 0, m_size);
}

/// 位置をクリア
void FIFOBuffer::ClearPos()
{
	m_rpos = 0;
	m_wpos = 0;
}

/// データを追加
/// @param[in] val    データ1バイト
/// @param[in] repeat 繰り返し数
void FIFOBuffer::AddData(wxByte val, size_t repeat)
{
	Reallocate(repeat);
	memset(&m_data[m_wpos], val, repeat);
	m_wpos += repeat;
}

/// データを追加
/// @param[in] buf  データ
/// @param[in] size データサイズ
void FIFOBuffer::AddData(const wxByte *buf, size_t size)
{
	Reallocate(size);
	memcpy(&m_data[m_wpos], buf, size);
	m_wpos += size;
}

/// データを返す
/// @note 内部のリード位置は更新しない
/// @return データ ないとき-1
int FIFOBuffer::PeekByte() const
{
	if (m_rpos < m_wpos) {
		return m_data[m_rpos];
	} else {
		return -1;
	}
}

/// データを返す
/// @note 内部のリード位置を更新する
/// @return データ ないとき-1
int FIFOBuffer::GetByte()
{
	if (m_rpos < m_wpos) {
		return m_data[m_rpos++];
	} else {
		return -1;
	}
}

/// リード開始位置からのデータを得る
/// @note 内部のリード位置を更新する
/// @param[out] buf  データ格納先バッファ
/// @param[in]  size バッファサイズ
/// @return 格納データサイズ
size_t FIFOBuffer::ReadData(wxByte *buf, size_t size)
{
	if (m_rpos + size >= m_wpos) size = m_wpos - m_rpos;
	memcpy(buf, &m_data[m_rpos], size);
	m_rpos += size;
	return size;
}

/// リード開始位置からのデータを得る
wxByte *FIFOBuffer::ReadData() const
{
	return &m_data[m_rpos];
}

/// 指定位置からデータを反転する
void FIFOBuffer::Invert(size_t start)
{
	for(size_t i=start; i<m_wpos; i++) {
		m_data[i] = ~m_data[i];
	}
}
