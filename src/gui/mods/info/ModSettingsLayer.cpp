#include "ModSettingsLayer.hpp"
#include "../settings/ModSettingsListView.hpp"

bool ModSettingsLayer::init(Mod* mod) {
    this->m_noElasticity = true;

    this->m_mod = mod;

    auto winSize = CCDirector::sharedDirector()->getWinSize();
	CCSize size { 420.f, 280.f };

    if (!this->initWithColor({ 0, 0, 0, 105 })) return false;
    this->m_mainLayer = CCLayer::create();
    this->addChild(this->m_mainLayer);

    auto bg = CCScale9Sprite::create("GJ_square02.png", { 0.0f, 0.0f, 80.0f, 80.0f });
    bg->setContentSize(size);
    bg->setPosition(winSize.width / 2, winSize.height / 2);
    this->m_mainLayer->addChild(bg);

    this->m_buttonMenu = CCMenu::create();
    this->m_mainLayer->addChild(this->m_buttonMenu);

	auto nameStr = this->m_mod->getName() + " Settings";
    auto nameLabel = CCLabelBMFont::create(
        nameStr.c_str(), "bigFont.fnt"
    );
    nameLabel->setPosition(winSize.width / 2, winSize.height / 2 + 110.f);
    nameLabel->setScale(.7f);
    this->m_mainLayer->addChild(nameLabel, 2); 

	auto settings = ModSettingsListView::create(mod, 300.f, 200.f);
	settings->setPosition(winSize.width / 2 - 150.f, winSize.height / 2 - 100.f);
	this->m_mainLayer->addChild(settings);

    CCDirector::sharedDirector()->getTouchDispatcher()->incrementForcePrio(2);
    this->registerWithTouchDispatcher();
    
    auto closeSpr = CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png");
    closeSpr->setScale(1.0f);

    auto closeBtn = CCMenuItemSpriteExtra::create(
        closeSpr,
        this,
        (SEL_MenuHandler)&ModSettingsLayer::onClose
    );
    closeBtn->setUserData(reinterpret_cast<void*>(this));

    this->m_buttonMenu->addChild(closeBtn);

    closeBtn->setPosition( - size.width / 2, size.height / 2 );

    this->setKeypadEnabled(true);
    this->setTouchEnabled(true);

    return true;
}

void ModSettingsLayer::keyDown(enumKeyCodes key) {
    if (key == KEY_Escape)
        return onClose(nullptr);
    if (key == KEY_Space)
        return;
    return FLAlertLayer::keyDown(key);
}

void ModSettingsLayer::onClose(CCObject* pSender) {
    this->setKeyboardEnabled(false);
    this->removeFromParentAndCleanup(true);
};

ModSettingsLayer* ModSettingsLayer::create(Mod* mod) {
    auto ret = new ModSettingsLayer;
    if (ret && ret->init(mod)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

