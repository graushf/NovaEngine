#pragma once

// ================================================================
// Raycast.h : Implements a raycast into the rendererd scene
// ================================================================

#include "Geometry.h"

class Intersection
{
public:
	FLOAT m_fDist;							// distance from ray origin to intersection
	DWORD m_dwFace;							// the face index of the intersection
	FLOAT m_fBary1, m_fBary2;				// Barycentric coordinates of the intersection
	FLOAT m_tu, m_tv;						// texture coords of intersection
	ActorId m_actorId;						// Which actor was intersected if there was one
	Vec3 m_worldLoc;						// world location of intersection
	Vec3 m_actorLoc;						// actor local coordinates of intersection
	Vec3 m_normal;							// normal of intersection

	bool const operator <(Intersection const& other) { return m_fDist < other.m_fDist; }
};

template <class T>
void InitIntersection(Intersection& intersection, DWORD faceIndex, FLOAT dist, FLOAT y, FLOAT v, ActorId actorId, WORD* pIndices, T* pVertices, const Mat4x4& matWorld);


typedef std::vector<Intersection> IntersectionArray;

class CDXUTSDKMesh;

class RayCast
{
protected:
	LPDIRECT3DVERTEXBUFFER9		m_pVB;

public:
	RayCast(Point point, DWORD maxIntersections = 16);

	DWORD m_MaxIntersections;
	DWORD m_NumIntersections;
	bool m_bUseD3DXIntersect;			// Whether to use D3DXIntersect
	bool m_bAllHits;					// Whether to just get the first "hit" or all "hits"
	Point m_Point;

	D3DXVECTOR3 m_vPickRayDir;
	D3DXVECTOR3 m_vPickRayOrig;

	IntersectionArray m_IntersectionArray;

	HRESULT Pick(Scene* pScene, ActorId actorId, ID3DXMesh* pMesh);
	HRESULT Pick(Scene* pScene, ActorId actorId, CDXUTSDKMesh* pMesh);

	HRESULT Pick(Scene* pScene, ActorId actorId, LPDIRECT3DVERTEXBUFFER9 pVerts, LPDIRECT3DINDEXBUFFER9 pIndices, DWORD numPolys);
	HRESULT Pick(Scene* pScene, ActorId actorId, LPDIRECT3DVERTEXBUFFER9 pVerts, DWORD numPolys);

	void Sort();
};