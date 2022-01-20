#include "ModSettingsListView.hpp"

bool ModSettingsListView::init(Mod* mod, float width, float height) {
	this->m_pMod = mod;
	this->m_pDelegate = nullptr;
	this->m_bCutContent = true;
	this->m_bDisableVertical = false;

	float offset = 0.f;
	for (auto const& sett : mod->getSettings()) {
		auto node = sett->generate(width);
		if (node) {
			node->setPosition(0.f, offset);
			this->m_pContentLayer->addChild(node);
			offset += node->m_fHeight;
		}
	}

	this->m_pContentLayer->setContentSize({ width, offset });

	this->moveToTop();

	CCDirector::sharedDirector()->getTouchDispatcher()->incrementForcePrio(2);
	this->registerWithTouchDispatcher();
	this->setMouseEnabled(true);

	return true;
}

void ModSettingsListView::scrollWheel(float y, float) {
	this->scrollLayer(y);
}

ModSettingsListView::ModSettingsListView(CCRect const& rect) :
	CCScrollLayerExt(rect) {}

ModSettingsListView* ModSettingsListView::create(
	Mod* mod, float width, float height
) {
	auto ret = new ModSettingsListView({ 0.f, 0.f, width, height });
	if (ret && ret->init(mod, width, height)) {
		ret->autorelease();
		return ret;
	}
	CC_SAFE_DELETE(ret);
	return nullptr;
}
