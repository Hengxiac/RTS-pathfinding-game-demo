#ifndef __PrimeEngineGhostManager_H__
#define __PrimeEngineGhostManager_H__

// API Abstraction
#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

// Outer-Engine includes
#include <assert.h>
#include <deque>

// Inter-Engine includes

#include "../Events/Component.h"

extern "C"
{
#include "../../luasocket_dist/src/socket.h"
};

#include "PrimeEngine/Networking/NetworkContext.h"
#include "PrimeEngine/Utils/Networkable.h"
#include "PrimeEngine/Utils/StrToHandleMap.h"

// Sibling/Children includes
#include "Packet.h"

//#define MASK_SIZE	3;

namespace PE {
namespace Components {

struct GhostManager : public Component
{
	static const int PE_GHOST_SLIDING_WINDOW = 64;

	PE_DECLARE_CLASS(GhostManager);

	// Constructor -------------------------------------------------------------
	GhostManager(PE::GameContext &context, PE::MemoryArena arena, PE::NetworkContext &netContext, Handle hMyself);

	virtual ~GhostManager();

	// Methods -----------------------------------------------------------------
	virtual void initialize();

	/// called by gameplay code to schedule event transmission to client(s)
	void scheduleGhost(PE::Networkable *pNetworkable, /*PE::Networkable *pNetworkableTarget,*/ 
		bool guaranteed, int GhostID, bool statusFlag = false, int ghostType = -1);

	/// called by stream manager to see how many events to send
	int haveGhostsToSend();

	/// called by StreamManager to put queued up events in packet
	int fillInNextPacket(char *pDataStream, TransmissionRecord *pRecord, int packetSizeAllocated, bool &out_usefulDataSent, bool &out_wantToSendMore);

	/// called by StreamManager to process transmission record deliver notification
	void processNotification(TransmissionRecord *pTransmittionRecord, bool delivered);

	void debugRender(int &threadOwnershipMask, float xoffset = 0, float yoffset = 0, bool isClient = false);

	int receiveNextPacket(char *pDataStream);

	int findGhostID(PE::Handle h);

	// Component ------------------------------------------------------------
	virtual void addDefaultComponents();

	bool addToLibrary(Handle h, int ghostId);

	bool removeFromLibrary(int ghostId);

	void resetStateMask(int ghostID);

	void updateStateMask(int ghostId, PrimitiveTypes::UInt32 bitNum);

	Array<bool> findStateMask(int ghostId);

	PE::Handle getGhostHandle(int ghostId);

	bool ghostIdExist(int ghostId);

	//int m_nextGhostId;

	// Individual events -------------------------------------------------------

	// Loading -----------------------------------------------------------------

#if 0 // template
	//////////////////////////////////////////////////////////////////////////
	// Skin Lua Interface
	//////////////////////////////////////////////////////////////////////////
	//
	static void SetLuaFunctions(PE::Components::LuaEnvironment *pLuaEnv, lua_State *luaVM);
	//
	static int l_clientConnectToTCPServer(lua_State *luaVM);
	//
	//////////////////////////////////////////////////////////////////////////
#endif
	//////////////////////////////////////////////////////////////////////////
	// Member variables 
	//////////////////////////////////////////////////////////////////////////

	std::deque<GhostTransmissionData> m_ghostsToSend;

	// transmitter
	int m_transmitterNextGstOrderId;
	int m_transmitterNumGstsNotAcked; //= number of events stored in TransmissionRecords

	//int m_nextGhostId;

	static const int MASK_SIZE = PE_GHOST_MASK_SIZE;
	static const int MAX_GHOST_NUM	= 1024;
										// receiver
	int m_receiverFirstGstOrderId; // evtOrderId of first element in m_receivedEvents

	EventReceptionData m_receivedGhosts[PE_GHOST_SLIDING_WINDOW];


	PE::NetworkContext *m_pNetContext;

	Array<Array<bool>> m_stateMask;

	StrToHandleMap m_ghostDictionary;

	static void getGhostIdChar(int ghostId, char* pKey);
};
}; // namespace Components
}; // namespace PE
#endif
