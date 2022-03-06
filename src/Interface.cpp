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

		for (auto const& scheduled : this->m_scheduledExports) {
			if (auto fn = static_cast<std::add_const_t<unknownmemfn_t>*>(std::get_if<0>(&scheduled.m_func))) {
				mod->exportAPIFunction(scheduled.m_selector, static_cast<unknownmemfn_t>(*fn));
			}
			else if (auto fn = static_cast<std::add_const_t<unknownfn_t>*>(std::get_if<1>(&scheduled.m_func))) {
				mod->exportAPIFunction(scheduled.m_selector, static_cast<unknownfn_t>(*fn));
			}
		}
		this->m_scheduledExports.clear();
	}
}

Result<Hook*> Interface::addHook(void* address, void* detour) {
	return this->addHook("", address, detour);
}

Result<Hook*> Interface::addHook(std::string_view displayName, void* address, void* detour) {
	if (this->m_mod) {
		return this->m_mod->addHook(displayName, address, detour);
	}
	this->m_scheduledHooks.push_back({ displayName, address, detour });
	return Ok<Hook*>(nullptr);
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
		mod->exportAPIFunction(selector, fn);
	}
	this->m_scheduledExports.push_back({ selector, fn });
}
void Interface::exportAPIFunctionInternal(std::string const& selector, unknownfn_t fn) {
	if (this->m_mod) {
		mod->exportAPIFunction(selector, fn);
	}
	this->m_scheduledExports.push_back({ selector, fn });
}

const char* operator"" _sprite(const char* str, size_t) {
        return Interface::mod()->expandSpriteName(str);
}
