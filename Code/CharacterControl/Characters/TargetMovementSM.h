#ifndef _PE_TARGET_MOVEMENT_SM_H_
#define _PE_TARGET_MOVEMENT_SM_H_


#include "PrimeEngine/Events/Component.h"
#include "PrimeEngine/Scene/SceneNode.h"

#include "../Events/Events.h"

namespace CharacterControl{
// events that can be sent to this state machine or sent by this state machine (like TARGET_REACHED)
namespace Events
{

// sent by behavior state machine when a soldier has to go somewhere
struct TargetMovementSM_Event_MOVE_TO : public PE::Events::Event {
	PE_DECLARE_CLASS(TargetMovementSM_Event_MOVE_TO);

	TargetMovementSM_Event_MOVE_TO(Vector3 targetPos = Vector3());

	Vector3 m_targetPosition;
	bool m_running;
};

struct TargetMovementSM_Event_STOP : public PE::Events::Event {
	PE_DECLARE_CLASS(TargetMovementSM_Event_STOP);

	TargetMovementSM_Event_STOP()
	{}
};

// sent by this state machine to its components. probably to behavior state machine
struct TargetMovementSM_Event_TARGET_REACHED : public PE::Events::Event {
	PE_DECLARE_CLASS(TargetMovementSM_Event_TARGET_REACHED);

	TargetMovementSM_Event_TARGET_REACHED()
	{}
};

struct TargetMovementSM_Event_UPDATE_POSITION : public PE::Events::Event {
	PE_DECLARE_CLASS(TargetMovementSM_Event_UPDATE_POSITION);

	TargetMovementSM_Event_UPDATE_POSITION(Vector3 pos)
	{
		m_curr_base = pos;
	}

	Vector3 m_curr_base;
};

};
namespace Components {

// movement state machine talks to associated animation state machine
struct TargetMovementSM : public PE::Components::Component
{
	PE_DECLARE_CLASS(TargetMovementSM);
	
	enum States
	{
		STANDING,
		RUNNING_TO_TARGET,
		WALKING_TO_TARGET,
	};


	TargetMovementSM(PE::GameContext &context, PE::MemoryArena arena, PE::Handle hMyself);

	//////////////////////////////////////////////////////////////////////////
	// utility
	//////////////////////////////////////////////////////////////////////////
	PE::Components::SceneNode *getParentsSceneNode();

	//////////////////////////////////////////////////////////////////////////
	// Component API and Event Handlers
	//////////////////////////////////////////////////////////////////////////
	//
	virtual void addDefaultComponents() ;
	//
	PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_TargetMovementSM_Event_MOVE_TO)
	virtual void do_TargetMovementSM_Event_MOVE_TO(PE::Events::Event *pEvt);
	//
	PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_TargetMovementSM_Event_STOP)
	virtual void do_TargetMovementSM_Event_STOP(PE::Events::Event *pEvt);
	//
	PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_UPDATE)
	virtual void do_UPDATE(PE::Events::Event *pEvt);

	//////////////////////////////////////////////////////////////////////////
	// Member Variables
	//////////////////////////////////////////////////////////////////////////
	PE::Handle m_hAnimationSM;
	//
	// State
	Vector3 m_targetPostion;
	States m_state;
};

};
};


#endif


