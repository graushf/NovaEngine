#pragma once

// ================================================================================
// MovemementController.h
//
// ================================================================================

#include "Common/interfaces.h"
#include "Geometry.h"

// Forward declarations
class SceneNode;

// ----------------------------------------------------------
//
// MovementController Description
//	Implements a WASD style movement controller.
//
//		class MovememntController		- Chapter 10, page 281
//
// ----------------------------------------------------------

class MovementController : public IPointerHandler, public IKeyboardHandler
{
protected:
	Mat4x4 m_matFromWorld;
	Mat4x4 m_matToWorld;
	Mat4x4 m_matPosition;

	Point m_lastMousePos;
	bool m_bKey[256];

	// Orientation Controls
	float			m_fTargetYaw;
	float			m_fTargetPitch;
	float			m_fYaw;
	float			m_fPitch;
	float			m_fPitchOnDown;
	float			m_fYawOnDown;
	float			m_maxSpeed;
	float			m_currentSpeed;

	// Added for Ch19/20 refactor
	bool			m_mouseLButtonDown;
	bool			m_bRotateWhenLButtonDown;

	std::shared_ptr<SceneNode> m_object;

public:
	MovementController(std::shared_ptr<SceneNode> object, float initialYaw, float initialPitch, bool rotateWhenLButtonDown);
	void SetObject(std::shared_ptr<SceneNode> newObject);

	void OnUpdate(DWORD const elapsedMs);

public:
	bool VOnPointerMove(const Point& mousePos, const int radius);
	bool VOnPointerButtonDown(const Point& mousePos, const int radius, const std::string& buttonName);
	bool VOnPointerButtonUp(const Point& mousePos, const int radius, const std::string& buttonName);

	bool VOnKeyDown(const BYTE c) { m_bKey[c] = true; return true; }
	bool VOnKeyUp(const BYTE c) { m_bKey[c] = false; return true; }

	const Mat4x4* GetToWorld() { return& m_matToWorld; }
	const Mat4x4* GetFromWorld() { return& m_matFromWorld; }
};
