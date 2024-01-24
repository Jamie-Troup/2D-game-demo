#include "Vec2.h"

Vec2::Vec2()
{
	x = 0;
	y = 0;
}

Vec2::Vec2(float xin, float yin)
	: x(xin), y(yin) 
{}
	
Vec2 Vec2::operator + (const Vec2& v) const
{
	return Vec2(x+v.x, y+v.y);
}

Vec2 Vec2::operator - (const Vec2& v) const
{
	return Vec2(x-v.x, y-v.y);
}

Vec2 Vec2::operator / (const float val) const
{
	return Vec2(x/val, y/val);
}

Vec2 Vec2::operator * (const float val) const
{
	return Vec2(x*val, y*val);
}

bool Vec2::operator == (const Vec2& v) const
{
	return x == v.x && y == v.y;
}

bool Vec2::operator != (const Vec2& v) const
{
	return x != v.x || y != v.y;
}

void Vec2::operator += (const Vec2& v)
{
	x += v.x;
	y += v.y;
}

void Vec2::operator -= (const Vec2& v)
{
	x -= v.x;
	y -= v.y;
}

void Vec2::operator *= (const float val)
{
	x *= val;
	y *= val;
}

void Vec2::operator /= (const float val)
{
	x /= val;
	y /= val;
}

float Vec2::length() const
{
	return sqrtf((x*x) + (y*y));
}

float Vec2::dist(const Vec2& v) const
{
	return (v-*this).length();
}

void Vec2::print() const
{
	std::cout << x << " " << y << std::endl;
}

std::ostream& operator << (std::ostream& os, const Vec2& v)
{
	os << v.x << " " << v.y;
	return os;
}
