#pragma once

// ================================================================================
// UserInterface.h : Defines UI elements of the App
//
// ================================================================================

#include "Common/CommonStd.h"
#include "Graphics3D/Scene.h"

//
// class BaseUI					- Chapter 10, page 286
//
// This was factored to create a common class that
// implements some of the IScreenElement class common
// to modal/modeless dialogs.
//
class BaseUI : public IScreenElement
{
protected:
	int			m_PosX, m_PosY;
	int			m_Width, m_Height;
	int			m_Result;
	bool		m_bIsVisible;

public:
	BaseUI() { m_bIsVisible = true; m_PosX = m_PosY = 0; m_Width = 100; m_Height = 100; }
	virtual void VOnUpdate(int) { };
	virtual HRESULT VOnLostDevice() { return S_OK; }
	virtual bool VIsVisible() const { return m_bIsVisible; }
	virtual void VSetVisible(bool visible) { m_bIsVisible = visible; }
};

class ScreenElementScene : public IScreenElement, public Scene
{
public:
	ScreenElementScene(std::shared_ptr<IRenderer> renderer) : Scene(renderer) { }
	virtual ~ScreenElementScene(void)
	{
		//Nv_WARNING("~ScreenElementScene()");
	}

	// IScreenElement Implementation
	virtual void VOnUpdate(int deltaMS) { OnUpdate(deltaMS); };
	virtual HRESULT VOnRestore()
	{
		OnRestore(); return S_OK;
	}
	virtual HRESULT VOnRender(double fTime, float fElapsedTime)
	{
		OnRender(); return S_OK;
	}
	virtual HRESULT VOnLostDevice()
	{
		OnLostDevice(); return S_OK;
	}
	virtual int VGetZOrder() const { return 0; }
	virtual void VSetZOrder(int const zOrder) { }

	// Don't handle any messages
	virtual LRESULT CALLBACK VOnMsgProc(AppMsg msg) { return 0; }

	virtual bool VIsVisible() const { return true; }
	virtual void VSetVisible(bool visible) { }
	virtual bool VAddChild(ActorId id, std::shared_ptr<ISceneNode> kid) { return Scene::AddChild(id, kid); }
};