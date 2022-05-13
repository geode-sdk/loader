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
 * @class InternalLoader
 */
class InternalLoader : public Loader {
protected:
	std::vector<LogPtr*> m_logQueue;
	std::vector<std::function<void(void)>> m_gdThreadQueue;
	bool m_platformConsoleReady = false;

	InternalLoader();
	~InternalLoader();
public:
	static InternalLoader* get();

	bool setup();

	bool loadHooks();

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
