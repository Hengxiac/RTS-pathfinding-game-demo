#ifndef _SHELL_H_
#define _SHELL_H_

#include "PrimeEngine/Events/Component.h"
#include "PrimeEngine/Math/Matrix4x4.h"

namespace PE {
    namespace Events{
        //struct EventQueueManager;
    }
}

namespace CharacterControl {
namespace Components {

    struct ClientShell : public PE::Components::Component, public PE::Networkable
    {

		enum BitNum
		{
			TOPPOSITION = 3,
			POSITION = 2,
			HITPOINT = 0,
			SHELLLEFT = 1
		};

        // component API
        PE_DECLARE_CLASS(ClientShell);
		PE_DECLARE_NETWORKABLE_CLASS

			ClientShell(PE::GameContext &context, PE::MemoryArena arena,
			PE::Handle myHandle, Vector3 speed, bool isActive,
			int damage, float networkPingInterval); // constructor
        
        virtual void addDefaultComponents(); // adds default children and event handlers
        
        
        PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_UPDATE);
        virtual void do_UPDATE(PE::Events::Event *pEvt);
		
		PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_Update_Ghost);
		virtual void do_Update_Ghost(PE::Events::Event *pEvt);
        
		virtual int packStateData(char *pDataStream);
		//virtual int constructFromStream(char *pDataStream);

		void overrideTransform(Matrix4x4 &t);

		void moveShell(float time);

		float m_time;
		float m_networkPingTimer;
		float m_networkPingInterval;
		bool m_active;
		Vector3 m_speed;
		int m_damage;
		int m_ghostID;
		bool ghostCreated;
		bool toRemoveMask;
		bool toRemovePhys;

    };
}; // namespace Components
}; // namespace CharacterControl

#endif
