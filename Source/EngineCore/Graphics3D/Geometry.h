#pragma once

#include "Common/CommonStd.h"


// ------------------------------------------------------------------
//	Content References in Game Coding Complete 3rd Edition
//
//  class Vec3				- Chapter 14, page 456
//  class Vec4				- Chapter 14, page 457
//  class Mat4x4			- Chapter 14, page 466
//  class Quaternion		- Chapter 14, page 471
//  class Plane				- Chapter 14, page 473
//  class Frustum			- Chapter 14, page 474
//

const float Nv_PI = 3.14159265358979f;
const float Nv_2PI = 2 * Nv_PI;

//
// Utility classes for vectors and matrices
//
typedef D3DXVECTOR2 Vec2;



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

//
// Plane Definition
//

class Plane : public D3DXPLANE
{
public:
	inline void Normalize();

	// normal faces away from you if you send in verts in counter clockwise order...
	inline void Init(const Vec3& p0, const Vec3& p1, const Vec3& p2);
	bool Inside(const Vec3& point, const float radius) const;
	bool Inside(const Vec3& point) const;
};

inline void Plane::Normalize()
{
	float mag;
	mag = sqrt(a * a + b * b + c * c);
	a = a / mag;
	b = b / mag;
	c = c / mag;
	d = d / mag;
}

inline void Plane::Init(const Vec3& p0, const Vec3& p1, const Vec3& p2)
{
	D3DXPlaneFromPoints(this, &p0, &p1, &p2);
	Normalize();
}

//
// Frustum definition
//
class Frustum
{
public:
	enum Side { Near, Far, Top, Right, Bottom, Left, NumPlanes };

	Plane m_Planes[NumPlanes];
	Vec3 m_NearClip[4];
	Vec3 m_FarClip[4];

	float m_Fov;
	float m_Aspect;
	float m_Near;
	float m_Far;

public:
	Frustum();

	bool Inside(const Vec3& point) const;
	bool Inside(const Vec3& point, const float radius) const;
	const Plane &Get(Side side) { return m_Planes[side]; }
	void SetFOV(float fov) { m_Fov = fov; Init(m_Fov, m_Aspect, m_Near, m_Far); }
	void SetAspect(float aspect) { m_Aspect = aspect; Init(m_Fov, m_Aspect, m_Near, m_Far); }
	void SetNear(float nearClip) { m_Near = nearClip; Init(m_Fov, m_Aspect, m_Near, m_Far); }
	void SetFar(float farClip) { m_Far = farClip; Init(m_Fov, m_Aspect, m_Near, m_Far); }
	void Init(const float fov, const float aspect, const float near, const float far);

	void Render();
};