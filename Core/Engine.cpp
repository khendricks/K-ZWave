#include "KZWave.h"

using namespace OpenZWave;

namespace KZWave {

Engine::Engine(string &device) : m_device(device)
{
}

void Engine::Initialize()
{
}

Engine::NodeInfoPtr Engine::GetNodeInfo(Notification const* _notification)
{
   uint32 const homeId = _notification->GetHomeId();
   uint8 const nodeId = _notification->GetNodeId();
   //for( list<NodeInfo*>::iterator it = g_nodes.begin(); it != g_nodes.end(); ++it )
   foreach(auto nodeInfo, m_nodes)
   {
      if( ( nodeInfo->m_homeId == homeId ) && ( nodeInfo->m_nodeId == nodeId ) )
      {
         return nodeInfo;
      }
   }

   return NodeInfoPtr();
}

void Engine::OnNotification(const Notification *_notification, void *_context)
{
}

}
