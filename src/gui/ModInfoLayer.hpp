#pragma once

#include <Geode>

USE_GEODE_NAMESPACE();

class ModInfoLayer : public FLAlertLayer {
    protected:
        Mod* m_pMod;

        void onHooks(CCObject*);
        void onDev(CCObject*);
        void onSettings(CCObject*);

		bool init(Mod* mod);

		void keyDown(cocos2d::enumKeyCodes) override;
		void onClose(cocos2d::CCObject*);
		
    public:
        static ModInfoLayer* create(Mod* Mod);
};
