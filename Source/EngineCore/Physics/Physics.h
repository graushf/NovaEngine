#pragma once

// ---------------------------------------------------------------------------
// Physics.h - Implements the IGamePhysics interface
// ---------------------------------------------------------------------------

#include "../Common/CommonStd.h"

extern IGamePhysics* CreateGamePhysics();
extern IGamePhysics* CreateNullPhysics();