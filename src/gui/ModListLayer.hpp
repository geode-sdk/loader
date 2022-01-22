#pragma once

#include <Geode>
#include "ModListView.hpp"

USE_GEODE_NAMESPACE();

class ModListLayer : public CCLayer {
protected:
	GJListLayer* m_pList = nullptr;
	CCLabelBMFont* m_pListLabel;
	CCMenu* m_pMenu;

	bool init() override;

	void onExit(CCObject*);
	void onReload(CCObject*);
	void keyDown(enumKeyCodes) override;

	void reloadList();

public:
	static ModListLayer* create();
	static ModListLayer* scene();
};
