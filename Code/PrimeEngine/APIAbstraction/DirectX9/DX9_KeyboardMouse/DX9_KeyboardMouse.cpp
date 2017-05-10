// API Abstraction
#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"
#if APIABSTRACTION_D3D9 | APIABSTRACTION_D3D11 | (APIABSTRACTION_OGL && !defined(SN_TARGET_PS3))

// Outer-Engine includes

// Inter-Engine includes
#include "PrimeEngine/Events/StandardKeyboardEvents.h"
#include "PrimeEngine/Render/D3D11Renderer.h"
#include "PrimeEngine/Render/D3D9Renderer.h"
#include "../../../Lua/LuaEnvironment.h"
// Sibling/Children includes
#include "DX9_KeyboardMouse.h"
#include "PrimeEngine/Application/WinApplication.h"

namespace PE {
using namespace Events;
namespace Components {

PE_IMPLEMENT_CLASS1(DX9_KeyboardMouse, Component);

void DX9_KeyboardMouse::addDefaultComponents()
{
	Component::addDefaultComponents();
	PE_REGISTER_EVENT_HANDLER(Events::Event_UPDATE, DX9_KeyboardMouse::do_UPDATE);
}

void DX9_KeyboardMouse::do_UPDATE(Events::Event *pEvt)
{
	
	m_pQueueManager = Events::EventQueueManager::Instance();

	generateButtonEvents();
}

void DX9_KeyboardMouse::generateButtonEvents()
{
#if PE_PLAT_IS_WIN32
	WinApplication *pWinApp = static_cast<WinApplication*>(m_pContext->getApplication());
	if(GetFocus() == pWinApp->getWindowHandle())
#endif
	{
		//Check for Button Down events

		//Check for Button Up events
		
		//Check for Button Held events
		if(GetAsyncKeyState('A') & 0x8000)
		{
			Handle h("EVENT", sizeof(Event_KEY_A_HELD));
			new (h) Event_KEY_A_HELD;
			m_pQueueManager->add(h, Events::QT_INPUT);
		}
		if(GetAsyncKeyState('S') & 0x8000)
		{
			Handle h("EVENT", sizeof(Event_KEY_S_HELD));
			new (h) Event_KEY_S_HELD;
			m_pQueueManager->add(h, Events::QT_INPUT);
		}
		if(GetAsyncKeyState('D') & 0x8000)
		{
			Handle h("EVENT", sizeof(Event_KEY_D_HELD));
			new (h) Event_KEY_D_HELD;
			m_pQueueManager->add(h, Events::QT_INPUT);
		}
		if(GetAsyncKeyState('W') & 0x8000)
		{
			Handle h("EVENT", sizeof(Event_KEY_W_HELD));
			new (h) Event_KEY_W_HELD;
			m_pQueueManager->add(h, Events::QT_INPUT);
		}
		if (GetAsyncKeyState('E') & 0x8000)
		{
			Handle h("EVENT", sizeof(Event_KEY_E_HELD));
			new (h) Event_KEY_E_HELD;
			m_pQueueManager->add(h, Events::QT_INPUT);
		}
		if (GetAsyncKeyState('R') & 0x8000)
		{
			Handle h("EVENT", sizeof(Event_KEY_R_HELD));
			new (h) Event_KEY_R_HELD;
			m_pQueueManager->add(h, Events::QT_INPUT);
		}
		if(GetAsyncKeyState(VK_LEFT) & 0x8000)
		{
			Handle h("EVENT", sizeof(Event_KEY_LEFT_HELD));
			new (h) Event_KEY_LEFT_HELD;
			m_pQueueManager->add(h, Events::QT_INPUT);
		}
		if(GetAsyncKeyState(VK_DOWN) & 0x8000)
		{
			Handle h("EVENT", sizeof(Event_KEY_DOWN_HELD));
			new (h) Event_KEY_DOWN_HELD;
			m_pQueueManager->add(h, Events::QT_INPUT);
		}
		if(GetAsyncKeyState(VK_RIGHT) & 0x8000)
		{
			Handle h("EVENT", sizeof(Event_KEY_RIGHT_HELD));
			new (h) Event_KEY_RIGHT_HELD;
			m_pQueueManager->add(h, Events::QT_INPUT);
		}
		if(GetAsyncKeyState(VK_UP) & 0x8000)
		{
			Handle h("EVENT", sizeof(Event_KEY_UP_HELD));
			new (h) Event_KEY_UP_HELD;
			m_pQueueManager->add(h, Events::QT_INPUT);
		}
		if(GetAsyncKeyState(',') & 0x8000)
		{
			Handle h("EVENT", sizeof(Event_KEY_COMMA_HELD));
			new (h) Event_KEY_COMMA_HELD;
			m_pQueueManager->add(h, Events::QT_INPUT);
		}
		if(GetAsyncKeyState('.') & 0x8000)
		{
			Handle h("EVENT", sizeof(Event_KEY_PERIOD_HELD));
			new (h) Event_KEY_PERIOD_HELD;
			m_pQueueManager->add(h, Events::QT_INPUT);
		}
		if(GetAsyncKeyState('K') & 0x8000)
		{
			Handle h("EVENT", sizeof(Event_KEY_K_HELD));
			new (h) Event_KEY_K_HELD;
			m_pQueueManager->add(h, Events::QT_INPUT);
		}
		if(GetAsyncKeyState('L') & 0x8000)
		{
			Handle h("EVENT", sizeof(Event_KEY_L_HELD));
			new (h) Event_KEY_L_HELD;
			m_pQueueManager->add(h, Events::QT_INPUT);
		}
		if (GetAsyncKeyState('T') & 0x8000)
		{
			if (pT->TickAndGetTimeDeltaInSeconds() > (GetDoubleClickTime() / 10000.0f))
			{
				Handle h("EVENT", sizeof(Event_KEY_T_HELD));
				new (h) Event_KEY_T_HELD;
				m_pQueueManager->add(h, Events::QT_INPUT);
			}

		}
		if (GetAsyncKeyState(VK_RBUTTON) & 0x8000)
		{

			if (pT->TickAndGetTimeDeltaInSeconds() > (GetDoubleClickTime() / 10000.0f))
			{
				//get screen location
				POINT p;
				if (GetCursorPos(&p))
				{
					//cursor position now in p.x and p.y
					if (ScreenToClient(pWinApp->getWindowHandle(), &p))
					{
						Handle h("EVENT", sizeof(Event_MOUSE_RIGHT_HELD));
						Event_MOUSE_RIGHT_HELD* pEvent = new (h) Event_MOUSE_RIGHT_HELD;

						if ((GetAsyncKeyState(VK_LSHIFT) & 0x8000) || GetAsyncKeyState(VK_RSHIFT) & 0x8000)
							pEvent->shiftHeld = true;
						else
							pEvent->shiftHeld = false;

						if (GetAsyncKeyState('P') & 0x8000)
							pEvent->pHeld = true;
						else
							pEvent->pHeld = false;

						pEvent->x = p.x;
						pEvent->y = p.y;
						m_pQueueManager->add(h, Events::QT_INPUT);
					}
				}
			}

		}

		if (GetAsyncKeyState(VK_LBUTTON) & 0x8000)
		{

			if (pT->TickAndGetTimeDeltaInSeconds() > (GetDoubleClickTime() / 10000.0f))
			{
				//get screen location
				POINT p;
				if (GetCursorPos(&p))
				{
					//cursor position now in p.x and p.y

					if (ScreenToClient(pWinApp->getWindowHandle(), &p))
					{
						Handle h("EVENT", sizeof(Event_MOUSE_LEFT_HELD));
						Event_MOUSE_LEFT_HELD* pEvent = new (h) Event_MOUSE_LEFT_HELD;

						if ((GetAsyncKeyState(VK_LCONTROL) & 0x8000) || GetAsyncKeyState(VK_RCONTROL) & 0x8000)
							pEvent->ctrlHeld = true;
						else
							pEvent->ctrlHeld = false;

						pEvent->x = p.x;
						pEvent->y = p.y;
						m_pQueueManager->add(h, Events::QT_INPUT);
					}
				}
			}

		}


		if (GetAsyncKeyState(VK_F1) & 0x8000)
		{
			if (pT->TickAndGetTimeDeltaInSeconds() > (GetDoubleClickTime() / 10000.0f))
			{
				Handle h("EVENT", sizeof(Event_FUN_KEY_HELD));
				Event_FUN_KEY_HELD* pEvent = new (h) Event_FUN_KEY_HELD;
				pEvent->num = VK_F1 % 16;

				if ((GetAsyncKeyState(VK_LCONTROL) & 0x8000) || GetAsyncKeyState(VK_RCONTROL) & 0x8000)
					pEvent->ctrlHeld = true;
				else
					pEvent->ctrlHeld = false;

				m_pQueueManager->add(h, Events::QT_INPUT);
			}
		}
		if (GetAsyncKeyState(VK_F2) & 0x8000)
		{
			if (pT->TickAndGetTimeDeltaInSeconds() > (GetDoubleClickTime() / 10000.0f))
			{
				Handle h("EVENT", sizeof(Event_FUN_KEY_HELD));
				Event_FUN_KEY_HELD* pEvent = new (h) Event_FUN_KEY_HELD;
				pEvent->num = VK_F2 % 16;

				if ((GetAsyncKeyState(VK_LCONTROL) & 0x8000) || GetAsyncKeyState(VK_RCONTROL) & 0x8000)
					pEvent->ctrlHeld = true;
				else
					pEvent->ctrlHeld = false;

				m_pQueueManager->add(h, Events::QT_INPUT);
			}
		}
		if (GetAsyncKeyState(VK_F3) & 0x8000)
		{
			if (pT->TickAndGetTimeDeltaInSeconds() > (GetDoubleClickTime() / 10000.0f))
			{
				Handle h("EVENT", sizeof(Event_FUN_KEY_HELD));
				Event_FUN_KEY_HELD* pEvent = new (h) Event_FUN_KEY_HELD;
				pEvent->num = VK_F3 % 16;

				if ((GetAsyncKeyState(VK_LCONTROL) & 0x8000) || GetAsyncKeyState(VK_RCONTROL) & 0x8000)
					pEvent->ctrlHeld = true;
				else
					pEvent->ctrlHeld = false;

				m_pQueueManager->add(h, Events::QT_INPUT);
			}
		}

		if (GetAsyncKeyState(VK_F4) & 0x8000)
		{
			if (pT->TickAndGetTimeDeltaInSeconds() > (GetDoubleClickTime() / 10000.0f))
			{
				Handle h("EVENT", sizeof(Event_FUN_KEY_HELD));
				Event_FUN_KEY_HELD* pEvent = new (h) Event_FUN_KEY_HELD;
				pEvent->num = VK_F4 % 16;

				if ((GetAsyncKeyState(VK_LCONTROL) & 0x8000) || GetAsyncKeyState(VK_RCONTROL) & 0x8000)
					pEvent->ctrlHeld = true;
				else
					pEvent->ctrlHeld = false;

				m_pQueueManager->add(h, Events::QT_INPUT);
			}
		}

		if (GetAsyncKeyState('1') & 0x8000)
		{
			if (pT->TickAndGetTimeDeltaInSeconds() > (GetDoubleClickTime() / 10000.0f))
			{
				Handle h("EVENT", sizeof(Event_NUM_KEY_HELD));
				Event_NUM_KEY_HELD* pEvent = new (h) Event_NUM_KEY_HELD;
				pEvent->num = '1' % 16;

				if ((GetAsyncKeyState(VK_LCONTROL) & 0x8000) || GetAsyncKeyState(VK_RCONTROL) & 0x8000)
					pEvent->ctrlHeld = true;
				else
					pEvent->ctrlHeld = false;

				m_pQueueManager->add(h, Events::QT_INPUT);
			}
		}

		if (GetAsyncKeyState('2') & 0x8000)
		{
			if (pT->TickAndGetTimeDeltaInSeconds() > (GetDoubleClickTime() / 10000.0f))
			{
				Handle h("EVENT", sizeof(Event_NUM_KEY_HELD));
				Event_NUM_KEY_HELD* pEvent = new (h) Event_NUM_KEY_HELD;
				pEvent->num = '2' % 16;

				if ((GetAsyncKeyState(VK_LCONTROL) & 0x8000) || GetAsyncKeyState(VK_RCONTROL) & 0x8000)
					pEvent->ctrlHeld = true;
				else
					pEvent->ctrlHeld = false;

				m_pQueueManager->add(h, Events::QT_INPUT);
			}
		}

		if (GetAsyncKeyState('3') & 0x8000)
		{
			if (pT->TickAndGetTimeDeltaInSeconds() > (GetDoubleClickTime() / 10000.0f))
			{
				Handle h("EVENT", sizeof(Event_NUM_KEY_HELD));
				Event_NUM_KEY_HELD* pEvent = new (h) Event_NUM_KEY_HELD;
				pEvent->num = '3' % 16;

				if ((GetAsyncKeyState(VK_LCONTROL) & 0x8000) || GetAsyncKeyState(VK_RCONTROL) & 0x8000)
					pEvent->ctrlHeld = true;
				else
					pEvent->ctrlHeld = false;

				m_pQueueManager->add(h, Events::QT_INPUT);
			}
		}

		if (GetAsyncKeyState('4') & 0x8000)
		{
			if (pT->TickAndGetTimeDeltaInSeconds() > (GetDoubleClickTime() / 10000.0f))
			{
				Handle h("EVENT", sizeof(Event_NUM_KEY_HELD));
				Event_NUM_KEY_HELD* pEvent = new (h) Event_NUM_KEY_HELD;
				pEvent->num = '4' % 16;

				if ((GetAsyncKeyState(VK_LCONTROL) & 0x8000) || GetAsyncKeyState(VK_RCONTROL) & 0x8000)
					pEvent->ctrlHeld = true;
				else
					pEvent->ctrlHeld = false;

				m_pQueueManager->add(h, Events::QT_INPUT);
			}
		}
	}
}

}; // namespace Components
}; // namespace PE

#endif // API Abstraction
