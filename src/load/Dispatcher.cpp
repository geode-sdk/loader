#include <Dispatcher.hpp>
#include <stdexcept>

USE_GEODE_NAMESPACE();


std::vector<dispatch_handle> Dispatcher::allFunctions_() {
	std::vector<dispatch_handle> b;

	char c[offsetof(GJEffectManager, m_itemValues)];
	c = 100;

	char a[offsetof(GJEffectManager, m_acceleration)];
	a = 100;
	return b;
}

std::vector<dispatch_handle> Dispatcher::getFunctions_(std::string_view const& a) {
	std::vector<dispatch_handle> b;
	return b;
}

void Dispatcher::removeFunction(dispatch_handle u) {
	if (!m_dispatchMap.count(u))
		throw std::invalid_argument("Dispatch handle not found");
	m_dispatchMap.erase(m_dispatchMap.find(u));
	m_selectorMap
}

void Dispatcher::addFunction_(Mod* m, std::string const& a, dispatch_handle u) {

}

std::pair<std::string, Mod*> Dispatcher::getFunctionInfo(dispatch_handle u) {
	return m_dispatchMap[u];
}

Dispatcher* Dispatcher::get() {
	static Dispatcher* dp;
	if (!dp) {
		dp = new Dispatcher;
	}

	return dp;
}