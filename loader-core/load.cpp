#include <Loader.hpp>
#include <Mod.hpp>
#include <InternalMod.hpp>
#include <Log.hpp>
#undef snprintf
#include <json.hpp>
#include <ZipUtils.h>
#include <helpers/general.hpp>
#include <helpers/string.hpp>

USE_LILAC_NAMESPACE();

#define JSON_ASSIGN_IF_CONTAINS_AND_TYPE_FROM(_name_, _from_, _type_)\
    if (json.contains(#_from_) && !json[#_from_].is_null()) {   \
        if (json[#_from_].is_##_type_()) {                      \
            info.m_##_name_ = json[#_from_];                    \
        }                                                       \
    }

#define JSON_ASSIGN_IF_CONTAINS_AND_TYPE(_name_, _type_)        \
    if (json.contains(#_name_) && !json[#_name_].is_null()) {   \
        if (json[#_name_].is_##_type_()) {                      \
            info.m_##_name_ = json[#_name_];                    \
        } else {                                                \
            InternalMod::get()->throwError(                     \
                strfmt("\"%s\": \"" #_name_ "\" is not of expected type \"" #_type_ "\"", info.m_id.c_str()),   \
                Severity::Warning                                                                       \
            );                                                  \
        }                                                       \
    }

#define JSON_ASSIGN_IF_CONTAINS_AND_TYPE_NO_NULL(_name_, _type_)\
    if (json.contains(#_name_)) {                               \
        if (json[#_name_].is_null()) {                          \
            return Err<>(                                       \
                "\"" + path + "\": \"" #_name_ "\" is not "     \
                "of expected type -- expected \""               \
                #_type_ "\", got " +                            \
                json[#_name_].type_name()                       \
            );                                                  \
        }                                                       \
        if (json[#_name_].is_##_type_()) {                      \
            info.m_##_name_ = json[#_name_];                    \
        }                                                       \
    }

#define JSON_ASSIGN_IF_CONTAINS_AND_TYPE_REQUIRED(_name_, _type_)\
    if (json.contains(#_name_)) {                               \
        if (!json[#_name_].is_##_type_()) {                     \
            return Err<>(                                       \
                "\"" + path + "\": \"" #_name_ "\" is not "     \
                "of expected type -- expected \""               \
                #_type_ "\", got " +                            \
                json[#_name_].type_name()                       \
            );                                                  \
        }                                                       \
        info.m_##_name_ = json[#_name_];                        \
    } else {                                                    \
        return Err<>(                                           \
            "\"" + path + "\": Missing required field \""       \
            #_name_ "\""                                        \
        );                                                      \
    }

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
    try {
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
        if (json.contains("lilac") && json["lilac"].is_number_integer()) {
            schema = json["lilac"];
        }
        if (schema < Loader::s_supportedSchemaMin) {
            return Err<>(
                "\"" + path + "\" has a lower target version (" + 
                std::to_string(schema) + ") than this version of "
                "lilac supports (" + std::to_string(Loader::s_supportedSchemaMin) +
                "). You may need to downdate lilac in order to use "
                "this mod."
            );
        }
        if (schema > Loader::s_supportedSchemaMax) {
            return Err<>(
                "\"" + path + "\" has a higher target version (" + 
                std::to_string(schema) + ") than this version of "
                "lilac supports (" + std::to_string(Loader::s_supportedSchemaMax) +
                "). You may need to update lilac in order to use "
                "this mod."
            );
        }
        
        // Handle mod.json data based on target
        switch (schema) {
            case 1: {
                return this->checkBySchema<1>(path, &json);
            }
        }

        return Err<>(
            "\"" + path + "\" has a version schema (" +
            std::to_string(schema) + ") that isn't "
            "supported by this version of lilac. "
            "This may be a bug, or the given version "
            "schema is invalid."
        );

    } catch(nlohmann::json::exception const& e) {
        return Err<>("\"" + path + "\": Unable to parse mod.json - \"" + e.what() + "\"");
    } catch(...) {
        return Err<>("\"" + path + "\": Unable to parse mod.json - Unknown Error");
    }
}

template<>
Result<Mod*> Loader::checkBySchema<1>(std::string const& path, void* jsonData) {
    nlohmann::json json = *reinterpret_cast<nlohmann::json*>(jsonData);
    if (!json.contains("id")) {
        return Err<>("\"" + path + "\" lacks a Mod ID");
    }

    if (
        !json.contains("version") ||
        !json["version"].is_string() ||
        !VersionInfo::validate(json["version"])
    ) {
        return Err<>(
            "\"" + path + "\" is either lacking a version field, "
            "or its value is incorrectly formatted (should be "
            "\"vX.X.X\")"
        );
    }

    ModInfo info;

    info.m_path    = path;
    info.m_version = VersionInfo(json["version"]);

    if (json.contains("id")) {
        if (!json["id"].is_string()) {
            return Err<>(
                "\"" + path + "\": \"id\" is not "
                "of expected type -- expected \"string\", "
                "got " + json["id"].type_name()
            );
        }
        if (Mod::validateID(json["id"])) {
            info.m_id = json["id"];
        } else {
            return Err<>( "\"" + path + "\": \"id\" is invalid (may only contain `a-z`, `A-Z`, `-`, `_` and `.`)" );
        }
    } else {
        return Err<>( "\"" + path + "\": Missing required field \"id\"" );
    }

    JSON_ASSIGN_IF_CONTAINS_AND_TYPE_REQUIRED(name, string);
    JSON_ASSIGN_IF_CONTAINS_AND_TYPE_REQUIRED(developer, string);
    JSON_ASSIGN_IF_CONTAINS_AND_TYPE(description, string);
    JSON_ASSIGN_IF_CONTAINS_AND_TYPE(details, string);
    JSON_ASSIGN_IF_CONTAINS_AND_TYPE(credits, string);

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
            #if defined(LILAC_IS_WINDOWS)
            if (bo.contains("windows") && bo["windows"].is_string()) {
                info.m_binaryName = bo["windows"];
            }
            #elif defined(LILAC_IS_MACOS)
            if (bo.contains("macos") && bo["macos"].is_string()) {
                info.m_binaryName = bo["macos"];
            }
            #elif defined(LILAC_IS_ANDROID)
            if (bo.contains("android") && bo["android"].is_string()) {
                info.m_binaryName = bo["android"];
            }
            #elif defined(LILAC_IS_IOS)
            if (bo.contains("ios") && bo["ios"].is_string()) {
                info.m_binaryName = bo["ios"];
            }
            #endif
        } else goto skip_binary_check;
        if (autoEnd && !string_utils::endsWith(info.m_binaryName, LILAC_PLATFORM_EXTENSION)) {
            info.m_binaryName += LILAC_PLATFORM_EXTENSION;
        }
    }
skip_binary_check:

    if (json.contains("dependencies")) {
        auto deps = json["dependencies"];
        if (deps.is_array()) {
            size_t ix = 0;
            for (auto const& dep : deps) {
                if (dep.is_object()) {
                    if (dep.contains("id") && dep["id"].is_string()) {
                        auto depobj = Dependency {};
                        depobj.m_id = dep["id"];
                        if (dep.contains("version")) {
                            depobj.m_version = VersionInfo(dep["version"]);
                        }
                        if (dep.contains("required")) {
                            depobj.m_required = dep["required"];
                        }
                        depobj.m_mod = this->getLoadedMod(depobj.m_id);
                        info.m_dependencies.push_back(depobj);
                    } else {
                        InternalMod::get()->throwError(
                            strfmt("\"%s\": Item #%d in \"dependencies\" array lacks ID", info.m_id.c_str(), ix),
                            Severity::Warning
                        );
                    }
                } else {
                    InternalMod::get()->throwError(
                        strfmt("\"%s\": Item #%d in \"dependencies\" array is not an object", info.m_id.c_str(), ix),
                        Severity::Warning
                    );
                }
                ix++;
            }
        } else if (!deps.is_null()) {
            InternalMod::get()->throwError(
                strfmt("\"%s\": \"dependencies\" is not an array", info.m_id.c_str()),
                Severity::Warning
            );
        }
    }

    if (json.contains("settings")) {
        auto settings = json["settings"];
        if (settings.is_object()) {
            for (auto const& [key, value] : settings.items()) {
                auto res = Setting::parseFromJSON(value);
                if (res) {
                    res.value()->m_key = key;
                    info.m_settings.insert({ key, res.value() });
                } else {
                    InternalMod::get()->throwError(
                        "Error parsing setting \"" + key + "\": " + res.error(),
                        Severity::Warning
                    );
                }
            }
        } else if (!settings.is_null()) {
            InternalMod::get()->throwError(
                strfmt("\"%s\": \"settings\" is not an object", info.m_id.c_str()),
                Severity::Warning
            );
        }
    }

    auto mod = new Mod(info);
    mod->m_enabled = true;
    this->m_mods.insert({ info.m_id, mod });
    mod->updateDependencyStates();

    return Ok<Mod*>(mod);
}
