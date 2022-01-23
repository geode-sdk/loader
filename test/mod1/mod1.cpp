#include "mod1.hpp"

GEODE_API bool GEODE_CALL geode_load(Mod* mod) {
    Interface::get()->init(mod);
    
    logMessage("Hi from TestMod1");

    return true;
}

GEODE_API void GEODE_CALL geode_unload() {}

void logMessage(std::string_view const& msg) {
    Interface::mod()->log() << "Logged by TestMod1: " << msg << geode::endl;
}

