#pragma once

namespace KZWave {

class Engine
{
public:
	Engine(const string &device);

	void Initialize();
	void Deinitialize();

	struct NodeInfo
	{
		friend class Engine;

		uint32_t         m_homeId;
		uint8_t       m_nodeId;
		bool        m_polled;

		string m_name;
		string m_location;

		string GetType() const
		{
			return OpenZWave::Manager::Get()->GetNodeType(m_homeId, m_nodeId);
		}

		void SetName(const string &name)
		{
			OpenZWave::Manager::Get()->SetNodeName(m_homeId, m_nodeId, name);
		}

		void SetLocation(const string &location)
		{
			OpenZWave::Manager::Get()->SetNodeLocation(m_homeId, m_nodeId, location);
		}

	private:

		list<OpenZWave::ValueID>  m_values;
	};
	typedef boost::shared_ptr<NodeInfo> NodeInfoPtr;

	typedef std::function<void (const NodeInfoPtr &object, const OpenZWave::ValueID &valueId)> ValueChanged;

	//struct NodeObject
	//{
	//	uint32_t homeId;
	//	uint8_t nodeId;
	//	string name;
	//	string location;

	//public:

	//	string GetType() const
	//	{
	//		return OpenZWave::Manager::Get()->GetNodeType(homeId, nodeId);
	//	}

	//	void SetName(const string &name)
	//	{
	//		OpenZWave::Manager::Get()->SetNodeName(homeId, nodeId, name);
	//	}

	//	void SetLocation(const string &location)
	//	{
	//		OpenZWave::Manager::Get()->SetNodeLocation(homeId, nodeId, location);
	//	}
	//};

	set<NodeInfoPtr> GetNodes();
	void Toggle(uint8_t nodeId);

	void AllOn();
	void AllOff();

	void SetValueChangedCallback(ValueChanged callback);

protected:

	void OnNotification(const OpenZWave::Notification *notification);

private:
	static void OnNotificationInternal(const OpenZWave::Notification *notification, void *engine);

	NodeInfoPtr GetNodeInfo(OpenZWave::Notification const* _notification);

	uint32_t m_homeId;

	ValueChanged m_valueChangedCallback;

	set<NodeInfoPtr> m_nodes;
	boost::mutex m_mutex;
	boost::condition_variable m_initCond;
	boost::mutex m_initMutex;
	string m_device;
	bool m_initFailed;
};

}
