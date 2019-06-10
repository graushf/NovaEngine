
#include "Common/CommonStd.h"

#include "BaseAppLogic.h"


BaseAppLogic::BaseAppLogic()
{

}

BaseAppLogic::~BaseAppLogic()
{

}

bool BaseAppLogic::Init(void)
{
	return true;
}

void BaseAppLogic::VRenderDiagnostics()
{
	if (m_RenderDiagnostics)
	{
		m_pPhysics->VRenderDiagnostics();
	}
}
