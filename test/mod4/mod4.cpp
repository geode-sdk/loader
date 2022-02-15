#include "mod4.hpp"
using namespace geode::modifier;

template <int I>
class MyGarageLayer : public GJGarageLayer {
public:
	bool init() {
		if (!GJGarageLayer::init()) return false;

	    auto label = CCLabelBMFont::create("New Modifier Syntax", "bigFont.fnt");
	    label->setPosition(500, 100);
	    label->setScale(.4f);
	    label->setZOrder(99999);
	    addChild(label);

	    auto label2 = CCLabelBMFont::create(std::to_string(I).c_str(), "bigFont.fnt");
	    label2->setPosition(500, I);
	    label2->setScale(.4f);
	    label2->setZOrder(99999);
	    addChild(label2);
	    
	    return true;
	}
};
Modify<MyGarageLayer<20>, GJGarageLayer> myModification20;
Modify<MyGarageLayer<40>, GJGarageLayer> myModification40;
Modify<MyGarageLayer<60>, GJGarageLayer> myModification60;
