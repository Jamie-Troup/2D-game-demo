#pragma once

#include <iostream>
#include <math.h>

class Vec2
{
public:
	float x = 0; 
	float y = 0;
	
	Vec2();
	Vec2(float xin, float yin);
	
	bool operator == (const Vec2& rhs) const;
	bool operator != (const Vec2& rhs) const;
	
	Vec2 operator + (const Vec2& rhs) const;
	Vec2 operator - (const Vec2& rhs) const;
	Vec2 operator / (const float val) const;
	Vec2 operator * (const float val) const;
	
	void operator += (const Vec2& rhs);
	void operator -= (const Vec2& rhs);
	void operator *= (const float val);
	void operator /= (const float val);

	float length() const;
	float dist(const Vec2& rhs) const;
	
	void print() const;
};

std::ostream& operator << (std::ostream& os, const Vec2& v);
