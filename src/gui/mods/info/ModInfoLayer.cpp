#include "ModInfoLayer.hpp"
#include "../dev/HookListLayer.hpp"
#include "../dev/DevSettingsLayer.hpp"
#include "ModSettingsLayer.hpp"

bool ModInfoLayer::init(Mod* mod) {
    this->m_noElasticity = true;

    this->m_mod = mod;

    auto winSize = CCDirector::sharedDirector()->getWinSize();
	CCSize size { 420.f, 280.f };

    if (!this->initWithColor({ 0, 0, 0, 105 })) return false;
    this->m_mainLayer = CCLayer::create();
    this->addChild(this->m_mainLayer);

    auto bg = CCScale9Sprite::create("GJ_square01.png", { 0.0f, 0.0f, 80.0f, 80.0f });
    bg->setContentSize(size);
    bg->setPosition(winSize.width / 2, winSize.height / 2);
    this->m_mainLayer->addChild(bg);

    this->m_buttonMenu = CCMenu::create();
    this->m_mainLayer->addChild(this->m_buttonMenu);

    auto nameLabel = CCLabelBMFont::create(
        this->m_mod->getName().c_str(), "bigFont.fnt"
    );
    nameLabel->setPosition(winSize.width / 2, winSize.height / 2 + 110.f);
    nameLabel->setScale(.7f);
    this->m_mainLayer->addChild(nameLabel, 2); 

    auto creatorStr = "by " + this->m_mod->getDeveloper();
    auto creatorLabel = CCLabelBMFont::create(
        creatorStr.c_str(), "goldFont.fnt"
    );
    creatorLabel->setPosition(winSize.width / 2, winSize.height / 2 + 85.f);
    creatorLabel->setScale(.8f);
    this->m_mainLayer->addChild(creatorLabel);

    auto descBG = CCScale9Sprite::create("square02b_001.png");
    descBG->setPosition(winSize.width / 2, winSize.height / 2 + 30.f);
    descBG->setContentSize({ 700.f, 120.f });
    descBG->setScale(.5f);
    descBG->setOpacity(100);
    descBG->setColor(cc3x(0));
    this->m_mainLayer->addChild(descBG);

    auto desc = this->m_mod->getDetails().size() ?
        this->m_mod->getDetails() :
        "[No Description Provided]";

    auto descLabel = TextArea::create(
        desc, "chatFont.fnt", 
        1.0f, 330.f, { .5f, .5f }, 50.f, false
    );
    descLabel->setPosition({ winSize.width / 2, winSize.height / 2 + 30.f });
    this->m_mainLayer->addChild(descLabel, 2);

    auto creditsBG = CCScale9Sprite::create("square02b_001.png");
    creditsBG->setPosition(winSize.width / 2, winSize.height / 2 - 50.f);
    creditsBG->setContentSize({ 700.f, 120.f });
    creditsBG->setScale(.5f);
    creditsBG->setOpacity(100);
    creditsBG->setColor(cc3x(0));
    this->m_mainLayer->addChild(creditsBG);

    auto credits = this->m_mod->getCredits().size() ?
        "Credits: " + this->m_mod->getCredits() :
        "[No Credits Provided]";

    auto creditsLabel = TextArea::create(
        credits, "chatFont.fnt",  
        1.0f, 300.f, { .5f, .5f }, 50.f, false
    );
    creditsLabel->setPosition({ winSize.width / 2, winSize.height / 2 - 50.f });
    this->m_mainLayer->addChild(creditsLabel, 2);


    auto hooksSpr = ButtonSprite::create(
        "Hooks", 0, 0, "bigFont.fnt", "GJ_button_05.png", 0, .8f
    );
    hooksSpr->setScale(.6f);

    auto hooksBtn = CCMenuItemSpriteExtra::create(
        hooksSpr, this, menu_selector(ModInfoLayer::onHooks)
    );
    hooksBtn->setPosition(
        -size.width / 2 + 45.f,
        -size.height / 2 + 25.f
    );
    this->m_buttonMenu->addChild(hooksBtn);


    auto settingsSpr = ButtonSprite::create(
        "Settings", 0, 0, "bigFont.fnt", "GJ_button_05.png", 0, .8f
    );
    settingsSpr->setScale(.6f);

    auto settingsBtn = CCMenuItemSpriteExtra::create(
        settingsSpr, this, menu_selector(ModInfoLayer::onSettings)
    );
    settingsBtn->setPosition(
        0.f,
        -size.height / 2 + 25.f
    );
    this->m_buttonMenu->addChild(settingsBtn);


    auto devSpr = ButtonSprite::create(
        "Dev Options", 0, 0, "bigFont.fnt", "GJ_button_05.png", 0, .8f
    );
    devSpr->setScale(.6f);

    auto devBtn = CCMenuItemSpriteExtra::create(
        devSpr, this, menu_selector(ModInfoLayer::onDev)
    );
    devBtn->setPosition(
        size.width / 2 - 65.f,
        -size.height / 2 + 25.f
    );
    this->m_buttonMenu->addChild(devBtn);


    CCDirector::sharedDirector()->getTouchDispatcher()->incrementForcePrio(2);
    this->registerWithTouchDispatcher();
    
    auto closeSpr = CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png");
    closeSpr->setScale(1.0f);

    auto closeBtn = CCMenuItemSpriteExtra::create(
        closeSpr,
        this,
        (SEL_MenuHandler)&ModInfoLayer::onClose
    );
    closeBtn->setUserData(reinterpret_cast<void*>(this));

    this->m_buttonMenu->addChild(closeBtn);

    closeBtn->setPosition( - size.width / 2, size.height / 2 );

    this->setKeypadEnabled(true);
    this->setTouchEnabled(true);

    return true;
}

void ModInfoLayer::onDev(CCObject*) {
    auto layer = DevSettingsLayer::create(this->m_mod);
    this->addChild(layer);
    layer->showLayer(false);
}

void ModInfoLayer::onHooks(CCObject*) {
    auto layer = HookListLayer::create(this->m_mod);
    this->addChild(layer);
    layer->showLayer(false);
}

void ModInfoLayer::onSettings(CCObject*) {
    ModSettingsLayer::create(this->m_mod)->show();
}

void ModInfoLayer::keyDown(enumKeyCodes key) {
    if (key == KEY_Escape)
        return onClose(nullptr);
    if (key == KEY_Space)
        return;
    
    return FLAlertLayer::keyDown(key);
}

void ModInfoLayer::onClose(CCObject* pSender) {
    this->setKeyboardEnabled(false);
    this->removeFromParentAndCleanup(true);
};

ModInfoLayer* ModInfoLayer::create(Mod* mod) {
    auto ret = new ModInfoLayer;
    if (ret && ret->init(mod)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}
