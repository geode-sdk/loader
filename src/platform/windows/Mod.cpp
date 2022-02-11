#include <Geode.hpp>

#ifdef GEODE_IS_WINDOWS

USE_GEODE_NAMESPACE();

template<typename T>
T findSymbolOrMangled(HMODULE load, const char* name, const char* mangled) {
    auto res = reinterpret_cast<T>(GetProcAddress(load, name));
    if (!res) {
        res = reinterpret_cast<T>(GetProcAddress(load, mangled));
    }
    return res;
}

std::string getLastWinError() {
    auto err = GetLastError();
    if (!err) return "None (0)";
    char* text = nullptr;
    auto len = FormatMessageA(
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        err,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        text,
        0,
        nullptr
    );
    if (len == 0) {
        return "Unformattable (" + std::to_string(err) + ")";
    }
    if (text != nullptr) {
        auto str = std::string(text, len) + " (" + std::to_string(err) + ")";
        LocalFree(text);
        return str;
    }
    return "Unknown (" + std::to_string(err) + ")";
}

Result<> Mod::loadPlatformBinary() {
    auto load = LoadLibraryW((this->m_tempDirName / this->m_info.m_binaryName).wstring().c_str());
    if (load) {
        this->m_implicitLoadFunc = findSymbolOrMangled<geode_load>(load, "geode_implicit_load", "_geode_implicit_load@4");
        this->m_loadFunc = findSymbolOrMangled<geode_load>(load, "geode_load", "_geode_load@4");
        this->m_unloadFunc = findSymbolOrMangled<geode_unload>(load, "geode_unload", "_geode_unload@0");
        this->m_enableFunc = findSymbolOrMangled<geode_enable>(load, "geode_enable", "_geode_enable@0");
        this->m_disableFunc = findSymbolOrMangled<geode_disable>(load, "geode_disable", "_geode_disable@0");
        this->m_saveDataFunc = findSymbolOrMangled<geode_save_data>(load, "geode_save_data", "_geode_save_data@4");
        this->m_loadDataFunc = findSymbolOrMangled<geode_load_data>(load, "geode_load_data", "_geode_load_data@4");

        if (!this->m_implicitLoadFunc && !this->m_loadFunc) {
            return Err<>("Unable to find mod entry point (lacking both implicit & explicit definition)");
        }

        if (this->m_platformInfo) {
            delete this->m_platformInfo;
        }
        this->m_platformInfo = new PlatformInfo { load };

        return Ok<>();
    }
    return Err<>("Unable to load the DLL: " + getLastWinError());
}

Result<> Mod::unloadPlatformBinary() {
    auto hmod = this->m_platformInfo->m_hmod;
    delete this->m_platformInfo;
    if (FreeLibrary(hmod)) {
        this->m_implicitLoadFunc = nullptr;
        this->m_unloadFunc = nullptr;
        this->m_loadFunc = nullptr;
        return Ok<>();
    } else {
        return Err<>("Unable to free the DLL: " + getLastWinError());
    }
}

#endif
