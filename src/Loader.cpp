#include <Hook.hpp>
#include <Mod.hpp>
#include <Log.hpp>
#include <Loader.hpp>
#include <Internal.hpp>
#include <InternalMod.hpp>
#include <utils/file.hpp>
#include <utils/conststring.hpp>
#include <utils/vector.hpp>
#include <utils/map.hpp>
#include <utils/types.hpp>
#include <mutex>
#include <Geode.hpp>

USE_GEODE_NAMESPACE();

bool Loader::s_unloading = false;
std::mutex g_unloadMutex;

Loader* Loader::get() {
    static auto g_loader = new Loader;
    return g_loader;
}

void Loader::createDirectories() {
    constexpr auto api_dir = const_join_path_c_str<geode_directory, geode_api_mod_directory>;
    constexpr auto mod_dir = const_join_path_c_str<geode_directory, geode_mod_directory>;
    
    try {
        file_utils::createDirectory(const_join_path_c_str<geode_directory>);
        file_utils::createDirectory(const_join_path_c_str<geode_directory, geode_resource_directory>);
        file_utils::createDirectory(mod_dir);
        file_utils::createDirectory(api_dir);
        ghc::filesystem::remove_all(const_join_path_c_str<geode_directory, geode_temp_directory>);
    } catch(...) {}

    if (!vector_utils::contains<std::string>(this->m_modDirectories, api_dir)) {
        this->m_modDirectories.push_back(api_dir);
    }
    if (!vector_utils::contains<std::string>(this->m_modDirectories, mod_dir)) {
        this->m_modDirectories.push_back(mod_dir);
    }
}

void Loader::addModResourcesPath(Mod* mod) {
    if (mod->m_addResourcesToSearchPath) {
        CCFileUtils::sharedFileUtils()->addSearchPath(
            (mod->m_tempDirName / "resources").string().c_str()
        );
    }
}

void Loader::updateResourcePaths() {
    CCFileUtils::sharedFileUtils()->addSearchPath(const_join_path_c_str<geode_directory, geode_resource_directory>);
    for (auto const& [_, mod] : this->m_mods) {
        this->addModResourcesPath(mod);
    }
}

void Loader::updateModResources(Mod* mod) {
    for (auto const& sheet : mod->m_info.m_spritesheets) {
        auto png = sheet + ".png";
        auto plist = sheet + ".plist";
        if (
            png == std::string(CCFileUtils::sharedFileUtils()->fullPathForFilename(png.c_str(), false)) ||
            plist == std::string(CCFileUtils::sharedFileUtils()->fullPathForFilename(plist.c_str(), false))
        ) {
            InternalMod::get()->logInfo(
                "The resource dir of \"" + mod->m_info.m_id + "\" is missing \"" + 
                sheet + "\" png and/or plist files",
                Severity::Warning
            );
        } else {
            CCTextureCache::sharedTextureCache()->addImage(png.c_str(), false);
            CCSpriteFrameCache::sharedSpriteFrameCache()->addSpriteFramesWithFile(plist.c_str());
        }
    }
}

void Loader::updateResources() {
    for (auto const& [_, mod] : this->m_mods) {
        this->updateModResources(mod);
    }
}

size_t Loader::refreshMods() {
    InternalMod::get()->log()
        << Severity::Debug
        << "Loading mods..."
        << geode::endl;

    size_t loaded = 0;
    this->createDirectories();

    for (auto const& dir : this->m_modDirectories) {
        InternalMod::get()->log()
            << Severity::Debug
            << "Searching " << dir 
            << geode::endl;

        for (auto const& entry : ghc::filesystem::directory_iterator(dir)) {
            if (
                ghc::filesystem::is_regular_file(entry) &&
                entry.path().extension() == geode_mod_extension
            ) {
                InternalMod::get()->log()
                    << Severity::Debug
                    << "Loading " << entry.path().string()
                    << geode::endl;
                if (!map_utils::contains<std::string, Mod*>(
                    this->m_mods,
                    [entry](Mod* p) -> bool {
                        return p->m_info.m_path == entry.path();
                    }
                )) {
                    auto res = this->loadModFromFile(entry.path().string());
                    if (res && res.value()) {
                        if (!res.value()->hasUnresolvedDependencies()) {
                            loaded++;
                            InternalMod::get()->log()
                                << "Succesfully loaded " << res.value() << geode::endl;
                        } else {
                            InternalMod::get()->log()
                                << res.value() << " has unresolved dependencies" << geode::endl;
                        }
                    } else {
                        InternalMod::get()->logInfo(res.error(), Severity::Error);
                    }
                }
            }
        }
    }

    InternalMod::get()->log()
        << Severity::Debug
        << "Loaded " << loaded << " new mods"
        << geode::endl;
    return loaded;
}

