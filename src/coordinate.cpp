/// @file coordinate.cpp
///
/// @brief 座標
///
/// @date 2019.11.20 Create
///
/// @author Copyright (c) 2019, Sasaji. All rights reserved.
///

#include "coordinate.h"

//

MyRect::MyRect()
{
	Clear();
}
MyRect::MyRect(int left_, int top_, int right_, int bottom_)
{
	Set(left_, top_, right_, bottom_);
}
void MyRect::Clear()
{
	left = 0;
	right = 0;
	top = 0;
	bottom = 0;
}
void MyRect::Set(int left_, int top_, int right_, int bottom_)
{
	left = left_;
	right = right_;
	top = top_;
	bottom = bottom_;
}
void MyRect::AddLeft(int val)
{
	left += val;
}
void MyRect::AddRight(int val)
{
	right += val;
}

//

MyRectWH::MyRectWH()
{
	Clear();
}
MyRectWH::MyRectWH(int left_, int top_, int width_, int height_)
{
	Set(left_, top_, width_, height_);
}
void MyRectWH::Clear()
{
	left = 0;
	width = 0;
	top = 0;
	height = 0;
}
void MyRectWH::Set(int left_, int top_, int width_, int height_)
{
	left = left_;
	width = width_;
	top = top_;
	height = height_;
}
void MyRectWH::AddWidth(int val)
{
	width += val;
}
void MyRectWH::SubWidth(int val)
{
	width -= val;
	if (width < 0) width = 0;
}

//

MyHoriz::MyHoriz()
{
	Clear();
}
MyHoriz::MyHoriz(int left_, int right_)
{
	Set(left_, right_);
}
void MyHoriz::Clear()
{
	left = 0;
	right = 0;
}
void MyHoriz::Set(int left_, int right_)
{
	left = left_;
	right = right_;
}

//

MyVert::MyVert()
{
	Clear();
}
MyVert::MyVert(int top_, int bottom_)
{
	Set(top_, bottom_);
}
void MyVert::Clear()
{
	top = 0;
	bottom = 0;
}
void MyVert::Set(int top_, int bottom_)
{
	top = top_;
	bottom = bottom_;
}

//

MySize::MySize()
{
	Clear();
}
MySize::MySize(int width_, int height_)
{
	Set(width_, height_);
}
void MySize::Clear()
{
	width = 0;
	height = 0;
}
void MySize::Set(int width_, int height_)
{
	width = width_;
	height = height_;
}
void MySize::AddWidth(int val)
{
	width += val;
}
void MySize::SubWidth(int val)
{
	width -= val;
	if (width < 0) width = 0;
}

//

MyPoint::MyPoint()
{
	Clear();
}
MyPoint::MyPoint(int x_, int y_)
{
	Set(x_, y_);
}
void MyPoint::Clear()
{
	x = 0;
	y = 0;
}
void MyPoint::Set(int x_, int y_)
{
	x = x_;
	y = y_;
}

void MyPoint::SetX(int val)
{
	x = val;
	if (x < 0) x = 0;
}

void MyPoint::SetY(int val)
{
	y = val;
	if (y < 0) y = 0;
}
