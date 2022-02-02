#include <Hook.hpp>
#include <Mod.hpp>
#include <Log.hpp>
#include <Loader.hpp>
#include <Internal.hpp>
#include <ZipUtils.h>
#include <utils/file.hpp>
#include <utils/vector.hpp>
#include <utils/map.hpp>
#include <utils/conststring.hpp>

USE_GEODE_NAMESPACE();

Mod::Mod(ModInfo const& info) {
    this->m_info = info;
}

Mod::~Mod() {
    this->unload();
    for (auto [_, sett] : this->m_info.m_settings) {
        delete sett;
    }
    this->m_info.m_settings.clear();
}

Result<> Mod::createTempDir() {
    ZipFile unzip(this->m_info.m_path);

    if (!unzip.isLoaded()) {
        return Err<>("Unable to unzip " + this->m_info.m_path);
    }

    if (!unzip.fileExists(this->m_info.m_binaryName)) {
        return Err<>(
            "Unable to find platform binary under the name \"" +
            this->m_info.m_binaryName + "\""
        );
    }

    auto tempDir = const_join_path_c_str<geode_directory, geode_temp_directory>;
    if (!ghc::filesystem::exists(tempDir)) {
        if (!ghc::filesystem::create_directory(tempDir)) {
            return Err<>("Unable to create temp directory for mods!");
        }
    }
    
    unsigned long size;
    auto data = unzip.getFileData(this->m_info.m_binaryName, &size);
    if (!data || !size) {
        return Err<>("Unable to read \"" + this->m_info.m_binaryName + "\"");
    }
    
    auto tempPath = ghc::filesystem::path(tempDir) / this->m_info.m_id;
    if (!ghc::filesystem::exists(tempPath) && !ghc::filesystem::create_directories(tempPath)) {
        return Err<>("Unable to create temp directory");
    }
    this->m_tempDirName = tempPath;
    tempPath /= this->m_info.m_binaryName;

    auto wrt = file_utils::writeBinary(tempPath, byte_array(data, data + size));
    if (!wrt) return Err<>(wrt.error());

    return Ok<>(tempPath);
}

Result<> Mod::load() {
    if (!this->m_enabled) {
        return Err<>("Mod is not enabled");
    }
    if (!this->m_tempDirName.string().size()) {
        auto err = this->createTempDir();
        if (!err) return Err<>("Unable to create temp directory: " + err.error());
    }
    if (this->m_loaded) {
        return Ok<>();
    }
    if (this->hasUnresolvedDependencies()) {
        return Err<>("Mod has unresolved dependencies");
    }
    auto err = this->loadPlatformBinary();
    if (!err) return err;
    auto r = this->m_loadFunc(this);
    if (!r) {
        this->unloadPlatformBinary();
        return Err<>("Mod entry point returned an error");
    }
    this->m_loaded = true;
    Loader::get()->updateAllDependencies();
    return Ok<>();
}

Result<> Mod::unload() {
    if (!this->m_loaded) {
        return Ok<>();
    }
    
    this->m_unloadFunc();

    for (auto const& hook : this->m_hooks) {
        auto d = this->disableHook(hook);
        if (!d) return d;
        delete hook;
    }
    this->m_hooks.clear();

    for (auto const& patch : this->m_patches) {
        if (!patch->restore()) {
            return Err<>("Unable to restore patch at " + std::to_string(patch->getAddress()));
        }
        delete patch;
    }
    this->m_patches.clear();

    auto res = this->unloadPlatformBinary();
    if (!res) {
        return res;
    }
    this->m_loaded = false;
    Loader::get()->updateAllDependencies();
    return Ok<>();
}

Result<> Mod::enable() {
    this->m_enabled = true;
    auto r = this->load();
    if (!r) this->m_enabled = false;
    return r;
}

Result<> Mod::disable() {
    this->m_enabled = false;
    auto r = this->unload();
    if (!r) this->m_enabled = true;
    return r;
}

bool Dependency::isUnresolved() const {
    return this->m_required &&
           this->m_state == ModResolveState::Unloaded ||
           this->m_state == ModResolveState::Unresolved ||
           this->m_state == ModResolveState::Disabled;
}

