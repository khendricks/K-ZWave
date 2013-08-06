#pragma once

namespace KZWave {

class Engine
{
public:
	Engine(const string &device);

	void Initialize();
	void Deinitialize();

	struct NodeObject
	{
		uint32_t homeId;
		uint8_t nodeId;
		string name;
		string location;

	public:

		string GetType() const
		{
			return OpenZWave::Manager::Get()->GetNodeType(homeId, nodeId);
		}

		void SetName(const string &name)
		{
			OpenZWave::Manager::Get()->SetNodeName(homeId, nodeId, name);
		}

		void SetLocation(const string &location)
		{
			OpenZWave::Manager::Get()->SetNodeLocation(homeId, nodeId, location);
		}
	};

	list<NodeObject> GetNodes();
	void Toggle(uint8_t nodeId);

	void AllOn();
	void AllOff();


protected:

	void OnNotification(const OpenZWave::Notification *notification);

private:
	static void OnNotificationInternal(const OpenZWave::Notification *notification, void *engine);

	typedef struct
	{
		uint32_t         m_homeId;
		uint8_t       m_nodeId;
		bool        m_polled;

		string m_name;
		string m_location;

		list<OpenZWave::ValueID>  m_values;
	} NodeInfo;
	typedef boost::shared_ptr<NodeInfo> NodeInfoPtr;

	NodeInfoPtr GetNodeInfo(OpenZWave::Notification const* _notification);

	uint32_t m_homeId;

	set<NodeInfoPtr> m_nodes;
	boost::mutex m_mutex;
	boost::condition_variable m_initCond;
	boost::mutex m_initMutex;
	string m_device;
	bool m_initFailed;
};

}
