#include <Geode.hpp>
#include <utils/platform.hpp>

#ifdef GEODE_IS_MACOS

#include <dlfcn.h>

USE_GEODE_NAMESPACE();

template<typename T>
T findSymbolOrMangled(void* dylib, const char* name, const char* mangled) {
	auto res = reinterpret_cast<T>(dlsym(dylib, name));
	if (!res) {
		res = reinterpret_cast<T>(dlsym(dylib,mangled));
	}
	return res;
}

Result<> Mod::loadPlatformBinary() {
	auto dylib = dlopen((this->m_tempDirName / this->m_info.m_binaryName).string().c_str(), RTLD_NOW);
	if (dylib) {
		this->m_implicitLoadFunc = findSymbolOrMangled<geode_load>(dylib, "geode_implicit_load", "_geode_implicit_load");
		this->m_loadFunc = findSymbolOrMangled<geode_load>(dylib, "geode_load", "_geode_load");
		this->m_unloadFunc = findSymbolOrMangled<geode_unload>(dylib, "geode_unload", "_geode_unload");

		if (!this->m_implicitLoadFunc && !this->m_loadFunc) {
			return Err<>("Unable to find mod entry point (lacking both implicit & explicit definition)");
		}

		if (this->m_platformInfo) {
			delete this->m_platformInfo;
		}
		this->m_platformInfo = new PlatformInfo { dylib };

		return Ok<>();
	}
	std::string err = (char const*)dlerror();
	return Err<>("Unable to load the DYLIB: dlerror returned (" + err + ")");
}

Result<> Mod::unloadPlatformBinary() {
	auto dylib = this->m_platformInfo->m_dylib;
	delete this->m_platformInfo;
	this->m_platformInfo = nullptr;
	if (dlclose(dylib) == 0) {
		this->m_unloadFunc = nullptr;
		this->m_loadFunc = nullptr;
		this->m_implicitLoadFunc = nullptr;
		return Ok<>();
	} else {
		
		return Err<>("Unable to free library");
	}
}

#endif
