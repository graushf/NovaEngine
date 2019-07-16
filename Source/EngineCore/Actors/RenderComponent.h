#pragma once

// ---------------------------------------------------------------------------
// RenderComponent.h - classes that define renderable components of actors like
//						Meshes, Skyboxes, Lights, etc.
// ---------------------------------------------------------------------------


#include "RenderComponentInterface.h"
#include "../Graphics3D/SceneNodes.h"
#include "../Graphics3D/Lights.h"

// -----------------------------------------------------------------------------
// RenderComponent base class. This class does most of the work except actually 
// creating the scene, which is delegated to the subclass through a factory 
// method
// -----------------------------------------------------------------------------
class BaseRenderComponent : public RenderComponentInterface
{
protected:
	Color m_color;
	std::shared_ptr<SceneNode> m_pSceneNode;

public:
	virtual bool VInit(TiXmlElement* pData) override;
	virtual void VPostInit(void) override;
	virtual void VOnChanged(void) override;
	virtual TiXmlElement* VGenerateXml(void) override;
	const Color GetColor() const { return m_color; }

protected:
	// loads the SceneNode specific data (represented in the <SceneNode> tag)
	virtual bool VDelegateInit(TiXmlElement* pData) { return true; }
	virtual std::shared_ptr<SceneNode> VCreateSceneNode(void) = 0; // factory method to create the appropriate scene node.
	Color LoadColor(TiXmlElement* pData);

	// editor stuff
	virtual TiXmlElement* VCreateBaseElement(void) { return Nv_NEW TiXmlElement(VGetName()); }
	virtual void VCreateInheritedXmlElements(TiXmlElement* pBaseElement) = 0;

private:
	virtual std::shared_ptr<SceneNode> VGetSceneNode(void) override;
};

// ======================================================================================
// Grids, which represent the world
// ======================================================================================
class GridRenderComponent : public BaseRenderComponent
{
	std::string m_textureResource;
	int m_squares;

public:
	static const char* g_Name;
	virtual const char* VGetName() const { return g_Name; }

	GridRenderComponent(void);
	const char* GetTextureResource() { return m_textureResource.c_str(); }
	const int GetDivision() { return m_squares; }

protected:
	virtual bool VDelegateInit(TiXmlElement* pData) override;
	virtual std::shared_ptr<SceneNode> VCreateSceneNode(void) override; // factory method to create the appropriate scene node

	// editor stuff
	virtual void VCreateInheritedXmlElements(TiXmlElement* pBaseElement);
};



// ======================================================================================
// Lights
// ======================================================================================
class LightRenderComponent : public BaseRenderComponent
{
	LightProperties m_Props;

public:
	static const char* g_Name;
	virtual const char* VGetName() const { return g_Name; }

	LightRenderComponent(void);

protected:
	virtual bool VDelegateInit(TiXmlElement* pData) override;
	virtual std::shared_ptr<SceneNode> VCreateSceneNode(void) override;	// factory method to create the appropriate scene node

	// editor stuff
	virtual void VCreateInheritedXmlElements(TiXmlElement* pBaseElement);
};
