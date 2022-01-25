#include "mod2.hpp"
#include <InterfaceBase.hpp>

class $modify(GJGarageLayer) {
	bool init() {
		if (!GJGarageLayer::init()) return false;

	    if (Loader::get()->isModLoaded("com.geode.test_one")) {
	        logMessage("Hi from TestMod2!");
	    } else {
	        Interface::mod()->log() << "TestMod1 is not loaded :(" << geode::endl;
	    }

	    auto label = CCLabelBMFont::create("Google En Passant", "bigFont.fnt");
	    label->setPosition(100, 100);
	    label->setZOrder(99999);
	    addChild(label);
	    
	    return true;
	}
};


GEODE_API bool GEODE_CALL geode_load(Mod* mod) {
    Interface::get()->init(mod);

    return true;
}

GEODE_API void GEODE_CALL geode_unload() {}
