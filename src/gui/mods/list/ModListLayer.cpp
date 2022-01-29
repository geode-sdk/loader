#include "ModListLayer.hpp"
#include <InternalMod.hpp>

bool ModListLayer::init() {
	if (!CCLayer::init())
		return false;
	
    auto winSize = CCDirector::sharedDirector()->getWinSize();
	
	auto bg = CCSprite::create("GJ_gradientBG.png");
	auto bgSize = bg->getTextureRect().size;

	bg->setAnchorPoint({ 0.0f, 0.0f });
	bg->setScaleX((winSize.width + 10.0f) / bgSize.width);
	bg->setScaleY((winSize.height + 10.0f) / bgSize.height);
	bg->setPosition({ -5.0f, -5.0f });
	bg->setColor({ 0, 102, 255 });

	this->addChild(bg);

	this->m_pMenu = CCMenu::create();


    auto backBtn = CCMenuItemSpriteExtra::create(
		CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png"),
		this,
		menu_selector(ModListLayer::onExit)
	);
	backBtn->setPosition(-winSize.width / 2 + 25.0f, winSize.height / 2 - 25.0f);
	this->m_pMenu->addChild(backBtn);

	this->addChild(this->m_pMenu);

	auto reloadSpr = CCSprite::createWithSpriteFrameName("GJ_updateBtn_001.png");
	reloadSpr->setScale(.8f);
    auto reloadBtn = CCMenuItemSpriteExtra::create(
		reloadSpr, this, menu_selector(ModListLayer::onReload)
	);
	reloadBtn->setPosition(-winSize.width / 2 + 30.0f, - winSize.height / 2 + 30.0f);
	this->m_pMenu->addChild(reloadBtn);

	this->addChild(this->m_pMenu);

	
    this->m_pListLabel = CCLabelBMFont::create("No mods loaded!", "bigFont.fnt");

    this->m_pListLabel->setPosition(winSize / 2);
    this->m_pListLabel->setScale(.6f);
    this->m_pListLabel->setVisible(false);
    this->m_pListLabel->setZOrder(1001);

    this->addChild(this->m_pListLabel);

	this->reloadList();

    this->setKeyboardEnabled(true);
    this->setKeypadEnabled(true);

	return true;
}

void ModListLayer::reloadList() {
    auto winSize = CCDirector::sharedDirector()->getWinSize();

    if (this->m_pList)
        this->m_pList->removeFromParent();

    ModListView* list = nullptr;

	auto mods = Loader::get()->getLoadedMods();
    if (!mods.size()) {
        m_pListLabel->setVisible(true);
    } else {
        m_pListLabel->setVisible(false);

		auto arr = CCArray::create();
		arr->addObject(new ModObject(InternalMod::get()));
		for (auto const& mod : mods) {
			arr->addObject(new ModObject(mod));
		}
        list = ModListView::create(arr);
    }

    this->m_pList = GJListLayer::create(
        list, "Mods", { 0, 0, 0, 180 }, 356.0f, 220.0f
    );
    this->m_pList->setPosition(
        winSize / 2 - this->m_pList->getScaledContentSize() / 2
    );
    this->addChild(this->m_pList);
}

void ModListLayer::onExit(CCObject*) {
	CCDirector::sharedDirector()->replaceScene(
		CCTransitionFade::create(.5f, MenuLayer::scene(false))
	);
}

void ModListLayer::onReload(CCObject*) {
	Loader::get()->refreshMods();
	this->reloadList();
}

void ModListLayer::keyDown(enumKeyCodes key) {
	if (key == KEY_Escape) {
        this->onExit(nullptr);
	}
}

ModListLayer* ModListLayer::create() {
	auto ret = new ModListLayer;
	if (ret && ret->init()) {
		ret->autorelease();
		return ret;
	}
	CC_SAFE_DELETE(ret);
	return nullptr;
}

ModListLayer* ModListLayer::scene() {
	auto scene = CCScene::create();
	auto layer = ModListLayer::create();
	scene->addChild(layer);
	CCDirector::sharedDirector()->replaceScene(
		CCTransitionFade::create(.5f, scene)
	);
	return layer;
}
