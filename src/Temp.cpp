#include <Geode>
class $modify(MenuLayer) {
	void onMoreGames(CCObject* ob) {
		FLAlertLayer::create("Geode", "Hello from loader", "OK")->show();
	} 
};
