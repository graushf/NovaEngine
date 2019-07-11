#include "../Common/CommonStd.h"
#include "SceneNodes.h"

Mat4x4 CameraNode::GetWorldViewProjection(Scene* pScene)
{
	Mat4x4 world = pScene->GetTopMatrix();
	Mat4x4 view = VGet()->FromWorld();
	Mat4x4 worldView = world * view;
	
	return worldView * m_Projection;
}