/// @file coordinate.cpp
///
/// @brief 座標
///
/// @date 2019.11.20 Create
///
/// @author Copyright (c) 2019, Sasaji. All rights reserved.
///
#ifndef COORDINATE_H
#define COORDINATE_H

#include "common.h"

/// @brief 矩形座標を保存するクラス
class MyRect
{
public:
	int left;
	int right;
	int top;
	int bottom;
public:
	MyRect();
	MyRect(int left_, int top_, int right_, int bottom_);
	virtual ~MyRect() {}
	void Clear();
	void Set(int left_, int top_, int right_, int bottom_);

	void AddLeft(int val);
	void AddRight(int val);
};

/// @brief 矩形座標を保存するクラス
class MyRectWH
{
public:
	int left;
	int top;
	int width;
	int height;
public:
	MyRectWH();
	MyRectWH(int left_, int top_, int witdh_, int height_);
	virtual ~MyRectWH() {}
	void Clear();
	void Set(int left_, int top_, int witdh_, int height_);

	void AddWidth(int val);
	void SubWidth(int val);

	int Right() const { return left + width; }
	int Bottom() const { return top + height; }
};

/// @brief 左右座標を保存するクラス
class MyHoriz
{
public:
	int left;
	int right;
public:
	MyHoriz();
	MyHoriz(int left_, int right_);
	virtual ~MyHoriz() {}
	void Clear();
	void Set(int left_, int right_);
};

/// @brief 上下座標を保存するクラス
class MyVert
{
public:
	int top;
	int bottom;
public:
	MyVert();
	MyVert(int top_, int bottom_);
	virtual ~MyVert() {}
	void Clear();
	void Set(int top_, int bottom_);
};

/// @brief 幅高さを保存するクラス
class MySize
{
public:
	int width;
	int height;
public:
	MySize();
	MySize(int width_, int height_);
	virtual ~MySize() {}
	void Clear();
	void Set(int width_, int height_);

	void AddWidth(int val);
	void SubWidth(int val);
};

/// @brief 位置を保存するクラス
class MyPoint
{
public:
	int x;
	int y;
public:
	MyPoint();
	MyPoint(int x_, int y_);
	virtual ~MyPoint() {}
	void Clear();
	void Set(int x_, int y_);

	void SetX(int val);
	void SetY(int val);
};

#endif /* COORDINATE_H */
