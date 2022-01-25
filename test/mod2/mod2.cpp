#include "mod2.hpp"
#include <InterfaceBase.hpp>

class $modify(GJGarageLayer) {
	bool init() {
		 Interface::mod()->log() << "uwu uwu uwu uwu" << geode::endl;
		if (!GJGarageLayer::init()) return false;

	    if (Loader::get()->isModLoaded("com.geode.test_one")) {
	        logMessage("Hi from TestMod2!");
	    } else {
	        Interface::mod()->log() << "TestMod1 is not loaded :(" << geode::endl;
	    }

	    auto label = CCLabelBMFont::create("Google En Passant", "bigFont.fnt");
	    label->setPosition(100, 100);
	    label->setScale(0.4);
	    label->setZOrder(99999);
	    addChild(label);

	    auto label2 = CCLabelBMFont::create("Holy Hell", "bigFont.fnt");
	    label2->setPosition(100, 90);
	    label2->setScale(0.4);
	    label2->setZOrder(99999);
	    addChild(label2);
	    
	    return true;
	}
};


GEODE_API bool GEODE_CALL geode_load(Mod* mod) {
    Interface::get()->init(mod);

    return true;
}

GEODE_API void GEODE_CALL geode_unload() {}
