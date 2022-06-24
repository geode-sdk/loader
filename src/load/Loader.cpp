#include <loader/Hook.hpp>
#include <loader/Mod.hpp>
#include <loader/Log.hpp>
#include <loader/Loader.hpp>
#include <InternalLoader.hpp>
#include <InternalMod.hpp>
#include <utils/file.hpp>
#include <utils/conststring.hpp>
#include <utils/vector.hpp>
#include <utils/map.hpp>
#include <utils/types.hpp>
#include <mutex>
#include <Geode.hpp>
#include <about.hpp>

USE_GEODE_NAMESPACE();

bool Loader::s_unloading = false;
std::mutex g_unloadMutex;

VersionInfo Loader::getVersion() const {
    return LOADER_VERSION;
}

std::string Loader::getVersionType() const {
    return LOADER_VERSION_TYPE;
}

Loader* Loader::get() {
    return InternalLoader::get();
}

void Loader::createDirectories() {
    auto modDir = this->getGeodeDirectory() / geodeModDirectory;
    auto logDir = this->getGeodeDirectory() / geodeLogDirectory;
    auto resDir = this->getGeodeDirectory() / geodeResourceDirectory;
    auto tempDir = this->getGeodeDirectory() / geodeTempDirectory;

    ghc::filesystem::create_directories(resDir);
    ghc::filesystem::create_directory(modDir);
    ghc::filesystem::create_directory(logDir);
    ghc::filesystem::create_directory(tempDir);

    if (!vector_utils::contains(this->m_modDirectories, modDir)) {
        this->m_modDirectories.push_back(modDir);
    }

    // files too
    this->m_logStream = std::ofstream(logDir / generateLogName());
}

void Loader::addModResourcesPath(Mod* mod) {
    if (mod->m_addResourcesToSearchPath) {
        CCFileUtils::sharedFileUtils()->addSearchPath(
            mod->m_tempDirName.string().c_str()
        );
        CCFileUtils::sharedFileUtils()->addSearchPath(
            (mod->m_tempDirName / "resources").string().c_str()
        );
    }
}

void Loader::updateResourcePaths() {
	auto resDir = this->getGeodeDirectory() / geodeResourceDirectory;
    CCFileUtils::sharedFileUtils()->addSearchPath(resDir.string().c_str());
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
        << "Loading mods...";

    this->createDirectories();

    for (auto const& dir : this->m_modDirectories) {
        InternalMod::get()->log()
            << Severity::Debug
            << "Searching " << dir ;

        for (auto const& entry : ghc::filesystem::directory_iterator(dir)) {
            if (
                ghc::filesystem::is_regular_file(entry) &&
                entry.path().extension() == geodeModExtension
            ) {
                InternalMod::get()->log()
                    << Severity::Debug
                    << "Loading " << entry.path().string();
                if (!map_utils::contains<std::string, Mod*>(
                    this->m_mods,
                    [entry](Mod* p) -> bool {
                        return p->m_info.m_path == entry.path();
                    }
                )) {
                    auto res = this->loadModFromFile(entry.path().string());
                    if (res && res.value()) {
                        if (!res.value()->hasUnresolvedDependencies()) {
                            InternalMod::get()->log()
                                << "Succesfully loaded " << res.value();
                        } else {
                            InternalMod::get()->log()
                                << res.value() << " has unresolved dependencies";
                        }
                    } else {
                        InternalMod::get()->logInfo(res.error(), Severity::Error);
                        this->m_erroredMods.push_back({ entry.path().string(), res.error() });
                    }
                }
            }
        }
    }

    auto count = Loader::get()->getAllMods().size();
    InternalMod::get()->log() << Severity::Debug << "Loaded " << count << " mods";
    return count;
}

Result<> Loader::saveSettings() {
    auto json = nlohmann::json::object();
    json["mods"] = nlohmann::json::object();
    for (auto [id, mod] : this->m_mods) {
        if (mod->isUninstalled()) continue;
        auto value = nlohmann::json::object();
        value["enabled"] = mod->m_enabled;
        mod->saveDataStore();
        json["mods"][id] = value;
    }
    auto path = this->getGeodeSaveDirectory() / "mods.json";
    return file_utils::writeString(path, json.dump(4));
}

