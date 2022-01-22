#include <Geode>

#ifdef GEODE_IS_MACOS

#include <dlfcn.h>

USE_GEODE_NAMESPACE();

Result<> Mod::loadPlatformBinary() {
	auto dylib = dlopen((this->m_tempDirName / this->m_info.m_binaryName).string().c_str(), RTLD_NOW);
    if (dylib) {
        this->m_loadFunc   = reinterpret_cast<geode_load>(  dlsym(dylib, "geode_load"));
        if (!this->m_loadFunc) 
        	this->m_loadFunc   = reinterpret_cast<geode_load>(  dlsym(dylib, "_geode_load"));
       	if (!this->m_loadFunc) 
       		return Err<>("Unable to find mod entry point");

        this->m_unloadFunc = reinterpret_cast<geode_unload>(dlsym(dylib, "geode_unload"));
        if (!this->m_unloadFunc) 
        	this->m_unloadFunc   = reinterpret_cast<geode_unload>(  dlsym(dylib, "_geode_unload"));
       	if (!this->m_unloadFunc) 
       		return Err<>("Unable to find mod unload function");

        if (this->m_platformInfo) {
            delete this->m_platformInfo;
        }
        this->m_platformInfo = new PlatformInfo { dylib };

        return Ok<>();
    }
    return Err<>("Unable to load the DYLIB");
}

Result<> Mod::unloadPlatformBinary() {
    auto dylib = this->m_platformInfo->m_dylib;
    delete this->m_platformInfo;
    if (dlclose(dylib)) {
        this->m_unloadFunc = nullptr;
        this->m_loadFunc = nullptr;
        return Ok<>();
    } else {
        return Err<>("Unable to free library");
    }
}

#endif
