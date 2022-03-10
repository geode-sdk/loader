#include "mod7.hpp"

USE_GEODE_NAMESPACE();

void Mod7Log::logMessage(std::string msg) {
    Log::get() << "Logged using the api function: " << msg;
}

void Mod7Log::logMessage(int msg, float msg2) {
    Log::get() << "Logged using the api function v2: " << msg << " " << msg2;
}

GEODE_API bool GEODE_CALL geode_load(Mod* mod) {
	Interface::get()->init(mod);
    return true;
}

