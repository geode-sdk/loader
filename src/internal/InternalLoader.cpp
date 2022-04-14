#include "InternalLoader.hpp"
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include "InternalMod.hpp"
#include <Log.hpp>
#include <Loader.hpp>
#include <Geode.hpp>
// #include <HotReloadLayer.hpp>

InternalLoader::InternalLoader() {

    #ifdef GEODE_PLATFORM_CONSOLE
    this->setupPlatformConsole();
    #endif
}

InternalLoader::~InternalLoader() {
    this->closePlatformConsole();
    delete Loader::get();
}

InternalLoader* InternalLoader::get() {
    static auto g_geode = new InternalLoader;
    return g_geode;
}

bool InternalLoader::setup() {
    InternalMod::get()->log()
        << Severity::Debug << "Set up internal mod representation";

    InternalMod::get()->log()
        << Severity::Debug << "Loading hooks... ";

    if (!this->loadHooks()) {
        InternalMod::get()->log()
            << "There were errors loading some hooks, "
            "see console for details";
    }

    InternalMod::get()->log()
        << Severity::Debug << "Loaded hooks";

    return true;
}

void InternalLoader::queueInGDThread(std::function<void GEODE_CALL()> func) {
    this->m_gdThreadQueue.push_back(func);
}

void InternalLoader::executeGDThreadQueue() {
    for (auto const& func : this->m_gdThreadQueue) {
        func();
    }
    this->m_gdThreadQueue.clear();
}

Result<> InternalLoader::enableHotReload(Mod* mod, ghc::filesystem::path const& path) {
    if (this->m_hotReloads.count(mod)) {
        return Ok<>();
    }
    FileWatcher* reload = new FileWatcher(path, [this, mod](ghc::filesystem::path const& file) -> void {
        try {
            if (!ghc::filesystem::copy_file(file, mod->m_info.m_path, ghc::filesystem::copy_options::overwrite_existing)) {
                mod->logInfo("Unable to copy compiled .geode file!", Severity::Error);
            }
        } catch(std::exception const& e) {
            mod->logInfo(e.what(), Severity::Error);
        }

        this->queueInGDThread([file, mod]() -> void {
            #ifdef GEODE_LOADER
            HotReloadLayer::scene(file.filename().string());
            #endif

            auto unload = mod->unload();
            if (!unload) mod->logInfo(unload.error(), Severity::Error);

            auto temp = mod->createTempDir();
            if (!temp) mod->logInfo(temp.error(), Severity::Error);

            auto load = mod->load();
            if (!load) mod->logInfo(load.error(), Severity::Error);

            cocos2d::CCDirector::sharedDirector()->replaceScene(MenuLayer::scene(false));
        });

    }, [this, mod, reload](std::string const& err) -> void {
        mod->logInfo(err, Severity::Error);
        this->disableHotReload(mod);
    });
    if (!reload->watching()) {
        delete reload;
        return Err<>("yeek");
    }
    this->m_hotReloads.insert({ mod, reload });
    return Ok<>();
}

void InternalLoader::disableHotReload(Mod* mod) {
    if (this->m_hotReloads.count(mod)) {
        delete this->m_hotReloads[mod];
        this->m_hotReloads.erase(mod);
    }
}

bool InternalLoader::isHotReloadEnabled(Mod* mod) const {
    return this->m_hotReloads.count(mod);
}

void InternalLoader::queueConsoleMessage(LogPtr* msg) {
    this->m_logQueue.push_back(msg);
}

bool InternalLoader::platformConsoleReady() const {
    return m_platformConsoleReady;
}

#if defined(GEODE_IS_WINDOWS)

void InternalLoader::platformMessageBox(const char* title, const char* info) {
    MessageBoxA(nullptr, title, info, MB_OK);
}

void InternalLoader::setupPlatformConsole() {
    if (m_platformConsoleReady) return;
    if (AllocConsole() == 0)    return;
    // redirect console output
    freopen_s(reinterpret_cast<FILE**>(stdout), "CONOUT$", "w", stdout);
    freopen_s(reinterpret_cast<FILE**>(stdin), "CONIN$", "r", stdin);

    m_platformConsoleReady = true;
}

void InternalLoader::awaitPlatformConsole() {
    if (!m_platformConsoleReady) return;

    for (auto const& log : this->m_logQueue) {
        std::cout << log->toString(true) << "\n";
        this->m_logQueue.clear();
    }

    std::string inp;
    getline(std::cin, inp);
    std::string inpa;
    std::stringstream ss(inp);
    std::vector<std::string> args;

    while (ss >> inpa) args.push_back(inpa);
    ss.clear();

    // CLIManager::get()->execute(args);

    if (inp == "reload") {
        Loader::get()->refreshMods();
    }

    if (args.size() > 1 && args[0] == "unload") {
        auto mod = Loader::get()->getLoadedMod(args[1]);
        if (mod) {
            auto res = mod->unload();
            if (res) {
                std::cout << "Mod unloaded\n";
            } else {
                std::cout << "Failed to unload: " << res.error() << "\n";
            }
        } else {
            std::cout << "No mod with ID \"" << args[1] << "\" loaded\n";
        }
    }

    if (args.size() > 1 && args[0] == "load") {
        auto mod = Loader::get()->getLoadedMod(args[1]);
        if (mod) {
            auto res = mod->load();
            if (res) {
                std::cout << "Mod loaded\n";
            } else {
                std::cout << "Failed to load: " << res.error() << "\n";
            }
        } else {
            std::cout << "No mod with ID \"" << args[1] << "\" loaded\n";
        }
    }

    if (args.size() > 2 && args[0] == "hot") {
        auto mod = Loader::get()->getLoadedMod(args[1]);
        if (!mod) {
            std::cout << "No mod with ID \"" << args[1] << "\" loaded\n";
        } else {
            if (args[2] == "on") {
                if (args.size() > 3) {
                    auto r = this->enableHotReload(mod, args[3]);
                    if (r) {
                        std::cout << "hot reload enabled\n";
                    } else {
                        std::cout << r.error() << "\n";
                    }
                } else {
                    std::cout << "No path specified\n";
                }
            } else {
                this->disableHotReload(mod);
                std::cout << "hot reload disabled\n";
            }
        }
    }

    if (inp != "e") this->awaitPlatformConsole();
}

void InternalLoader::closePlatformConsole() {
    if (!m_platformConsoleReady) return;

    fclose(stdin);
    fclose(stdout);
    FreeConsole();
}

#elif defined(GEODE_IS_MACOS)
#include <iostream>

void InternalLoader::platformMessageBox(const char* title, const char* info) {
	std::cout << title << ": " << info << std::endl;
}

void InternalLoader::setupPlatformConsole() {
    m_platformConsoleReady = true;
}

void InternalLoader::awaitPlatformConsole() {
}

void InternalLoader::closePlatformConsole() {
}

#elif defined(GEODE_IS_IOS)

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

void InternalLoader::platformMessageBox(const char* title, const char* info) {
    std::cout << title << ": " << info << std::endl;
}

void InternalLoader::setupPlatformConsole() {
    ghc::filesystem::path(getpwuid(getuid())->pw_dir);
    freopen(ghc::filesystem::path(utils::dirs::geode_root() / "geode_log.txt").string().c_str(),"w",stdout);
    InternalLoader::
    m_platformConsoleReady = true;
}

void InternalLoader::awaitPlatformConsole() {
}

void InternalLoader::closePlatformConsole() {
}
#endif

