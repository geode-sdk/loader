#include <loader/Event.hpp>
#include <Geode.hpp>

USE_GEODE_NAMESPACE();

EventCenter* EventCenter::shared = nullptr;

EventCenter::EventCenter() : m_observers() {
	// cocos2d::CCDirector::sharedDirector()->getScheduler()->scheduleUpdateForTarget((cocos2d::CCObject*)this, 0, false);
}

EventCenter* EventCenter::get() {
	if (EventCenter::shared == nullptr)
		EventCenter::shared = new EventCenter;
	return EventCenter::shared;
}

/*void EventCenter::send(Event n, Mod* m) {
	for (auto& obs : m_observers[m][n.selector()]) {
		obs->m_callback(n);
	}
}

void EventCenter::broadcast(Event n) {
	for (auto& [k, v] : m_observers) {
		for (auto& obs : v[n.selector()]) {
			obs->m_callback(n);
		}
	}
}

Observer* EventCenter::registerObserver(Mod* m, std::string selector, callback_t cb) {
	Observer* ob = new Observer;
	ob->m_selector = selector;
	ob->m_callback = cb;
	ob->m_mod = m;

	m_observers[m][selector].push_back(ob);

	return ob;
}*/

void EventCenter::unregisterObserver(Observer<std::monostate>* ob) {
	auto& v2 = m_observers[ob->m_mod][ob->m_info.selector];
	v2.erase(std::remove(v2.begin(), v2.end(), ob), v2.end());

	if (v2.empty())
		m_observers[ob->m_mod].erase(ob->m_info.selector);
	delete ob;
}

std::vector<Observer<std::monostate>*> EventCenter::getObservers(std::string selector, Mod* m) {
	if (m) {
		return m_observers[m][selector];
	} else {
		std::vector<Observer<std::monostate>*> filtered;

		for (auto& [k, v] : m_observers) {
			filtered.insert(filtered.end(), v[selector].begin(), v[selector].end());
		}
		return filtered;
	}
}
