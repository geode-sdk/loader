#include "../mod7/mod7.hpp"

#define EXPORT_NAME TestMod8
#include <Geode.hpp>


class Mod8Log : geode::ModAPI {
public:
	API_INIT("com.geode.test_eight");

	void logMessage(std::string msg) 
		API_DECL(&Mod8Log::logMessage, this, msg);
};
