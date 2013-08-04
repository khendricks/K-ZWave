#include "KZWave.h"

using namespace OpenZWave;

static uint32 g_homeId = 0;
static bool   g_initFailed = false;

typedef struct
{
   uint32         m_homeId;
   uint8       m_nodeId;
   bool        m_polled;
   list<ValueID>  m_values;
}NodeInfo;

static list<NodeInfo*> g_nodes;
static pthread_mutex_t g_criticalSection;
static pthread_cond_t  initCond  = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t initMutex = PTHREAD_MUTEX_INITIALIZER;

//-----------------------------------------------------------------------------
// <GetNodeInfo>
// Return the NodeInfo object associated with this notification
//-----------------------------------------------------------------------------
NodeInfo* GetNodeInfo(Notification const* _notification)
{
   uint32 const homeId = _notification->GetHomeId();
   uint8 const nodeId = _notification->GetNodeId();
   for( list<NodeInfo*>::iterator it = g_nodes.begin(); it != g_nodes.end(); ++it )
   {
      NodeInfo* nodeInfo = *it;
      if( ( nodeInfo->m_homeId == homeId ) && ( nodeInfo->m_nodeId == nodeId ) )
      {
         return nodeInfo;
      }
   }

   return NULL;
}

void OnNotification(const Notification *_notification, void *_context)
{
	pthread_mutex_lock( &g_criticalSection );

	switch(_notification->GetType())
	{
		case Notification::Type_ValueAdded:
			std::cout << "Value Added" << std::endl;

			if( NodeInfo* nodeInfo = GetNodeInfo( _notification ) )
         {
            // Add the new value to our list
            nodeInfo->m_values.push_back( _notification->GetValueID() );
         }
			break;
		case Notification::Type_ValueRemoved:
			std::cout << "Value Removed" << std::endl;

			if( NodeInfo* nodeInfo = GetNodeInfo( _notification ) )
         {
            // Remove the value from out list
            for( list<ValueID>::iterator it = nodeInfo->m_values.begin(); it != nodeInfo->m_values.end(); ++it )
            {
               if( (*it) == _notification->GetValueID() )
               {
                  nodeInfo->m_values.erase( it );
                  break;
               }
            }
         }
			break;
		case Notification::Type_ValueChanged:

			// One of the node values has changed
         if( NodeInfo* nodeInfo = GetNodeInfo( _notification ) )
         {
				auto vid = _notification->GetValueID();
				auto name = Manager::Get()->GetNodeName(nodeInfo->m_homeId, nodeInfo->m_nodeId);
				auto location = Manager::Get()->GetNodeLocation(nodeInfo->m_homeId, nodeInfo->m_nodeId);
				auto fullName = " " + name + " (" + location + ")";

				std::string i;
				Manager::Get()->GetValueAsString(vid, &i);
				std::cout << "Value changed" << fullName << " " << Manager::Get()->GetValueLabel(vid) << ":" << i << std::endl;

            nodeInfo = nodeInfo;    // placeholder for real action
         }
			break;
		case Notification::Type_Group:
			std::cout << "Group" << std::endl;

			// One of the node's association groups has changed
         if( NodeInfo* nodeInfo = GetNodeInfo( _notification ) )
         {
            nodeInfo = nodeInfo;    // placeholder for real action
         }
			break;
		case Notification::Type_NodeAdded:
		{
			std::cout << "Node Added" << std::endl;
			
			// Add the new node to our list
         NodeInfo* nodeInfo = new NodeInfo();
         nodeInfo->m_homeId = _notification->GetHomeId();
         nodeInfo->m_nodeId = _notification->GetNodeId();
         nodeInfo->m_polled = false;
         g_nodes.push_back(nodeInfo);
			break;
		}
		case Notification::Type_NodeRemoved:
		{
			std::cout << "Node Removed" << std::endl;

			// Remove the node from our list
         uint32 const homeId = _notification->GetHomeId();
         uint8 const nodeId = _notification->GetNodeId();
         for( list<NodeInfo*>::iterator it = g_nodes.begin(); it != g_nodes.end(); ++it )
         {
            NodeInfo* nodeInfo = *it;
            if( ( nodeInfo->m_homeId == homeId ) && ( nodeInfo->m_nodeId == nodeId ) )
            {
               g_nodes.erase( it );
               delete nodeInfo;
               break;
            }
         }
			break;
		}
		case Notification::Type_NodeEvent:
			std::cout << "Node Event" << std::endl;

			// We have received an event from the node, caused by a
         // basic_set or hail message.
         if( NodeInfo* nodeInfo = GetNodeInfo( _notification ) )
         {
            nodeInfo = nodeInfo;    // placeholder for real action
         }

			break;
		case Notification::Type_PollingDisabled:
			std::cout << "Polling Disabled" << std::endl;

			if( NodeInfo* nodeInfo = GetNodeInfo( _notification ) )
         {
            nodeInfo->m_polled = false;
         }
			break;
		case Notification::Type_PollingEnabled:
			std::cout << "Polling Enabled" << std::endl;

			if( NodeInfo* nodeInfo = GetNodeInfo( _notification ) )
         {
            nodeInfo->m_polled = true;
         }
			break;
		case Notification::Type_DriverReady:
			std::cout << "Driver Ready" << std::endl;

			g_homeId = _notification->GetHomeId();
			break;
		case Notification::Type_DriverFailed:
			std::cout << "Driver Failed" << std::endl;

			g_initFailed = true;
         pthread_cond_broadcast(&initCond);
			break;

		case Notification::Type_AwakeNodesQueried:
			std::cout << "Awake Nodes Queried" << std::endl;
         pthread_cond_broadcast(&initCond);
		break;
		case Notification::Type_AllNodesQueried:
			std::cout << "All Nodes Queried" << std::endl;
         pthread_cond_broadcast(&initCond);
		break;
		case Notification::Type_AllNodesQueriedSomeDead:
			std::cout << "All Nodes Queried, some dead" << std::endl;
         pthread_cond_broadcast(&initCond);
		break;

		case Notification::Type_DriverReset:
			std::cout << "Driver Rest" << std::endl;
			break;
		case Notification::Type_Notification:
			std::cout << "Notification" << std::endl;
			break;
		case Notification::Type_NodeNaming:
			std::cout << "Node Naming" << std::endl;
			break;
		case Notification::Type_NodeProtocolInfo:
			std::cout << "Node Protocol Info" << std::endl;
			break;
		case Notification::Type_NodeQueriesComplete:
			std::cout << "Node Queries Complete" << std::endl;
			break;
		default:
			std::cout << "Other" << std::endl;
			break;
	}

	pthread_mutex_unlock( &g_criticalSection );
}

