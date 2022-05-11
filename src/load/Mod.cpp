#include <loader/Hook.hpp>
#include <loader/Mod.hpp>
#include <loader/Log.hpp>
#include <loader/Loader.hpp>
#include <InternalLoader.hpp>
#include <ZipUtils.h>
#include <utils/file.hpp>
#include <utils/vector.hpp>
#include <utils/map.hpp>
#include <utils/conststring.hpp>
#include <utils/string.hpp>
#include <InternalMod.hpp>

USE_GEODE_NAMESPACE();

nlohmann::json& DataStore::operator[](std::string const& key) {
    return m_mod->m_dataStore[key];
}

DataStore& DataStore::operator=(nlohmann::json& jsn) {
    m_mod->m_dataStore = jsn;
    return *this;
}

DataStore::operator nlohmann::json() {
    return m_mod->m_dataStore;
}

DataStore::~DataStore() {
    if (m_store != m_mod->m_dataStore) {
        m_mod->postDSUpdate();
    }
}

Mod::Mod(ModInfo const& info) {
    this->m_info = info;
}

Mod::~Mod() {
    this->unload();
}

Result<> Mod::loadDataStore() {
    auto dsPath = this->m_saveDirPath / "ds.json";
    if (!ghc::filesystem::exists(dsPath)) {
        this->m_dataStore = this->m_info.m_defaultDataStore;
    } else {
        auto dsData = file_utils::readString(dsPath);
        if (!dsData) return dsData;

        this->m_dataStore = nlohmann::json::parse(dsData.value());
    }
    return Ok<>();
}

Result<> Mod::saveDataStore() {
    auto dsPath = this->m_saveDirPath / "ds.json";
    return file_utils::writeString(dsPath, m_dataStore.dump(4));
}

DataStore Mod::getDataStore() {
    return DataStore(this, m_dataStore);
}

