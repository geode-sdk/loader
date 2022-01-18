#include "Internal.hpp"
#include <Windows.h>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include "InternalMod.hpp"
#include <interface/Log.hpp>
#include <interface/Loader.hpp>
#include <interface/CLIManager.hpp>
#include <interface/HotReloadLayer.hpp>

Lilac::Lilac() {
    // init KeybindManager & load default keybinds
    KeybindManager::get();

    #ifdef LILAC_PLATFORM_CONSOLE
    this->setupPlatformConsole();
    #endif
}

Lilac::~Lilac() {
    this->closePlatformConsole();
    delete Loader::get();
}

Lilac* Lilac::get() {
    static auto g_lilac = new Lilac;
    return g_lilac;
}

bool Lilac::setup() {
    InternalMod::get()->log()
        << Severity::Debug << "Set up internal mod representation" << lilac::endl;

    InternalMod::get()->log()
        << Severity::Debug << "Loading hooks... " << lilac::endl;

    if (!this->loadHooks()) {
        InternalMod::get()->log()
            << "There were errors loading some hooks, "
            "see console for details" << lilac::endl;
    }

    InternalMod::get()->log()
        << Severity::Debug << "Loaded hooks" << lilac::endl;

    InternalMod::get()->addKeybindAction(TriggerableAction {
        "Yeetus Feetus",
        "lilac.yeetus",
        KB_GLOBAL_CATEGORY,
        [](auto node, bool down) -> bool {
            if (down) {
                auto count = Loader::get()->updateMods();
                FLAlertLayer::create(
                    nullptr, "yea", "OK", nullptr,
                    "woo wee " + std::to_string(count)
                )->show();
            }
            return false;
        }
    }, {{ KEY_G, Keybind::kmControl | Keybind::kmAlt }});

    return true;
}

void Lilac::queueInGDThread(std::function<void()> func) {
    this->m_gdThreadQueue.push_back(func);
}

void Lilac::executeGDThreadQueue() {
    for (auto const& func : this->m_gdThreadQueue) {
        func();
    }
    this->m_gdThreadQueue.clear();
}

Result<> Lilac::enableHotReload(Mod* mod, ghc::filesystem::path const& path) {
    if (this->m_hotReloads.count(mod)) {
        return Ok<>();
    }
    FileWatcher* reload = new FileWatcher(path, [this, mod](ghc::filesystem::path const& file) -> void {
        try {
            if (!ghc::filesystem::copy_file(file, mod->m_info.m_path, ghc::filesystem::copy_options::overwrite_existing)) {
                mod->throwError("Unable to copy compiled .lilac file!", Severity::Error);
            }
        } catch(std::exception const& e) {
            mod->throwError(e.what(), Severity::Error);
        }

        this->queueInGDThread([file, mod]() -> void {
            HotReloadLayer::scene(file.filename().string());

            auto unload = mod->unload();
            if (!unload) mod->throwError(unload.error(), Severity::Error);

            auto temp = mod->createTempDir();
            if (!temp) mod->throwError(temp.error(), Severity::Error);

            auto load = mod->load();
            if (!load) mod->throwError(load.error(), Severity::Error);

            CCDirector::sharedDirector()->replaceScene(MenuLayer::scene(false));
        });

    }, [this, mod, reload](std::string const& err) -> void {
        mod->throwError(err, Severity::Error);
        this->disableHotReload(mod);
    });
    if (!reload->watching()) {
        delete reload;
        return Err<>("yeek");
    }
    this->m_hotReloads.insert({ mod, reload });
    return Ok<>();
}

void Lilac::disableHotReload(Mod* mod) {
    if (this->m_hotReloads.count(mod)) {
        delete this->m_hotReloads[mod];
        this->m_hotReloads.erase(mod);
    }
}

bool Lilac::isHotReloadEnabled(Mod* mod) const {
    return this->m_hotReloads.count(mod);
}

std::string Lilac::getHotReloadPath(Mod* mod) const {
    if (!this->isHotReloadEnabled(mod)) {
        return "";
    }
    return this->m_hotReloads.at(mod)->path().string();
}

#ifdef LILAC_IS_WINDOWS

void Lilac::queueConsoleMessage(LogMessage* msg) {
    this->m_logQueue.push_back(msg);
}

bool Lilac::platformConsoleReady() const {
    return m_platformConsoleReady;
}

void Lilac::platformMessageBox(const char* title, const char* info) {
    MessageBoxA(nullptr, title, info, MB_OK);
}

void Lilac::setupPlatformConsole() {
    if (m_platformConsoleReady) return;
    if (AllocConsole() == 0)    return;
    // redirect console output
    freopen_s(reinterpret_cast<FILE**>(stdout), "CONOUT$", "w", stdout);
    freopen_s(reinterpret_cast<FILE**>(stdin), "CONIN$", "r", stdin);

    m_platformConsoleReady = true;
}

void Lilac::awaitPlatformConsole() {
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
        Loader::get()->updateMods();
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

void Lilac::closePlatformConsole() {
    if (!m_platformConsoleReady) return;

    fclose(stdin);
    fclose(stdout);
    FreeConsole();
}

#endif

