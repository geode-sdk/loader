#include <Loader.hpp>
#include <Mod.hpp>
#include <InternalMod.hpp>
#include <Log.hpp>
#undef snprintf
#include <utils/json.hpp>
#include <ZipUtils.h>
#include <utils/general.hpp>
#include <utils/string.hpp>
#include <Geode.hpp>

USE_GEODE_NAMESPACE();

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
            InternalMod::get()->logInfo(                     \
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
                return this->checkBySchema<1>(path, &json);
            }
        }

        return Err<>(
            "\"" + path + "\" has a version schema (" +
            std::to_string(schema) + ") that isn't "
            "supported by this version of geode. "
            "This may be a bug, or the given version "
            "schema is invalid."
        );

    } catch(nlohmann::json::exception const& e) {
        return Err<>("\"" + path + "\": Unable to parse mod.json - \"" + e.what() + "\"");
    } catch(...) {
        return Err<>("\"" + path + "\": Unable to parse mod.json - Unknown Error");
    }
}

std::string Credits::Person::expandKnownLink(std::string const& link) {
    switch (hash(string_utils::toLower(link).c_str())) {
        case hash("github"):
            if (!string_utils::contains(link, "/")) {
                return "https://github.com/" + link;
            } break;

        case hash("youtube"):
            if (!string_utils::contains(link, "/")) {
                return "https://youtube.com/channel/" + link;
            } break;

        case hash("twitter"):
            if (!string_utils::contains(link, "/")) {
                return "https://twitter.com/" + link;
            } break;

        case hash("newgrounds"):
            if (!string_utils::contains(link, "/")) {
                return "https://" + link + ".newgrounds.com";
            } break;
    }
    return link;
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
    JSON_ASSIGN_IF_CONTAINS_AND_TYPE(repository, string);

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

    if (json.contains("credits")) {
        if (json["credits"].is_string()) {
            info.m_creditsString = json["credits"];
        } else if (json["credits"].is_object()) {
            
            auto credits = json["credits"];

            if (credits.contains("people")) {
                if (credits["people"].is_object()) {
                    
                    for (auto const& [key, value] : credits["people"].items()) {
                        auto credit = Credits::Person();
                        credit.m_name = key;

                        if (value.is_object()) {

                            if (value.contains("gd")) {
                                if (value["gd"].is_string()) {
                                    credit.m_gdAccountName = value["gd"];
                                } else if (value["gd"].is_number()) {
                                    credit.m_gdAccountID = value["gd"];
                                } else {
                                    return Err<>("\"" + path + "\": \"credits.people." + credit.m_name + ".gd\" is not a string nor a number");
                                }
                            }

                            if (value.contains("role")) {
                                if (value.contains("reason")) {
                                    InternalMod::get()->logInfo(
                                        "\"credits.people." + credit.m_name + "\" contains both \"role\" and \"reason\"",
                                        Severity::Warning
                                    );
                                }
                                if (value["role"].is_string()) {
                                    credit.m_reason = value["role"];
                                } else {
                                    return Err<>("\"" + path + "\": \"credits.people." + credit.m_name + ".role\" is not a string");
                                }
                            }

                            if (value.contains("reason")) {
                                if (value["reason"].is_string()) {
                                    credit.m_reason = value["reason"];
                                } else {
                                    return Err<>("\"" + path + "\": \"credits.people." + credit.m_name + ".reason\" is not a string");
                                }
                            }

                            if (value.contains("links")) {
                                if (value["links"].is_object()) {
                                    for (auto const& [site, url] : value["links"].items()) {
                                        if (url.is_string()) {
                                            credit.m_links[string_utils::toLower(site)] = Credits::Person::expandKnownLink(url);
                                        } else {
                                            return Err<>("\"" + path + "\": \"credits.people." + credit.m_name + ".links." + site + "\" is not a string");
                                        }
                                    }
                                } else {
                                    return Err<>("\"" + path + "\": \"credits.people." + credit.m_name + ".links\" is not an object");
                                }
                            }

                        } else {
                            return Err<>("\"" + path + "\": \"credits.people." + credit.m_name + "\" is not an object");
                        }

                        info.m_credits.m_people.push_back(credit);
                    }

                } else if (credits["people"].is_null()) {
                    return Err<>("\"" + path + "\": \"credits.people\" is not an object");
                }
            }

            if (credits.contains("thanks")) {
                if (credits["thanks"].is_array()) {

                    for (auto const& item : credits["thanks"]) {
                        if (item.is_string()) {
                            info.m_credits.m_thanks.push_back(item);
                        } else {
                            return Err<>("\"" + path + "\": \"credits.thanks\" contains a non-string item");
                        }
                    }

                } else if (credits["thanks"].is_null()) {
                    return Err<>("\"" + path + "\": \"credits.thanks\" is not an array");
                }
            }

            if (credits.contains("libraries")) {
                if (credits["libraries"].is_object()) {

                    for (auto const& [name, url] : credits["libraries"].items()) {
                        if (url.is_string()) {
                            info.m_credits.m_libraries.push_back({ name, url });
                        } else {
                            return Err<>("\"" + path + "\": \"credits.libraries." + name + "\" is not a string");
                        }
                    }

                } else if (credits["libraries"].is_null()) {
                    return Err<>("\"" + path + "\": \"credits.libraries\" is not an object");
                }
            }

        } else if (!json["credits"].is_null()) {
            return Err<>("\"" + path + "\": \"credits\" is not an object, string nor null");
        }
    }

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
                        InternalMod::get()->logInfo(
                            strfmt("\"%s\": Item #%d in \"dependencies\" array lacks ID", info.m_id.c_str(), ix),
                            Severity::Warning
                        );
                    }
                } else {
                    InternalMod::get()->logInfo(
                        strfmt("\"%s\": Item #%d in \"dependencies\" array is not an object", info.m_id.c_str(), ix),
                        Severity::Warning
                    );
                }
                ix++;
            }
        } else if (!deps.is_null()) {
            InternalMod::get()->logInfo(
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
                    InternalMod::get()->logInfo(
                        "Error parsing setting \"" + key + "\": " + res.error(),
                        Severity::Warning
                    );
                }
            }
        } else if (!settings.is_null()) {
            InternalMod::get()->logInfo(
                strfmt("\"%s\": \"settings\" is not an object", info.m_id.c_str()),
                Severity::Warning
            );
        }
    }

    if (json.contains("resources")) {
        auto json_resources = json["resources"];
        if (json_resources.is_object()) {

            if (json_resources.contains("spritesheets")) {
                if (json_resources["spritesheets"].is_object()) {
                    for (auto const& [key, _] : json_resources["spritesheets"].items()) {
                        info.m_spritesheets.push_back(key);
                    }
                } else if (!json_resources["spritesheets"].is_null()) {
                    InternalMod::get()->logInfo(
                        strfmt("\"%s\": \"resources.spritesheets\" is not an object", info.m_id.c_str()),
                        Severity::Warning
                    );
                }
            }

        } else if (!json_resources.is_null()) {
            InternalMod::get()->logInfo(
                strfmt("\"%s\": \"resources\" is not an object", info.m_id.c_str()),
                Severity::Warning
            );
        }
    }

    // this was not meant to be available to the end user but ok
    if (json.contains("toggleable") && json["toggleable"].is_boolean()) {
        info.m_supportsDisabling = json["toggleable"];
    }

    auto mod = new Mod(info);

    mod->m_saveDirPath = CCFileUtils::sharedFileUtils()->getWritablePath();
    mod->m_saveDirPath /= "geode/mods";
    mod->m_saveDirPath /= info.m_id;

    ghc::filesystem::create_directories(mod->m_saveDirPath);

    auto r = mod->loadData();
    if (!r) mod->logInfo(r.error(), Severity::Error);

    mod->m_enabled = true;
    this->m_mods.insert({ info.m_id, mod });
    mod->updateDependencyStates();

    return Ok<Mod*>(mod);
}
