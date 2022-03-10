#include "mod2.hpp"
#include <InterfaceBase.hpp>

class $modify(GJGarageLayer) {
	bool init() {
		if (!GJGarageLayer::init()) return false;

	    if (Loader::get()->isModLoaded("com.geode.test_one")) {
	        logMessage("Hi from TestMod2!");
	    } else {
	        Log::get() << "TestMod1 is not loaded :(";
	    }

	    auto label = CCLabelBMFont::create("Google En Passant", "bigFont.fnt");
	    label->setPosition(100, 100);
	    label->setScale(.4f);
	    label->setZOrder(99999);
	    addChild(label);

	    auto label2 = CCLabelBMFont::create("Holy Hell", "bigFont.fnt");
	    label2->setPosition(100, 90);
	    label2->setScale(.4f);
	    label2->setZOrder(99999);
	    addChild(label2);
	    
	    return true;
	}
};

GEODE_API bool GEODE_CALL geode_enable() {
	Log::get() << "Enabling TestMod2!";
	return true;
}

GEODE_API bool GEODE_CALL geode_disable() {
	Log::get() << "Disabling TestMod2!";
	return true;
}

