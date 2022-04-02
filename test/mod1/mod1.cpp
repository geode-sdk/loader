#include "mod1.hpp"

GEODE_API bool GEODE_CALL geode_load(Mod* mod) {
	std::cout <<"dsfkj"<< std::endl;
    logMessage("Hi from TestMod1");
    return true;
}

void logMessage(std::string_view const& msg) {
    Log::get() << "Logged by TestMod1: " << msg;
}

