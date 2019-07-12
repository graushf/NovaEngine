#pragma once

// ================================================================================
// Scene.h : Implements the container class for 3D Graphics scenes.
//
// ================================================================================

#include "Geometry.h"
#include "SceneNodes.h"

// Forward declarations

// -----------------------------------------------------------------------
//
// SceneActorMap Description					- Chapter 16, page Y
//
// An STL map that allows fast lookups of a scene node given an ActorId.
//
// -----------------------------------------------------------------------

typedef std::map<ActorId, std::shared_ptr<ISceneNode>> SceneActorMap;

// -----------------------------------------------------------------------
//
// Scene Description					- Chapter 16, page 536
//
// A heirarchical container of scene nodes, which are classes that 
// implement the ISceneNode interface.
//
// -----------------------------------------------------------------------

class LightManager;


class Scene
{
protected:
	std::shared_ptr<CameraNode> m_Camera;
	std::shared_ptr<IRenderer>	m_Renderer;

	ID3DXMatrixStack*			m_MatrixStack;

	LightManager				*m_LightManager;

public:
	Scene(std::shared_ptr<IRenderer> renderer);
	virtual ~Scene();

	HRESULT OnRender();
	HRESULT OnRestore();
	HRESULT OnLostDevice();
	HRESULT OnUpdate(const int deltaMilliseconds);

	bool AddChild(ActorId id, std::shared_ptr<ISceneNode> kid);


	void SetCamera(std::shared_ptr<CameraNode> camera) { m_Camera = camera; }
	const std::shared_ptr<CameraNode> GetCamera() const { return m_Camera; }

	const Mat4x4 GetTopMatrix()
	{
		// Scene::GetTopMatrix - Chapter 16, page 541
		return static_cast<const Mat4x4>(*m_MatrixStack->GetTop());
	}

	LightManager* GetLightManager() { return m_LightManager; }

	std::shared_ptr<IRenderer> GetRenderer() { return m_Renderer; }
};
