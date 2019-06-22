#pragma once

#include "Common/CommonStd.h"

class Vec3 : public D3DXVECTOR3
{
public:
	Vec3(D3DXVECTOR3 &v3) { x = v3.x; y = v3.y; z = v3.z; }
	Vec3() : D3DXVECTOR3() { x = 0; y = 0; z = 0; }
	Vec3(const float _x, const float _y, const float _z) { x = _x; y = _y; z = _z; }
	Vec3(const double _x, const double _y, const double _z) { x = (float)_x; y = (float)_y; z = (float)_z; }
	inline Vec3(const class Vec4 &v4);
};

class Vec4 : public D3DXVECTOR4
{
public:
	inline float Length() { return D3DXVec4Length(this); }
	inline Vec4* Normalize() { return static_cast<Vec4 *>(D3DXVec4Normalize(this, this)); }
	// If you want the cross product, Use Vec3::Cross
	inline float Dot(const Vec4 &b) { return D3DXVec4Dot(this, &b); }

	Vec4(D3DXVECTOR4 &v4) { x = v4.x, y = v4.y, z = v4.z; w = v4.w; }
	Vec4() : D3DXVECTOR4() { }
	Vec4(const float _x, const float _y, const float _z, const float _w) { x = _x; y = _y; z = _z; w = _w; }
	Vec4(const Vec3& v3) { x = v3.x; y = v3.y; z = v3.z; w = 1.0f; }
};

inline Vec3::Vec3(const Vec4& v4) { x = v4.x; y = v4.y; z = v4.z; }

extern Vec3 g_Up;
extern Vec3 g_Right;
extern Vec3 g_Forward;

extern Vec4 g_Up4;
extern Vec4 g_Right4;
extern Vec4 g_Forward4;



class Mat4x4 : public D3DXMATRIX
{
public:
	// Modifiers
	inline void SetPosition(Vec3 const &pos);
	inline void SetPosition(Vec4 const &pos);
	inline void SetScale(Vec3 const &scale);

	// Accessors and Calculation Methods
	inline Vec3 GetPosition() const;
	inline Vec3 GetDirection() const;
	inline Vec3 GetUp() const;
	inline Vec3 GetRight() const;
	inline Vec3 GetYawPitchRoll() const;
	inline Vec3 GetScale() const;
	inline Vec4 Xform(Vec4 &v) const;
	inline Vec3 Xform(Vec3 &v) const;
	inline Mat4x4 Inverse() const;

	Mat4x4(D3DXMATRIX &mat) { memcpy(&m, &mat.m, sizeof(mat.m)); }
	Mat4x4() : D3DXMATRIX() { }

	static const Mat4x4 g_Identity;
};