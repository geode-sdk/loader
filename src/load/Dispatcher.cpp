#include <loader/Dispatcher.hpp>

USE_GEODE_NAMESPACE();

std::vector<dispatch_handle> Dispatcher::allFunctions_() {
	std::vector<dispatch_handle> b;

	for (auto& [k, v] : m_selectorMap) {
		b.insert(b.end(), v.begin(), v.end());
	}
	return b;
}

std::vector<dispatch_handle> Dispatcher::getFunctions_(std::string const& a) {
	return m_selectorMap[a];
}

void Dispatcher::removeFunction(dispatch_handle u) {
	if (!m_dispatchMap.count(u))
		throw std::invalid_argument("Dispatch handle not found");


	auto& vtr = m_selectorMap[m_dispatchMap[u].first];
	vtr.erase(std::remove(vtr.begin(), vtr.end(), u), vtr.end());

	m_dispatchMap.erase(m_dispatchMap.find(u));

	operator delete(u.handle);
}

void Dispatcher::addFunction_(Mod* m, std::string const& a, dispatch_handle u) {
	m_dispatchMap[u] = std::make_pair(a, m);
	m_selectorMap[a].push_back(u);
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
