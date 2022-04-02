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
#include <utils/string.hpp>

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
    ZipFile unzip(this->m_info.m_path.string());

    if (!unzip.isLoaded()) {
        return Err<>("Unable to unzip " + this->m_info.m_path.string());
    }

    if (!unzip.fileExists(this->m_info.m_binaryName)) {
        return Err<>(
            "Unable to find platform binary under the name \"" +
            this->m_info.m_binaryName + "\""
        );
    }

    auto tempDir = Loader::get()->getGeodeDirectory() / geodeTempDirectory;
    if (!ghc::filesystem::exists(tempDir)) {
        if (!ghc::filesystem::create_directory(tempDir)) {
            return Err<>("Unable to create temp directory for mods!");
        }
    }
    
    auto tempPath = ghc::filesystem::path(tempDir) / this->m_info.m_id;
    if (!ghc::filesystem::exists(tempPath) && !ghc::filesystem::create_directories(tempPath)) {
        return Err<>("Unable to create temp directory");
    }
    this->m_tempDirName = tempPath;

    for (auto file : unzip.getAllFiles()) {
        auto path = ghc::filesystem::path(file);
        if (path.has_parent_path()) {
            if (
                !ghc::filesystem::exists(tempPath / path.parent_path()) &&
                !ghc::filesystem::create_directories(tempPath / path.parent_path())
            ) {
                return Err<>("Unable to create directories \"" + path.parent_path().string() + "\"");
            }
        }
        unsigned long size;
        auto data = unzip.getFileData(file, &size);
        if (!data || !size) {
            return Err<>("Unable to read \"" + std::string(file) + "\"");
        }
        auto wrt = file_utils::writeBinary(
            tempPath / file,
            byte_array(data, data + size)
        );
        if (!wrt) return Err<>("Unable to write \"" + file + "\": " + wrt.error());
    }

    this->m_addResourcesToSearchPath = true;
    Loader::get()->addModResourcesPath(this);

    return Ok<>(tempPath);
}

Result<> Mod::load() {
    #define RETURN_LOAD_ERR(str) \
        {m_loadErrorInfo = str; \
        return Err<>(m_loadErrorInfo);}

    if (!this->m_tempDirName.string().size()) {
        auto err = this->createTempDir();
        if (!err) RETURN_LOAD_ERR("Unable to create temp directory: " + err.error());
    }
    if (this->m_loaded) {
        return Ok<>();
    }
    if (this->hasUnresolvedDependencies()) {
        RETURN_LOAD_ERR("Mod has unresolved dependencies");
    }
    auto err = this->loadPlatformBinary();
    if (!err) RETURN_LOAD_ERR(err.error());
    if (this->m_implicitLoadFunc) {
        auto r = this->m_implicitLoadFunc(this);
        if (!r) {
            this->unloadPlatformBinary();
            RETURN_LOAD_ERR("Implicit mod entry point returned an error");
        }
    }
    if (this->m_loadFunc) {
        auto r = this->m_loadFunc(this);
        if (!r) {
            this->unloadPlatformBinary();
            RETURN_LOAD_ERR("Mod entry point returned an error");
        }
    }
    this->m_loaded = true;
    this->m_enabled = true;
    if (this->m_loadDataFunc) {
        if (!this->m_loadDataFunc(this->m_saveDirPath.string().c_str())) {
            this->logInfo("Mod load data function returned false", Severity::Error);
        }
    }
    m_loadErrorInfo = "";
    Loader::get()->updateAllDependencies();
    return Ok<>();
}

Result<> Mod::unload() {
    if (!this->m_loaded) {
        return Ok<>();
    }
    
    if (this->m_saveDataFunc) {
        if (!this->m_saveDataFunc(this->m_saveDirPath.string().c_str())) {
            this->logInfo("Mod save data function returned false", Severity::Error);
        }
    }

    if (this->m_unloadFunc) {
        this->m_unloadFunc();
    }

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
    if (!this->m_loaded) {
        auto r = this->load();
        if (!r) this->m_enabled = false;
        return r;
    }
    
    if (this->m_enableFunc) {
        if (!this->m_enableFunc()) {
            return Err<>("Mod enable function returned false");
        }
    }

    for (auto const& hook : this->m_hooks) {
        auto d = this->enableHook(hook);
        if (!d) return d;
    }

    for (auto const& patch : this->m_patches) {
        if (!patch->apply()) {
            return Err<>("Unable to apply patch at " + std::to_string(patch->getAddress()));
        }
    }

    this->m_enabled = true;

    return Ok<>();
}

