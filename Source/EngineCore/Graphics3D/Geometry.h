#pragma once

//========================================================================
// Geometry.h
//========================================================================

#include "../Common/CommonStd.h"


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
	inline float Length() { return D3DXVec3Length(this); }
	inline Vec3 *Normalize() { return static_cast<Vec3*>(D3DXVec3Normalize(this, this)); }
	inline float Dot(const Vec3& b) { return D3DXVec3Dot(this, &b); }
	inline Vec3 Cross(const Vec3& b) const;

	Vec3(D3DXVECTOR3 &v3) { x = v3.x; y = v3.y; z = v3.z; }
	Vec3() : D3DXVECTOR3() { x = 0; y = 0; z = 0; }
	Vec3(const float _x, const float _y, const float _z) { x = _x; y = _y; z = _z; }
	Vec3(const double _x, const double _y, const double _z) { x = (float)_x; y = (float)_y; z = (float)_z; }
	inline Vec3(const class Vec4 &v4);
};

inline Vec3 Vec3::Cross(const Vec3& b) const
{
	Vec3 out;
	D3DXVec3Cross(&out, this, &b);
	return out;
}

inline Vec3 operator + (const Vec3& a, const Vec3& b)
{
	Vec3 out;
	D3DXVec3Add(&out, &a, &b);

	return out;
}

inline Vec3 operator - (const Vec3& a, const Vec3& b)
{
	Vec3 out;
	D3DXVec3Subtract(&out, &a, &b);

	return out;
}

// [graushf]
inline Vec3 operator * (const float& a, const Vec3& b)
{
	Vec3 out;
	D3DXVec3Scale(&out, &b, FLOAT(a));

	return out;
}

//
//
//

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

inline Vec4 operator + (const Vec4& a, const Vec4& b)
{
	Vec4 out;
	D3DXVec4Add(&out, &a, &b);

	return out;
}

extern Vec3 g_Up;
extern Vec3 g_Right;
extern Vec3 g_Forward;

extern Vec4 g_Up4;
extern Vec4 g_Right4;
extern Vec4 g_Forward4;

// ----------------------------------------------------
//
// Vec3List Description
// Vec4List Description
//
// An STL list of Vectors.
//
// ----------------------------------------------------

typedef std::list<Vec3> Vec3List;
typedef std::list<Vec4> Vec4List;

// ----------------------------------------------------
//
// Quaternion Description
//
// ----------------------------------------------------

class Quaternion : public D3DXQUATERNION
{
public:

	// Modifiers
	void Normalize() { D3DXQuaternionNormalize(this, this); };
	void Slerp(const Quaternion& begin, const Quaternion& end, float coeff)
	{
		// performs spherical linear interpolation between begin & end
		// NOTE: set coeff between 0.0f and -1.0f
		D3DXQuaternionSlerp(this, &begin, &end, coeff);
	}

	// Accessors
	void GetAxisAngle(Vec3& axis, float& angle) const
	{
		D3DXQuaternionToAxisAngle(this, &axis, &angle);
	}

	// Initializers
	void Build(const class Mat4x4& mat);

	void BuildRotYawPitchRoll(
		const float yawRadians,
		const float pitchRadians,
		const float rollRadians)
	{
		D3DXQuaternionRotationYawPitchRoll(this, yawRadians, pitchRadians, rollRadians);
	}

	void BuildAxisAngle(const Vec3& axis, const float radians)
	{
		D3DXQuaternionRotationAxis(this, &axis, radians);
	}

	Quaternion(D3DXQUATERNION& q) : D3DXQUATERNION(q) { }

	// [graushf]
	Quaternion(D3DXQUATERNION q) : D3DXQUATERNION(q) { }

	Quaternion() { }

	static const Quaternion g_Identity;
};

inline Quaternion operator * (const Quaternion& a, const Quaternion& b)
{
	// for rotations, this is exactly like concatenating
	// matrices - the new quat represents rot A followed by rot B.
	Quaternion out;
	D3DXQuaternionMultiply(&out, &a, &b);
	return out;
}

// -------------------------------------------------------
//
// Mat4x4 Description
//
// -------------------------------------------------------

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

	// [graushf]
	Mat4x4(D3DXMATRIX mat) { memcpy(&m, mat.m, sizeof(mat.m)); }

	Mat4x4() : D3DXMATRIX() { }

	static const Mat4x4 g_Identity;

	// Initialization methods
	inline void BuildTranslation(const Vec3& pos);
	inline void BuildTranslation(const float x, const float y, const float z);
	inline void BuildRotationX(const float radians) { D3DXMatrixRotationX(this, radians); }
	inline void BuildRotationY(const float radians) { D3DXMatrixRotationY(this, radians); }
	inline void BuildRotationZ(const float radians) { D3DXMatrixRotationZ(this, radians); }
	inline void BuildYawPitchRoll(const float yawRadians, const float pitchRadians, const float rollRadians);
	inline void BuildYawPitchRoll(const float yawRadians, const float pitchRadians, const float rollRadians)
	{
		D3DXMatrixRotationYawPitchRoll(this, yawRadians, pitchRadians, rollRadians);
	}
	inline void BuildRotationQuat(const Quaternion& q) 
	{
		D3DXMatrixRotationQuaternion(this, &q);
	}
	inline void BuildRotationLookAt(const Vec3& eye, const Vec3& at, const Vec3& up) {
		D3DXMatrixLookAtRH(this, &eye, &at, &up);
	}
	inline void BuildScale(const float x, const float y, const float z);

};

