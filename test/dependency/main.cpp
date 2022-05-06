#include <Geode.hpp>
#include "../shared.hpp"

USE_GEODE_NAMESPACE();

// API pt. 1
void TestDependency::depTest(GJGarageLayer* gl) {
	    auto label = CCLabelBMFont::create("Mod Interop works!", "bigFont.fnt");
	    label->setPosition(100, 70);
	    label->setScale(.4f);
	    label->setZOrder(99999);
	    gl->addChild(label);
}
