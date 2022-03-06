#include <Notification.hpp>
#include <Geode.hpp>

USE_GEODE_NAMESPACE();

NotificationCenter* NotificationCenter::shared = nullptr;

NotificationCenter::NotificationCenter() {
	cocos2d::CCDirector::sharedDirector()->getScheduler()->scheduleUpdateForTarget((cocos2d::CCObject*)this, 0, false);
}

NotificationCenter* NotificationCenter::get() {
	if (NotificationCenter::shared == nullptr)
		NotificationCenter::shared = new NotificationCenter;
	return NotificationCenter::shared;
}

void NotificationCenter::send(Notification n, Mod* m) {
	m_queue.push({m, std::move(n)});
}

void NotificationCenter::broadcast(Notification n) {
	send(std::move(n), nullptr);
}

void NotificationCenter::sendSync(Notification n, Mod* m) {
	for (auto& obs : m_observers) {
		if (obs.m_selector == n.selector() && (m == nullptr || m == obs.m_mod)) {
			obs.m_callback(n);
		}
	}
}

void NotificationCenter::broadcastSync(Notification n) {
	sendSync(std::move(n), nullptr);
}

Observer* NotificationCenter::registerObserver(Mod* m, std::string selector, callback_t cb) {
	Observer ob;
	ob.m_selector = selector;
	ob.m_callback = cb;
	ob.m_mod = m;

	m_observers.push_back(std::move(ob));

	return &m_observers.back();
}

void NotificationCenter::unregisterObserver(Observer* ob) {
	for (int i = 0; i < m_observers.size(); ++i) {
		if (&m_observers[i] == ob) {
			m_observers.erase(m_observers.begin()+i);
			return;
		}
	}
}

std::vector<Observer*> NotificationCenter::getObservers(std::string selector, Mod* m) {
	std::vector<Observer*> filtered;

	for (auto& obs : m_observers) {
		if (obs.m_selector == selector && (m == nullptr || m == obs.m_mod)) {
			filtered.push_back(&obs);
		}
	}

	return filtered;
}

void NotificationCenter::update(float) {
	while (!m_queue.empty()) {
		auto n = std::move(m_queue.front());
		m_queue.pop();
		sendSync(std::move(n.second), n.first);
	}
}
