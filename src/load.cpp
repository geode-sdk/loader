#include <loader/Loader.hpp>
#include <loader/Mod.hpp>
#include <InternalMod.hpp>
#include <loader/Log.hpp>
#undef snprintf
#include <utils/json.hpp>
#include <ZipUtils.h>
#include <utils/general.hpp>
#include <utils/string.hpp>
#include <Geode.hpp>

USE_GEODE_NAMESPACE();
using namespace std::string_literals;

bool Mod::validateID(std::string const& id) {
    if (!id.size()) return false;
    for (auto const& c : id) {
        if (!(
            ('a' <= c && c <= 'z') ||
            ('A' <= c && c <= 'Z') ||
            (c == '-') ||
            (c == '_') ||
            (c == '.')
        )) return false;
    }
    return true;
}

std::string sanitizeDetailsData(unsigned char* start, unsigned char* end) {
    // delete CRLF
    return string_utils::replace(std::string(start, end), "\r", "");
}

template<> Result<Mod*> Loader::checkBySchema<1>(std::string const& path, void* jsonData);

Result<Mod*> Loader::loadModFromFile(std::string const& path) {
    // Unzip file
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
    //try {
        json = nlohmann::json::parse(std::string(read, read + readSize));

        // Free up memory
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
                "). You may need to downdate geode in order to use "
                "this mod."
            );
        }
        if (schema > Loader::s_supportedSchemaMax) {
            return Err<>(
                "\"" + path + "\" has a higher target version (" + 
                std::to_string(schema) + ") than this version of "
                "geode supports (" + std::to_string(Loader::s_supportedSchemaMax) +
                "). You may need to update geode in order to use "
                "this mod."
            );
        }

        // Handle mod.json data based on target
        switch (schema) {
            case 1: {
                auto res = this->checkBySchema<1>(path, &json);
                if (res && res.value() && unzip.fileExists("about.md")) {
                    // we can just reuse this variable
                    readSize = 0;
                    auto aboutData = unzip.getFileData("about.md", &readSize);
                    if (!aboutData || !readSize) {
                        return Err<>("\"" + path + "\": Unable to read about.md");
                    }
                    res.value()->m_info.m_details = sanitizeDetailsData(aboutData, aboutData + readSize);
                }
                return res;
            } break;
        }

        return Err<>(
            "\"" + path + "\" has a version schema (" +
            std::to_string(schema) + ") that isn't "
            "supported by this version of geode. "
            "This may be a bug, or the given version "
            "schema is invalid."
        );

    /*} catch(nlohmann::json::exception const& e) {
        return Err<>("\"" + path + "\": Unable to parse mod.json - \"" + e.what() + "\"");
    } catch(std::exception const& e) {
	    return Err<>("\"" + path + "\": Unable to open mod.json - \"" + e.what() + "\"");
	} catch(...) {
        return Err<>("\"" + path + "\": Unable to open mod.json - Unknown Error");
    }*/
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

template<>
Result<Mod*> Loader::checkBySchema<1>(std::string const& path, void* jsonData) {
    nlohmann::json json = *reinterpret_cast<nlohmann::json*>(jsonData);

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
        .has("settings")
        .as<nlohmann::json::object_t>()
        .each([&info](auto key, auto value) -> void {
            auto res = Setting::parseFromJSON(value.get_json());
            if (res) {
                res.value()->m_key = key;
                info.m_settings.insert({ key, res.value() });
            } else {
                throw json_check_failure("Error parsing setting \"" + key + "\": " + res.error());
            }
        });
    
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

    auto mod = new Mod(info);

    for (auto& [_, setting] : mod->m_info.m_settings) {
        setting->m_mod = mod;
    }

    mod->m_saveDirPath = Loader::get()->getGeodeSaveDirectory() / geodeModDirectory / info.m_id;

    ghc::filesystem::create_directories(mod->m_saveDirPath);

    auto r = mod->loadData();
    if (!r) mod->logInfo(r.error(), Severity::Error);

    mod->m_enabled = Loader::get()->shouldLoadMod(mod->m_info.m_id);
    this->m_mods.insert({ info.m_id, mod });
    mod->updateDependencyStates();

    return Ok<Mod*>(mod);
}
