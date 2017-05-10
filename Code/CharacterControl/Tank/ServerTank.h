#ifndef S_TANK_H_
#define S_TANK_H_

#include "PrimeEngine/Events/Component.h"
#include "PrimeEngine/Math/Matrix4x4.h"

namespace PE {
    namespace Events{
        //struct EventQueueManager;
    }
}

namespace CharacterControl {
namespace Components {

    struct ServerTank : public PE::Components::Component, public PE::Networkable
    {

		enum BitNum
		{
			TOPPOSITION = 3,
			POSITION = 2,
			HITPOINT = 0,
			SHELLLEFT = 1
		};

        // component API
        PE_DECLARE_CLASS(ServerTank);
		PE_DECLARE_NETWORKABLE_CLASS

			ServerTank(PE::GameContext &context, PE::MemoryArena arena,
			PE::Handle myHandle, float speed, Matrix4x4 &spawnPos,
			float networkPingInterval, int hitPoint, int shellVolume); // constructor
        
        virtual void addDefaultComponents(); // adds default children and event handlers
        
        
        PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_UPDATE);
        virtual void do_UPDATE(PE::Events::Event *pEvt);
		
		PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_Update_Ghost);
		virtual void do_Update_Ghost(PE::Events::Event *pEvt);

		PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_Restart_Game);
		virtual void do_Restart_Game(PE::Events::Event *pEvt);
        
		virtual int packStateData(char *pDataStream);
		//virtual int constructFromStream(char *pDataStream);

		void overrideTransform(Matrix4x4 &t);

        float m_timeSpeed;
        float m_time;
		float m_networkPingTimer;
		float m_networkPingInterval;
        Vector2 m_center;
		Matrix4x4 m_spawnPos;
		Matrix4x4 m_topTransform;
        PrimitiveTypes::UInt32 m_counter;
		int m_hitPoint;
		int m_HPMax;
		int m_shellMax;
		int m_shellLeft;
		int m_clientProcessing;

    };
}; // namespace Components
}; // namespace CharacterControl

#endif
