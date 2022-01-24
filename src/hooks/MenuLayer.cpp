#include "hook.hpp"
#include <ModListLayer.hpp>

class $modify(MenuLayer) {
	void onMoreGames(CCObject*) {
		FLAlertLayer::create(
			nullptr, "Title",
			"Description", "Btn1", "Btn2"
		)->show();
		// ModListLayer::scene();
	}
};

