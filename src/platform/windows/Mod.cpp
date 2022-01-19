#pragma once

#include <lilac.hpp>

#ifdef LILAC_IS_WINDOWS

USE_LILAC_NAMESPACE();

Result<> Mod::loadPlatformBinary() {
    auto load = LoadLibraryW((this->m_tempDirName / this->m_info.m_binaryName).wstring().c_str());
    if (load) {
        this->m_loadFunc   = reinterpret_cast<lilac_load>(  GetProcAddress(load, "lilac_load"));
        this->m_unloadFunc = reinterpret_cast<lilac_unload>(GetProcAddress(load, "lilac_unload"));

        if (
            !this->m_loadFunc &&
            !(this->m_loadFunc = reinterpret_cast<lilac_load>(  GetProcAddress(load, "_lilac_load@4")))
        ) {
            return Err<>("Unable to find mod entry point");
        }
        if (
            !this->m_unloadFunc &&
            !(this->m_unloadFunc = reinterpret_cast<lilac_unload>(  GetProcAddress(load, "_lilac_unload@0")))
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
