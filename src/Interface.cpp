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
			this->m_mod->addHook(hook.m_address, hook.m_detour);
		}
		this->m_scheduledHooks.clear();

		for (auto const& log : this->m_scheduledLogs) {
			// Loader::get()->log(log);
		}
		this->m_scheduledLogs.clear();
	}
}

Result<Hook*> Interface::addHook(void* address, void* detour) {
	if (this->m_mod) {
		return this->m_mod->addHook(address, detour);
	}
	this->m_scheduledHooks.push_back({ address, detour });
	return Ok<Hook*>(nullptr);
}

void Interface::throwError(
	std::string const& info,
	Severity severity
) {
	if (this->m_mod) {
		return this->m_mod->throwError(info, severity);
	}
    auto log = new LogMessage(
        std::string(info),
        severity,
        this->m_mod
    );
	this->m_scheduledLogs.push_back(log);
}
