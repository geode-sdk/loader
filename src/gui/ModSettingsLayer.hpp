#pragma once

#include <Geode>

USE_GEODE_NAMESPACE();

class ModSettingsLayer : public FLAlertLayer {
    protected:
        Mod* m_mod;

		bool init(Mod* mod);

		void keyDown(cocos2d::enumKeyCodes) override;
		void onClose(cocos2d::CCObject*);
		
    public:
        static ModSettingsLayer* create(Mod* Mod);
};

