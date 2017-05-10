
#include "StandardGameEvents.h"

#include "../Lua/LuaEnvironment.h"

namespace PE {
namespace Events {

	PE_IMPLEMENT_CLASS1(Event_FLY_CAMERA, Event);
	PE_IMPLEMENT_CLASS1(Event_ROTATE_CAMERA, Event);
	PE_IMPLEMENT_CLASS1(Event_GET_WORLD_POS, Event);
	PE_IMPLEMENT_CLASS1(Event_SELECT_UNIT, Event);
	PE_IMPLEMENT_CLASS1(Event_TOGGLE_DEBUG_RENDER, Event);
	PE_IMPLEMENT_CLASS1(Event_CAMERA_STORAGE, Event);
	PE_IMPLEMENT_CLASS1(Event_CONTROL_GROUP, Event);
};
};
