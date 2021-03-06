#include "kzwave.h"

using namespace OpenZWave;

int main(int argc, char *argv[])
{
	//Py_Initialize();
	//auto moduleName = PyString_FromString("/home/generallee5686/K-ZWave/Rules/switch_state_email.py");

	//if(!moduleName)
	//{
	//	PyErr_Print();
	//	exit(0);
	//}

	//auto module = PyImport_Import(moduleName);

	//if(!module)
	//{
	//	PyErr_Print();
	//	exit(0);
	//}

	//Py_Finalize();

	KZWave::Engine engine("/dev/ttyUSB0");
	engine.Initialize();

	map<uint8_t, KZWave::Engine::NodeInfoPtr> nodes;

	auto list = engine.GetNodes();

	foreach(auto &node, list)
	{
		auto id = node->m_nodeId;
		nodes.insert(make_pair(id, std::move(node)));
	}
	
	char buffer[1024];
	while(true)
	{
		std::cout << "> ";
		std::cin.getline(buffer, sizeof(buffer));
		string line(buffer);

		try
		{
			if(line == "exit")
				break;
			else if(line == "ls")
			{
				foreach(auto &node, nodes)
				{
					std::cout << static_cast<int>(node.second->m_nodeId) << ") ";
					std::cout << "[" << node.second->GetType() << "] ";
					std::cout << "(" <<  node.second->m_location << ") -- " << node.second->m_name << std::endl;
				}
			}
			else if(boost::starts_with(line, "toggle "))
			{
				vector<string> split;
				boost::split(split, line, boost::is_any_of(" "));

				if(split.size() != 2)
					continue;

				auto nodeId = boost::lexical_cast<uint32_t>(split[1]);
				engine.Toggle(nodeId);
			}
			else if(line == "allon")
			{
				engine.AllOn();
			}
			else if(line == "alloff")
			{
				engine.AllOff();
			}
			else if(boost::starts_with(line, "setname "))
			{
				vector<string> split;
				boost::split(split, line, boost::is_any_of(" "));

				if(split.size() != 3)
				{
					std::cout << "Needs 2 arguments (nodeId name)" << std::endl;
					continue;
				}

				auto nodeId = boost::lexical_cast<uint32_t>(split[1]);

				auto node = nodes.find(nodeId);

				if(node == nodes.end())
					cout << "Invalid node id" << endl;

				node->second->SetName(split[2]);
			}
			else if(boost::starts_with(line, "setlocation "))
			{
				vector<string> split;
				boost::split(split, line, boost::is_any_of(" "));

				if(split.size() != 3)
				{
					std::cout << "Needs 2 arguments (nodeId name)" << std::endl;
					continue;
				}

				auto nodeId = boost::lexical_cast<uint32_t>(split[1]);

				auto node = nodes.find(nodeId);

				if(node == nodes.end())
					cout << "Invalid node id" << endl;

				node->second->SetLocation(split[2]);
			}

		}
		catch(boost::bad_lexical_cast){}
	}

	engine.Deinitialize();
}