bool Loader::isModLoaded(std::string const& id, bool resolved) const {
    if (this->m_mods.count(id)) {
        if (resolved && this->m_mods.at(id)->hasUnresolvedDependencies()) {
            return false;
        }
        return true;
    }
    return false;
}

Mod* Loader::getLoadedMod(std::string const& id, bool resolved) const {
    if (this->m_mods.count(id)) {
        auto mod = this->m_mods.at(id);
        if (resolved && mod->hasUnresolvedDependencies()) {
            return nullptr;
        }
        return mod;
    }
    return nullptr;
}

std::vector<Mod*> Loader::getLoadedMods(bool resolved) const {
    if (!resolved) {
        return map_utils::getValues(this->m_mods);
    }
    std::vector<Mod*> res;
    for (auto const& [_, val] : this->m_mods) {
        if (!val->hasUnresolvedDependencies()) {
            res.push_back(val);
        }
    }
    return res;
}

std::tuple<size_t, size_t> Loader::getLoadedModCount() const {
    auto count = 0u;
    for (auto const& [_, mod] : this->m_mods) {
        if (mod->hasUnresolvedDependencies()) count++;
    }
    return { this->m_mods.size(), count };
}

void Loader::updateAllDependencies() {
    for (auto const& [_, mod] : this->m_mods) {
        mod->updateDependencyStates();
    }
}

void Loader::unloadMod(Mod* mod) {
    this->m_mods.erase(mod->m_info.m_id);
    // ~Mod will call FreeLibrary 
    // automatically
    delete mod;
}

bool Loader::setup() {
    if (this->m_isSetup)
        return true;

    InternalMod::get()->log()
        << Severity::Debug
        << "Setting up Loader..."
        << geode::endl;

    this->createDirectories();
    this->refreshMods();

    this->m_isSetup = true;

    return true;
}

Loader::Loader() {
    this->m_logStream = new LogStream;
}

Loader::~Loader() {
    g_unloadMutex.lock();
    s_unloading = true;
    g_unloadMutex.unlock();
    for (auto const& [_, mod] : this->m_mods) {
        delete mod;
    }
    this->m_mods.clear();
    for (auto const& log : this->m_logs) {
        delete log;
    }
    this->m_logs.clear();
    delete this->m_logStream;
    ghc::filesystem::remove_all(const_join_path<geode_directory, geode_temp_directory>);
}

LogStream& Loader::logStream() {
    return *this->m_logStream;
}

void Loader::log(LogMessage* log) {
    this->m_logs.push_back(log);
}

void Loader::deleteLog(LogMessage* log) {
    vector_utils::erase(this->m_logs, log);
    delete log;
}

std::vector<LogMessage*> const& Loader::getLogs() const {
    return this->m_logs;
}

std::vector<LogMessage*> Loader::getLogs(
    std::initializer_list<Severity> severityFilter
) {
    if (!severityFilter.size()) {
        return this->m_logs;
    }

    std::vector<LogMessage*> logs;

    for (auto const& log : this->m_logs) {
        if (vector_utils::contains<Severity>(severityFilter, log->getSeverity())) {
            logs.push_back(log);
        }
    }

    return logs;
}

void Loader::queueInGDThread(std::function<void GEODE_CALL()> func) {
    Geode::get()->queueInGDThread(func);
}

Mod* Loader::getInternalMod() {
    return InternalMod::get();
}

bool Loader::isUnloading() {
    return Loader::s_unloading;
}

