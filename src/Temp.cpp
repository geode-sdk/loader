#include <Geode>
#include <helper/AlertPopup.hpp>

USE_GEODE_NAMESPACE();

class $modify(MenuLayer) {
	void onQuit(CCObject* ob) {
		AlertPopup::create("Geode", "Hello from AlertPopup", "OK", "B")->show();
	} 
};
