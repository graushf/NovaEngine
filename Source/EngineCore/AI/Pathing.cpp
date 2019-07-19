// ================================================================
// Pathing.cpp : Implements a simple pathing system using A*
// ================================================================

#include "../Common/CommonStd.h"
#include "Pathing.h"
#include "../App/App.h"

// ==============================================================
// PathingNode
// ==============================================================
Nv_MEMORYPOOL_DEFINITION(PathingNode);
Nv_MEMORYPOOL_AUTOINIT(PathingNode, 128); // there are currently 81 pathing nodes generated; if you make more, you should increase this number


void PathingNode::AddArc(PathingArc* pArc)
{
	//Nv_ASSERT(pArc);
	m_arcs.push_back(pArc);
}