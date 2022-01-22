#include <Geode>

#ifdef GEODE_IS_MACOS

#include <dlfcn.h>

USE_GEODE_NAMESPACE();

Result<> Mod::loadPlatformBinary() {
	void* dylib = dlopen((this->m_tempDirName / this->m_info.m_binaryName).string().c_str(), RTLD_NOW);
    if (dylib) {
        this->m_loadFunc   = reinterpret_cast<geode_load>(  GetProcAddress(load, "geode_load"));
        this->m_unloadFunc = reinterpret_cast<geode_unload>(GetProcAddress(load, "geode_unload"));

        if (
            !this->m_loadFunc &&
            !(this->m_loadFunc = reinterpret_cast<geode_load>(  GetProcAddress(load, "_geode_load@4")))
        ) {
            return Err<>("Unable to find mod entry point");
        }
        if (
            !this->m_unloadFunc &&
            !(this->m_unloadFunc = reinterpret_cast<geode_unload>(  GetProcAddress(load, "_geode_unload@0")))
        ) {
            return Err<>("Unable to find mod unload function");
        }

        if (this->m_platformInfo) {
            delete this->m_platformInfo;
        }
        this->m_platformInfo = new PlatformInfo { load };

        return Ok<>();
    }
    return Err<>("Unable to load the DLL");
}

Result<> Mod::unloadPlatformBinary() {
    auto hmod = this->m_platformInfo->m_hmod;
    delete this->m_platformInfo;
    if (FreeLibrary(hmod)) {
        this->m_unloadFunc = nullptr;
        this->m_loadFunc = nullptr;
        return Ok<>();
    } else {
        return Err<>("Unable to free library");
    }
}

#endif
