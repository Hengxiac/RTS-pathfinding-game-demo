#define NOMINMAX
// API Abstraction
#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

#include "GhostManager.h"

// Outer-Engine includes
#include <stdio.h>

// Inter-Engine includes

#include "../Lua/LuaEnvironment.h"

// additional lua includes needed
extern "C"
{
#include "../../luasocket_dist/src/socket.h"
#include "../../luasocket_dist/src/inet.h"
};

#include "../../../GlobalConfig/GlobalConfig.h"

#include "PrimeEngine/Events/StandardEvents.h"
#include "PrimeEngine/Networking/NetworkManager.h"
#include "PrimeEngine/Networking/Client/ClientNetworkManager.h"

#include "PrimeEngine/Scene/DebugRenderer.h"

#include "StreamManager.h"
// Sibling/Children includes
using namespace PE::Events;

namespace PE {
namespace Components {

PE_IMPLEMENT_CLASS1(GhostManager, Component);

GhostManager::GhostManager(PE::GameContext &context, PE::MemoryArena arena, PE::NetworkContext &netContext, Handle hMyself)
: Component(context, arena, hMyself)
, m_transmitterNextGstOrderId(1) // start at 1 since id = 0 is not ordered
, m_transmitterNumGstsNotAcked(0)
, m_stateMask(context, arena, MAX_GHOST_NUM)
, m_ghostDictionary(context,arena, MAX_GHOST_NUM)
//, m_nextGhostId(0)
// receiver
, m_receiverFirstGstOrderId(1) // start at 1 since id = 0 is not ordered
{
	m_pNetContext = &netContext;

	memset(&m_receivedGhosts[0], 0, sizeof(m_receivedGhosts));
}

GhostManager::~GhostManager()
{

}

void GhostManager::initialize()
{

}

void GhostManager::addDefaultComponents()
{
	Component::addDefaultComponents();
}

void GhostManager::scheduleGhost(PE::Networkable *pNetworkableGhost, /*PE::Networkable *pNetworkableTarget,*/ bool guaranteed, int GhostID, bool statusFlag, int ghostType)
{
	if (haveGhostsToSend() >= PE_MAX_EVENT_JAM)
	{
		assert(!"Sending too many ghosts have to drop, need throttling mechanism here");
		return;
	}

	char pKey[256];
	getGhostIdChar(GhostID, pKey);
	int index = m_ghostDictionary.findIndex(pKey);

	if (index < 0)
		return;

	bool updated = statusFlag;

	for (int i = 0; i < MASK_SIZE; i++)
	{
		updated = updated || m_stateMask[index][i];
	}

	if (!updated)
	{
		//assert(!"Nothing is updated for this ghost");
		return;
	}

	if (statusFlag && ghostType < 0)
	{
		assert(!"Must Specify the ghost type to create");
		return;
	}

	m_ghostsToSend.push_back(GhostTransmissionData());
	GhostTransmissionData &back = m_ghostsToSend.back();
	int dataSize = 0;

	back.m_isGuaranteed = guaranteed;
	
	if (!guaranteed)
		back.m_orderId = 0; // zero means not guaranteed
	else
		back.m_orderId = m_transmitterNextGstOrderId++;

	// debug info to show event id sceduled
	//PEINFO("Scheduling event order id: %d\n", m_transmitterNextEvtOrderId);


	//write ordering id (0 = not guaranteed)
	dataSize += StreamManager::WriteInt32(back.m_orderId, &back.m_payload[dataSize]);

	//target
	//dataSize += StreamManager::WriteNetworkId(pNetworkableTarget->m_networkId, &back.m_payload[dataSize]);

	//ghost id
	back.m_ghostId = GhostID;
	dataSize += StreamManager::WriteInt32(back.m_ghostId, &back.m_payload[dataSize]);
	
	//status flag
	back.m_statusFlag = statusFlag;
	dataSize += StreamManager::WriteBool(back.m_statusFlag, &back.m_payload[dataSize]);
	
	back.m_ghostType = ghostType;
	//class id
	if (statusFlag)
	{
		dataSize += StreamManager::WriteInt32(back.m_ghostType, &back.m_payload[dataSize]);
	}

	//state mask
	for (int i = 0; i < MASK_SIZE; i++)
	{
		back.m_stateMask[i] = m_stateMask[index][i];
		dataSize += StreamManager::WriteBool(back.m_stateMask[i], &back.m_payload[dataSize]);
	}

	/*PrimitiveTypes::Int32 classId = pNetworkableEvent->net_getClassMetaInfo()->m_classId;

	if (classId == -1)
	{
		assert(!"Event's class id is -1, need to add it to global registry");
	}

	dataSize += StreamManager::WriteInt32(classId, &back.m_payload[dataSize]);*/
	
	dataSize += pNetworkableGhost->packStateData(&back.m_payload[dataSize]);
	
	back.m_size = dataSize;
	
}

int GhostManager::haveGhostsToSend()
{
	return m_ghostsToSend.size() > 0;
}

int GhostManager::fillInNextPacket(char *pDataStream, TransmissionRecord *pRecord, int packetSizeAllocated, bool &out_usefulDataSent, bool &out_wantToSendMore)
{
	out_usefulDataSent = false;
    out_wantToSendMore = false;

	int ghostsToSend = haveGhostsToSend();
	assert(ghostsToSend);

	int ghostsReallySent = 0;

	int size = 0;
	size += StreamManager::WriteInt32(ghostsToSend, &pDataStream[size]);

	int sizeLeft = packetSizeAllocated - size;

	for (int i = 0; i < ghostsToSend; ++i)
	{
		int iGst = i;
		assert(iGst < (int)(m_ghostsToSend.size()));
		GhostTransmissionData &gst = m_ghostsToSend[iGst];

		if (gst.m_size > sizeLeft)
		{
			// can't fit this event, break out
			// note this code can be optimized to include next events that can potentailly fit in
            out_wantToSendMore = true;
			break;
		}

		// store this to be able to resolve which events were delivered or dropped on transmittion notification
		// todo: optimize to use pointers and store data somewhere else
		pRecord->m_sentGhosts.push_back(gst);

		//int checkOrderId = 0;
		//StreamManager::ReadInt32( &evt.m_payload[0], checkOrderId);
		//debug info to show event order id of packet being sent
		//PEINFO("Order id check: %d\n", checkOrderId);
		
		memcpy(&pDataStream[size], &gst.m_payload[0], gst.m_size);
		size += gst.m_size;
		sizeLeft = packetSizeAllocated - size;

		ghostsReallySent++;
		m_transmitterNumGstsNotAcked++;
		
		resetStateMask(gst.m_ghostId);
	}
	
	if (ghostsReallySent > 0)
	{
		m_ghostsToSend.erase(m_ghostsToSend.begin(), m_ghostsToSend.begin() + ghostsReallySent);
	}
	
	//write real value into the beginning of event chunk
	StreamManager::WriteInt32(ghostsReallySent, &pDataStream[0 /* the number of events is stored in the beginning and we already wrote value into here*/]);
	
	// we are sending useful data only if we are sending events
	out_usefulDataSent = ghostsReallySent > 0;

	return size;
}

void GhostManager::processNotification(TransmissionRecord *pTransmittionRecord, bool delivered)
{
	for (unsigned int i = 0; i < pTransmittionRecord->m_sentGhosts.size(); ++i)
	{
		GhostTransmissionData &gst = pTransmittionRecord->m_sentGhosts[i];

		if (gst.m_isGuaranteed)
		{
			if (delivered)
			{
				//we're good, can pop this event off front
				m_transmitterNumGstsNotAcked--; // will advance sliding window
			}
			else
			{
				// need to readjust our sliding window and make sure we start sending events starting at at least this event
				//assert(!"Not supported for now!");
				//m_ghostsToSend.push_front(evt);
				for (int i = 0; i < MASK_SIZE; i++)
				{

				}
				
				m_transmitterNumGstsNotAcked--; // will advance sliding window since we need to resend this event
			}
		}
		else
		{
			m_transmitterNumGstsNotAcked--; // will advance sliding window

			// event wasn't guaranteed, we can forget about it
		}
	}
}



int GhostManager::receiveNextPacket(char *pDataStream)
{
	int read = 0;
	PrimitiveTypes::Int32 numGhosts;
	
	read += StreamManager::ReadInt32(&pDataStream[read], numGhosts);

	for (int i = 0; i < numGhosts; ++i)
	{
		PrimitiveTypes::Int32 gstOrderId;
		read += StreamManager::ReadInt32(&pDataStream[read], gstOrderId); // 0 means not guaranteed, > 0 means ordering id
		
		PrimitiveTypes::Int32 ghostId;
		read += StreamManager::ReadInt32(&pDataStream[read], ghostId);

		PrimitiveTypes::Bool statusFlag;
		read += StreamManager::ReadBool(&pDataStream[read], statusFlag);
		//Networkable *pTargetNetworkable = NULL;
		Component *pTargetComponent = NULL;

		PrimitiveTypes::Int32 ghostType = -1;
		Events::Event *pEvt;
		// todo: retrieve object to send event to

		char pKey[256];
		getGhostIdChar(ghostId, pKey);
		if (m_ghostDictionary.findIndex(pKey) >= 0 && !statusFlag)
		{			
			PE::Handle h = m_ghostDictionary.findHandle(pKey);
			pTargetComponent = h.getObject<Component>();

			pEvt = new (m_arena) Event_Update_Ghost();
			read += pEvt->constructFromStream(&pDataStream[read]);
		}
		else
		{
			if (statusFlag)
			{				
				//read += StreamManager::ReadInt32(&pDataStream[read], ghostType);			
				pTargetComponent = (Component*) m_pContext->getGameObjectManager();
				
				if (m_ghostDictionary.findIndex(pKey) >= 0)
				{
					Event_Destroy_Ghost* pRealEvt = new (m_arena) Event_Destroy_Ghost();
					pRealEvt->m_ghostId = ghostId;
					pEvt = (Events::Event*)pRealEvt;
					read += pEvt->constructFromStream(&pDataStream[read]);
				
				}
				else
				{
					Event_Create_Ghost* pRealEvt = new (m_arena) Event_Create_Ghost();
					pRealEvt->m_ghostId = ghostId;

					pEvt = (Events::Event*)pRealEvt;
					read += pEvt->constructFromStream(&pDataStream[read]);
				}
						
			}

			else
			{
				pEvt = new (m_arena) Event_Update_Ghost();
				read += pEvt->constructFromStream(&pDataStream[read]);

				//assert(!"Ghost is not in the library, the update will not be processed");
			}	
		}
		
		pEvt->m_networkClientId = m_pNetContext->getClientId(); // will be id of client on server, or -1 on client

		
		// debug to show id of received event
		//PEINFO("Received Event with eventOrderId %d\n", evtOrderId);

		if (gstOrderId > 0)
		{
			// this is an ordered guaranteed event
			
			// is it within sliding window?
			int indexInGhostsArray = gstOrderId - m_receiverFirstGstOrderId;
			if (indexInGhostsArray < 0)
			{
				// received an old event, discard
				PEASSERT(false, "Received event order id %d that precedes current sliding window currently starting at %d. We dont support this situation (old events)\n", gstOrderId, m_receiverFirstGstOrderId);
				delete pEvt;
			}
			else if (indexInGhostsArray >= PE_GHOST_SLIDING_WINDOW)
			{
				// received event too much in advance, have to discard
				PEASSERT(false, "Received ghost too far in advance of current sliding window. We dont have mechanism to handle this yet, so make sliding window bigger. \
								Current sliding window starts at %d, order of this event is %d. Sliding window size: \n", m_receiverFirstGstOrderId, gstOrderId, PE_GHOST_SLIDING_WINDOW);
				delete pEvt;
			}
			else if (m_receivedGhosts[indexInGhostsArray].m_pEvent)
			{
				// this event has already been received. discard
				PEASSERT(false, "Received event order id %d that has already been received We dont support this situation\n", gstOrderId);
				delete pEvt;
			}
			else
			{
				m_receivedGhosts[indexInGhostsArray].m_pEvent = pEvt;
				m_receivedGhosts[indexInGhostsArray].m_pTargetComponent = pTargetComponent;
			}

		}
		else
		{
			// this is not guaranteed event (execute and forget)
			if (pTargetComponent)
			{
				pTargetComponent->handleEvent(pEvt);
			}
				
			delete pEvt;
		}
	}

	// check receiver sliding window and process events if have events for needed order ids

	int numProcessed = 0;
	for (int indexInGhostsArray = 0; indexInGhostsArray < PE_GHOST_SLIDING_WINDOW; ++indexInGhostsArray)
	{
		if (m_receivedGhosts[indexInGhostsArray].m_pEvent)
		{
			if (m_receivedGhosts[indexInGhostsArray].m_pTargetComponent)
			{
				m_receivedGhosts[indexInGhostsArray].m_pTargetComponent->handleEvent(m_receivedGhosts[indexInGhostsArray].m_pEvent);
			}
			numProcessed++;
			delete m_receivedGhosts[indexInGhostsArray].m_pEvent;
		}
		else
			break;
	}

	//shift leftover events if can
	if (numProcessed && numProcessed < PE_GHOST_SLIDING_WINDOW)
	{
		int numLeftoverGhosts = PE_GHOST_SLIDING_WINDOW-numProcessed;
		memmove(&m_receivedGhosts[0], &m_receivedGhosts[numProcessed], sizeof(EventReceptionData) * numLeftoverGhosts);
		memset(&m_receivedGhosts[numLeftoverGhosts], 0, sizeof(EventReceptionData) * (PE_GHOST_SLIDING_WINDOW-numLeftoverGhosts));
	}

	m_receiverFirstGstOrderId += numProcessed; // advance sliding window
	
	return read;
}

void GhostManager::debugRender(int &threadOwnershipMask, float xoffset/* = 0*/, float yoffset/* = 0*/, bool isClient)
{
	float dy = 0.025f;
	float dx = 0.01f;
	sprintf(PEString::s_buf, "Ghost Manager:");
	DebugRenderer::Instance()->createTextMesh(
		PEString::s_buf, true, false, false, false, 0,
		Vector3(xoffset, yoffset, 0), 1.0f, threadOwnershipMask);

	sprintf(PEString::s_buf, "Recv Window Range: [%d, %d]", m_receiverFirstGstOrderId, m_receiverFirstGstOrderId + PE_GHOST_SLIDING_WINDOW - 1);
	DebugRenderer::Instance()->createTextMesh(
		PEString::s_buf, true, false, false, false, 0,
		Vector3(xoffset + dx, yoffset + dy, 0), 1.0f, threadOwnershipMask);

	char tmpBuf[1024];
	char tmpBuf2[1024];

	sprintf(tmpBuf, "%s", "[");

	for (int i = 0; i < PE_GHOST_SLIDING_WINDOW; ++i)
	{
		if (m_receivedGhosts[i].m_pEvent && m_receivedGhosts[i].m_pTargetComponent )
		{
			sprintf(tmpBuf2, "%s+", tmpBuf);
		}
		else
		{
			sprintf(tmpBuf2, "%s ", tmpBuf);
		}
		sprintf(tmpBuf, "%s", tmpBuf2);
	}

	sprintf(tmpBuf2, "%s]", tmpBuf);
	sprintf(tmpBuf, "%s", tmpBuf2);

	DebugRenderer::Instance()->createTextMesh(
		tmpBuf, true, false, false, false, 0,
		Vector3(xoffset + dx, yoffset + dy * 2, 0), 0.7f, threadOwnershipMask);

	sprintf(PEString::s_buf, "Send next id: %d", m_transmitterNextGstOrderId);

	DebugRenderer::Instance()->createTextMesh(
		PEString::s_buf, true, false, false, false, 0,
		Vector3(xoffset + dx, yoffset + dy * 3, 0), 1.0f, threadOwnershipMask);

	sprintf(tmpBuf, "State Mask: ");

	if (isClient)
	{
		int clientId = ((ClientNetworkManager *)(m_pContext->getNetworkManager()))->m_clientId;
		
		
		sprintf(tmpBuf2, "%s[", tmpBuf);
		sprintf(tmpBuf, "%s", tmpBuf2);

		if (clientId+1)
		{
			for (int i = 0; i < MASK_SIZE; ++i)
			{
				sprintf(tmpBuf2, "%s%d", tmpBuf, m_stateMask[clientId][i]);
				sprintf(tmpBuf, "%s", tmpBuf2);
			}
		}

		sprintf(tmpBuf2, "%s] ", tmpBuf);
		sprintf(tmpBuf, "%s", tmpBuf2);
	}
	else
	{
		for (int j = 0; j < m_stateMask.m_size; j++)
		{
			sprintf(tmpBuf2, "%s[", tmpBuf);
			sprintf(tmpBuf, "%s", tmpBuf2);

			for (int i = 0; i < MASK_SIZE; ++i)
			{
				sprintf(tmpBuf2, "%s%d", tmpBuf, m_stateMask[j][i]);
				sprintf(tmpBuf, "%s", tmpBuf2);
			}
		
			sprintf(tmpBuf2, "%s] ", tmpBuf);
			sprintf(tmpBuf, "%s", tmpBuf2);
		}
	}
	DebugRenderer::Instance()->createTextMesh(
		tmpBuf, true, false, false, false, 0,
		Vector3(xoffset + dx, yoffset + dy * 4, 0), 1.0f, threadOwnershipMask);
	
}

bool GhostManager::addToLibrary(Handle h, int ghostId) 
{
	if (m_stateMask.m_size == MAX_GHOST_NUM)
	{
		return false;
	}
	char pKey[256];
	getGhostIdChar(ghostId, pKey);
	m_ghostDictionary.add(pKey, h);
	
	Array<bool> stateMask(*m_pContext, m_arena, MASK_SIZE);
	
	for (int i = 0;i < MASK_SIZE; i++)
	{
		stateMask.add(1);
	}
	
	m_stateMask.add(stateMask);

	return true;
}

bool GhostManager::removeFromLibrary(int ghostId)
{
	char pKey[256];
	getGhostIdChar(ghostId, pKey);
	int index = m_ghostDictionary.findIndex(pKey);

	if (index < 0)
	{
		return false;
	}

	m_stateMask.remove(index);
	m_ghostDictionary.m_pairs.remove(index);

	return true;
}

void GhostManager::resetStateMask(int ghostId)
{

	char pKey[256];
	getGhostIdChar(ghostId, pKey);
	int index = m_ghostDictionary.findIndex(pKey);
	
	for (int i = 0; i < MASK_SIZE; i++)
	{
		m_stateMask[index][i] = 0;
	}
}

int GhostManager::findGhostID(PE::Handle h) 
{
	char* strId = m_ghostDictionary.findStr(h);

	if (strId)
	{
		int ghostId = atoi(strId);
		//sscanf(strId, "%d", &ghostId);
		
		if (ghostId >= 0)
			return ghostId;
	}
	return -1;
}

void GhostManager::updateStateMask(int ghostId, PrimitiveTypes::UInt32 bitnum)
{
	char pKey[256];
	getGhostIdChar(ghostId, pKey);
	int index = m_ghostDictionary.findIndex(pKey);
	m_stateMask[index][bitnum] = 1;

}

Array<bool> GhostManager::findStateMask(int ghostId)
{
	char pKey[256];
	getGhostIdChar(ghostId, pKey);
	int index = m_ghostDictionary.findIndex(pKey);
	return m_stateMask[index];

}

PE::Handle GhostManager::getGhostHandle(int ghostId)
{
	char pKey[256];
	getGhostIdChar(ghostId, pKey);
	int index = m_ghostDictionary.findIndex(pKey);
	if (index >= 0)
	{
		return m_ghostDictionary.m_pairs[index].m_handle;
	}
	else
	{
		return Handle();
	}
}

bool GhostManager::ghostIdExist(int ghostId)
{
	char pKey[256];
	getGhostIdChar(ghostId, pKey);
	int index = m_ghostDictionary.findIndex(pKey);
	return index >= 0;
}
void GhostManager::getGhostIdChar(int ghostId, char* pKey)
{
	sprintf(pKey, "%ld", ghostId);
}

#if 0 // template
//////////////////////////////////////////////////////////////////////////
// ConnectionManager Lua Interface
//////////////////////////////////////////////////////////////////////////
//
void ConnectionManager::SetLuaFunctions(PE::Components::LuaEnvironment *pLuaEnv, lua_State *luaVM)
{
	/*
	static const struct luaL_Reg l_functions[] = {
		{"l_clientConnectToTCPServer", l_clientConnectToTCPServer},
		{NULL, NULL} // sentinel
	};

	luaL_register(luaVM, 0, l_functions);
	*/

	lua_register(luaVM, "l_clientConnectToTCPServer", l_clientConnectToTCPServer);


	// run a script to add additional functionality to Lua side of Skin
	// that is accessible from Lua
// #if APIABSTRACTION_IOS
// 	LuaEnvironment::Instance()->runScriptWorkspacePath("Code/PrimeEngine/Scene/Skin.lua");
// #else
// 	LuaEnvironment::Instance()->runScriptWorkspacePath("Code\\PrimeEngine\\Scene\\Skin.lua");
// #endif

}

int ConnectionManager::l_clientConnectToTCPServer(lua_State *luaVM)
{
	lua_Number lPort = lua_tonumber(luaVM, -1);
	int port = (int)(lPort);

	const char *strAddr = lua_tostring(luaVM, -2);

	GameContext *pContext = (GameContext *)(lua_touserdata(luaVM, -3));

	lua_pop(luaVM, 3);

	pContext->getConnectionManager()->clientConnectToTCPServer(strAddr, port);

	return 0; // no return values
}
#endif
//////////////////////////////////////////////////////////////////////////

	
}; // namespace Components
}; // namespace PE
