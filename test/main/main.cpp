#include <Geode.hpp>
#include "../shared.hpp"

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
	bool init() {
		if (!GJGarageLayer::init()) return false;

	    auto label = CCLabelBMFont::create("Modify works!", "bigFont.fnt");
	    label->setPosition(100, 100);
	    label->setScale(.4f);
	    label->setZOrder(99999);
	    addChild(label);

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

	    // Event system pt. 1
	    EventCenter::get()->broadcast(Event(
	    	"test-garage-open",
	    	this,
	    	Mod::get()
	    ));
	    return true;
	}
};

// Event system pt. 2
$observe("test-garage-open", GJGarageLayer*, evt) {
		auto gl = evt.object();
	    auto label = CCLabelBMFont::create("EventCenter works!", "bigFont.fnt");
	    label->setPosition(100, 80);
	    label->setScale(.4f);
	    label->setZOrder(99999);
	    gl->addChild(label);

	    // API pt. 2
	    TestDependency::depTest(gl);
}
