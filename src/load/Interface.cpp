#include <loader/Interface.hpp>
#include <loader/Mod.hpp>
#include <loader/Hook.hpp>
#include <loader/Log.hpp>
#include <loader/Loader.hpp>

USE_GEODE_NAMESPACE();

void Interface::init(Mod* mod) {
	if (!this->m_mod) {
		this->m_mod = mod;
		for (auto const& hook : this->m_scheduledHooks) {
			std::invoke(hook.m_addFunction, this->m_mod, hook.m_displayName, hook.m_address);
		}
		this->m_scheduledHooks.clear();

		for (auto const& log : this->m_scheduledLogs) {
			this->m_mod->logInfo(log.m_info, log.m_severity);
		}
		this->m_scheduledLogs.clear();

		for (auto const& scheduled : this->m_scheduledExports) {
			if (auto fn = static_cast<std::add_const_t<unknownmemfn_t>*>(std::get_if<0>(&scheduled.m_func))) {
				this->m_mod->exportAPIFunction(scheduled.m_selector, static_cast<unknownmemfn_t>(*fn));
			}
			else if (auto fn = static_cast<std::add_const_t<unknownfn_t>*>(std::get_if<1>(&scheduled.m_func))) {
				this->m_mod->exportAPIFunction(scheduled.m_selector, static_cast<unknownfn_t>(*fn));
			}
		}
		this->m_scheduledExports.clear();

		for (auto const& fn : this->m_scheduledFunctions) {
			fn(this->m_mod);
		}
		this->m_scheduledFunctions.clear();
	}
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

void Interface::exportAPIFunctionInternal(std::string const& selector, unknownmemfn_t fn) {
	if (this->m_mod) {
		m_mod->exportAPIFunction(selector, fn);
	}
	this->m_scheduledExports.push_back({ selector, fn });
}
void Interface::exportAPIFunctionInternal(std::string const& selector, unknownfn_t fn) {
	if (this->m_mod) {
		m_mod->exportAPIFunction(selector, fn);
	}
	this->m_scheduledExports.push_back({ selector, fn });
}

void Interface::scheduleOnLoad(loadfn_t fn) {
	this->m_scheduledFunctions.push_back(fn);
}

Interface* Interface::create() {
	return new Interface;
}
