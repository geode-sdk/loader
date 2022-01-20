#pragma once

#include <Geode>

USE_GEODE_NAMESPACE();

class DevSettingsLayer : public GJDropDownLayer {
protected:
	Mod* m_pMod;
	CCTextInputNode* m_pInput;

	bool init(Mod* mod);

	void onEnableHotReload(CCObject*);
	void onPastePathFromClipboard(CCObject*);

public:
	static DevSettingsLayer* create(Mod* mod);
};

