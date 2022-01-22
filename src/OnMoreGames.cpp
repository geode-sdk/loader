#include <Geode>
#include <fmt/format.h>

class $(MenuLayer) {
	field_t<int> myInt;
    void onMoreGames(cocos2d::CCObject* ob) {
    	this->*myInt += 1;
    	std::string form = fmt::format("Hello {} times!", this->*myInt);
    	FLAlertLayer::create("Geode", form, "OK")->show();
    } 
};
