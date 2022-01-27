#include <Interface.hpp>
#include <Mod.hpp>
#include <Hook.hpp>
#include <Log.hpp>
#include <Loader.hpp>

USE_GEODE_NAMESPACE();

void Interface::init(Mod* mod) {
	if (!this->m_mod) {
		this->m_mod = mod;

		for (auto const& hook : this->m_scheduledHooks) {
			this->m_mod->addHook(hook.m_displayName, hook.m_address, hook.m_detour);
		}
		this->m_scheduledHooks.clear();

		for (auto const& log : this->m_scheduledLogs) {
			this->m_mod->logInfo(log.m_info, log.m_severity);
		}
		this->m_scheduledLogs.clear();
	}
}

Result<Hook*> Interface::addHook(std::string_view displayName, void* address, void* detour) {
	if (this->m_mod) {
		return this->m_mod->addHook(displayName, address, detour);
	}
	this->m_scheduledHooks.push_back({ displayName, address, detour });
	return Ok<Hook*>(nullptr);
}

Result<Hook*> Interface::addHook(void* address, void* detour) {
	return Interface::addHook("", address, detour);
}

void Interface::logInfo(
	std::string const& info,
	Severity severity
) {
	if (this->m_mod) {
		return this->m_mod->logInfo(info, severity);
	}
	this->m_scheduledLogs.push_back({ info, severity });
}
