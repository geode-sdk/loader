#include <Geode.hpp>
#include "../shared.hpp"
#include <malloc/malloc.h>

USE_GEODE_NAMESPACE();

// Exported functions
GEODE_API bool GEODE_CALL geode_enable() {
	geode::log << "Enabled";
	return true;
}

GEODE_API bool GEODE_CALL geode_disable() {
	geode::log << "Disabled";
	return true;
}

GEODE_API bool GEODE_CALL geode_load(Mod*) {
	geode::log << "Loaded";
	return true;
}

GEODE_API bool GEODE_CALL geode_unload() {
	geode::log << "Unoaded";
	return true;
}

// Modify
class $modify(GJGarageLayer) {
	field<int> myValue = 42;
	bool init() {
		if (!GJGarageLayer::init()) return false;

	    auto label = CCLabelBMFont::create("Modify works!", "bigFont.fnt");
	    label->setPosition(100, 110);
	    label->setScale(.4f);
	    label->setZOrder(99999);
	    addChild(label);

	    if (this->*myValue == 42) {
	    	auto label = CCLabelBMFont::create("Field default works!", "bigFont.fnt");
		    label->setPosition(100, 100);
		    label->setScale(.4f);
		    label->setZOrder(99999);
		    addChild(label);
	    }

	    // Data Store
	    auto ds = Mod::get()->getDataStore();
	    int out = ds["times-opened"];
	    ds["times-opened"] = out + 1;

	    std::string text = std::string("Times opened: ") + std::to_string(out);

	    auto label2 = CCLabelBMFont::create(text.c_str(), "bigFont.fnt");
	    label2->setPosition(100, 90);
	    label2->setScale(.4f);
	    label2->setZOrder(99999);
	    addChild(label2);

	    // Dispatch system pt. 1
	    //auto fn = Dispatcher::get()->getSelector<void(GJGarageLayer*)>("test-garage-open");
	    return true;
	}
};


/*// Event system pt. 2
int a = (0, []() {

	Dispatcher::get()->addSelector("test-garage-open", [](GJGarageLayer* gl) {
		auto label = CCLabelBMFont::create("EventCenter works!", "bigFont.fnt");
		label->setPosition(100, 80);
		label->setScale(.4f);
		label->setZOrder(99999);
		gl->addChild(label);

		TestDependency::depTest(gl);
	});

	return 0;
}());*/