inline void Mat4x4::SetPosition(Vec3 const& pos)
{
	m[3][0] = pos.x;
	m[3][1] = pos.y;
	m[3][2] = pos.z;
	m[3][3] = 1.0f;
}

inline void Mat4x4::SetPosition(Vec4 const& pos)
{
	m[3][0] = pos.x;
	m[3][1] = pos.y;
	m[3][2] = pos.z;
	m[3][3] = pos.w;
}

inline void Mat4x4::SetScale(Vec3 const& scale)
{
	m[1][1] = scale.x;
	m[2][2] = scale.y;
	m[3][3] = scale.z;
}

inline Vec3 Mat4x4::GetPosition() const
{
	return Vec3(m[3][0], m[3][1], m[3][2]);
}

inline Vec3 Mat4x4::GetDirection() const
{
	// Note - the following code can be used to double check the vector construction above.
	Mat4x4 justRot = *this;
	justRot.SetPosition(Vec3(0.f, 0.f, 0.f));
	Vec3 forward = justRot.Xform(g_Forward);
	return forward;
}

inline Vec3 Mat4x4::GetRight() const
{
	// Note - the following code can be used to double check the vector construction above.
	Mat4x4 justRot = *this;
	justRot.SetPosition(Vec3(0.f, 0.f, 0.f));
	Vec3 right = justRot.Xform(g_Right);
	return right;
}

inline Vec3 Mat4x4::GetUp() const
{
	// Note - the following code can be used to double check the vector construction above.
	Mat4x4 justRot = *this;
	justRot.SetPosition(Vec3(0.f, 0.f, 0.f));
	Vec3 up = justRot.Xform(g_Up);
	return up;
}

inline Vec3 Mat4x4::GetYawPitchRoll() const
{
	float yaw, pitch, roll;

	pitch = asin(-_32);

	double threshold = 0.001; // Hardcoded constant - watchout
	double test = cos(pitch);

	if (test > threshold)
	{
		roll = atan2(_12, _22);
		yaw = atan2(_31, _33);
	}
	else
	{
		roll = atan2(-_21, _11);
		yaw = 0.0f;
	}

	return (Vec3(yaw, pitch, roll));
}

inline Vec3 Mat4x4::GetScale() const
{
	return Vec3(m[0][0], m[1][1], m[2][2]);
}

inline Vec4 Mat4x4::Xform(Vec4& v) const
{
	Vec4 temp;
	D3DXVec4Transform(&temp, &v, this);
	return temp;
}

inline Vec3 Mat4x4::Xform(Vec3& v) const
{
	Vec4 temp(v);
	Vec4 out;
	D3DXVec4Transform(&out, &temp, this);
	return Vec3(out.x, out.y, out.z);
}

inline Mat4x4 Mat4x4::Inverse() const
{
	Mat4x4 out;
	D3DXMatrixInverse(&out, NULL, this);
	return out;
}

inline void Mat4x4::BuildTranslation(const Vec3& pos)
{
	*this = Mat4x4::g_Identity;
	m[3][0] = pos.x;
	m[3][1] = pos.y;
	m[3][2] = pos.z;
}

inline void Mat4x4::BuildTranslation(const float x, const float y, const float z)
{
	*this = Mat4x4::g_Identity;
	m[3][0] = x;
	m[3][1] = y;
	m[3][2] = z;
}

inline void Mat4x4::BuildScale(const float x, const float y, const float z)
{
	*this = Mat4x4::g_Identity;
	m[1][1] = x;
	m[2][2] = y;
	m[3][3] = z;
}

inline Mat4x4 operator* (const Mat4x4& a, const Mat4x4& b)
{
	Mat4x4 out;
	D3DXMatrixMultiply(&out, &a, &b);

	return out;
}

inline void Quaternion::Build(const Mat4x4& mat)
{
	D3DXQuaternionRotationMatrix(this, &mat);
}

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

	Plane m_Planes[NumPlanes];		// planes of the frustum in camera space
	Vec3 m_NearClip[4];				// verts of the near clip plane in camera space.
	Vec3 m_FarClip[4];				// verts of the far clip plane in camera space.

	float m_Fov;					// field of view in radians
	float m_Aspect;					// aspect ratio - width divided by height
	float m_Near;					// near clipping distance.
	float m_Far;					// far clipping distance.

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