Result<> Mod::disable() {
    if (this->m_disableFunc) {
        if (!this->m_disableFunc()) {
            return Err<>("Mod disable function returned false");
        }
    }

    for (auto const& hook : this->m_hooks) {
        auto d = this->disableHook(hook);
        if (!d) return d;
    }

    for (auto const& patch : this->m_patches) {
        if (!patch->restore()) {
            return Err<>("Unable to restore patch at " + std::to_string(patch->getAddress()));
        }
    }

    this->m_enabled = false;

    return Ok<>();
}

bool Dependency::isUnresolved() const {
    return this->m_required &&
           (this->m_state == ModResolveState::Unloaded ||
           this->m_state == ModResolveState::Unresolved ||
           this->m_state == ModResolveState::Disabled);
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
                    auto r = dep.m_mod->load();
                    if (!r) {
                        dep.m_state = ModResolveState::Unloaded;
                        dep.m_mod->logInfo(r.error(), Severity::Error);
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
        Log::get() << Severity::Debug << "All dependencies for " << m_info.m_id << " found";
        this->m_resolved = true;
        if (this->m_enabled) {
            Log::get() << Severity::Debug << "Resolved & loading " << m_info.m_id;
            auto r = this->load();
            if (!r) {
                Log::get() << Severity::Error << this << "Error loading: " << r.error();
            }
        } else {
            Log::get() << Severity::Debug << "Resolved " << m_info.m_id << ", however not loading it as it is disabled";
        }
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
    return this->m_saveDirPath;
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

decltype(ModInfo::m_creditsString) Mod::getCredits() const {
    return this->m_info.m_creditsString;
}

decltype(ModInfo::m_description) Mod::getDescription() const {
    return this->m_info.m_description;
}

decltype(ModInfo::m_details) Mod::getDetails() const {
    return this->m_info.m_details;
}

Result<> Mod::saveData() {
    bool savedmod = true;
    if (this->m_saveDataFunc) {
        savedmod = this->m_saveDataFunc(this->m_saveDirPath.string().c_str());
    }
    auto json = nlohmann::json::object();
    for (auto [key, sett] : this->m_info.m_settings) {
        auto value = nlohmann::json::object();
        auto r = sett->save(value);
        if (!r) return r;
        json[key] = value;
    }
    auto res = file_utils::writeString(this->m_saveDirPath / "settings.json", json.dump(4));
    if (!res) return res;
    if (!savedmod) return Err<>("Mod save function returned false");
    return Ok<>();
}

Result<> Mod::loadData() {
    bool loadedmod = true;
    if (this->m_loadDataFunc) {
        loadedmod = this->m_loadDataFunc(this->m_saveDirPath.string().c_str());
    }
    if (!ghc::filesystem::exists(this->m_saveDirPath / "settings.json")) {
        if (!loadedmod)
            return Err<>("Mod load function returned false");
        return Ok<>();
    }
    auto read = file_utils::readString(this->m_saveDirPath / "settings.json");
    if (!read) return read;
    try {
        auto json = nlohmann::json::parse(read.value());
        for (auto [key, sett] : this->m_info.m_settings) {
            if (json.contains(key)) {
                auto r = sett->load(json[key]);
                if (!r) return r;
            }
        }
        if (!loadedmod)
            return Err<>("Mod load function returned false");
        return Ok<>();
    } catch(std::exception const& e) {
        return Err<>(e.what());
    }
}

std::string Mod::getPath() const {
    return this->m_info.m_path.string();
}

VersionInfo Mod::getVersion() const {
    return this->m_info.m_version;
}

bool Mod::isEnabled() const {
    return this->m_enabled;
}

bool Mod::isLoaded() const {
    return this->m_loaded;
}

bool Mod::supportsDisabling() const {
    return this->m_info.m_supportsDisabling;
}

bool Mod::wasSuccesfullyLoaded() const {
    return !this->isEnabled() || this->isLoaded();
}

std::vector<Hook*> Mod::getHooks() const {
    return this->m_hooks;
}

Log Mod::log() {
    return Log(this);
}

void Mod::logInfo(
    std::string const& info,
    Severity severity
) {
    Log l(this);
    l << severity << info;
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

const char* Mod::expandSpriteName(const char* name) {
    static std::unordered_map<std::string, const char*> expanded = {};
    if (expanded.count(name)) {
        return expanded[name];
    }
    auto exp = new char[strlen(name) + 2 + this->m_info.m_id.size()];
    auto exps = this->m_info.m_id + "_" + name;
    memcpy(exp, exps.c_str(), exps.size() + 1);
    expanded[name] = exp;
    return exp;
}

std::string Mod::getLoadErrorInfo() const {
    return m_loadErrorInfo;
}
