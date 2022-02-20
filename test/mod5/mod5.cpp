#include "mod5.hpp"

template <int I>
class MyGarageLayerEdit;

class MyGarageMultiplier : public GJGarageLayer {
public:
	field<int> multiplier;
	void onPlayerColor1(cocos2d::CCObject* ob) {
		GJGarageLayer::onPlayerColor1(ob);

		this->*multiplier += 1;
		this->updateLabel();
	}
	bool init() {
		if (!GJGarageLayer::init()) return false;

		this->*multiplier = 1;
		return true;
	}
	void updateLabel() {
		auto label = reinterpret_cast<CCLabelBMFont*>(this->getChildByTag(40));
		if (label) label->setString(std::to_string(40 * this->*multiplier).c_str());
	}
};

template <int I>
class MyGarageLayerEdit : public MyGarageLayer<I> {
public:
	bool init() {
		if (!MyGarageLayer<I>::init()) return false;
		auto label = reinterpret_cast<CCLabelBMFont*>(this->getChildByTag(I));
		label->setString(std::to_string(I * 10).c_str());
		return true;
	}
};

Modify<MyGarageLayerEdit<20>, MyGarageLayer<20>> myModification20;
Modify<MyGarageLayerEdit<60>, MyGarageLayer<60>> myModification60;

Modify<MyGarageMultiplier, GJGarageLayer> myModification40;
