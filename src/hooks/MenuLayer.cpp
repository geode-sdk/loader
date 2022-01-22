#include "hook.hpp"
#include <ModListLayer.hpp>

class $modify(MenuLayer) {
	void onMoreGames(CCObject*) {
		ModListLayer::scene();
	}
};

