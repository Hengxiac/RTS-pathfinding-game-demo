#ifndef _CHARACTER_CONTROL_TARGET_NPC_
#define _CHARACTER_CONTROL_TARGET_NPC_

#include "PrimeEngine/Events/Component.h"


#include "../Events/Events.h"

namespace CharacterControl {

	namespace Components {

		struct Target : public PE::Components::Component
		{
			PE_DECLARE_CLASS(Target);

			Target(PE::GameContext &context, PE::MemoryArena arena, PE::Handle hMyself, Events::Event_CreateTarget *pEvt);

			virtual void addDefaultComponents();

			Matrix4x4 m_base;
		};
	}; // namespace Components
}; // namespace CharacterControl
#endif
