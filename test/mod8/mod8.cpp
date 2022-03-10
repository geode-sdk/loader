#include "mod8.hpp"

USE_GEODE_NAMESPACE();

void Mod8Log::logMessage(std::string msg) {
    Log::get() << "Logged using the api function: " << msg;
}

GEODE_API bool GEODE_CALL geode_load(Mod* mod) {
	Interface::get()->init(mod);
	Mod7Log log;
	log.logMessage("adgsjdgsklf");
	Mod7Log::logMessage(122, 0.433f);
    return true;
}