bool Mod::updateDependencyStates() {
    bool hasUnresolved = false;
	for (auto & dep : this->m_info.m_dependencies) {
		if (!dep.m_mod) {
			dep.m_mod = Loader::get()->getLoadedMod(dep.m_id);
		}
		if (dep.m_mod) {
			dep.m_mod->updateDependencyStates();

			if (dep.m_mod->hasUnresolvedDependencies()) {
				dep.m_state = ModResolveState::Unresolved;
			} else {
				if (!dep.m_mod->m_resolved) {
					dep.m_mod->m_resolved = true;
					dep.m_state = ModResolveState::Resolved;
                    auto r = this->load();
                    if (!r) {
                        dep.m_state = ModResolveState::Unloaded;
                        this->logInfo(r.error(), Severity::Error);
                    }
				} else {
					if (dep.m_mod->isEnabled()) {
						dep.m_state = ModResolveState::Loaded;
					} else {
						dep.m_state = ModResolveState::Disabled;
					}
				}
			}
		} else {
			dep.m_state = ModResolveState::Unloaded;
		}
		if (dep.isUnresolved()) {
			this->m_resolved = false;
            this->unload();
            hasUnresolved = true;
		}
	}
    if (!hasUnresolved && !this->m_resolved) {
        this->m_resolved = true;
        auto r = this->load();
        if (!r) this->logInfo(r.error(), Severity::Error);
    }
    return hasUnresolved;
}

bool Mod::hasUnresolvedDependencies() const {
	for (auto const& dep : this->m_info.m_dependencies) {
		if (dep.isUnresolved()) {
			return true;
		}
	}
	return false;
}

std::vector<Dependency> Mod::getUnresolvedDependencies() {
    std::vector<Dependency> res;
	for (auto const& dep : this->m_info.m_dependencies) {
		if (dep.isUnresolved()) {
			res.push_back(dep);
		}
	}
    return res;
}

ghc::filesystem::path Mod::getSaveDir() const {
    return this->m_saveDirName;
}

decltype(ModInfo::m_id) Mod::getID() const {
    return this->m_info.m_id;
}

decltype(ModInfo::m_name) Mod::getName() const {
    return this->m_info.m_name;
}

decltype(ModInfo::m_developer) Mod::getDeveloper() const {
    return this->m_info.m_developer;
}

decltype(ModInfo::m_credits) Mod::getCredits() const {
    return this->m_info.m_credits;
}

decltype(ModInfo::m_description) Mod::getDescription() const {
    return this->m_info.m_description;
}

decltype(ModInfo::m_details) Mod::getDetails() const {
    return this->m_info.m_details;
}

decltype(ModInfo::m_path) Mod::getPath() const {
    return this->m_info.m_path;
}

VersionInfo Mod::getVersion() const {
    return this->m_info.m_version;
}

bool Mod::isEnabled() const {
    return this->m_enabled;
}

bool Mod::supportsDisabling() const {
    return this->m_supportsDisabling;
}

std::vector<Hook*> Mod::getHooks() const {
    return this->m_hooks;
}

LogStream& Mod::log() {
    return Loader::get()->logStream() << this;
}

void Mod::logInfo(
    std::string const& info,
    Severity severity
) {
    auto log = new LogMessage(
        std::string(info),
        severity,
        this
    );
    Loader::get()->log(log);
    #ifdef GEODE_PLATFORM_CONSOLE
    if (Geode::get()->platformConsoleReady()) {
        std::cout << log->toString(true) << "\n";
    } else {
        Geode::get()->queueConsoleMessage(log);
    }
    #endif
}

bool Mod::depends(std::string const& id) const {
    return vector_utils::contains<Dependency>(
        this->m_info.m_dependencies,
        [id](Dependency t) -> bool { return t.m_id == id; }
    );
}

Result<> Mod::setCustomSetting(
    std::string const& key,
    Setting* control,
    bool override
) {
    if (!this->m_info.m_settings.count(key)) {
        return Err<>("No setting with key \"" + key + "\" found");
    }
    auto old = this->m_info.m_settings[key];
    if (override || dynamic_cast<CustomSettingPlaceHolder*>(old)) {
        delete old;
        this->m_info.m_settings[key] = control;
        return Ok<>();
    }
    return Err<>("Setting \"" + key + "\" is not a custom setting type, or has already been registered");
}

std::vector<Setting*> Mod::getSettings() const {
    return map_utils::getValues(this->m_info.m_settings);
}

ghc::filesystem::path Mod::getHotReloadPath() const {
    return this->m_hotReloadPath;
}

Result<> Mod::enableHotReload() {
    if (this->m_hotReloadPath.empty()) {
        return Err<>("Mod does not have a hot reload path set");
    }
    return Geode::get()->enableHotReload(this, this->m_hotReloadPath);
}

void Mod::disableHotReload() {
    return Geode::get()->disableHotReload(this);
}

bool Mod::isHotReloadEnabled() const {
    return Geode::get()->isHotReloadEnabled(const_cast<Mod*>(this));
}
