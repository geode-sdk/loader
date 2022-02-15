#include "mod5.hpp"

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
