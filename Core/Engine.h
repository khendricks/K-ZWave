#pragma once

namespace KZWave {

class Engine
{
public:
	Engine(string &device);

	void Initialize();

protected:

	void OnNotification(const OpenZWave::Notification *_notification, void *_context);

private:
	typedef struct
	{
		uint32_t         m_homeId;
		uint8_t       m_nodeId;
		bool        m_polled;
		list<OpenZWave::ValueID>  m_values;
	} NodeInfo;
	typedef boost::shared_ptr<NodeInfo> NodeInfoPtr;

	NodeInfoPtr GetNodeInfo(OpenZWave::Notification const* _notification);

	uint32_t m_homeId;

	list<NodeInfoPtr> m_nodes;
	boost::mutex m_mutex;
	boost::condition_variable m_initCond;
	boost::mutex m_initMutex;
	string m_device;
};

}
