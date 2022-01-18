#pragma once

#include <interface/Log.hpp>
#include <vector>
#include "FileWatcher.hpp"

USE_LILAC_NAMESPACE();

/**
 * For internal state management.
 * @class Lilac
 */
class Lilac {
protected:
	std::vector<LogMessage*> m_logQueue;
	std::unordered_map<Mod*, FileWatcherBase*> m_hotReloads;
	std::vector<std::function<void(void)>> m_gdThreadQueue;
	bool m_platformConsoleReady = false;

	Lilac();

public:
	static Lilac* get();
	~Lilac();

	bool setup();

	bool loadHooks();

	Result<> enableHotReload(Mod* mod, ghc::filesystem::path const& path);
	void disableHotReload(Mod* mod);
	bool isHotReloadEnabled(Mod* mod) const;
	std::string getHotReloadPath(Mod* mod) const;

	void queueInGDThread(std::function<void(void)> func);
	void executeGDThreadQueue();

	bool platformConsoleReady() const;
	void queueConsoleMessage(LogMessage*);
	void setupPlatformConsole();
	void awaitPlatformConsole();
	void closePlatformConsole();
	void platformMessageBox(const char* title, const char* info);
};