void Mod::postDSUpdate() {
    EventCenter::get()->send(Event(
        "datastore-changed",
        this
    ), this);
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

    if (!m_info.m_supportsUnloading) {
        return Err<>("Mod does not support unloading");
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
    if (!m_info.m_supportsDisabling) {
        return Err<>("Mod does not support disabling");
    }

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

Result<> Mod::uninstall() {
    if (m_info.m_supportsDisabling) {
        auto r = this->disable();
        if (!r) return r;
        if (m_info.m_supportsUnloading) {
            auto ur = this->unload();
            if (!ur) return ur;
        }
    }
    if (!ghc::filesystem::remove(m_info.m_path)) {
        return Err<>(
            "Unable to delete mod's .geode file! "
            "This might be due to insufficient permissions - "
            "try running GD as administrator."
        );
    }
    return Ok<>();
}

bool Mod::isUninstalled() const {
    return this != InternalMod::get() && !ghc::filesystem::exists(m_info.m_path);
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

ModInfo Mod::getModInfo() const {
    return m_info;
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

bool Mod::supportsUnloading() const {
    return this->m_info.m_supportsUnloading;
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

ghc::filesystem::path Mod::getHotReloadPath() const {
    return this->m_hotReloadPath;
}

Result<> Mod::enableHotReload() {
    if (this->m_hotReloadPath.empty()) {
        return Err<>("Mod does not have a hot reload path set");
    }
    return InternalLoader::get()->enableHotReload(this, this->m_hotReloadPath);
}

void Mod::disableHotReload() {
    return InternalLoader::get()->disableHotReload(this);
}

bool Mod::isHotReloadEnabled() const {
    return InternalLoader::get()->isHotReloadEnabled(const_cast<Mod*>(this));
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

struct json_check_failure : public std::exception {
    std::string m_info;
    const char* what() const throw() {
        return m_info.c_str();
    }
    json_check_failure(std::string const& info) : m_info(info) {}
};

struct json_check {
    nlohmann::json m_json;
    bool m_continue = true;
    bool m_branched = false;
    bool m_hasBranch= false;
    bool m_required = false;
    std::string m_key = "";
    std::string m_hierarchy = "";
    std::string m_types = "";
    static std::set<std::string_view> s_knownKeys;

    nlohmann::json const& get_json() {
        return m_key.size() ? m_json[m_key] : m_json;
    }

    json_check(nlohmann::json const& json) : m_json(json) {}

    json_check& has(std::string_view const& key) {
        if (!m_continue) return *this;
        s_knownKeys.insert(key);
        m_continue = m_json.contains(key);
        m_key = key;
        return *this;
    }
    json_check& needs(std::string_view const& key) {
        if (!m_continue) return *this;
        if (!m_json.contains(key))
            throw json_check_failure(
                "[mod.json]" + m_hierarchy + " is missing required key \"" + std::string(key) + "\""
            );
        s_knownKeys.insert(key);
        m_key = key;
        m_required = true;
        return *this;
    }
    template<typename T>
    json_check& as() {
        if (!m_continue) return *this;
        auto json = get_json();
        auto keyName = m_key.size() ? "[self]" : m_key;
        if constexpr (std::is_same_v<T, std::string>) {
            m_types += "string, ";
            if (!json.is_string()) {
                if (m_required) {
                    throw json_check_failure("[mod.json]" + m_hierarchy + "." + keyName + " is not a string");
                } else {
                    if (json.is_null()) {
                        m_continue = false;
                    } else {
                        throw json_check_failure("[mod.json]" + m_hierarchy + "." + keyName + " is not a string nor null");
                    }
                }
            }
        } else
        if constexpr (std::is_same_v<T, int>) {
            m_types += "int, ";
            if (!json.is_number_integer()) {
                if (m_required) {
                    throw json_check_failure("[mod.json]" + m_hierarchy + "." + keyName + " is not a int");
                } else {
                    if (json.is_null()) {
                        m_continue = false;
                    } else {
                        throw json_check_failure("[mod.json]" + m_hierarchy + "." + keyName + " is not a int nor null");
                    }
                }
            }
        } else
        if constexpr (std::is_same_v<T, bool>) {
            m_types += "bool, ";
            if (!json.is_boolean()) {
                if (m_required) {
                    throw json_check_failure("[mod.json]" + m_hierarchy + "." + keyName + " is not a boolean");
                } else {
                    if (json.is_null()) {
                        m_continue = false;
                    } else {
                        throw json_check_failure("[mod.json]" + m_hierarchy + "." + keyName + " is not a boolean nor null");
                    }
                }
            }
        } else
        if constexpr (std::is_same_v<T, nlohmann::json::object_t>) {
            m_types += "object, ";
            if (!json.is_object()) {
                if (m_required) {
                    throw json_check_failure("[mod.json]" + m_hierarchy + "." + keyName + " is not a object");
                } else {
                    if (json.is_null()) {
                        m_continue = false;
                    } else {
                        throw json_check_failure("[mod.json]" + m_hierarchy + "." + keyName + " is not a object nor null");
                    }
                }
            }
        } else
        if constexpr (std::is_same_v<T, nlohmann::json::array_t>) {
            m_types += "array, ";
            if (!json.is_array()) {
                if (m_required) {
                    throw json_check_failure("[mod.json]" + m_hierarchy + "." + keyName + " is not a array");
                } else {
                    if (json.is_null()) {
                        m_continue = false;
                    } else {
                        throw json_check_failure("[mod.json]" + m_hierarchy + "." + keyName + " is not a array nor null");
                    }
                }
            }
        } else
            static_assert(!std::is_same_v<T, T>, "Unimplemented type for json_check");
        return *this;
    }
    template<typename T>
    json_check& is(std::function<void(nlohmann::json const&)> branch) {
        if (!m_continue) return *this;
        m_hasBranch = true;
        try { this->as<T>(); } catch(json_check_failure&) { return *this; }
        branch(get_json());
        m_continue = false;
        m_branched = true;
        return *this;
    }
    json_check& is_ok() {
        if (m_hasBranch && !m_branched) {
            throw json_check_failure("[mod.json]" + m_hierarchy + "." + m_key + " is not one of " + m_types.substr(0, m_types.size() - 2));
        }
        return *this;
    }
    json_check& validate(std::function<bool(nlohmann::json const&)> predicate) {
        if (!m_continue) return *this;
        if (!predicate(get_json())) {
            throw json_check_failure("[mod.json]" + m_hierarchy + "." + m_key + " is invalidly formatted");
        }
        return *this;
    }
    template<typename T>
    json_check& into(T& var) {
        if (!m_continue) return *this;
        var = get_json().get<T>();
        return *this;
    }
    template<typename T>
    json_check& into_if(T& var) {
        return this->is<T>([&var](nlohmann::json const& json) -> void { json_check(json).as<T>().into(var); });
    }
    template<typename T>
    json_check& into_as(T& var) {
        return this->as<T>().into(var);
    }
    json_check& into(std::function<void(nlohmann::json const&)> var) {
        if (!m_continue) return *this;
        var(get_json());
        return *this;
    }
    json_check& each(std::function<void(std::string const&, json_check)> func) {
        if (!m_continue) return *this;
        for (auto const& [key, val] : get_json().items()) {
            auto c = json_check(*this);
            c.step();
            c.m_key = key;
            c.step();
            func(key, c);
        }
        return *this;
    }
    json_check& each(std::function<void(json_check)> func) {
        if (!m_continue) return *this;
        size_t ix = 0;
        for (auto const& val : get_json()) {
            auto c = json_check(val);
            c.m_hierarchy = m_hierarchy;
            if (m_key.size()) c.m_hierarchy += "." + m_key;
            c.m_hierarchy += "." + std::to_string(ix);
            func(c);
            ix++;
        }
        return *this;
    }
    json_check& step() {
        if (!m_continue) return *this;
        if (m_key.size()) {
            this->m_hierarchy += "." + m_key;
            this->m_json = get_json();
            m_key = "";
        }
        return *this;
    }

    template<typename T>
    T get() {
        if (!m_continue) return T();
        return this->get_json().get<T>();
    }
};

std::set<std::string_view> json_check::s_knownKeys = {};

template<typename T>
struct json_assign_required : json_check {
    json_assign_required(nlohmann::json& json, std::string_view const& key, T& var) : json_check(json) {
        this->needs(key).template as<T>().into(var);
    }
};

template<typename T>
struct json_assign_optional : json_check {
    json_assign_optional(nlohmann::json& json, std::string_view const& key, T& var) : json_check(json) {
        this->has(key).template as<T>().into(var);
    }
};

struct json_check_unknown {
    json_check_unknown(nlohmann::json& json, std::string const& hierarchy) {
        for (auto& [key, _] : json.items()) {
            if (!json_check::s_knownKeys.count(key)) {
                // throw json_check_failure(
                //     "[mod.json]" + hierarchy + " contains unknown key \"" + std::string(key) + "\""
                // );
                InternalMod::get()->log() << Severity::Warning
                    << "[mod.json]" << hierarchy
                    << " contains unknown key \"" << key << "\"";
                // todo: log as warning for mod
            }
        }
    }
};

std::string sanitizeDetailsData(unsigned char* start, unsigned char* end) {
    // delete CRLF
    return string_utils::replace(std::string(start, end), "\r", "");
}

template<>
Result<ModInfo> ModInfo::createFromSchema<1>(std::string const& path, nlohmann::json& json, ZipFile& unzip) {
    ModInfo info;

    info.m_path = path;

    try {

    json_check(json)
        .needs("id")
        .as<std::string>()
        .validate([](auto t) -> bool { return Mod::validateID(t.template get<std::string>()); })
        .into(info.m_id);

    json_check(json)
        .needs("version")
        .as<std::string>()
        .validate([](auto t) -> bool { return VersionInfo::validate(t.template get<std::string>()); })
        .into([&info](auto json) -> void { info.m_version = VersionInfo(json.template get<std::string>()); });

    json_assign_required(json, "name", info.m_name);
    json_assign_required(json, "developer", info.m_developer);
    json_assign_optional(json, "description", info.m_description);
    json_assign_optional(json, "repository", info.m_repository);

    json_check(json)
        .has("dependencies")
        .as<nlohmann::json::array_t>()
        .each([&info](json_check dep) -> void {
            dep.as<nlohmann::json::object_t>();
            auto depobj = Dependency {};
            json_check(dep).needs("id").as<std::string>().into(depobj.m_id);
            json_check(dep)
                .has("version")
                .as<std::string>()
                .validate([&](auto t) -> bool { return VersionInfo::validate(t.template get<std::string>()); })
                .into([&info](auto json) -> void { info.m_version = VersionInfo(json.template get<std::string>()); });
            json_check(dep).has("required").as<bool>().into(depobj.m_required);
            json_check_unknown(dep.m_json, dep.m_hierarchy);
            info.m_dependencies.push_back(depobj);
        });
    
    json_check(json)
        .has("datastore")
        .as<nlohmann::json::object_t>()
        .into(info.m_defaultDataStore);
    
    json_check(json)
        .has("resources")
        .as<nlohmann::json::object_t>()
        .step()
        .has("spritesheets")
        .as<nlohmann::json::object_t>()
        .each([&info](auto key, auto) -> void {
            info.m_spritesheets.push_back(info.m_id + "_" + key);
        });
    
    json_assign_optional(json, "toggleable", info.m_supportsDisabling);
    json_assign_optional(json, "unloadable", info.m_supportsUnloading);

    json_check::s_knownKeys.insert("geode");
    json_check::s_knownKeys.insert("binary");
    json_check::s_knownKeys.insert("userdata");
    json_check_unknown(json, "");

    } catch(std::exception& e) {
        return Err<>(e.what());
    }

    if (json.contains("binary")) {
        bool autoEnd = true;
        if (json["binary"].is_string()) {
            info.m_binaryName = json["binary"];
        } else if (json["binary"].is_object()) {
            auto bo = json["binary"];
            if (bo.contains("*") && bo["*"].is_string()) {
                info.m_binaryName = bo["*"];
            }
            if (bo.contains("auto") && bo["auto"].is_boolean()) {
                autoEnd = bo["auto"];
            }
            #if defined(GEODE_IS_WINDOWS)
            if (bo.contains("windows") && bo["windows"].is_string()) {
                info.m_binaryName = bo["windows"];
            }
            #elif defined(GEODE_IS_MACOS)
            if (bo.contains("macos") && bo["macos"].is_string()) {
                info.m_binaryName = bo["macos"];
            }
            #elif defined(GEODE_IS_ANDROID)
            if (bo.contains("android") && bo["android"].is_string()) {
                info.m_binaryName = bo["android"];
            }
            #elif defined(GEODE_IS_IOS)
            if (bo.contains("ios") && bo["ios"].is_string()) {
                info.m_binaryName = bo["ios"];
            }
            #endif
        } else goto skip_binary_check;
        if (autoEnd && !string_utils::endsWith(info.m_binaryName, GEODE_PLATFORM_EXTENSION)) {
            info.m_binaryName += GEODE_PLATFORM_EXTENSION;
        }
    }
    skip_binary_check:

    if (unzip.fileExists("about.md")) {
        // we can just reuse this variable
        unsigned long readSize = 0;
        auto aboutData = unzip.getFileData("about.md", &readSize);
        if (!aboutData || !readSize) {
            return Err<>("Unable to read \"" + path + "\"/about.md");
        } else {
            info.m_details = sanitizeDetailsData(aboutData, aboutData + readSize);
        }
    }

    return Ok(info);
}

Result<ModInfo> ModInfo::createFromFile(std::string const& path) {
    ZipFile unzip(path);
    if (!unzip.isLoaded()) {
        return Err<>("\"" + path + "\": Unable to unzip");
    }
    // Check if mod.json exists in zip
    if (!unzip.fileExists("mod.json")) {
        return Err<>("\"" + path + "\" is missing mod.json");
    }
    // Read mod.json & parse if possible
    unsigned long readSize = 0;
    auto read = unzip.getFileData("mod.json", &readSize);
    if (!read || !readSize) {
        return Err<>("\"" + path + "\": Unable to read mod.json");
    }

    nlohmann::json json;
    try {
        json = nlohmann::json::parse(std::string(read, read + readSize));
    } catch(std::exception const& e) {
        return Err<>(e.what());
    }

    delete[] read;

    if (!json.is_object()) {
        return Err<>(
            "\"" + path + "/mod.json\" does not have an "
            "object at root despite expected"
        );
    }

    // Check mod.json target version
    auto schema = 1;
    if (json.contains("geode") && json["geode"].is_number_integer()) {
        schema = json["geode"];
    }
    if (schema < Loader::s_supportedSchemaMin) {
        return Err<>(
            "\"" + path + "\" has a lower target version (" + 
            std::to_string(schema) + ") than this version of "
            "geode supports (" + std::to_string(Loader::s_supportedSchemaMin) +
            "). You may need to downgrade geode in order to use "
            "this mod."
        );
    }
    if (schema > Loader::s_supportedSchemaMax) {
        return Err<>(
            "\"" + path + "\" has a higher target version (" + 
            std::to_string(schema) + ") than this version of "
            "geode supports (" + std::to_string(Loader::s_supportedSchemaMax) +
            "). You may need to upgrade geode in order to use "
            "this mod."
        );
    }

    // Handle mod.json data based on target
    switch (schema) {
        case 1: {
            return ModInfo::createFromSchema<1>(path, json, unzip);
        } break;
    }



    return Err<>(
        "\"" + path + "\" has a version schema (" +
        std::to_string(schema) + ") that isn't "
        "supported by this version of geode. "
        "This may be a bug, or the given version "
        "schema is invalid."
    );
}
