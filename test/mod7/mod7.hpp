#define EXPORT_NAME TestMod7
#include <Geode.hpp>


//GEODE_CONCAT(EXPORT_, PROJECT_NAME)

class Mod7Log : geode::ModAPI {
public:
	API_INIT("com.geode.test_seven");

	void logMessage(std::string msg) 
		API_DECL((void(Mod7Log::*)(std::string))&Mod7Log::logMessage, this, msg);

	static void logMessage(int msg, float msg2) 
		API_DECL((void(*)(int, float))&Mod7Log::logMessage, msg, msg2);
};