Result<> Loader::loadSettings() {
    auto path = this->getGeodeSaveDirectory() / "mods.json";
    if (!ghc::filesystem::exists(path))
        return Ok<>();
    auto read = file_utils::readString(path);
    if (!read) return read;
    try {
        auto json = nlohmann::json::parse(read.value());
        if (json.contains("mods")) {
            auto mods = json["mods"];
            if (mods.is_object()) {
                for (auto [key, val] : mods.items()) {
                    if (!val.is_object()) {
                        return Err<>("[loader settings].mods.\"" + key + "\" is not an object");
                    }
                    LoaderSettings::ModSettings mod;
                    if (val.contains("enabled")) {
                        if (val["enabled"].is_boolean()) {
                            mod.m_enabled = val["enabled"];
                        } else {
                            return Err<>("[loader settings].mods.\"" + key + "\".enabled is not a boolean");
                        }
                    }

                    this->m_loadedSettings.m_mods.insert({ key, mod });
                }
            } else {
                return Err<>("[loader settings].mods is not an object");
            }
        }
        return Ok<>();
    } catch(std::exception const& e) {
        return Err<>(e.what());
    }
}

bool Loader::shouldLoadMod(std::string const& id) const {
    if (this->m_loadedSettings.m_mods.count(id)) {
        return this->m_loadedSettings.m_mods.at(id).m_enabled;
    }
    return true;
}

bool Loader::isModInstalled(std::string const& id) const {
    return m_mods.count(id) && !m_mods.at(id)->isUninstalled();
}

Mod* Loader::getInstalledMod(std::string const& id) const {
    if (m_mods.count(id) && !m_mods.at(id)->isUninstalled()) {
        return m_mods.at(id);
    }
    return nullptr;
}

bool Loader::isModLoaded(std::string const& id) const {
    return m_mods.count(id) && m_mods.at(id)->isLoaded();
}

Mod* Loader::getLoadedMod(std::string const& id) const {
    if (m_mods.count(id)) {
        auto mod = m_mods.at(id);
        if (mod->isLoaded()) {
            return mod;
        }
    }
    return nullptr;
}

std::vector<Mod*> Loader::getAllMods() const {
    return map_utils::getValues(m_mods);
}

std::vector<Loader::FailedModInfo> const& Loader::getFailedMods() const {
    return m_erroredMods;
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
        << "Setting up Loader...";

    this->createDirectories();
    this->loadSettings();
    this->refreshMods();

    this->m_isSetup = true;

    return true;
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

    auto tempDir = this->getGeodeDirectory() / geodeTempDirectory;
    ghc::filesystem::remove_all(tempDir);
}

void Loader::pushLog(LogPtr* logptr) {
    this->m_logs.push_back(logptr);

    #ifdef GEODE_PLATFORM_CONSOLE
    if (InternalLoader::get()->platformConsoleReady()) {
        std::cout << logptr->toString(true);
    } else {
        InternalLoader::get()->queueConsoleMessage(logptr);
    }
    #endif

    this->m_logStream << logptr->toString(true) << std::endl;
}

void Loader::popLog(LogPtr* log) {
    vector_utils::erase(this->m_logs, log);
    delete log;
}

std::vector<LogPtr*> const& Loader::getLogs() const {
    return this->m_logs;
}

std::vector<LogPtr*> Loader::getLogs(
    std::initializer_list<Severity> severityFilter
) {
    if (!severityFilter.size()) {
        return this->m_logs;
    }

    std::vector<LogPtr*> logs;

    for (auto const& log : this->m_logs) {
        if (vector_utils::contains<Severity>(severityFilter, log->getSeverity())) {
            logs.push_back(log);
        }
    }

    return logs;
}

void Loader::queueInGDThread(std::function<void GEODE_CALL()> func) {
    InternalLoader::get()->queueInGDThread(func);
}

Mod* Loader::getInternalMod() {
    return InternalMod::get();
}

bool Loader::isUnloading() {
    return Loader::s_unloading;
}

ghc::filesystem::path Loader::getGameDirectory() const {
    return ghc::filesystem::path(CCFileUtils::sharedFileUtils()->getWritablePath2().c_str());
}

ghc::filesystem::path Loader::getSaveDirectory() const {
    return ghc::filesystem::path(CCFileUtils::sharedFileUtils()->getWritablePath().c_str());
}

ghc::filesystem::path Loader::getGeodeDirectory() const {
    return geode::utils::dirs::geodeRoot() / geodeDirectory;
}

ghc::filesystem::path Loader::getGeodeSaveDirectory() const {
    return this->getSaveDirectory() / geodeDirectory;
}

size_t Loader::getFieldIndexForClass(size_t hash) {
	static std::unordered_map<size_t, size_t> nextIndex;
	return nextIndex[hash]++;
}
