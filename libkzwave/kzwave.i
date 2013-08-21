%module kzwave
%{
#include "kzwave.h"
%}

%include "std_string.i"

namespace KZWave {

class Engine
{
public:
	Engine(const std::string &device);

	void Initialize();
	void Deinitialize();
};

}

//%include "kzwave.h"
