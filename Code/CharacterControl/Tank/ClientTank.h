#ifndef _TANK_H_
#define _TANK_H_

#include "PrimeEngine/Events/Component.h"
#include "PrimeEngine/Math/Vector3.h"

namespace PE {
    namespace Events{
        struct EventQueueManager;
    }
}

namespace CharacterControl {
namespace Components {

	struct TankGameControls : public PE::Components::Component
	{
		PE_DECLARE_CLASS(TankGameControls);

	public:

		TankGameControls(PE::GameContext &context, PE::MemoryArena arena, PE::Handle hMyself)
			: PE::Components::Component(context, arena, hMyself)
		{
		}

		virtual ~TankGameControls(){}
		// Component ------------------------------------------------------------

		PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_UPDATE);
		virtual void do_UPDATE(PE::Events::Event *pEvt);

		virtual void addDefaultComponents() ;

		//Methods----------------
		void handleIOSDebugInputEvents(PE::Events::Event *pEvt);
		void handleKeyboardDebugInputEvents(PE::Events::Event *pEvt);
		void handleControllerDebugInputEvents(PE::Events::Event *pEvt);

		PE::Events::EventQueueManager *m_pQueueManager;

		PrimitiveTypes::Float32 m_frameTime;
	};

    struct TankController : public PE::Components::Component, public PE::Networkable
    {

		enum BitNum
		{
			TOPPOSITION = 3,
			POSITION = 2,
			HITPOINT = 0,
			SHELLLEFT = 1
		};

        // component API
        PE_DECLARE_CLASS(TankController);
		PE_DECLARE_NETWORKABLE_CLASS

        TankController(PE::GameContext &context, PE::MemoryArena arena,
			PE::Handle myHandle, float speed, Vector3 spawnPos, float networkPingInterval, 
			int hitPoint, int shellVolume, float timeCoe, Matrix4x4 &topTransform); // constructor
        
        virtual void addDefaultComponents(); // adds default children and event handlers
        
        
        PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_UPDATE);
        virtual void do_UPDATE(PE::Events::Event *pEvt);

		PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_Update_Ghost);
		virtual void do_Update_Ghost(PE::Events::Event *pEvt);
        
		PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_Tank_Throttle);
		virtual void do_Tank_Throttle(PE::Events::Event *pEvt);

		PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_Tank_Turn);
		virtual void do_Tank_Turn(PE::Events::Event *pEvt);

		PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_Tank_Aim);
		virtual void do_Tank_Aim(PE::Events::Event *pEvt);

		PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_PRE_RENDER_needsRC)
		void do_PRE_RENDER_needsRC(PE::Events::Event *pEvt);

		PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_Tank_Fire)
			void do_Tank_Fire(PE::Events::Event *pEvt);

		PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_Get_Hit)
			void do_Get_Hit(PE::Events::Event *pEvt);

		PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_Tank_Reload)
			void do_Tank_Reload(PE::Events::Event *pEvt);

		virtual int packStateData(char *pDataStream);
		//virtual int constructFromStream(char *pDataStream);

		void overrideTransform(Matrix4x4 &t);
		void activate();

		void updateReloading();

		float getSpeedCoe();

		const float RELOAD_TIME_SINGLE = 1.0f;
		const float FIRE_TIME = 2.0f;

        float m_timeSpeed;
        float m_time;
		float m_networkPingTimer;
		float m_networkPingInterval;
        Vector2 m_center;
        PrimitiveTypes::UInt32 m_counter;
		Vector3 m_spawnPos;
		bool m_active;
		bool m_overriden;
		Matrix4x4 m_transformOverride;
		Matrix4x4 m_topTransform;
		int m_HPMax;
		int m_hitPoint;
		int m_shellMax;
		int m_shellLeft;
		float m_fireInterval;
		float m_reloadInterval;
		float m_timeCoefficient;
    };
}; // namespace Components
}; // namespace CharacterControl

#endif
