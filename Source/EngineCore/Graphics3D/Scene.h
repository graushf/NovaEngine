#pragma once

// ================================================================================
// Scene.h : Implements the container class for 3D Graphics scenes.
//
// ================================================================================

#include "Geometry.h"
#include "SceneNodes.h"

class Scene
{
protected:
	std::shared_ptr<CameraNode> m_Camera;
	std::shared_ptr<IRenderer> m_Renderer;

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

	std::shared_ptr<IRenderer> GetRenderer() { return m_Renderer; }
};
