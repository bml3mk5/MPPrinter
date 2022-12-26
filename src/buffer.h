/// @file buffer.cpp
///
/// @brief バッファ
///
/// @date 2020.06.30 Create
///
/// @author Copyright (c) 2020, Sasaji. All rights reserved.
///
#ifndef BUFFER_H
#define BUFFER_H

#include "common.h"
#include <wx/defs.h>

/// FIFOバッファ
class FIFOBuffer
{
private:
	wxByte  *m_data;
	size_t	 m_size;
	size_t   m_rpos;
	size_t	 m_wpos;
public:
	FIFOBuffer();
	FIFOBuffer(size_t val);
	virtual ~FIFOBuffer();
	/// @brief バッファサイズを設定
	void Allocate(size_t val);
	/// @brief バッファサイズが足りなければ再設定
	void Reallocate(size_t val);
	/// @brief バッファをクリア
	virtual void Clear();
	/// @brief 位置をクリア
	void ClearPos();
	/// @brief データを追加
	void AddData(wxByte val, size_t repeat = 1);
	/// @brief データを追加
	void AddData(const wxByte *buf, size_t size);
	/// @brief データを返す
	int PeekByte() const;
	/// @brief データを返す
	int GetByte();
	/// @brief リード開始位置からのデータを得る
	size_t ReadData(wxByte *buf, size_t size);
	/// @brief リード開始位置からのデータを得る
	wxByte *ReadData() const;
	/// @brief データバッファを返す
	wxByte *GetData() const { return m_data; }
	/// @brief バッファサイズを返す
	size_t GetSize() const { return m_size; }
	/// @brief リード位置を返す
	size_t GetReadPos() const { return m_rpos; }
	/// @brief ライト位置を返す
	size_t GetWritePos() const { return m_wpos; }
	/// @brief 読み残した残りを返す
	int Remain() const { return (int)(m_wpos - m_rpos); }
	/// @brief リード位置をセット
	void SetReadPos(size_t val) { m_rpos = val; }
	/// @brief ライト位置をセット
	void SetWritePos(size_t val) { m_wpos = val; }
	/// @brief リード位置を加算
	void AddReadPos(int val) { m_rpos = (size_t)((int)m_rpos + val); }
	/// @brief ライト位置を加算
	void AddWritePos(int val) { m_wpos = (size_t)((int)m_wpos + val); }
	/// @brief すべて読み込んだことにする
	void Fix() { m_rpos = m_wpos; }
	/// @brief データを反転する
	void Invert(size_t start);
};

#endif /* BUFFER_H */
