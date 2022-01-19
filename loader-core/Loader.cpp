#include <Hook.hpp>
#include <Mod.hpp>
#include <Log.hpp>
#include <Loader.hpp>
#include <Internal.hpp>
#include <InternalMod.hpp>
#include <helpers/file.hpp>
#include <helpers/conststring.hpp>
#include <helpers/vector.hpp>
#include <helpers/map.hpp>
#include <helpers/types.hpp>

USE_LILAC_NAMESPACE();

Loader* Loader::get() {
    static auto g_loader = new Loader;
    return g_loader;
}

void Loader::createDirectories() {
    try {
        file_utils::createDirectory(const_join_path_c_str<lilac_directory>);
        file_utils::createDirectory(const_join_path_c_str<lilac_directory, lilac_resource_directory>);
        file_utils::createDirectory(const_join_path_c_str<lilac_directory, lilac_mod_directory>);
        ghc::filesystem::remove_all(const_join_path_c_str<lilac_directory, lilac_temp_directory>);
    } catch(...) {}
}

size_t Loader::updateMods() {
    InternalMod::get()->log()
        << Severity::Debug
        << "Loading mods..."
        << lilac::endl;

    size_t loaded = 0;
    this->createDirectories();
    for (auto const& entry : ghc::filesystem::directory_iterator(
        ghc::filesystem::absolute(lilac_directory) / lilac_mod_directory
    )) {
        if (
            ghc::filesystem::is_regular_file(entry) &&
            entry.path().extension() == lilac_mod_extension
        ) {
            InternalMod::get()->log()
                << Severity::Debug
                << "Loading " << entry.path().string()
                << lilac::endl;
            if (!map_utils::contains<std::string, Mod*>(
                this->m_mods,
                [entry](Mod* p) -> bool {
                    return p->m_info.m_path == entry.path().string();
                }
            )) {
                auto res = this->loadModFromFile(entry.path().string());
                if (res && res.value()) {
                    if (!res.value()->hasUnresolvedDependencies()) {
                        loaded++;
                        InternalMod::get()->log()
                            << "Succesfully loaded " << res.value() << lilac::endl;
                    } else {
                        InternalMod::get()->log()
                            << res.value() << " has unresolved dependencies" << lilac::endl;
                    }
                } else {
                    InternalMod::get()->throwError(res.error(), Severity::Error);
                }
            }
        }
    }
    InternalMod::get()->log()
        << Severity::Debug
        << "Loaded " << loaded << " new mods"
        << lilac::endl;
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

std::vector<Mod*> Loader::getLoadedMods() const {
    return map_utils::getValues(this->m_mods);
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
        << lilac::endl;

    this->createDirectories();
    this->updateMods();

    this->m_isSetup = true;

    return true;
}

Loader::Loader() {
    this->m_logStream = new LogStream;
}

Loader::~Loader() {
    for (auto const& [_, mod] : this->m_mods) {
        delete mod;
    }
    this->m_mods.clear();
    for (auto const& log : this->m_logs) {
        delete log;
    }
    this->m_logs.clear();
    delete this->m_logStream;
    ghc::filesystem::remove_all(const_join_path<lilac_directory, lilac_temp_directory>);
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

void Loader::queueInGDThread(std::function<void()> func) {
    Lilac::get()->queueInGDThread(func);
}
