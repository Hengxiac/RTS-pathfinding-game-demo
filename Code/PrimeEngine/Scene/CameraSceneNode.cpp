#include "CameraSceneNode.h"
#include "../Lua/LuaEnvironment.h"
#include "PrimeEngine/Events/StandardEvents.h"

#define Z_ONLY_CAM_BIAS 0.0f
namespace PE {
namespace Components {

PE_IMPLEMENT_CLASS1(CameraSceneNode, SceneNode);

CameraSceneNode::CameraSceneNode(PE::GameContext &context, PE::MemoryArena arena, Handle hMyself) : SceneNode(context, arena, hMyself)
{
	m_near = 0.05f;
	m_far = 2000.0f;
}
void CameraSceneNode::addDefaultComponents()
{
	Component::addDefaultComponents();
	PE_REGISTER_EVENT_HANDLER(Events::Event_CALCULATE_TRANSFORMATIONS, CameraSceneNode::do_CALCULATE_TRANSFORMATIONS);
}

void CameraSceneNode::do_CALCULATE_TRANSFORMATIONS(Events::Event *pEvt)
{
	Handle hParentSN = getFirstParentByType<SceneNode>();
	if (hParentSN.isValid())
	{
		Matrix4x4 parentTransform = hParentSN.getObject<PE::Components::SceneNode>()->m_worldTransform;
		m_worldTransform = parentTransform * m_base;
	}
	
	Matrix4x4 &mref_worldTransform = m_worldTransform;

	Vector3 pos = Vector3(mref_worldTransform.m[0][3], mref_worldTransform.m[1][3], mref_worldTransform.m[2][3]);
	Vector3 n = Vector3(mref_worldTransform.m[0][2], mref_worldTransform.m[1][2], mref_worldTransform.m[2][2]);
	Vector3 target = pos + n;
	Vector3 up = Vector3(mref_worldTransform.m[0][1], mref_worldTransform.m[1][1], mref_worldTransform.m[2][1]);

	m_worldToViewTransform = CameraOps::CreateViewMatrix(pos, target, up);

	m_worldTransform2 = mref_worldTransform;

	m_worldTransform2.moveForward(Z_ONLY_CAM_BIAS);

	Vector3 pos2 = Vector3(m_worldTransform2.m[0][3], m_worldTransform2.m[1][3], m_worldTransform2.m[2][3]);
	Vector3 n2 = Vector3(m_worldTransform2.m[0][2], m_worldTransform2.m[1][2], m_worldTransform2.m[2][2]);
	Vector3 target2 = pos2 + n2;
	Vector3 up2 = Vector3(m_worldTransform2.m[0][1], m_worldTransform2.m[1][1], m_worldTransform2.m[2][1]);

	m_worldToViewTransform2 = CameraOps::CreateViewMatrix(pos2, target2, up2);
    
    PrimitiveTypes::Float32 aspect = (PrimitiveTypes::Float32)(m_pContext->getGPUScreen()->getWidth()) / (PrimitiveTypes::Float32)(m_pContext->getGPUScreen()->getHeight());
    
    PrimitiveTypes::Float32 verticalFov = 0.33f * PrimitiveTypes::Constants::c_Pi_F32;
    if (aspect < 1.0f)
    {
        //ios portrait view
        static PrimitiveTypes::Float32 factor = 0.5f;
        verticalFov *= factor;
    }

	m_viewToProjectedTransform = CameraOps::CreateProjectionMatrix(verticalFov, 
		aspect,
		m_near, m_far);
	

	float d_offset = 0;//5;

	Matrix4x4 m_worldToProjectedTransform = m_viewToProjectedTransform * m_worldToViewTransform;

	//left = col4 + col1
	m_frustumPlane[0].m_x = m_worldToProjectedTransform.m[3][0] + m_worldToProjectedTransform.m[0][0];
	m_frustumPlane[0].m_y = m_worldToProjectedTransform.m[3][1] + m_worldToProjectedTransform.m[0][1];
	m_frustumPlane[0].m_z = m_worldToProjectedTransform.m[3][2] + m_worldToProjectedTransform.m[0][2];
	m_frustumPlane[0].m_w = m_worldToProjectedTransform.m[3][3] + m_worldToProjectedTransform.m[0][3] - d_offset;
	//right = col4 - col1
	m_frustumPlane[1].m_x = m_worldToProjectedTransform.m[3][0] - m_worldToProjectedTransform.m[0][0];
	m_frustumPlane[1].m_y = m_worldToProjectedTransform.m[3][1] - m_worldToProjectedTransform.m[0][1];
	m_frustumPlane[1].m_z = m_worldToProjectedTransform.m[3][2] - m_worldToProjectedTransform.m[0][2];
	m_frustumPlane[1].m_w = m_worldToProjectedTransform.m[3][3] - m_worldToProjectedTransform.m[0][3] - d_offset;
	//bottom = col4 + col2
	m_frustumPlane[2].m_x = m_worldToProjectedTransform.m[3][0] + m_worldToProjectedTransform.m[1][0];
	m_frustumPlane[2].m_y = m_worldToProjectedTransform.m[3][1] + m_worldToProjectedTransform.m[1][1];
	m_frustumPlane[2].m_z = m_worldToProjectedTransform.m[3][2] + m_worldToProjectedTransform.m[1][2];
	m_frustumPlane[2].m_w = m_worldToProjectedTransform.m[3][3] + m_worldToProjectedTransform.m[1][3] - d_offset;
	//top = col4 - col2
	m_frustumPlane[3].m_x = m_worldToProjectedTransform.m[3][0] - m_worldToProjectedTransform.m[1][0];
	m_frustumPlane[3].m_y = m_worldToProjectedTransform.m[3][1] - m_worldToProjectedTransform.m[1][1];
	m_frustumPlane[3].m_z = m_worldToProjectedTransform.m[3][2] - m_worldToProjectedTransform.m[1][2];
	m_frustumPlane[3].m_w = m_worldToProjectedTransform.m[3][3] - m_worldToProjectedTransform.m[1][3] - d_offset;
	//near = col3
	m_frustumPlane[4].m_x = m_worldToProjectedTransform.m[2][0];
	m_frustumPlane[4].m_y = m_worldToProjectedTransform.m[2][1];
	m_frustumPlane[4].m_z = m_worldToProjectedTransform.m[2][2];
	m_frustumPlane[4].m_w = m_worldToProjectedTransform.m[2][3];
	//far = col4- col3
	m_frustumPlane[5].m_x = m_worldToProjectedTransform.m[3][0] - m_worldToProjectedTransform.m[2][0];
	m_frustumPlane[5].m_y = m_worldToProjectedTransform.m[3][1] - m_worldToProjectedTransform.m[2][1];
	m_frustumPlane[5].m_z = m_worldToProjectedTransform.m[3][2] - m_worldToProjectedTransform.m[2][2];
	m_frustumPlane[5].m_w = m_worldToProjectedTransform.m[3][3] - m_worldToProjectedTransform.m[2][3];
	SceneNode::do_CALCULATE_TRANSFORMATIONS(pEvt);

}
void CameraSceneNode::calculateScreenMatrix() {

	WinApplication *pWinApp = static_cast<WinApplication*>(m_pContext->getApplication());

	RECT screenDim;

	if (GetClientRect(pWinApp->getWindowHandle(), &screenDim))
	{
		m_projectedToScreenTransform = CameraOps::CreateScreenMatrix(screenDim.right, screenDim.bottom);
	}

}
}; // namespace Components
}; // namespace PE
