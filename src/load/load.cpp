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

Result<Mod*> Loader::loadModFromFile(std::string const& path) {
    auto res = ModInfo::createFromFile(path);
    if (!res) {
        return Err<>(res.error());
    }
    
    auto mod = new Mod(res.value());
    mod->m_saveDirPath = Loader::get()->getGeodeSaveDirectory() / geodeModDirectory / res.value().m_id;
    mod->loadDataStore();
    ghc::filesystem::create_directories(mod->m_saveDirPath);

    mod->m_enabled = Loader::get()->shouldLoadMod(mod->m_info.m_id);
    this->m_mods.insert({ res.value().m_id, mod });
    mod->updateDependencyStates();

    return Ok<>(mod);
    /*} catch(nlohmann::json::exception const& e) {
        return Err<>("\"" + path + "\": Unable to parse mod.json - \"" + e.what() + "\"");
    } catch(std::exception const& e) {
	    return Err<>("\"" + path + "\": Unable to open mod.json - \"" + e.what() + "\"");
	} catch(...) {
        return Err<>("\"" + path + "\": Unable to open mod.json - Unknown Error");
    }*/
}
