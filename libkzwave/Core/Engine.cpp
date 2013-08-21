#include "kzwave.h"

using namespace OpenZWave;

namespace KZWave {

Engine::Engine(const string &device) : m_device(device),
	m_initFailed(false)
{
}

void Engine::OnNotificationInternal(const Notification *notification, void *engine)
{
	static_cast<Engine*>(engine)->OnNotification(notification);
}

void Engine::Initialize()
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
	Manager::Get()->AddWatcher(OnNotificationInternal, this);

	Manager::Get()->AddDriver(m_device);

	// Wait for the driver to become initialized
	boost::unique_lock<boost::mutex> lock(m_initMutex);
	m_initCond.wait(lock);
}

void Engine::Deinitialize()
{
	Manager::Get()->RemoveDriver(m_device);
	Manager::Get()->Destroy();
	Options::Destroy();
}

set<Engine::NodeInfoPtr> Engine::GetNodes()
{
	return m_nodes;
}

void Engine::Toggle(uint8_t nodeId)
{
	foreach(auto &node, m_nodes)
	{
		std::cout << static_cast<int>(node->m_nodeId) << " " << static_cast<int>(nodeId) << std::endl;
		if(node->m_nodeId == nodeId)
		{

			foreach(auto &vid, node->m_values)
			{
				if(vid.GetCommandClassId() == OpenZWave::SwitchBinary::StaticGetCommandClassId())
				{
					bool val;
					if(!Manager::Get()->GetValueAsBool(vid, &val))
						return;

					Manager::Get()->SetValue(vid, !val);
				}
			}

			break;
		}
	}
}

void Engine::AllOn()
{
	Manager::Get()->SwitchAllOn(m_homeId);
}

void Engine::AllOff()
{
	Manager::Get()->SwitchAllOff(m_homeId);
}

void Engine::SetValueChangedCallback(ValueChanged callback)
{
	m_valueChangedCallback = callback;
}

Engine::NodeInfoPtr Engine::GetNodeInfo(Notification const* notification)
{
   uint32 const homeId = notification->GetHomeId();
   uint8 const nodeId = notification->GetNodeId();

   foreach(auto nodeInfo, m_nodes)
   {
      if( ( nodeInfo->m_homeId == homeId ) && ( nodeInfo->m_nodeId == nodeId ) )
      {
         return nodeInfo;
      }
   }

   return NodeInfoPtr();
}

void Engine::OnNotification(const Notification *notification)
{
	boost::lock_guard<boost::mutex> lock(m_mutex);

	switch(notification->GetType())
	{
		case Notification::Type_ValueAdded:
			std::cout << "Value Added" << std::endl;

			if( auto nodeInfo = GetNodeInfo( notification ) )
         {
            // Add the new value to our list
            nodeInfo->m_values.push_back( notification->GetValueID() );
         }
			break;
		case Notification::Type_ValueRemoved:
			std::cout << "Value Removed" << std::endl;

			if( auto nodeInfo = GetNodeInfo( notification ) )
         {
            // Remove the value from out list
            for( list<ValueID>::iterator it = nodeInfo->m_values.begin(); it != nodeInfo->m_values.end(); ++it )
            {
               if( (*it) == notification->GetValueID() )
               {
                  nodeInfo->m_values.erase( it );
                  break;
               }
            }
         }
			break;
		case Notification::Type_ValueChanged:

			// One of the node values has changed
         if(auto nodeInfo = GetNodeInfo(notification))
         {
				auto vid = notification->GetValueID();
				auto name = Manager::Get()->GetNodeName(nodeInfo->m_homeId, nodeInfo->m_nodeId);
				auto location = Manager::Get()->GetNodeLocation(nodeInfo->m_homeId, nodeInfo->m_nodeId);
				auto fullName = " " + name + " (" + location + ")";

				std::string i;
				Manager::Get()->GetValueAsString(vid, &i);
				std::cout << "Value changed" << fullName << " " << Manager::Get()->GetValueLabel(vid) << ":" << i << std::endl;

				if(m_valueChangedCallback)
					m_valueChangedCallback(nodeInfo, vid);
         }
			break;
		case Notification::Type_Group:
			std::cout << "Group" << std::endl;

			// One of the node's association groups has changed
         if( auto nodeInfo = GetNodeInfo( notification ) )
         {
            nodeInfo = nodeInfo;    // placeholder for real action
         }
			break;
		case Notification::Type_NodeAdded:
		{
			std::cout << "Node Added" << std::endl;
			
			// Add the new node to our list
         auto nodeInfo = NodeInfoPtr(new NodeInfo());
         nodeInfo->m_homeId = notification->GetHomeId();
         nodeInfo->m_nodeId = notification->GetNodeId();
         nodeInfo->m_polled = false;
         m_nodes.insert(nodeInfo);
			break;
		}
		case Notification::Type_NodeRemoved:
		{
			std::cout << "Node Removed" << std::endl;

			// Remove the node from our list
         uint32 const homeId = notification->GetHomeId();
         uint8 const nodeId = notification->GetNodeId();
         //for( list<NodeInfo*>::iterator it = g_nodes.begin(); it != g_nodes.end(); ++it )
         foreach(auto nodeInfo, m_nodes)
         {
            if( ( nodeInfo->m_homeId == homeId ) && ( nodeInfo->m_nodeId == nodeId ) )
            {
               m_nodes.erase(nodeInfo);
               break;
            }
         }
			break;
		}
		case Notification::Type_NodeEvent:
			std::cout << "Node Event" << std::endl;

			// We have received an event from the node, caused by a
         // basic_set or hail message.
         if( auto nodeInfo = GetNodeInfo(notification) )
         {
            nodeInfo = nodeInfo;    // placeholder for real action
         }

			break;
		case Notification::Type_PollingDisabled:
			std::cout << "Polling Disabled" << std::endl;

			if( auto nodeInfo = GetNodeInfo( notification ) )
         {
            nodeInfo->m_polled = false;
         }
			break;
		case Notification::Type_PollingEnabled:
			std::cout << "Polling Enabled" << std::endl;

			if( auto nodeInfo = GetNodeInfo( notification ) )
         {
            nodeInfo->m_polled = true;
         }
			break;
		case Notification::Type_DriverReady:
			std::cout << "Driver Ready" << std::endl;

			m_homeId = notification->GetHomeId();
			break;
		case Notification::Type_DriverFailed:
			std::cout << "Driver Failed" << std::endl;

			m_initFailed = true;
			m_initCond.notify_all();
			break;

		case Notification::Type_AwakeNodesQueried:
			std::cout << "Awake Nodes Queried" << std::endl;
			m_initCond.notify_all();
		break;
		case Notification::Type_AllNodesQueried:
			std::cout << "All Nodes Queried" << std::endl;
			m_initCond.notify_all();
		break;
		case Notification::Type_AllNodesQueriedSomeDead:
			std::cout << "All Nodes Queried, some dead" << std::endl;
			m_initCond.notify_all();
		break;

		case Notification::Type_DriverReset:
			std::cout << "Driver Rest" << std::endl;
			break;
		case Notification::Type_Notification:
			std::cout << "Notification" << std::endl;
			break;
		case Notification::Type_NodeNaming:
			std::cout << "Node Naming" << std::endl;
         if( auto nodeInfo = GetNodeInfo( notification ) )
         {
				nodeInfo->m_name = Manager::Get()->GetNodeName(nodeInfo->m_homeId, nodeInfo->m_nodeId);
				nodeInfo->m_location = Manager::Get()->GetNodeLocation(nodeInfo->m_homeId, nodeInfo->m_nodeId);
			}
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
}

}
