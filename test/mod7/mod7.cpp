#include <Geode.hpp>

USE_GEODE_NAMESPACE();

void logMessage(std::string msg) {
    Interface::mod()->log() << "Logged using the api functions: " << msg;
}

GEODE_API bool GEODE_CALL geode_load(Mod* mod) {
    logMessage("Hi from TestMod7");
    mod->exportAPIFunction("TestMod7.&logMessage", &logMessage);
    auto logMessageAPI = mod->importAPIFunction<decltype(&logMessage)>("TestMod7.&logMessage");
    logMessageAPI("Hi from TestMod7 API");
    return true;
}

