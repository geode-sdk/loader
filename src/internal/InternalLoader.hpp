#pragma once

#include <loader/Log.hpp>
#include <vector>
#include <unordered_map>
#include <utils/Result.hpp>
#include <Loader.hpp>
#include "FileWatcher.hpp"

USE_GEODE_NAMESPACE();

/**
 * Internal extension of Loader for private information
 * @class Geode
 */
class InternalLoader : public Loader {
protected:
	std::vector<LogPtr*> m_logQueue;
	std::unordered_map<Mod*, FileWatcher*> m_hotReloads;
	std::vector<std::function<void(void)>> m_gdThreadQueue;
	bool m_platformConsoleReady = false;

	InternalLoader();
	~InternalLoader();
public:
	static InternalLoader* get();

	bool setup();

	bool loadHooks();

	Result<> enableHotReload(Mod* mod, ghc::filesystem::path const& path);
	void disableHotReload(Mod* mod);
	bool isHotReloadEnabled(Mod* mod) const;

	void queueInGDThread(std::function<void GEODE_CALL(void)> func);
	void executeGDThreadQueue();

	bool platformConsoleReady() const;
	void queueConsoleMessage(LogPtr*);
	void setupPlatformConsole();
	void awaitPlatformConsole();
	void closePlatformConsole();
	static void platformMessageBox(const char* title, const char* info);

	friend int geodeEntry(void* platformData);
};
