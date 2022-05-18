#include <Geode.hpp>

#ifdef GEODE_IS_ANDROID

USE_GEODE_NAMESPACE();

Result<> Mod::loadPlatformBinary() {
    return Err<>("Unable to load the .so: i havent implemented it :(");
}

Result<> Mod::unloadPlatformBinary() {
    return Err<>("Unable to unload the .so: i havent implemented it :(");
}

#endif
