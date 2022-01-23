#include "entry.hpp"

int geodeEntry(void* platformData) {
    // setup internals
    std::cout << "111" << std::endl;
    auto a = geode::core::hook::initialize();
    Interface::get()->init(InternalMod::get());
    if (!Geode::get()->setup()) {
        // if we've made it here, Geode will 
        // be gettable (otherwise the call to 
        // setup would've immediately crashed)

        Geode::get()->platformMessageBox(
            "Unable to Load Geode!",
            "There was an unknown fatal error setting up "
            "internal tools and Geode can not be loaded."
        );
        return 1;
    }

    std::cout << "222" << std::endl;
    InternalMod::get()->log()
        << Severity::Debug
        << "Loaded internal Geode class"
        << geode::endl;

    // set up loader, load mods, etc.
    if (!Loader::get()->setup()) {
        Geode::get()->platformMessageBox(
            "Unable to Load Geode!",
            "There was an unknown fatal error setting up "
            "the loader and Geode can not be loaded."
        );
        delete Geode::get();
        return 1;
    }

    std::cout << "333" << std::endl;
    InternalMod::get()->log()
        << Severity::Debug
        << "Set up loader"
        << geode::endl;

    // debugging console
    #ifdef GEODE_PLATFORM_CONSOLE
    InternalMod::get()->log()
        << Severity::Debug
        << "Loading Console..."
        << geode::endl;

    Geode::get()->setupPlatformConsole();
    Geode::get()->awaitPlatformConsole();
    Geode::get()->closePlatformConsole();

    InternalMod::get()->log()
        << Severity::Debug
        << "Cleaning up..."
        << geode::endl;

    delete Geode::get();
    #endif
    std::cout << "444" << std::endl;
    return 0;
}
