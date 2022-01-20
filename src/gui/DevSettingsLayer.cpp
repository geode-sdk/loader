#include "DevSettingsLayer.hpp"
#include <Internal.hpp>

bool DevSettingsLayer::init(Mod* mod) {
    if (!GJDropDownLayer::init("Dev Settings", 220.f))
        return false;

	this->m_pMod = mod;
    
    auto winSize = CCDirector::sharedDirector()->getWinSize();

	auto menu = CCMenu::create();
	GameToolbox::createToggleButton(
		menu_selector(DevSettingsLayer::onEnableHotReload),
		Geode::get()->isHotReloadEnabled(mod), menu,
		this, menu, .75f, .5f, 100.f, nullptr,
		false, 0, nullptr, "Enable Hot Reload",
		{ winSize.width / 2 - 40.f, winSize.height / 2 + 40.f }, { 10, 0 }
	);

	this->m_pInput = CCTextInputNode::create(
		"Path to .geode file", this, "chatFont.fnt",
		200.f, 50.f
	);
	auto path = Geode::get()->getHotReloadPath(mod);
	if (path.size()) {
		this->m_pInput->setString(path.c_str());
	}
	this->m_pInput->setAllowedChars("0123456789.:-_/\\()[]abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ{|!}");
	this->m_pInput->setPosition(winSize.width / 2, winSize.height / 2);
	this->m_pLayer->addChild(this->m_pInput);

	auto spr = ButtonSprite::create(
		"Paste From Clipboard", 0, 0, "bigFont.fnt", "GJ_button_01.png", 0, .8f
	);
	spr->setScale(.45f);
	auto btn = CCMenuItemSpriteExtra::create(
		spr, this, menu_selector(DevSettingsLayer::onPastePathFromClipboard)
	);
	btn->setPosition(0, -40.f);
	menu->addChild(btn);
	
	this->m_pLayer->addChild(menu);

    return true;
}

void DevSettingsLayer::onEnableHotReload(CCObject* pSender) {
	std::string path = this->m_pInput->getString();
	if (!as<CCMenuItemToggler*>(pSender)->isToggled()) {
		if (!path.size()) {
			FLAlertLayer::create(
				nullptr,
				"Error",
				"OK", nullptr,
				"Set a .geode file path first"
			)->show();
			as<CCMenuItemToggler*>(pSender)->toggle(true);
		} else {
			auto res = Geode::get()->enableHotReload(this->m_pMod, path);
			if (!res) {
				FLAlertLayer::create(
					nullptr,
					"Error",
					"OK", nullptr,
					res.error()
				)->show();
			}
		}
	} else {
		Geode::get()->disableHotReload(this->m_pMod);
	}
}

void DevSettingsLayer::onPastePathFromClipboard(CCObject*) {
	auto data = clipboard::read();
	if (data.size()) {
		this->m_pInput->setString(data.c_str());
	}
}

DevSettingsLayer* DevSettingsLayer::create(Mod* mod) {
    auto ret = new DevSettingsLayer;
    if (ret && ret->init(mod)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}


