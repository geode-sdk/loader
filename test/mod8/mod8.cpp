#include "../mod7/mod7.hpp"

USE_GEODE_NAMESPACE();

GEODE_API bool GEODE_CALL geode_load(Mod* mod) {
	Interface::get()->init(mod);
	Mod7Log log;
	log.logMessage("adgsjdgsklf");
	Mod7Log::logMessage(122, 0.433f);
    return true;
}