int main(int argc, char *argv[])
{
	// Create the OpenZWave Manager.
	// The first argument is the path to the config files (where the manufacturer_specific.xml file is located
	// The second argument is the path for saved Z-Wave network state and the log file.  If you leave it NULL 
	// the log file will appear in the program's working directory.
	Options::Create("config", "", "" );
	//Options::Get()->AddOptionInt( "SaveLogLevel", LogLevel_Detail );
	//Options::Get()->AddOptionInt( "QueueLogLevel", LogLevel_Debug );
	//Options::Get()->AddOptionInt( "DumpTrigger", LogLevel_Error );
	Options::Get()->AddOptionInt( "PollInterval", 500 );
	Options::Get()->AddOptionBool( "ConsoleOutput", false );
	Options::Get()->AddOptionBool( "IntervalBetweenPolls", true );
	Options::Get()->AddOptionBool("ValidateValueChanges", true);
	Options::Get()->Lock();

	Manager::Create();

	// Add a callback handler to the manager.  The second argument is a context that
	// is passed to the OnNotification method.  If the OnNotification is a method of
	// a class, the context would usually be a pointer to that class object, to
	// avoid the need for the notification handler to be a static.
	Manager::Get()->AddWatcher(OnNotification, NULL);

	Manager::Get()->AddDriver("/dev/ttyUSB0");

	pthread_cond_wait( &initCond, &initMutex );
	std::cout << "DONE" << std::endl;

	std::string s;

	std::cin.ignore();

	for(NodeInfo *nodeInfo : g_nodes)
	{
		pthread_mutex_lock( &g_criticalSection );
		if(nodeInfo->m_nodeId == 2)
		{
			for(auto value : nodeInfo->m_values)
			{
				if(value.GetType() == ValueID::ValueType_Bool)
				{
					bool s;
					Manager::Get()->GetValueAsBool(value, &s);
					std::cout << s << std::endl;
					std::cout << !s << std::endl;
					Manager::Get()->SetValue(value, !s);
					//Manager::Get()->SetNodeName(g_homeId, nodeInfo->m_nodeId, "LIVING_LIGHT");
					//Manager::Get()->SetNodeLocation(g_homeId, nodeInfo->m_nodeId, "LivingRoom");
				}
			}
		}
		pthread_mutex_unlock( &g_criticalSection );
	}

	std::cin.ignore();
	std::cout << "END" << std::endl;

	Manager::Get()->RemoveDriver("/dev/ttyUSB0");
	Manager::Get()->Destroy();
	Options::Destroy();
	pthread_mutex_destroy( &g_criticalSection );
}
